#pragma once

#include "std.h"
#include "string.h"

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

typedef struct
{
	uint32_t Signature;
	uint16_t Version;
	uint32_t VendorOffset;
	uint32_t Capatiblities;
	uint32_t VideoModeOffset;
	uint16_t MemoryBlockCount;
	uint8_t Reserved[492];
} packed VBE_INFO_BLOCK, *PVBE_INFO_BLOCK;

typedef struct
{
	uint8_t Reserved[18];
	uint16_t Width;
	uint16_t Height;
	uint8_t Reserved2[3];
	uint8_t BytesPerPixel;
	uint8_t Reserved3[14];
	uint32_t FramebufferOffset;
	uint8_t reserved4[212];
} packed VBE_MODE_INFO_BLOCK, *PVBE_MODE_INFO_BLOCK;

bool vgaSetupText2()
{
	VBE_INFO_BLOCK vbeInfoBlock;

	uint16_t ax;

	asm(
		"mov %%es, %%ax;"
		"push %%ax;"
		"mov %%edi, %%eax;"
		"shr $20, %%eax;"
		"mov %%ax, %%es;"
		"mov $0x4F00, %%ax;"
		"int $0x10;"
		"pop %%ax;"
		"mov %%ax, %%es;"
		: "=a"(ax)
		: "D"((uint32_t)&vbeInfoBlock));

	if (ax != 0x4F)
		return false;

	return true;
}

void vgaSetupText(uint8_t color)
{
	asm(
		"mov $0x03, %ax;"
		"int $0x10;"
		"mov $0x1003, %ax;"
		"xor %bx, %bx;"
		"int $0x10;");

	asm(
		"mov %0, %%bh;"
		"mov $0x700, %%ax;"
		"xor %%cx, %%cx;"
		"mov $0x184F, %%dx;"
		"int $0x10;" ::"r"(color));
}

void vgaPrintSymbol(char symbol)
{
	asm(
		"mov $0x0E, %%ah;"
		"int $0x10;" ::"a"(symbol));
}

void vgaPrintString(const char *string)
{
	while (*string)
	{
		vgaPrintSymbol(*string);
		string++;
	}
}

void vgaPrint(const char *format, ...)
{
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
				char c = va_arg(args, uint32_t);
				vgaPrintSymbol(c);
				break;
			case 's':
				const char *string = va_arg(args, const char *);
				vgaPrintString(string);
				break;
			case 'i':
				integer = va_arg(args, uint32_t);
				vgaPrintString(strFromInteger(integer, INTEGER_DECIMAL));
				break;
			case 'p':
				integer = va_arg(args, uint32_t);
				vgaPrintString(strFromInteger(integer, INTEGER_HEXADECIMAL));
				break;
			case 'b':
				bool boolean = va_arg(args, uint32_t);
				vgaPrintString(boolean ? "true" : "false");
				break;
			case 'f':
				integer = va_arg(args, uint32_t);
				vgaPrintString(strFromInteger(integer, INTEGER_BINARY));
				break;
			default:
				vgaPrintSymbol(*format);
				break;
			}
		}
		else
			vgaPrintSymbol(*format);

		format++;
	}

	va_end(args);
}