#pragma once

#include "std.h"

#define INTEGER_BINARY (uint8_t)2
#define INTEGER_DECIMAL (uint8_t)10
#define INTEGER_HEXADECIMAL (uint8_t)16

uint32_t strLength(const char* string)
{
    uint32_t length = 0;
    while (*string++) ++length;
    return length;
}

bool strCompare(const char* string1, const char* string2)
{
    if (strLength(string1) != strLength(string2))
    {
        return false;
    }
    else
    {
        for (uint32_t i = 0; i < strLength(string1); i++)
            if (string1[i] != string2[i])
            {
                return false;
            }
    }

    return true;
} 

uint32_t strCopy(const char* source, char* destination)
{
    uint32_t i = 0;

    for (; i < strLength(source) + 1; i++)
        destination[i] = source[i];

    return i;
}

uint32_t strExpand(char* string1, const char* string2)
{
    return strCopy(string2, &string1[strLength(string1)]);
}

uint32_t strToInteger(const char* string, uint8_t base)
{
    uint32_t integer = 0;

    while (*string)
    {
        integer = integer * base + (*string - '0');
        string++;
    }

    return integer;
}

const char* strFromInteger(uint32_t integer, uint8_t base)
{
    if (base < 2 || base > 16) return (char*)0;

    static char converted[sizeof(uint32_t) * 8 + 1];
    uint8_t i = 1;

    for (; i < sizeof(uint32_t) * 8 + 1; i++)
    {
        converted[sizeof(uint32_t) * 8 + 1 - i] = "FEDCBA9876543210123456789ABCDEF"[15 + integer % base];
        integer /= base;
        if (!integer) break;
    }

    converted[sizeof(uint32_t) * 8 + 1] = '\0';

    return &converted[sizeof(uint32_t) * 8 + 1 - i];
}