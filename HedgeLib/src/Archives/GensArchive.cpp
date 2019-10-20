#include "HedgeLib/Archives/GensArchive.h"
#include "HedgeLib/Archives/Archive.h"
#include "HedgeLib/IO/Path.h"
#include "HedgeLib/IO/File.h"
#include "INArchive.h"
#include "../IO/INPath.h"
#include "../INString.h"
#include "../INBlob.h"
#include <algorithm>

const char* const hl_ARExtension = ".ar";
const char* const hl_PFDExtension = ".pfd";
const char* const hl_ARLSignature = "ARL2";

#ifdef _WIN32
const hl_NativeChar* const hl_ARExtensionNative = L".ar";
const hl_NativeChar* const hl_PFDExtensionNative = L".pfd";
#endif

size_t hl_INGensArchiveGetBufferSize(const hl_Blob* blob, size_t& fileCount)
{
    // Check if file size was set by hl_INLoadHHArchive
    size_t bufSize = 0;
    const hl_GensArchiveHeader* header = blob->GetData<hl_GensArchiveHeader>();
    if (!header->FileSize) return bufSize;

    // Get current position and end position
    const uint8_t* curPos = (blob->GetData<uint8_t>() + sizeof(*header));
    const uint8_t* endPos = (blob->GetData<uint8_t>() + header->FileSize);

    // Generate file entries
    while (curPos < endPos)
    {
        const hl_GensArchiveFileEntry* fileEntry = reinterpret_cast
            <const hl_GensArchiveFileEntry*>(curPos);

        // Increase buffer size enough to hold this file entry, its name, and its data
        const char* fileName = reinterpret_cast<const char*>(fileEntry + 1);
        bufSize += sizeof(hl_ArchiveFileEntry);
        bufSize += (strlen(fileName) + 1);
        bufSize += fileEntry->DataSize;

        // Increase file count
        ++fileCount;

        // Go to next file entry
        curPos += fileEntry->EntrySize;
    }

    return bufSize;
}

HL_RESULT hl_INCreateGensArchive(const hl_Blob* blob,
    hl_ArchiveFileEntry*& entries, uint8_t*& data)
{
    // Check if file size was set by hl_INLoadHHArchive
    const hl_GensArchiveHeader* header = blob->GetData<hl_GensArchiveHeader>();
    if (!header->FileSize) return HL_ERROR_UNKNOWN;

    // Get current position and end position
    const uint8_t* curPos = (blob->GetData<uint8_t>() + sizeof(*header));
    const uint8_t* endPos = (blob->GetData<uint8_t>() + header->FileSize);

    // Generate file entries
    while (curPos < endPos)
    {
        const hl_GensArchiveFileEntry* fileEntry = reinterpret_cast
            <const hl_GensArchiveFileEntry*>(curPos);

        // Set file size
        entries->Size = static_cast<size_t>(fileEntry->DataSize);

        // Copy file name
        const char* fileName = reinterpret_cast<const char*>(fileEntry + 1);
        size_t nameLen = (strlen(fileName) + 1);
        entries->Name = reinterpret_cast<const char*>(data);

        std::copy(fileName, fileName + nameLen, data);
        data += nameLen;

        // Copy file data
        const uint8_t* fileData = (curPos + fileEntry->DataOffset);
        std::copy(fileData, fileData + entries->Size, data);

        entries->Data = data;
        data += entries->Size;
        ++entries;

        // Go to next file entry
        curPos += fileEntry->EntrySize;
    }

    return HL_SUCCESS;
}

size_t hl_GensArchiveGetFileCount(const hl_Blob* blob)
{
    // Check if file size was set by hl_INLoadHHArchive
    size_t fileCount = 0;
    const hl_GensArchiveHeader* header = blob->GetData<hl_GensArchiveHeader>();
    if (!header->FileSize) return fileCount;

    // Get current position and end position
    const uint8_t* curPos = (blob->GetData<uint8_t>() + sizeof(*header));
    const uint8_t* endPos = (blob->GetData<uint8_t>() + header->FileSize);

    // Generate file entries
    while (curPos < endPos)
    {
        const hl_GensArchiveFileEntry* fileEntry = reinterpret_cast
            <const hl_GensArchiveFileEntry*>(curPos);

        // Increase file count
        ++fileCount;

        // Go to next file entry
        curPos += fileEntry->EntrySize;
    }

    return fileCount;
}

HL_RESULT hl_INLoadGensArchiveBlob(const hl_NativeChar* filePath, hl_Blob** blob)
{
    // Get the file's size
    size_t fileSize;
    HL_RESULT result = hl_PathGetSizeNative(filePath, &fileSize);
    if (HL_FAILED(result)) return result;

    // Create a blob big enough to store the archive
    *blob = hl_INCreateBlob(fileSize,
        HL_BLOB_FORMAT_GENERIC, HL_ARC_TYPE_GENS);

    if (!(*blob)) return HL_ERROR_OUT_OF_MEMORY;

    // Load the archive
    hl_File file;
    result = file.OpenReadNative(filePath);
    if (HL_FAILED(result)) return result;

    result = file.ReadBytes(&(*blob)->Data, fileSize);

    if (HL_FAILED(result))
    {
        free(*blob);
        return result;
    }

    // Set file size and return
    (*blob)->GetData<hl_GensArchiveHeader>()->FileSize =
        static_cast<uint32_t>(fileSize);

    return HL_SUCCESS;
}

HL_RESULT hl_LoadGensArchiveBlob(const char* filePath, hl_Blob** blob)
{
    if (!filePath || !blob) return HL_ERROR_INVALID_ARGS;

    HL_INSTRING_NATIVE_CALL(filePath,
        hl_INLoadGensArchiveBlob(nativeStr, blob));
}

HL_RESULT hl_LoadGensArchiveBlobNative(
    const hl_NativeChar* filePath, hl_Blob** blob)
{
    if (!filePath || !blob) return HL_ERROR_INVALID_ARGS;
    return hl_INLoadGensArchiveBlob(filePath, blob);
}

HL_RESULT hl_INExtractGensArchive(const hl_Blob* blob, const hl_NativeChar* dir)
{
    // Create directory for file extraction
    HL_RESULT result = hl_INPathCreateDirectory(dir);
    if (HL_FAILED(result)) return result;

    // Check if file size was set by hl_INLoadHHArchive
    const hl_GensArchiveHeader* header = blob->GetData<hl_GensArchiveHeader>();
    if (!header->FileSize) return HL_ERROR_UNKNOWN;

    // Get current position and end position
    const uint8_t* curPos = (blob->GetData<uint8_t>() + sizeof(*header));
    const uint8_t* endPos = (blob->GetData<uint8_t>() + header->FileSize);

    // Extract files
    hl_File file;
    hl_NativeChar* filePath;

    while (curPos < endPos)
    {
        const hl_GensArchiveFileEntry* fileEntry = reinterpret_cast
            <const hl_GensArchiveFileEntry*>(curPos);

        // Get file path
        result = hl_PathCombine(dir, reinterpret_cast<const char*>(
            fileEntry + 1), &filePath);

        if (HL_FAILED(result)) return result;

        // Create file
        result = file.OpenWriteNative(filePath);
        free(filePath);

        if (HL_FAILED(result)) return result;

        // Write data
        // TODO: Handle compressed files
        result = file.WriteBytes(curPos + fileEntry->DataOffset,
            static_cast<size_t>(fileEntry->DataSize));

        if (HL_FAILED(result)) return result;

        // Go to next file entry
        curPos += fileEntry->EntrySize;
    }

    return HL_SUCCESS;
}

HL_RESULT hl_ExtractGensArchive(const hl_Blob* blob, const char* dir)
{
    if (!blob || !dir) return HL_ERROR_INVALID_ARGS;
    HL_INSTRING_NATIVE_CALL(dir, hl_INExtractGensArchive(blob, nativeStr));
}

HL_RESULT hl_ExtractGensArchiveNative(
    const hl_Blob* blob, const hl_NativeChar* dir)
{
    if (!blob || !dir) return HL_ERROR_INVALID_ARGS;
    return hl_INExtractGensArchive(blob, dir);
}

