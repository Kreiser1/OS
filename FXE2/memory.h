#pragma once

#include "std.h"

void mmSet(uintptr_t pointer, uint64_t length, uint8_t value)
{
    for (uint64_t i = 0; i < length; i++)
        *(uint8_t*)(pointer + i) = value;
}

void mmCopy(uintptr_t source, uintptr_t destination, uint64_t length)
{
    for (uint64_t i = 0; i < length; i++)
        *(uint8_t*)(destination + i) = *(uint8_t*)(source + i);
}

void mmMove(uintptr_t source, uintptr_t destination, uint64_t length)
{
    for (uint64_t i = 0; i < length; i++)
    {
        *(uint8_t*)(destination + i) = *(uint8_t*)(source + i);
        *(uint8_t*)(source + i) = 0;
    }
}

bool mmCompare(uintptr_t pointer1, uintptr_t pointer2, uint64_t length)
{
    for (uint64_t i = 0; i < length; i++)
        if (*(uint8_t*)(pointer1 + i) != *(uint8_t*)(pointer2 + i)) return false;
    
    return true;
}

bool mmOverlap(uintptr_t pointer1, uint64_t length1, uintptr_t pointer2, uint64_t length2)
{
    if (pointer1 >= pointer2 && pointer1 < pointer2 + length2)
        return true;

    if (pointer2 >= pointer1 && pointer2 < pointer1 + length1)
        return true;

    return false;
}

void mmInvert(uintptr_t pointer, uint64_t length)
{
    for (uint64_t i = 0; i < length / 2 / 2; i++)
    {
        uint8_t tmp = *(uint8_t*)(pointer + i);
        *(uint8_t*)(pointer + i) = *(uint8_t*)(pointer + length - i - 1);
        *(uint8_t*)(pointer + length - i - 1) = tmp;
    }
}