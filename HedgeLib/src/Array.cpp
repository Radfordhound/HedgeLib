#include "HedgeLib/Array.h"

void hl_FreePtrArray(struct hl_PtrArray* arr)
{
    if (!arr) return;
    for (size_t i = 0; i < arr->Count; ++i)
    {
        free(arr->Data[i]);
    }

    free(arr->Data);
}
