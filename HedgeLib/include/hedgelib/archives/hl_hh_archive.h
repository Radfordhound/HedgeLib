#ifndef HL_HH_ARCHIVE_H_INCLUDED
#define HL_HH_ARCHIVE_H_INCLUDED
#include "hl_archive.h"
#include "../hl_compression.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HlStream HlStream;

#define HL_HH_DEFAULT_SPLIT_LIMIT       0xA01000
#define HL_HH_DEFAULT_ALIGNMENT         0x40
#define HL_HH_DEFAULT_ALIGNMENT_PFD     0x800

#define HL_HH_ARL_SIG                   HL_MAKE_SIG('A', 'R', 'L', '2')

HL_API extern const HlNChar HL_HH_ARL_EXT[5];
HL_API extern const HlNChar HL_HH_PFD_EXT[5];
HL_API extern const HlNChar HL_HH_PFI_EXT[5];
HL_API extern const HlNChar HL_HH_AR_EXT[4];

typedef struct HlHHArchiveHeader
{
    /** @brief Always 0? */
    HlU32 unknown1;
    /** @brief Size of this header. Always 0x10. The game doesn't seem to use it. */
    HlU32 headerSize;
    /** @brief Size of a single file entry struct. Always 0x14. The game doesn't seem to use it. */
    HlU32 entrySize;
    /** @brief The multiple all the data within the archive is aligned to. */
    HlU32 dataAlignment;
}
HlHHArchiveHeader;

HL_STATIC_ASSERT_SIZE(HlHHArchiveHeader, 16)

typedef struct HlHHArchiveFileEntry
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
HlHHArchiveFileEntry;

HL_STATIC_ASSERT_SIZE(HlHHArchiveFileEntry, 0x14)

typedef struct HlHHArchiveListHeader
{
    /** @brief "ARL2" */
    HlU32 signature;
    /** @brief The total number of split archives. */
    HlU32 splitCount;
}
HlHHArchiveListHeader;

HL_STATIC_ASSERT_SIZE(HlHHArchiveListHeader, 8)

typedef struct HlHHPackedFileEntry
{
    /** @brief Offset to the name of the file this entry represents. */
    HL_OFF32_STR nameOffset;
    /** @brief The absolute position of the file within the packed data (e.g. within the .pfd). */
    HlU32 dataPos;
    /** @brief The size of the file within the packed data (e.g. within the .pfd). */
    HlU32 dataSize;
}
HlHHPackedFileEntry;

HL_STATIC_ASSERT_SIZE(HlHHPackedFileEntry, 12)

typedef struct HlHHPackedFileIndexV0
{
    HlU32 entryCount;
    HL_OFF32(HL_OFF32(HlHHPackedFileEntry)) entriesOffset;
}
HlHHPackedFileIndexV0;

HL_STATIC_ASSERT_SIZE(HlHHPackedFileIndexV0, 8)

HL_API void hlHHPackedFileEntrySwap(HlHHPackedFileEntry* entry, HlBool swapOffsets);
HL_API void hlHHPackedFileIndexV0Swap(HlHHPackedFileIndexV0* pfi, HlBool swapOffsets);

HL_API HlResult hlHHArchiveParseInto(const HlHHArchiveHeader* HL_RESTRICT hhArc,
    size_t hhArcSize, HlArchive* HL_RESTRICT hlArc);

HL_API HlResult hlHHArchiveReadInto(void* HL_RESTRICT hhArc,
    size_t hhArcSize, HlArchive* HL_RESTRICT hlArc);

HL_API HlResult hlHHArchiveLoadSingleInto(const HlNChar* HL_RESTRICT filePath,
    HlBlobList* HL_RESTRICT hhArcs, HlArchive* HL_RESTRICT hlArc);

HL_API HlResult hlHHArchiveLoadAllInto(const HlNChar* HL_RESTRICT filePath,
    HlBlobList* HL_RESTRICT hhArcs, HlArchive* HL_RESTRICT hlArc);

HL_API HlResult hlHHArchiveLoad(const HlNChar* HL_RESTRICT filePath,
    HlBool loadSplits, HlBlobList* HL_RESTRICT hhArcs,
    HlArchive* HL_RESTRICT * HL_RESTRICT hlArc);

HL_API HlResult hlHHArchiveSave(const HlArchive* HL_RESTRICT arc, HlU32 splitLimit,
    HlU32 dataAlignment, HlCompressType compressType, HlBool generateARL,
    HlPackedFileIndex* HL_RESTRICT pfi, const HlNChar* HL_RESTRICT filePath);

HL_API HlResult hlHHPackedFileIndexV0Write(
    const HlPackedFileIndex* HL_RESTRICT pfi,
    size_t dataPos, HlOffTable* HL_RESTRICT offTable,
    HlStream* HL_RESTRICT stream);

HL_API HlResult hlHHPackedFileIndexWrite(
    const HlPackedFileIndex* HL_RESTRICT pfi,
    HlU32 version, size_t dataPos,
    HlOffTable* HL_RESTRICT offTable,
    HlStream* HL_RESTRICT stream);

HL_API HlResult hlHHPackedFileIndexSave(
    const HlPackedFileIndex* HL_RESTRICT pfi,
    HlU32 version, const HlNChar* HL_RESTRICT filePath);

#ifdef __cplusplus
}
#endif
#endif
