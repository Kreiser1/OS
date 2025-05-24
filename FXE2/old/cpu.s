align 0x08
cpu.gdt.unreal_mode:

    dq 0x00
	db 0xFF, 0xFF, 0x00, 0x00, 0x00, 10011010b, 0x00, 0x00
	db 0xFF, 0xFF, 0x00, 0x00, 0x00, 10010010b, 11001111b, 0x00

	.register:

		dw cpu.gdt.unreal_mode.register - cpu.gdt.unreal_mode - 1
		dd cpu.gdt.unreal_mode

align 0x08
cpu.gdt.long_mode:

    dq 0x00
    dq 0x00209A0000000000
    dq 0x0000920000000000

    .register:

        dw cpu.gdt.long_mode.register - cpu.gdt.long_mode - 1
        dq cpu.gdt.long_mode

%macro isr_error_stub 1
isr_stub_%1:
    cli
    push %1
    jmp interrupt
%endmacro

%macro isr_no_error_stub 1
isr_stub_%1:
    cli
    push 0x00
    push %1
    jmp interrupt
%endmacro

bits 64

isr_no_error_stub 0
isr_no_error_stub 1
isr_no_error_stub 2
isr_no_error_stub 3
isr_no_error_stub 4
isr_no_error_stub 5
isr_no_error_stub 6
isr_no_error_stub 7
isr_error_stub 8
isr_no_error_stub 9
isr_error_stub 10
isr_error_stub 11
isr_error_stub 12
isr_error_stub 13
isr_error_stub 14
isr_no_error_stub 15
isr_no_error_stub 16
isr_error_stub 17
isr_no_error_stub 18
isr_no_error_stub 19
isr_no_error_stub 20
isr_no_error_stub 21
isr_no_error_stub 22
isr_no_error_stub 23
isr_no_error_stub 24
isr_no_error_stub 25
isr_no_error_stub 26
isr_no_error_stub 27
isr_no_error_stub 28
isr_no_error_stub 29
isr_error_stub 30
isr_no_error_stub 31

bits 16

align 0x08
cpu.idt:
    %assign i 0
    %rep 32
        dw isr_stub_%+i
        dw 0x08
        db 0x00
        db 0x8F
        dd 0x00
        dw 0x00
        dd 0x00
    %assign i i+1
    %endrep

    .register:

        dw cpu.idt.register - cpu.idt - 1
        dq cpu.idt

cpu.check:

    push bx
    push dx

    pushf
    pushf

    pop dx
    xor dh, 0x40

    push dx
    popf

    pushf
    pop bx
    popf

    cmp dx, bx
    jne .fail1

    pop dx
    pop bx

    push eax
    push ecx
    push edx

    pushfd
    pop eax

    mov ecx, eax

    xor eax, 0x200000

    push eax
    popfd

    push ecx
    popfd

    xor eax, ecx
    jz .fail2

    mov eax, 0x80000000
    cpuid

    cmp eax, 0x80000001
    jb .fail2

    mov eax, 0x80000001
    cpuid

    test edx, 0x20000000
    jz .fail2

    pop edx
    pop ecx
    pop eax

    clc
    ret

    .fail1:

    pop dx
    pop bx

    stc
    ret

    .fail2:

    pop edx
    pop ecx
    pop eax

    stc
    ret

cpu.features.get:

    push eax
    push ebx
    push ecx
    push edx
    
    mov eax, 0x0001
    cpuid

    mov [edi], edx
    mov [edi+4], ecx

    pop edx
    pop ebx
    pop ecx
    pop edx

    ret

cpu.vendor.get:

    push eax
    push ebx
    push ecx
    push edx

    xor eax, eax
    cpuid

    mov [edi], ebx
    mov [edi+4], ebx
    mov [edi+8], ecx

    pop edx
    pop ebx
    pop ecx
    pop edx

    ret

cpu.mode.enter_unreal:

    push eax
    push ebx

    cli

    push ds

    lgdt [cpu.gdt.unreal_mode.register]

    mov eax, 0x0001
    mov cr0, eax

    jmp 0x08:.flush_segments

    .flush_segments:

    mov bx, 0x10
    mov ds, bx
	mov es, bx
	mov ss, bx
	mov fs, bx
	mov gs, bx
    xor eax, eax
	mov cr0, eax

    jmp 0x00:.restore_segments

    .restore_segments:

	pop ds
	mov ax, ds
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax

	sti

    pop ebx
    pop eax

    ret

cpu.mode.enter_long:

    push dword 0x00
    push eax
    push dword 0x00
    push ebx
    push dword 0x00
    push ecx

    mov ax, 0xEC00
    mov bl, 0x02
    int 0x15

    cli

    mov eax, 0x34
    mov cr4, eax

    mov ecx, 0xC0000080
    rdmsr

    or eax, 0x100
    wrmsr

    lgdt [cpu.gdt.long_mode.register]

    mov eax, 0x80050003
    mov cr0, eax

    jmp 0x08:.flush_segments

bits 64

    .flush_segments:

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    lidt [cpu.idt.register]

    pop rcx
    pop rbx
    pop rax

    o16 ret

bits 16

cpu.pic.disable:

    push ax

    xor al, al
    not al

    out 0x21, al
    out 0xA1, al

    pop ax

    ret

cpu.nmi.disable:

    push ax

    in al, 0x70
    or al, 0x80
    out 0x70, al

    pop ax

    ret