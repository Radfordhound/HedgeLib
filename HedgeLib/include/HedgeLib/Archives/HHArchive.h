#pragma once
#include "../Endian.h"
#include "../IO/IO.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char* const hl_ARExtension;
extern const char* const hl_PFDExtension;

enum HL_HHARCHIVE_COMPRESS_TYPE
{
    HL_HHARCHIVE_TYPE_UNCOMPRESSED = 0, // Not compressed.
    HL_HHARCHIVE_TYPE_CAB,              // CAB-Compressed. Used on PC.
    HL_HHARCHIVE_TYPE_XCOMPRESS,        // X-Compressed. Used on Xbox 360.
    HL_HHARCHIVE_TYPE_SEGS              // SEGS-Compressed. Used on PS3.
};

struct hl_DHHArchiveHeader
{
    uint32_t FileSize;          // HedgeLib stores the file size here when loading; in the file this is always 0.
    uint32_t FileEntriesOffset; // Offset to the first file entry? The game doesn't seem to use it?
    uint32_t Unknown2;          // Always 0x14?
    uint32_t PadAmount;         // The amount the file has been (or should be) padded to before each file is written.
};

struct hl_DHHArchiveFileEntry
{
    uint32_t EntrySize;     // The complete size of this entry, including padding and the file's data.
    uint32_t DataSize;      // The size of this file's data.
    uint32_t DataOffset;    // The offset to this file's data relative to the beginning of this struct.
    uint32_t Unknown1;      // Maybe hash or date? Game doesn't care if you set this to 0.
    uint32_t Unknown2;      // Maybe hash or date? Game doesn't care if you set this to 0.
};

HL_API enum HL_RESULT hl_LoadHHArchive(const char* filePath, struct hl_Blob** blob);
HL_API enum HL_RESULT hl_ExtractHHArchive(const struct hl_Blob* blob, const char* dir);

HL_API enum HL_RESULT hl_CreateHHArchive(const struct hl_ArchiveFileEntry* files,
    size_t fileCount, const char* dir, const char* name, uint32_t splitLimit,
    uint32_t padAmount, enum HL_HHARCHIVE_COMPRESS_TYPE compressType);

#ifdef __cplusplus
}
#endif
