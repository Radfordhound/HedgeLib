#include "hedgelib/hl_list.h"
#include "hedgelib/hl_text.h"
#include <stdlib.h>

void* hlINListReserve(void* HL_RESTRICT data,
    size_t size, size_t count, size_t desiredCapacity,
    size_t* HL_RESTRICT capacity)
{
    void* newData;
    
    /* Only reserve if necessary. */
    if (*capacity > desiredCapacity)
        return data;

    /* If capacity != 0, the list owns its data pointer. Just realloc. */
    if (*capacity)
    {
        /* Reallocate the list's data buffer. */
        newData = hlRealloc(data, size * desiredCapacity);
        if (!newData) return NULL;
    }

    /* If capacity == 0, the list does not own its data pointer. Alloc and copy. */
    else
    {
        /* Allocate a new buffer for the list. */
        newData = hlAlloc(size * desiredCapacity);
        if (!newData) return NULL;

        /* Copy the existing data (if any) into the new buffer. */
        memcpy(newData, data, size * count);
    }

    /* Set new capacity and return new buffer. */
    *capacity = desiredCapacity;
    return newData;
}

void* hlINListGrow(void* HL_RESTRICT data,
    size_t size, size_t count, size_t* HL_RESTRICT capacity)
{
    /* If capacity != 0, the list owns its data pointer. Just realloc. */
    if (*capacity)
    {
        /* Double the list's capacity. */
        *capacity *= 2;

        /* Reallocate the list's data buffer. */
        return hlRealloc(data, size * (*capacity));
    }

    /* If capacity == 0, the list does not own its data pointer. Alloc and copy. */
    else
    {
        void* newData;

        /* Set the list's new capacity. */
        *capacity = (count) ? (count * 2) : 1;

        /* Allocate a new buffer for the list. */
        newData = hlAlloc(size * (*capacity));
        if (!newData) return NULL;

        /* Copy the existing data (if any) into the new buffer and return new buffer. */
        memcpy(newData, data, size * count);
        return newData;
    }
}

void hlINListRemove(void* data, size_t size,
    size_t count, size_t index)
{
    /*
       Move everything after the value we specified "back" by one.

       Example:

       = Remove(1)
       Indices:     0 1 2 3
       Before:      A B C D
       After:       A C D
                      ^

       count:  3   (NOTE: We subtracted 1 from count before calling this function.)
       index:  1

       A B C D % % % %
         ^
        dst

       A B C D % % % %
         --^
       dst + size

       [We're moving count - index (2) values from dst + size to dst.]
    */
    HlU8* dst = (((HlU8*)data) + (size * index));
    memmove(dst, dst + size, size * (count - index));
}

void hlStrListDestruct(HlStrList* strList)
{
    size_t i;
    for (i = 0; i < strList->count; ++i)
    {
        hlFree(strList->data[i]);
    }

    HL_LIST_FREE(*strList);
}

static int hlINOffTableCompareOffsets(const void* a, const void* b)
{
    const size_t off1 = *(const size_t*)a;
    const size_t off2 = *(const size_t*)b;

    return ((off1 > off2) - (off1 < off2));
}

void hlOffTableSort(HlOffTable* offTable)
{
    /* Sort offsets in offset table. */
    qsort(offTable->data, offTable->count,
        sizeof(size_t), hlINOffTableCompareOffsets);
}

HlResult hlStrTableAddStrRefNativeEx(HlStrTable* HL_RESTRICT strTable,
    const HlNChar* HL_RESTRICT str, size_t offPos, size_t len)
{
#ifdef HL_IN_WIN32_UNICODE
    return hlStrTableAddStrNativeEx(strTable, str, offPos, len);
#else
    return hlStrTableAddStrRefUTF8Ex(strTable, str, offPos, len);
#endif
}

HlResult hlStrTableAddStrRefNative(HlStrTable* HL_RESTRICT strTable,
    const HlNChar* HL_RESTRICT str, size_t offPos)
{
    return hlStrTableAddStrRefNativeEx(strTable,
        str, offPos, hlNStrLen(str));
}

