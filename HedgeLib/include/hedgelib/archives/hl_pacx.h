#ifndef HL_PACX_H_INCLUDED
#define HL_PACX_H_INCLUDED
#include "hl_archive.h"
#include "../hl_blob.h"
#include "../io/hl_bina.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HL_PACX_SIG                         HL_MAKE_SIG('P', 'A', 'C', 'x')
#define HL_PACX_DEFAULT_ALIGNMENT           16
#define HL_PACXV402_DEFAULT_CHUNK_SIZE      65536

#define HL_LW_DEFAULT_SPLIT_LIMIT           0xA00000U
#define HL_RIO_DEFAULT_SPLIT_LIMIT          0xA00000U
#define HL_FORCES_DEFAULT_SPLIT_LIMIT       0x1E00000U
#define HL_TOKYO1_DEFAULT_SPLIT_LIMIT       0x1E00000U
#define HL_TOKYO2_DEFAULT_SPLIT_LIMIT       0x1E00000U
#define HL_SAKURA_DEFAULT_SPLIT_LIMIT       0x1E00000U
#define HL_PPT2_DEFAULT_SPLIT_LIMIT         0x1E00000U

HL_API extern const HlNChar HL_PACX_EXT[5];

HL_API extern const char* const HlPACxDataTypes[];
HL_API extern const size_t HlPACxDataTypeCount;

typedef enum HlPACxSupportedExtFlags
{
    HL_PACX_EXT_FLAGS_TYPE_MASK = 3,

    /** @brief This type can appear in root PACs. */
    HL_PACX_EXT_FLAGS_ROOT_TYPE = 1,
    /** @brief This type can appear in split PACs. */
    HL_PACX_EXT_FLAGS_SPLIT_TYPE = 2,
    /** @brief This type can appear in both root and split PACs. */
    HL_PACX_EXT_FLAGS_MIXED_TYPE = (HL_PACX_EXT_FLAGS_ROOT_TYPE | HL_PACX_EXT_FLAGS_SPLIT_TYPE),
    /**
       @brief The BINA string and offset tables of this type are to be "merged"
       into the global string and offset tables of the PAC file(s).
    */
    HL_PACX_EXT_FLAGS_V2_MERGED_TYPE = (
        HL_PACX_EXT_FLAGS_ROOT_TYPE | 4)
}
HlPACxSupportedExtFlags;

typedef struct HlPACxSupportedExt
{
    /**
        @brief A lowercased extension, without a dot at the beginning, that
        is supported/used by this version of PACx (e.g. "dds").
    */
    const HlNChar* const ext;
    /**
        @brief The index of the PACx data type for this extension (e.g. "ResTexture")
        within the HlPACxDataTypes array.
    */
    const unsigned short pacxDataTypeIndex;
    /** @brief See HlPACxSupportedExtFlags. */
    const unsigned short flags;
    /** @brief A number indicating how this type will be sorted; smaller == higher priority. */
    const short rootSortWeight;
    /** @brief A number indicating how this type will be sorted; smaller == higher priority. */
    const short splitSortWeight;
}
HlPACxSupportedExt;

HL_API extern const HlPACxSupportedExt HlPACxLWExts[];
HL_API extern const size_t HlPACxLWExtCount;

HL_API extern const HlPACxSupportedExt HlPACxRioExts[];
HL_API extern const size_t HlPACxRioExtCount;

HL_API extern const HlPACxSupportedExt HlPACxForcesExts[];
HL_API extern const size_t HlPACxForcesExtCount;

HL_API extern const HlPACxSupportedExt HlPACxTokyo1Exts[];
HL_API extern const size_t HlPACxTokyo1ExtCount;

HL_API extern const HlPACxSupportedExt HlPACxTokyo2Exts[];
HL_API extern const size_t HlPACxTokyo2ExtCount;

HL_API extern const HlPACxSupportedExt HlPACxSakuraExts[];
HL_API extern const size_t HlPACxSakuraExtCount;

HL_API extern const HlPACxSupportedExt HlPACxPPT2Exts[];
HL_API extern const size_t HlPACxPPT2ExtCount;

typedef HlBINAV2Header HlPACxV2Header;
typedef HlBINAV2BlockHeader HlPACxV2BlockHeader;

