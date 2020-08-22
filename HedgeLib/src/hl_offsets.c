#include "hedgelib/hl_internal.h"
#include "hl_in_assert.h"

#if HL_UPTR_MAX > 0xffffffffu
void* hlOff32Get(const HlU32* offPtr)
{
    /* Convert from relative pointer to absolute pointer and return result. */
    return (*offPtr) ? (void*)((HlSPtr)offPtr +
        (HlSPtr)((HlS32)*offPtr)) : NULL;
}

void hlOff32Set(HlU32* offPtr, void* ptr)
{
    /*
       Pointers are > 32 bits, so ptr will not fit within a 32-bit offset.
       We have to convert ptr to a relative pointer instead, and
       store that within the given offset.
    */
    HlSPtr addr = (HlSPtr)offPtr;

    /* Return early if the given offset is null. */
    if (*offPtr == 0) return;

    /*
       Ensure offset does not point to itself; we can't do that
       with our relative offsets, since we still use 0 for null.
    */
    HL_ASSERT((HlSPtr)ptr != addr);

    /* Compute a relative offset which points to ptr. */
    addr = ((HlSPtr)ptr - addr);

    /*
       Ensure relative offset can fit within a signed
       32-bit integer, since we're about to cast to that.
    */
    HL_ASSERT(addr <= 2147483647 && addr >= (-2147483647 - 1));

    /* Set the given offset to the relative offset we just computed. */
    *offPtr = (HlU32)addr;
}
#endif

#if HL_UPTR_MAX > 0xffffffffffffffffu
void* hlOff64Get(const HlU64* offPtr)
{
    /* Convert from relative pointer to absolute pointer and return result. */
    return (*offPtr) ? (void*)((HlSPtr)offPtr +
        (HlSPtr)((HlS64)*offPtr)) : 0;
}

void hlOff64Set(HlU64* offPtr, void* ptr)
{
    /*
       Pointers are > 64 bits, so ptr will not fit within a 64-bit offset.
       We have to convert ptr to a relative pointer instead, and
       store that within the given offset.
    */
    HlSPtr addr = (HlSPtr)offPtr;

    /* Return early if the given offset is null. */
    if (*offPtr == 0) return;

    /*
       Ensure offset does not point to itself; we can't do that
       with our relative offsets, since we still use 0 for null.
    */
    HL_ASSERT((HlSPtr)ptr != addr);

    /* Compute a relative offset which points to ptr. */
    addr = ((HlSPtr)ptr - addr);

    /*
       Ensure relative offset can fit within a signed
       64-bit integer, since we're about to cast to that.
    */
    HL_ASSERT(addr <= 9223372036854775807 && addr >= (-9223372036854775807 - 1));

    /* Set the given offset to the relative offset we just computed. */
    *offPtr = (HlU64)addr;
}
#endif

#ifndef HL_NO_EXTERNAL_WRAPPERS
void* hlOff32GetExt(const HlU32* offPtr)
{
    return hlOff32Get(offPtr);
}

void hlOff32SetExt(HlU32* offPtr, void* ptr)
{
    hlOff32Set(offPtr, ptr);
}

void* hlOff64GetExt(const HlU64* offPtr)
{
    return hlOff64Get(offPtr);
}

void hlOff64SetExt(HlU64* offPtr, void* ptr)
{
    hlOff64Set(offPtr, ptr);
}

void hlOff32FixExt(HlU32* offPtr, void* base)
{
    hlOff32Fix(offPtr, base);
}

void hlOff64FixExt(HlU64* offPtr, void* base)
{
    hlOff64Fix(offPtr, base);
}
#endif
