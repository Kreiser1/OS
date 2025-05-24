#pragma once

#include "std.h"

void ioWrite8(uint16_t port, uint8_t value)
{
    asm(
        "out %%al, %%dx"
        :: "a" (value), "d" (port)
    );
}

void ioWrite16(uint16_t port, uint16_t value)
{
    asm(
        "out %%ax, %%dx"
        :: "a" (value), "d" (port)
    );
}

void ioWrite32(uint16_t port, uint32_t value)
{
    asm(
        "out %%eax, %%dx"
        :: "a" (value), "d" (port)
    );
}

uint8_t ioRead8(uint16_t port)
{
    uint8_t value;

    asm(
        "in %%dx, %%al"
        : "=a" (value)
        : "d" (port)
    );

    return value;
}

uint16_t ioRead16(uint16_t port)
{
    uint16_t value;

    asm(
        "in %%dx, %%ax"
        : "=a" (value)
        : "d" (port)
    );

    return value;
}

uint32_t ioRead32(uint16_t port)
{
    uint32_t value;

    asm(
        "in %%dx, %%eax"
        : "=a" (value)
        : "d" (port)
    );

    return value;
}