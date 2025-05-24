#pragma once

#include "std.h"
#include "memory.h"

typedef struct
{
    uint8_t BlockLength;
    uint32_t BlockCount;
    uint64_t Length;
    uint8_t* BlockMap;
    uintptr_t Blocks;
} HEAP, *PHEAP;

#define HEAP_BLOCK_REQUIRESTED (uint8_t)1
#define HEAP_BLOCK_NEXT_REQUIRESTED (uint8_t)2

void initializeHeap(PHEAP heap, uintptr_t pointer, uint64_t length, uint8_t blockLength)
{
    heap->BlockLength = blockLength;
    heap->BlockCount = aligned_count(length, blockLength) - sizeof(uint8_t) * aligned_count(length, blockLength) / blockLength + 1;
    mmSet(pointer, sizeof(uint8_t) * aligned_count(length, blockLength), 0);
    heap->BlockMap = (uint8_t*)pointer;
    heap->Blocks = (uintptr_t)heap->BlockMap + (sizeof(uint8_t) * aligned_count(length, blockLength) / blockLength + 1) * blockLength;
    heap->Length = heap->BlockCount * heap->BlockLength;
}

uintptr_t heapRequirest(PHEAP heap, uint64_t length)
{
    if (length > heap->Length)
        return 0;

    uint32_t blockCount = aligned_count_padded(length, heap->BlockLength);

    for (uint32_t i = 0; i < heap->BlockCount; i++)
    {
        bool blocksFound = false;

        if (!(heap->BlockMap[i] & HEAP_BLOCK_REQUIRESTED))
            for (uint32_t j = i; j < i + blockCount; j++)
                if (!(heap->BlockMap[j] & HEAP_BLOCK_REQUIRESTED))
                    blocksFound = true;
                else
                {
                    blocksFound = false;
                    break;
                }

        if (!blocksFound) continue;

        for (uint32_t j = i; j < i + blockCount; j++)
            heap->BlockMap[j] |= HEAP_BLOCK_REQUIRESTED | HEAP_BLOCK_NEXT_REQUIRESTED;

        heap->BlockMap[i + blockCount - 1] &= ~HEAP_BLOCK_NEXT_REQUIRESTED;

        return (uintptr_t)heap->Blocks + i * heap->BlockLength;
    }

    return 0;
}

void heapFree(PHEAP heap, uintptr_t* pool)
{
    if (*pool < heap->Blocks || *pool > heap->Blocks + heap->Length)
        return;

    uint32_t i;

    for (i = aligned_count(*pool - heap->Blocks, heap->BlockLength); heap->BlockMap[i] & HEAP_BLOCK_NEXT_REQUIRESTED; i++)
        heap->BlockMap[i] &= ~HEAP_BLOCK_REQUIRESTED;

    heap->BlockMap[i] &= ~HEAP_BLOCK_REQUIRESTED;
    *pool = 0;
}