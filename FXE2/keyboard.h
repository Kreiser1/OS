#pragma once

#include "std.h"
#include "power.h"
#include "io.h"

#define INTERRUPT_KEYBOARD (uint8_t)33

static char kbMap[128] =
{
    '\0', '\0', '1', '2',
    '3', '4', '5', '6',
    '7', '8', '9', '0',
    '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r',
    't', 'y', 'u', 'i',
    'o', 'p', '[', ']',
    '\r', '\0', 'a', 's',
    'd', 'f', 'g', 'h',
    'j', 'k', 'l', ';',
    '\'', '`', '\0', '\\',
    'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',',
    '.', '/', '\0', '*',
    '\0', ' '
};

static char kbMapShifted[128] =
{
    '\0', '\0', '!', '@',
    '#', '$', '%', '^',
    '&', '*', '(', ')',
    '_', '+', '\b', '\t',
    'Q', 'W', 'E', 'R',
    'T', 'Y', 'U', 'I',
    'O', 'P', '{', '}',
    '\r', '\0', 'A', 'S',
    'D', 'F', 'G', 'H',
    'J', 'K', 'L', ':',
    '\"', '~', '\0', '|',
    'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<',
    '>', '?', '\0', '*',
    '\0', ' '
};

static bool kbKeys[128];

static char* kbMapCurrent = kbMap;

static bool kbKeyPressed = false;
static uint8_t kbCode = 0;
static char kbSymbol = '\0';

void kbUpdate()
{
    kbCode = ioRead8(0x60);

    if (kbCode <= 128)
    {
        if (kbCode == 0x2A) kbMapCurrent = kbMapShifted;

        if (kbMapCurrent[kbCode])
            kbSymbol = kbMapCurrent[kbCode];
        else
            kbSymbol = '\0';

        kbKeyPressed = true;
    }
    else if (kbCode == 0xAA) kbMapCurrent = kbMap;

    if (kbCode <= 128)
        kbKeys[kbCode] = true;
    else
        kbKeys[kbCode - 128] = false;
}

char kbReadSymbol()
{
    while (!kbKeyPressed) halt();
    kbKeyPressed = false;

    return kbMapCurrent[kbCode];
}

bool kbKey(uint8_t code)
{
    if (code <= 128)
        return kbKeys[code];

    return false;
}

uint16_t kbReadString(char* string, uint16_t limit)
{
    uint16_t length = 0;

    while (true)
    {
        char c = kbReadSymbol();

        if (c == '\r' && length < limit)
        {
            string[length] = '\0';
            return length;
        }
        else if (c == '\b' && length) length--;
        else if (length < limit)
        {
            string[length] = c;
            length++;
        }
    }
}