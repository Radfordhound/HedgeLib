#ifndef HL_GENS_ARCHIVE_H_INCLUDED
#define HL_GENS_ARCHIVE_H_INCLUDED
#include "hl_archive.h"
#include "../hl_compress_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HlFile HlFile;
typedef struct HlBlob HlBlob;

#define HL_GENS_DEFAULT_SPLIT_LIMIT     0xA00000
#define HL_GENS_DEFAULT_PAD_AMOUNT      0x40
#define HL_GENS_DEFAULT_PAD_AMOUNT_PFD  0x800

#define HL_GENS_ARL_SIG                 HL_MAKE_SIG('A', 'R', 'L', '2')

HL_API extern const HlNChar HL_GENS_ARL_EXT[5];
HL_API extern const HlNChar HL_GENS_AR_EXT[4];
HL_API extern const HlNChar HL_GENS_PFD_EXT[5];

typedef struct HlGensArchiveHeader
{
    /** @brief Always 0? */
    HlU32 unknown1;
    /** @brief Offset to the first file entry? The game doesn't seem to use it? */
    HlU32 fileEntriesOffset;
    /** @brief Always 0x14? */
    HlU32 unknown2;
    /** @brief The amount of padding written before each file. */
    HlU32 padAmount;
}
HlGensArchiveHeader;

HL_STATIC_ASSERT_SIZE(HlGensArchiveHeader, 16);

typedef struct HlGensArchiveFileEntry
{
    /** @brief The complete size of this entry, including padding and the file's data. */
    HlU32 entrySize;
    /** @brief The size of this file's data. */
    HlU32 dataSize;
    /** @brief The offset to this file's data relative to the beginning of this struct. */
    HlU32 dataOffset;
    /** @brief Hash or date? Unleashed/Gens don't care if you set this to 0. */
    HlU32 unknown1;
    /** @brief Hash or date? Unleashed/Gens don't care if you set this to 0. */
    HlU32 unknown2;
}
HlGensArchiveFileEntry;

HL_STATIC_ASSERT_SIZE(HlGensArchiveFileEntry, 20);

typedef struct HlGensArchiveListHeader
{
    /** @brief "ARL2" */
    HlU32 signature;
    /** @brief The total number of split archives. */
    HlU32 splitCount;
}
HlGensArchiveListHeader;

HL_API HlResult hlGensArchiveStreamPFI(const HlNChar* HL_RESTRICT filePath,
    HlArchive* HL_RESTRICT * HL_RESTRICT archive);

HL_API HlResult hlGensArchiveRead(const HlBlob* HL_RESTRICT * HL_RESTRICT splits,
    size_t splitCount, HlArchive* HL_RESTRICT * HL_RESTRICT archive);

HL_API HlResult hlGensArchiveLoad(const HlNChar* HL_RESTRICT filePath,
    HlBool loadSplits, HlArchive* HL_RESTRICT * HL_RESTRICT archive);

HL_API HlResult hlGensArchiveSave(const HlArchive* arc, HlU32 splitLimit,
    HlU32 padAmount, HlCompressType compressType, HlBool generateARL,
    const HlNChar* filePath);

#ifdef __cplusplus
}
#endif
#endif
