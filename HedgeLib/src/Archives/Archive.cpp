#include "HedgeLib/Archives/Archive.h"
#include "HedgeLib/Archives/PACx.h"
#include "HedgeLib/Archives/LWArchive.h"
#include "HedgeLib/IO/Path.h"
#include "HedgeLib/String.h"
#include "../IO/INPath.h"
#include "../INBlob.h"
#include <cstdlib>
#include <cctype>

enum hl_ArchiveType hl_INGetArchiveTypeExt(const char* ext)
{
    // Lost World
    if (hl_StringsEqualASCII(ext, hl_PACxExtension))
    {
        return HL_ARC_TYPE_PACX_V2;
    }

    // TODO: Support other Archive types
    return HL_ARC_TYPE_UNKNOWN;
}

enum hl_ArchiveType hl_GetArchiveTypeExt(const char* ext)
{
    if (!ext || !*ext) return HL_ARC_TYPE_UNKNOWN;
    return hl_INGetArchiveTypeExt(ext);
}

bool hl_INGetArchiveType(const char* filePath, enum hl_ArchiveType* type)
{
    // Get file name pointer
    filePath = hl_INPathGetNamePtr(filePath);
    if (!*filePath) return false;

    // Check extension
    const char* ext = hl_INPathGetExtPtrName(filePath);

    // Check if this is a split
    if (ext[0] == '.' && std::isdigit(ext[1]) && std::isdigit(ext[2]))
    {
        // Check root extension
        bool pacxV3 = std::isdigit(ext[3]); // PACX V3 splits have three digits
        size_t i = 3;

        *type = HL_ARC_TYPE_UNKNOWN;
        while (--ext > filePath && i)
        {
            // On first iteration, get root type
            if (*type == HL_ARC_TYPE_UNKNOWN)
            {
                if (HL_TOLOWERASCII(*ext) == hl_PACxExtension[i--])     // c
                {
                    *type = (pacxV3) ? HL_ARC_TYPE_PACX_V3 :
                        HL_ARC_TYPE_PACX_V2;
                }
                // TODO: Add Hedgehog Engine Archive support and un-comment this
                //else if (HL_TOLOWERASCII(*ext) == hl_ARExtension[i--])  // r
                //{
                //    type = HL_ARC_TYPE_HEDGEHOG;
                //}
                else break;
                continue;
            }

            // Check that each character of the extension continues to match
            else if (pacxV3 || *type == HL_ARC_TYPE_PACX_V2)
            {
                if (HL_TOLOWERASCII(*ext) == hl_PACxExtension[i--])     // pa
                {
                    // Break if we've reached the . in .pac and the extension is still valid
                    if (!i && --ext > filePath && *ext == hl_PACxExtension[i]) break;
                    continue;
                }
            }
            // TODO: Add Hedgehog Engine Archive support and un-comment this
            //else if (HL_TOLOWERASCII(*ext) == hl_ARExtension[i--] &&    // a
            //    *ext == hl_ARExtension[i])                              // .
            //{
            //    break;
            //}

            // The extension is invalid
            *type = HL_ARC_TYPE_UNKNOWN;
            break;
        }

        return true;
    }

    // Otherwise, just check extension
    *type = hl_GetArchiveTypeExt(ext);
    return false;
}

bool hl_GetArchiveType(const char* filePath, enum hl_ArchiveType* type)
{
    if (!filePath || !type) return false;
    return hl_INGetArchiveType(filePath, type);
}

char* hl_INGetRootArchivePath(const char* filePath)
{
    char* rootPath;
    return (HL_FAILED(hl_INPathRemoveExt<false>(
        filePath, &rootPath))) ? nullptr : rootPath;
}

char* hl_GetRootArchivePath(const char* filePath)
{
    if (!filePath) return nullptr;
    return hl_INGetRootArchivePath(filePath);
}

struct hl_Blob* hl_INLoadArchiveOfType(const char* filePath,
    enum hl_ArchiveType type)
{
    // Load archive based on type
    hl_Blob* blob;
    switch (type)
    {
        // Lost World
    case HL_ARC_TYPE_PACX_V2:
    {
        hl_BINALoad(filePath, &blob); // TODO: Error check
        if (hl_BINAIsBigEndian(blob))
        {
            hl_DLWArchive* arc = hl_BINAGetDataV2<hl_DLWArchive>(blob);
            arc->EndianSwapRecursive(true);
        }
        return blob;
    }

    // TODO: Add other Archive Types

    default:
        return nullptr;
    }
}

