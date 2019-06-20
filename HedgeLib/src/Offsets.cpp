#include "HedgeLib/Offsets.h"
#include "HedgeLib/Endian.h"

void hl_FixOffset32(uint32_t* off, const void* data, bool isBigEndian)
{
    // Null pointers should remain null
    if (*off == 0)
    {
#ifdef x64
        *off = hl_x64AddAbsPtr32(0);
#endif
        return;
    }

    // Endian swap offset
    if (isBigEndian) hl_SwapUInt32(off);

    // Get absolute pointer
    uintptr_t absPtr = (reinterpret_cast
        <uintptr_t>(data) + *off);

    // Fix offset
#ifdef x86
    *off = static_cast<uint32_t>(absPtr);
#elif x64
    *off = hl_x64AddAbsPtr32(absPtr);
#endif
}

void hl_FixOffset64(uint64_t* off, const void* data, bool isBigEndian)
{
    // Null pointers should remain null
    if (*off == 0) return;

    // Endian swap offset
    if (isBigEndian) hl_SwapUInt64(off);

    // Get absolute pointer
    uintptr_t absPtr = (reinterpret_cast
        <uintptr_t>(data) + *off);

    // Fix offset
    *off = static_cast<uint64_t>(absPtr);
}

hl_OffsetTable* hl_CreateOffsetTable()
{
    return new hl_OffsetTable();
}

void hl_AddOffset(hl_OffsetTable* offTable, long offset)
{
    offTable->push_back(offset);
}

long* hl_GetOffsetsPtr(hl_OffsetTable* offTable)
{
    return offTable->data();
}

size_t hl_GetOffsetCount(const hl_OffsetTable* offTable)
{
    return offTable->size();
}

void hl_GetOffsets(hl_OffsetTable* offTable,
    long** offsets, size_t* count)
{
    *offsets = offTable->data();
    *count = offTable->size();
}

void hl_DestroyOffsetTable(hl_OffsetTable* offTable)
{
    delete offTable;
}

#ifdef x64
std::vector<uintptr_t> ptrs = { 0 };
static std::vector<bool> ptrFlags = { false };
static size_t nextFreeIndex = 1;

uintptr_t hl_x64GetAbsPtr32(hl_DataOff32 index)
{
    return ptrs[index];
}

hl_DataOff32 hl_x64AddAbsPtr32(uintptr_t ptr)
{
    hl_DataOff32 index = static_cast<hl_DataOff32>(nextFreeIndex);
    if (nextFreeIndex == ptrs.size())
    {
        // No "free" (unused) spots available; Add pointer
        ptrs.push_back(ptr);
        ptrFlags.push_back(true);
        ++nextFreeIndex;
    }
    else
    {
        // Set pointer
        ptrs[nextFreeIndex] = ptr;
        ptrFlags[nextFreeIndex] = true;

        // Determine next free index
        while (++nextFreeIndex < ptrs.size())
        {
            if (!ptrFlags[nextFreeIndex])
                break;
        }
    }

    return index;
}

void hl_x64SetAbsPtr32(hl_DataOff32 index, uintptr_t ptr)
{
    ptrs[index] = ptr;
}

void hl_x64RemoveAbsPtr32(hl_DataOff32 index)
{
    if (index == 0)
        return;

    if (index == (ptrs.size() - 1) &&
        nextFreeIndex == ptrs.size())
    {
        ptrs.pop_back();
        ptrFlags.pop_back();

        --nextFreeIndex;
    }
    else
    {
        ptrFlags[index] = false;
        if (index < nextFreeIndex)
        {
            nextFreeIndex = index;
        }
    }
}
#endif
