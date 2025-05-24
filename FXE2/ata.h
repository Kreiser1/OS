#pragma once

#include "std.h"
#include "io.h"
#include "pit.h"
#include "power.h"

#define ATA_PRIMARY_1 (uint16_t)0x1F0
#define ATA_PRIMARY_2 (uint16_t)0x170
#define ATA_SECONDARY_1 (uint16_t)0x1E8
#define ATA_SECONDARY_2 (uint16_t)0x168

#define ATA_BSY (uint8_t)128
#define ATA_RDY (uint8_t)64
#define ATA_DRQ (uint8_t)8
#define ATA_DF (uint8_t)32
#define ATA_ERR (uint8_t)1

#define ATA_WAIT_TIME (PIT_TICK_FREQUENCY * 5)

#define ATA_SECTOR_LENGTH (uint16_t)512

#define ATA_MAX_SECTOR (uint64_t)0x1000000000000ull

#define ATA_DRIVE_1 (ATA_DRIVE)1
#define ATA_DRIVE_2 (ATA_DRIVE)2
#define ATA_DRIVE_3 (ATA_DRIVE)3
#define ATA_DRIVE_4 (ATA_DRIVE)4
#define ATA_DRIVE_5 (ATA_DRIVE)5
#define ATA_DRIVE_6 (ATA_DRIVE)6
#define ATA_DRIVE_7 (ATA_DRIVE)7
#define ATA_DRIVE_8 (ATA_DRIVE)8
#define ATA_MAX_DRIVE ATA_DRIVE_8

typedef uint8_t ATA_DRIVE;
typedef uint64_t ATA_BUS;

bool ataWaitBsy(ATA_BUS bus)
{
	uint64_t now = pitGetTickCount();

	while (ioRead8(bus + 7) & ATA_BSY)
	{
		if (pitGetTickCount() > (now + ATA_WAIT_TIME)) return false;
		halt();
	}

	return true;
}

bool ataWaitDrq(ATA_BUS bus)
{
	uint64_t now = pitGetTickCount();

	while (!(ioRead8(bus + 7) & ATA_DRQ))
	{
		if (pitGetTickCount() > (now + ATA_WAIT_TIME)) return false;
		halt();
	}

	return true;
}

bool ataReadSector(ATA_DRIVE drive, uint64_t offset, uintptr_t buffer)
{
	uint64_t sectorOffset = aligned_count(offset, ATA_SECTOR_LENGTH);

	if (!drive || drive > ATA_MAX_DRIVE || sectorOffset >= ATA_MAX_SECTOR)
		return false;

	drive--;

	ATA_BUS bus;

	switch (drive & ~1)
	{
	case 0:
		bus = ATA_PRIMARY_1;
		break;
	case 2:
		bus = ATA_PRIMARY_2;
		break;
	case 4:
		bus = ATA_SECONDARY_1;
		break;
	case 6:
		bus = ATA_SECONDARY_2;
		break;
	default:
		return false;
		break;
	}

	bool slave = drive & 1;

	if (!ataWaitBsy(bus)) return false;

	ioWrite8(bus + 6, 0x40 | (slave << 4));
	ioWrite8(bus + 2, 0);
	ioWrite8(bus + 3, sectorOffset >> 24);
	ioWrite8(bus + 4, sectorOffset >> 32);
	ioWrite8(bus + 5, sectorOffset >> 40);
	ioWrite8(bus + 2, 1);
	ioWrite8(bus + 3, sectorOffset);
	ioWrite8(bus + 4, sectorOffset >> 8);
	ioWrite8(bus + 5, sectorOffset >> 16);
	ioWrite8(bus + 7, 0x24);

	if (!ataWaitBsy(bus)) return false;
	if (!ataWaitDrq(bus)) return false;

	uint16_t* target = (uint16_t*)buffer;

	for (uint16_t j = 0; j < aligned_count(ATA_SECTOR_LENGTH, sizeof(uint16_t)); j++) target[j] = ioRead16(bus);

	return !(ioRead8(bus + 7) & ATA_ERR);
}