typedef struct HlPACxV2BlockDataHeader
{
    /** @brief Used to determine what type of block this is. See HlBINAV2BlockType. */
    HlU32 signature;
    /** @brief The complete size of the block, including this header. */
    HlU32 size;
    /**
       @brief The combined size, in bytes, of every HlPACxV2DataEntry
       (and its corresponding data) in the file, including padding.
    */
    HlU32 dataEntriesSize;
    /**
       @brief The combined size, in bytes, of every HlPACxV2NodeTree
       (and its corresponding HlPACxV2Nodes) in the file.
    */
    HlU32 treesSize;
    /** @brief TODO */
    HlU32 proxyTableSize;
    /** @brief The size of the string table in bytes, including padding. */
    HlU32 stringTableSize;
    /** @brief The size of the offset table in bytes, including padding. */
    HlU32 offsetTableSize;
    /** @brief Always 1? Probably a boolean? */
    HlU8 unknown1;
    /** @brief Included so fwrite won't write a byte of garbage. */
    HlU8 padding1;
    /** @brief Included so fwrite won't write 2 bytes of garbage. */
    HlU16 padding2;
}
HlPACxV2BlockDataHeader;

HL_STATIC_ASSERT_SIZE(HlPACxV2BlockDataHeader, 0x20)

typedef struct HlPACxV2Node
{
    HL_OFF32_STR name;
    HL_OFF32(void) data;
}
HlPACxV2Node;

HL_STATIC_ASSERT_SIZE(HlPACxV2Node, 8)

typedef struct HlPACxV2NodeTree
{
    HlU32 nodeCount;
    HL_OFF32(HlPACxV2Node) nodes;
}
HlPACxV2NodeTree;

HL_STATIC_ASSERT_SIZE(HlPACxV2NodeTree, 8)

typedef enum HlPACxV2DataFlags
{
    HL_PACXV2_DATA_FLAGS_NONE = 0,
    /**
       @brief Indicates that this is a "proxy entry" - that is, an entry which
       doesn't actually contain any data. The actual data is contained within
       a separate HlPACxV2DataEntry located in a separate "split" packfile.
    */
    HL_PACXV2_DATA_FLAGS_NOT_HERE = 0x80
}
HlPACxV2DataFlags;

typedef struct HlPACxV2DataEntry
{
    /**
       @brief The size of the data contained within this entry, including
       padding, starting immediately after this HlPACxV2DataEntry struct.
    */
    HlU32 dataSize;
    /** @brief Always 0? */
    HlU32 unknown1;
    /** @brief Always 0? */
    HlU32 unknown2;
    /** @brief See HlPACxV2DataFlags. */
    HlU8 flags;
    /** @brief Included so fwrite won't write a byte of garbage. */
    HlU8 padding1;
    /** @brief Included so fwrite won't write 2 bytes of garbage. */
    HlU16 padding2;
}
HlPACxV2DataEntry;

HL_STATIC_ASSERT_SIZE(HlPACxV2DataEntry, 16)

typedef struct HlPACxV2SplitTable
{
    /**
       @brief Pointer to an array of pointers to strings
       reprsenting the filenames of every "split" packfile.
    */
    HL_OFF32(HL_OFF32_STR) splitNames;
    HlU32 splitCount;
}
HlPACxV2SplitTable;

HL_STATIC_ASSERT_SIZE(HlPACxV2SplitTable, 8)

typedef struct HlPACxV2ProxyEntry
{
    /**
       @brief Pointer to a string formatted as such: "ext:restype", where "ext"
       is the file's extension without the dot at the beginning, and "restype"
       is the game's internal name used to represent data of this type.
       
       For example: "dds:ResTexture", or "material:ResMirageMaterial".
    */
    HL_OFF32_STR type;
    /** @brief Pointer to a string representing the file's name without an extension. */
    HL_OFF32_STR name;
    /**
       @brief Index of the corresponding HlPACxV2Node within the
       HlPACxV2NodeTree for the type specified by type.
    */
    HlU32 nodeIndex;
}
HlPACxV2ProxyEntry;

HL_STATIC_ASSERT_SIZE(HlPACxV2ProxyEntry, 12)

typedef struct HlPACxV2ProxyEntryTable
{
    HlU32 proxyEntryCount;
    HL_OFF32(HlPACxV2ProxyEntry) proxyEntries;
}
HlPACxV2ProxyEntryTable;

HL_STATIC_ASSERT_SIZE(HlPACxV2ProxyEntryTable, 8)

