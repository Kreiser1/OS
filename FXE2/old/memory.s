memory.a20.enable:

    push ax

    mov ax, 0x2403
    int 0x15

    jb .next

    test ah, ah
    jnz .next

    mov ax, 0x2402
    int 0x15

    jb .next

    test ah, ah
    jnz .next

    test al, al
    jz .skip

    mov ax, 0x2401
    int 0x15

    jb .next

    test ah, ah
    jnz .next

    .next:

    in al, 0x92
    or al, 0x02
    out 0x92, al

    in al, 0xEE

    .skip:

    cli

    call .wait

    mov al, 0xAD
    out 0x64, al

    call .wait
    
    mov al, 0xD0
    out 0x64, al

    call .wait2

    in al, 0x60

    push eax

    call .wait

    mov al, 0xD1
    out 0x64, al

    call .wait

    pop eax
    
    or al, 2
    out 0x60, al

    call .wait

    mov al, 0xAE
    out 0x64, al

    call .wait
    
    pop ax

    sti

    ret

.wait:

    in al, 0x64
    test al, 2
    jnz .wait
    ret

.wait2:

    in al, 0x64
    test al, 1
    jz .wait2
    ret

memory.bios_map.get:

    push eax
    push ebx
    push ecx
    push edx
    push edi

    xor esi, esi
    xor ebx, ebx
    mov edx, 0x534D4150

    .next:

        mov eax, 0xE820
        mov ecx, 0x18
        int 0x15

        jc .fail

        test ebx, ebx
        jz .finish

        add edi, 0x18
        inc esi

        jmp .next

    .finish:

    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax

    clc
    ret

    .fail:

    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax

    stc
    ret

memory.smbios_eps.lookup:

    push ecx

    mov esi, 0x80000
    mov ecx, 0x8000

    .search:

        cmp dword [esi], "_SM3"
        jne .next

        add esi, 0x04
        cmp byte [esi], "_"
        jne .next

        jmp .finish

        .next:

        add esi, 0x10

        loop .search

        jmp .fail

    .fail:

    xor esi, esi

    .finish:

    pop ecx

    ret

memory.acpi_rsdp.lookup:

    push ecx

    mov esi, 0xE0000
    mov ecx, 0x2000

    .search:

        cmp dword [esi], "RSD "
        jne .next

        add esi, 0x04
        cmp dword [esi], "PTR "
        jne .next

        jmp .finish

        .next:

        add esi, 0x10

        loop .search

        jmp .fail

    .fail:

    xor esi, esi

    .finish:

    pop ecx
    
    ret

memory.copy:

    push eax
    push ecx
    push esi
    push edi

    .next:

        mov al, [esi]
        mov [edi], al

        inc esi
        inc edi

        dec ecx
        jnz .next

    pop edi
    pop esi
    pop ecx
    pop eax

    ret

memory.compare:

    push eax
    push ecx
    push esi
    push edi

    .next:

        mov al, [esi]
        cmp [edi], al

        jne .not_equal

        inc esi
        inc edi

        dec ecx
        jnz .next

    pop edi
    pop esi
    pop ecx
    pop eax

    clc
    ret

    .not_equal:

    pop edi
    pop esi
    pop ecx
    pop eax

    stc
    ret

memory.clear:

    push eax
    push ecx
    push edi

    xor al, al

    .next:

        mov [edi], al
        inc edi

        dec ecx
        jnz .next

    pop edi
    pop ecx
    pop eax

    ret

memory.ensure:

    push esi
    push eax
    push ecx
    push ebx

    add ebx, edi
    
    .search:

        cmp dword [esi+16], 0x01
        jne .next

        cmp dword [esi+4], 0x00
        jnz .next

        mov eax, [esi]
        cmp eax, edi
        ja .next

        cmp dword [esi+12], 0x00
        jnz .finish

        add eax, [esi+8]
        cmp ebx, eax
        jle .finish

        .next:

        add esi, 0x18

        loop .search

    pop ebx
    pop ecx
    pop eax
    pop esi

    stc
    ret

    .finish:

    pop ebx
    pop ecx
    pop eax
    pop esi

    clc
    ret

memory.paging.setup:

    push ebx

    sub edi, 0x2000

    push ecx

    mov ecx, 0x6000
    call memory.clear

    pop ecx

    mov ebx, edi
    add ebx, 0x1000

    mov [edi], ebx
    or byte [edi], 0x03

    mov edi, ebx
    add ebx, 0x1000

    push ebx

    mov [edi], ebx
    or byte [edi], 0x03

    add edi, 0x08
    add ebx, 0x1000

    mov [edi], ebx
    or byte [edi], 0x03

    add edi, 0x08
    add ebx, 0x1000

    mov [edi], ebx
    or byte [edi], 0x03

    add edi, 0x08
    add ebx, 0x1000

    mov [edi], ebx
    or byte [edi], 0x03

    pop ebx

    mov edi, ebx

    pop ebx

    ret

memory.align:

    push edx
    
    xor edx, edx
    div ebx

    xor edx, edx
    mul ebx

    pop edx

    ret

memory.align_padded:

    push edx
    
    xor edx, edx
    div ebx

    test edx, edx
    jz .no_padding

    inc eax

    .no_padding:

    xor edx, edx
    mul ebx

    pop edx

    ret

memory.paging.map:

    push edi
    push esi
    push eax
    push ebx
    push ecx
    push edx

    push ebx

    xor edx, edx
    mov ebx, 0x200000
    mov eax, ecx
    call memory.align_padded

    div ebx

    mov ecx, eax

    xor edx, edx
    mov ebx, 0x200000
    mov eax, esi
    call memory.align

    mov esi, eax

    xor edx, edx
    mov ebx, 0x200000
    mov eax, edi
    call memory.align

    div ebx

    mov ebx, 0x08
    mul ebx

    mov edi, eax

    pop ebx

    add ebx, edi

    .next:

        mov eax, esi
        or eax, 0x283
        mov [ebx], eax

        add esi, 0x200000
        add ebx, 0x08

        loop .next

    pop edx
    pop ecx
    pop ebx
    pop eax
    pop esi
    pop edi

    ret