#include "HedgeLib/Offsets.h"
#include "HedgeLib/Endian.h"
#include <new>

void hl_FixOffset32(hl_DataOff32* off, const void* data, bool isBigEndian)
{
    // Null pointers should remain null
    if (*off == 0) return;

    // Endian swap offset
    if (isBigEndian) hl_SwapUInt32(off);

    // Get absolute pointer
    uintptr_t absPtr = (reinterpret_cast<uintptr_t>(data) + *off);

    // Fix offset
#ifdef x86
    // (Set offset to absolute pointer)
    *off = static_cast<hl_DataOff32>(absPtr);
#elif x64
    // (Make offset relative to itself rather than to the base address)
    *off = static_cast<hl_DataOff32>(
        absPtr - reinterpret_cast<uintptr_t>(off));
#endif
}

void hl_FixOffset64(hl_DataOff64* off, const void* data, bool isBigEndian)
{
    // Null pointers should remain null
    if (*off == 0) return;

    // Endian swap offset
    if (isBigEndian) hl_SwapUInt64(off);

    // Get absolute pointer
    uintptr_t absPtr = (reinterpret_cast<uintptr_t>(data) + *off);

    // Fix offset
    // (Set offset to absolute pointer)
    *off = static_cast<hl_DataOff64>(absPtr);
}

hl_OffsetTable* hl_CreateOffsetTable()
{
    return new (std::nothrow) hl_OffsetTable();
}

HL_RESULT hl_AddOffset(hl_OffsetTable* offTable, long offset)
{
    try
    {
        offTable->push_back(offset);
    }
    catch (const std::bad_alloc&)
    {
        return HL_ERROR_OUT_OF_MEMORY;
    }

    return HL_SUCCESS;
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