/* Thanks to Skyth for cracking the majority of the PACxV3 format! */
typedef struct HlPACxV3SplitEntry
{
    HL_OFF64_STR nameOffset;
}
HlPACxV3SplitEntry;

HL_STATIC_ASSERT_SIZE(HlPACxV3SplitEntry, 8)

typedef struct HlPACxV3SplitTable
{
    HlU64 splitCount;
    HL_OFF64(void) splitEntries;
}
HlPACxV3SplitTable;

HL_STATIC_ASSERT_SIZE(HlPACxV3SplitTable, 16)

typedef enum PACxV3DataType
{
    HL_PACXV3_DATA_TYPE_REGULAR_FILE = 0,
    HL_PACXV3_DATA_TYPE_NOT_HERE = 1,
    HL_PACXV3_DATA_TYPE_BINA_FILE = 2
}
PACxV3DataType;

typedef struct HlPACxV3DataEntry
{
    /**
        @brief Same as HlPACxV3Header.uid in PACxV3.
        Different number for each data entry in PACxV4.
    */
    HlU32 uid;
    HlU32 dataSize;
    /** @brief Always 0? Unknown1 from PACxV2DataEntry?? */
    HlU64 unknown2;
    HL_OFF64(void) dataOffset;
    /** @brief Always 0? Unknown2 from PACxV2DataEntry?? */
    HlU64 unknown3;
    HL_OFF64_STR extOffset;
    /** @brief Probably actually just a single byte with 7 bytes of padding?? */
    HlU64 dataType;
}
HlPACxV3DataEntry;

HL_STATIC_ASSERT_SIZE(HlPACxV3DataEntry, 0x30)

typedef struct HlPACxV3Node
{
    HL_OFF64_STR nameOffset;
    HL_OFF64(void) dataOffset;
    HL_OFF64(HlS32) childIndicesOffset;
    HlS32 parentIndex;
    HlS32 globalIndex;
    HlS32 dataIndex;
    HlU16 childCount;
    HlU8 hasData;
    /** @brief Where this node's name should be copied to within the global name buffer. */
    HlU8 bufStartIndex;
}
HlPACxV3Node;

HL_STATIC_ASSERT_SIZE(HlPACxV3Node, 0x28)

typedef struct HlPACxV3NodeTree
{
    HlU32 nodeCount;
    HlU32 dataNodeCount;
    HL_OFF64(HlPACxV3Node) nodesOffset;
    HL_OFF64(HlS32) dataNodeIndicesOffset;
}
HlPACxV3NodeTree;

HL_STATIC_ASSERT_SIZE(HlPACxV3NodeTree, 0x18)

typedef enum HlPACxV3Type
{
    HL_PACXV3_TYPE_IS_ROOT = 1,
    HL_PACXV3_TYPE_IS_SPLIT = 2,
    HL_PACXV3_TYPE_HAS_SPLITS = 4,
    HL_PACXV3_TYPE_UNKNOWN = 8
}
HlPACxV3Type;

typedef struct HlPACxV3Header
{
    /** @brief "PACx" */
    HlU32 signature;
    /** @brief Version Number. */
    HlU8 version[3];
    /** @brief 'B' for Big Endian, 'L' for Little Endian. */
    HlU8 endianFlag;
    /** @brief A random number that serves as a unique identifier for this pac file. */
    HlU32 uid;
    HlU32 fileSize;
    HlU32 treesSize;
    HlU32 splitTableSize;
    HlU32 dataEntriesSize;
    /** @brief The size of the string table in bytes, including padding. */
    HlU32 stringTableSize;
    HlU32 fileDataSize;
    /** @brief The size of the offset table in bytes, including padding. */
    HlU32 offsetTableSize;
    /** @brief Bitwise-and this with values from the PACxV3Type enum. */
    HlU16 type;
    /** @brief Always 0x108? */
    HlU16 unknown2;
    HlU32 splitCount;
}
HlPACxV3Header;

HL_STATIC_ASSERT_SIZE(HlPACxV3Header, 0x30)

/**
   @brief Information explaining how to decompress a blob of LZ4-compressed data.

   When decompressing the root pac allocate a buffer of
   size header.rootUncompressedSize, then loop through these
   chunks and decompress each one, one-by-one, into that buffer.

   If you try to decompress all at once instead the data can be corrupted.
*/
typedef struct HlPACxV402Chunk
{
    HlU32 compressedSize;
    HlU32 uncompressedSize;
}
HlPACxV402Chunk;

