OUTPUT_FORMAT("binary")
OUTPUT_ARCH("i386:x86-64")

SECTIONS
{
    FXE_OFFSET = 0x200000;
    . = FXE_OFFSET;

    .header :
    {
        FXE_SECTION_HEADER = .;
        LONG(0x20455846)
        QUAD(FXE_OFFSET)
        QUAD(END_OF_FILE - FXE_OFFSET)
        QUAD(CODE_OFFSET)
        QUAD(CODE_END - CODE_OFFSET)
        QUAD(DATA_OFFSET)
        QUAD(DATA_END - DATA_OFFSET)
        QUAD(start)
    }

    .text :
    {
        FXE_SECTION_TEXT = .;
        CODE_OFFSET = .;
        *(.text)
        CODE_END = .;
    }

    .data :
    {
        FXE_SECTION_DATA = .;
        DATA_OFFSET = .;
        *(.rodata)
        *(.data)
        *(.bss)
        *(COMMON)
        DATA_END = .;
    }

    END_OF_FILE = .;
}