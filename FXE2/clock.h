#pragma once

#include "std.h"
#include "io.h"

typedef struct
{
    uint8_t Seconds, Minutes, Hours, DayOfWeek, Day, Month, Year;
} RTC_TIME, * PRTC_TIME;

void rtcRead(PRTC_TIME time)
{
    ioWrite8(0x70, 0x00);
    time->Seconds = ioRead8(0x71);
    ioWrite8(0x70, 0x02);
    time->Minutes = ioRead8(0x71);
    ioWrite8(0x70, 0x04);
    time->Hours = ioRead8(0x71);
    ioWrite8(0x70, 0x06);
    time->DayOfWeek = ioRead8(0x71);
    ioWrite8(0x70, 0x07);
    time->Day = ioRead8(0x71);
    ioWrite8(0x70, 0x08);
    time->Month = ioRead8(0x71);
    ioWrite8(0x70, 0x09);
    time->Year = ioRead8(0x71);
}

void rtcWrite(RTC_TIME time)
{
    ioWrite8(0x70, 0x00);
    ioWrite8(0x71, time.Seconds);
    ioWrite8(0x70, 0x02);
    ioWrite8(0x71, time.Minutes);
    ioWrite8(0x70, 0x04);
    ioWrite8(0x71, time.Hours);
    ioWrite8(0x70, 0x06);
    ioWrite8(0x71, time.DayOfWeek);
    ioWrite8(0x70, 0x07);
    ioWrite8(0x71, time.Day);
    ioWrite8(0x70, 0x08);
    ioWrite8(0x71, time.Month);
    ioWrite8(0x70, 0x09);
    ioWrite8(0x71, time.Year);
}