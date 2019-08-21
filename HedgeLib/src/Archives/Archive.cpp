#include "HedgeLib/Archives/Archive.h"
#include "HedgeLib/Archives/PACx.h"
#include "HedgeLib/Archives/HHArchive.h"
#include "HedgeLib/Archives/LWArchive.h"
#include "HedgeLib/IO/Path.h"
#include "../IO/INBINA.h"
#include "../IO/INPath.h"
#include "../INBlob.h"
#include "../INString.h"
#include <cctype>
#include <cstring>
#include <algorithm>

HL_RESULT hl_INLoadHHArchive(
    const hl_NativeStr filePath, hl_Blob** blob);

template<typename char_t>
bool hl_INGetArchiveIsSplitExt(const char_t* ext)
{
    return (ext[0] == '.' && std::isdigit(ext[1]) && std::isdigit(ext[2]));
}

bool hl_GetArchiveIsSplitExt(const char* ext)
{
    if (!ext) return false;
    return hl_INGetArchiveIsSplitExt(ext);
}

bool hl_GetArchiveIsSplitExtNative(const hl_NativeStr ext)
{
    if (!ext) return false;
    return hl_INGetArchiveIsSplitExt(ext);
}

template<typename char_t>
bool hl_INGetArchiveIsSplit(const char_t* filePath)
{
    // Get extension
    const char_t* ext = hl_PathGetExtPtr(filePath);
    if (!*ext) return 0;

    // Return whether or not the archive is a split
    return hl_INGetArchiveIsSplitExt(ext);
}

bool hl_GetArchiveIsSplit(const char* filePath)
{
    return hl_INGetArchiveIsSplit(filePath);
}

bool hl_GetArchiveIsSplitNative(const hl_NativeStr filePath)
{
    return hl_INGetArchiveIsSplit(filePath);
}