struct hl_Blob* hl_INLoadRootArchiveOfType(const char* filePath,
    enum hl_ArchiveType type, bool isSplit)
{
    // Get root path if filePath is the path of a split
    if (isSplit)
    {
        // Load root archive instead
        char* rootPath = hl_INGetRootArchivePath(filePath);
        hl_Blob* arc = hl_INLoadArchiveOfType(rootPath, type);
        std::free(rootPath);
        return arc;
    }

    // Load archive
    return hl_INLoadArchiveOfType(filePath, type);
}

struct hl_Blob* hl_LoadArchiveOfType(
    const char* filePath, enum hl_ArchiveType type)
{
    if (!filePath || !*filePath) return nullptr;
    return hl_INLoadArchiveOfType(filePath, type);
}

struct hl_Blob* hl_LoadArchive(const char* filePath)
{
    if (!filePath) return nullptr;

    // Get archive type
    hl_ArchiveType type;
    hl_INGetArchiveType(filePath, &type);

    // Load archive
    return hl_INLoadArchiveOfType(filePath, type);
}

struct hl_Blob* hl_LoadRootArchive(const char* filePath)
{
    if (!filePath) return nullptr;

    // Get archive type
    hl_ArchiveType type;
    bool isSplit = hl_INGetArchiveType(filePath, &type);

    if (type == HL_ARC_TYPE_UNKNOWN) return nullptr;

    // Load archive
    return hl_INLoadRootArchiveOfType(filePath, type, isSplit);
}

const char** hl_ArchiveGetSplits(const struct hl_Blob* blob, size_t* splitCount)
{
    if (!blob) return nullptr;
    switch (blob->Type)
    {
    // .ar/.pfd (Unleashed/Generations)
    case HL_BLOB_TYPE_HEDGEHOG_ENGINE:
        // TODO: AR Support
        return nullptr;

    // .pac (LW/Forces)
    case HL_BLOB_TYPE_BINA:
        return hl_PACxArchiveGetSplits(blob, splitCount);

    default:
        return nullptr;
    }
}

void hl_ExtractArchive(const struct hl_Blob* blob, const char* dir)
{
    if (!blob) return;
    switch (blob->Type)
    {
    // .ar/.pfd (Unleashed/Generations)
    case HL_BLOB_TYPE_HEDGEHOG_ENGINE:
        // TODO: AR Support
        break;

    // .pac (LW/Forces)
    case HL_BLOB_TYPE_BINA:
        hl_ExtractPACxArchive(blob, dir);
        break;

    default:
        // TODO: Should we return an error or something??
        break;
    }
}

void hl_ExtractArchives(const char* filePath, const char* dir)
{
    if (!filePath || !dir) return; // TODO: Return error

    // Get archive type
    hl_ArchiveType type;
    bool isSplit = hl_INGetArchiveType(filePath, &type);

    if (type == HL_ARC_TYPE_UNKNOWN) return; // TODO: Return error

    // Extract root archive
    hl_Blob* arc = hl_INLoadRootArchiveOfType(filePath, type, isSplit);
    hl_ExtractArchive(arc, dir);

    // Extract splits
    size_t splitCount = 0;
    const char** splits = hl_ArchiveGetSplits(arc, &splitCount);

    if (splitCount)
    {
        hl_Blob* splitArc;
        char* rootDir, *splitPath;
        hl_INPathGetParent(filePath, &rootDir); // TODO: Error check

        for (size_t i = 0; i < splitCount; ++i)
        {
            // Get split path
            hl_INPathCombine(rootDir, splits[i], &splitPath); // TODO: Error check

            // Extract split
            splitArc = hl_INLoadArchiveOfType(splitPath, type);
            hl_ExtractArchive(splitArc, dir);
            std::free(splitPath);
        }

        std::free(rootDir);
    }

    // Free root archive and splits list
    std::free(splits);
    hl_FreeBlob(arc);
}

void hl_CreateArchiveFileEntry(const char* filePath,
    struct hl_ArchiveFileEntry* entry)
{
    // Create the file entry
    *entry =
    {
        //HL_ARCHIVE_FILE_ENTRY_TYPE_FILEPATH,
        0, hl_PathGetNamePtr(filePath),
        filePath
    };
}

struct hl_ArchiveFileEntry* hl_CreateArchiveFileEntries(
    const char** files, size_t fileCount)
{
    // Create file entries
    hl_ArchiveFileEntry* fileEntries = static_cast<hl_ArchiveFileEntry*>(
        std::malloc(sizeof(hl_ArchiveFileEntry) * fileCount));

    for (size_t i = 0; i < fileCount; ++i)
    {
        hl_CreateArchiveFileEntry(files[i], &(fileEntries[i]));
    }

    return fileEntries;
}
