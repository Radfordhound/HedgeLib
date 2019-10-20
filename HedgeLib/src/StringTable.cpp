#include "HedgeLib/StringTable.h"
#include <new>

hl_StringTable* hl_CreateStringTable()
{
    return new (std::nothrow) hl_StringTable();
}

HL_RESULT hl_AddString(hl_StringTable* strTable, const char* str, long offPos)
{
    try
    {
        strTable->push_back({ str, offPos });
    }
    catch (const std::bad_alloc&)
    {
        return HL_ERROR_OUT_OF_MEMORY;
    }

    return HL_SUCCESS;
}

HL_RESULT hl_AddStringEntry(hl_StringTable* strTable, const hl_StringTableEntry* entry)
{
    try
    {
        strTable->push_back(*entry);
    }
    catch (const std::bad_alloc&)
    {
        return HL_ERROR_OUT_OF_MEMORY;
    }

    return HL_SUCCESS;
}

size_t hl_GetStringCount(const hl_StringTable* strTable)
{
    return strTable->size();
}

hl_StringTableEntry* hl_GetStringEntriesPtr(hl_StringTable* strTable)
{
    return strTable->data();
}

hl_StringTableEntry* hl_GetStrings(hl_StringTable* strTable, size_t* count)
{
    *count = strTable->size();
    return strTable->data();
}

void hl_DestroyStringTable(hl_StringTable* strTable)
{
    delete strTable;
}
