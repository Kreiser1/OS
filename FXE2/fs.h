#pragma once

#include "std.h"
#include "ata.h"
#include "memory.h"
#include "string.h"

#define FS_TABLE_MAGIC (uint64_t)0x454C424154205346
#define FS_RECORD_MAGIC (uint64_t)0x44524F4345525346

#define FS_TABLE_OFFSET (uint64_t)0x100000

#define FS_BLOCK_LENGTH ATA_SECTOR_LENGTH
#define FS_NAME_LENGTH (uint64_t)128

#define FS_BLOCK_REQUIRESTED (uint8_t)1
#define FS_BLOCK_NEXT_REQUIRESTED (uint8_t)2

#define FS_RECORD_FILE (uint64_t)0
#define FS_RECORD_FOLDER (uint64_t)1
#define FS_RECORD_READ_ONLY (uint64_t)2
#define FS_RECORD_NO_REMOVE (uint64_t)4

typedef struct
{
    uint64_t Magic;
    uint64_t BlockCount;
    uint64_t Length;
    uint64_t RootOffset;
    uint8_t Reserved[480];
} packed FS_TABLE, *PFS_TABLE;

typedef uint8_t* FS_BLOCK_MAP;

typedef struct
{
    uint64_t Magic;
    uint64_t Flags;
    uint64_t Length;
    uint64_t NextRecordOffset, SubRecordOffset, Offset, PreviousRecordOffset;
    uint8_t Name[FS_NAME_LENGTH];
    uint8_t Reserved[328];
} packed FS_RECORD, *PFS_RECORD;

bool fsReadTable(uint8_t disk, PFS_TABLE fsTable)
{
    return ataRead(disk, FS_TABLE_OFFSET, FS_BLOCK_LENGTH, (uintptr_t)fsTable) && fsTable->Magic == FS_TABLE_MAGIC;
}

bool fsWriteTable(uint8_t disk, PFS_TABLE fsTable)
{
    return fsTable->Magic == FS_TABLE_MAGIC && ataWrite(disk, FS_TABLE_OFFSET, FS_BLOCK_LENGTH, (uintptr_t)fsTable);
}

bool fsReadBlockMap(uint8_t disk, FS_BLOCK_MAP blockMap)
{
    FS_TABLE fsTable;

    if (!fsReadTable(disk, &fsTable))
        return false;

    return ataRead(disk, FS_TABLE_OFFSET + FS_BLOCK_LENGTH, fsTable.BlockCount, (uintptr_t)blockMap);
}

bool fsWriteBlockMap(uint8_t disk, FS_BLOCK_MAP blockMap)
{
    FS_TABLE fsTable;

    if (!fsReadTable(disk, &fsTable))
        return false;

    return ataWrite(disk, FS_TABLE_OFFSET + FS_BLOCK_LENGTH, fsTable.BlockCount, (uintptr_t)blockMap);
}

bool fsReadRecord(uint8_t disk, uint64_t recordOffset, PFS_RECORD record)
{
    return ataRead(disk, recordOffset, sizeof(FS_RECORD), (uintptr_t)record) && record->Magic == FS_RECORD_MAGIC;
}

bool fsWriteRecord(uint8_t disk, uint64_t recordOffset, PFS_RECORD record)
{
    return record->Magic == FS_RECORD_MAGIC && ataWrite(disk, recordOffset, sizeof(FS_RECORD), (uintptr_t)record);
}

bool fsReadFileData(uint8_t disk, FS_RECORD file, uintptr_t data)
{
    if (file.Flags & FS_RECORD_FOLDER || !file.Length)
        return false;
    
    return ataRead(disk, file.Offset + sizeof(FS_RECORD), file.Length, data);
}

uint64_t fsRequirest(uint8_t disk, uint64_t length)
{
    if (!length)
        return 0;

    FS_TABLE fsTable;

    if (!fsReadTable(disk, &fsTable))
        return 0;

    uint8_t blockMap[align_padded(fsTable.BlockCount, FS_BLOCK_LENGTH)];

    if (!fsReadBlockMap(disk, blockMap))
        return 0;

    uint64_t blockCount = aligned_count_padded(length, FS_BLOCK_LENGTH);
    uint64_t fsBlocksOffset = FS_TABLE_OFFSET + FS_BLOCK_LENGTH + align_padded(fsTable.BlockCount, FS_BLOCK_LENGTH);

    for (uint64_t i = 0; i < fsTable.BlockCount; i++)
    {
        bool blocksFound = false;

        if (!(blockMap[i] & FS_BLOCK_REQUIRESTED))
            for (uint64_t j = i; j < i + blockCount; j++)
                if (!(blockMap[j] & FS_BLOCK_REQUIRESTED))
                    blocksFound = true;
                else
                {
                    blocksFound = false;
                    break;
                }

        if (!blocksFound) continue;

        for (uint64_t j = i; j < i + blockCount; j++)
            blockMap[j] |= FS_BLOCK_REQUIRESTED | FS_BLOCK_NEXT_REQUIRESTED;

        blockMap[i + blockCount - 1] &= ~FS_BLOCK_NEXT_REQUIRESTED;

        if (!fsWriteBlockMap(disk, blockMap))
            return 0;

        return fsBlocksOffset + i * FS_BLOCK_LENGTH;
    }

    return 0;
}