HL_STATIC_ASSERT_SIZE(HlPACxV402Chunk, 8)

typedef HL_LIST(HlPACxV402Chunk) HlPACxV402ChunkList;

typedef struct HlPACxV402SplitEntry
{
    HL_OFF64_STR nameOffset;
    HlU32 compressedSize;
    HlU32 uncompressedSize;
    HlU32 splitPos;
    HlU32 chunkCount;
    HL_OFF64(HlPACxV402Chunk) chunksOffset;
}
HlPACxV402SplitEntry;

HL_STATIC_ASSERT_SIZE(HlPACxV402SplitEntry, 0x20)

typedef struct HlPACxV403SplitEntry
{
    HL_OFF64_STR nameOffset;
    HlU32 compressedSize;
    HlU32 uncompressedSize;
    HlU32 splitPos;
    HlU32 padding;
}
HlPACxV403SplitEntry;

HL_STATIC_ASSERT_SIZE(HlPACxV403SplitEntry, 0x18)

typedef struct HlPACxV402Header
{
    /** @brief "PACx" */
    HlU32 signature;
    /** @brief Version Number. */
    HlU8 version[3];
    /** @brief 'B' for Big Endian, 'L' for Little Endian. */
    HlU8 endianFlag;
    /** @brief A random number that serves as a unique identifier for this pac file. */
    HlU32 uid;
    HlU32 fileSize;
    HL_OFF32(void) rootOffset;
    /**
       @brief The size, in bytes, of the compressed root
       PACxV3 pac data within this PACxV4 file.
       
       If this is the same as rootUncompresedSize, it means
       that the root PACxV3 pac data is uncompressed.
    */
    HlU32 rootCompressedSize;
    /**
       @brief The size, in bytes, of the uncompressed root
       PACxV3 pac data within this PACxV4 file.

       If this is the same as rootCompresedSize, it means
       that the root PACxV3 pac data is uncompressed.
    */
    HlU32 rootUncompressedSize;
    /** @brief Bitwise-and this with values from the HlPACxV3Type enum. */
    HlU16 flags;
    /** @brief Always 0x208? */
    HlU16 unknown2;
    HlU32 chunkCount;
}
HlPACxV402Header;

HL_STATIC_ASSERT_SIZE(HlPACxV402Header, 0x24)

typedef struct HlPACxV403Header
{
    /** @brief "PACx" */
    HlU32 signature;
    /** @brief Version Number. */
    HlU8 version[3];
    /** @brief 'B' for Big Endian, 'L' for Little Endian. */
    HlU8 endianFlag;
    /** @brief A random number that serves as a unique identifier for this pac file. */
    HlU32 uid;
    HlU32 fileSize;
    HL_OFF32(void) rootOffset;
    /**
       @brief The size, in bytes, of the compressed root
       PACxV3 pac data within this PACxV4 file.
       
       If this is the same as rootUncompresedSize, it means
       that the root PACxV3 pac data is uncompressed.
    */
    HlU32 rootCompressedSize;
    /**
       @brief The size, in bytes, of the uncompressed root
       PACxV3 pac data within this PACxV4 file.

       If this is the same as rootCompresedSize, it means
       that the root PACxV3 pac data is uncompressed.
    */
    HlU32 rootUncompressedSize;
    /** @brief Bitwise-and this with values from the HlPACxV3Type enum. */
    HlU16 flags;
    /** @brief Always 0x208? */
    HlU16 unknown2;
}
HlPACxV403Header;

HL_STATIC_ASSERT_SIZE(HlPACxV403Header, 0x20)

/* This type is a "subset" of all V4 revision headers. */
typedef HlPACxV403Header HlPACxV4Header;

/* ========================================== PACx V2 ========================================== */
HL_API void hlPACxV2NodeSwap(HlPACxV2Node* node, HlBool swapOffsets);
HL_API void hlPACxV2NodeTreeSwap(HlPACxV2NodeTree* nodeTree, HlBool swapOffsets);
HL_API void hlPACxV2DataEntrySwap(HlPACxV2DataEntry* dataEntry);
HL_API void hlPACxV2SplitTableSwap(HlPACxV2SplitTable* splitTable, HlBool swapOffsets);
HL_API void hlPACxV2ProxyEntrySwap(HlPACxV2ProxyEntry* proxyEntry, HlBool swapOffsets);
HL_API void hlPACxV2ProxyEntryTableSwap(HlPACxV2ProxyEntryTable* proxyEntryTable,
    HlBool swapOffsets);

