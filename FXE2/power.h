#pragma once

#include "std.h"
#include "io.h"

void powerReset()
{
    ioWrite8(0xCF9, 0x04);
    ioWrite8(0xCF9, 0xFF);
}

void powerOff()
{
    ioWrite16(0xB004, 0x2000);
    ioWrite16(0x604, 0x2000);
    ioWrite16(0x4004, 0x3400);
}

void halt()
{
    asm("hlt");
}

void hang()
{
    asm("hlt; jmp . - 1");
}