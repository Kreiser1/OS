#pragma once

#include "std.h"
#include "pmm.h"

typedef uint64_t PTE, *PT;

#define PAGE_PRESENT (PTE)3
#define PAGE_USER (PTE)4
#define PAGE_CACHE_DISABLE (PTE)24
#define PAGE_REQUIRESTED (PTE)512
#define PAGE_NEXT_REQUIRESTED (PTE)1024
#define PAGE_REQUIRESTABLE (PTE)2048

#define PAGE_LENGTH (PTE)4096
#define VM_LENGTH (PTE)103079215104
#define PME_LENGTH (PTE)512
#define PTES_LENGTH aligned_count(VM_LENGTH, PAGE_LENGTH)
#define PDS_LENGTH aligned_count(PTES_LENGTH, PME_LENGTH)
#define PDPT_LENGTH aligned_count(PDS_LENGTH, PME_LENGTH)
#define VMM_OFFSET (PTE)0xF0000000
#define PML4_OFFSET VMM_OFFSET
#define PDPT_OFFSET (PML4_OFFSET + PME_LENGTH * sizeof(PTE))
#define PDS_OFFSET (PDPT_OFFSET + PME_LENGTH * sizeof(PTE))
#define PTES_OFFSET (PDS_OFFSET + PDS_LENGTH * sizeof(PTE))
#define VMM_LENGTH (PME_LENGTH * 2 * sizeof(PTE) + PDS_LENGTH * sizeof(PTE) + PTES_LENGTH * sizeof(PTE))

void initializeVmm(uint64_t vmmPhysicalOffset)
{
	for (PT pt = (PT)VMM_OFFSET; pt < VMM_OFFSET + VMM_LENGTH; pt += sizeof(PTE))
		*pt = 0;

	PT pageMapLevel4 = (PT)PML4_OFFSET;
	PT pageDirectoryPointerTable = (PT)PDPT_OFFSET;
	PT pageDirectories = (PT)PDS_OFFSET;
	PT pageTableEntries = (PT)PTES_OFFSET;

	pageMapLevel4[0] = ((PTE)pageDirectoryPointerTable - VMM_OFFSET + vmmPhysicalOffset) | PAGE_PRESENT | PAGE_USER;

	for (uint16_t i = 0; i < PDPT_LENGTH; i++)
		pageDirectoryPointerTable[i] = ((PTE)&pageDirectories[i * PME_LENGTH] - VMM_OFFSET + vmmPhysicalOffset) | PAGE_PRESENT | PAGE_USER;

	for (uint32_t i = 0; i < PDS_LENGTH; i++)
		pageDirectories[i] = ((PTE)&pageTableEntries[i * PME_LENGTH] - VMM_OFFSET + vmmPhysicalOffset) | PAGE_PRESENT | PAGE_USER;

	for (uint32_t i = 0; i < aligned_count_padded(VMM_LENGTH, PAGE_LENGTH); i++)
		pageTableEntries[aligned_count(VMM_OFFSET, PAGE_LENGTH) + i] = vmmPhysicalOffset + i * PAGE_LENGTH | PAGE_PRESENT;
}

void loadVmm(uint64_t vmmPhysicalOffset)
{
	asm("mov %0, %%cr3" ::"r"(vmmPhysicalOffset));
}

bool vmMap(uint64_t offset, uintptr_t pointer, uint64_t length)
{
	if (offset > PM_LENGTH || pointer > VM_LENGTH || offset + length > PM_LENGTH || pointer + length > VM_LENGTH)
		return false;

	uint32_t startPage = aligned_count(pointer, PAGE_LENGTH);
	uint32_t endPage = startPage + aligned_count_padded(length, PAGE_LENGTH);

	if (endPage <= startPage)
		return false;

	offset = align(offset, PAGE_LENGTH);

	PT pageTableEntries = (PT)PTES_OFFSET;

	for (uint32_t i = startPage; i < endPage; i++)
		pageTableEntries[i] = offset + (i - startPage) * PAGE_LENGTH | PAGE_PRESENT;

	return true;
}

