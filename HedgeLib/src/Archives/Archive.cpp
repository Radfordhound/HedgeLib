#include "HedgeLib/Archives/Archive.h"
#include "HedgeLib/Archives/PACx.h"
#include "HedgeLib/Archives/LWArchive.h"
#include "HedgeLib/IO/Path.h"
#include "HedgeLib/String.h"
#include "../IO/INBINA.h"
#include "../IO/INPath.h"
#include "../INBlob.h"
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <algorithm>

bool hl_INGetArchiveIsSplitExt(const char* ext)
{
    return (ext[0] == '.' && std::isdigit(ext[1]) && std::isdigit(ext[2]));
}

bool hl_GetArchiveIsSplitExt(const char* ext)
{
    if (!ext) return false;
    return hl_INGetArchiveIsSplitExt(ext);
}

bool hl_GetArchiveIsSplit(const char* filePath)
{
    // Get extension
    const char* ext = hl_PathGetExtPtr(filePath);
    if (!*ext) return 0;

    // Return whether or not the archive is a split
    return hl_GetArchiveIsSplitExt(ext);
}

enum hl_ArchiveType hl_INGetArchiveTypeExt(const char* ext)
{
    // Lost World
    if (hl_StringsEqualASCII(ext, hl_PACxExtension))
    {
        return HL_ARC_TYPE_PACX;
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
    if (hl_INGetArchiveIsSplitExt(ext))
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

HL_RESULT hl_INGetRootArchivePath(const char* splitPath, char** rootPath)
{
    return hl_INPathRemoveExt<false>(splitPath, rootPath);
}

enum HL_RESULT hl_GetRootArchivePath(const char* splitPath, char** rootPath)
{
    if (!splitPath || !rootPath) return HL_ERROR_UNKNOWN;
    return hl_INGetRootArchivePath(splitPath, rootPath);
}

struct hl_Blob* hl_INLoadArchiveOfType(
    const hl_INNativeStr filePath, enum hl_ArchiveType type)
{
    // Load archive based on type
    hl_Blob* blob;
    switch (type)
    {
    // Unknown PACx
    case HL_ARC_TYPE_PACX:
    {
        // Figure out what type of pac this is
        hl_INBINALoad(filePath, &blob); // TODO: Error check
        uint8_t version = blob->GetData<hl_DBINAV2Header>()->Version[0];
        
        if (version == 0x32) // '2'
        {
            goto HL_PACX_V2;
        }
        // TODO: Add Forces archive support and un-comment this
        /*else if (version == 0x33) // '3'
        {
            goto HL_PACX_V3;
        }*/
        else
        {
            hl_BINAFreeBlob(blob);
            return nullptr;
        }
    }

    // Lost World
    case HL_ARC_TYPE_PACX_V2:
    {
        hl_INBINALoad(filePath, &blob); // TODO: Error check
        
    HL_PACX_V2:
        blob->Type = static_cast<uint16_t>(HL_ARC_TYPE_PACX_V2);
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

#ifdef _WIN32
struct hl_Blob* hl_INLoadArchiveOfType(
    const char* filePath, enum hl_ArchiveType type)
{
    hl_INNativeStr nativePath;
    if (HL_FAILED(hl_INWin32StringConvertToNative(filePath, &nativePath)))
        return nullptr;

    hl_Blob* blob = hl_INLoadArchiveOfType(nativePath, type);
    std::free(nativePath);
    return blob;
}
#endif

struct hl_Blob* hl_INLoadRootArchiveOfType(const char* filePath,
    enum hl_ArchiveType type, bool isSplit)
{
    // Get root path if filePath is the path of a split
    if (isSplit)
    {
        // Load root archive instead
        char* rootPath;
        if (HL_FAILED(hl_INGetRootArchivePath(filePath, &rootPath)))
            return nullptr;

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

bool hl_INArchiveNextSplit(hl_INNativeStr splitCharPtr, bool pacv3)
{
    // Weird code, I know. ':' comes after '9'
    if (++(*splitCharPtr) == HL_INNATIVE_TEXT(':'))
    {
        if (++(*(splitCharPtr - 1)) == HL_INNATIVE_TEXT(':'))
        {
            // Check extra digit in Forces splits as well
            if (!pacv3 || ++(*(splitCharPtr - 2)) == HL_INNATIVE_TEXT(':'))
            {
                return false;
            }

            // Reset second digit in Forces splits
            *(splitCharPtr - 1) = HL_INNATIVE_TEXT('0');
        }

        // Reset second digit in split (third digit in Forces splits)
        *splitCharPtr = HL_INNATIVE_TEXT('0');
    }
    
    return true;
}

size_t hl_INGetArchiveSplitCountRoot(const char* rootPath, const char* ext)
{
    // Get pointer to the end of rootPath and determine if this is a pac file
    size_t i = 0;
    bool pacv3 = true;

    while (*ext)
    {
        if (pacv3 && *ext != hl_PACxExtension[i++])
        {
            pacv3 = false;
        }

        ++ext;
    }
    
    if (pacv3) pacv3 = (i == 3);

    // Get size of root path
    // (ext points to the end of the path now because of that for loop)
    size_t rootPathLen = (static_cast<size_t>(ext - rootPath) + 1);

    // Get size of split path
    size_t splitPathLen = (rootPathLen + 3);

    // Increase split path length by 1 if this is a pac file
    // since Forces pacs use three digits for the split index.
    if (pacv3) ++splitPathLen;

    // Create split path buffer
    hl_INNativeStr splitPath = HL_INCREATE_NATIVE_STR(splitPathLen);

#ifdef _WIN32
    // Convert root path to UTF-16 and copy into split path buffer
    hl_INStringConvertUTF8ToUTF16NoAlloc(rootPath,
        reinterpret_cast<uint16_t*>(splitPath)); // TODO: Error check
#else
    // Copy root path into split path buffer
    std::copy(rootPath, ext, splitPath);
#endif

    // Change split path extension
    splitPath[rootPathLen - 1] = HL_INNATIVE_TEXT('.');
    splitPath[rootPathLen]     = HL_INNATIVE_TEXT('0');
    splitPath[rootPathLen + 1] = HL_INNATIVE_TEXT('0');
    splitPath[rootPathLen + 2] = HL_INNATIVE_TEXT('\0');

    // If this is a pac file, determine if this is a Forces pac
    size_t maxSplitIndex = 99;
    hl_INNativeStr splitCharPtr = (splitPath + rootPathLen + 1);
    i = 0;

    if (pacv3)
    {
        // Use extra byte we allocated earlier to add an extra number to the extension
        // (E.G. Sonic.pac.00 -> Sonic.pac.000)
        splitPath[rootPathLen + 2] = HL_INNATIVE_TEXT('0');
        splitPath[rootPathLen + 3] = HL_INNATIVE_TEXT('\0');

        // Determine if this is a forces pac by checking for a split with a three-digit split index
        pacv3 = hl_INPathExists(splitPath);

        // If it isn't a forces pac, get rid of that third number
        if (!pacv3)
        {
            splitPath[rootPathLen + 2] = HL_INNATIVE_TEXT('\0');
            ++splitCharPtr;
        }
        else
        {
            maxSplitIndex = 999;
            i = 1; // We already checked the first split, no need to check again
        }
    }

    // Get the split count
    for (; i <= maxSplitIndex; ++i)
    {
        // Check if this split exists and get the next split, if any
        if (!hl_INPathExists(splitPath) || !hl_INArchiveNextSplit(
            splitCharPtr, pacv3)) break;
    }

    std::free(splitPath);
    return ++i;
}

size_t hl_GetArchiveSplitCountRoot(const char* rootPath)
{
    // Get extension
    const char* ext = hl_PathGetExtPtr(rootPath);
    if (!*ext) return 0;

    // Get split count and return
    return hl_INGetArchiveSplitCountRoot(rootPath, ext);
}

size_t hl_GetArchiveSplitCount(const char* filePath)
{
    // Get extension
    const char* ext = hl_PathGetExtPtr(filePath);
    if (!*ext) return 0;

    // Check if this is a split
    if (hl_INGetArchiveIsSplitExt(ext))
    {
        // Get root path
        char* rootPath;
        if (HL_FAILED(hl_INGetRootArchivePath(filePath, &rootPath)))
            return 0;

        // Get split count from root path and return
        size_t splitCount = hl_GetArchiveSplitCountRoot(rootPath);
        std::free(rootPath);
        return splitCount;
    }

    // Get split count and return
    return hl_INGetArchiveSplitCountRoot(filePath, ext);
}

void hl_ExtractArchive(const struct hl_Blob* blob, const char* dir)
{
    if (!blob) return; // TODO: Return error
    switch (blob->Type)
    {
    // .ar/.pfd (Unleashed/Generations)
    case HL_ARC_TYPE_HEDGEHOG:
        // TODO: AR Support
        break;

    // .pac (LW/Forces/Tokyo 2020)
    case HL_ARC_TYPE_PACX:
        hl_ExtractPACxArchive(blob, dir);
        break;

    // .pac V2 (LW)
    case HL_ARC_TYPE_PACX_V2:
        hl_ExtractLWArchive(blob, dir);
        break;

    // .pac V3 (Forces)
    case HL_ARC_TYPE_PACX_V3:
        // TODO: Forces Archives
        //hl_ExtractForcesArchive(blob, dir);
        break;

    // .pac V4 (Tokyo 2020)
    case HL_ARC_TYPE_PACX_V4:
        // TODO: Tokyo 2020 Archives
        //hl_ExtractTokyoArchive(blob, dir);
        break;
    }

    // Attempt to auto-detect archive type
    if (blob->Format == HL_BLOB_FORMAT_BINA)
    {
        // Attempt to extract BINA file as a pac
        hl_ExtractPACxArchive(blob, dir);
        return;
    }

    // TODO: Return error
}

void hl_INExtractArchivesOfTypeRoot(const char* rootPath,
    const char* dir, hl_ArchiveType type)
{
    // Extract root
    hl_Blob* arc = hl_INLoadArchiveOfType(rootPath, type); // TODO: Nullptr check
    hl_ExtractArchive(arc, dir);

    // Extract splits
    size_t splitCount;
    if (type & HL_ARC_TYPE_PACX)
    {
        // Get splits list from data in root PAC
        hl_Blob* splitArc;
        char* rootDir, *splitPath;
        const char** splits = hl_PACxArchiveGetSplits(arc, &splitCount);

        if (!splits)
        {
            // There are no splits to extract
            hl_FreeBlob(arc);
            return;
        }

        // Extract splits
        hl_INPathGetParent(rootPath, &rootDir); // TODO: Error check

        for (size_t i = 0; i < splitCount; ++i)
        {
            // Get split path
            hl_INPathCombine(rootDir, splits[i], &splitPath); // TODO: Error check

            // Extract split
            splitArc = hl_INLoadArchiveOfType(splitPath, type);
            hl_ExtractArchive(splitArc, dir);
            std::free(splitPath);
        }

        // Free data
        std::free(rootDir);
        std::free(splits);
        hl_FreeBlob(arc);
    }
    else
    {
        // Free root archive
        hl_FreeBlob(arc);

        // Get size of paths
        size_t rootPathLen = (std::strlen(rootPath) + 1);
        size_t splitPathLen = (rootPathLen + 3);

        // Create split path buffer
        hl_INNativeStr splitPath = HL_INCREATE_NATIVE_STR(splitPathLen);

#ifdef _WIN32
        // Convert root path to UTF-16 and copy into split path buffer
        hl_INStringConvertUTF8ToUTF16NoAlloc(rootPath,
            reinterpret_cast<uint16_t*>(splitPath)); // TODO: Error check
#else
        // Copy root path into split path buffer
        std::copy(rootPath, rootPath + rootPathLen, splitPath);
#endif

        // Change split path extension
        splitPath[rootPathLen - 1] = HL_INNATIVE_TEXT('.');
        splitPath[rootPathLen]     = HL_INNATIVE_TEXT('0');
        splitPath[rootPathLen + 1] = HL_INNATIVE_TEXT('0');
        splitPath[rootPathLen + 2] = HL_INNATIVE_TEXT('\0');

        // Get the split count
        size_t maxSplitIndex = 99;
        hl_INNativeStr splitCharPtr = (splitPath + rootPathLen + 1);

        for (size_t i = 0; i <= 99; ++i)
        {
            // Check if this split exists
            if (!hl_INPathExists(splitPath)) break;

            // Extract split
            arc = hl_INLoadArchiveOfType(splitPath, type);
            hl_ExtractArchive(arc, dir);
            hl_FreeBlob(arc);

            // Get next split, if any
            if (!hl_INArchiveNextSplit(splitCharPtr, false)) break;
        }

        std::free(splitPath);
    }
}

void hl_INExtractArchivesOfType(const char* filePath,
    const char* dir, hl_ArchiveType type, bool isSplit)
{
    if (isSplit)
    {
        // This is a split; load the root archive instead
        char* rootPath;
        if (HL_FAILED(hl_INGetRootArchivePath(filePath, &rootPath)))
            return; // TODO: Return error

        // Extract root archive and its splits
        hl_INExtractArchivesOfTypeRoot(rootPath, dir, type);
        std::free(rootPath);
    }
    else
    {
        // Extract root archive and its splits
        hl_INExtractArchivesOfTypeRoot(filePath, dir, type);
    }
}

void hl_ExtractArchivesOfType(const char* filePath,
    const char* dir, enum hl_ArchiveType type)
{
    if (!dir) return; // TODO: Return error

    const char* ext = hl_PathGetExtPtr(filePath);
    hl_INExtractArchivesOfType(filePath, dir,
        type, hl_INGetArchiveIsSplitExt(ext));
}

void hl_ExtractArchives(const char* filePath, const char* dir)
{
    if (!filePath || !dir) return; // TODO: Return error

    // Get archive type
    hl_ArchiveType type;
    bool isSplit = hl_INGetArchiveType(filePath, &type);

    if (type == HL_ARC_TYPE_UNKNOWN) return; // TODO: Return error

    // Extract archives
    hl_INExtractArchivesOfType(filePath, dir, type, isSplit);
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
