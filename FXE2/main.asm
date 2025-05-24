bits 16

section .data

    fxebtldr2_header:
        .fxebtldr_signature dq 0x52444C5442455846
        .boot_drive: db 0x00
        .cpu.vendor: dd 0x00, 0x00, 0x00
        .cpu.features: dq 0x00
        .bios_memory_map.offset: dd 0x00
        .bios_memory_map.length: dd 0x00
        .smbios_eps.offset: dd 0x00
        .acpi_rsdp.offset: dd 0x00
        .framebuffer.offset: dd 0x00
        .framebuffer.length: dd 0x00
        .framebuffer.width: dw 0x00
        .framebuffer.height: dw 0x00
        .framebuffer.pixel_length: db 0x00
        .gdt.offset: dd 0x00
        .gdt.length: dd 0x00
        .idt.offset: dd 0x00
        .idt.length: dd 0x00
        .page_map.offset: dd 0x00
        .page_map.length: dd 0x00
        .kernel.offset: dd 0x00
        .kernel.length: dd 0x00

; BSD 3-Clause License

; Copyright (c) 2025, Syrtsov Vadim

; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:

; 1. Redistributions of source code must retain the above copyright notice, this
;    list of conditions and the following disclaimer.

; 2. Redistributions in binary form must reproduce the above copyright notice,
;    this list of conditions and the following disclaimer in the documentation
;    and/or other materials provided with the distribution.

; 3. Neither the name of the copyright holder nor the names of its
;    contributors may be used to endorse or promote products derived from
;    this software without specific prior written permission.

; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
; FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
; DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
; SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
; CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
; OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
; OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

incbin "license.txt"

section .text

    extern main

    global start
    start:

        cli

        xor al, al
        not al

        out 0x21, al
        out 0xA1, al

        in al, 0x70
        or al, 0x80
        out 0x70, al

        mov [fxebtldr2_header.boot_drive], dl

        push sp
        pop ax

        cmp ax,sp
        jne .hang

        smsw ax
        cmp ax, 0xFFF0
        jae .hang

        pushfd
        mov dword [esp], 0x00
        popfd

        sti

        call main

        .hang:

        hlt
        jmp $ - 1