HlResult hlStrTableAddStrRefUTF8Ex(HlStrTable* HL_RESTRICT strTable,
    const char* HL_RESTRICT str, size_t offPos, size_t len)
{
    /* Generate string table entry. */
    HlStrTableEntry strTableEntry;
    strTableEntry.flags = HL_STR_TABLE_ENTRY_FLAGS_NONE;
    strTableEntry.str = str;
    strTableEntry.offPos = offPos;
    strTableEntry.len = len;

    /* Add string table entry to string table and return result. */
    return HL_LIST_PUSH(*strTable, strTableEntry);
}

HlResult hlStrTableAddStrRefUTF8(HlStrTable* HL_RESTRICT strTable,
    const char* HL_RESTRICT str, size_t offPos)
{
    return hlStrTableAddStrRefUTF8Ex(strTable,
        str, offPos, strlen(str));
}

HlResult hlStrTableAddStrNativeEx(HlStrTable* HL_RESTRICT strTable,
    const HlNChar* HL_RESTRICT str, size_t offPos, size_t len)
{
    HlStrTableEntry strTableEntry;
    char* newStr;
    size_t dstLen;
    HlResult result;

    /* Create a copy of the given string. */
    /* NOTE: We don't require a null terminator in string table entries. */
    dstLen = hlStrGetReqLenNativeToUTF8(str, len);
    newStr = HL_ALLOC_ARR(char, dstLen);
    if (!newStr) return HL_ERROR_OUT_OF_MEMORY;

    hlStrConvNativeToUTF8NoAlloc(str, newStr, len, dstLen);

    /* Generate string table entry. */
    strTableEntry.flags = HL_STR_TABLE_ENTRY_FLAGS_DOES_OWN;
    strTableEntry.str = newStr;
    strTableEntry.offPos = offPos;
    strTableEntry.len = len;

    /* Add string table entry to string table and return result. */
    result = HL_LIST_PUSH(*strTable, strTableEntry);
    if (HL_FAILED(result))
    {
        hlFree(newStr);
        return result;
    }

    return result;
}

HlResult hlStrTableAddStrNative(HlStrTable* HL_RESTRICT strTable,
    const HlNChar* HL_RESTRICT str, size_t offPos)
{
    return hlStrTableAddStrNativeEx(strTable,
        str, offPos, hlNStrLen(str));
}

HlResult hlStrTableAddStrUTF8Ex(HlStrTable* HL_RESTRICT strTable,
    const char* HL_RESTRICT str, size_t offPos, size_t len)
{
    HlStrTableEntry strTableEntry;
    char* newStr;
    HlResult result;

    /* Create a copy of the given string. */
    /* NOTE: We don't require a null terminator in string table entries. */
    newStr = HL_ALLOC_ARR(char, len);
    if (!newStr) return HL_ERROR_OUT_OF_MEMORY;

    memcpy(newStr, str, sizeof(char) * len);

    /* Generate string table entry. */
    strTableEntry.flags = HL_STR_TABLE_ENTRY_FLAGS_DOES_OWN;
    strTableEntry.str = newStr;
    strTableEntry.offPos = offPos;
    strTableEntry.len = len;

    /* Add string table entry to string table and return result. */
    result = HL_LIST_PUSH(*strTable, strTableEntry);
    if (HL_FAILED(result))
    {
        hlFree(newStr);
        return result;
    }

    return result;
}

HlResult hlStrTableAddStrUTF8(HlStrTable* HL_RESTRICT strTable,
    const char* HL_RESTRICT str, size_t offPos)
{
    return hlStrTableAddStrUTF8Ex(strTable,
        str, offPos, strlen(str));
}

void hlStrTableDestruct(HlStrTable* strTable)
{
    size_t i;
    for (i = 0; i < strTable->count; ++i)
    {
        HlStrTableEntry* entry = &strTable->data[i];
        if (entry->flags & HL_STR_TABLE_ENTRY_FLAGS_DOES_OWN)
        {
            hlFree((char*)entry->str);
        }
    }

    HL_LIST_FREE(*strTable);
}
