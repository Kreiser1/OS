#pragma once

#include "std.h"
#include "segmentation.h"
#include "io.h"

#define INTERRUPT_PRESENT (uint8_t)128
#define INTERRUPT_USER (uint8_t)96
#define INTERRUPT_TASK_GATE (uint8_t)5
#define INTERRUPT_GATE (uint8_t)14
#define INTERRUPT_TRAP (uint8_t)15

#define INTERRUPT_DESCRIPTOR_TABLE_LENGTH (uint16_t)256

#define INTERRUPT_EXCEPTION (uint8_t)32
#define INTERRUPT_SYSTEM (uint8_t)48
#define INTERRUPT_PIC_MASTER (uint8_t)32
#define INTERRUPT_PIC_SLAVE (uint8_t)40

typedef struct
{
    uint16_t OffsetLow;
    uint16_t CodeSegment;
    uint8_t Reserved;
    uint8_t Flags;
    uint64_t OffsetHigh : 48;
    uint32_t Reserved2;
} packed INTERRUPT_DESCRIPTOR, *PINTERRUPT_DESCRIPTOR, *INTERRUPT_DESCRIPTOR_TABLE;

typedef struct
{
    uint16_t Limit;
    uint64_t Offset;
} packed INTERRUPT_DESCRIPTOR_TABLE_REGISTER, *PINTERRUPT_DESCRIPTOR_TABLE_REGISTER;

typedef struct
{
    uint64_t Ds, Rax, Rcx, Rdx, Rbx, Rbp, Rsi, Rdi;
    uint64_t Interrupt, ErrorCode;
    uint64_t Rip, Cs, Rflags, Rsp, Ss;
} packed CPU_FRAME, *PCPU_FRAME;

void interruptsEnable()
{
    asm("sti");
}

void interruptsDisable()
{
    asm("cli");
}

void initializePic()
{
    ioWrite8(0x20, 0x11);
    ioWrite8(0xA0, 0x11);
    ioWrite8(0x21, INTERRUPT_PIC_MASTER);
    ioWrite8(0xA1, INTERRUPT_PIC_SLAVE);
    ioWrite8(0x21, 0x04);
    ioWrite8(0xA1, 0x02);
    ioWrite8(0x21, 0x01);
    ioWrite8(0xA1, 0x01);
    ioWrite8(0x21, 0x00);
    ioWrite8(0xA1, 0x00);
}

void picDisable()
{
    ioWrite16(0x21, 0xFF);
    ioWrite16(0xA1, 0xFF);
}

void picUpdate(bool slave)
{
    if (slave) ioWrite8(0xA0, 0x20);
    ioWrite8(0x20, 0x20);
}

void mapInterrupt(INTERRUPT_DESCRIPTOR_TABLE idt, uint8_t interrupt, uint64_t offset, uint8_t flags)
{
    idt[interrupt].Reserved = 0;
    idt[interrupt].Reserved2 = 0;
    idt[interrupt].CodeSegment = SEGMENT_KERNEL_CODE;
    idt[interrupt].OffsetLow = offset;
    idt[interrupt].OffsetHigh = offset >> 16;
    idt[interrupt].Flags = flags | INTERRUPT_PRESENT;
}

void deleteInterrupt(INTERRUPT_DESCRIPTOR_TABLE idt, uint8_t interrupt)
{
    idt[interrupt].Flags &= ~INTERRUPT_PRESENT;
}

void loadIdt(INTERRUPT_DESCRIPTOR_TABLE idt)
{
	static INTERRUPT_DESCRIPTOR_TABLE_REGISTER idtr;
	idtr.Offset = (uint64_t)idt;
	idtr.Limit = INTERRUPT_DESCRIPTOR_TABLE_LENGTH * sizeof(INTERRUPT_DESCRIPTOR) - 1;
    
	asm("lidt %0" :: "m" (idtr));
}