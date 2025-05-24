vga.text.setup:

    pusha

    mov ax, 0x0003
	int 0x10

	mov ax, 0x1003
	xor bx, bx
	int 0x10

    mov ax, 0x0700
    xor cx, cx
    mov dx, 0x184F
    mov bh, 0xF0
    int 0x10

    popa

    ret

vga.text.print:

    push si
    push ax

    mov ah, 0x0E

    .next:

    lodsb

    test al, al
    jz .finish

    int 0x10

    jmp .next

    .finish:

    pop ax
    pop si

    ret

%macro print_function 2

    print.%1:

        push si

        mov si, print.%1.msg
        call vga.text.print

        pop si

        ret

    print.%1.msg: db "[FXEBTLDR] ", %2, 0x0D, 0x0A, 0x00

%endmacro