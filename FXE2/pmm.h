#pragma once

#include "std.h"

#define PHYSICAL_PAGE_REQUIRESTED (uint64_t)1
#define PHYSICAL_PAGE_NEXT_REQUIRESTED (uint64_t)4
#define PHYSICAL_PAGE_REQUIRESTABLE (uint64_t)8

#define PHYSICAL_PAGE_LENGTH (uint64_t)2097152
#define PM_LENGTH (uint64_t)103079215104
#define PHYSICAL_PAGE_MAP_LENGTH (PM_LENGTH / PHYSICAL_PAGE_LENGTH)

typedef uint8_t* PHYSICAL_PAGE_MAP;

bool pmYield(PHYSICAL_PAGE_MAP physicalPageMap, uintptr_t pointer, uint64_t length)
{
    if (pointer > PM_LENGTH || pointer + length > PM_LENGTH)
        return false;

    uint32_t startPage = aligned_count(pointer, PHYSICAL_PAGE_LENGTH);
    uint32_t endPage = startPage + aligned_count_padded(length, PHYSICAL_PAGE_LENGTH);

    if (endPage <= startPage)
        return false;

    for (uint32_t i = startPage; i < endPage; i++)
        physicalPageMap[i] = physicalPageMap[i] & ~PHYSICAL_PAGE_REQUIRESTED | PHYSICAL_PAGE_REQUIRESTABLE;

    return true;
}

bool pmClaim(PHYSICAL_PAGE_MAP physicalPageMap, uintptr_t pointer, uint64_t length)
{
    if (pointer > PM_LENGTH || pointer + length > PM_LENGTH)
        return false;

    uint32_t startPage = aligned_count(pointer, PHYSICAL_PAGE_LENGTH);
    uint32_t endPage = startPage + aligned_count_padded(length, PHYSICAL_PAGE_LENGTH);

    if (endPage <= startPage)
        return false;

    for (uint32_t i = startPage; i < endPage; i++)
        physicalPageMap[i] = physicalPageMap[i] & ~PHYSICAL_PAGE_REQUIRESTABLE & ~PHYSICAL_PAGE_REQUIRESTED;

    return true;
}

uintptr_t pmRequirest(PHYSICAL_PAGE_MAP physicalPageMap, uint64_t length)
{
    uint32_t pageCount = aligned_count_padded(length, PHYSICAL_PAGE_LENGTH);

    if (PHYSICAL_PAGE_MAP_LENGTH - pageCount > PHYSICAL_PAGE_MAP_LENGTH)
        return 0;

    for (uint32_t i = 0; i < PHYSICAL_PAGE_MAP_LENGTH - pageCount; i++)
    {
        bool pagesFound = false;

        if (physicalPageMap[i] & PHYSICAL_PAGE_REQUIRESTABLE && !(physicalPageMap[i] & PHYSICAL_PAGE_REQUIRESTED))
            for (uint32_t j = i; j < i + pageCount; j++)
                if (physicalPageMap[j] & PHYSICAL_PAGE_REQUIRESTABLE && !(physicalPageMap[j] & PHYSICAL_PAGE_REQUIRESTED))
                    pagesFound = true;
                else
                {
                    pagesFound = false;
                    break;
                }

        if (!pagesFound) continue;

        for (uint32_t j = i; j < i + pageCount; j++)
            physicalPageMap[j] |= PHYSICAL_PAGE_REQUIRESTED | PHYSICAL_PAGE_NEXT_REQUIRESTED;

        physicalPageMap[i + pageCount - 1] &= ~PHYSICAL_PAGE_NEXT_REQUIRESTED;

        return (uintptr_t)i * PHYSICAL_PAGE_LENGTH;
    }

	return 0;
}

void pmFree(PHYSICAL_PAGE_MAP physicalPageMap, uintptr_t* pool)
{
    uint32_t i;

    for (i = aligned_count(*pool, PHYSICAL_PAGE_LENGTH); physicalPageMap[i] & PHYSICAL_PAGE_NEXT_REQUIRESTED; i++)
        physicalPageMap[i] &= ~PHYSICAL_PAGE_REQUIRESTED;

    physicalPageMap[i] &= ~PHYSICAL_PAGE_REQUIRESTED;
    *pool = 0;
}

uint64_t pmRequirestableLength(PHYSICAL_PAGE_MAP physicalPageMap)
{
    uint64_t length = 0;
    
    for (uint32_t i = 0; i < PHYSICAL_PAGE_MAP_LENGTH; i++)
        if (physicalPageMap[i] & PHYSICAL_PAGE_REQUIRESTABLE && !(physicalPageMap[i] & PHYSICAL_PAGE_REQUIRESTED))
            length += PHYSICAL_PAGE_LENGTH;

    return length;
}