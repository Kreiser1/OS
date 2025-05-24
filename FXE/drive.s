drive.buffer equ 0x2800

drive.packet:

	db 0x10, 0x00
	.count: dw 0x01
	.offset: dw drive.buffer
	.segment: dw 0x00
	.sector.low: dd 0x00
	.sector.high: dd 0x00

drive.chs.read:

    push ax
    push bx
    push cx
    push dx

    mov dl, al
    mov al, cl
    xchg ch, cl
    xor ch, ch
    xor dh, dh
    mov bx, di
    mov ah, 0x02

    int 0x13

    pop dx
    pop cx
    pop bx
    pop ax

    ret

drive.lba.check:

    push ax
    push bx
    push cx
    push dx

    mov dl, al
    mov ah, 0x41
	mov bx, 0x55AA
	int 0x13

    jc .fail
	
    cmp bx, 0xAA55
    jne .fail
	
    and cx, 0x04
	jz .fail

    pop dx
    pop cx
    pop bx
    pop ax

    clc
    ret

    .fail:

    pop dx
    pop cx
    pop bx
    pop ax

    stc
    ret

drive.lba.read:

    push esi
    push edi
    push eax
    push ecx

    mov dl, al

    push eax
    push ebx
    push edx

    mov eax, ecx
    mov ebx, 0x200
    call memory.align_padded
    xor edx, edx
    div ebx

    mov ecx, eax

    pop edx
    pop ebx
    pop eax

    mov [drive.packet.sector.low], esi
    mov esi, drive.packet

    .next:
    
        mov ah, 0x42
        int 0x13

        jc .fail

        push esi
        push ecx

        mov ecx, 0x0200
        xor esi, esi

        mov esi, drive.buffer

        call memory.copy
        
        call memory.compare
        jc .fail

        pop ecx
        pop esi

        inc dword [drive.packet.sector.low]
        add edi, 0x200

        dec ecx
        jnz .next

    pop ecx
    pop eax
    pop edi
    pop esi

    clc
    ret

    .fail:

    pop ecx
    pop eax
    pop edi
    pop esi

    stc
    ret