bool fsFree(uint8_t disk, uint64_t offset)
{
    if (!offset)
        return false;

    FS_TABLE fsTable;

    if (!fsReadTable(disk, &fsTable))
        return false;

    uint8_t blockMap[align_padded(fsTable.BlockCount, FS_BLOCK_LENGTH)];

    if (!fsReadBlockMap(disk, blockMap))
        return false;

    uint64_t i;
    uint64_t fsBlocksOffset = FS_TABLE_OFFSET + FS_BLOCK_LENGTH + align_padded(fsTable.BlockCount, FS_BLOCK_LENGTH);

    for (i = aligned_count(offset - fsBlocksOffset, FS_BLOCK_LENGTH); blockMap[i] & FS_BLOCK_NEXT_REQUIRESTED; i++)
        blockMap[i] &= ~FS_BLOCK_REQUIRESTED;

    blockMap[i] &= ~FS_BLOCK_REQUIRESTED;

    return fsWriteBlockMap(disk, blockMap);
}

bool fsInstall(uint8_t disk, uint64_t length)
{
    FS_TABLE fsTable;

    fsTable.Magic = FS_TABLE_MAGIC;
    fsTable.BlockCount = aligned_count(length, FS_BLOCK_LENGTH) - sizeof(uint8_t) * aligned_count(length, FS_BLOCK_LENGTH) / FS_BLOCK_LENGTH + 1;
    fsTable.Length = fsTable.BlockCount * FS_BLOCK_LENGTH;
    fsTable.RootOffset = 0;

    if (!fsWriteTable(disk, &fsTable))
        return false;

    uint8_t blockMap[fsTable.BlockCount];

    for (uint64_t i = 0; i < fsTable.BlockCount; i++)
        blockMap[i] = 0;

    if (!fsWriteBlockMap(disk, blockMap))
        return false;

    fsTable.RootOffset = fsRequirest(disk, sizeof(FS_RECORD));

    if (!fsTable.RootOffset)
        return false;

    if (!fsWriteTable(disk, &fsTable))
        return false;

    FS_RECORD rootRecord;

    rootRecord.Magic = FS_RECORD_MAGIC;
    rootRecord.Flags = FS_RECORD_FOLDER | FS_RECORD_NO_REMOVE;
    rootRecord.Offset = fsTable.RootOffset;
    rootRecord.SubRecordOffset = 0;

    strCopy("root", rootRecord.Name);

    return fsWriteRecord(disk, fsTable.RootOffset, &rootRecord);
}

bool fsSplitPath(char* path, uint64_t* length)
{
    if (*path != '/')
        return false;

    *path = '\0';

    *length = 1;
    path++;

    uint64_t nameLength = 0;

    while (*path)
    {
        if (!(!*path || *path == '/' || *path == ' ' || *path == '.' || (*path >= 48 && *path <= 57) || (*path >= 65 && *path <= 90) || (*path >= 97 && *path <= 122)))
            return false;

        if (*path == '/')
        {
            if (!nameLength)
                return false;

            *path = '\0';
            nameLength = 0;
            ++*length;
        }
        else nameLength++;

        path++;

        if (*path == '\0' && !nameLength)
            return false;
    }

    return true;
}

bool fsWalkSplittedPath(const char** path, uint64_t* length)
{
    if (!*length)
        return false;

    *path += strLength(*path) + 1;
    --*length;

    return true;
}

