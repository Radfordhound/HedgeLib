#include "HedgeLib/StringTable.h"

hl_StringTable* hl_CreateStringTable()
{
    return new hl_StringTable();
}

void hl_AddString(hl_StringTable* strTable, char* str, long offPos)
{
    strTable->push_back({ str, offPos });
}

void hl_AddStringEntry(hl_StringTable* strTable, const struct hl_StringTableEntry* entry)
{
    strTable->push_back(*entry);
}

size_t hl_GetStringCount(const hl_StringTable* strTable)
{
    return strTable->size();
}

struct hl_StringTableEntry* hl_GetStringEntriesPtr(hl_StringTable* strTable)
{
    return strTable->data();
}

struct hl_StringTableEntry* hl_GetStrings(hl_StringTable* strTable, size_t* count)
{
    *count = strTable->size();
    return strTable->data();
}

void hl_FreeStringTable(hl_StringTable* strTable)
{
    delete strTable;
}
