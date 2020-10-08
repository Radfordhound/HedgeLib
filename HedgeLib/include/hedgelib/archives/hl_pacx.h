#ifndef HL_PACX_H_INCLUDED
#define HL_PACX_H_INCLUDED
#include "hl_archive.h"
#include "../io/hl_bina.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HL_PACX_SIG     HL_MAKE_SIG('P', 'A', 'C', 'x')

HL_API extern const HlNChar HL_PACX_EXT[5];

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

HL_STATIC_ASSERT_SIZE(HlPACxV2BlockDataHeader, 0x20);

typedef struct HlPACxV2Node
{
    HL_OFF32_STR name;
    HL_OFF32(void) data;
}
HlPACxV2Node;

HL_STATIC_ASSERT_SIZE(HlPACxV2Node, 8);

typedef struct HlPACxV2NodeTree
{
    HlU32 nodeCount;
    HL_OFF32(HlPACxV2Node) nodes;
}
HlPACxV2NodeTree;

HL_STATIC_ASSERT_SIZE(HlPACxV2NodeTree, 8);

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

HL_STATIC_ASSERT_SIZE(HlPACxV2DataEntry, 16);

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

HL_STATIC_ASSERT_SIZE(HlPACxV2SplitTable, 8);

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

HL_STATIC_ASSERT_SIZE(HlPACxV2ProxyEntry, 12);

typedef struct HlPACxV2ProxyEntryTable
{
    HlU32 proxyEntryCount;
    HL_OFF32(HlPACxV2ProxyEntry) proxyEntries;
}
HlPACxV2ProxyEntryTable;

HL_STATIC_ASSERT_SIZE(HlPACxV2ProxyEntryTable, 8);

/* Thanks to Skyth for cracking the majority of the PACxV3 format! */
typedef struct HlPACxV3SplitEntry
{
    HL_OFF64_STR name;
}
HlPACxV3SplitEntry;

HL_STATIC_ASSERT_SIZE(HlPACxV3SplitEntry, 8);

typedef struct HlPACxV3SplitTable
{
    HlU64 splitCount;
    HL_OFF64(void) splitEntries;
}
HlPACxV3SplitTable;

HL_STATIC_ASSERT_SIZE(HlPACxV3SplitTable, 16);

typedef enum PACxV3DataType
{
    HL_PACXV3_DATA_TYPE_REGULAR_FILE = 0,
    HL_PACXV3_DATA_TYPE_NOT_HERE = 1,
    HL_PACXV3_DATA_TYPE_BINA_FILE = 2
}
PACxV3DataType;

typedef struct HlPACxV3DataEntry
{
    /** @brief Date Modified or Hash?? */
    HlU32 unknown1;
    HlU32 dataSize;
    /** @brief Always 0? Unknown1 from PACxV2DataEntry?? */
    HlU64 unknown2;
    HL_OFF64(void) data;
    /** @brief Always 0? Unknown2 from PACxV2DataEntry?? */
    HlU64 unknown3;
    HL_OFF64_STR extension;
    /** @brief Probably actually just a single byte with 7 bytes of padding?? */
    HlU64 dataType;
}
HlPACxV3DataEntry;

HL_STATIC_ASSERT_SIZE(HlPACxV3DataEntry, 0x30);

typedef struct HlPACxV3Node
{
    HL_OFF64_STR name;
    HL_OFF64(void) data;
    HL_OFF64(HlS32) childIndices;
    HlS32 parentIndex;
    HlS32 globalIndex;
    HlS32 dataIndex;
    HlU16 childCount;
    HlU8 hasData;
    /** @brief Where this node's name should be copied to within the global name buffer. */
    HlU8 bufStartIndex;
}
HlPACxV3Node;

HL_STATIC_ASSERT_SIZE(HlPACxV3Node, 0x28);

typedef struct HlPACxV3NodeTree
{
    HlU32 nodeCount;
    HlU32 dataNodeCount;
    HL_OFF64(HlPACxV3Node) nodes;
    HL_OFF64(HlS32) dataNodeIndices;
}
HlPACxV3NodeTree;

HL_STATIC_ASSERT_SIZE(HlPACxV3NodeTree, 0x18);

typedef enum HlPACxV3Type
{
    HL_PACXV3_TYPE_IS_ROOT = 1,
    HL_PACXV3_TYPE_IS_SPLIT = 2,
    HL_PACXV3_TYPE_HAS_SPLITS = 4
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
    /** @brief Date Modified or Hash?? */
    HlU32 unknown1;
    HlU32 fileSize;
    HlU32 nodesSize;
    HlU32 splitTableSize;
    HlU32 dataEntriesSize;
    /** @brief The size of the string table in bytes, including padding. */
    HlU32 stringTableSize;
    HlU32 dataSize;
    /** @brief The size of the offset table in bytes, including padding. */
    HlU32 offsetTableSize;
    /** @brief Bitwise-and this with values from the PACxV3Type enum. */
    HlU16 type;
    /** @brief Always 0x108? */
    HlU16 unknown2;
    HlU32 splitCount;
}
HlPACxV3Header;