bool fsCreateRecord(uint8_t disk, PFS_RECORD folder, const char* name, uint64_t flags, uintptr_t data, uint64_t length, PFS_RECORD record)
{
    if (strLength(name) >= FS_NAME_LENGTH)
        return false;

    if (!(folder->Flags & FS_RECORD_FOLDER) || folder->Flags & FS_RECORD_READ_ONLY)
        return false;

    if (folder->SubRecordOffset)
    {
        FS_RECORD fsLastRecord;

        if (!fsReadRecord(disk, folder->SubRecordOffset, &fsLastRecord))
            return false;

        if (strCompare(fsLastRecord.Name, name))
            return false;

        uint64_t lastRecordOffset = folder->SubRecordOffset;

        while (fsLastRecord.NextRecordOffset)
        {
            lastRecordOffset = fsLastRecord.NextRecordOffset;
            
            if (!fsReadRecord(disk, fsLastRecord.NextRecordOffset, &fsLastRecord))
                return false;

            if (strCompare(fsLastRecord.Name, name))
                return false;
        }

        FS_RECORD fsNewRecord;

        fsNewRecord.Magic = FS_RECORD_MAGIC;
        fsNewRecord.Flags = flags;
        fsNewRecord.Length = !(fsNewRecord.Flags & FS_RECORD_FOLDER) && length ? length : 0;
        fsNewRecord.NextRecordOffset = 0;
        fsNewRecord.PreviousRecordOffset = lastRecordOffset;
        fsNewRecord.SubRecordOffset = 0;
        
        strCopy(name, fsNewRecord.Name);

        fsLastRecord.NextRecordOffset = fsRequirest(disk, sizeof(FS_RECORD) + fsNewRecord.Length);

        if (!fsLastRecord.NextRecordOffset)
            return false;

        fsNewRecord.Offset = fsLastRecord.NextRecordOffset;

        if (!fsWriteRecord(disk, fsNewRecord.Offset, &fsNewRecord))
            return false;

        if (record)
            *record = fsNewRecord;

        if (fsNewRecord.Length && !ataWrite(disk, fsNewRecord.Offset + sizeof(FS_RECORD), fsNewRecord.Length, data))
            return false;

        return fsWriteRecord(disk, lastRecordOffset, &fsLastRecord);
    }
    else
    {
        FS_RECORD fsNewRecord;

        fsNewRecord.Magic = FS_RECORD_MAGIC;
        fsNewRecord.Flags = flags;
        fsNewRecord.Length = !(fsNewRecord.Flags & FS_RECORD_FOLDER) && length ? length : 0;
        fsNewRecord.NextRecordOffset = 0;
        fsNewRecord.PreviousRecordOffset = folder->Offset;
        fsNewRecord.SubRecordOffset = 0;

        strCopy(name, fsNewRecord.Name);

        folder->SubRecordOffset = fsRequirest(disk, sizeof(FS_RECORD) + fsNewRecord.Length);

        if (!folder->SubRecordOffset)
            return false;

        fsNewRecord.Offset = folder->SubRecordOffset;

        if (!fsWriteRecord(disk, fsNewRecord.Offset, &fsNewRecord))
            return false;

        if (!(fsNewRecord.Flags && FS_RECORD_FOLDER) && fsNewRecord.Length && !ataWrite(disk, fsNewRecord.Offset + sizeof(FS_RECORD), fsNewRecord.Length, data))
            return false;

        return fsWriteRecord(disk, folder->Offset, folder);
    }
}

bool fsWalkFolder(uint8_t disk, FS_RECORD folder, PFS_RECORD record)
{
    if (folder.Magic != FS_RECORD_MAGIC || !(folder.Flags & FS_RECORD_FOLDER) || !folder.SubRecordOffset)
        return false;

    if (record->Magic != FS_RECORD_MAGIC)
        return fsReadRecord(disk, folder.SubRecordOffset, record);

    if (!record->NextRecordOffset)
    {
        record->Magic = 0;
        return false;
    }

    return fsReadRecord(disk, record->NextRecordOffset, record);
}

bool fsGetRecordByName(uint8_t disk, FS_RECORD folder, const char* name, PFS_RECORD record)
{
    if (folder.Magic != FS_RECORD_MAGIC || !(folder.Flags & FS_RECORD_FOLDER) || !folder.SubRecordOffset)
        return false;

    record->Magic = 0;

    while (fsWalkFolder(disk, folder, record))
        if (strCompare(record->Name, name))
            return true;
    
    return false;
}

bool fsGetRecordByPath(uint8_t disk, const char* path, PFS_RECORD record)
{
    FS_TABLE fsTable;

    if (!fsReadTable(disk, &fsTable))
        return false;

    if (strCompare(path, "/"))
        return fsReadRecord(disk, fsTable.RootOffset, record);

    uint64_t pathLength;
    char splittedPath[strLength(path)];

    strCopy(path, splittedPath);
    path = splittedPath;

    if (!fsSplitPath(path, &pathLength))
        return false;

    FS_RECORD folder;

    if (!fsReadRecord(disk, fsTable.RootOffset, &folder))
        return false;

    while (fsWalkSplittedPath(&path, &pathLength))
    {
        if (!pathLength)
            return fsGetRecordByName(disk, folder, path, record);

        if (!fsGetRecordByName(disk, folder, path, record) || !(record->Flags & FS_RECORD_FOLDER) || !record->SubRecordOffset)
            return false;

        folder = *record;
    }

    return false;
}

bool fsWalkFolderByPath(uint8_t disk, const char* path, PFS_RECORD record)
{
    FS_RECORD folder;

    if (!fsGetRecordByPath(disk, path, &folder))
        return false;

    return fsWalkFolder(disk, folder, record);
}


