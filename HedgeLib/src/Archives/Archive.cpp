#include "HedgeLib/Archives/Archive.h"
#include "HedgeLib/Archives/PACx.h"
#include "../INBlob.h"
#include <cstring>
#include <cstdlib>

void hl_ExtractArchive(const struct hl_Blob* blob, const char* dir)
{
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

void hl_CreateArchiveFileEntry(const char* filePath,
    struct hl_ArchiveFileEntry* entry)
{
    // Get the file name (without the directory)
    size_t filePathLen = std::strlen(filePath);
    const char* name = nullptr;

    if (filePathLen > 0)
    {
        for (size_t i = (filePathLen - 1); i >= 0; --i)
        {
            if (filePath[i] == '/' || filePath[i] == '\\')
            {
                name = &filePath[i + 1];
                break;
            }
        }
    }

    // Create the file entry
    *entry =
    {
        //HL_ARCHIVE_FILE_ENTRY_TYPE_FILEPATH,
        0, (name) ? name : filePath,
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
