#pragma once

/*
BSD 3-Clause License

Copyright (c) 2025, Syrtsov Vadim

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

volatile static const char fxeLicenseString[] =
	"BSD 3-Clause License\n\n"
	"Copyright (c) 2025, Syrtsov Vadim\n\n"
	"Redistribution and use in source and binary forms, with or without\n"
	"modification, are permitted provided that the following conditions are met:\n\n"
	"1.\tRedistributions of source code must retain the above copyright notice, this\n"
	"\tlist of conditions and\n"
	"\tthe following disclaimer.\n\n"
	"2.\tRedistributions in binary form must reproduce the above copyright notice,\n"
	"\tthis list of conditions and the following disclaimer in the documentation\n"
	"\tand/or other materials provided with the distribution.\n\n"
	"3.\tNeither the name of the copyright holder nor the names of its\n"
	"\tcontributors may be used to endorse or promote products derived from\n"
	"\tthis software without specific prior written permission.\n\n"
	"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\"\n"
	"AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n"
	"IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\n"
	"DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE\n"
	"FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL\n"
	"DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR\n"
	"SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER\n"
	"CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,\n"
	"OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE\n"
	"OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.";

#define FXE_FALSE (__UINT8_TYPE__)0
#define FXE_TRUE (__UINT8_TYPE__)1

#define FXE_ALIGN(value, alignment) ((__UINT64_TYPE__)value / (__UINT64_TYPE__)alignment * (__UINT64_TYPE__)alignment)
#define FXE_ALIGN_PADDED(value, alignment) ((__UINT64_TYPE__)value % (__UINT64_TYPE__)alignment ? (FXE_ALIGN((__UINT64_TYPE__)value, alignment) + (__UINT64_TYPE__)alignment) : FXE_ALIGN(value, alignment))

#define FXE_ALIGNED_COUNT(value, alignment) ((__UINT64_TYPE__)value / (__UINT64_TYPE__)alignment)
#define FXE_ALIGNED_COUNT_PADDED(value, alignment) ((__UINT64_TYPE__)value % (__UINT64_TYPE__)alignment ? FXE_ALIGNED_COUNT(value, alignment) + 1ull : FXE_ALIGNED_COUNT(value, alignment))

#define FXE_SIGNATURE (__UINT32_TYPE__)0x20455846ul

typedef struct
{
	__UINT32_TYPE__ Signature;
	__UINT64_TYPE__ Offset;
	__UINT64_TYPE__ Length;
	__UINT64_TYPE__ CodeOffset;
	__UINT64_TYPE__ CodeLength;
	__UINT64_TYPE__ DataOffset;
	__UINT64_TYPE__ DataLength;
	__UINT64_TYPE__ EntryOffset;
} __attribute__((__packed__)) FXE_HEADER, *PFXE_HEADER;

#define FXEBTLDR_SIGNATURE (__UINT64_TYPE__)0x52444C5442455846ull

typedef struct
{
	__UINT32_TYPE__ Jump : 24;
	__UINT64_TYPE__ Reserved : 40;
	__UINT64_TYPE__ Signature;
	__UINT8_TYPE__ Key[64];
	__UINT8_TYPE__ BootDrive;
	__UINT8_TYPE__ CpuVendor[12];
	__UINT64_TYPE__ CpuFeatures;
	__UINT32_TYPE__ BiosMemoryMapOffset;
	__UINT32_TYPE__ BiosMemoryMapLength;
	__UINT32_TYPE__ SmBiosEpsOffset;
	__UINT32_TYPE__ AcpiRsdpOffset;
	__UINT32_TYPE__ FramebufferOffset;
	__UINT32_TYPE__ FramebufferLength;
	__UINT16_TYPE__ FramebufferWidth;
	__UINT16_TYPE__ FramebufferHeight;
	__UINT8_TYPE__ FramebufferPixelLength;
	__UINT32_TYPE__ GdtOffset;
	__UINT32_TYPE__ GdtLength;
	__UINT32_TYPE__ IdtOffset;
	__UINT32_TYPE__ IdtLength;
	__UINT32_TYPE__ PageMapOffset;
	__UINT32_TYPE__ PageMapLength;
	__UINT32_TYPE__ KernelOffset;
	__UINT32_TYPE__ KernelLength;
} __attribute__((__packed__)) FXEBTLDR, *PFXEBTLDR;

#define BIOS_MEMORY_REGION_USABLE ((__UINT32_TYPE__)1ul)
#define BIOS_MEMORY_REGION_RESERVED ((__UINT32_TYPE__)2ul)
#define BIOS_MEMORY_REGION_ACPI_RECLAIMABLE ((__UINT32_TYPE__)3ul)
#define BIOS_MEMORY_REGION_ACPI_NVS ((__UINT32_TYPE__)4ul)
#define BIOS_MEMORY_REGION_BAD ((__UINT32_TYPE__)5ul)

typedef struct
{
	__UINT64_TYPE__ Offset;
	__UINT64_TYPE__ Length;
	__UINT32_TYPE__ Type;
	__UINT32_TYPE__ Attributes;
} __attribute__((__packed__)) BIOS_MEMORY_REGION, *PBIOS_MEMORY_REGION, *BIOS_MEMORY_MAP;

#define FXE_PAGE_LENGTH (__UINT32_TYPE__)0x200000ul
#define FXE_PAGE_PRESENT (__UINT64_TYPE__)0x83ull
#define FXE_PAGE_RESERVED (__UINT64_TYPE__)0x200ull
#define FXE_ADDRESS_SPACE_LENGTH (__UINT64_TYPE__)0x100000000ull
#define FXE_SYSTEM_MEMORY (__UINT64_TYPE__)0ull
#define FXE_SYSTEM_MEMORY_LENGTH (__UINT64_TYPE__)0x100000ull
#define FXE_PAGE_MAP_LENGTH (FXE_ADDRESS_SPACE_LENGTH / FXE_PAGE_LENGTH)

typedef __UINT64_TYPE__ *PT;

extern FXE_HEADER FXE_SECTION_HEADER;

__UINT8_TYPE__ fxeMap(PFXEBTLDR fxebtldr, __UINT64_TYPE__ offset, __UINTPTR_TYPE__ pointer, __UINT64_TYPE__ length)
{
	if (fxebtldr->Signature != FXEBTLDR_SIGNATURE)
		return FXE_FALSE;

	if (offset > FXE_ADDRESS_SPACE_LENGTH || pointer > FXE_ADDRESS_SPACE_LENGTH || offset + length > FXE_ADDRESS_SPACE_LENGTH || pointer + length > FXE_ADDRESS_SPACE_LENGTH)
		return FXE_FALSE;

	__UINT32_TYPE__ startPage = FXE_ALIGNED_COUNT(pointer, FXE_PAGE_LENGTH);
	__UINT32_TYPE__ endPage = startPage + FXE_ALIGNED_COUNT_PADDED(length, FXE_PAGE_LENGTH);

	if (endPage <= startPage)
		return FXE_FALSE;

	offset = FXE_ALIGN(offset, FXE_PAGE_LENGTH);

	PT pageTableEntries = (PT)fxebtldr->PageMapOffset;

	for (__UINT32_TYPE__ i = startPage; i < endPage; i++)
		if (!(pageTableEntries[i] & FXE_PAGE_PRESENT && pageTableEntries[i] & FXE_PAGE_RESERVED))
			pageTableEntries[i] = offset + (i - startPage) * FXE_PAGE_LENGTH | FXE_PAGE_PRESENT;
		else
			return FXE_FALSE;

	return FXE_TRUE;
}

__UINT8_TYPE__ fxeDelete(PFXEBTLDR fxebtldr, __UINTPTR_TYPE__ pointer, __UINT64_TYPE__ length)
{
	if (fxebtldr->Signature != FXEBTLDR_SIGNATURE)
		return FXE_FALSE;

	if (pointer > FXE_ADDRESS_SPACE_LENGTH || pointer + length > FXE_ADDRESS_SPACE_LENGTH)
		return FXE_FALSE;

	__UINT32_TYPE__ startPage = FXE_ALIGNED_COUNT(pointer, FXE_PAGE_LENGTH);
	__UINT32_TYPE__ endPage = startPage + FXE_ALIGNED_COUNT_PADDED(length, FXE_PAGE_LENGTH);

	if (endPage <= startPage)
		return FXE_FALSE;

	PT pageTableEntries = (PT)fxebtldr->PageMapOffset;

	for (__UINT32_TYPE__ i = startPage; i < endPage; i++)
		if (!(pageTableEntries[i] & FXE_PAGE_PRESENT && pageTableEntries[i] & FXE_PAGE_RESERVED))
			pageTableEntries[i] &= ~FXE_PAGE_PRESENT;
		else
			return FXE_FALSE;

	return FXE_TRUE;
}

__UINT8_TYPE__ fxeCopy(PFXEBTLDR fxebtldr, __UINTPTR_TYPE__ pointer1, __UINTPTR_TYPE__ pointer2, __UINT64_TYPE__ length)
{
	if (fxebtldr->Signature != FXEBTLDR_SIGNATURE)
		return FXE_FALSE;

	if (!length || pointer1 > FXE_ADDRESS_SPACE_LENGTH || pointer2 > FXE_ADDRESS_SPACE_LENGTH || pointer1 + length > FXE_ADDRESS_SPACE_LENGTH || pointer2 + length > FXE_ADDRESS_SPACE_LENGTH)
		return FXE_FALSE;

	__UINT32_TYPE__ pageCount = FXE_ALIGNED_COUNT_PADDED(length, FXE_PAGE_LENGTH);
	__UINT32_TYPE__ sourcePage = FXE_ALIGNED_COUNT(pointer1, FXE_PAGE_LENGTH);
	__UINT32_TYPE__ destinationPage = FXE_ALIGNED_COUNT(pointer2, FXE_PAGE_LENGTH);

	PT pageTableEntries = (PT)fxebtldr->PageMapOffset;

	for (__UINT32_TYPE__ i = 0; i < pageCount; i++)
		if (pageTableEntries[sourcePage + i] & FXE_PAGE_PRESENT)
			if (!(pageTableEntries[i] & FXE_PAGE_PRESENT && pageTableEntries[i] & FXE_PAGE_RESERVED))
				pageTableEntries[destinationPage + i] = pageTableEntries[sourcePage + i] & ~FXE_PAGE_RESERVED;
			else
				return FXE_FALSE;
		else
			return FXE_FALSE;

	return FXE_TRUE;
}

__UINT8_TYPE__ fxeRelocate(PFXEBTLDR fxebtldr, __UINTPTR_TYPE__ pointer1, __UINTPTR_TYPE__ pointer2, __UINT64_TYPE__ length)
{
	if (fxebtldr->Signature != FXEBTLDR_SIGNATURE)
		return FXE_FALSE;

	if (fxeCopy(fxebtldr, pointer1, pointer2, length))
		return fxeDelete(fxebtldr, pointer1, length);
	else
		return FXE_FALSE;
}

__UINT8_TYPE__ fxeFlags(PFXEBTLDR fxebtldr, __UINTPTR_TYPE__ pointer)
{
	if (fxebtldr->Signature != FXEBTLDR_SIGNATURE)
		return 0;

	if (pointer > FXE_ADDRESS_SPACE_LENGTH)
		return 0;

	PT pageTableEntries = (PT)fxebtldr->PageMapOffset;

	return pageTableEntries[FXE_ALIGNED_COUNT(pointer, FXE_PAGE_LENGTH)] & (__UINT64_TYPE__)0xFFF;
}

#define CPU_FEATURE_SSE3 ((__UINT64_TYPE__)1ull << 0)
#define CPU_FEATURE_PCLMUL ((__UINT64_TYPE__)1ull << 1)
#define CPU_FEATURE_DTES64 ((__UINT64_TYPE__)1ull << 2)
#define CPU_FEATURE_MONITOR ((__UINT64_TYPE__)1ull << 3)
#define CPU_FEATURE_DS_CPL ((__UINT64_TYPE__)1ull << 4)
#define CPU_FEATURE_VMX ((__UINT64_TYPE__)1ull << 5)
#define CPU_FEATURE_SMX ((__UINT64_TYPE__)1ull << 6)
#define CPU_FEATURE_EST ((__UINT64_TYPE__)1ull << 7)
#define CPU_FEATURE_TM2 ((__UINT64_TYPE__)1ull << 8)
#define CPU_FEATURE_SSSE3 ((__UINT64_TYPE__)1ull << 9)
#define CPU_FEATURE_CID ((__UINT64_TYPE__)1ull << 10)
#define CPU_FEATURE_SDBG ((__UINT64_TYPE__)1ull << 11)
#define CPU_FEATURE_FMA ((__UINT64_TYPE__)1ull << 12)
#define CPU_FEATURE_CX16 ((__UINT64_TYPE__)1ull << 13)
#define CPU_FEATURE_XTPR ((__UINT64_TYPE__)1ull << 14)
#define CPU_FEATURE_PDCM ((__UINT64_TYPE__)1ull << 15)
#define CPU_FEATURE_PCID ((__UINT64_TYPE__)1ull << 17)
#define CPU_FEATURE_DCA ((__UINT64_TYPE__)1ull << 18)
#define CPU_FEATURE_SSE4_1 ((__UINT64_TYPE__)1ull << 19)
#define CPU_FEATURE_SSE4_2 ((__UINT64_TYPE__)1ull << 20)
#define CPU_FEATURE_X2APIC ((__UINT64_TYPE__)1ull << 21)
#define CPU_FEATURE_MOVBE ((__UINT64_TYPE__)1ull << 22)
#define CPU_FEATURE_POPCNT ((__UINT64_TYPE__)1ull << 23)
#define CPU_FEATURE_AES ((__UINT64_TYPE__)1ull << 25)
#define CPU_FEATURE_TSC2 ((__UINT64_TYPE__)1ull << 24)
#define CPU_FEATURE_XSAVE ((__UINT64_TYPE__)1ull << 26)
#define CPU_FEATURE_OSXSAVE ((__UINT64_TYPE__)1ull << 27)
#define CPU_FEATURE_AVX ((__UINT64_TYPE__)1ull << 28)
#define CPU_FEATURE_F16C ((__UINT64_TYPE__)1ull << 29)
#define CPU_FEATURE_RDRAND ((__UINT64_TYPE__)1ull << 30)
#define CPU_FEATURE_HYPERVISOR ((__UINT64_TYPE__)1ull << 31)
#define CPU_FEATURE_FPU ((__UINT64_TYPE__)1ull << 32)
#define CPU_FEATURE_VME ((__UINT64_TYPE__)1ull << 33)
#define CPU_FEATURE_DE ((__UINT64_TYPE__)1ull << 34)
#define CPU_FEATURE_PSE ((__UINT64_TYPE__)1ull << 35)
#define CPU_FEATURE_TSC ((__UINT64_TYPE__)1ull << 36)
#define CPU_FEATURE_MSR ((__UINT64_TYPE__)1ull << 37)
#define CPU_FEATURE_PAE ((__UINT64_TYPE__)1ull << 38)
#define CPU_FEATURE_MCE ((__UINT64_TYPE__)1ull << 39)
#define CPU_FEATURE_CX8 ((__UINT64_TYPE__)1ull << 40)
#define CPU_FEATURE_APIC ((__UINT64_TYPE__)1ull << 41)
#define CPU_FEATURE_SEP ((__UINT64_TYPE__)1ull << 42)
#define CPU_FEATURE_MTRR ((__UINT64_TYPE__)1ull << 43)
#define CPU_FEATURE_PGE ((__UINT64_TYPE__)1ull << 44)
#define CPU_FEATURE_MCA ((__UINT64_TYPE__)1ull << 45)
#define CPU_FEATURE_CMOV ((__UINT64_TYPE__)1ull << 46)
#define CPU_FEATURE_PAT ((__UINT64_TYPE__)1ull << 47)
#define CPU_FEATURE_PSE36 ((__UINT64_TYPE__)1ull << 48)
#define CPU_FEATURE_PSN ((__UINT64_TYPE__)1ull << 49)
#define CPU_FEATURE_CLFLUSH ((__UINT64_TYPE__)1ull << 50)
#define CPU_FEATURE_DS ((__UINT64_TYPE__)1ull << 51)
#define CPU_FEATURE_ACPI ((__UINT64_TYPE__)1ull << 52)
#define CPU_FEATURE_MMX ((__UINT64_TYPE__)1ull << 53)
#define CPU_FEATURE_FXSR ((__UINT64_TYPE__)1ull << 54)
#define CPU_FEATURE_SSE ((__UINT64_TYPE__)1ull << 55)
#define CPU_FEATURE_SSE2 ((__UINT64_TYPE__)1ull << 56)
#define CPU_FEATURE_SS ((__UINT64_TYPE__)1ull << 57)
#define CPU_FEATURE_HTT ((__UINT64_TYPE__)1ull << 58)
#define CPU_FEATURE_TM ((__UINT64_TYPE__)1ull << 59)
#define CPU_FEATURE_IA64 ((__UINT64_TYPE__)1ull << 60)
#define CPU_FEATURE_PBE ((__UINT64_TYPE__)1ull << 61)