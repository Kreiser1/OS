section .text

extern interrupt
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

%macro isr_error_stub 1
isr_stub%1:
    cli
    push %1
    jmp interrupt_stub
%endmacro

%macro isr_no_error_stub 1
isr_stub%1:
    cli
    push 0x00
    push %1
    jmp interrupt_stub
%endmacro

%macro irq_stub 1
irq_stub%1:
    cli
    push 0x00
    push %1 + 0x20
    jmp interrupt_stub
%endmacro

interrupt_stub:

    push rdi
    push rsi
    push rbp
    push rbx
    push rdx
    push rcx
    push rax

    mov ax, ds
    push rax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    call interrupt

    pop rax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    pop rax
    pop rcx
    pop rdx
    pop rbx
    pop rbp
    pop rsi
    pop rdi

    add rsp, 0x10

    iretq

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

irq_stub 0
irq_stub 1
irq_stub 2
irq_stub 3
irq_stub 4
irq_stub 5
irq_stub 6
irq_stub 7
irq_stub 8
irq_stub 9
irq_stub 10
irq_stub 11
irq_stub 12
irq_stub 13
irq_stub 14
irq_stub 15

isr_syscall:
    cli
    push 0x00
    push 0xFF
    jmp interrupt_stub

section .data

global interrupt_offset_table
interrupt_offset_table:
    %assign i 0
    %rep 32
        dq isr_stub%+i
    %assign i i+1
    %endrep
    %assign i 0
    %rep 16
        dq irq_stub%+i
    %assign i i+1
    %endrep
    dq isr_syscall

align 16
resb 65536
stack_offset: