#include "HedgeLib/Archives/HHArchive.h"
#include "HedgeLib/Archives/Archive.h"
#include "HedgeLib/IO/File.h"
#include "../IO/INPath.h"
#include "../INString.h"
#include "../INBlob.h"
#include <filesystem>

const char* const hl_ARExtension = ".ar";
const char* const hl_PFDExtension = ".pfd";

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
    return hl_INBINALoad(filePath, blob);
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
    const uint8_t* curPos = (blob->GetData<uint8_t>() + header->FileEntriesOffset);
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
            &fileEntry->FileName, &filePath);

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
