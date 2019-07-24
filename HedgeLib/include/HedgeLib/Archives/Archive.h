#pragma once
#include "../HedgeLib.h"
#include "../Array.h"
#include "../Errors.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

enum hl_ArchiveType
{
    // These values were picked such that you can safely do
    // (type & HL_ARC_TYPE_ONE) or (type & HL_ARC_TYPE_PACX)
    HL_ARC_TYPE_UNKNOWN = 0,
    HL_ARC_TYPE_ONE = 1,            // .one files
    HL_ARC_TYPE_PACX = 2,           // .pac files

    HL_ARC_TYPE_HEROES = 17,        // .one files from Heroes/Shadow the Hedgehog
    HL_ARC_TYPE_S06 = 16,           // .arc files from Sonic '06
    HL_ARC_TYPE_STORYBOOK = 33,     // .one files from Secret Rings/Black Knight
    HL_ARC_TYPE_HEDGEHOG = 32,      // .ar/.pfd files from Unleashed/Generations
    HL_ARC_TYPE_COLORS = 48,        // .arc files from Sonic Colors
    HL_ARC_TYPE_PACX_V2 = 18,       // .pac files from Lost World
    HL_ARC_TYPE_PACX_V3 = 34,       // .pac files from Forces
    HL_ARC_TYPE_TSR = 64,           // .cpu.sp2/.gpu.sp2 files from TSR
    HL_ARC_TYPE_PACX_V4 = 50        // .pac files from Tokyo 2020
};

struct hl_ArchiveFileEntry
{
    size_t Size;        // The size of the data, or 0 if Data points to a file name.
    const char* Name;   // The name of the file + its extension. Does NOT include the path!!
    const void* Data;   // Points to the file path if Size == 0, or the file's data if Size != 0.
};

HL_API bool hl_GetArchiveIsSplitExt(const char* ext);
HL_API bool hl_GetArchiveIsSplit(const char* filePath);
HL_API enum hl_ArchiveType hl_GetArchiveTypeExt(const char* ext);
HL_API bool hl_GetArchiveType(const char* filePath, enum hl_ArchiveType* type);
HL_API enum HL_RESULT hl_GetRootArchivePath(const char* splitPath, char** rootPath);
HL_API struct hl_Blob* hl_LoadArchiveOfType(
    const char* filePath, enum hl_ArchiveType type);

HL_API struct hl_Blob* hl_LoadArchive(const char* filePath);
HL_API struct hl_Blob* hl_LoadRootArchive(const char* filePath);

HL_API size_t hl_GetArchiveSplitCountRoot(const char* rootPath);
HL_API size_t hl_GetArchiveSplitCount(const char* filePath);

// TODO: Should this return an HL_RESULT?
HL_API void hl_ExtractArchive(const struct hl_Blob* blob, const char* dir);
HL_API void hl_ExtractArchivesOfType(const char* filePath,
    const char* dir, enum hl_ArchiveType type);

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
