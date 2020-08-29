#ifndef HL_PACX_H_INCLUDED
#define HL_PACX_H_INCLUDED
#include "hl_archive.h"
#include "../io/hl_bina.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HL_PACX_SIG     HL_MAKE_SIG('P', 'A', 'C', 'x')

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
    HL_ADD_OFF(hlPACxV2DataGetTypeTree(dataBlock), dataBlock->treesSize))

#define hlPACxV2DataGetProxyEntryTable(dataBlock) (HlPACxV2ProxyEntryTable*)(\
    HL_ADD_OFF(hlPACxV2DataGetDataEntries(dataBlock), dataBlock->dataEntriesSize))

#define hlPACxV2DataGetStringTable(dataBlock) (char*)(\
    HL_ADD_OFF(hlPACxV2DataGetProxyEntryTable(dataBlock), dataBlock->proxyTableSize))

#define hlPACxV2DataGetOffsetTable(dataBlock) HL_ADD_OFFC(\
    hlPACxV2DataGetStringTable(dataBlock), dataBlock->stringTableSize)

HL_API HlResult hlPACxV2Read(const HlBlob** HL_RESTRICT pacs,
    size_t pacCount, HlArchive** HL_RESTRICT archive);

HL_API HlResult hlPACxV2Load(const HlNChar* HL_RESTRICT filePath,
    HlBool loadSplits, HlArchive** HL_RESTRICT archive);

#ifndef HL_NO_EXTERNAL_WRAPPERS
HL_API HlPACxV2NodeTree* hlPACxV2DataGetTypeTreeExt(HlPACxV2BlockDataHeader* dataBlock);
HL_API HlPACxV2DataEntry* hlPACxV2DataGetDataEntriesExt(HlPACxV2BlockDataHeader* dataBlock);
HL_API HlPACxV2ProxyEntryTable* hlPACxV2DataGetProxyEntryTableExt(
    HlPACxV2BlockDataHeader* dataBlock);

HL_API char* hlPACxV2DataGetStringTableExt(HlPACxV2BlockDataHeader* dataBlock);
HL_API const void* hlPACxV2DataGetOffsetTableExt(const HlPACxV2BlockDataHeader* dataBlock);
#endif

#ifdef __cplusplus
}
#endif
#endif
