#pragma once
#include "HedgeLib.h"
#include "Result.h"

#ifdef __cplusplus
#include <vector>
extern "C" {
#endif

typedef struct hl_StringTableEntry
{
    const char* String; // The string referenced by the DataOffset.
    long OffPosition;   // The position of the DataOffset which points to String within the file.
}
hl_StringTableEntry;

#ifdef __cplusplus
using hl_StringTable = std::vector<hl_StringTableEntry>;
#else
typedef struct hl_StringTable hl_StringTable;
#endif

HL_API hl_StringTable* hl_CreateStringTable();
HL_API HL_RESULT hl_AddString(hl_StringTable* strTable, const char* str, long offPos);
HL_API HL_RESULT hl_AddStringEntry(hl_StringTable* strTable, const hl_StringTableEntry* entry);
HL_API size_t hl_GetStringCount(const hl_StringTable* strTable);
HL_API hl_StringTableEntry* hl_GetStringEntriesPtr(hl_StringTable* strTable);
HL_API hl_StringTableEntry* hl_GetStrings(hl_StringTable* strTable, size_t* count);
HL_API void hl_DestroyStringTable(hl_StringTable* strTable);

#ifdef __cplusplus
}
#endif