HL_RESULT hl_INWriteGensArchive(const hl_File& file, const hl_File* arl,
    const hl_Archive* arc, size_t& fileIndex,
    uint32_t splitLimit, uint32_t padAmount,
    HL_GENS_ARCHIVE_COMPRESS_TYPE compressType)
{
    // Write the header
    hl_GensArchiveHeader header = { 0, 0x10, 0x14, padAmount };
    HL_RESULT result = file.Write(header);
    if (HL_FAILED(result)) return result;

    // If pad amount is less than 2, no padding is necessary
    // Note how we decrease padAmount by 1 here. This is necessary for
    // the algorithm which determines padding amount to work properly.
    if (padAmount-- < 2) padAmount = 0;

    // Create a dummy file entry
    hl_GensArchiveFileEntry fileEntry;
    fileEntry.Unknown1 = 0;
    fileEntry.Unknown2 = 0;

    // Write the file entries
    hl_File dataFile;
    size_t nameLen;
    uint32_t curPos, padSize = 0, arSize = sizeof(header);

    for (; fileIndex < arc->FileCount; ++fileIndex)
    {
        // Get file name size
        nameLen = (strlen(arc->Files[fileIndex].Name) + 1);

        // Compute entry size
        fileEntry.EntrySize = static_cast<uint32_t>(
            sizeof(fileEntry) + nameLen);

        if (padAmount)
        {
            // Padding has to be factored in
            curPos = (static_cast<uint32_t>(file.Tell()) +
                fileEntry.EntrySize);

            padSize = (((curPos + padAmount) & ~padAmount) - curPos);
            fileEntry.EntrySize += padSize;
        }

        // Get file size
        size_t fileSize;
        if (arc->Files[fileIndex].Size)
        {
            fileSize = arc->Files[fileIndex].Size;
        }
        else
        {
            result = hl_PathGetSize(static_cast<const char*>(
                arc->Files[fileIndex].Data), &fileSize);

            if (HL_FAILED(result)) return result;
        }

        // TODO: Support compression

        // Generate file entry
        fileEntry.DataSize = static_cast<uint32_t>(fileSize);
        fileEntry.DataOffset = fileEntry.EntrySize;
        fileEntry.EntrySize += fileEntry.DataSize;

        arSize += (fileEntry.EntrySize);

        // Write file entry
        result = file.Write(fileEntry);
        if (HL_FAILED(result)) return result;

        // Write file name
        result = file.WriteBytes(arc->Files[fileIndex].Name, nameLen);
        if (HL_FAILED(result)) return result;

        // Write padding
        result = file.WriteNulls(padSize);
        if (HL_FAILED(result)) return result;

        // Get file data
        const void* fileData;
        if (!arc->Files[fileIndex].Size)
        {
            // Open file
            dataFile.OpenRead(static_cast<const char*>(
                arc->Files[fileIndex].Data));

            // Read its data
            void* data = malloc(fileSize);
            result = dataFile.ReadBytes(data, fileSize);

            if (HL_FAILED(result))
            {
                free(data);
                return result;
            }

            fileData = data;
        }
        else
        {
            fileData = arc->Files[fileIndex].Data;
        }

        // Write file data
        result = file.WriteBytes(fileData, fileSize);

        // Free file data if necessary
        if (!arc->Files[fileIndex].Size)
        {
            free(const_cast<void*>(fileData));
        }

        // If we failed to write the data earlier, return
        if (HL_FAILED(result)) return result;

        // If we're generating a split, break if we've surpassed the split limit
        // Note how we check this *after* we've already written the file entry
        // as it seems that's how the actual game does it for some reason?
        if (splitLimit && arSize >= splitLimit) break;
    }

    // Write file size to arl if necessary and return
    if (arl) arl->Write(arSize);
    return HL_SUCCESS;
}