HL_API void hlPACxV2DataHeaderSwap(HlPACxV2BlockDataHeader* dataHeader);

HL_API void hlPACxV2BlockFix(HlPACxV2BlockHeader* HL_RESTRICT block,
    HlU8 endianFlag, HlPACxV2Header* HL_RESTRICT header);

HL_API void hlPACxV2BlocksFix(HlPACxV2BlockHeader* HL_RESTRICT firstBlock,
    HlU16 blockCount, HlU8 endianFlag, HlPACxV2Header* HL_RESTRICT header);

HL_API void hlPACxV2Fix(void* rawData);

HL_API const HlPACxV2BlockDataHeader* hlPACxV2GetDataBlock(const void* rawData);

#define hlPACxV2DataGetTypeTree(dataBlock) (const HlPACxV2NodeTree*)((dataBlock) + 1)

HL_API const HlPACxV2NodeTree* hlPACxV2DataGetFileTree(
    const HlPACxV2BlockDataHeader* HL_RESTRICT dataBlock,
    const char* HL_RESTRICT resType);

HL_API const HlPACxV2DataEntry* hlPACxV2DataGetDataEntries(
    const HlPACxV2BlockDataHeader* dataBlock);

HL_API const HlPACxV2ProxyEntryTable* hlPACxV2DataGetProxyEntryTable(
    const HlPACxV2BlockDataHeader* dataBlock);

HL_API const char* hlPACxV2DataGetStringTable(const HlPACxV2BlockDataHeader* dataBlock);
HL_API const HlU8* hlPACxV2DataGetOffsetTable(const HlPACxV2BlockDataHeader* dataBlock);

HL_API HlResult hlPACxV2ParseInto(const HlPACxV2Header* HL_RESTRICT pac,
    HlBool skipProxies, HlArchive* HL_RESTRICT hlArc);

HL_API HlResult hlPACxV2ReadInto(void* HL_RESTRICT rawData,
    HlBool skipProxies, HlArchive* HL_RESTRICT hlArc);

HL_API HlResult hlPACxV2LoadSingleInto(const HlNChar* HL_RESTRICT filePath,
    HlBool skipProxies, HlBlobList* HL_RESTRICT pacs, HlArchive* HL_RESTRICT hlArc);

HL_API HlResult hlPACxV2LoadAllInto(const HlNChar* HL_RESTRICT filePath,
    HlBlobList* HL_RESTRICT pacs, HlArchive* HL_RESTRICT hlArc);

HL_API HlResult hlPACxV2Load(const HlNChar* HL_RESTRICT filePath,
    HlBool loadSplits, HlBlobList* HL_RESTRICT pacs,
    HlArchive* HL_RESTRICT * HL_RESTRICT hlArc);

HL_API HlResult hlPACxV2StartWrite(HlBINAEndianFlag endianFlag, HlStream* stream);

HL_API HlResult hlPACxV2FinishWrite(size_t headerPos, HlU16 blockCount,
    HlBINAEndianFlag endianFlag, HlStream* stream);

HL_API HlResult hlPACxV2DataBlockStartWrite(HlStream* stream);

HL_API HlResult hlPACxV2DataBlockFinishWrite(size_t headerPos, size_t dataBlockPos,
    HlU32 treesSize, HlU32 dataEntriesSize, HlU32 proxyTableSize,
    HlBINAEndianFlag endianFlag, const HlStrTable* HL_RESTRICT strTable,
    HlOffTable* HL_RESTRICT offTable, HlStream* HL_RESTRICT stream);

HL_API HlResult hlPACxV2SaveEx(const HlArchive* HL_RESTRICT arc,
    HlU32 splitLimit, HlU32 dataAlignment, HlBINAEndianFlag endianFlag,
    const HlPACxSupportedExt* HL_RESTRICT exts, const size_t extCount,
    HlPackedFileIndex* HL_RESTRICT pfi, const HlNChar* HL_RESTRICT filePath);

