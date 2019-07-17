#pragma once
#include "../HedgeLib.h"
#include "../Array.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

enum hl_ArchiveType
{
    HL_ARC_TYPE_UNKNOWN = 0,
    HL_ARC_TYPE_HEROES,         // .one files from Heroes/Shadow the Hedgehog
    HL_ARC_TYPE_STORYBOOK,      // .one files from Secret Rings/Black Knight
    HL_ARC_TYPE_HEDGEHOG,       // .ar/.pfd files from Unleashed/Generations
    HL_ARC_TYPE_PACX_V2,        // .pac files from Lost World
    HL_ARC_TYPE_PACX_V3         // .pac files from Forces
};

HL_API enum hl_ArchiveType hl_GetArchiveTypeExt(const char* ext);
HL_API bool hl_GetArchiveType(const char* filePath, enum hl_ArchiveType* type);
HL_API char* hl_GetRootArchivePath(const char* filePath);
HL_API struct hl_Blob* hl_LoadArchiveOfType(
    const char* filePath, enum hl_ArchiveType type);

HL_API struct hl_Blob* hl_LoadArchive(const char* filePath);
HL_API struct hl_Blob* hl_LoadRootArchive(const char* filePath);

struct hl_ArchiveFileEntry
{
    size_t Size;        // The size of the data, or 0 if Data points to a file name.
    const char* Name;   // The name of the file + its extension. Does NOT include the path!!
    const void* Data;   // Points to the file path if Size == 0, or the file's data if Size != 0.
};

HL_API const char** hl_ArchiveGetSplits(const struct hl_Blob* blob, size_t* splitCount);

// TODO: Should this return an HL_RESULT?
HL_API void hl_ExtractArchive(const struct hl_Blob* blob, const char* dir);
HL_API void hl_ExtractArchives(const char* filePath, const char* dir);

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