template<typename char_t>
hl_ArchiveType hl_INGetArchiveTypeExt(const char_t* ext)
{
    // Hedgehog Engine
    if (hl_INStringsEqualInvASCII(ext, hl_ARExtension) ||
        hl_INStringsEqualInvASCII(ext, hl_PFDExtension))
    {
        return HL_ARC_TYPE_HEDGEHOG;
    }

    // PACx
    if (hl_INStringsEqualInvASCII(ext, hl_PACxExtension))
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

enum hl_ArchiveType hl_GetArchiveTypeExtNative(const hl_NativeStr ext)
{
    if (!ext || !*ext) return HL_ARC_TYPE_UNKNOWN;
    return hl_INGetArchiveTypeExt(ext);
}

template<typename char_t>
bool hl_INGetArchiveType(const char_t* filePath, hl_ArchiveType* type)
{
    // Get file name pointer
    filePath = hl_INPathGetNamePtr(filePath);
    if (!*filePath) return false;

    // Check extension
    const char_t* ext = hl_INPathGetExtPtrName(filePath);

    // Check if this is a split
    if (hl_INGetArchiveIsSplitExt(ext))
    {
        // Check root extension
        bool pacxV3 = std::isdigit(static_cast<int>(ext[3])); // PACX V3 splits have three digits
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
                else if (HL_TOLOWERASCII(*ext) == hl_ARExtension[i--])  // r
                {
                    *type = HL_ARC_TYPE_HEDGEHOG;
                }
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
            else if (HL_TOLOWERASCII(*ext) == hl_ARExtension[i--] &&    // a
                *(--ext) == hl_ARExtension[i])                              // .
            {
                break;
            }

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

bool hl_GetArchiveTypeNative(const hl_NativeStr filePath, enum hl_ArchiveType* type)
{
    if (!filePath || !type) return false;
    return hl_INGetArchiveType(filePath, type);
}

template<typename char_t>
HL_RESULT hl_INGetRootArchivePath(const char_t* splitPath, char_t** rootPath)
{
    return hl_INPathRemoveExt<char_t, false>(splitPath, rootPath);
}

enum HL_RESULT hl_GetRootArchivePath(const char* splitPath, char** rootPath)
{
    if (!splitPath || !rootPath) return HL_ERROR_UNKNOWN;
    return hl_INGetRootArchivePath(splitPath, rootPath);
}

enum HL_RESULT hl_GetRootArchivePathNative(
    const hl_NativeStr splitPath, hl_NativeStr* rootPath)
{
    if (!splitPath || !rootPath) return HL_ERROR_UNKNOWN;
    return hl_INGetRootArchivePath(splitPath, rootPath);
}

hl_Blob* hl_INLoadArchiveOfType(
    const hl_NativeStr filePath, hl_ArchiveType type)
{
    // Load archive based on type
    hl_Blob* blob;
    switch (type)
    {
    // Unleashed/Generations
    case HL_ARC_TYPE_HEDGEHOG:
    {
        hl_INLoadHHArchive(filePath, &blob); // TODO: Error check
        return blob;
    }

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
hl_Blob* hl_INLoadArchiveOfType(
    const char* filePath, hl_ArchiveType type)
{
    hl_NativeStr nativePath;
    if (HL_FAILED(hl_INStringConvertUTF8ToNative(filePath, &nativePath)))
        return nullptr;

    hl_Blob* blob = hl_INLoadArchiveOfType(nativePath, type);
    free(nativePath);
    return blob;
}
#endif

template<typename char_t>
hl_Blob* hl_INLoadRootArchiveOfType(const char_t* filePath,
    hl_ArchiveType type, bool isSplit)
{
    // Get root path if filePath is the path of a split
    if (isSplit)
    {
        // Load root archive instead
        char_t* rootPath;
        if (HL_FAILED(hl_INGetRootArchivePath(filePath, &rootPath)))
            return nullptr;

        hl_Blob* arc = hl_INLoadArchiveOfType(rootPath, type);
        free(rootPath);
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

struct hl_Blob* hl_LoadArchiveOfTypeNative(
    const hl_NativeStr filePath, enum hl_ArchiveType type)
{
    if (!filePath || !*filePath) return nullptr;
    return hl_INLoadArchiveOfType(filePath, type);
}

template<typename char_t>
hl_Blob* hl_INLoadArchive(const char_t* filePath)
{
    // Get archive type
    hl_ArchiveType type;
    hl_INGetArchiveType(filePath, &type);

    // Load archive
    return hl_INLoadArchiveOfType(filePath, type);
}

struct hl_Blob* hl_LoadArchive(const char* filePath)
{
    if (!filePath) return nullptr;
    return hl_INLoadArchive(filePath);
}

struct hl_Blob* hl_LoadArchiveNative(const hl_NativeStr filePath)
{
    if (!filePath) return nullptr;
    return hl_INLoadArchive(filePath);
}

template<typename char_t>
hl_Blob* hl_INLoadRootArchive(const char_t* filePath)
{
    // Get archive type
    hl_ArchiveType type;
    bool isSplit = hl_INGetArchiveType(filePath, &type);

    if (type == HL_ARC_TYPE_UNKNOWN) return nullptr;

    // Load archive
    return hl_INLoadRootArchiveOfType(filePath, type, isSplit);
}

struct hl_Blob* hl_LoadRootArchive(const char* filePath)
{
    if (!filePath) return nullptr;
    return hl_INLoadRootArchive(filePath);
}

struct hl_Blob* hl_LoadRootArchiveNative(const hl_NativeStr filePath)
{
    if (!filePath) return nullptr;
    return hl_INLoadRootArchive(filePath);
}

template<bool pacv3>
bool hl_INArchiveNextSplit(hl_NativeStr splitCharPtr)
{
    // Weird code, I know. ':' comes after '9'
    if (++(*splitCharPtr) == HL_NATIVE_TEXT(':'))
    {
        if (++(*(splitCharPtr - 1)) == HL_NATIVE_TEXT(':'))
        {
            // Check extra digit in Forces splits as well
            if constexpr (pacv3)
            {
                if (++(*(splitCharPtr - 2)) == HL_NATIVE_TEXT(':'))
                {
                    return false;
                }
            }
            else
            {
                return false;
            }

            // Reset second digit in Forces splits
            *(splitCharPtr - 1) = HL_NATIVE_TEXT('0');
        }

        // Reset second digit in split (third digit in Forces splits)
        *splitCharPtr = HL_NATIVE_TEXT('0');
    }

    return true;
}

bool hl_INArchiveNextSplit(hl_NativeStr splitCharPtr, bool pacv3)
{
    if (pacv3) return hl_INArchiveNextSplit<true>(splitCharPtr);
    return hl_INArchiveNextSplit<false>(splitCharPtr);
}

size_t hl_INGetArchiveSplitCountRoot(const hl_NativeStr rootPath, const hl_NativeStr ext)
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
    hl_NativeStr splitPath = HL_CREATE_NATIVE_STR(splitPathLen);

    // Copy root path into split path buffer
    std::copy(rootPath, ext, splitPath);

    // Change split path extension
    splitPath[rootPathLen - 1] = HL_NATIVE_TEXT('.');
    splitPath[rootPathLen]     = HL_NATIVE_TEXT('0');
    splitPath[rootPathLen + 1] = HL_NATIVE_TEXT('0');
    splitPath[rootPathLen + 2] = HL_NATIVE_TEXT('\0');

    // If this is a pac file, determine if this is a Forces pac
    size_t maxSplitIndex = 99;
    hl_NativeStr splitCharPtr = (splitPath + rootPathLen + 1);
    i = 0;

    if (pacv3)
    {
        // Use extra byte we allocated earlier to add an extra number to the extension
        // (E.G. Sonic.pac.00 -> Sonic.pac.000)
        splitPath[rootPathLen + 2] = HL_NATIVE_TEXT('0');
        splitPath[rootPathLen + 3] = HL_NATIVE_TEXT('\0');

        // Determine if this is a forces pac by checking for a split with a three-digit split index
        pacv3 = hl_INPathExists(splitPath);

        // If it isn't a forces pac, get rid of that third number
        if (!pacv3)
        {
            splitPath[rootPathLen + 2] = HL_NATIVE_TEXT('\0');
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

    free(splitPath);
    return ++i;
}

size_t hl_INGetArchiveSplitCountRoot(const hl_NativeStr rootPath)
{
    // Get extension
    const hl_NativeStr ext = hl_PathGetExtPtr(rootPath);
    if (!*ext) return 0;

    // Get split count and return
    return hl_INGetArchiveSplitCountRoot(rootPath, ext);
}

#ifdef _WIN32
size_t hl_INGetArchiveSplitCountRoot(const char* rootPath)
{
    // Convert to UTF-16
    hl_NativeStr nativePath;
    if (HL_FAILED(hl_INStringConvertUTF8ToNative(
        rootPath, &nativePath))) return 0;

    // Get split count
    size_t splitCount = hl_INGetArchiveSplitCountRoot(nativePath);
    free(nativePath);
    return splitCount;
}
#endif

size_t hl_GetArchiveSplitCountRoot(const char* rootPath)
{
#ifdef _WIN32
    if (!rootPath) return 0;
#endif
    return hl_INGetArchiveSplitCountRoot(rootPath);
}

size_t hl_GetArchiveSplitCountRootNative(const hl_NativeStr rootPath)
{
    return hl_INGetArchiveSplitCountRoot(rootPath);
}

size_t hl_INGetArchiveSplitCount(const hl_NativeStr filePath)
{
    // Get extension
    const hl_NativeStr ext = hl_PathGetExtPtr(filePath);
    if (!*ext) return 0;

    // Check if this is a split
    if (hl_INGetArchiveIsSplitExt(ext))
    {
        // Get root path
        hl_NativeStr rootPath;
        if (HL_FAILED(hl_INGetRootArchivePath(filePath, &rootPath)))
            return 0;

        // Get split count from root path and return
        size_t splitCount = hl_INGetArchiveSplitCountRoot(rootPath);
        free(rootPath);
        return splitCount;
    }

    // Get split count and return
    return hl_INGetArchiveSplitCountRoot(filePath, ext);
}

size_t hl_GetArchiveSplitCount(const char* filePath)
{
#ifdef _WIN32
    if (!filePath) return 0;

    // Convert to UTF-16
    hl_NativeStr nativePath;
    if (HL_FAILED(hl_INStringConvertUTF8ToNative(
        filePath, &nativePath))) return 0;

    // Get split count
    size_t splitCount = hl_INGetArchiveSplitCount(nativePath);
    free(nativePath);
    return splitCount;
#else
    return hl_INGetArchiveSplitCount(filePath);
#endif
}

size_t hl_GetArchiveSplitCountNative(const hl_NativeStr filePath)
{
    return hl_INGetArchiveSplitCount(filePath);
}

HL_RESULT hl_INExtractArchive(const hl_Blob* blob, const hl_NativeStr dir)
{
    switch (blob->Type)
    {
    // .ar/.pfd (Unleashed/Generations)
    case HL_ARC_TYPE_HEDGEHOG:
        return hl_ExtractHHArchive(blob, dir);

    // .pac (LW/Forces/Tokyo 2020)
    case HL_ARC_TYPE_PACX:
        return hl_ExtractPACxArchive(blob, dir);

    // .pac V2 (LW)
    case HL_ARC_TYPE_PACX_V2:
        return hl_ExtractLWArchive(blob, dir);

    // .pac V3 (Forces)
    case HL_ARC_TYPE_PACX_V3:
        // TODO: Forces Archives
        //return hl_ExtractForcesArchive(blob, dir);
        break;

    // .pac V4 (Tokyo 2020)
    case HL_ARC_TYPE_PACX_V4:
        // TODO: Tokyo 2020 Archives
        //return hl_ExtractTokyoArchive(blob, dir);
        break;
    }

    // Attempt to auto-detect archive type
    if (blob->Format == HL_BLOB_FORMAT_BINA)
    {
        // Attempt to extract BINA file as a pac
        return hl_ExtractPACxArchive(blob, dir);
    }

    // Unknown type
    return HL_ERROR_UNKNOWN;
}

enum HL_RESULT hl_ExtractArchive(const struct hl_Blob* blob, const char* dir)
{
    if (!blob) return HL_ERROR_UNKNOWN;
    HL_INSTRING_NATIVE_CALL(dir, hl_INExtractArchive(blob, nativeStr));
}

enum HL_RESULT hl_ExtractArchiveNative(const struct hl_Blob* blob, const hl_NativeStr dir)
{
    if (!blob) return HL_ERROR_UNKNOWN;
    return hl_INExtractArchive(blob, dir);
}

HL_RESULT hl_INExtractArchivesOfTypeRoot(const hl_NativeStr rootPath,
    const hl_NativeStr dir, hl_ArchiveType type)
{
    // Extract root
    hl_Blob* arc;
    bool rootExists = hl_INPathExists(rootPath);
    HL_RESULT result;

    if (rootExists)
    {
        arc = hl_INLoadArchiveOfType(rootPath, type); // TODO: Nullptr check
        result = hl_ExtractArchiveNative(arc, dir);
        if (HL_FAILED(result)) return result;
    }

    // Extract splits
    size_t splitCount;
    if (rootExists && type & HL_ARC_TYPE_PACX)
    {
        // Get splits list from data in root PAC
        hl_Blob* splitArc;
        hl_NativeStr rootDir, *splitPath;
        const char** splits = hl_PACxArchiveGetSplits(arc, &splitCount);

        if (!splits)
        {
            // There are no splits to extract
            hl_FreeBlob(arc);
            return HL_SUCCESS;
        }

        // Extract splits
        result = hl_INPathGetParent(rootPath, &rootDir);
        if (HL_FAILED(result)) return result;

        for (size_t i = 0; i < splitCount; ++i)
        {
            // Get split path
            result = hl_PathCombine(rootDir, splits[i], &splitPath);
            if (HL_FAILED(result)) return result;

            // Extract split
            splitArc = hl_INLoadArchiveOfType(splitPath, type);
            result = hl_ExtractArchiveNative(splitArc, dir);
            free(splitPath);
            
            if (HL_FAILED(result))
            {
                free(rootDir);
                free(splits);
                hl_FreeBlob(arc);
                return result;
            }
        }

        // Free data
        free(rootDir);
        free(splits);
        hl_FreeBlob(arc);
    }
    else
    {
        // Free root archive
        if (rootExists) hl_FreeBlob(arc);

        // Get size of paths
        size_t rootPathLen = (hl_StrLenNative(rootPath));
        size_t splitPathLen = (rootPathLen + 4);

        // Create split path buffer
        hl_NativeStr splitPath = HL_CREATE_NATIVE_STR(splitPathLen);
        std::copy(rootPath, rootPath + rootPathLen, splitPath);

        // Add split extension to split path
        splitPath[rootPathLen] = HL_NATIVE_TEXT('.');
        splitPath[rootPathLen + 1] = HL_NATIVE_TEXT('0');
        splitPath[rootPathLen + 2] = HL_NATIVE_TEXT('0');
        splitPath[rootPathLen + 3] = HL_NATIVE_TEXT('\0');

        // Get the split count
        size_t maxSplitIndex = 99;
        hl_NativeStr splitCharPtr = (splitPath + rootPathLen + 2);

        for (size_t i = 0; i <= 99; ++i)
        {
            // Check if this split exists
            if (!hl_INPathExists(splitPath)) break;

            // Extract split
            arc = hl_INLoadArchiveOfType(splitPath, type);
            result = hl_ExtractArchiveNative(arc, dir);
            hl_FreeBlob(arc);

            if (HL_FAILED(result))
            {
                free(splitPath);
                return result;
            }

            // Get next split, if any
            if (!hl_INArchiveNextSplit(splitCharPtr, false)) break;
        }

        // Free data
        free(splitPath);
    }

    return HL_SUCCESS;
}

HL_RESULT hl_INExtractArchivesOfType(const hl_NativeStr filePath,
    const hl_NativeStr dir, hl_ArchiveType type, bool isSplit)
{
    if (isSplit)
    {
        // This is a split; load the root archive instead
        hl_NativeStr rootPath;
        HL_RESULT result = hl_INGetRootArchivePath(filePath, &rootPath);
        if (HL_FAILED(result)) return result;

        // Extract root archive and its splits
        result = hl_INExtractArchivesOfTypeRoot(rootPath, dir, type);
        free(rootPath);
        return result;
    }
    else
    {
        // Extract root archive and its splits
        return hl_INExtractArchivesOfTypeRoot(filePath, dir, type);
    }
}

HL_RESULT hl_INExtractArchivesOfType(const hl_NativeStr filePath,
    const hl_NativeStr dir, hl_ArchiveType type)
{
    const hl_NativeStr ext = hl_PathGetExtPtr(filePath);
    return hl_INExtractArchivesOfType(filePath, dir,
        type, hl_INGetArchiveIsSplitExt(ext));
}

enum HL_RESULT hl_ExtractArchivesOfType(const char* filePath,
    const char* dir, enum hl_ArchiveType type)
{
    if (!filePath || !dir) return HL_ERROR_UNKNOWN;

#ifdef _WIN32
    // Convert filePath
    hl_NativeStr filePathNative;
    HL_RESULT result = hl_INStringConvertUTF8ToNative(
        filePath, &filePathNative);

    if (HL_FAILED(result)) return result;

    // Convert dir
    hl_NativeStr dirNative;
    result = hl_INStringConvertUTF8ToNative(
        dir, &dirNative);

    if (HL_FAILED(result)) return result;

    // Extract archives of type
    result = hl_INExtractArchivesOfType(filePathNative, dirNative, type);
    
    free(filePathNative);
    free(dirNative);
    return result;
#else
    return hl_INExtractArchivesOfType(filePath, dir, type);
#endif
}

enum HL_RESULT hl_ExtractArchivesOfTypeNative(const hl_NativeStr filePath,
    const hl_NativeStr dir, enum hl_ArchiveType type)
{
    if (!filePath || !dir) return HL_ERROR_UNKNOWN;
    return hl_INExtractArchivesOfType(filePath, dir, type);
}

HL_RESULT hl_INExtractArchives(const hl_NativeStr filePath, const hl_NativeStr dir)
{
    // Get archive type
    hl_ArchiveType type;
    bool isSplit = hl_INGetArchiveType(filePath, &type);

    if (type == HL_ARC_TYPE_UNKNOWN) return HL_ERROR_UNKNOWN;

    // Extract archives
    return hl_INExtractArchivesOfType(filePath, dir, type, isSplit);
}

enum HL_RESULT hl_ExtractArchives(const char* filePath, const char* dir)
{
    if (!filePath || !dir) return HL_ERROR_UNKNOWN;

#ifdef _WIN32
    // Convert filePath
    hl_NativeStr filePathNative;
    HL_RESULT result = hl_INStringConvertUTF8ToNative(
        filePath, &filePathNative);

    if (HL_FAILED(result)) return result;

    // Convert dir
    hl_NativeStr dirNative;
    result = hl_INStringConvertUTF8ToNative(
        dir, &dirNative);

    if (HL_FAILED(result)) return result;

    // Extract archives of type
    result = hl_INExtractArchives(filePathNative, dirNative);

    free(filePathNative);
    free(dirNative);
    return result;
#else
    return hl_INExtractArchives(filePath, dir);
#endif
}

enum HL_RESULT hl_ExtractArchivesNative(
    const hl_NativeStr filePath, const hl_NativeStr dir)
{
    if (!filePath || !dir) return HL_ERROR_UNKNOWN;
    return hl_INExtractArchives(filePath, dir);
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
        malloc(sizeof(hl_ArchiveFileEntry) * fileCount));

    for (size_t i = 0; i < fileCount; ++i)
    {
        hl_CreateArchiveFileEntry(files[i], &(fileEntries[i]));
    }

    return fileEntries;
}
