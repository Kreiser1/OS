section .text

    extern main

    global start
    start:
        mov rax, 0x52444C5442455846
        cmp rsi, rax
        jne $

        cli

        mov rsp, stack_offset
        mov rbp, rsp

        sti

        xor rax, rax
        xor rsi, rsi

        call main

        hlt
        jmp $ - 1

section .data

    align 16
    resb 65536
    stack_offset: