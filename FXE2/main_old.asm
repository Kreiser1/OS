bits 16
org 0x7C00

cli
jmp 0x00:start
dw 0x00
fxebtldr_key:
dq 0x52444C5442455846
incbin "key.bin"
fxebtldr_key.length equ $ - fxebtldr_key

%include "drive.s"

start:
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    mov bp, sp

    pushf
    mov bx, sp
    mov word [bx], 0x00
    popf

    sti

    mov cl, 0x7F
    mov di, fxebtldr_header

    .boot_drive_search:

        mov al, 0xFF
        sub al, cl

        push cx
        push di

        mov cx, 0x020F
        call drive.chs.read

        mov si, fxebtldr_key
        mov di, fxebtldr_header.key
        mov cx, fxebtldr_key.length
        repe cmpsb

        pop di
        pop cx

        je fxebtldr_header

        loop .boot_drive_search

    int 0x18

times 0x1FE - ($-$$) db 0x00
dw 0xAA55

fxebtldr_header:
jmp main
dd 0x00
db 0x00
.key:
dq 0x52444C5442455846
incbin "key.bin"
.boot_drive: db 0x00
.cpu.vendor: dd 0x00, 0x00, 0x00
.cpu.features: dq 0x00
.bios_memory_map.offset: dd 0xF000
.bios_memory_map.length: dd 0x00
.smbios_eps.offset: dd 0x00
.acpi_rsdp.offset: dd 0x00
.framebuffer.offset: dd 0xB8000
.framebuffer.length: dd 0xFA0
.framebuffer.width: dw 0x50
.framebuffer.height: dw 0x19
.framebuffer.pixel_length: db 0x02
.gdt.offset: dd cpu.gdt.long_mode
.gdt.length: dd cpu.gdt.long_mode.register - cpu.gdt.long_mode
.idt.offset: dd cpu.idt
.idt.length: dd cpu.idt.register - cpu.idt
.page_map.offset: dd 0xB000
.page_map.length: dd 0x4000
.kernel.offset: dd 0x1000000
.kernel.length: dd 0x00

fxe_header equ 0x2000
fxe_header.signature equ fxe_header
fxe_header.offset equ fxe_header+4
fxe_header.length equ fxe_header+12
fxe_header.code.offset equ fxe_header+20
fxe_header.code.length equ fxe_header+28
fxe_header.data.offset equ fxe_header+36
fxe_header.data.length equ fxe_header+44
fxe_header.entry.offset equ fxe_header+52

%include "cpu.s"
%include "memory.s"
%include "vga.s"
%include "fail.s"

print_function startup, "Startup."
print_function cpu, "CPU is supported."
print_function drive, "Drive extensions supported."
print_function cpu_information, "CPU information obtained."
print_function extended_memory, "Extended memory enabled."
print_function memory_map, "Memory map obtained."
print_function smbios, "SMBIOS structures found."
print_function acpi, "ACPI structures found."
print_function kernel_verified, "Kernel verified."
print_function kernel_read, "Kernel read."
print_function memory_layout, "Memory layout initialized."
print_function long_mode, "Switched CPU to 64-bit mode."
print_function kernel_launch, "Launching the kernel..."

fail_function cpu_unsupported, "CPU unsupported."
fail_function bios_unsupported, "BIOS unsupported."
fail_function drive_unsupported, "Drive unsupported."
fail_function memory_unsupported, "Memory layout unsupported."
fail_function invalid_kernel, "Kernel is invalid or missing."
fail_function kernel_not_read, "Failed to read kernel."