#define hlPACxV2SaveLW(arc, splitLimit, endianFlag, filePath)\
    hlPACxV2SaveEx(arc, splitLimit, HL_PACX_DEFAULT_ALIGNMENT, endianFlag,\
        HlPACxLWExts, HlPACxLWExtCount, NULL, filePath)

#define hlPACxV2SaveRio(arc, splitLimit, endianFlag, filePath)\
    hlPACxV2SaveEx(arc, splitLimit, HL_PACX_DEFAULT_ALIGNMENT, endianFlag,\
        HlPACxRioExts, HlPACxRioExtCount, NULL, filePath)

/* ========================================== PACx V3 ========================================== */
HL_API void hlPACxV3HeaderSwap(HlPACxV3Header* header, HlBool swapOffsets);

HL_API void hlPACxV3Fix(void* rawData);

#define hlPACxV3GetTypeTree(rawData) (const HlPACxV3NodeTree*)(\
    (const HlPACxV3Header*)(rawData) + 1)

HL_API const HlPACxV3Node* hlPACxV3GetChildNode(const HlPACxV3Node* node,
    const HlPACxV3Node* nodes, const char* HL_RESTRICT name);

HL_API const HlPACxV3Node* hlPACxV3GetNode(
    const HlPACxV3NodeTree* HL_RESTRICT nodeTree,
    const char* HL_RESTRICT name);

HL_API const HlPACxV3SplitTable* hlPACxV3GetSplitTable(const void* rawData);
HL_API const HlPACxV3DataEntry* hlPACxV3GetDataEntries(const void* rawData);
HL_API const char* hlPACxV3GetStringTable(const void* rawData);
HL_API const void* hlPACxV3GetFileData(const void* rawData);
HL_API const HlU8* hlPACxV3GetOffsetTable(const void* rawData);

HL_API HlResult hlPACxV3ParseInto(const HlPACxV3Header* HL_RESTRICT pac,
    HlBool skipProxies, HlArchive* HL_RESTRICT hlArc);

HL_API HlResult hlPACxV3ReadInto(void* HL_RESTRICT rawData,
    HlBool skipProxies, HlArchive* HL_RESTRICT hlArc);

HL_API HlResult hlPACxV3LoadSingleInto(const HlNChar* HL_RESTRICT filePath,
    HlBool skipProxies, HlBlobList* HL_RESTRICT pacs,
    HlArchive* HL_RESTRICT hlArc);

HL_API HlResult hlPACxV3LoadAllInto(const HlNChar* HL_RESTRICT filePath,
    HlBlobList* HL_RESTRICT pacs, HlArchive* HL_RESTRICT hlArc);

HL_API HlResult hlPACxV3Load(const HlNChar* HL_RESTRICT filePath,
    HlBool loadSplits, HlBlobList* HL_RESTRICT pacs,
    HlArchive* HL_RESTRICT * HL_RESTRICT hlArc);

HL_API HlResult hlPACxV3StartWrite(HlU32 version, HlU32 uid,
    HlU16 type, HlBINAEndianFlag endianFlag, HlStream* stream);

HL_API HlResult hlPACxV3FinishWrite(size_t headerPos, HlU32 treesSize,
    HlU32 splitTableSize, HlU32 dataEntriesSize, HlU32 stringTableSize,
    size_t fileDataPos, HlU32 splitCount, HlBINAEndianFlag endianFlag,
    HlOffTable* HL_RESTRICT offTable, HlStream* HL_RESTRICT stream);

HL_API HlResult hlPACxV3SaveEx(const HlArchive* HL_RESTRICT arc,
    HlU32 splitLimit, HlU32 dataAlignment, HlBINAEndianFlag endianFlag,
    const HlPACxSupportedExt* HL_RESTRICT exts, const size_t extCount,
    HlPackedFileIndex* HL_RESTRICT pfi, const HlNChar* HL_RESTRICT filePath);

#define hlPACxV3SaveForces(arc, splitLimit, endianFlag, filePath)\
    hlPACxV3SaveEx(arc, splitLimit, HL_PACX_DEFAULT_ALIGNMENT, endianFlag,\
        HlPACxForcesExts, HlPACxForcesExtCount, NULL, filePath)

/* ========================================== PACx V4 ========================================== */
HL_API void hlPACxV4Fix(void* rawData);

#define hlPACxV402GetRootChunks(rawData) (const HlPACxV402Chunk*)\
    (((const HlPACxV402Header*)(rawData)) + 1)