bool vmDelete(uintptr_t pointer, uint64_t length)
{
	if (pointer > VM_LENGTH || pointer + length > VM_LENGTH)
		return false;

	uint32_t startPage = aligned_count(pointer, PAGE_LENGTH);
	uint32_t endPage = startPage + aligned_count_padded(length, PAGE_LENGTH);

	if (endPage <= startPage)
		return false;

	PT pageTableEntries = (PT)PTES_OFFSET;

	for (uint32_t i = startPage; i < endPage; i++)
		pageTableEntries[i] &= ~PAGE_PRESENT;

	return true;
}

bool vmLock(uintptr_t pointer, uint64_t length)
{
	if (pointer > VM_LENGTH || pointer + length > VM_LENGTH)
		return false;

	uint32_t startPage = aligned_count(pointer, PAGE_LENGTH);
	uint32_t endPage = startPage + aligned_count_padded(length, PAGE_LENGTH);

	if (endPage <= startPage)
		return false;

	bool result = true;

	PT pageTableEntries = (PT)PTES_OFFSET;

	for (uint32_t i = startPage; i < endPage; i++)
		if (pageTableEntries[i] & PAGE_PRESENT)
			pageTableEntries[i] &= ~PAGE_USER;
		else
			result = false;

	return result;
}

bool vmUnlock(uintptr_t pointer, uint64_t length)
{
	if (pointer > VM_LENGTH || pointer + length > VM_LENGTH)
		return false;

	uint32_t startPage = aligned_count(pointer, PAGE_LENGTH);
	uint32_t endPage = startPage + aligned_count_padded(length, PAGE_LENGTH);

	if (endPage <= startPage)
		return false;

	bool result = true;

	PT pageTableEntries = (PT)PTES_OFFSET;

	for (uint32_t i = startPage; i < endPage; i++)
		if (pageTableEntries[i] & PAGE_PRESENT)
			pageTableEntries[i] |= PAGE_USER;
		else
			result = false;

	return result;
}

bool vmYield(uintptr_t pointer, uint64_t length)
{
	if (pointer > VM_LENGTH || pointer + length > VM_LENGTH)
		return false;

	uint32_t startPage = aligned_count(pointer, PAGE_LENGTH);
	uint32_t endPage = startPage + aligned_count_padded(length, PAGE_LENGTH);

	if (endPage <= startPage)
		return false;

	bool result = true;

	PT pageTableEntries = (PT)PTES_OFFSET;

	for (uint32_t i = startPage; i < endPage; i++)
		if (pageTableEntries[i] & PAGE_PRESENT)
			pageTableEntries[i] |= PAGE_REQUIRESTABLE;
		else
			result = false;

	return result;
}

bool vmClaim(uintptr_t pointer, uint64_t length)
{
	if (pointer > VM_LENGTH || pointer + length > VM_LENGTH)
		return false;

	uint32_t startPage = aligned_count(pointer, PAGE_LENGTH);
	uint32_t endPage = startPage + aligned_count_padded(length, PAGE_LENGTH);

	if (endPage <= startPage)
		return false;

	bool result = true;

	PT pageTableEntries = (PT)PTES_OFFSET;

	for (uint32_t i = startPage; i < endPage; i++)
		if (pageTableEntries[i] & PAGE_PRESENT)
			pageTableEntries[i] &= ~PAGE_REQUIRESTABLE;
		else
			result = false;

	return result;
}

