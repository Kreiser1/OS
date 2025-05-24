OUTPUT_FORMAT("binary")
OUTPUT_ARCH("i386")

SECTIONS
{
    FXEBTLDR_OFFSET = 0x8000;
    . = FXEBTLDR_OFFSET;

    .header :
    {
        FXEBTLDR_SECTION_HEADER = .;
        SHORT(start)
    }

    .text :
    {
        FXEBTLDR_SECTION_TEXT = .;
        CODE_OFFSET = .;
        *(.text)
        CODE_END = .;
    }

    .data :
    {
        FXEBTLDR_SECTION_DATA = .;
        DATA_OFFSET = .;
        *(.rodata)
        *(.data)
        *(.bss)
        *(COMMON)
        DATA_END = .;
    }

    . = FXEBTLDR_OFFSET + 28160;
    END_OF_FILE = .;
}