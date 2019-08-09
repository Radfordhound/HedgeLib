#include "HedgeLib/Archives/HHArchive.h"
#include "HedgeLib/Archives/Archive.h"
#include "HedgeLib/IO/Path.h"
#include "HedgeLib/IO/File.h"
#include "../IO/INPath.h"
#include "../INString.h"
#include "../INBlob.h"
#include <filesystem>
#include <cstring>

const char* const hl_ARExtension = ".ar";
const char* const hl_PFDExtension = ".pfd";
const char* const hl_ARLSignature = "ARL2";

bool hl_INArchiveNextSplit(hl_INNativeStr splitCharPtr, bool pacv3);

HL_RESULT hl_INLoadHHArchive(const hl_INNativeStr filePath,
    struct hl_Blob** blob)
{
    // Get the file's size
    size_t fileSize;
    HL_RESULT result = hl_File::GetSize(filePath, fileSize);
    if (HL_FAILED(result)) return result;

    // Create a blob big enough to store the archive
    *blob = hl_INCreateBlob(fileSize,
        HL_BLOB_FORMAT_UNKNOWN, HL_ARC_TYPE_HEDGEHOG);

    if (!(*blob)) return HL_ERROR_OUT_OF_MEMORY;

    // Load the archive
    hl_File file = hl_File(filePath);
    result = file.ReadBytes(&(*blob)->Data, fileSize);

    if (HL_FAILED(result))
    {
        hl_FreeBlob(*blob);
        return result;
    }

    // Set file size and return
    (*blob)->GetData<hl_DHHArchiveHeader>()->FileSize =
        static_cast<uint32_t>(fileSize);

    return HL_SUCCESS;
}

enum HL_RESULT hl_LoadHHArchive(const char* filePath, struct hl_Blob** blob)
{
    if (!filePath) return HL_ERROR_UNKNOWN;

#ifdef _WIN32
    // Convert UTF-8 path to wide UTF-16 path
    hl_INNativeStr nativePath;
    HL_RESULT result = hl_INWin32StringConvertToNative(filePath, &nativePath);
    if (HL_FAILED(result)) return result;

    result = hl_INLoadHHArchive(nativePath, blob);
    std::free(nativePath);
    return result;
#else
    return hl_INLoadHHArchive(filePath, blob);
#endif
}

enum HL_RESULT hl_ExtractHHArchive(const struct hl_Blob* blob, const char* dir)
{
    // Create directory for file extraction
    if (!blob || !dir) return HL_ERROR_UNKNOWN;

    // TODO: Create directory without std::filesystem
    std::filesystem::path fdir = std::filesystem::u8path(dir);
    std::filesystem::create_directory(fdir);

    // Check if file size was set by hl_INLoadHHArchive
    const hl_DHHArchiveHeader* header = blob->GetData<hl_DHHArchiveHeader>();
    if (!header->FileSize) return HL_ERROR_UNKNOWN;

    // Get current position and end position
    const uint8_t* curPos = (blob->GetData<uint8_t>() + sizeof(*header));
    const uint8_t* endPos = (blob->GetData<uint8_t>() + header->FileSize);

    // Extract files
    hl_File file;
    char* filePath;

    while (curPos < endPos)
    {
        const hl_DHHArchiveFileEntry* fileEntry = reinterpret_cast
            <const hl_DHHArchiveFileEntry*>(curPos);

        // Get file path
        HL_RESULT result = hl_INPathCombine(dir,
            reinterpret_cast<const char*>(fileEntry + 1), &filePath);

        if (HL_FAILED(result)) return result;

        // Write data
        // TODO: Handle compressed files
        result = file.OpenWrite(filePath);
        if (HL_FAILED(result)) return result;

        result = file.WriteBytes(curPos + fileEntry->DataOffset,
            static_cast<size_t>(fileEntry->DataSize));

        if (HL_FAILED(result)) return result;

        // Go to next file entry
        curPos += fileEntry->EntrySize;
        std::free(filePath);
        file.Close();
    }

    return HL_SUCCESS;
}