HL_RESULT hl_INSaveGensArchive(const hl_Archive* arc,
    const hl_NativeChar* filePath, uint32_t splitLimit, uint32_t padAmount,
    bool generateARL, HL_GENS_ARCHIVE_COMPRESS_TYPE compressType)
{
    // Create directory
    hl_NativeChar* dir;
    HL_RESULT result = hl_INPathGetParent(filePath, &dir);
    if (HL_FAILED(result)) return result;

    result = hl_INPathCreateDirectory(dir);
    free(dir);

    if (HL_FAILED(result)) return result;

    // Start writing ARL if the user requested it
    hl_File arl;

    if (generateARL)
    {
        // Create copy of filePath for ARL if necessary
        const hl_NativeChar* ext = hl_PathGetExtPtr(filePath);

        // Replace extension with .arl
        size_t nameLen = static_cast<size_t>(ext - filePath);
        hl_NStrPtr arlPath = HL_CREATE_NATIVE_STR(nameLen + 5);
        if (!arlPath) return result;

        std::copy(filePath, filePath + nameLen, arlPath.Get());

        arlPath[nameLen] = HL_NATIVE_TEXT('.');
        arlPath[nameLen + 1] = HL_NATIVE_TEXT('a');
        arlPath[nameLen + 2] = HL_NATIVE_TEXT('r');
        arlPath[nameLen + 3] = HL_NATIVE_TEXT('l');
        arlPath[nameLen + 4] = HL_NATIVE_TEXT('\0');

        // Create ARL
        result = arl.OpenWriteNative(arlPath);
        if (HL_FAILED(result)) return result;

        // Write ARL header
        result = arl.WriteBytes(hl_ARLSignature, 4);
        if (HL_FAILED(result)) return result;

        result = arl.WriteNulls(4);
        if (HL_FAILED(result)) return result;
    }

    // Create copy of filePath for splits if necessary
    const hl_NativeChar* arcPath;
    hl_NativeChar* splitCharPtr;
    hl_NStrPtr splitPath;

    if (splitLimit)
    {
        // Get split path
        result = hl_StringJoinNative(filePath,
            HL_NATIVE_TEXT(".00"), &splitCharPtr);

        if (HL_FAILED(result)) return result;

        splitPath = splitCharPtr;
        arcPath = splitPath;

        // Get split character pointer
        splitCharPtr = (splitPath + (hl_StrLenNative(splitPath) - 1));
    }
    else
    {
        // We're not generating splits; just use the given filePath
        arcPath = filePath;
    }

    // Generate archives
    uint32_t splitCount = 0;
    hl_File file;

    for (size_t i = 0; i < arc->FileCount;) // i is incremented by hl_INWriteGensArchive
    {
        // Generate archive
        result = file.OpenWrite(arcPath);
        if (HL_FAILED(result)) return result;

        result = hl_INWriteGensArchive(file, &arl, arc,
            i, splitLimit, padAmount, compressType);

        if (HL_FAILED(result)) return result;

        // Increase split count
        ++splitCount;

        // Get the path of the next split if we're generating splits
        // If we're not generating splits, i should now == arc->FileCount
        if (splitLimit && !hl_INArchiveNextSplit(splitCharPtr))
        {
            // ERROR: We've surpassed 99 splits
            return HL_ERROR_UNSUPPORTED;
        }
    }

    // Write ARL file names
    for (size_t i = 0; i < arc->FileCount; ++i)
    {
        // Write file name length
        size_t fileNameLen = strlen(arc->Files[i].Name);
        uint8_t len = (fileNameLen > 255) ? 255 :
            static_cast<uint8_t>(fileNameLen); // TODO: Is this really a single byte? It might be like C# strings

        result = arl.Write(len);
        if (HL_FAILED(result)) return result;

        // Write file name
        result = arl.WriteBytes(arc->Files[i].Name, fileNameLen);
        if (HL_FAILED(result)) return result;
    }

    // Fill-in ARL split count if necessary
    if (generateARL)
    {
        result = arl.JumpTo(4);
        if (HL_FAILED(result)) return result;

        result = arl.Write(splitCount);
        if (HL_FAILED(result)) return result;
    }

    return HL_SUCCESS;
}

HL_RESULT hl_SaveGensArchive(const hl_Archive* arc,
    const char* filePath, uint32_t splitLimit,
    uint32_t padAmount, bool generateARL,
    HL_GENS_ARCHIVE_COMPRESS_TYPE compressType)
{
    if (!arc || !filePath) return HL_ERROR_INVALID_ARGS;
    HL_INSTRING_NATIVE_CALL(filePath, hl_INSaveGensArchive(arc, nativeStr,
        splitLimit, padAmount, generateARL, compressType));
}

HL_RESULT hl_SaveGensArchiveNative(const hl_Archive* arc,
    const hl_NativeChar* filePath, uint32_t splitLimit,
    uint32_t padAmount, bool generateARL,
    HL_GENS_ARCHIVE_COMPRESS_TYPE compressType)
{
    if (!arc || !filePath) return HL_ERROR_INVALID_ARGS;
    return hl_INSaveGensArchive(arc, filePath, splitLimit,
        padAmount, generateARL, compressType);
}