bool ataWriteSector(ATA_DRIVE drive, uint64_t offset, uintptr_t buffer)
{
	uint64_t sectorOffset = aligned_count(offset, ATA_SECTOR_LENGTH);

	if (!drive || drive > ATA_MAX_DRIVE || sectorOffset >= ATA_MAX_SECTOR)
		return false;

	drive--;

	ATA_BUS bus;

	switch (drive & ~1)
	{
	case 0:
		bus = ATA_PRIMARY_1;
		break;
	case 2:
		bus = ATA_PRIMARY_2;
		break;
	case 4:
		bus = ATA_SECONDARY_1;
		break;
	case 6:
		bus = ATA_SECONDARY_2;
		break;
	default:
		return false;
		break;
	}

	bool slave = drive & 1;

	if (!ataWaitBsy(bus)) return false;

	ioWrite8(bus + 6, 0x40 | (slave << 4));
	ioWrite8(bus + 2, 0);
	ioWrite8(bus + 3, sectorOffset >> 24);
	ioWrite8(bus + 4, sectorOffset >> 32);
	ioWrite8(bus + 5, sectorOffset >> 40);
	ioWrite8(bus + 2, 1);
	ioWrite8(bus + 3, sectorOffset);
	ioWrite8(bus + 4, sectorOffset >> 8);
	ioWrite8(bus + 5, sectorOffset >> 16);
	ioWrite8(bus + 7, 0x34);

	if (!ataWaitBsy(bus)) return false;
	if (!ataWaitDrq(bus)) return false;

	uint16_t* target = (uint16_t*)buffer;

	for (uint16_t j = 0; j < ATA_SECTOR_LENGTH / sizeof(uint16_t); j++) ioWrite16(bus, target[j]);

	ioWrite8(bus + 7, 0xE7);

	if (!ataWaitBsy(bus)) return false;

	return !(ioRead8(bus + 7) & ATA_ERR);
}

bool ataRead(ATA_DRIVE drive, uint64_t offset, uint64_t length, uintptr_t buffer)
{
	bool result = true;
	uint64_t sectorCount = aligned_count_padded(length, ATA_SECTOR_LENGTH);
	
	for (uint64_t i = 0; i < sectorCount; i++)
		if (!ataReadSector(drive, offset + i * ATA_SECTOR_LENGTH, buffer + i * ATA_SECTOR_LENGTH))
			result = false;

	return result;
}

bool ataWrite(ATA_DRIVE drive, uint64_t offset, uint64_t length, uintptr_t buffer)
{
	bool result = true;
	uint64_t sectorCount = aligned_count_padded(length, ATA_SECTOR_LENGTH);
	
	for (uint64_t i = 0; i < sectorCount; i++)
		if (!ataWriteSector(drive, offset + i * ATA_SECTOR_LENGTH, buffer + i * ATA_SECTOR_LENGTH))
			result = false;

	return result;
}

typedef struct
{
	uint8_t Present;
	uint64_t Length;
	uint8_t Reserved[111];
	uint32_t MaxSector32;
	uint8_t Reserved2[43];
	uint8_t Reserved3 : 2;
	uint8_t Lba48Supported : 1;
	uint8_t Reserved4 : 5;
	uint8_t Reserved5[32];
	uint64_t MaxSector;
	uint8_t Reserved6[302];
} packed ATA_IDENTITY, *PATA_IDENTITY;

bool ataIdentify(ATA_DRIVE drive, PATA_IDENTITY ataIdentity)
{
	if (!drive || drive > ATA_MAX_DRIVE)
		return false;

	drive--;

	ATA_BUS bus;

	switch (drive & ~1)
	{
	case 0:
		bus = ATA_PRIMARY_1;
		break;
	case 2:
		bus = ATA_PRIMARY_2;
		break;
	case 4:
		bus = ATA_SECONDARY_1;
		break;
	case 6:
		bus = ATA_SECONDARY_2;
		break;
	default:
		return false;
		break;
	}

	bool slave = drive & 1;

	ioWrite8(bus + 6, 0xA0 | (slave << 4));
	ioWrite8(bus + 2, 0);
	ioWrite8(bus + 3, 0);
	ioWrite8(bus + 4, 0);
	ioWrite8(bus + 5, 0);
	ioWrite8(bus + 7, 0xEC);

	if (!ioRead8(bus + 7))
		return false;

	if (ioRead8(bus + 4))
		return false;

	if (ioRead8(bus + 5))
		return false;

	if (!ataWaitBsy(bus)) return false;
	if (!ataWaitDrq(bus)) return false;

	uint16_t* target = (uint16_t*)ataIdentity;

	for (uint16_t i = 0; i < ATA_SECTOR_LENGTH / sizeof(uint16_t); i++) target[i] = ioRead16(bus);

	if (!ataIdentity->Lba48Supported)
		ataIdentity->MaxSector = ataIdentity->MaxSector32;

	ataIdentity->Length = ataIdentity->MaxSector * ATA_SECTOR_LENGTH;
	
	ataIdentity->Present = true;

	return true;
}