HL_RESULT hl_INCreateHHArchive(const hl_File& file, const hl_File* arl,
    const hl_ArchiveFileEntry* files, size_t& fileIndex, size_t fileCount,
    uint32_t splitLimit, uint32_t padAmount,
    HL_HHARCHIVE_COMPRESS_TYPE compressType)
{
    // Write the header
    hl_DHHArchiveHeader header = { 0, 0x10, 0x14, padAmount};
    HL_RESULT result = file.Write(header);
    if (HL_FAILED(result)) return result;

    // If pad amount is less than 2, no padding is necessary
    if (padAmount-- < 2) padAmount = 0;

    // Create a dummy file entry
    hl_DHHArchiveFileEntry fileEntry;
    fileEntry.Unknown1 = 0;
    fileEntry.Unknown2 = 0;

    // Write the file entries
    uint32_t curPos;
    size_t nameLen, padSize = 0;
    uint32_t arSize = 0x10;

    for (; fileIndex < fileCount; ++fileIndex)
    {
        // Get file name size
        nameLen = (std::strlen(files[fileIndex].Name) + 1);

        // Compute entry size
        fileEntry.EntrySize = static_cast<uint32_t>(
            sizeof(fileEntry) + nameLen);

        if (padAmount)
        {
            // Padding has to be factored in
            padSize = static_cast<size_t>(padAmount);
            curPos = (static_cast<uint32_t>(file.Tell()) +
                fileEntry.EntrySize);

            padSize = (((curPos + padSize) & ~padSize) - curPos);
            fileEntry.EntrySize += static_cast<uint32_t>(padSize);
        }

        // Get file size
        size_t fileSize;
        if (files[fileIndex].Size)
        {
            fileSize = files[fileIndex].Size;
        }
        else
        {
            result = hl_File::GetSize(static_cast<const char*>(
                files[fileIndex].Data), fileSize);

            if (HL_FAILED(result)) return result;
        }

        // TODO: Support compression

        // Generate file entry
        fileEntry.DataSize = static_cast<uint32_t>(fileSize);
        fileEntry.DataOffset = fileEntry.EntrySize;
        fileEntry.EntrySize += fileEntry.DataSize;

        if ((arSize + fileEntry.EntrySize) >= splitLimit) break;
        arSize += (fileEntry.EntrySize);

        // Write file entry
        result = file.Write(fileEntry);
        if (HL_FAILED(result)) return result;

        // Write file name
        result = file.WriteBytes(files[fileIndex].Name, nameLen);
        if (HL_FAILED(result)) return result;

        // Write padding
        result = file.WriteNulls(padSize);
        if (HL_FAILED(result)) return result;

        // Get file data
        const void* fileData;
        if (!files[fileIndex].Size)
        {
            // Read file and write data to PAC
            hl_File dataFile = hl_File(
                static_cast<const char*>(files[fileIndex].Data));

            void* data = std::malloc(fileSize);
            result = dataFile.ReadBytes(data, fileSize);
            if (HL_FAILED(result))
            {
                std::free(data);
                return result;
            }

            result = dataFile.Close();
            if (HL_FAILED(result))
            {
                std::free(data);
                return result;
            }

            fileData = data;
        }
        else
        {
            fileData = files[fileIndex].Data;
        }

        // Write file data
        result = file.WriteBytes(fileData, fileSize);

        // Free file data if necessary
        if (!files[fileIndex].Size)
        {
            std::free(const_cast<void*>(fileData));
        }

        // If we failed to write the data earlier, return
        if (HL_FAILED(result)) return result;
    }

    // Write file size to arl if necessary and return
    if (arl) arl->Write(arSize);
    return HL_SUCCESS;
}

