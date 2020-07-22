#include "hedgelib/hl_memory.h"

#ifdef _WIN32
#include <windows.h>

/* Win32-specific memory-allocation functions which are not reliant upon the CRT. */
static HANDLE hlINWin32ProcessHeap = 0;

static void* hlINWin32Alloc(size_t size)
{
    return HeapAlloc(hlINWin32ProcessHeap, 0, size);
}

static void* hlINWin32Realloc(void* ptr, size_t size)
{
    /*
       We call HeapAlloc if ptr is null instead as HeapReAlloc's
       behavior is undefined in this case.
    */
    if (!ptr) return HeapAlloc(hlINWin32ProcessHeap, 0, size);
    return HeapReAlloc(hlINWin32ProcessHeap, 0, ptr, size);
}

static void hlINWin32Free(void* ptr)
{
    /* We return if ptr is null as HeapFree's behavior is undefined in this case. */
    if (!ptr) return;
    HeapFree(hlINWin32ProcessHeap, 0, ptr);
}

static void* hlINWin32FirstAlloc(size_t size)
{
    /* If hlINWin32ProcessHeap has not yet been set, set it to the current process's heap. */
    if (!hlINWin32ProcessHeap) hlINWin32ProcessHeap = GetProcessHeap();

    /* Set HlAllocPtr to hlINWin32Alloc so it'll be called directly from now on. */
    HlAllocPtr = hlINWin32Alloc;

    /* Call hpINAlloc to actually allocate the given amount of memory. */
    return hlINWin32Alloc(size);
}

static void* hlINWin32FirstRealloc(void* ptr, size_t size)
{
    /* If hlINWin32ProcessHeap has not yet been set, set it to the current process's heap. */
    if (!hlINWin32ProcessHeap) hlINWin32ProcessHeap = GetProcessHeap();

    /* Set HlReallocPtr to hlINWin32Realloc so it'll be called directly from now on. */
    HlReallocPtr = hlINWin32Realloc;

    /* Call hlINWin32Realloc to actually reallocate the given pointer. */
    return hlINWin32Realloc(ptr, size);
}

static void hlINWin32FirstFree(void* ptr)
{
    /* If hlINWin32ProcessHeap has not yet been set, set it to the current process's heap. */
    if (!hlINWin32ProcessHeap) hlINWin32ProcessHeap = GetProcessHeap();

    /* Set HlFreePtr to hlINWin32Free so it'll be called directly from now on. */
    HlFreePtr = hlINWin32Free;

    /* Call hpINFree to actually free the given pointer. */
    hlINWin32Free(ptr);
}

HlAllocFunc HlAllocPtr = hlINWin32FirstAlloc;
HlReallocFunc HlReallocPtr = hlINWin32FirstRealloc;
HlFreeFunc HlFreePtr = hlINWin32FirstFree;
#else
#include <stdlib.h>
#include <string.h>

/* Standard memory allocation functions */
HlAllocFunc HlAllocPtr = malloc;
HlReallocFunc HlReallocPtr = realloc;
HlFreeFunc HlFreePtr = free;
#endif

void hlSetAllocators(HlAllocFunc alloc,
    HlReallocFunc realloc, HlFreeFunc free)
{
    HlAllocPtr = alloc;
    HlReallocPtr = realloc;
    HlFreePtr = free;
}

#ifndef HL_NO_EXTERNAL_WRAPPERS
void* hlAllocExt(size_t size)
{
    return hlAlloc(size);
}

void* hlReallocExt(void* ptr, size_t size)
{
    return hlRealloc(ptr, size);
}

void hlFreeExt(void* ptr)
{
    hlFree(ptr);
}
#endif