HL_STATIC_ASSERT_SIZE(HlPACxV3Header, 0x30);

/**
   @brief Information explaining how to decompress a blob of LZ4-compressed data.

   When decompressing the root pac allocate a buffer of
   size header.rootUncompressedSize, then loop through these
   chunks and decompress each one, one-by-one, into that buffer.

   If you try to decompress all at once instead the data can be corrupted.
*/
typedef struct HlPACxV4Chunk
{
    HlU32 compressedSize;
    HlU32 uncompressedSize;
}
HlPACxV4Chunk;

HL_STATIC_ASSERT_SIZE(HlPACxV4Chunk, 8);

typedef struct HlPACxV4SplitEntry
{
    HL_OFF64_STR name;
    HlU32 compressedSize;
    HlU32 uncompressedSize;
    HlU32 offset;
    HlU32 chunkCount;
    HL_OFF64(HlPACxV4Chunk) chunksOffset;
}
HlPACxV4SplitEntry;

HL_STATIC_ASSERT_SIZE(HlPACxV4SplitEntry, 0x20);

typedef struct HlPACxV4Header
{
    /** @brief "PACx" */
    HlU32 signature;
    /** @brief Version Number. */
    HlU8 version[3];
    /** @brief 'B' for Big Endian, 'L' for Little Endian. */
    HlU8 endianFlag;
    /** @brief Date Modified or Hash?? */
    HlU32 unknown1;
    HlU32 fileSize;
    HL_OFF32(void) rootOffset;
    HlU32 rootCompressedSize;
    HlU32 rootUncompressedSize;
    /** @brief Bitwise-and this with values from the PACxV3Type enum. */
    HlU16 type;
    /** @brief Always 0x208? */
    HlU16 unknown2;
    HlU32 chunkCount;
}
HlPACxV4Header;

HL_STATIC_ASSERT_SIZE(HlPACxV4Header, 0x24);

HL_API void hlPACxV2NodeSwap(HlPACxV2Node* node, HlBool swapOffsets);
HL_API void hlPACxV2NodeTreeSwap(HlPACxV2NodeTree* nodeTree, HlBool swapOffsets);
HL_API void hlPACxV2DataEntrySwap(HlPACxV2DataEntry* dataEntry);
HL_API void hlPACxV2SplitTableSwap(HlPACxV2SplitTable* splitTable, HlBool swapOffsets);
HL_API void hlPACxV2ProxyEntrySwap(HlPACxV2ProxyEntry* proxyEntry, HlBool swapOffsets);
HL_API void hlPACxV2ProxyEntryTableSwap(HlPACxV2ProxyEntryTable* proxyEntryTable,
    HlBool swapOffsets);

HL_API void hlPACxV2DataHeaderSwap(HlPACxV2BlockDataHeader* dataHeader);

HL_API void hlPACxV2BlockHeaderFix(HlPACxV2BlockHeader* HL_RESTRICT blockHeader,
    HlU8 endianFlag, HlPACxV2Header* HL_RESTRICT header);

HL_API void hlPACxV2BlocksFix(HlPACxV2BlockHeader* HL_RESTRICT curBlock,
    HlU16 blockCount, HlU8 endianFlag, HlPACxV2Header* HL_RESTRICT header);

HL_API void hlPACxV2Fix(HlBlob* blob);

#define hlPACxV2DataGetTypeTree(dataBlock) (HlPACxV2NodeTree*)((dataBlock) + 1)

HL_API HlPACxV2NodeTree* hlPACxV2DataGetFileTree(
    HlPACxV2BlockDataHeader* HL_RESTRICT dataBlock,
    const char* HL_RESTRICT resType);

#define hlPACxV2DataGetDataEntries(dataBlock) (HlPACxV2DataEntry*)(\
    HL_ADD_OFF(hlPACxV2DataGetTypeTree(dataBlock), (dataBlock)->treesSize))

#define hlPACxV2DataGetProxyEntryTable(dataBlock) (HlPACxV2ProxyEntryTable*)(\
    HL_ADD_OFF(hlPACxV2DataGetDataEntries(dataBlock), (dataBlock)->dataEntriesSize))

#define hlPACxV2DataGetStringTable(dataBlock) (char*)(\
    HL_ADD_OFF(hlPACxV2DataGetProxyEntryTable(dataBlock), (dataBlock)->proxyTableSize))

#define hlPACxV2DataGetOffsetTable(dataBlock) HL_ADD_OFFC(\
    hlPACxV2DataGetStringTable(dataBlock), (dataBlock)->stringTableSize)

HL_API HlResult hlPACxV2Read(const HlBlob* const HL_RESTRICT * HL_RESTRICT pacs,
    size_t pacCount, HlArchive* HL_RESTRICT * HL_RESTRICT archive);

HL_API HlResult hlPACxV2Load(const HlNChar* HL_RESTRICT filePath,
    HlBool loadSplits, HlArchive* HL_RESTRICT * HL_RESTRICT archive);