HL_API HlResult hlPACxV402DecompressNoAlloc(const void* HL_RESTRICT compressedData,
    const HlPACxV402Chunk* HL_RESTRICT chunks, HlU32 chunkCount,
    HlU32 compressedSize, HlU32 uncompressedSize,
    void* HL_RESTRICT uncompressedData);

HL_API HlResult hlPACxV402Decompress(const void* HL_RESTRICT compressedData,
    const HlPACxV402Chunk* HL_RESTRICT chunks, HlU32 chunkCount,
    HlU32 compressedSize, HlU32 uncompressedSize,
    void* HL_RESTRICT * HL_RESTRICT uncompressedData);

HL_API HlResult hlPACxV402DecompressBlob(const void* HL_RESTRICT compressedData,
    const HlPACxV402Chunk* HL_RESTRICT chunks, HlU32 chunkCount,
    HlU32 compressedSize, HlU32 uncompressedSize,
    HlBlob* HL_RESTRICT * HL_RESTRICT uncompressedBlob);

HL_API HlResult hlPACxV403DecompressNoAlloc(const void* HL_RESTRICT compressedData,
    HlU32 compressedSize, HlU32 uncompressedSize,
    void* HL_RESTRICT uncompressedData);

HL_API HlResult hlPACxV403Decompress(const void* HL_RESTRICT compressedData,
    HlU32 compressedSize, HlU32 uncompressedSize,
    void* HL_RESTRICT * HL_RESTRICT uncompressedData);

HL_API HlResult hlPACxV403DecompressBlob(const void* HL_RESTRICT compressedData,
    HlU32 compressedSize, HlU32 uncompressedSize,
    HlBlob* HL_RESTRICT * HL_RESTRICT uncompressedBlob);

HL_API size_t hlPACxV402CompressBound(size_t uncompressedSize);

HL_API HlResult hlPACxV402CompressNoAlloc(const void* HL_RESTRICT uncompressedData,
    size_t uncompressedSize, size_t compressedBufSize, size_t maxChunkSize,
    size_t* HL_RESTRICT compressedSize, void* HL_RESTRICT compressedBuf,
    HlPACxV402ChunkList* HL_RESTRICT chunks);

HL_API HlResult hlPACxV402Compress(const void* HL_RESTRICT uncompressedData,
    size_t uncompressedSize, size_t maxChunkSize, size_t* HL_RESTRICT compressedSize,
    void* HL_RESTRICT * HL_RESTRICT compressedData, HlPACxV402ChunkList* HL_RESTRICT chunks);

HL_API HlResult hlPACxV402CompressBlob(const void* HL_RESTRICT uncompressedData,
    size_t uncompressedSize, size_t maxChunkSize,
    HlBlob* HL_RESTRICT * HL_RESTRICT compressedBlob,
    HlPACxV402ChunkList* HL_RESTRICT chunks);

HL_API size_t hlPACxV403CompressBound(size_t uncompressedSize);

HL_API HlResult hlPACxV403CompressNoAlloc(const void* HL_RESTRICT uncompressedData,
    size_t uncompressedSize, size_t compressedBufSize,
    size_t* HL_RESTRICT compressedSize, void* HL_RESTRICT compressedBuf);

HL_API HlResult hlPACxV403Compress(const void* HL_RESTRICT uncompressedData,
    size_t uncompressedSize, size_t* HL_RESTRICT compressedSize,
    void* HL_RESTRICT * HL_RESTRICT compressedData);

HL_API HlResult hlPACxV403CompressBlob(const void* HL_RESTRICT uncompressedData,
    size_t uncompressedSize, HlBlob* HL_RESTRICT * HL_RESTRICT compressedBlob);

HL_API HlResult hlPACxV4ReadInto(void* HL_RESTRICT rawData,
    HlBool parseSplits, HlBlobList* HL_RESTRICT uncompressedPacs,
    HlArchive* HL_RESTRICT hlArc);

HL_API HlResult hlPACxV4LoadInto(const HlNChar* HL_RESTRICT filePath,
    HlBool parseSplits, HlBlobList* HL_RESTRICT uncompressedPacs,
    HlArchive* HL_RESTRICT hlArc);

HL_API HlResult hlPACxV4Load(const HlNChar* HL_RESTRICT filePath,
    HlBool parseSplits, HlBlobList* HL_RESTRICT uncompressedPacs,
    HlArchive* HL_RESTRICT * HL_RESTRICT hlArc);

