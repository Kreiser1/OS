%macro fail_function 2

    fail.%1:

        mov si, fail.%1.msg
        call vga.text.print

        jmp $

    fail.%1.msg: db "[FXEBTLDR] [FAIL] ", %2, 0x0D, 0x0A, 0x00

%endmacro