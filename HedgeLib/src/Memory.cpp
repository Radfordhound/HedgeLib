#include "HedgeLib/Direct/Memory.h"
#include <vector>
#include <cstdlib>

#ifdef x64
extern std::vector<uintptr_t> ptrs;
#endif

hl_DataOff32 hl_Alloc32(size_t size)
{
    void* ptr = std::malloc(size);

#ifdef x64
    return (ptr) ? hl_x64AddAbsPtr32(
        reinterpret_cast<uintptr_t>(ptr)) : 0;
#elif x86
    return static_cast<hl_DataOff32>(
        reinterpret_cast<uintptr_t>(ptr));
#endif
}

void hl_Free32(hl_DataOff32 off)
{
#ifdef x64
    // If off is 0, ptr will be set to a null pointer, and
    // free will simply do nothing as-per the C/C++ standard.
    std::free(reinterpret_cast<void*>(ptrs[off]));

    // Will check if off is 0
    hl_x64RemoveAbsPtr32(off);
#elif x86
    // Simply cast to void* and free
    std::free(reinterpret_cast<void*>(
        static_cast<uintptr_t>(off)));
#endif
}

hl_DataOff64 hl_Alloc64(size_t size)
{
    // On x64, should compile down to just a simple call to malloc.
    // On x86, an additional opcode to extend the 32-bit uintptr_t
    // to a 64-bit hl_DataOff64 may be added.
    return static_cast<hl_DataOff64>(reinterpret_cast
        <uintptr_t>(std::malloc(size)));
}

void hl_Free64(hl_DataOff64 off)
{
    // Simply cast to void* and free
    std::free(reinterpret_cast<void*>(
        static_cast<uintptr_t>(off)));
}
