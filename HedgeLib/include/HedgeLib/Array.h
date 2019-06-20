#pragma once
#include "HedgeLib.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hl_Array
{
    size_t Count;
    void* Data;
};

inline struct hl_Array hl_CreateArray(size_t count, size_t elementSize)
{
    return { count, malloc(elementSize * count) };
}

inline void hl_FreeArray(struct hl_Array* arr)
{
    free(arr->Data);
}

struct hl_PtrArray
{
    size_t Count;
    void** Data;
};

inline struct hl_PtrArray hl_CreatePtrArray(size_t count)
{
    return { count, (void**)calloc(count, sizeof(void*)) };
}

HL_API void hl_FreePtrArray(struct hl_PtrArray* arr);

#ifdef __cplusplus
}
#endif