enum HL_RESULT hl_CreateHHArchive(const struct hl_ArchiveFileEntry* files,
    size_t fileCount, const char* dir, const char* name, uint32_t splitLimit,
    uint32_t padAmount, enum HL_HHARCHIVE_COMPRESS_TYPE compressType)
{
    if (!dir || !name || !*dir) return HL_ERROR_UNKNOWN;

    // Create directory
    HL_RESULT result = hl_PathCreateDirectory(dir);
    if (HL_FAILED(result)) return result;

    // Determine path lengths
    size_t dirLen = std::strlen(dir);
    size_t nameLen = std::strlen(name);

    // Check if we need to append a slash to the end of dir in the combined filePath
    bool addSlash = hl_INPathCombineNeedsSlash(dir, name, dirLen);
    if (addSlash) ++dirLen;

    // Malloc buffer large enough to hold all paths
    char* filePath = static_cast<char*>(std::malloc(
        dirLen + ++nameLen + 6));

    if (!filePath) return HL_ERROR_OUT_OF_MEMORY;

    // Combine directory and name to form root path
    hl_INPathCombineNoAlloc(dir, name, dirLen,
        nameLen, filePath, addSlash);

    // Add root extension to file path
    // TODO: Provide some way for the user to specify they want a .pfd instead
    filePath[dirLen + --nameLen] = '.';
    filePath[dirLen + ++nameLen] = 'a';
    filePath[dirLen + ++nameLen] = 'r';
    filePath[dirLen + ++nameLen] = '.';
    filePath[dirLen + ++nameLen] = '0';
    filePath[dirLen + ++nameLen] = '0';
    filePath[dirLen + ++nameLen] = '\0';

    // Get native path
    hl_INNativeStr nativePath;

#ifdef _WIN32
    // Figure out the amount of characters in the UTF-8 string
    size_t strLen = hl_INStringGetReqUTF16CharCountUTF8(filePath);

    // Convert path from UTF-8 to a native (UTF-16) path
    result = hl_INStringConvertUTF8ToUTF16(filePath,
        reinterpret_cast<uint16_t**>(&nativePath), strLen);

    // Free UTF-8 path and return if native path was improperly converted
    std::free(filePath);
    if (HL_FAILED(result)) return result;    
#else
    // No conversion is necessary on POSIX systems which support UTF-8
    size_t strLen = (dirLen + ++nameLen); // TODO: Is this right??
    nativePath = filePath;
#endif

    // Get ARL path
    nativePath[strLen - 4] = 'l';
    nativePath[strLen - 3] = '\0';

    // Start writing ARL
    uint32_t splitCount = 0;
    hl_File file, arl = hl_File(nativePath, HL_FILEMODE_WRITE_BINARY);

    arl.WriteBytes(hl_ARLSignature, 4);
    arl.WriteNulls(4);

    // Generate splits if necessary
    size_t fileIndex = 0;
    if (fileIndex < fileCount)
    {
        // Get the first split's path
        nativePath[strLen - 4] = '.';
        nativePath[strLen - 3] = '0';
        nativePath[strLen - 2] = '0';
        nativePath[strLen - 1] = '\0';

        hl_INNativeStr splitCharPtr = (nativePath + (strLen - 2));

        // Generate splits
        bool withinSplitLimit = true;
        do
        {
            // Generate split archive
            file.OpenWrite(nativePath);
            result = hl_INCreateHHArchive(file, &arl, files, fileIndex,
                fileCount, splitLimit, padAmount, compressType);

            if (HL_FAILED(result))
            {
                std::free(nativePath);
                return result;
            }

            // Increase split count
            ++splitCount;
        }
        while (fileIndex < fileCount && (withinSplitLimit =
            hl_INArchiveNextSplit(splitCharPtr, false)));

        // ERROR: We've surpassed 99 splits
        if (!withinSplitLimit)
        {
            std::free(nativePath);
            return HL_ERROR_UNKNOWN;
        }
    }

    // Write ARL file names
    for (fileIndex = 0; fileIndex < fileCount; ++fileIndex)
    {
        // Write file name length
        size_t fileNameLen = std::strlen(files[fileIndex].Name);
        uint8_t len = static_cast<uint8_t>(fileNameLen); // TODO: Is this really a single byte? It might be like C# strings

        result = arl.Write(len);
        if (HL_FAILED(result))
        {
            std::free(nativePath);
            return result;
        }

        // Write file name
        result = arl.WriteBytes(files[fileIndex].Name, fileNameLen);
        if (HL_FAILED(result))
        {
            std::free(nativePath);
            return result;
        }
    }

    // Fill-in ARL split count
    arl.JumpTo(4);
    arl.Write(splitCount);

    // Free native path and return
    std::free(nativePath);
    return HL_SUCCESS;
}