bool vmCopy(uintptr_t pointer1, uintptr_t pointer2, uint64_t length)
{
	if (!length || pointer1 > VM_LENGTH || pointer2 > VM_LENGTH || pointer1 + length > VM_LENGTH || pointer2 + length > VM_LENGTH)
		return false;

	uint32_t pageCount = aligned_count_padded(length, PAGE_LENGTH);
	uint32_t sourcePage = aligned_count(pointer1, PAGE_LENGTH);
	uint32_t destinationPage = aligned_count(pointer2, PAGE_LENGTH);

	bool result = true;

	PT pageTableEntries = (PT)PTES_OFFSET;

	for (uint32_t i = 0; i < pageCount; i++)
		if (pageTableEntries[sourcePage + i] & PAGE_PRESENT)
			pageTableEntries[destinationPage + i] = pageTableEntries[sourcePage + i];
		else
			result = false;

	return result;
}

bool vmClone(PT pageTableEntries2, uintptr_t pointer1, uintptr_t pointer2, uint64_t length)
{
	if (!length || pointer1 > VM_LENGTH || pointer2 > VM_LENGTH || pointer1 + length > VM_LENGTH || pointer2 + length > VM_LENGTH)
		return false;

	uint32_t pageCount = aligned_count_padded(length, PAGE_LENGTH);
	uint32_t sourcePage = aligned_count(pointer1, PAGE_LENGTH);
	uint32_t destinationPage = aligned_count(pointer2, PAGE_LENGTH);

	bool result = true;

	PT pageTableEntries = (PT)PTES_OFFSET;

	for (uint32_t i = 0; i < pageCount; i++)
		if (pageTableEntries[sourcePage + i] & PAGE_PRESENT)
			pageTableEntries2[destinationPage + i] = pageTableEntries[sourcePage + i];
		else
			result = false;

	return result;
}

bool vmRelocate(uintptr_t pointer1, uintptr_t pointer2, uint64_t length)
{
	if (vmCopy(pointer1, pointer2, length))
	{
		vmDelete(pointer1, length);
		return true;
	}

	return false;
}

uintptr_t vmRequirest(uint64_t length)
{
	uint32_t pageCount = aligned_count_padded(length, PAGE_LENGTH);

	if (PTES_LENGTH - pageCount > PTES_LENGTH)
		return 0;

	PT pageTableEntries = (PT)PTES_OFFSET;

	for (uint32_t i = 0; i < PTES_LENGTH - pageCount; i++)
	{
		bool pagesFound = false;

		if (pageTableEntries[i] & PAGE_REQUIRESTABLE && !(pageTableEntries[i] & PAGE_REQUIRESTED))
			for (uint32_t j = i; j < i + pageCount; j++)
				if (pageTableEntries[j] & PAGE_REQUIRESTABLE && !(pageTableEntries[j] & PAGE_REQUIRESTED))
					pagesFound = true;
				else
				{
					pagesFound = false;
					break;
				}

		if (!pagesFound)
			continue;

		for (uint32_t j = i; j < i + pageCount; j++)
			pageTableEntries[j] |= PAGE_REQUIRESTED | PAGE_NEXT_REQUIRESTED;

		pageTableEntries[i + pageCount - 1] &= ~PAGE_NEXT_REQUIRESTED;

		return (uintptr_t)i * PAGE_LENGTH;
	}

	return 0;
}

void vmFree(uintptr_t *pool)
{
	uint32_t i;

	PT pageTableEntries = (PT)PTES_OFFSET;

	for (i = aligned_count(*pool, PAGE_LENGTH); pageTableEntries[i] & PAGE_NEXT_REQUIRESTED; i++)
		pageTableEntries[i] &= ~PAGE_REQUIRESTED;

	pageTableEntries[i] &= ~PAGE_REQUIRESTED;
	*pool = 0;
}

uint64_t vmRequirestableLength()
{
	uint64_t length = 0;

	PT pageTableEntries = (PT)PTES_OFFSET;

	for (uint32_t i = 0; i < PTES_LENGTH; i++)
		if (pageTableEntries[i] & PAGE_REQUIRESTABLE && !(pageTableEntries[i] & PAGE_REQUIRESTED))
			length += PAGE_LENGTH;

	return length;
}