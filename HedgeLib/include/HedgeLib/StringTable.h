#pragma once
#include "HedgeLib.h"

#ifdef __cplusplus
#include <vector>
extern "C" {
#endif

struct hl_StringTableEntry
{
    char* String;       // The string referenced by the DataOffset.
    long OffPosition;   // The position of the DataOffset which points to String within the file.
};

#ifdef __cplusplus
using hl_StringTable = std::vector<hl_StringTableEntry>;
#else
struct hl_StringTable; // TODO: Is this line needed?
typedef struct hl_StringTable hl_StringTable;
#endif

HL_API hl_StringTable* hl_CreateStringTable();
HL_API void hl_AddString(hl_StringTable* strTable, char* str, long offPos);
HL_API void hl_AddStringEntry(hl_StringTable* strTable, const struct hl_StringTableEntry* entry);
HL_API size_t hl_GetStringCount(const hl_StringTable* strTable);
HL_API struct hl_StringTableEntry* hl_GetStringEntriesPtr(hl_StringTable* strTable);
HL_API struct hl_StringTableEntry* hl_GetStrings(hl_StringTable* strTable, size_t* count);
HL_API void hl_FreeStringTable(hl_StringTable* strTable);

#ifdef __cplusplus
}
#endif
