#pragma once
#include "../Endian.h"
#include "../String.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char* const hl_ARExtension;
extern const char* const hl_PFDExtension;
extern const char* const hl_ARLSignature;

#ifdef _WIN32
HL_API extern const hl_NativeChar* const hl_ARExtensionNative;
HL_API extern const hl_NativeChar* const hl_PFDExtensionNative;
#else
#define hl_ARExtensionNative hl_ARExtension
#define hl_PFDExtensionNative hl_PFDExtension
#endif

#define HL_GENS_DEFAULT_SPLIT_LIMIT     0xA00000
#define HL_GENS_DEFAULT_PAD_AMOUNT      0x40
#define HL_GENS_DEFAULT_PAD_AMOUNT_PFD  0x800

typedef struct hl_Blob hl_Blob;
typedef struct hl_Archive hl_Archive;

typedef enum HL_GENS_ARCHIVE_COMPRESS_TYPE
{
    /*! @brief Not compressed. Used by ARs on PC. */
    HL_GENS_ARCHIVE_TYPE_UNCOMPRESSED = 0,
    /*! @brief CAB-Compressed. Used by ARs within PFDs on PC. */
    HL_GENS_ARCHIVE_TYPE_CAB,
    /*! @brief X-Compressed. Used by ARs (by themselves and in PFDs) on Xbox 360. */
    HL_GENS_ARCHIVE_TYPE_XCOMPRESS,
    /*! @brief SEGS-Compressed. Used by ARs (by themselves and in PFDs) on PS3. */
    HL_GENS_ARCHIVE_TYPE_SEGS
}
HL_GENS_ARCHIVE_COMPRESS_TYPE;

typedef struct hl_GensArchiveHeader
{
    uint32_t FileSize;          // HedgeLib stores the file size here when loading; in the file this is always 0.
    uint32_t FileEntriesOffset; // Offset to the first file entry? The game doesn't seem to use it?
    uint32_t Unknown2;          // Always 0x14?
    uint32_t PadAmount;         // The amount the file has been (or should be) padded to before each file is written.
}
hl_GensArchiveHeader;

typedef struct hl_GensArchiveFileEntry
{
    uint32_t EntrySize;     // The complete size of this entry, including padding and the file's data.
    uint32_t DataSize;      // The size of this file's data.
    uint32_t DataOffset;    // The offset to this file's data relative to the beginning of this struct.
    uint32_t Unknown1;      // Maybe hash or date? Game doesn't care if you set this to 0.
    uint32_t Unknown2;      // Maybe hash or date? Game doesn't care if you set this to 0.
}
hl_GensArchiveFileEntry;

HL_API size_t hl_GensArchiveGetFileCount(const hl_Blob* blob);
HL_API HL_RESULT hl_LoadGensArchiveBlob(const char* filePath, hl_Blob** blob);
HL_API HL_RESULT hl_LoadGensArchiveBlobNative(const hl_NativeChar* filePath, hl_Blob** blob);

HL_API HL_RESULT hl_ExtractGensArchive(const hl_Blob* blob, const char* dir);
HL_API HL_RESULT hl_ExtractGensArchiveNative(const hl_Blob* blob, const hl_NativeChar* dir);

HL_API HL_RESULT hl_SaveGensArchive(const hl_Archive* arc,
    const char* filePath, uint32_t HL_DEFARG(splitLimit, HL_GENS_DEFAULT_SPLIT_LIMIT),
    uint32_t HL_DEFARG(padAmount, HL_GENS_DEFAULT_PAD_AMOUNT), bool HL_DEFARG(generateARL, true),
    HL_GENS_ARCHIVE_COMPRESS_TYPE HL_DEFARG(compressType, HL_GENS_ARCHIVE_TYPE_UNCOMPRESSED));

HL_API HL_RESULT hl_SaveGensArchiveNative(const hl_Archive* arc,
    const hl_NativeChar* filePath, uint32_t HL_DEFARG(splitLimit, HL_GENS_DEFAULT_SPLIT_LIMIT),
    uint32_t HL_DEFARG(padAmount, HL_GENS_DEFAULT_PAD_AMOUNT), bool HL_DEFARG(generateARL, true),
    HL_GENS_ARCHIVE_COMPRESS_TYPE HL_DEFARG(compressType, HL_GENS_ARCHIVE_TYPE_UNCOMPRESSED));

#ifdef __cplusplus
}

// Windows-specific overloads
#ifdef _WIN32
inline HL_RESULT hl_LoadGensArchiveBlob(const hl_NativeChar* filePath, hl_Blob** blob)
{
    return hl_LoadGensArchiveBlobNative(filePath, blob);
}

inline HL_RESULT hl_ExtractGensArchive(const hl_Blob* blob, const hl_NativeChar* dir)
{
    return hl_ExtractGensArchiveNative(blob, dir);
}

inline HL_RESULT hl_SaveGensArchive(const hl_Archive* arc,
    const hl_NativeChar* filePath, uint32_t splitLimit = HL_GENS_DEFAULT_SPLIT_LIMIT,
    uint32_t padAmount = HL_GENS_DEFAULT_PAD_AMOUNT, bool generateARL = true,
    HL_GENS_ARCHIVE_COMPRESS_TYPE compressType = HL_GENS_ARCHIVE_TYPE_UNCOMPRESSED)
{
    return hl_SaveGensArchiveNative(arc, filePath,
        splitLimit, padAmount, generateARL, compressType);
}
#endif
#endif
