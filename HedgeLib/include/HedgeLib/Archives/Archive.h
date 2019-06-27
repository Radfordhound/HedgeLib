#pragma once
#include "../HedgeLib.h"
#include "../Array.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hl_ArchiveFileEntry
{
    size_t Size;        // The size of the data, or 0 if Data points to a file name.
    const char* Name;   // The name of the file + its extension. Does NOT include the path!!
    const void* Data;   // Points to the file path if Size == 0, or the file's data if Size != 0.
};

// TODO: Should this return an HL_RESULT?
HL_API void hl_ExtractArchive(const struct hl_Blob* blob, const char* dir);
HL_API void hl_CreateArchiveFileEntry(const char* filePath,
    struct hl_ArchiveFileEntry* entry);

inline void hl_CreateArchiveFileEntryData(size_t dataSize,
    const char* name, const void* data, struct hl_ArchiveFileEntry* entry)
{
    *entry =
    {
        dataSize, name, data
    };
}

HL_API struct hl_ArchiveFileEntry* hl_CreateArchiveFileEntries(
    const char** files, size_t fileCount);

inline struct hl_ArchiveFileEntry* hl_CreateArchiveFileEntriesArr(
    struct hl_PtrArray* arr)
{
    return hl_CreateArchiveFileEntries(
        (const char**)arr->Data, arr->Count);
}

#ifdef __cplusplus
}
#endif