main:

    call cpu.pic.disable
    call cpu.nmi.disable

    call vga.text.setup

    call print.startup

    mov [fxebtldr_header.boot_drive], al

    call cpu.check
    jc fail.cpu_unsupported

    call print.cpu

    call drive.lba.check
    jc fail.drive_unsupported

    call print.drive

    mov edi, fxebtldr_header.cpu.vendor  
    call cpu.vendor.get

    mov edi, fxebtldr_header.cpu.features  
    call cpu.features.get

    call print.cpu_information

    call cpu.mode.enter_unreal

    call memory.a20.enable

    call print.extended_memory

    mov edi, [fxebtldr_header.bios_memory_map.offset]
    call memory.bios_map.get

    jc fail.bios_unsupported

    mov [fxebtldr_header.bios_memory_map.length], esi

    call print.memory_map

    call memory.smbios_eps.lookup
    
    mov [fxebtldr_header.smbios_eps.offset], esi
    
    test esi, esi
    jz .no_smbios_eps

    call print.smbios

    .no_smbios_eps:

    call memory.acpi_rsdp.lookup
    
    mov [fxebtldr_header.acpi_rsdp.offset], esi
    
    test esi, esi
    jz .no_smbios_eps

    call print.acpi

    .no_acpi_rsdp:

    mov cx, 0x1101
    mov di, fxe_header
    mov al, [fxebtldr_header.boot_drive]

    call drive.chs.read

    cmp dword [fxe_header.signature], 0x20455846
    jne fail.invalid_kernel

    mov eax, [fxe_header.offset]

    cmp eax, 0x200000
    jnae fail.invalid_kernel

    xor edx, edx
    mov ebx, 0x200000
    div ebx

    test edx, edx
    jnz fail.invalid_kernel

    mov eax, [fxe_header.offset]
    mov ebx, [fxe_header.code.offset]

    cmp ebx, eax
    jl fail.invalid_kernel

    mov ebx, [fxe_header.data.offset]

    cmp ebx, eax
    jl fail.invalid_kernel

    mov ebx, [fxe_header.entry.offset]

    cmp ebx, eax
    jl fail.invalid_kernel

    mov ebx, [fxe_header.length]

    push ebx
    push eax

    mov eax, ebx
    mov ebx, 0x200000
    call memory.align_padded

    mov ebx, eax
    
    mov [fxebtldr_header.kernel.length], ebx

    pop eax
    pop ebx

    add eax, ebx

    mov ebx, [fxe_header.code.offset]

    cmp ebx, eax
    jae fail.invalid_kernel

    add ebx, [fxe_header.code.length]

    cmp ebx, eax
    ja fail.invalid_kernel

    mov ebx, [fxe_header.data.offset]

    cmp ebx, eax
    jae fail.invalid_kernel

    add ebx, [fxe_header.data.length]

    cmp ebx, eax
    ja fail.invalid_kernel

    mov ebx, [fxe_header.entry.offset]

    cmp ebx, eax
    jae fail.invalid_kernel

    call print.kernel_verified

    mov edi, [fxebtldr_header.kernel.offset]
    mov ebx, [fxebtldr_header.kernel.length]
    mov esi, [fxebtldr_header.bios_memory_map.offset]
    mov ecx, [fxebtldr_header.bios_memory_map.length]

    call memory.ensure

    jc fail.memory_unsupported

    call memory.clear

    mov esi, 0x0010
    mov ecx, [fxe_header.length]
    mov al, [fxebtldr_header.boot_drive]

    call drive.lba.read

    jc fail.kernel_not_read

    call print.kernel_read

    mov edi, [fxebtldr_header.page_map.offset]

    call memory.paging.setup

    mov ebx, edi

    xor edi, edi
    xor esi, esi
    mov ecx, 0x100000

    call memory.paging.map

    mov edi, [fxe_header.offset]
    mov esi, [fxebtldr_header.kernel.offset]
    mov ecx, [fxebtldr_header.kernel.length]

    call memory.paging.map

    sub ebx, 0x2000
    mov cr3, ebx

    call print.memory_layout
    call print.long_mode
    call print.kernel_launch

    push dword 0x00
    push dword main64

    jmp cpu.mode.enter_long

bits 64

interrupt:

    mov rax, 0x52444C5442455846
    pop rbx
    pop rcx
    pop rdx

    hlt
    jmp $ - 1

main64:
    
    mov rdi, fxebtldr_header
    mov rsi, 0x52444C5442455846

    xor rax, rax
    mov rsp, rax
    mov rbp, rsp

    xor rax, rax
    xor rbx, rbx
    xor rcx, rcx
    xor rdx, rdx

    sti

    jmp [fxe_header.entry.offset]

times 0x2000 - ($-$$) db 0x00
end: