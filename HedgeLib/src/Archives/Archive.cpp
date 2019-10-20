#include "INArchive.h"
#include "INPACx.h"
#include "HedgeLib/Archives/Archive.h"
#include "HedgeLib/Archives/PACx.h"
#include "HedgeLib/Archives/GensArchive.h"
#include "HedgeLib/Archives/LWArchive.h"
#include "HedgeLib/IO/Path.h"
#include "HedgeLib/IO/File.h"
#include "../IO/INBINA.h"
#include "../IO/INPath.h"
#include "../INBlob.h"
#include "../INString.h"
#include <cctype>
#include <cstring>
#include <algorithm>

size_t hl_INGensArchiveGetBufferSize(const hl_Blob* blob, size_t& fileCount);
HL_RESULT hl_INCreateGensArchive(const hl_Blob* blob, hl_ArchiveFileEntry*& entries, uint8_t*& data);

size_t hl_INLWArchiveGetBufferSize(const hl_Blob* blob, size_t& fileCount);
HL_RESULT hl_INCreateLWArchive(const hl_Blob* blob, hl_ArchiveFileEntry*& entries, uint8_t*& data);

HL_ARCHIVE_TYPE hl_DArchiveBlobGetType(const hl_Blob* blob)
{
    return (blob) ? static_cast<HL_ARCHIVE_TYPE>(blob->Type) : HL_ARC_TYPE_UNKNOWN;
}

template<typename char_t>
bool hl_INArchiveNextSplit(char_t* splitCharPtr)
{
    // Weird code, I know. ':' comes after '9'
    if (++(*splitCharPtr) == static_cast<char_t>(':'))
    {
        if (++(*(splitCharPtr - 1)) == static_cast<char_t>(':'))
        {
            return false;
        }

        // Reset second digit in split
        *splitCharPtr = static_cast<char_t>('0');
    }

    return true;
}

template<typename char_t>
bool hl_INArchiveNextSplitPACxV3(char_t* splitCharPtr)
{
    // Weird code, I know. ':' comes after '9'
    if (++(*splitCharPtr) == static_cast<char_t>(':'))
    {
        if (++(*(splitCharPtr - 1)) == static_cast<char_t>(':'))
        {
            if (++(*(splitCharPtr - 2)) == static_cast<char_t>(':'))
            {
                return false;
            }

            // Reset second digit in split
            *(splitCharPtr - 1) = static_cast<char_t>('0');
        }

        // Reset third digit in split
        *splitCharPtr = static_cast<char_t>('0');
    }

    return true;
}

template bool hl_INArchiveNextSplit<char>(char* splitCharPtr);
template bool hl_INArchiveNextSplitPACxV3<char>(char* splitCharPtr);

#ifdef _WIN32
template bool hl_INArchiveNextSplit<hl_NativeChar>(hl_NativeChar* splitCharPtr);
template bool hl_INArchiveNextSplitPACxV3<hl_NativeChar>(hl_NativeChar* splitCharPtr);
#endif

template<typename char_t>
bool hl_INArchiveNextSplit(char_t* splitCharPtr, bool pacv3)
{
    if (pacv3) return hl_INArchiveNextSplitPACxV3<char_t>(splitCharPtr);
    return hl_INArchiveNextSplit<char_t>(splitCharPtr);
}

template<typename char_t>
bool hl_INArchiveIsSplitExt(const char_t* ext)
{
    return (ext[0] == '.' && std::isdigit(ext[1]) && std::isdigit(ext[2]));
}

bool hl_ArchiveIsSplitExt(const char* ext)
{
    if (!ext) return false;
    return hl_INArchiveIsSplitExt(ext);
}

bool hl_ArchiveIsSplitExtNative(const hl_NativeChar* ext)
{
    if (!ext) return false;
    return hl_INArchiveIsSplitExt(ext);
}

template<typename char_t>
bool hl_INArchiveIsSplit(const char_t* filePath)
{
    // Get extension
    const char_t* ext = hl_PathGetExtPtr(filePath);
    if (!*ext) return 0;

    // Return whether or not the archive is a split
    return hl_INArchiveIsSplitExt(ext);
}

bool hl_ArchiveIsSplit(const char* filePath)
{
    return hl_INArchiveIsSplit(filePath);
}

bool hl_ArchiveIsSplitNative(const hl_NativeChar* filePath)
{
    return hl_INArchiveIsSplit(filePath);
}

template<typename char_t>
HL_ARCHIVE_TYPE hl_INArchiveGetTypeExt(const char_t* ext)
{
    // Generations
    if (hl_INStringsEqualInvASCII(ext, hl_ARExtension) ||
        hl_INStringsEqualInvASCII(ext, hl_PFDExtension))
    {
        return HL_ARC_TYPE_GENS;
    }

    // PACx
    if (hl_INStringsEqualInvASCII(ext, hl_PACxExtension))
    {
        return HL_ARC_FORMAT_PACX;
    }

    // TODO: Support other Archive types
    return HL_ARC_TYPE_UNKNOWN;
}

HL_ARCHIVE_TYPE hl_ArchiveGetTypeExt(const char* ext)
{
    if (!ext || !*ext) return HL_ARC_TYPE_UNKNOWN;
    return hl_INArchiveGetTypeExt(ext);
}

HL_ARCHIVE_TYPE hl_ArchiveGetTypeExtNative(const hl_NativeChar* ext)
{
    if (!ext || !*ext) return HL_ARC_TYPE_UNKNOWN;
    return hl_INArchiveGetTypeExt(ext);
}

template<typename char_t>
bool hl_INArchiveGetType(const char_t* filePath, HL_ARCHIVE_TYPE* type)
{
    // Get file name pointer
    filePath = hl_INPathGetNamePtr(filePath);
    if (!*filePath) return false;

    // Check extension
    const char_t* ext = hl_INPathGetExtPtrName(filePath);

    // Check if this is a split
    if (hl_INArchiveIsSplitExt(ext))
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
                    *type = HL_ARC_TYPE_GENS;
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
                *(--ext) == hl_ARExtension[i])                          // .
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
    *type = hl_ArchiveGetTypeExt(ext);
    return false;
}

bool hl_ArchiveGetType(const char* filePath, HL_ARCHIVE_TYPE* type)
{
    if (!filePath || !type) return false;
    return hl_INArchiveGetType(filePath, type);
}

bool hl_ArchiveGetTypeNative(const hl_NativeChar* filePath, HL_ARCHIVE_TYPE* type)
{
    if (!filePath || !type) return false;
    return hl_INArchiveGetType(filePath, type);
}

HL_RESULT hl_ArchiveGetRootPath(const char* splitPath, char** rootPath)
{
    if (!splitPath || !rootPath) return HL_ERROR_UNKNOWN;
    return hl_INPathRemoveExt<char, false>(splitPath, rootPath);
}

HL_RESULT hl_ArchiveGetRootPathNative(
    const hl_NativeChar* splitPath, hl_NativeChar** rootPath)
{
    if (!splitPath || !rootPath) return HL_ERROR_UNKNOWN;
    return hl_INPathRemoveExt<hl_NativeChar, false>(splitPath, rootPath);
}

size_t hl_DArchiveGetFileCount(const hl_Blob* blob, bool includeProxies)
{
    if (!blob) return 0;
    switch (blob->Type)
    {
    // Unknown PACx
    case HL_ARC_FORMAT_PACX:
        return hl_PACxGetFileCount(blob, includeProxies);

    // Unleashed/Generations
    case HL_ARC_TYPE_GENS:
        return hl_GensArchiveGetFileCount(blob);

    // Lost World
    case HL_ARC_TYPE_PACX_V2:
        return hl_LWArchiveGetFileCount(blob, includeProxies);

    // TODO: Add other Archive Types

    default:
        return 0;
    }
}

HL_RESULT hl_INDCreateArchive(const hl_NativeChar* dir, hl_Archive** arc)
{
    // Get files in the given directory
    size_t fileCount;
    char** filesPtr;

    HL_RESULT result = hl_INPathGetFilesInDirectoryUTF8(
        dir, false, &fileCount, &filesPtr);

    if (HL_FAILED(result)) return result;

    // Get buffer length
    hl_CPtr files = filesPtr;
    size_t bufLen = (sizeof(hl_Archive) +
        (sizeof(hl_ArchiveFileEntry) * fileCount));

    for (size_t i = 0; i < fileCount; ++i)
    {
        bufLen += (strlen(files[i]) + 1);
    }

    // Create buffer
    *arc = static_cast<hl_Archive*>(malloc(bufLen));
    if (!(*arc)) return HL_ERROR_OUT_OF_MEMORY;

    // Create file entries
    hl_ArchiveFileEntry* entries = reinterpret_cast<hl_ArchiveFileEntry*>((*arc) + 1);
    char* names = reinterpret_cast<char*>(entries + fileCount);

    for (size_t i = 0; i < fileCount; ++i)
    {
        // Generate file entry
        entries[i].Size = 0;
        entries[i].Data = names;
        
        // Copy file path
        size_t nameLen = (strlen(files[i]) + 1);
        std::copy(files[i], files[i] + nameLen, names);

        // Set file name pointer
        entries[i].Name = hl_INPathGetNamePtr(names);
        names += nameLen;
    }

    // Generate hl_Archive
    (*arc)->FileCount = fileCount;
    (*arc)->Files = entries;
    return HL_SUCCESS;
}

HL_RESULT hl_CreateArchive(const char* dir, hl_Archive** arc)
{
    if (!dir || !arc) return HL_ERROR_INVALID_ARGS;
    HL_INSTRING_NATIVE_CALL(dir, hl_INDCreateArchive(nativeStr, arc));
}

HL_RESULT hl_CreateArchiveNative(const hl_NativeChar* dir, hl_Archive** arc)
{
    if (!dir || !arc) return HL_ERROR_INVALID_ARGS;
    return hl_INDCreateArchive(dir, arc);
}

size_t hl_INArchiveGetBufferSize(const hl_Blob* blob, size_t& fileCount)
{
    switch (blob->Type)
    {
    // Unknown PACx
    case HL_ARC_FORMAT_PACX:
        return hl_INPACxGetBufferSize(blob, fileCount);

    // Unleashed/Generations
    case HL_ARC_TYPE_GENS:
        return hl_INGensArchiveGetBufferSize(blob, fileCount);

    // Lost World
    case HL_ARC_TYPE_PACX_V2:
        return hl_INLWArchiveGetBufferSize(blob, fileCount);

    // TODO: Add other Archive Types
    default: return 0;
    }
}

HL_RESULT hl_INDCreateArchive(const hl_Blob* blob,
    hl_ArchiveFileEntry*& entries, uint8_t*& data)
{
    switch (blob->Type)
    {
    // Unknown PACx
    case HL_ARC_FORMAT_PACX:
        return hl_INPACxCreateArchive(blob, entries, data);

    // Unleashed/Generations
    case HL_ARC_TYPE_GENS:
        return hl_INCreateGensArchive(blob, entries, data);

    // Lost World
    case HL_ARC_TYPE_PACX_V2:
        return hl_INCreateLWArchive(blob, entries, data);

    // TODO: Add other Archive Types
    }

    return HL_ERROR_UNSUPPORTED;
}

HL_RESULT hl_DCreateArchive(const hl_Blob** blobs,
    size_t blobCount, hl_Archive** arc)
{
    if (!blobs || !arc) return HL_ERROR_INVALID_ARGS;

    // Get size of buffer and file count
    size_t bufSize = sizeof(hl_Archive);
    size_t fileCount = 0;

    for (size_t i = 0; i < blobCount; ++i)
    {
        if (!blobs[i]) return HL_ERROR_INVALID_ARGS;
        bufSize += hl_INArchiveGetBufferSize(blobs[i], fileCount);
    }

    // Allocate buffer big enough to hold an hl_Archive and its file entries
    *arc = static_cast<hl_Archive*>(malloc(bufSize));
    if (!*arc) return HL_ERROR_OUT_OF_MEMORY;

    // Generate hl_Archive
    hl_ArchiveFileEntry* entries = reinterpret_cast<hl_ArchiveFileEntry*>((*arc) + 1);
    uint8_t* data = reinterpret_cast<uint8_t*>(entries + fileCount);

    (*arc)->FileCount = fileCount;
    (*arc)->Files = entries;

    // Generate file entires
    HL_RESULT result;
    for (size_t i = 0; i < blobCount; ++i)
    {
        result = hl_INDCreateArchive(blobs[i], entries, data);
        if (HL_FAILED(result)) return result;
    }

    return HL_SUCCESS;
}

template<typename char_t>
HL_RESULT hl_INDLoadArchiveOfType(const char_t* filePath,
    HL_ARCHIVE_TYPE type, hl_Blob** blob)
{
    // Load archive based on type
    switch (type)
    {
    // Unknown PACx
    case HL_ARC_FORMAT_PACX:
    {
        // Load the PAC
        HL_RESULT result = hl_PACxLoad(filePath, blob);
        if (HL_FAILED(result)) return result;

        // Endian swap the PAC
        switch ((*blob)->Type)
        {
        case HL_ARC_TYPE_PACX_V2:
            if (hl_INBINAIsBigEndianV2((*blob)->GetData<hl_BINAV2Header>()))
            {
                hl_LWArchive* arc = hl_PACxGetDataV2<hl_LWArchive>(*blob);
                arc->EndianSwapRecursive(true);
            }
            return HL_SUCCESS;

        // TODO: Add Forces archive support and un-comment this
        /*case HL_ARC_TYPE_PACX_V3:
            if (hl_INBINAIsBigEndianV2(*blob))
            {
                hl_ForcesArchive* arc = hl_PACxGetDataV3<hl_ForcesArchive>(*blob);
                arc->EndianSwapRecursive(true);
            }
            return HL_SUCCESS;*/

        // TODO: Add Tokyo 2020 archive support and un-comment this
        /*case HL_ARC_TYPE_PACX_V4:
            if (hl_INBINAIsBigEndianV2(*blob))
            {
                hl_TokyoArchive* arc = hl_PACxGetDataV4<hl_TokyoArchive>(*blob);
                arc->EndianSwapRecursive(true);
            }
            return HL_SUCCESS;*/

        default:
            free(*blob);
            return HL_ERROR_UNSUPPORTED;
        }
    }

    // Unleashed/Generations
    case HL_ARC_TYPE_GENS:
        return hl_LoadGensArchiveBlob(filePath, blob);

    // Lost World
    case HL_ARC_TYPE_PACX_V2:
        return hl_DLoadLWArchive(filePath, blob);

    // TODO: Add other Archive Types

    default:
        return HL_ERROR_UNSUPPORTED;
    }
}

HL_RESULT hl_DLoadArchiveOfType(const char* filePath,
    HL_ARCHIVE_TYPE type, hl_Blob** blob)
{
    if (!filePath || !blob || !*filePath) return HL_ERROR_INVALID_ARGS;
    return hl_INDLoadArchiveOfType(filePath, type, blob);
}

HL_RESULT hl_DLoadArchiveOfTypeNative(const hl_NativeChar* filePath,
    HL_ARCHIVE_TYPE type, hl_Blob** blob)
{
    if (!filePath || !blob || !*filePath) return HL_ERROR_INVALID_ARGS;
    return hl_INDLoadArchiveOfType(filePath, type, blob);
}

template<typename char_t>
HL_RESULT hl_INDLoadArchive(const char_t* filePath, hl_Blob** blob)
{
    // Get archive type
    HL_ARCHIVE_TYPE type;
    hl_INArchiveGetType(filePath, &type);

    // Load archive
    return hl_INDLoadArchiveOfType(filePath, type, blob);
}

HL_RESULT hl_DLoadArchive(const char* filePath, hl_Blob** blob)
{
    if (!filePath || !blob) return HL_ERROR_INVALID_ARGS;
    return hl_INDLoadArchive(filePath, blob);
}

HL_RESULT hl_DLoadArchiveNative(const hl_NativeChar* filePath, hl_Blob** blob)
{
    if (!filePath || !blob) return HL_ERROR_INVALID_ARGS;
    return hl_INDLoadArchive(filePath, blob);
}

template<typename char_t>
HL_RESULT hl_INDLoadRootArchiveOfType(const char_t* filePath,
    HL_ARCHIVE_TYPE type, bool isSplit, hl_Blob** blob)
{
    // Load root archive instead of the given file if the given file is a split
    if (isSplit)
    {
        // Get root path
        HL_RESULT result;
        char_t* rootPath;

        result = hl_INPathRemoveExt<char_t, false>(filePath, &rootPath);
        if (HL_FAILED(result)) return result;

        // Load root archive
        result = hl_INDLoadArchiveOfType(rootPath, type, blob);
        free(rootPath);
        return result;
    }

    // Otherwise, just load the given file
    return hl_INDLoadArchiveOfType(filePath, type, blob);
}

template<typename char_t>
HL_RESULT hl_INDLoadRootArchive(const char_t* filePath, hl_Blob** blob)
{
    // Get archive type
    HL_ARCHIVE_TYPE type;
    bool isSplit = hl_INArchiveGetType(filePath, &type);
    if (type == HL_ARC_TYPE_UNKNOWN) return HL_ERROR_UNSUPPORTED;

    // Load archive
    return hl_INDLoadRootArchiveOfType(
        filePath, type, isSplit, blob);
}

HL_RESULT hl_DLoadRootArchive(const char* filePath, hl_Blob** blob)
{
    if (!filePath || !blob) return HL_ERROR_INVALID_ARGS;
    return hl_INDLoadRootArchive(filePath, blob);
}

HL_RESULT hl_DLoadRootArchiveNative(const hl_NativeChar* filePath, hl_Blob** blob)
{
    if (!filePath || !blob) return HL_ERROR_INVALID_ARGS;
    return hl_INDLoadRootArchive(filePath, blob);
}

HL_RESULT hl_INLoadArchive(const hl_NativeChar* rootPath,
    HL_ARCHIVE_TYPE type, hl_Archive** arc)
{
    // Extract root
    HL_RESULT result;
    hl_BlobPtr rootBlob;
    bool rootExists = hl_PathExists(rootPath);

    size_t bufSize = sizeof(hl_Archive);
    size_t fileCount = 0;

    if (rootExists)
    {
        // Load root
        hl_Blob* arcPtr;
        result = hl_INDLoadArchiveOfType(rootPath, type, &arcPtr);
        if (HL_FAILED(result)) return result;

        // Get buffer size
        rootBlob = arcPtr;
        bufSize += hl_INArchiveGetBufferSize(rootBlob, fileCount);
    }

    // Get split count
    hl_CPtr<void> splits;
    size_t splitCount = 0;
    hl_NativeChar* splitCharPtr;

    if (rootExists && (type & HL_ARC_FORMAT_PACX))
    {
        // Get splits list from data in root PAC
        splits = hl_PACxArchiveGetSplits(rootBlob, &splitCount);
    }
    else
    {
        // Get size of paths
        size_t rootPathLen = (hl_StrLenNative(rootPath));
        size_t splitPathLen = (rootPathLen + 4);

        // Create split path buffer
        splits = HL_CREATE_NATIVE_STR(splitPathLen);
        if (!splits) return HL_ERROR_OUT_OF_MEMORY;

        hl_NativeChar* splitPath = static_cast<hl_NativeChar*>(splits.Get());
        std::copy(rootPath, rootPath + rootPathLen, splitPath);

        // Add split extension to split path
        splitPath[rootPathLen] = HL_NATIVE_TEXT('.');
        splitPath[rootPathLen + 1] = HL_NATIVE_TEXT('0');
        splitPath[rootPathLen + 2] = HL_NATIVE_TEXT('0');
        splitPath[rootPathLen + 3] = HL_NATIVE_TEXT('\0');

        // Get the split count
        splitCharPtr = (splitPath + rootPathLen + 2);
        for (; splitCount <= 99; ++splitCount)
        {
            // Get next split, if any
            if (!hl_INPathExists(splitPath) ||
                !hl_INArchiveNextSplit(splitCharPtr)) break;
        }

        // Reset split path
        splitPath[rootPathLen + 1] = HL_NATIVE_TEXT('0');
        splitPath[rootPathLen + 2] = HL_NATIVE_TEXT('0');
    }

    // Create blobs array
    size_t arcCount = (splitCount + 1);
    hl_CPtr<hl_Blob*> blobs = static_cast<hl_Blob**>(malloc(
        sizeof(hl_Blob*) * arcCount));

    if (!blobs) return HL_ERROR_OUT_OF_MEMORY;

    // Set root blob pointer
    blobs[0] = rootBlob;

    // Load splits
    if (splitCount)
    {
        if (rootExists && (type & HL_ARC_FORMAT_PACX))
        {
            // Get root directory
            hl_NativeChar* rootDirPtr;
            result = hl_INPathGetParent(rootPath, &rootDirPtr);
            if (HL_FAILED(result)) return result;

            hl_NStrPtr rootDir = rootDirPtr;

            // Extract splits
            for (size_t i = 0; i < splitCount;)
            {
                // Get split path
                hl_NativeChar* splitPath;
                result = hl_PathCombine(rootDir, reinterpret_cast<const char**>(
                    splits.Get())[i], &splitPath);
                
                if (HL_FAILED(result)) break;

                // Load split
                result = hl_INDLoadArchiveOfType(splitPath, type, &blobs[++i]);
                free(splitPath);

                if (HL_FAILED(result)) break;
            }
        }
        else
        {
            for (size_t i = 0; i < splitCount;)
            {
                // Load split
                result = hl_INDLoadArchiveOfType(static_cast<hl_NativeChar*>(
                    splits.Get()), type, &blobs[++i]);

                if (HL_FAILED(result)) break;

                // Get next split, if any
                if (!hl_INArchiveNextSplit(splitCharPtr)) break;
            }
        }

        // Free all the loaded blobs if anything went wrong
        if (HL_FAILED(result))
        {
            for (size_t i2 = 0; i2 < splitCount;)
            {
                free(blobs[++i2]);
            }
            return result;
        }
    }

    // Create hl_Archive
    result = hl_DCreateArchive(const_cast<const hl_Blob**>(
        blobs.Get()), arcCount, arc);

    // Free all blobs and return
    for (size_t i2 = 0; i2 < splitCount;)
    {
        free(blobs[++i2]);
    }

    return result;
}

HL_RESULT hl_INLoadArchive(const hl_NativeChar* filePath, hl_Archive** arc)
{
    HL_ARCHIVE_TYPE type;
    bool isSplit = hl_INArchiveGetType(filePath, &type);

    if (isSplit)
    {
        // This is a split; get the root archive instead
        hl_NativeChar* rootPath;
        HL_RESULT result = hl_INPathRemoveExt<hl_NativeChar, false>(
            filePath, &rootPath);

        if (HL_FAILED(result)) return result;

        // Load root archive and its splits
        result = hl_INLoadArchive(rootPath, type, arc);
        free(rootPath);
        return result;
    }

    // Load root archive and its splits
    return hl_INLoadArchive(filePath, type, arc);
}

HL_RESULT hl_LoadArchive(const char* filePath, hl_Archive** arc)
{
    if (!filePath || !arc) return HL_ERROR_INVALID_ARGS;
    HL_INSTRING_NATIVE_CALL(filePath, hl_INLoadArchive(nativeStr, arc));
}

HL_RESULT hl_LoadArchiveNative(const hl_NativeChar* filePath, hl_Archive** arc)
{
    if (!filePath || !arc) return HL_ERROR_INVALID_ARGS;
    return hl_INLoadArchive(filePath, arc);
}

size_t hl_INArchiveGetSplitCountRoot(
    const hl_NativeChar* rootPath, const hl_NativeChar* ext)
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
    hl_NativeChar* splitPath = HL_CREATE_NATIVE_STR(splitPathLen);

    // Copy root path into split path buffer
    std::copy(rootPath, ext, splitPath);

    // Change split path extension
    splitPath[rootPathLen - 1] = HL_NATIVE_TEXT('.');
    splitPath[rootPathLen]     = HL_NATIVE_TEXT('0');
    splitPath[rootPathLen + 1] = HL_NATIVE_TEXT('0');
    splitPath[rootPathLen + 2] = HL_NATIVE_TEXT('\0');

    // If this is a pac file, determine if this is a Forces pac
    size_t maxSplitIndex = 99;
    hl_NativeChar* splitCharPtr = (splitPath + rootPathLen + 1);
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

size_t hl_ArchiveGetSplitCountRoot(const char* rootPath)
{
#ifdef _WIN32
    // This is checked anyway in hl_ArchiveGetSplitCountRootNative
    // but on Windows we have to convert the string to UTF-16 before
    // that gets called, so it's better to check before we do all that.
    if (!rootPath) return 0;

    // Convert to UTF-16
    hl_NativeChar* nativePath;
    if (HL_FAILED(hl_INStringConvertUTF8ToNative(
        rootPath, &nativePath))) return 0;

    // Get split count
    size_t splitCount = hl_ArchiveGetSplitCountRootNative(nativePath);
    free(nativePath);
    return splitCount;
#else
    return hl_ArchiveGetSplitCountRootNative(rootPath);
#endif
}

size_t hl_ArchiveGetSplitCountRootNative(const hl_NativeChar* rootPath)
{
    // Get extension
    const hl_NativeChar* ext = hl_PathGetExtPtrNative(rootPath);
    if (!*ext) return 0;

    // Get split count and return
    return hl_INArchiveGetSplitCountRoot(rootPath, ext);
}

size_t hl_ArchiveGetSplitCount(const char* filePath)
{
#ifdef _WIN32
    // This is checked anyway in hl_ArchiveGetSplitCountNative
    // but on Windows we have to convert the string to UTF-16 before
    // that gets called, so it's better to check before we do all that.
    if (!filePath) return 0;

    // Convert to UTF-16
    hl_NativeChar* nativePath;
    if (HL_FAILED(hl_INStringConvertUTF8ToNative(
        filePath, &nativePath))) return 0;

    // Get split count
    size_t splitCount = hl_ArchiveGetSplitCountNative(nativePath);
    free(nativePath);
    return splitCount;
#else
    return hl_ArchiveGetSplitCountNative(filePath);
#endif
}

size_t hl_ArchiveGetSplitCountNative(const hl_NativeChar* filePath)
{
    // Get extension
    const hl_NativeChar* ext = hl_PathGetExtPtr(filePath);
    if (!*ext) return 0;

    // Check if this is a split
    if (hl_INArchiveIsSplitExt(ext))
    {
        // Get root path
        hl_NativeChar* rootPath;
        HL_RESULT result = hl_INPathRemoveExt<hl_NativeChar, false>(
            filePath, &rootPath);

        if (HL_FAILED(result)) return 0;

        // Get split count from root path and return
        size_t splitCount = hl_ArchiveGetSplitCountRootNative(rootPath);
        free(rootPath);
        return splitCount;
    }

    // Get split count and return
    return hl_INArchiveGetSplitCountRoot(filePath, ext);
}

HL_RESULT hl_INExtractArchiveBlob(const hl_Blob* blob, const hl_NativeChar* dir)
{
    switch (blob->Type)
    {
    // TODO: More archive types

    // .pac (LW/Forces/Tokyo 2020)
    case HL_ARC_FORMAT_PACX:
        return hl_ExtractPACxArchiveNative(blob, dir);

    // .ar/.pfd (Unleashed/Generations)
    case HL_ARC_TYPE_GENS:
        return hl_ExtractGensArchiveNative(blob, dir);

    // .pac V2 (LW)
    case HL_ARC_TYPE_PACX_V2:
        return hl_DExtractLWArchiveNative(blob, dir);

    // .pac V3 (Forces)
    case HL_ARC_TYPE_PACX_V3:
        // TODO: Forces Archives
        //return hl_ExtractForcesArchiveNative(blob, dir);
        break;

    // .pac V4 (Tokyo 2020)
    case HL_ARC_TYPE_PACX_V4:
        // TODO: Tokyo 2020 Archives
        //return hl_ExtractTokyoArchiveNative(blob, dir);
        break;
    }

    // Attempt to auto-detect archive type
    if (blob->Format == HL_BLOB_FORMAT_BINA)
    {
        // Attempt to extract BINA file as a pac
        return hl_ExtractPACxArchive(blob, dir);
    }

    // Unknown type
    return HL_ERROR_UNSUPPORTED;
}

HL_RESULT hl_DExtractArchive(const hl_Blob* blob, const char* dir)
{
    if (!blob) return HL_ERROR_INVALID_ARGS;
    HL_INSTRING_NATIVE_CALL(dir, hl_INExtractArchiveBlob(blob, nativeStr));
}

HL_RESULT hl_DExtractArchiveNative(const hl_Blob* blob, const hl_NativeChar* dir)
{
    if (!blob) return HL_ERROR_INVALID_ARGS;
    return hl_INExtractArchiveBlob(blob, dir);
}

HL_RESULT hl_INExtractArchive(const hl_Archive* arc, const hl_NativeChar* dir)
{
    // Create directory for file extraction
    HL_RESULT result = hl_PathCreateDirectory(dir);
    if (HL_FAILED(result)) return result;

    // Extract files
    hl_File file;
    for (size_t i = 0; i < arc->FileCount; ++i)
    {
        // Get file size
        size_t fileSize;
        hl_CPtr<void> fileData;

        if (!arc->Files[i].Size)
        {
            result = hl_PathGetSize(static_cast<const char*>(
                arc->Files[i].Data), &fileSize);
            
            if (HL_FAILED(result)) return result;

            // If the given file is not already in memory, load it
            result = file.OpenRead(static_cast<const char*>(arc->Files[i].Data));
            if (HL_FAILED(result)) return result;

            fileData = malloc(fileSize);
            if (!fileData) return HL_ERROR_OUT_OF_MEMORY;

            result = file.ReadBytes(fileData, fileSize);
            if (HL_FAILED(result)) return result;
        }
        else
        {
            fileSize = arc->Files[i].Size;
        }

        // Get file path
        hl_NativeChar* filePath;
        result = hl_PathCombine(dir, arc->Files[i].Name, &filePath);
        if (HL_FAILED(result)) return result;

        // Create file
        result = file.OpenWriteNative(filePath);
        free(filePath);
        if (HL_FAILED(result)) return result;

        // Write data to file
        result = file.WriteBytes((arc->Files[i].Size) ?
            arc->Files[i].Data : fileData.Get(), fileSize);

        if (HL_FAILED(result)) return result;
    }

    return HL_SUCCESS;
}

HL_RESULT hl_ExtractArchive(const hl_Archive* arc, const char* dir)
{
    if (!arc || !dir) return HL_ERROR_INVALID_ARGS;
    HL_INSTRING_NATIVE_CALL(dir, hl_INExtractArchive(arc, nativeStr));
}

HL_RESULT hl_ExtractArchiveNative(const hl_Archive* arc, const hl_NativeChar* dir)
{
    if (!arc || !dir) return HL_ERROR_INVALID_ARGS;
    return hl_INExtractArchive(arc, dir);
}

HL_RESULT hl_INExtractArchivesOfTypeRoot(const hl_NativeChar* rootPath,
    const hl_NativeChar* dir, HL_ARCHIVE_TYPE type)
{
    // Extract root
    hl_BlobPtr arc;
    bool rootExists = hl_INPathExists(rootPath);
    HL_RESULT result;

    if (rootExists)
    {
        // Load root
        hl_Blob* arcPtr;
        result = hl_INDLoadArchiveOfType(rootPath, type, &arcPtr);
        if (HL_FAILED(result)) return result;

        arc = arcPtr;

        // Extract root
        result = hl_INExtractArchiveBlob(arc, dir);
        if (HL_FAILED(result)) return result;
    }

    // Extract splits
    size_t splitCount;
    if (rootExists && (type & HL_ARC_FORMAT_PACX))
    {
        // Get splits list from data in root PAC
        hl_CPtr<const char*> splits = hl_PACxArchiveGetSplits(
            arc, &splitCount);

        // There are no splits to extract
        if (!splits) return HL_SUCCESS;

        // Get root directory
        hl_NativeChar* rootDirPtr;
        result = hl_INPathGetParent(rootPath, &rootDirPtr);
        if (HL_FAILED(result)) return result;

        hl_NStrPtr rootDir = rootDirPtr;

        // Extract splits
        for (size_t i = 0; i < splitCount; ++i)
        {
            // Get split path
            hl_NativeChar* splitPath;
            result = hl_PathCombine(rootDir, splits[i], &splitPath);
            if (HL_FAILED(result)) return result;

            // Load split
            hl_Blob* splitArc;
            result = hl_INDLoadArchiveOfType(splitPath, type, &splitArc);
            free(splitPath);

            if (HL_FAILED(result)) return result;

            // Extract split
            result = hl_INExtractArchiveBlob(splitArc, dir);
            free(splitArc);

            if (HL_FAILED(result)) return result;
        }
    }
    else
    {
        // Get size of paths
        size_t rootPathLen = (hl_StrLenNative(rootPath));
        size_t splitPathLen = (rootPathLen + 4);

        // Create split path buffer
        hl_NStrPtr splitPath = HL_CREATE_NATIVE_STR(splitPathLen);
        if (!splitPath) return HL_ERROR_OUT_OF_MEMORY;
        std::copy(rootPath, rootPath + rootPathLen, splitPath.Get());

        // Add split extension to split path
        splitPath[rootPathLen] = HL_NATIVE_TEXT('.');
        splitPath[rootPathLen + 1] = HL_NATIVE_TEXT('0');
        splitPath[rootPathLen + 2] = HL_NATIVE_TEXT('0');
        splitPath[rootPathLen + 3] = HL_NATIVE_TEXT('\0');

        // Get the split count
        hl_NativeChar* splitCharPtr = (splitPath + rootPathLen + 2);
        for (size_t i = 0; i <= 99; ++i)
        {
            // Check if this split exists
            if (!hl_INPathExists(splitPath)) break;

            // Load split
            hl_Blob* arcPtr;
            result = hl_INDLoadArchiveOfType(splitPath.Get(), type, &arcPtr);
            if (HL_FAILED(result)) return result;

            arc = arcPtr;

            // Extract split
            result = hl_INExtractArchiveBlob(arc, dir);
            if (HL_FAILED(result)) return result;

            // Get next split, if any
            if (!hl_INArchiveNextSplit(splitCharPtr)) break;
        }
    }

    return HL_SUCCESS;
}

HL_RESULT hl_INExtractArchivesOfType(const hl_NativeChar* filePath,
    const hl_NativeChar* dir, HL_ARCHIVE_TYPE type, bool isSplit)
{
    if (isSplit)
    {
        // This is a split; load the root archive instead
        hl_NativeChar* rootPath;
        HL_RESULT result = hl_INPathRemoveExt<hl_NativeChar, false>(
            filePath, &rootPath);

        if (HL_FAILED(result)) return result;

        // Extract root archive and its splits
        result = hl_INExtractArchivesOfTypeRoot(rootPath, dir, type);
        free(rootPath);
        return result;
    }
    
    // Extract root archive and its splits
    return hl_INExtractArchivesOfTypeRoot(filePath, dir, type);
}

HL_RESULT hl_INExtractArchivesOfType(const hl_NativeChar* filePath,
    const hl_NativeChar* dir, HL_ARCHIVE_TYPE type)
{
    const hl_NativeChar* ext = hl_PathGetExtPtr(filePath);
    return hl_INExtractArchivesOfType(filePath, dir,
        type, hl_INArchiveIsSplitExt(ext));
}

HL_RESULT hl_ExtractArchivesOfType(const char* filePath,
    const char* dir, HL_ARCHIVE_TYPE type)
{
    if (!filePath || !dir) return HL_ERROR_INVALID_ARGS;

#ifdef _WIN32
    // Convert filePath
    hl_NativeChar* filePathNative;
    HL_RESULT result = hl_INStringConvertUTF8ToNative(
        filePath, &filePathNative);

    if (HL_FAILED(result)) return result;

    // Convert dir
    hl_NativeChar* dirNative;
    result = hl_INStringConvertUTF8ToNative(
        dir, &dirNative);

    if (HL_FAILED(result))
    {
        free(filePathNative);
        return result;
    }

    // Extract archives of type
    result = hl_INExtractArchivesOfType(
        filePathNative, dirNative, type);
    
    free(filePathNative);
    free(dirNative);
    return result;
#else
    return hl_INExtractArchivesOfType(filePath, dir, type);
#endif
}

HL_RESULT hl_ExtractArchivesOfTypeNative(const hl_NativeChar* filePath,
    const hl_NativeChar* dir, HL_ARCHIVE_TYPE type)
{
    if (!filePath || !dir) return HL_ERROR_INVALID_ARGS;
    return hl_INExtractArchivesOfType(filePath, dir, type);
}

HL_RESULT hl_INExtractArchives(const hl_NativeChar* filePath, const hl_NativeChar* dir)
{
    // Get archive type
    HL_ARCHIVE_TYPE type;
    bool isSplit = hl_INArchiveGetType(filePath, &type);

    if (type == HL_ARC_TYPE_UNKNOWN) return HL_ERROR_UNSUPPORTED;

    // Extract archives
    return hl_INExtractArchivesOfType(filePath, dir, type, isSplit);
}

HL_RESULT hl_ExtractArchives(const char* filePath, const char* dir)
{
    if (!filePath || !dir) return HL_ERROR_INVALID_ARGS;

#ifdef _WIN32
    // Convert filePath
    hl_NativeChar* filePathNative;
    HL_RESULT result = hl_INStringConvertUTF8ToNative(
        filePath, &filePathNative);

    if (HL_FAILED(result)) return result;

    // Convert dir
    hl_NativeChar* dirNative;
    result = hl_INStringConvertUTF8ToNative(
        dir, &dirNative);

    if (HL_FAILED(result))
    {
        free(filePathNative);
        return result;
    }

    // Extract archives of type
    result = hl_INExtractArchives(filePathNative, dirNative);

    free(filePathNative);
    free(dirNative);
    return result;
#else
    return hl_INExtractArchives(filePath, dir);
#endif
}

HL_RESULT hl_ExtractArchivesNative(
    const hl_NativeChar* filePath, const hl_NativeChar* dir)
{
    if (!filePath || !dir) return HL_ERROR_INVALID_ARGS;
    return hl_INExtractArchives(filePath, dir);
}

hl_ArchiveFileEntry hl_CreateArchiveFileEntry(const char* filePath)
{
    // Create the file entry
    return
    {
        //HL_ARCHIVE_FILE_ENTRY_TYPE_FILEPATH,
        0, hl_PathGetNamePtr(filePath),
        filePath
    };
}

hl_ArchiveFileEntry* hl_CreateArchiveFileEntries(
    const char** files, size_t fileCount)
{
    // Create file entries
    hl_ArchiveFileEntry* fileEntries = static_cast<hl_ArchiveFileEntry*>(
        malloc(sizeof(hl_ArchiveFileEntry) * fileCount));

    for (size_t i = 0; i < fileCount; ++i)
    {
        fileEntries[i] = hl_CreateArchiveFileEntry(files[i]);
    }

    return fileEntries;
}
