#pragma once

#include "std.h"
#include "pit.h"
#include "string.h"

typedef struct
{
    char Symbol;
    uint8_t Color;
} packed VGA_TEXT_CHARACTER, *VGA_TEXT_FRAMEBUFFER;

typedef struct
{
    bool CursorVisible;
    uint8_t Color, Width, Height, PageCount;
    uint16_t CursorPos;
    uint32_t FramePos, Length;
    char* Framebuffer;
} packed TEXT_RENDERER, *PTEXT_RENDERER;

#define VGA_TEXT_COLOR_BLACK (uint8_t)0x00
#define VGA_TEXT_COLOR_BLUE (uint8_t)0x01
#define VGA_TEXT_COLOR_GREEN (uint8_t)0x02
#define VGA_TEXT_COLOR_CYAN (uint8_t)0x03
#define VGA_TEXT_COLOR_RED (uint8_t)0x04
#define VGA_TEXT_COLOR_MAGENTA (uint8_t)0x05
#define VGA_TEXT_COLOR_BROWN (uint8_t)0x06
#define VGA_TEXT_COLOR_LIGHT_GRAY (uint8_t)0x07
#define VGA_TEXT_COLOR_DARK_GRAY (uint8_t)0x08
#define VGA_TEXT_COLOR_LIGHT_BLUE (uint8_t)0x09
#define VGA_TEXT_COLOR_LIGHT_GREEN (uint8_t)0x0A
#define VGA_TEXT_COLOR_LIGHT_CYAN (uint8_t)0x0B
#define VGA_TEXT_COLOR_LIGHT_RED (uint8_t)0x0C
#define VGA_TEXT_COLOR_LIGHT_MAGENTA (uint8_t)0x0D
#define VGA_TEXT_COLOR_YELLOW (uint8_t)0x0E
#define VGA_TEXT_COLOR_WHITE (uint8_t)0x0F

#define VGA_BACKGROUND_COLOR(color) ((uint8_t)color << 4)

#define TEXT_RENDER_FREQUENCY (PIT_TICK_FREQUENCY / 16)

void initializeTextRenderer(PTEXT_RENDERER textRenderer, uint8_t color, uint16_t width, uint16_t height, uint8_t pageCount)
{
    textRenderer->CursorVisible = true;
    textRenderer->FramePos = 0;
    textRenderer->CursorPos = 0;
    textRenderer->Color = color;
    textRenderer->Width = width;
    textRenderer->Height = height;
    textRenderer->PageCount = pageCount;
    textRenderer->Length = width * height * pageCount;
    textRenderer->Framebuffer = (char*)0;
}

void txClear(PTEXT_RENDERER textRenderer)
{
    for (uint32_t i = 0; i < textRenderer->Length; i++)
        textRenderer->Framebuffer[i] = '\0';

    textRenderer->FramePos = 0;
    textRenderer->CursorPos = 0;
}

void txSetColor(PTEXT_RENDERER textRenderer, uint8_t color)
{
    textRenderer->Color = color;
}

void txSetCursorVisiblity(PTEXT_RENDERER textRenderer, bool visible)
{
    textRenderer->CursorVisible = visible;
}

void txRender(PTEXT_RENDERER textRenderer, VGA_TEXT_FRAMEBUFFER framebuffer)
{
    for (uint16_t i = 0; i < textRenderer->Width * textRenderer->Height; i++)
    {
        framebuffer[i].Color = textRenderer->Color;
        framebuffer[i].Symbol = textRenderer->Framebuffer[textRenderer->FramePos + i];
    }

    if (textRenderer->CursorVisible)
    {
        ioWrite8(0x3D4, 0x0F);
        ioWrite8(0x3D5, textRenderer->CursorPos);
        ioWrite8(0x3D4, 0x0E);
        ioWrite8(0x3D5, textRenderer->CursorPos >> 8);
    }
    else
    {
        ioWrite8(0x3D4, 0x0F);
        ioWrite8(0x3D5, 0xFF);
        ioWrite8(0x3D4, 0x0E);
        ioWrite8(0x3D5, 0xFF);
    }
}

bool txPrintSymbol(PTEXT_RENDERER textRenderer, char symbol)
{
    switch (symbol)
    {
    case '\r':
        textRenderer->CursorPos = align(textRenderer->CursorPos, textRenderer->Width) + textRenderer->Width;
        break;
    case '\n':
        textRenderer->CursorPos = align(textRenderer->CursorPos, textRenderer->Width);
        break;
    case '\b':
        if (!textRenderer->CursorPos)
            if (textRenderer->FramePos)
            {
                textRenderer->FramePos -= textRenderer->Width;
                textRenderer->CursorPos = textRenderer->Width;
            }
            else return false;
        textRenderer->CursorPos--;
        textRenderer->Framebuffer[textRenderer->FramePos + textRenderer->CursorPos] = '\0';
    default:
        if (symbol > 0x19 && symbol < 0x7F)
            textRenderer->Framebuffer[textRenderer->FramePos + textRenderer->CursorPos++] = symbol;
        else return false;
        break;
    }

    if (textRenderer->CursorPos >= textRenderer->Width * textRenderer->Height)
    {
        textRenderer->FramePos += textRenderer->Width;
        textRenderer->CursorPos -= textRenderer->Width;
    }

    if (textRenderer->FramePos + (textRenderer->Height - 1) * textRenderer->Width >= textRenderer->Length)
        txClear(textRenderer);

    return true;
}

bool txPrintString(PTEXT_RENDERER textRenderer, const char* string)
{
    bool result = true;

    while (*string)
    {
        if (!txPrintSymbol(textRenderer, *string))
            result = false;

        string++;
    }

    return result;
}

bool txPrint(PTEXT_RENDERER textRenderer, const char* format, ...)
{
    bool result = true;

    va_list args;
    va_start(args, format);

    while (*format)
    {
        if (*format == '$' && *format++)
        {
            uint64_t integer;

            switch (*format)
            {
            case 'c':
                char c = va_arg(args, uint64_t);
                if (!txPrintSymbol(textRenderer, c))
                    result = false;
                break;
            case 's':
                const char* string = va_arg(args, const char*);
                if (!txPrintString(textRenderer, string))
                    result = false;
                break;
            case 'i':
                integer = va_arg(args, uint64_t);
                if (!txPrintString(textRenderer, strFromInteger(integer, INTEGER_DECIMAL)))
                    result = false;
                break;
            case 'p':
                integer = va_arg(args, uint64_t);
                if (!txPrintString(textRenderer, strFromInteger(integer, INTEGER_HEXADECIMAL)))
                    result = false;
                break;
            case 'b':
                bool boolean = va_arg(args, uint64_t);
                if (!txPrintString(textRenderer, boolean ? "true" : "false"))
                    result = false;
                break;
            case 'f':
                integer = va_arg(args, uint64_t);
                if (!txPrintString(textRenderer, strFromInteger(integer, INTEGER_BINARY)))
                    result = false;
                break;
            default:
                if (!txPrintSymbol(textRenderer, *format))
                    result = false;
                break;
            }
        }
        else if (!txPrintSymbol(textRenderer, *format))
            result = false;

        format++;
    }

    va_end(args);

    return result;
}