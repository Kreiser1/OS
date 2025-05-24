#pragma once

#include "std.h"

#define SEGMENT_NULL (uint16_t)0x00
#define SEGMENT_KERNEL_CODE (uint16_t)0x08
#define SEGMENT_KERNEL_DATA (uint16_t)0x10
#define SEGMENT_USER_CODE (uint16_t)0x18
#define SEGMENT_USER_DATA (uint16_t)0x20
#define GLOBAL_DESCRIPTOR_TABLE_LENGTH ((SEGMENT_USER_DATA + sizeof(SEGMENT_DESCRIPTOR)) / sizeof(SEGMENT_DESCRIPTOR))

#define SEGMENT_ACCESSED (uint8_t)1
#define SEGMENT_READ (uint8_t)2
#define SEGMENT_WRITE (uint8_t)2
#define SEGMENT_INVERT_DIRECTION (uint8_t)4
#define SEGMENT_CONFORMING (uint8_t)4
#define SEGMENT_EXECUTABLE (uint8_t)8
#define SEGMENT_NON_SYSTEM (uint8_t)16
#define SEGMENT_USER (uint8_t)96
#define SEGMENT_PRESENT (uint8_t)128

#define SEGMENT_PROTECTED (uint8_t)12
#define SEGMENT_LONG (uint8_t)2

typedef struct
{
	uint64_t Reserved : 40;
	uint8_t Flags;
	uint8_t Reserved2 : 4;
	uint8_t Flags2 : 4;
	uint8_t Reserved3;
} packed aligned(8) SEGMENT_DESCRIPTOR, *PSEGMENT_DESCRIPTOR, *GLOBAL_DESCRIPTOR_TABLE;

typedef struct
{
	uint16_t Limit;
	uint64_t Offset;
} packed aligned(8) GLOBAL_DESCRIPTOR_TABLE_REGISTER, *PGLOBAL_DESCRIPTOR_TABLE_REGISTER;

void flushSegments()
{
	asm(
		"mov %ds, %ax;"
		"mov %ax, %ds;"
		"mov %ax, %es;"
		"mov %ax, %fs;"
		"mov %ax, %gs;"
		"mov %ax, %ss;"
		"mov %cs, %rax;"
		"push %rax;"
		"movabs $next, %rax;"
		"push %rax;"
		"retfq;"
		"next:"
	);
}

void mapSegment(GLOBAL_DESCRIPTOR_TABLE gdt, uint16_t segment, uint8_t flags)
{
	if (segment > SEGMENT_USER_DATA || segment % sizeof(SEGMENT_DESCRIPTOR))
		return;

	gdt[segment / sizeof(SEGMENT_DESCRIPTOR)].Reserved = 0;
	gdt[segment / sizeof(SEGMENT_DESCRIPTOR)].Reserved2 = 0;
	gdt[segment / sizeof(SEGMENT_DESCRIPTOR)].Reserved3 = 0;
	gdt[segment / sizeof(SEGMENT_DESCRIPTOR)].Flags = flags | SEGMENT_PRESENT;
	gdt[segment / sizeof(SEGMENT_DESCRIPTOR)].Flags2 = segment == SEGMENT_KERNEL_CODE || segment == SEGMENT_USER_CODE ? SEGMENT_LONG : 0;
}

void deleteSegment(GLOBAL_DESCRIPTOR_TABLE gdt, uint16_t segment)
{
	gdt[segment / sizeof(SEGMENT_DESCRIPTOR)].Flags &= ~SEGMENT_PRESENT;
}

void loadGdt(GLOBAL_DESCRIPTOR_TABLE gdt)
{
	static GLOBAL_DESCRIPTOR_TABLE_REGISTER gdtr;
	gdtr.Offset = (uint64_t)gdt;
	gdtr.Limit = GLOBAL_DESCRIPTOR_TABLE_LENGTH * sizeof(SEGMENT_DESCRIPTOR) - 1;

	asm("lgdt %0" :: "m" (gdtr));
}