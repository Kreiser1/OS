; Copyright (c) 2025 Syrtsov Vadim. All rights reserved.
; Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
; 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
; 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
; 3. All advertising materials mentioning features or use of this software must display the following acknowledgement: This product includes software developed by the Syrtsov Vadim and its contributors.
; 4. Neither the name of the Syrtsov Vadim nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
; THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

incbin "license.txt"

%define FXE_FALSE 0
%define FXE_TRUE 1

%define FXE_ALIGN(value, alignment) (value / alignment * alignment)
%define FXE_ALIGN_PADDED(value, alignment) (value % alignment ? (FXE_ALIGN(value, alignment) + alignment) : FXE_ALIGN(value, alignment))

%define FXE_ALIGNED_COUNT(value, alignment) (value / alignment)
%define FXE_ALIGNED_COUNT_PADDED(value, alignment) (value % alignment ? FXE_ALIGNED_COUNT(value, alignment) + 1 : FXE_ALIGNED_COUNT(value, alignment))

%define FXE_SIGNATURE 0x20455846

struc FXE_HEADER
    .Signature: resd 1
    .FxeOffset: resq 1
    .Length: resq 1
    .CodeOffset: resq 1
    .CodeLength: resq 1
    .DataOffset: resq 1
    .DataLength: resq 1
    .EntryOffset: resq 1
endstruc

%define FXEBTLDR_SIGNATURE 0x52444C5442455846

struc FXEBTLDR
    .Jump: resb 3
    .Reserved: resb 5
    .Signature: resq 1
    .Key: resb 64
    .BootDrive: resb 1
    .CpuVendor: resb 12
    .CpuFeatures: resq 1
    .BiosMemoryMapOffset: resd 1
    .BiosMemoryMapLength: resd 1
    .SmBiosEpsOffset: resd 1
    .AcpiRsdpOffset: resd 1
    .FramebufferOffset: resd 1
    .FramebufferLength: resd 1
    .FramebufferWidth: resw 1
    .FramebufferHeight: resw 1
    .FramebufferPixelLength: resb 1
    .GdtOffset: resd 1
    .GdtLength: resd 1
    .IdtOffset: resd 1
    .IdtLength: resd 1
    .PageMapOffset: resd 1
    .PageMapLength: resd 1
    .KernelOffset: resd 1
    .KernelLength: resd 1
endstruc

%define BIOS_MEMORY_REGION_USABLE 1
%define BIOS_MEMORY_REGION_RESERVED 2
%define BIOS_MEMORY_REGION_ACPI_RECLAIMABLE 3
%define BIOS_MEMORY_REGION_ACPI_NVS 4
%define BIOS_MEMORY_REGION_BAD 5

struc BIOS_MEMORY_REGION
    .Offset: resq 1
    .Length: resq 1
    .Type: resd 1
    .Attributes: resd 1
endstruc

%define FXE_PAGE_LENGTH 0x200000
%define FXE_PAGE_PRESENT 0x83
%define FXE_PAGE_RESERVED 0x200
%define FXE_ADDRESS_SPACE_LENGTH 0x100000000
%define FXE_SYSTEM_MEMORY 0
%define FXE_SYSTEM_MEMORY_LENGTH 0x100000
%define FXE_PAGE_MAP_LENGTH (FXE_ADDRESS_SPACE_LENGTH / FXE_PAGE_LENGTH)

extern FXE_SECTION_HEADER
extern FXE_SECTION_DATA
extern FXE_SECTION_TEXT

extern fxeMap
extern fxeDelete
extern fxeCopy
extern fxeRelocate
extern fxeFlags

%define CPU_FEATURE_SSE3 (1 << 0)
%define CPU_FEATURE_PCLMUL (1 << 1)
%define CPU_FEATURE_DTES64 (1 << 2)
%define CPU_FEATURE_MONITOR (1 << 3)
%define CPU_FEATURE_DS_CPL (1 << 4)
%define CPU_FEATURE_VMX (1 << 5)
%define CPU_FEATURE_SMX (1 << 6)
%define CPU_FEATURE_EST (1 << 7)
%define CPU_FEATURE_TM2 (1 << 8)
%define CPU_FEATURE_SSSE3 (1 << 9)
%define CPU_FEATURE_CID (1 << 10)
%define CPU_FEATURE_SDBG (1 << 11)
%define CPU_FEATURE_FMA (1 << 12)
%define CPU_FEATURE_CX16 (1 << 13)
%define CPU_FEATURE_XTPR (1 << 14)
%define CPU_FEATURE_PDCM (1 << 15)
%define CPU_FEATURE_PCID (1 << 17)
%define CPU_FEATURE_DCA (1 << 18)
%define CPU_FEATURE_SSE4_1 (1 << 19)
%define CPU_FEATURE_SSE4_2 (1 << 20)
%define CPU_FEATURE_X2APIC (1 << 21)
%define CPU_FEATURE_MOVBE (1 << 22)
%define CPU_FEATURE_POPCNT (1 << 23)
%define CPU_FEATURE_AES (1 << 25)
%define CPU_FEATURE_TSC2 (1 << 24)
%define CPU_FEATURE_XSAVE (1 << 26)
%define CPU_FEATURE_OSXSAVE (1 << 27)
%define CPU_FEATURE_AVX (1 << 28)
%define CPU_FEATURE_F16C (1 << 29)
%define CPU_FEATURE_RDRAND (1 << 30)
%define CPU_FEATURE_HYPERVISOR (1 << 31)
%define CPU_FEATURE_FPU (1 << 32)
%define CPU_FEATURE_VME (1 << 33)
%define CPU_FEATURE_DE (1 << 34)
%define CPU_FEATURE_PSE (1 << 35)
%define CPU_FEATURE_TSC (1 << 36)
%define CPU_FEATURE_MSR (1 << 37)
%define CPU_FEATURE_PAE (1 << 38)
%define CPU_FEATURE_MCE (1 << 39)
%define CPU_FEATURE_CX8 (1 << 40)
%define CPU_FEATURE_APIC (1 << 41)
%define CPU_FEATURE_SEP (1 << 42)
%define CPU_FEATURE_MTRR (1 << 43)
%define CPU_FEATURE_PGE (1 << 44)
%define CPU_FEATURE_MCA (1 << 45)
%define CPU_FEATURE_CMOV (1 << 46)
%define CPU_FEATURE_PAT (1 << 47)
%define CPU_FEATURE_PSE36 (1 << 48)
%define CPU_FEATURE_PSN (1 << 49)
%define CPU_FEATURE_CLFLUSH (1 << 50)
%define CPU_FEATURE_DS (1 << 51)
%define CPU_FEATURE_ACPI (1 << 52)
%define CPU_FEATURE_MMX (1 << 53)
%define CPU_FEATURE_FXSR (1 << 54)
%define CPU_FEATURE_SSE (1 << 55)
%define CPU_FEATURE_SSE2 (1 << 56)
%define CPU_FEATURE_SS (1 << 57)
%define CPU_FEATURE_HTT (1 << 58)
%define CPU_FEATURE_TM (1 << 59)
%define CPU_FEATURE_IA64 (1 << 60)
%define CPU_FEATURE_PBE (1 << 61)