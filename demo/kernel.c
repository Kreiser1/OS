void print_string(const char* str, unsigned char color)
{
    char* textPtr = (char*)0xB8000;
    
    while (*str)
    {
        *textPtr++ = *str++;
        *textPtr++ = color;
    }
}

void clear_screen(unsigned char color)
{
    char* textPtr = (char*)0xB8000;
    
    while (textPtr < 0xB8FA0)
    {
        *textPtr++ = '\0';
        *textPtr++ = color;
    }
}

void ioWrite8(unsigned short port, unsigned char value)
{
    __asm__ volatile(
        "out %%al, %%dx"
        :: "a" (value), "d" (port)
    );
}

void vga_hide_cursor()
{
    ioWrite8(0x3D4, 0x0F);
    ioWrite8(0x3D5, 0xFF);
    ioWrite8(0x3D4, 0x0E);
    ioWrite8(0x3D5, 0xFF);
}

void main(void* fxebtldr)
{
    clear_screen(0x0E);
    vga_hide_cursor();
    print_string("Hello, world!", 0xE0);
}