#include "std.h"
#include "memory.h"
#include "../FXE/fxebtldr.h"
#include "segmentation.h"
#include "interrupts.h"
#include "pit.h"
#include "io.h"
#include "power.h"
#include "string.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"
#include "text_render.h"
#include "keyboard.h"
#include "clock.h"
#include "ata.h"
#include "fs.h"
#include "license.h"

#define INTERRUPT_SYSCALL (uint8_t)0xFF
#define SYSTEM_MEMORY_POOL (uint64_t)0x100000000
#define SYSTEM_MEMORY_POOL_MINIMAL_LENGTH (uint64_t)67108864
#define SYSTEM_MEMORY_POOL_MAXIMAL_LENGTH (uint64_t)77309411328
#define SYSTEM_HEAP_LENGTH (uint64_t)4194304
#define SYSTEM_TASK_COUNT (uint64_t)2048

SEGMENT_DESCRIPTOR globalDescriptorTable[GLOBAL_DESCRIPTOR_TABLE_LENGTH];
extern uint64_t interrupt_offset_table[];
INTERRUPT_DESCRIPTOR interruptDescriptorTable[INTERRUPT_DESCRIPTOR_TABLE_LENGTH];
TEXT_RENDERER textRenderer;
VGA_TEXT_FRAMEBUFFER framebuffer;
static uint8_t physicalPageMap[PHYSICAL_PAGE_MAP_LENGTH];
HEAP heap;
uint64_t totalMemory = 0;
RTC_TIME bootTime;
ATA_DRIVE bootDrive = 0;
ATA_IDENTITY ataDrives[ATA_DRIVE_LAST + 2];

void interrupt(CPU_FRAME cpuFrame)
{
	if (cpuFrame.Interrupt < INTERRUPT_EXCEPTION)
	{
		if (textRenderer.Framebuffer)
		{
			uint64_t cr2;
			asm("mov %%cr2, %0" : "=r"(cr2));
			txSetColor(&textRenderer, VGA_BACKGROUND_COLOR(VGA_TEXT_COLOR_RED) | VGA_TEXT_COLOR_WHITE);
			txPrint(&textRenderer, "\r\rKernel-level exception $i:$i occured at address 0x$p.\rExecution was stopped.\r\rCR2 = 0x$p.",
					cpuFrame.Interrupt, cpuFrame.ErrorCode, cpuFrame.Rip, cr2);
			txRender(&textRenderer, framebuffer);
		}

		powerHalt();
	}
	else if (cpuFrame.Interrupt >= INTERRUPT_PIC_MASTER && cpuFrame.Interrupt < INTERRUPT_SYSTEM)
	{
		switch (cpuFrame.Interrupt)
		{
		case INTERRUPT_PIT:
			pitTick();
			if (textRenderer.Framebuffer && !(pitGetTickCount() % TEXT_RENDER_FREQUENCY))
				txRender(&textRenderer, framebuffer);
			break;
		case INTERRUPT_KEYBOARD:
			kbUpdate();
			break;
		default:
			break;
		}

		picUpdate(cpuFrame.Interrupt >= INTERRUPT_PIC_SLAVE);
	}
	else if (cpuFrame.Interrupt == INTERRUPT_SYSCALL)
	{
		if (cpuFrame.Cs == SEGMENT_KERNEL_CODE)
		{
			switch (cpuFrame.Rax)
			{
			default:
				break;
			}
		}
		else
		{
			switch (cpuFrame.Rax)
			{
			default:
				break;
			}
		}
	}
}