HL_API HlResult hlPACxLoadInto(const HlNChar* HL_RESTRICT filePath,
    HlU8 majorVersion, HlBool loadSplits, HlBlobList* HL_RESTRICT pacs,
    HlArchive* HL_RESTRICT hlArc);

HL_API HlResult hlPACxLoad(const HlNChar* HL_RESTRICT filePath,
    HlU8 majorVersion, HlBool loadSplits, HlBlobList* HL_RESTRICT pacs,
    HlArchive* HL_RESTRICT * HL_RESTRICT hlArc);

HL_API HlResult hlPACxV402Write(const HlArchive* HL_RESTRICT arc,
    HlU32 splitLimit, HlU32 dataAlignment, HlU32 maxChunkSize,
    HlBool noCompress, HlBINAEndianFlag endianFlag,
    const HlPACxSupportedExt* HL_RESTRICT exts, const size_t extCount,
    const HlNChar* HL_RESTRICT fileName, HlStream* HL_RESTRICT stream);

HL_API HlResult hlPACxV403Write(const HlArchive* HL_RESTRICT arc,
    HlU32 splitLimit, HlU32 dataAlignment, HlBool noCompress,
    HlBINAEndianFlag endianFlag, const HlPACxSupportedExt* HL_RESTRICT exts,
    const size_t extCount, const HlNChar* HL_RESTRICT fileName,
    HlStream* HL_RESTRICT stream);

HL_API HlResult hlPACxV402SaveEx(const HlArchive* HL_RESTRICT arc,
    HlU32 splitLimit, HlU32 dataAlignment, HlU32 maxChunkSize,
    HlBool noCompress, HlBINAEndianFlag endianFlag,
    const HlPACxSupportedExt* HL_RESTRICT exts, const size_t extCount,
    const HlNChar* HL_RESTRICT filePath);

HL_API HlResult hlPACxV403SaveEx(const HlArchive* HL_RESTRICT arc,
    HlU32 splitLimit, HlU32 dataAlignment, HlBool noCompress,
    HlBINAEndianFlag endianFlag, const HlPACxSupportedExt* HL_RESTRICT exts,
    const size_t extCount, const HlNChar* HL_RESTRICT filePath);

#define hlPACxV4SaveTokyo1(arc, splitLimit, noCompress, endianFlag, filePath)\
    hlPACxV402SaveEx(arc, splitLimit, HL_PACX_DEFAULT_ALIGNMENT,\
        HL_PACXV402_DEFAULT_CHUNK_SIZE, noCompress, endianFlag,\
        HlPACxTokyo1Exts, HlPACxTokyo1ExtCount, filePath)

#define hlPACxV4SaveTokyo2(arc, splitLimit, noCompress, endianFlag, filePath)\
    hlPACxV402SaveEx(arc, splitLimit, HL_PACX_DEFAULT_ALIGNMENT,\
        HL_PACXV402_DEFAULT_CHUNK_SIZE, noCompress, endianFlag,\
        HlPACxTokyo2Exts, HlPACxTokyo2ExtCount, filePath)

#define hlPACxV4SaveSakura(arc, splitLimit, noCompress, endianFlag, filePath)\
    hlPACxV402SaveEx(arc, splitLimit, HL_PACX_DEFAULT_ALIGNMENT,\
        HL_PACXV402_DEFAULT_CHUNK_SIZE, noCompress, endianFlag,\
        HlPACxSakuraExts, HlPACxSakuraExtCount, filePath)

#define hlPACxV4SavePPT2(arc, splitLimit, noCompress, endianFlag, filePath)\
    hlPACxV403SaveEx(arc, splitLimit, HL_PACX_DEFAULT_ALIGNMENT, noCompress,\
        endianFlag, HlPACxPPT2Exts, HlPACxPPT2ExtCount, filePath)

#ifndef HL_NO_EXTERNAL_WRAPPERS
HL_API const HlPACxV2NodeTree* hlPACxV2DataGetTypeTreeExt(
    const HlPACxV2BlockDataHeader* dataBlock);

HL_API const HlPACxV3NodeTree* hlPACxV3GetTypeTreeExt(const void* rawData);
HL_API const HlPACxV402Chunk* hlPACxV402GetRootChunksExt(const void* rawData);
#endif

#ifdef __cplusplus
}
#endif
#endif