bool fsCreateRecordByPath(uint8_t disk, const char* path, uint64_t flags, uintptr_t data, uint64_t length, PFS_RECORD record)
{
    uint64_t pathLength;
    char splittedPath[strLength(path)];

    strCopy(path, splittedPath);
    path = splittedPath;

    if (!fsSplitPath(path, &pathLength))
        return false;

    FS_TABLE fsTable;

    if (!fsReadTable(disk, &fsTable))
        return false;

    FS_RECORD folder;

    if (!fsReadRecord(disk, fsTable.RootOffset, &folder))
        return false;

    FS_RECORD _record;

    if (!record)
        record = &_record;

    while (fsWalkSplittedPath(&path, &pathLength))
    {
        if (!pathLength)
            return fsCreateRecord(disk, &folder, path, flags, data, length, record);

        if (!fsGetRecordByName(disk, folder, path, record) || !(record->Flags & FS_RECORD_FOLDER))
            return false;

        folder = *record;
    }

    return false;
}

bool fsDeleteRecord(uint8_t disk, PFS_RECORD folder, const char* name)
{
    if (!(folder->Flags & FS_RECORD_FOLDER) || folder->Flags & FS_RECORD_READ_ONLY)
        return false;

    FS_RECORD record;

    record.Magic = 0;

    while (fsWalkFolder(disk, *folder, &record))
        if (strCompare(record.Name, name))
            if (record.Flags & FS_RECORD_NO_REMOVE || record.Flags & FS_RECORD_READ_ONLY)
                return false;
            else if (record.PreviousRecordOffset == folder->Offset && folder->SubRecordOffset == record.Offset)
            {
                folder->SubRecordOffset = record.NextRecordOffset;

                if (!fsWriteRecord(disk, folder->Offset, folder))
                    return false;

                return fsFree(disk, record.Offset);
            }
            else if (record.PreviousRecordOffset)
            {
                FS_RECORD previousRecord;

                if (!fsReadRecord(disk, record.PreviousRecordOffset, &previousRecord))
                    return false;
                
                previousRecord.NextRecordOffset = record.NextRecordOffset;

                if (!fsWriteRecord(disk, previousRecord.Offset, &previousRecord))
                    return false;

                return fsFree(disk, record.Offset);
            }
            else return false;

    return false;
}

bool fsDeleteRecordByPath(uint8_t disk, const char* path)
{
    uint64_t pathLength;
    char splittedPath[strLength(path)];

    strCopy(path, splittedPath);
    path = splittedPath;

    if (!fsSplitPath(path, &pathLength))
        return false;

    FS_TABLE fsTable;

    if (!fsReadTable(disk, &fsTable))
        return false;

    FS_RECORD folder, record;

    if (!fsReadRecord(disk, fsTable.RootOffset, &folder))
        return false;

    while (fsWalkSplittedPath(&path, &pathLength))
    {
        if (!pathLength)
        {
            if (!fsGetRecordByName(disk, folder, path, &record))
                return false;

            return fsDeleteRecord(disk, &folder, record.Name);
        }

        if (!fsGetRecordByName(disk, folder, path, &record) || !(record.Flags & FS_RECORD_FOLDER))
            return false;

        folder = record;
    }

    return false;
}

bool fsReadFile(uint8_t disk, const char* path, uintptr_t data)
{
    FS_RECORD file;

    if (!fsGetRecordByPath(disk, path, &file))
        return false;

    return fsReadFileData(disk, file, data);
}

bool fsWriteFile(uint8_t disk, const char* path, uintptr_t data, uint64_t length)
{
    uint64_t pathLength;
    char splittedPath[strLength(path)];

    strCopy(path, splittedPath);
    path = splittedPath;

    if (!fsSplitPath(path, &pathLength))
        return false;

    FS_TABLE fsTable;

    if (!fsReadTable(disk, &fsTable))
        return false;

    FS_RECORD folder, record;

    if (!fsReadRecord(disk, fsTable.RootOffset, &folder))
        return false;

    while (fsWalkSplittedPath(&path, &pathLength))
    {
        if (!pathLength)
            if (!fsGetRecordByName(disk, folder, path, &record))
                return false;
            else
            {
                if (record.Flags & FS_RECORD_FOLDER || record.Flags & FS_RECORD_READ_ONLY)
                    return false;
                else if (record.Length <= length)
                    return ataWrite(disk, record.Offset + sizeof(FS_RECORD), length, data);
                else
                {
                    if (!fsDeleteRecord(disk, &folder, record.Name))
                        return false;

                    FS_RECORD newRecord;

                    return fsCreateRecord(disk, &folder, record.Name, record.Flags, data, length, &newRecord);
                }
            }
        
        if (!fsGetRecordByName(disk, folder, path, &record) || !(record.Flags & FS_RECORD_FOLDER))
            return false;

        folder = record;
    }

    return false;
}