void main(PFXEBTLDR fxebtldr)
{
	framebuffer = (VGA_TEXT_FRAMEBUFFER)fxebtldr->FramebufferOffset;

	deleteSegment(globalDescriptorTable, SEGMENT_NULL);

	mapSegment(globalDescriptorTable, SEGMENT_KERNEL_CODE, SEGMENT_EXECUTABLE | SEGMENT_NON_SYSTEM);
	mapSegment(globalDescriptorTable, SEGMENT_KERNEL_DATA, SEGMENT_WRITE | SEGMENT_NON_SYSTEM);

	mapSegment(globalDescriptorTable, SEGMENT_USER_CODE, SEGMENT_USER | SEGMENT_EXECUTABLE | SEGMENT_NON_SYSTEM);
	mapSegment(globalDescriptorTable, SEGMENT_USER_DATA, SEGMENT_USER | SEGMENT_WRITE | SEGMENT_NON_SYSTEM);

	loadGdt(globalDescriptorTable);
	flushSegments();

	for (uint16_t i = 0; i < INTERRUPT_DESCRIPTOR_TABLE_LENGTH; i++)
		if (i < INTERRUPT_EXCEPTION)
			mapInterrupt(interruptDescriptorTable, i, interrupt_offset_table[i], INTERRUPT_TRAP);
		else if (i < INTERRUPT_SYSTEM)
			mapInterrupt(interruptDescriptorTable, i, interrupt_offset_table[i], INTERRUPT_GATE);
		else
			deleteInterrupt(interruptDescriptorTable, i);

	mapInterrupt(interruptDescriptorTable, INTERRUPT_SYSCALL, interrupt_offset_table[INTERRUPT_SYSTEM], INTERRUPT_GATE);

	interruptsDisable();

	loadIdt(interruptDescriptorTable);

	initializePic();
	initializePit();
	initializeRtc();

	interruptsEnable();

	pmClaim(physicalPageMap, 0, PM_LENGTH);

	for (uint32_t i = 0; i < fxebtldr->BiosMemoryMapLength; i++)
		if (((PBIOS_MEMORY_REGION)fxebtldr->BiosMemoryMapOffset)[i].Type == BIOS_MEMORY_REGION_USABLE)
			for (uint32_t j = aligned_count_padded(((PBIOS_MEMORY_REGION)fxebtldr->BiosMemoryMapOffset)[i].Offset, PHYSICAL_PAGE_LENGTH); j < aligned_count_padded(((PBIOS_MEMORY_REGION)fxebtldr->BiosMemoryMapOffset)[i].Offset + ((PBIOS_MEMORY_REGION)fxebtldr->BiosMemoryMapOffset)[i].Length, PHYSICAL_PAGE_LENGTH); j++)
				pmYield(physicalPageMap, j * PHYSICAL_PAGE_LENGTH, PHYSICAL_PAGE_LENGTH);

	pmClaim(physicalPageMap, FXE_SYSTEM_MEMORY, FXE_SYSTEM_MEMORY_LENGTH);
	pmClaim(physicalPageMap, fxebtldr->KernelOffset, fxebtldr->KernelLength);

	uint64_t vmmPhysicalOffset = pmRequirest(physicalPageMap, VMM_LENGTH);

	if (!vmmPhysicalOffset)
		return;

	fxeMap(fxebtldr, vmmPhysicalOffset, VMM_OFFSET, VMM_LENGTH);
	initializeVmm(vmmPhysicalOffset);

	vmMap(FXE_SYSTEM_MEMORY, FXE_SYSTEM_MEMORY, FXE_SYSTEM_MEMORY_LENGTH);
	vmMap(fxebtldr->KernelOffset, FXE_SECTION_HEADER.Offset, fxebtldr->KernelLength);

	do
	{
		uintptr_t physicalPageOffset = pmRequirest(physicalPageMap, PHYSICAL_PAGE_LENGTH);

		if (!physicalPageOffset)
			break;

		vmMap(physicalPageOffset, SYSTEM_MEMORY_POOL + totalMemory, PHYSICAL_PAGE_LENGTH);
		totalMemory += PHYSICAL_PAGE_LENGTH;

		if (totalMemory >= SYSTEM_MEMORY_POOL_MAXIMAL_LENGTH)
			break;
	} while (true);

	vmYield(SYSTEM_MEMORY_POOL, totalMemory);

	vmDelete(0, PAGE_LENGTH);

	loadVmm(vmmPhysicalOffset);

	if (totalMemory < SYSTEM_MEMORY_POOL_MINIMAL_LENGTH)
		return;

	initializeHeap(&heap, vmRequirest(SYSTEM_HEAP_LENGTH), SYSTEM_HEAP_LENGTH, 64);

	if (!heap.BlockMap)
		return;

	initializeTextRenderer(&textRenderer, VGA_BACKGROUND_COLOR(VGA_TEXT_COLOR_BLACK) | VGA_TEXT_COLOR_WHITE, fxebtldr->FramebufferWidth, fxebtldr->FramebufferHeight, 16);

	textRenderer.Framebuffer = (char *)heapRequirest(&heap, textRenderer.Length);
	txClear(&textRenderer);

	rtcRead(&bootTime);

	ataDrives[0].Present = false;

	for (ATA_DRIVE drive = ATA_DRIVE_1; drive <= ATA_DRIVE_LAST; drive++)
		ataIdentify(drive, &ataDrives[drive]);

	for (ATA_DRIVE drive = ATA_DRIVE_1; drive <= ATA_DRIVE_LAST; drive++)
		if (ataDrives[drive].Present && ataDrives[drive].Supported)
		{
			PFXEBTLDR driveFxebtldr = heapRequirest(&heap, ATA_SECTOR_LENGTH);

			if (ataReadSector(drive, 0, driveFxebtldr) &&
				driveFxebtldr->Signature == FXEBTLDR_SIGNATURE &&
				mmCompare((uintptr_t)fxebtldr->Key, (uintptr_t)driveFxebtldr->Key, sizeof(fxebtldr->Key)))
			{
				if (bootDrive)
					return;

				bootDrive = drive;
			}

			heapFree(&heap, &driveFxebtldr);
		}

	txPrint(
		&textRenderer,
		"OS initialized.\r\r"
		"Kernel length: $iK\r"
		"Usable memory length: $iM\r"
		"VMM address space length: $iG\r"
		"Video mode: Text $ix$i\r"
		"Power control: $s\r"
		"System time: $i-$i-$i $i:$i:$i\r"
		"Timer frequency: $iHz\r"
		"BIOS boot drive: $p\r"
		"ATA boot drive: $i\r"
		"\r",
		FXE_SECTION_HEADER.Length / 1024, totalMemory / 1048576, VM_LENGTH / 1073741824, fxebtldr->FramebufferWidth,
		fxebtldr->FramebufferHeight, !fxebtldr->AcpiRsdpOffset && !fxebtldr->SmBiosEpsOffset ? "None" : fxebtldr->AcpiRsdpOffset && fxebtldr->SmBiosEpsOffset ? "ACPI + SMBIOS"
																									: fxebtldr->AcpiRsdpOffset								  ? "ACPI"
																																							  : "SMBIOS",
		bootTime.Day, bootTime.Month, bootTime.Year, bootTime.Hours, bootTime.Minutes, bootTime.Seconds,
		PIT_TICK_FREQUENCY, fxebtldr->BootDrive, bootDrive);

	txPrint(&textRenderer, "\rDrives:\r");

	for (ATA_DRIVE i = ATA_DRIVE_1; i <= ATA_DRIVE_LAST; i++)
		if (ataDrives[i].Present && ataDrives[i].Supported)
			txPrint(&textRenderer, "ATA $i: $iK.\r", i, ataDrives[i].Length / 1024);

	txPrint(&textRenderer, "\r");

	if (!bootDrive)
	{
		txPrint(&textRenderer, "Could not determine the boot drive.\r");
		return;
	}

	txRender(&textRenderer, framebuffer);

	// detectFS:

	// if (!fsReadTable(bootDrive, &fsTable))
	// {
	// 	txPrint(&textRenderer, "No filesystem found on the boot drive.\r");

	// 	if (!fsInstall(bootDrive, ataDrives[bootDrive].Length - FS_TABLE_OFFSET <= 268435456 ? ataDrives[bootDrive].Length - FS_TABLE_OFFSET : 268435456))
	// 	{
	// 		txPrint(&textRenderer, "Failed to install filesystem on the boot drive.\r");
	// 		return;
	// 	}
	// 	txPrint(&textRenderer, "Installed filesystem onto the boot drive.\r");

	// 	goto detectFS;
	// }

	// txPrint(&textRenderer, "$iK filesystem found on the boot drive.\r\r", fsTable.Length / 1024);

	txPrint(&textRenderer, "End of the kernel.");
}