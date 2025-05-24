bits 16
org 0x7C00

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

cli
jmp 0x00:start
dw 0x00
fxebtldr.key:
dq 0x52444C5442455846
incbin "key.bin"
fxebtldr.key.length equ $ - fxebtldr.key

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
    mov bx, 0x8000

    .search:

        mov dl, 0xFF
        sub dl, cl

        push bx
        push cx
        push di

        mov ax, 0x0201
        mov cx, 0x0001

        int 0x13

        mov si, fxebtldr.key
        mov di, 0x8008
        mov cx, fxebtldr.key.length
        repe cmpsb

        pop di
        pop cx
        pop bx

        pop di
        pop cx

        je .found

        loop .search

    .fail:

    mov word [0x472], 0x1234
    int 0x19

    .found:

    mov ax, 0x0200 | 55
    mov cx, 0x0002

    clc

    int 0x13

    jc .fail

    jmp word [0x8000]

times 0x1FE - ($-$$) db 0x00
dw 0xAA55