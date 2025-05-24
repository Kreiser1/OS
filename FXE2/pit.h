#pragma once

#include "io.h"
#include "power.h"

#define PIT_TICK_FREQUENCY (uint32_t)1024
#define INTERRUPT_PIT (uint8_t)32

uint64_t pitTickCount = 0;

void initializePit()
{
    uint16_t divisor = 1193182 / PIT_TICK_FREQUENCY;

    ioWrite8(0x43, 0x36);
    ioWrite8(0x40, divisor & 0xff);
    ioWrite8(0x40, divisor >> 8);

    pitTickCount = 0;
}

void pitTick()
{
    pitTickCount++;
}

uint64_t pitGetTickCount()
{
    return pitTickCount;
}

void pitSleep(uint64_t ms)
{
    uint64_t now = pitTickCount;
    while (pitTickCount < (now + ms * PIT_TICK_FREQUENCY / 1000)) halt();
}