HL_API void hlPACxV3Fix(HlBlob* blob);

#define hlPACxV3GetTypeTree(header) (HlPACxV3NodeTree*)((header) + 1)
#define hlPACxV3GetSplitTable(header) (HlPACxV3SplitTable*)(\
    HL_ADD_OFF(hlPACxV3GetTypeTree(header), (header)->nodesSize))

#define hlPACxV3GetDataEntries(header) (HlPACxV3DataEntry*)(\
    HL_ADD_OFF(hlPACxV3GetSplitTable(header), (header)->splitTableSize))

#define hlPACxV3GetStringTable(header) (char*)(\
    HL_ADD_OFF(hlPACxV3GetDataEntries(header), (header)->dataEntriesSize))

#define hlPACxV3GetData(header) (void*)(\
    HL_ADD_OFF(hlPACxV3GetStringTable(header), (header)->stringTableSize))

#define hlPACxV3GetOffsetTable(header) HL_ADD_OFFC(\
    hlPACxV3GetData(header), (header)->dataSize)

HL_API const HlPACxV3Node* hlPACxV3GetChildNode(const HlPACxV3Node* node,
    const HlPACxV3Node* nodes, const char* HL_RESTRICT name);

HL_API const HlPACxV3Node* hlPACxV3GetNode(
    const HlPACxV3NodeTree* HL_RESTRICT nodeTree,
    const char* HL_RESTRICT name);

HL_API HlResult hlPACxV3Read(const HlBlob* const HL_RESTRICT * HL_RESTRICT pacs,
    size_t pacCount, HlArchive* HL_RESTRICT * HL_RESTRICT archive);

HL_API HlResult hlPACxV3Load(const HlNChar* HL_RESTRICT filePath,
    HlBool loadSplits, HlArchive* HL_RESTRICT * HL_RESTRICT archive);

HL_API void hlPACxV4Fix(HlBlob* blob);

#define hlPACxV4GetRootChunks(header) (HlPACxV4Chunk*)((header) + 1)

HL_API HlResult hlPACxV4DecompressNoAlloc(const void* HL_RESTRICT compressedData,
    const HlPACxV4Chunk* HL_RESTRICT chunks, HlU32 chunkCount,
    HlU32 uncompressedSize, void* HL_RESTRICT uncompressedData);

HL_API HlResult hlPACxV4Decompress(const void* HL_RESTRICT compressedData,
    const HlPACxV4Chunk* HL_RESTRICT chunks, HlU32 chunkCount,
    HlU32 uncompressedSize, void* HL_RESTRICT * HL_RESTRICT uncompressedData);

HL_API HlResult hlPACxV4Read(HlBlob* HL_RESTRICT pac,
    HlBool loadSplits, HlArchive* HL_RESTRICT * HL_RESTRICT archive);

HL_API HlResult hlPACxV4Load(const HlNChar* HL_RESTRICT filePath,
    HlBool loadSplits, HlArchive* HL_RESTRICT * HL_RESTRICT archive);

HL_API HlResult hlPACxLoadBlobs(const HlNChar* HL_RESTRICT filePath,
    HlBlob** HL_RESTRICT * HL_RESTRICT pacs, size_t* HL_RESTRICT pacCount);

HL_API HlResult hlPACxLoad(const HlNChar* HL_RESTRICT filePath,
    HlBool loadSplits, HlArchive* HL_RESTRICT * HL_RESTRICT archive);

#ifndef HL_NO_EXTERNAL_WRAPPERS
HL_API HlPACxV2NodeTree* hlPACxV2DataGetTypeTreeExt(HlPACxV2BlockDataHeader* dataBlock);
HL_API HlPACxV2DataEntry* hlPACxV2DataGetDataEntriesExt(HlPACxV2BlockDataHeader* dataBlock);
HL_API HlPACxV2ProxyEntryTable* hlPACxV2DataGetProxyEntryTableExt(
    HlPACxV2BlockDataHeader* dataBlock);

HL_API char* hlPACxV2DataGetStringTableExt(HlPACxV2BlockDataHeader* dataBlock);
HL_API const void* hlPACxV2DataGetOffsetTableExt(const HlPACxV2BlockDataHeader* dataBlock);

HL_API HlPACxV3NodeTree* hlPACxV3GetTypeTreeExt(HlPACxV3Header* header);
HL_API HlPACxV3SplitTable* hlPACxV3GetSplitTableExt(HlPACxV3Header* header);
HL_API HlPACxV3DataEntry* hlPACxV3GetDataEntriesExt(HlPACxV3Header* header);
HL_API char* hlPACxV3GetStringTableExt(HlPACxV3Header* header);
HL_API void* hlPACxV3GetDataExt(HlPACxV3Header* header);
HL_API const void* hlPACxV3GetOffsetTableExt(HlPACxV3Header* header);
HL_API HlPACxV4Chunk* hlPACxV4GetRootChunksExt(HlPACxV4Header* header);
#endif

#ifdef __cplusplus
}
#endif
#endif
