#include "hl_in_archive.h"
#include "../hl_in_assert.h"
#include "../io/hl_in_path.h"
#include "../depends/lz4/lz4.h"
#include "hedgelib/effects/hl_grif.h"
#include "hedgelib/archives/hl_pacx.h"
#include "hedgelib/io/hl_path.h"
#include "hedgelib/io/hl_file.h"
#include "hedgelib/hl_endian.h"
#include "hedgelib/hl_compression.h"
#include "hedgelib/hl_math.h"
#include "hedgelib/hl_text.h"
#include <stdlib.h>

const HlNChar HL_PACX_EXT[5] = HL_NTEXT(".pac");

/* Auto-generate PACx data type enum. */
typedef enum HlINPACxDataType
{
#define HL_IN_PACX_TYPE_AUTOGEN(pacxType) pacxType,
#include "hl_in_pacx_type_autogen.h"

    HL_IN_PACX_DATA_TYPE_COUNT
}
HlINPACxDataType;

/* Auto-generate PACx data types array. */
const char* const HlPACxDataTypes[] =
{
#define HL_IN_PACX_TYPE_AUTOGEN(pacxType) #pacxType,
#include "hl_in_pacx_type_autogen.h"

    NULL
};

const size_t HlPACxDataTypeCount = HL_IN_PACX_DATA_TYPE_COUNT;

/* Auto-generate LW supported extensions array. */
const HlPACxSupportedExt HlLWSupportedExts[] =
{
#define HL_IN_PACX_LW_AUTOGEN(ext, pacxType, type, rootSortWeight, splitSortWeight)\
    { HL_NTEXT(ext), pacxType, HL_PACX_EXT_FLAGS_##type##_TYPE, rootSortWeight, splitSortWeight },

#include "hl_in_pacx_type_autogen.h"

    { 0 }
};

const size_t HlLWSupportedExtCount = ((sizeof(HlLWSupportedExts) /
    sizeof(const HlPACxSupportedExt)) - 1);

const HlPACxSupportedExt HlRioSupportedExts[] =
{
#define HL_IN_PACX_RIO_AUTOGEN(ext, pacxType, type, rootSortWeight, splitSortWeight)\
    { HL_NTEXT(ext), pacxType, HL_PACX_EXT_FLAGS_##type##_TYPE, rootSortWeight, splitSortWeight },

#include "hl_in_pacx_type_autogen.h"

    { 0 }
};

const size_t HlRioSupportedExtCount = ((sizeof(HlRioSupportedExts) /
    sizeof(const HlPACxSupportedExt)) - 1);

const HlPACxSupportedExt HlForcesSupportedExts[] =
{
#define HL_IN_PACX_WARS_AUTOGEN(ext, pacxType, type)\
    { HL_NTEXT(ext), pacxType, HL_PACX_EXT_FLAGS_##type##_TYPE },

#include "hl_in_pacx_type_autogen.h"

    { 0 }
};

const size_t HlForcesSupportedExtCount = ((sizeof(HlForcesSupportedExts) /
    sizeof(const HlPACxSupportedExt)) - 1);

static const HlPACxSupportedExt* hlINPACxGetSupportedExt(const HlNChar* HL_RESTRICT ext,
    const HlPACxSupportedExt* HL_RESTRICT exts, size_t extCount)
{
    size_t i;

    /* Try to find a matching extension in the array. */
    for (i = 0; i < (extCount - 1); ++i)
    {
        /* TODO: Do a case-insensitive check instead of this. */
        if (hlNStrsEqual(ext, exts[i].ext))
        {
            return &exts[i];
        }
    }

    /* Fallback to using the last extension in the array (should be ResRawData). */
    return &exts[i];
}

static HlResult hlINPACxGetRootPathStackBuf(const HlNChar* HL_RESTRICT filePath,
    size_t* HL_RESTRICT pathBufCap, HlNChar* HL_RESTRICT * HL_RESTRICT pathBufPtr)
{
    const HlNChar* ext = hlPathGetExt(filePath);
    size_t splitExtLen, rootPathLen;

    /* Get the length of the file path. */
    rootPathLen = hlNStrLen(filePath);

    /* Create a copy of filePath big enough to hold the root's name and future splits. */
    if ((rootPathLen + 1) > *pathBufCap)
    {
        *pathBufCap = (rootPathLen + 1);
        *pathBufPtr = HL_ALLOC_ARR(HlNChar, *pathBufCap);
        if (!(*pathBufPtr)) return HL_ERROR_OUT_OF_MEMORY;
    }

    /* Determine whether this filePath refers to a split. */
    if ((splitExtLen = hlArchiveExtIsSplit(ext)))
    {
        /* This is a split; remove dot and split extension from root path length. */
        rootPathLen -= (splitExtLen + 1);
    }

    /* Copy the root path into the buffer. */
    memcpy(*pathBufPtr, filePath, rootPathLen * sizeof(HlNChar));

    /* Set null terminator. */
    (*pathBufPtr)[rootPathLen] = HL_NTEXT('\0');
    return HL_RESULT_SUCCESS;
}

void hlPACxV2NodeSwap(HlPACxV2Node* node, HlBool swapOffsets)
{
    /*
       There are no offsets in PACxV2 nodes - we only provide the
       swapOffsets boolean for consistency with other Swap functions.
    */
    if (!swapOffsets) return;

    hlSwapU32P(&node->name);
    hlSwapU32P(&node->data);
}

void hlPACxV2NodeTreeSwap(HlPACxV2NodeTree* nodeTree, HlBool swapOffsets)
{
    hlSwapU32P(&nodeTree->nodeCount);
    if (swapOffsets) hlSwapU32P(&nodeTree->nodes);
}

void hlPACxV2DataEntrySwap(HlPACxV2DataEntry* dataEntry)
{
    hlSwapU32P(&dataEntry->dataSize);
    hlSwapU32P(&dataEntry->unknown1);
    hlSwapU32P(&dataEntry->unknown2);
}

void hlPACxV2SplitTableSwap(HlPACxV2SplitTable* splitTable, HlBool swapOffsets)
{
    if (swapOffsets) hlSwapU32P(&splitTable->splitNames);
    hlSwapU32P(&splitTable->splitCount);
}

void hlPACxV2ProxyEntrySwap(HlPACxV2ProxyEntry* proxyEntry, HlBool swapOffsets)
{
    if (swapOffsets)
    {
        hlSwapU32P(&proxyEntry->type);
        hlSwapU32P(&proxyEntry->name);
    }

    hlSwapU32P(&proxyEntry->nodeIndex);
}

void hlPACxV2ProxyEntryTableSwap(HlPACxV2ProxyEntryTable* proxyEntryTable,
    HlBool swapOffsets)
{
    hlSwapU32P(&proxyEntryTable->proxyEntryCount);
    if (swapOffsets) hlSwapU32P(&proxyEntryTable->proxyEntries);
}

void hlPACxV2DataHeaderSwap(HlPACxV2BlockDataHeader* dataHeader)
{
    hlSwapU32P(&dataHeader->size);
    hlSwapU32P(&dataHeader->dataEntriesSize);
    hlSwapU32P(&dataHeader->treesSize);
    hlSwapU32P(&dataHeader->proxyTableSize);
    hlSwapU32P(&dataHeader->stringTableSize);
    hlSwapU32P(&dataHeader->offsetTableSize);
}

static void hlINPACxV2DataBlockSwapRecursive(HlPACxV2BlockDataHeader* dataBlock)
{
    /* Swap trees, data entries, and split tables. */
    {
        /* Get type tree and type nodes pointer (safe because we already swapped offsets). */
        HlPACxV2NodeTree* typeTree = (HlPACxV2NodeTree*)hlPACxV2DataGetTypeTree(dataBlock);
        HlPACxV2Node* typeNodes = (HlPACxV2Node*)hlOff32Get(&typeTree->nodes);
        HlU32 i;

        /* Swap type tree. */
        hlPACxV2NodeTreeSwap(typeTree, HL_FALSE);

        /* Swap file trees. */
        for (i = 0; i < typeTree->nodeCount; ++i)
        {
            /* Get pointers. */
            const char* typeStr = (const char*)hlOff32Get(&typeNodes[i].name);
            HlPACxV2NodeTree* fileTree = (HlPACxV2NodeTree*)hlOff32Get(
                &typeNodes[i].data);

            HlPACxV2Node* fileNodes = (HlPACxV2Node*)hlOff32Get(&fileTree->nodes);

            /* Swap file tree. */
            hlPACxV2NodeTreeSwap(fileTree, HL_FALSE);

            /* Get PACx type pointer. */
            typeStr = strchr(typeStr, ':');

            /* Swap data entries and split tables. */
            {
                const HlBool isSplitTable = (typeStr &&
                    strcmp(++typeStr, HlPACxDataTypes[ResPacDepend]) == 0);

                HlU32 i2;

                for (i2 = 0; i2 < fileTree->nodeCount; ++i2)
                {
                    /* Swap data entry. */
                    HlPACxV2DataEntry* dataEntry = (HlPACxV2DataEntry*)
                        hlOff32Get(&fileNodes[i2].data);

                    hlPACxV2DataEntrySwap(dataEntry);

                    /* Swap split table if necessary. */
                    if (isSplitTable)
                    {
                        HlPACxV2SplitTable* splitTable = (HlPACxV2SplitTable*)
                            (dataEntry + 1);

                        hlPACxV2SplitTableSwap(splitTable, HL_FALSE);
                    }
                }
            }
        }
    }

    /* Swap proxy entry table and proxy entries. */
    if (dataBlock->proxyTableSize)
    {
        /* Get pointers. */
        HlPACxV2ProxyEntryTable* proxyEntryTable = (HlPACxV2ProxyEntryTable*)
            hlPACxV2DataGetProxyEntryTable(dataBlock);

        HlPACxV2ProxyEntry* proxyEntries = (HlPACxV2ProxyEntry*)
            hlOff32Get(&proxyEntryTable->proxyEntries);

        HlU32 i;

        /* Swap proxy entry table. */
        hlPACxV2ProxyEntryTableSwap(proxyEntryTable, HL_FALSE);

        /* Swap proxy entries. */
        for (i = 0; i < proxyEntryTable->proxyEntryCount; ++i)
        {
            hlPACxV2ProxyEntrySwap(&proxyEntries[i], HL_FALSE);
        }
    }
}

void hlPACxV2BlockFix(HlPACxV2BlockHeader* HL_RESTRICT block,
    HlU8 endianFlag, HlPACxV2Header* HL_RESTRICT header)
{
    switch (block->signature)
    {
    case HL_BINAV2_BLOCK_TYPE_DATA:
    {
        /* Swap block data header if necessary. */
        HlPACxV2BlockDataHeader* dataHeader = (HlPACxV2BlockDataHeader*)block;
        if (hlBINANeedsSwap(endianFlag))
        {
            hlPACxV2DataHeaderSwap(dataHeader);
        }

        /* Fix offsets. */
        {
            const void* offsets = hlPACxV2DataGetOffsetTable(dataHeader);
            hlBINAOffsetsFix32(offsets, endianFlag,
                dataHeader->offsetTableSize, header);
        }

        /* Swap data block if necessary. */
        if (hlBINANeedsSwap(endianFlag))
        {
            hlINPACxV2DataBlockSwapRecursive(dataHeader);
        }
        break;
    }

    default:
        HL_ASSERT(HL_FALSE);
        break;
    }
}

void hlPACxV2BlocksFix(HlPACxV2BlockHeader* HL_RESTRICT curBlock,
    HlU16 blockCount, HlU8 endianFlag, HlPACxV2Header* HL_RESTRICT header)
{
    HlU16 i;
    for (i = 0; i < blockCount; ++i)
    {
        /* Fix this block. */
        hlPACxV2BlockFix(curBlock, endianFlag, header);

        /* Get the next block. */
        curBlock = (HlPACxV2BlockHeader*)hlBINAV2BlockGetNext(curBlock);
    }
}

void hlPACxV2Fix(void* rawData)
{
    /* Swap header if necessary. */
    HlPACxV2Header* header = (HlPACxV2Header*)rawData;
    if (hlBINANeedsSwap(header->endianFlag))
    {
        hlBINAV2HeaderSwap(header);
    }

    /* Fix blocks. */
    {
        HlPACxV2BlockHeader* blocks = (HlPACxV2BlockHeader*)(header + 1);
        hlPACxV2BlocksFix(blocks, header->blockCount,
            header->endianFlag, header);
    }
}

const HlPACxV2BlockDataHeader* hlPACxV2GetDataBlock(const void* rawData)
{
    const HlPACxV2Header* header = (const HlPACxV2Header*)rawData;
    const HlPACxV2BlockHeader* curBlock = (const HlPACxV2BlockHeader*)(header + 1);
    HlU16 i;

    /* Go through all blocks in the file. */
    for (i = 0; i < header->blockCount; ++i)
    {
        /* If this block is a data block, return it. */
        if (curBlock->signature == HL_BINAV2_BLOCK_TYPE_DATA)
        {
            return (const HlPACxV2BlockDataHeader*)curBlock;
        }

        /* Otherwise, get the next block. */
        curBlock = (HlPACxV2BlockHeader*)hlBINAV2BlockGetNext(curBlock);
    }

    return NULL;
}

const HlPACxV2NodeTree* hlPACxV2DataGetFileTree(
    const HlPACxV2BlockDataHeader* HL_RESTRICT dataBlock,
    const char* HL_RESTRICT resType)
{
    const HlPACxV2NodeTree* typeTree = hlPACxV2DataGetTypeTree(dataBlock);
    const HlPACxV2Node* typeNodes = (const HlPACxV2Node*)hlOff32Get(&typeTree->nodes);
    HlU32 i;

    for (i = 0; i < typeTree->nodeCount; ++i)
    {
        const char* typeStr = (const char*)hlOff32Get(&typeNodes[i].name);
        if (!strcmp(strchr(typeStr, ':') + 1, resType))
        {
            return (const HlPACxV2NodeTree*)
                hlOff32Get(&typeNodes[i].data);
        }
    }

    return NULL;
}

const HlPACxV2DataEntry* hlPACxV2DataGetDataEntries(
    const HlPACxV2BlockDataHeader* dataBlock)
{
    return (const HlPACxV2DataEntry*)HL_ADD_OFFC(           /* dataEntries = */
        hlPACxV2DataGetTypeTree(dataBlock),                 /* (typeTree + */
        dataBlock->treesSize);                              /* treesSize) */
}

const HlPACxV2ProxyEntryTable* hlPACxV2DataGetProxyEntryTable(
    const HlPACxV2BlockDataHeader* dataBlock)
{
    return (const HlPACxV2ProxyEntryTable*)HL_ADD_OFFC(     /* proxyTable = */
        hlPACxV2DataGetDataEntries(dataBlock),              /* (dataEntries + */
        dataBlock->dataEntriesSize);                        /* dataEntriesSize) */
}

const char* hlPACxV2DataGetStringTable(const HlPACxV2BlockDataHeader* dataBlock)
{
    return (const char*)HL_ADD_OFFC(                        /* stringTable = */
        hlPACxV2DataGetProxyEntryTable(dataBlock),          /* (proxyTable + */
        dataBlock->proxyTableSize);                         /* proxyTableSize) */
}

const HlU8* hlPACxV2DataGetOffsetTable(const HlPACxV2BlockDataHeader* dataBlock)
{
    return HL_ADD_OFFC(                                     /* offsetTable = */
        hlPACxV2DataGetStringTable(dataBlock),              /* (stringTable + */
        dataBlock->stringTableSize);                        /* stringTableSize) */
}

typedef struct HlINPACxV2MergedStrEntry
{
    const char* str;
    size_t size;
    HlU32 off;
}
HlINPACxV2MergedStrEntry;

static HlResult hlINPACxV2FileTreeSetupEntries(
    const HlPACxV2NodeTree* HL_RESTRICT fileTree, HlBool skipProxies,
    HlU8 endianFlag, const HlU32* HL_RESTRICT curOffset,
    const char* HL_RESTRICT strings, const HlU8* HL_RESTRICT offsets,
    const HlU8* HL_RESTRICT eof, const char* HL_RESTRICT typeStr,
    size_t extSrcLen, size_t extDstLen, HlArchive* HL_RESTRICT hlArc)
{
    /* Get nodes pointer. */
    const HlPACxV2Node* nodes = (const HlPACxV2Node*)hlOff32Get(&fileTree->nodes);
    HlU32 i;
    HlResult result;

    for (i = 0; i < fileTree->nodeCount; ++i)
    {
        /* Get pointers. */
        const HlPACxV2DataEntry* dataEntry = (const HlPACxV2DataEntry*)
            hlOff32Get(&nodes[i].data);

        const char* fileName = (const char*)hlOff32Get(&nodes[i].name);
        HlArchiveEntry hlArcEntry;

        /* Skip proxies if requested. */
        if (skipProxies && (dataEntry->flags & HL_PACXV2_DATA_FLAGS_NOT_HERE))
            continue;

        /* Setup path buffer. */
        {
            /* Get required length for native path buffer. */
            const size_t fileNameSize = hlStrGetReqLenUTF8ToNative(fileName, 0);
            if (!fileNameSize) return HL_ERROR_INVALID_DATA;

            /* Allocate native path buffer, convert fileName, and store the result. */

            /*
              NOTE: We add one to account for the dot in the extension.
              The null-terminator is already accounted for by hlStrGetReqLenUTF8ToNative.
            */
            hlArcEntry.path = HL_ALLOC_ARR(HlNChar, fileNameSize + extDstLen + 1);
            if (!hlArcEntry.path) return HL_ERROR_OUT_OF_MEMORY;

            if (!hlStrConvUTF8ToNativeNoAlloc(fileName,
                hlArcEntry.path, 0, fileNameSize))
            {
                hlFree(hlArcEntry.path);
                return HL_ERROR_UNKNOWN;
            }

            /* Convert extension to native encoding and copy into buffer. */
            if (extSrcLen)
            {
                hlArcEntry.path[fileNameSize - 1] = HL_NTEXT('.');

                if (!hlStrConvUTF8ToNativeNoAlloc(typeStr,
                    &hlArcEntry.path[fileNameSize], extSrcLen, 0))
                {
                    hlFree(hlArcEntry.path);
                    return HL_ERROR_UNKNOWN;
                }
            }

            /* Set null terminator. */
            hlArcEntry.path[fileNameSize + extDstLen] = HL_NTEXT('\0');
        }

        /* Set entry size and meta. */
        hlArcEntry.size = (size_t)dataEntry->dataSize;
        hlArcEntry.meta = 0;

        /* Set meta and data within file entry. */
        if ((dataEntry->flags & HL_PACXV2_DATA_FLAGS_NOT_HERE) == 0)
        {
            /* Copy data and "unmerge" merged BINA data if necessary. */
            const HlU32* data = (const HlU32*)(dataEntry + 1);
            const HlU32* dataEnd = (const HlU32*)HL_ADD_OFFC(
                data, dataEntry->dataSize);

            HlU8 *dstDataBuf, *curDataPtr;
            HlBool isMergedBINA = HL_FALSE;

            /* Determine if this is a "merged" BINA file. */
            while (HL_TRUE)
            {
                if (curOffset >= data)
                {
                    if (curOffset < dataEnd)
                    {
                        /* This is a merged BINA file. */
                        isMergedBINA = HL_TRUE;

                        /* Increase entry size to account for BINA header, DATA block, and padding. */
                        hlArcEntry.size += (sizeof(HlBINAV2Header) +
                            (sizeof(HlBINAV2BlockDataHeader) * 2));
                    }

                    /* Now we know if this is a merged BINA file or not; we can stop checking. */
                    break;
                }

                /*
                    Get the next offset's address - break early
                    if we've reached the end of the offset table.
                */
                if (offsets >= eof || !hlBINAOffsetsNext(&offsets, &curOffset))
                    break;
            }

            /* "Unmerge" this "merged" BINA file. */
            if (isMergedBINA)
            {
                HL_LIST(HlINPACxV2MergedStrEntry) mergedStrs;
                const HlU32* prevOffset = data;
                size_t strTablePadding, offTablePadding,
                    strTableSize = 0, offTableSize = 0;

                HlU32 curStrTableOff = dataEntry->dataSize;

                /* Initialize merged string entry list. */
                /* TODO: Use stack unless we run out of space, then switch to heap. */
                HL_LIST_INIT(mergedStrs);

                /* Compute table sizes and setup merged string entries. */
                {
                    const HlU32* firstDataOffset = curOffset;
                    const HlU8* firstDataOffsetPos = offsets;

                    while (HL_TRUE)
                    {
                        /* Stop if this offset is not part of the current file. */
                        if (curOffset >= dataEnd) break;

                        /* Account for strings. */
                        {
                            /* Get the value the current offset points to. */
                            const char* curOffsetVal = (const char*)hlOff32Get(curOffset);

                            /* Ensure this is a string. */
                            if (curOffsetVal >= strings)
                            {
                                /*
                                   Ensure this string is not already
                                   in the merged string entries list.
                                */
                                size_t i2;
                                HlBool skipString = HL_FALSE;

                                for (i2 = 0; i2 < mergedStrs.count; ++i2)
                                {
                                    if (mergedStrs.data[i2].str == curOffsetVal)
                                    {
                                        skipString = HL_TRUE;
                                        break;
                                    }
                                }

                                if (!skipString)
                                {
                                    /*
                                       Make a temporary "merged string entry" full of information
                                       we'll need later to set up this string in the BINA data.
                                    */
                                    const HlINPACxV2MergedStrEntry mergedStr =
                                    {
                                        curOffsetVal,               /* str */
                                        strlen(curOffsetVal) + 1,   /* size */
                                        curStrTableOff              /* off */
                                    };

                                    /* Add merged string entry to string list. */
                                    result = HL_LIST_PUSH(mergedStrs, mergedStr);
                                    if (HL_FAILED(result))
                                    {
                                        HL_LIST_FREE(mergedStrs);
                                        hlFree(hlArcEntry.path);
                                        return result;
                                    }

                                    /* Increase current string offset and string table size. */
                                    curStrTableOff += (HlU32)mergedStr.size;
                                    strTableSize += mergedStr.size;
                                }
                            }
                        }

                        /* Account for offset table entries. */
                        {
                            const HlU32 offDiff = (HlU32)(curOffset - prevOffset);
                            
                            if (offDiff <= 0x3FU)
                            {
                                /* Account for six-bit BINA offset table entry. */
                                ++offTableSize;
                            }
                            else if (offDiff <= 0x3FFFU)
                            {
                                /* Account for fourteen-bit BINA offset table entry. */
                                offTableSize += 2;
                            }
                            else
                            {
                                /*
                                    Ensure offset difference is within 30 bits.
                                    (This *ALWAYS* should be true, so if it's false,
                                    something has seriously gone wrong.)
                                */
                                HL_ASSERT(offDiff <= 0x3FFFFFFFU);

                                /* Account for thirty-bit BINA offset table entry. */
                                offTableSize += 4;
                            }
                        }

                        /* Set previous offset pointer. */
                        prevOffset = curOffset;

                        /*
                            Get the next offset's address - return early
                            if we've reached the end of the offset table.
                        */
                        if (offsets >= eof || !hlBINAOffsetsNext(&offsets, &curOffset))
                            break;
                    }

                    /* Reset curOffset, offsets, and prevOffset pointers. */
                    curOffset = firstDataOffset;
                    offsets = firstDataOffsetPos;
                    prevOffset = data;
                }

                /* Compute table padding amounts and setup data buffer. */
                {
                    HlBINAV2Header* dstHeader;
                    HlBINAV2BlockDataHeader* dstDataBlock;

                    /* Compute string and offset table padding amount. */
                    strTablePadding = (HL_ALIGN(strTableSize, 4) - strTableSize);
                    offTablePadding = (HL_ALIGN(offTableSize, 4) - offTableSize);

                    /* Increase string and offset table size to account for padding. */
                    strTableSize += strTablePadding;
                    offTableSize += offTablePadding;

                    /* Increase entry size to account for string and offset tables. */
                    hlArcEntry.size += strTableSize;
                    hlArcEntry.size += offTableSize;

                    /* Allocate data buffer. */
                    dstDataBuf = (HlU8*)hlAlloc(hlArcEntry.size);

                    if (!dstDataBuf)
                    {
                        HL_LIST_FREE(mergedStrs);
                        hlFree(hlArcEntry.path);
                        return HL_ERROR_OUT_OF_MEMORY;
                    }

                    /* Set data pointer within file entry. */
                    hlArcEntry.data = (HlUMax)((HlUPtr)dstDataBuf);

                    /* Setup BINA header. */
                    dstHeader = (HlBINAV2Header*)dstDataBuf;
                    dstHeader->signature = HL_BINA_SIG;
                    dstHeader->version[0] = '2';
                    dstHeader->version[1] = '0';
                    dstHeader->version[2] = '0';
                    dstHeader->endianFlag = endianFlag;
                    dstHeader->fileSize = (HlU32)hlArcEntry.size;
                    dstHeader->blockCount = 1;
                    dstHeader->padding = 0;

                    /* Setup BINA data block header. */
                    dstDataBlock = (HlBINAV2BlockDataHeader*)(dstHeader + 1);
                    dstDataBlock->signature = HL_BINAV2_BLOCK_TYPE_DATA;
                    dstDataBlock->size = (dstHeader->fileSize - sizeof(HlBINAV2Header));
                    dstDataBlock->stringTableOffset = dataEntry->dataSize;
                    dstDataBlock->stringTableSize = (HlU32)strTableSize;
                    dstDataBlock->offsetTableSize = (HlU32)offTableSize;
                    dstDataBlock->relativeDataOffset = sizeof(HlBINAV2BlockDataHeader);
                    dstDataBlock->padding = 0;

                    /* Endian swap header and data block header if necessary. */
                    if (hlBINANeedsSwap(endianFlag))
                    {
                        hlBINAV2HeaderSwap(dstHeader);
                        hlBINAV2DataHeaderSwap(dstDataBlock, HL_TRUE);
                    }

                    /* Increase curDataPtr pointer past header and data block. */
                    curDataPtr = (HlU8*)(dstDataBlock + 1);

                    /* Pad BINA data block. */
                    memset(curDataPtr, 0, sizeof(HlBINAV2BlockDataHeader));

                    /* Increase curDataPtr pointer past BINA data block padding. */
                    curDataPtr += sizeof(HlBINAV2BlockDataHeader);

                    /* Increase dstDataBuf past curDataPtr. */
                    dstDataBuf = curDataPtr;

                    /* Copy data. */
                    memcpy(curDataPtr, data, dataEntry->dataSize);

                    /* Increase curDataPtr pointer past data. */
                    curDataPtr += dataEntry->dataSize;

                    /* TODO: Pad data? */
                }

                /* Setup string table. */
                {
                    /* Copy strings into data buffer. */
                    size_t i2;
                    for (i2 = 0; i2 < mergedStrs.count; ++i2)
                    {
                        /* Copy strings (and null terminators) into data buffer. */
                        memcpy(curDataPtr, mergedStrs.data[i2].str,
                            mergedStrs.data[i2].size);

                        /* Increase curDataPtr pointer past string. */
                        curDataPtr += mergedStrs.data[i2].size;
                    }

                    /* Pad string table. */
                    memset(curDataPtr, 0, strTablePadding);

                    /* Increase curDataPtr pointer past string table padding. */
                    curDataPtr += strTablePadding;
                }

                /* Fix offsets and setup offset table. */
                while (HL_TRUE)
                {
                    const char* curOffVal;
                    HlU32* dstOffPtr;

                    /* Stop if this offset is not part of the current file. */
                    if (curOffset >= dataEnd) break;

                    /* Get the value the current offset points to. */
                    curOffVal = (const char*)hlOff32Get(curOffset);

                    /* Get a pointer to the destination offset in the data buffer. */
                    dstOffPtr = (((HlU32*)dstDataBuf) +
                        (curOffset - data));

                    /* Copy strings and fix string offsets. */
                    if (curOffVal >= strings)
                    {
                        /*
                           Ensure this string is not already
                           in the merged string entries list.
                        */
                        size_t i2;
                        for (i2 = 0; i2 < mergedStrs.count; ++i2)
                        {
                            /* Fix string offsets. */
                            if (mergedStrs.data[i2].str == curOffVal)
                            {
                                *dstOffPtr = mergedStrs.data[i2].off;
                                break;
                            }
                        }
                    }

                    /* Fix non-string offsets. */
                    else
                    {
                        *dstOffPtr = (HlU32)(
                            (const HlU8*)curOffVal -
                            (const HlU8*)data);
                    }

                    /* Endian-swap offset if necessary. */
                    if (hlBINANeedsSwap(endianFlag))
                    {
                        hlSwapU32P(dstOffPtr);
                    }

                    /* Setup offset table entry. */
                    {
                        const HlU32 offDiff = (HlU32)(curOffset - prevOffset);

                        if (offDiff <= 0x3FU)
                        {
                            /* Setup six-bit BINA offset table entry. */
                            *(curDataPtr++) = (HL_BINA_OFF_SIZE_SIX_BIT | (HlU8)offDiff);
                        }
                        else if (offDiff <= 0x3FFFU)
                        {
                            /* Setup fourteen-bit BINA offset table entry. */
                            *(curDataPtr++) = (HL_BINA_OFF_SIZE_FOURTEEN_BIT |
                                (HlU8)(offDiff >> 8));

                            *(curDataPtr++) = (HlU8)(offDiff & 0xFF);
                        }
                        else
                        {
                            /*
                                Ensure offset difference is within 30 bits.
                                (This *ALWAYS* should be true, so if it's false,
                                something has seriously gone wrong.)
                            */
                            HL_ASSERT(offDiff <= 0x3FFFFFFFU);

                            /* Setup thirty-bit BINA offset table entry. */
                            *(curDataPtr++) = (HL_BINA_OFF_SIZE_THIRTY_BIT |
                                (HlU8)(offDiff >> 24));

                            *(curDataPtr++) = (HlU8)((offDiff & 0xFF0000) >> 16);
                            *(curDataPtr++) = (HlU8)((offDiff & 0xFF00) >> 8);
                            *(curDataPtr++) = (HlU8)(offDiff & 0xFF);
                        }
                    }

                    /* Set previous offset pointer. */
                    prevOffset = curOffset;

                    /*
                        Get the next offset's address - return early
                        if we've reached the end of the offset table.
                    */
                    if (offsets >= eof || !hlBINAOffsetsNext(&offsets, &curOffset))
                        break;
                }

                /* Free merged string entry list. */
                HL_LIST_FREE(mergedStrs);

                /* Pad offset table. */
                memset(curDataPtr, 0, offTablePadding);
            }
            else
            {
                /* Allocate data buffer. */
                dstDataBuf = (HlU8*)hlAlloc(hlArcEntry.size);

                if (!dstDataBuf)
                {
                    hlFree(hlArcEntry.path);
                    return HL_ERROR_OUT_OF_MEMORY;
                }

                /* Copy data. */
                memcpy(dstDataBuf, data, hlArcEntry.size);

                /* Set data pointer within file entry. */
                hlArcEntry.data = (HlUMax)((HlUPtr)dstDataBuf);
            }
        }
        else
        {
            /* Set meta and data within file entry. */
            hlArcEntry.meta = HL_ARC_ENTRY_STREAMING_FLAG;
            hlArcEntry.data = (HlUMax)((HlUPtr)NULL);
        }

        /* Add entry to archive. */
        result = HL_LIST_PUSH(hlArc->entries, hlArcEntry);
        if (HL_FAILED(result))
        {
            hlArchiveEntryDestruct(&hlArcEntry);
            return result;
        }
    }

    return HL_RESULT_SUCCESS;
}

static HlResult hlINPACxV2ParseInto(const HlPACxV2Header* HL_RESTRICT pac,
    HlBool skipProxies, HlArchive* HL_RESTRICT hlArc,
    const HlPACxV2NodeTree** depsFileTree)
{
    /* Get data block pointer. */
    const HlPACxV2BlockDataHeader* dataBlock = hlPACxV2GetDataBlock(pac);
    const HlU32* firstDataOffset;
    const char* strings;
    const HlU8 *offsets, *eof;
    HlResult result = HL_RESULT_SUCCESS;

    /*
       Store NULL in depsFileTree pointer if a pointer was requested.

       (We'll replace it with the actual pointer later if a ResPacDepend
       file tree is found.)
    */
    if (depsFileTree) *depsFileTree = NULL;

    /* NOTE: Some .pac files in LW actually don't have DATA blocks (e.g. w1a03_far.pac) */
    if (!dataBlock) return HL_RESULT_SUCCESS;

    /* Get current offset pointer. */
    firstDataOffset = (const HlU32*)pac;

    /* Get strings, offsets, and eof pointers. */
    strings = hlPACxV2DataGetStringTable(dataBlock);
    offsets = (const HlU8*)hlPACxV2DataGetOffsetTable(dataBlock);
    eof = (offsets + dataBlock->offsetTableSize);

    /* OPTIMIZATION: Skip through all offsets that aren't part of the data. */
    {
        const HlU32* dataEntries = (const HlU32*)
            hlPACxV2DataGetDataEntries(dataBlock);

        while (offsets < eof)
        {
            /* Break if we've reached an offset within the data entries. */
            if (firstDataOffset >= dataEntries) break;

            /*
                Get the next offset's address - break if
                we've reached the end of the offset table.
            */
            if (!hlBINAOffsetsNext(&offsets, &firstDataOffset))
                break;
        }
    }

    /* Setup file entries in this pac. */
    {
        /* Get pointers. */
        const HlPACxV2NodeTree* typeTree = (const HlPACxV2NodeTree*)
            (dataBlock + 1);

        const HlPACxV2Node* typeNodes = (const HlPACxV2Node*)
            hlOff32Get(&typeTree->nodes);

        HlU32 i2;

        /* Setup file entries in this pac. */
        for (i2 = 0; i2 < typeTree->nodeCount; ++i2)
        {
            const HlPACxV2NodeTree* fileTree;
            const char* typeStr = (const char*)hlOff32Get(&typeNodes[i2].name);
            const char* colonPtr = strchr(typeStr, ':');
            size_t extSrcLen, extDstLen;

            /* Get fileTree pointer. */
            fileTree = (const HlPACxV2NodeTree*)hlOff32Get(&typeNodes[i2].data);

            /* Skip ResPacDepend file trees. */
            if (!strcmp(colonPtr + 1, HlPACxDataTypes[ResPacDepend]))
            {
                /* Store pointer to ResPacDepend file tree if requested. */
                if (depsFileTree) *depsFileTree = fileTree;
                continue;
            }

            /* Compute extension length. */
            extSrcLen = (size_t)(colonPtr - typeStr);

            /* Account for UTF-8 -> native conversion. */
            extDstLen = hlStrGetReqLenUTF8ToNative(typeStr, extSrcLen);

            /* Setup file entries. */
            result = hlINPACxV2FileTreeSetupEntries(fileTree,
                skipProxies, pac->endianFlag, firstDataOffset,
                strings, offsets, eof, typeStr, extSrcLen,
                extDstLen, hlArc);

            if (HL_FAILED(result)) return result;
        }
    }

    return result;
}

HlResult hlPACxV2ParseInto(const HlPACxV2Header* HL_RESTRICT pac,
    HlBool skipProxies, HlArchive* HL_RESTRICT hlArc)
{
    return hlINPACxV2ParseInto(pac, skipProxies, hlArc, NULL);
}

HlResult hlPACxV2ReadInto(void* HL_RESTRICT rawData,
    HlBool skipProxies, HlArchive* HL_RESTRICT hlArc)
{
    /* Fix PACxV2 data. */
    hlPACxV2Fix(rawData);

    /* Parse PACxV2 data into HlArchive and return result. */
    return hlPACxV2ParseInto((const HlPACxV2Header*)rawData,
        skipProxies, hlArc);
}

HlResult hlPACxV2LoadSingleInto(const HlNChar* HL_RESTRICT filePath,
    HlBool skipProxies, HlBlobList* HL_RESTRICT pacs, HlArchive* HL_RESTRICT hlArc)
{
    HlBlob* pac;
    HlResult result;

    /* Load archive. */
    result = hlBlobLoad(filePath, &pac);
    if (HL_FAILED(result)) return result;

    /* Fix PACxV2 data. */
    hlPACxV2Fix(pac->data);

    /* Add this archive's blob to the blobs list if necessary. */
    if (pacs)
    {
        result = HL_LIST_PUSH(*pacs, pac);
        if (HL_FAILED(result))
        {
            hlBlobFree(pac);
            return result;
        }
    }

    /* Parse blob into HlArchive, free blob if necessary, and return. */
    if (hlArc)
    {
        result = hlPACxV2ParseInto(
            (const HlPACxV2Header*)pac->data,
            skipProxies, hlArc);
    }

    if (!pacs) hlBlobFree(pac);
    return result;
}

static HlResult hlINPACxV2LoadAllInto(
    HlBlob* HL_RESTRICT rootPac, HlBool loadSplits, HlNChar* pathBuf,
    HlNChar* HL_RESTRICT * HL_RESTRICT pathBufPtr,
    size_t* HL_RESTRICT pathBufCap, HlBlobList* HL_RESTRICT pacs,
    HlArchive* HL_RESTRICT hlArc)
{
    const HlPACxV2NodeTree* splitFileTree;
    const HlPACxV2Node* splitFileNodes;
    size_t dirLen;
    HlU32 i;
    HlResult result = HL_RESULT_SUCCESS;

    /* Fix root PACxV2 data. */
    hlPACxV2Fix(rootPac->data);

    /* Add this archive's blob to the blobs list if necessary. */
    if (pacs)
    {
        result = HL_LIST_PUSH(*pacs, rootPac);
        if (HL_FAILED(result)) return result;
    }

    /* Parse root pac if necessary. */
    if (hlArc)
    {
        result = hlINPACxV2ParseInto(
            (const HlPACxV2Header*)rootPac->data,
            HL_TRUE, hlArc, &splitFileTree);

        if (HL_FAILED(result)) return result;
    }

    /* Get split file tree if necessary. */
    else if (loadSplits)
    {
        const HlPACxV2BlockDataHeader* dataBlock =
            hlPACxV2GetDataBlock(rootPac->data);

        if (!dataBlock) return result;

        splitFileTree = hlPACxV2DataGetFileTree(
            dataBlock, HlPACxDataTypes[ResPacDepend]);
    }

    /* Return early if not loading splits or if the root pac has no dependencies ("splits"). */
    if (!loadSplits || !splitFileTree) return result;

    /* Compute directory (including path separator, if any) length from root path. */
    dirLen = (size_t)(hlPathGetName(*pathBufPtr) - *pathBufPtr);

    /* Get split file nodes. */
    splitFileNodes = (const HlPACxV2Node*)hlOff32Get(
        &splitFileTree->nodes);

    /* Load all splits referenced in each split table. */
    for (i = 0; i < splitFileTree->nodeCount; ++i)
    {
        const HlPACxV2DataEntry* dataEntry = (const HlPACxV2DataEntry*)
            hlOff32Get(&splitFileNodes[i].data);

        const HlPACxV2SplitTable* splitTable = (const HlPACxV2SplitTable*)
            (dataEntry + 1);

        const HL_OFF32_STR* splitNames = (const HL_OFF32_STR*)
            hlOff32Get(&splitTable->splitNames);

        HlU32 i2;

        /* Load all splits referenced in this split table. */
        for (i2 = 0; i2 < splitTable->splitCount; ++i2)
        {
            const char* splitName = (const char*)hlOff32Get(&splitNames[i2]);
            const size_t splitNameSize = hlStrGetReqLenUTF8ToNative(splitName, 0);

            /* Account for directory. */
            const size_t reqPathBufSize = (splitNameSize + dirLen);

            /* Resize path buffer if necessary. */
            if (reqPathBufSize > *pathBufCap)
            {
                if (*pathBufPtr != pathBuf)
                {
                    /* Switch from stack buffer to heap buffer. */
                    *pathBufPtr = HL_ALLOC_ARR(HlNChar, reqPathBufSize);
                }
                else
                {
                    /* Resize existing heap buffer. */
                    *pathBufPtr = HL_RESIZE_ARR(HlNChar, reqPathBufSize, *pathBufPtr);
                }

                /* Return error if allocation failed. */
                if (!(*pathBufPtr))
                {
                    result = HL_ERROR_OUT_OF_MEMORY;
                    return result;
                }

                /* Set new path buffer capacity. */
                *pathBufCap = reqPathBufSize;
            }

            /* Copy split name into buffer. */
            if (!hlStrConvUTF8ToNativeNoAlloc(splitName,
                &((*pathBufPtr)[dirLen]), 0, *pathBufCap))
            {
                result = HL_ERROR_UNKNOWN;
                return result;
            }

            /* Load split pac. */
            result = hlPACxV2LoadSingleInto(*pathBufPtr, HL_TRUE, pacs, hlArc);
            if (HL_FAILED(result)) return result;
        }
    }

    return result;
}

HlResult hlPACxV2LoadAllInto(const HlNChar* HL_RESTRICT filePath,
    HlBlobList* HL_RESTRICT pacs, HlArchive* HL_RESTRICT hlArc)
{
    HlNChar pathBuf[255];
    HlNChar* pathBufPtr = pathBuf;
    HlBlob* rootPac;
    size_t pathBufCap = 255;
    HlResult result;

    /* Get root pac path and store it in our stack buffer, or in a heap buffer if necessary. */
    result = hlINPACxGetRootPathStackBuf(filePath, &pathBufCap, &pathBufPtr);
    if (HL_FAILED(result)) return result;

    /* Load root pac. */
    result = hlBlobLoad(pathBufPtr, &rootPac);
    if (HL_FAILED(result)) goto end;

    /* Load all pacs. */
    result = hlINPACxV2LoadAllInto(rootPac, HL_TRUE,
        pathBuf, &pathBufPtr, &pathBufCap, pacs, hlArc);

    if (!pacs) hlBlobFree(rootPac);

end:
    /* Free path buffer if necessary and return result. */
    if (pathBufPtr != pathBuf) hlFree(pathBufPtr);
    return result;
}

HlResult hlPACxV2Load(const HlNChar* HL_RESTRICT filePath,
    HlBool loadSplits, HlBlobList* HL_RESTRICT pacs,
    HlArchive* HL_RESTRICT * HL_RESTRICT hlArc)
{
    HlArchive* hlArcBuf;
    HlResult result;

    /* Allocate HlArchive buffer. */
    hlArcBuf = HL_ALLOC_OBJ(HlArchive);
    if (!hlArcBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Setup archive. */
    HL_LIST_INIT(hlArcBuf->entries);

    /*
       Add all the files from all the splits or simply a
       single archive based on user request.
    */
    result = (loadSplits) ?
        hlPACxV2LoadAllInto(filePath, pacs, hlArcBuf) :
        hlPACxV2LoadSingleInto(filePath, HL_TRUE, pacs, hlArcBuf);

    if (HL_FAILED(result)) return result;

    /* Set pointer and return result. */
    *hlArc = hlArcBuf;
    return result;
}

HlResult hlPACxV2StartWrite(HlBINAEndianFlag endianFlag, HlStream* stream)
{
    /* Generate PACxV2 header. */
    const HlPACxV2Header header =
    {
        HL_PACX_SIG,                                        /* signature */
        { '2', '0', '1' },                                  /* version */
        (HlU8)endianFlag,                                   /* endianFlag */
        0,                                                  /* fileSize */
        0,                                                  /* blockCount */
        0                                                   /* padding */
    };

    /*
       NOTE: We don't need to swap the header yet since the only values
       that ever need to be swapped are going to be filled-in later.
    */

    /* Write PACxV2 header and return result. */
    return hlStreamWrite(stream, sizeof(header), &header, NULL);
}

HlResult hlPACxV2FinishWrite(size_t headerPos, HlU16 blockCount,
    HlBINAEndianFlag endianFlag, HlStream* stream)
{
    return hlBINAV2FinishWrite(headerPos, blockCount, endianFlag, stream);
}

HlResult hlPACxV2DataBlockStartWrite(HlStream* stream)
{
    /* Generate data block header. */
    const HlPACxV2BlockDataHeader dataBlock =
    {
        HL_BINAV2_BLOCK_TYPE_DATA,          /* signature */
        0,                                  /* size */
        0,                                  /* dataEntriesSize */
        0,                                  /* treesSize */
        0,                                  /* proxyTableSize */
        0,                                  /* stringTableSize */
        0,                                  /* offsetTableSize */
        1,                                  /* unknown1 */
        0,                                  /* padding1 */
        0                                   /* padding2 */
    };

    /*
       NOTE: We don't need to swap the header yet since the only values
       that ever need to be swapped are going to be filled-in later.
    */

    /* Write PACx data block and return. */
    return hlStreamWrite(stream, sizeof(dataBlock), &dataBlock, NULL);
}

HlResult hlPACxV2DataBlockFinishWrite(size_t headerPos, size_t dataBlockPos,
    HlU32 treesSize, HlU32 dataEntriesSize, HlU32 proxyTableSize,
    HlBINAEndianFlag endianFlag, const HlStrTable* HL_RESTRICT strTable,
    HlOffTable* HL_RESTRICT offTable, HlStream* HL_RESTRICT stream)
{
    size_t strTablePos, offTablePos, eofPos;
    HlResult result;

    /* Get string table position. */
    strTablePos = hlStreamTell(stream);

    /* Write string table. */
    result = hlBINAStringsWrite32(headerPos, endianFlag, strTable, offTable, stream);
    if (HL_FAILED(result)) return result;

    /* Get offset table position. */
    offTablePos = hlStreamTell(stream);

    /* Write offset table. */
    result = hlBINAOffsetsWrite32(headerPos, offTable, stream);
    if (HL_FAILED(result)) return result;

    /* Get end of stream position. */
    eofPos = hlStreamTell(stream);

    /* Jump to data block size position. */
    result = hlStreamJumpTo(stream, dataBlockPos + 4);
    if (HL_FAILED(result)) return result;

    /* Fill-in data block header values. */
    {
        struct
        {
            HlU32 blockSize;
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
        }
        values;

        /* Compute data block header values. */
        values.blockSize = (HlU32)(eofPos - dataBlockPos);
        values.dataEntriesSize = dataEntriesSize;
        values.treesSize = treesSize;
        values.proxyTableSize = proxyTableSize;
        values.stringTableSize = (HlU32)(offTablePos - strTablePos);
        values.offsetTableSize = (HlU32)(eofPos - offTablePos);

        /* Endian-swap data block header values if necessary. */
        if (hlBINANeedsSwap(endianFlag))
        {
            hlSwapU32P(&values.blockSize);
            hlSwapU32P(&values.dataEntriesSize);
            hlSwapU32P(&values.treesSize);
            hlSwapU32P(&values.proxyTableSize);
            hlSwapU32P(&values.stringTableSize);
            hlSwapU32P(&values.offsetTableSize);
        }

        /* Fill-in data block header values. */
        result = hlStreamWrite(stream, sizeof(values), &values, NULL);
        if (HL_FAILED(result)) return result;
    }

    /* Jump to end of stream and return. */
    return hlStreamJumpTo(stream, eofPos);
}

typedef struct HlINPACxV2FileMetadata
{
    const HlArchiveEntry* entry;
    const HlNChar* name;
    /** @brief The extensions of this file, without the initial dot (e.g. "dds"). */
    const HlNChar* ext;
    const HlPACxSupportedExt* pacxExt;
    short rootSortWeight;
    short splitSortWeight;
    unsigned short splitIndex;
    short priority;
}
HlINPACxV2FileMetadata;

typedef struct HlINPACxV2TypeMetadata
{
    char* typeStr;
    const HlINPACxV2FileMetadata* files;
    size_t fileCount;
}
HlINPACxV2TypeMetadata;

typedef HL_LIST(HlINPACxV2TypeMetadata) HlINPACxV2TypeMetadataList;

static char* hlINPACxV2CreateTypeStr(const HlINPACxV2FileMetadata* fileMetadata)
{
    /* Get pointers and sizes. */
    const char* pacxDataType = HlPACxDataTypes[fileMetadata->pacxExt->pacxDataTypeIndex];
    const HlNChar* ext = ((*fileMetadata->ext == HL_NTEXT('.')) ?
        (fileMetadata->ext + 1) : fileMetadata->ext);

    const size_t u8ExtSize = hlStrGetReqLenNativeToUTF8(ext, 0);
    const size_t u8TypeSize = (strlen(pacxDataType) + 1);
    const size_t typeStrSize = (u8ExtSize + u8TypeSize);

    /* Allocate buffer for type string. */
    char* typeStr = HL_ALLOC_ARR(char, typeStrSize);
    if (!typeStr) return NULL;

    /* Convert extension to UTF-8 and copy into type string. */
    if (!hlStrConvNativeToUTF8NoAlloc(ext, typeStr, 0, typeStrSize))
    {
        hlFree(typeStr);
        return NULL;
    }

    /* Copy ':' separator into type string. */
    typeStr[u8ExtSize - 1] = ':';

    /* Copy PACx data type and null terminator into type string and return type string. */
    memcpy(&typeStr[u8ExtSize], pacxDataType, u8TypeSize);
    return typeStr;
}

static int hlINPACxV2CompareFileNames(
    const HlNChar* HL_RESTRICT fileName1, const HlNChar* HL_RESTRICT fileName2,
    size_t fileNameLen1, size_t fileNameLen2)
{
    if (fileNameLen1 == fileNameLen2)
    {
        return hlNStrNCmp(fileName1, fileName2, fileNameLen1);
    }
    else
    {
        const size_t fileNameMinLen = HL_MIN(fileNameLen1, fileNameLen2);
        const int nameSortWeight = hlNStrNCmp(fileName1,
            fileName2, fileNameMinLen);

        if (nameSortWeight != 0)
        {
            return nameSortWeight;
        }
        else
        {
            return (fileNameMinLen == fileNameLen1) ?
                (0 - (int)fileName2[fileNameLen2 - 1]) :
                ((int)fileName1[fileNameLen1 - 1] - 0);
        }
    }
}

static int hlINPACxV2FileMetadataCompareNames(
    const HlINPACxV2FileMetadata* HL_RESTRICT fileMeta1,
    const HlINPACxV2FileMetadata* HL_RESTRICT fileMeta2)
{
    return hlINPACxV2CompareFileNames(fileMeta1->name, fileMeta2->name,
        (fileMeta1->ext - fileMeta1->name),
        (fileMeta2->ext - fileMeta2->name));
}

static HlResult hlINPACxV2FileMetadataMakeCopyOfData(
    const HlINPACxV2FileMetadata* HL_RESTRICT fileMetadata,
    void* HL_RESTRICT* HL_RESTRICT copyOfFileData)
{
    void* newFileDataBuf;
    HlResult result;

    /* Allocate a buffer to hold a copy of the current file's data. */
    newFileDataBuf = hlAlloc(fileMetadata->entry->size);
    if (!newFileDataBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Copy the current file's data into the buffer. */
    if (fileMetadata->entry->data)
    {
        memcpy(newFileDataBuf, (const void*)(
            (HlUPtr)fileMetadata->entry->data),
            fileMetadata->entry->size);
    }
    else
    {
        /* This is a file reference; open up the file. */
        HlFileStream* file;
        result = hlFileStreamOpen(fileMetadata->entry->path,
            HL_FILE_MODE_READ, &file);

        if (HL_FAILED(result)) goto failed;

        /* Read the file's data into the buffer. */
        result = hlStreamRead(file, fileMetadata->entry->size,
            newFileDataBuf, NULL);

        if (HL_FAILED(result))
        {
            hlFileStreamClose(file);
            goto failed;
        }

        /* Close the file. */
        result = hlFileStreamClose(file);
        if (HL_FAILED(result)) goto failed;
    }

    /* Set pointer and return success. */
    *copyOfFileData = newFileDataBuf;
    return HL_RESULT_SUCCESS;

failed:
    hlFree(newFileDataBuf);
    return result;
}

static const HlINPACxV2FileMetadata* hlINPACxV2TypeMetadataFindFile(
    const HlINPACxV2TypeMetadata* HL_RESTRICT typeMetadata,
    const HlNChar* HL_RESTRICT fileName, size_t fileNameLen)
{
    size_t i;
    for (i = 0; i < typeMetadata->fileCount; ++i)
    {
        const HlINPACxV2FileMetadata* curFileMetadata = &typeMetadata->files[i];

        if (hlINPACxV2CompareFileNames(curFileMetadata->name,
            fileName, (curFileMetadata->ext - curFileMetadata->name),
            fileNameLen) == 0)
        {
            return curFileMetadata;
        }
    }

    return NULL;
}

static const HlINPACxV2FileMetadata* hlINPACxV2TypeMetadataListFindFileOfType(
    const HlINPACxV2TypeMetadataList* HL_RESTRICT typeMetadata,
    unsigned short pacxDataType, const HlNChar* HL_RESTRICT fileName,
    size_t fileNameLen)
{
    size_t i;

    /* Find file metadata within type metadatas with the given PACx data type. */
    for (i = 0; i < typeMetadata->count; ++i)
    {
        const HlINPACxV2TypeMetadata* curTypeMetadata = &typeMetadata->data[i];
        const HlINPACxV2FileMetadata* fileMetadata;

        /* Skip this type metadata if it's not of the given PACx data type. */
        if (curTypeMetadata->files->pacxExt->pacxDataTypeIndex != pacxDataType)
            continue;

        /* Find file metadata witin this type metadata. */
        fileMetadata = hlINPACxV2TypeMetadataFindFile(curTypeMetadata,
            fileName, fileNameLen);

        if (fileMetadata) return fileMetadata;
    }

    return NULL;
}

static const HlINPACxV2FileMetadata* hlINPACxV2TypeMetadataListFindFileOfTypeUTF8(
    const HlINPACxV2TypeMetadataList* HL_RESTRICT typeMetadata,
    unsigned short pacxDataType, const char* HL_RESTRICT fileName)
{
    const HlINPACxV2FileMetadata* fileMetadata;

    /* Get native file name. */
#ifdef HL_IN_WIN32_UNICODE
    HlNChar* nativeFileName = hlStrConvUTF8ToNative(fileName, 0);
    if (!nativeFileName) return NULL;
#else
    const HlNChar* nativeFileName = fileName;
#endif

    /* Get file metadata. */
    fileMetadata = hlINPACxV2TypeMetadataListFindFileOfType(
        typeMetadata, pacxDataType, nativeFileName,
        hlNStrLen(nativeFileName));

    /* Free native file name if necessary and return file metadata. */
#ifdef HL_IN_WIN32_UNICODE
    hlFree(nativeFileName);
#endif

    return fileMetadata;
}

static HlResult hlINPACxV2MarkRefsGrifEffect(HlGrifEffect* HL_RESTRICT effect,
    HlBINAEndianFlag endianFlag, HlINPACxV2TypeMetadataList* HL_RESTRICT typeMetadata)
{
    const HlGrifEffectParameterV106* param;
    HlResult result;

    /* Fix effect. */
    result = hlGrifEffectFix(effect, endianFlag);
    if (HL_FAILED(result)) return result;

    /* Get first parameter offset. */
    param = (const HlGrifEffectParameterV106*)hlOff32Get(&effect->paramsOffset);

    /* Mark references within parameters. */
    while (param)
    {
        const HlGrifEmitterParameterV106* emitter = (const HlGrifEmitterParameterV106*)
            hlOff32Get(&param->emitterOffset);

        while (emitter)
        {
            const HlGrifParticleParameterV106* particle = (const HlGrifParticleParameterV106*)
                hlOff32Get(&emitter->particleOffset);

            while (particle)
            {
                const HlGrifMaterialParameterV106* material = (const HlGrifMaterialParameterV106*)
                    hlOff32Get(&particle->materialOffset);

                if (material)
                {
                    HlU32 i, texCount = HL_MIN(material->texCount, 2);
                    for (i = 0; i < texCount; ++i)
                    {
                        const char* texName = (const char*)hlOff32Get(
                            &material->textures[i].nameOffset);

                        HlINPACxV2FileMetadata* texture = (HlINPACxV2FileMetadata*)
                            hlINPACxV2TypeMetadataListFindFileOfTypeUTF8(
                                typeMetadata, ResTexture, texName);

                        if (texture)
                        {
                            texture->priority = -1;
                        }
                    }
                }

                particle = (const HlGrifParticleParameterV106*)
                    hlOff32Get(&particle->nextParticleOffset);
            }

            emitter = (const HlGrifEmitterParameterV106*)
                hlOff32Get(&emitter->nextEmitterOffset);
        }

        param = (const HlGrifEffectParameterV106*)
            hlOff32Get(&param->nextParamOffset);
    }

    return HL_RESULT_SUCCESS;
}

static HlResult hlINPACxV2TypeMetadatalistSetFilePriorities(
    HlINPACxV2TypeMetadataList* typeMetadata, HlBINAEndianFlag endianFlag)
{
    size_t i;
    HlResult result;

    for (i = 0; i < typeMetadata->count; ++i)
    {
        HlINPACxV2TypeMetadata* curTypeMetadata = &typeMetadata->data[i];
        size_t i2;

        if (curTypeMetadata->files->pacxExt->pacxDataTypeIndex == ResTexture)
        {
            /* Mark textures that share the same name as terrain models as lower priority. */
            for (i2 = 0; i2 < curTypeMetadata->fileCount; ++i2)
            {
                HlINPACxV2FileMetadata* texture = (HlINPACxV2FileMetadata*)
                    (&curTypeMetadata->files[i2]);

                const size_t texNameLen = (texture->ext - texture->name);
                size_t i3;

                if (texture->priority != 0)
                    continue;

                /*
                   Rio 2016 seems to mark textures that start with "t_" as lower priority as well
                   for some reason, so we do that here as well.

                   TODO: Lost World *doesn't* do this from what I can tell, so it's probably ideal
                   to only do this check if generating Rio 2016 .pac files - although in practice
                   it really doesn't matter as base LW never has any textures that start with "t_"
                   anyway and custom pacs that have textures like this in them should still load
                   just fine).
                */
                if (hlNStrNCmp(HL_NTEXT("t_"), texture->name, 2) == 0)
                {
                    texture->priority = -1;
                    continue;
                }

                for (i3 = 0; i3 < typeMetadata->count; ++i3)
                {
                    const HlINPACxV2TypeMetadata* type = &typeMetadata->data[i3];
                    const HlINPACxV2FileMetadata* fileMetadata;

                    /* Skip this type metadata if it's not of the required PACx data types. */
                    if (type->files->pacxExt->pacxDataTypeIndex != ResMirageTerrainModel &&
                        type->files->pacxExt->pacxDataTypeIndex != ResMirageTerrainInstanceInfo)
                    {
                        continue;
                    }

                    /* Find file metadata witin this type metadata. */
                    fileMetadata = hlINPACxV2TypeMetadataFindFile(type,
                        texture->name, texNameLen);

                    if (fileMetadata)
                    {
                        texture->priority = -1;
                        break;
                    }
                }
            }
        }
        else if (curTypeMetadata->files->pacxExt->pacxDataTypeIndex == ResGrifEffect)
        {
            for (i2 = 0; i2 < curTypeMetadata->fileCount; ++i2)
            {
                const HlINPACxV2FileMetadata* curFileMetadata = &curTypeMetadata->files[i2];
                void* copyOfFileData;
                HlGrifEffect* effect;
                HlBINAEndianFlag effectEndianFlag;

                /* Make a copy of the file's data that we can safely operate on. */
                result = hlINPACxV2FileMetadataMakeCopyOfData(curFileMetadata, &copyOfFileData);
                if (HL_FAILED(result)) return result;

                /* Get effect endianness. */
                effectEndianFlag = (hlBINAHasV2Header(copyOfFileData)) ?
                    ((HlBINAV2Header*)copyOfFileData)->endianFlag : endianFlag;

                /* Fix BINA general data. */
                hlBINAV2Fix(copyOfFileData, curFileMetadata->entry->size);

                /* Get effect pointer. */
                effect = (HlGrifEffect*)hlBINAV2GetData(copyOfFileData);
                if (!effect)
                {
                    hlFree(copyOfFileData);
                    return HL_ERROR_INVALID_DATA;
                }

                /* Mark texture references within effect data. */
                result = hlINPACxV2MarkRefsGrifEffect(effect,
                    effectEndianFlag, typeMetadata);

                hlFree(copyOfFileData);
                if (HL_FAILED(result)) return result;
            }
        }
    }

    return HL_RESULT_SUCCESS;
}

static void hlINPACxV2TypeMetadataListFree(HlINPACxV2TypeMetadataList* typeMetadata)
{
    /* Free type strings. */
    size_t i;
    for (i = 0; i < typeMetadata->count; ++i)
    {
        hlFree(typeMetadata->data[i].typeStr);
    }

    /* Free type metadata list. */
    HL_LIST_FREE(*typeMetadata);
}

static int hlINPACxV2SortFileMetadataRoot(const void* a, const void* b)
{
    const HlINPACxV2FileMetadata* fileMeta1 = (const HlINPACxV2FileMetadata*)a;
    const HlINPACxV2FileMetadata* fileMeta2 = (const HlINPACxV2FileMetadata*)b;
    const int typeSortWeight = ((int)fileMeta1->rootSortWeight -
        (int)fileMeta2->rootSortWeight);

    /* Sort by extensions and/or names if types are the same. */
    if (typeSortWeight == 0)
    {
        /* Sort by extensions. */
        {
            const int extSortWeight = hlNStrCmp(fileMeta1->ext, fileMeta2->ext);
            if (extSortWeight != 0) return extSortWeight;
        }
        
        /* Sort by names if extensions are the same. */
        return hlINPACxV2FileMetadataCompareNames(fileMeta1, fileMeta2);
    }

    /* Otherwise, sort by type weight. */
    return typeSortWeight;
}

static int hlINPACxV2SortFileMetadataPtrsSplit(const void* a, const void* b)
{
    const HlINPACxV2FileMetadata* fileMeta1 = *(const HlINPACxV2FileMetadata**)a;
    const HlINPACxV2FileMetadata* fileMeta2 = *(const HlINPACxV2FileMetadata**)b;
    const int typeSortWeight = ((int)fileMeta1->splitSortWeight -
        (int)fileMeta2->splitSortWeight);

    /* Sort by extensions and/or names if types are the same. */
    if (typeSortWeight == 0)
    {
        /* Sort by extensions. */
        {
            const int extSortWeight = hlNStrICmp(fileMeta1->ext, fileMeta2->ext);
            if (extSortWeight != 0) return extSortWeight;
        }

        /* Sort by priority if extensions are the same. */
        {
            const int prioritySortWeight = ((int)fileMeta2->priority -
                (int)fileMeta1->priority);

            if (prioritySortWeight != 0) return prioritySortWeight;
        }

        /* Sort by names if priorities are the same. */
        {
            const size_t fileNameLen1 = (fileMeta1->ext - fileMeta1->name);
            const size_t fileNameLen2 = (fileMeta2->ext - fileMeta2->name);

            if (fileNameLen1 == fileNameLen2)
            {
                return hlNStrNICmp(fileMeta1->name, fileMeta2->name, fileNameLen1);
            }
            else
            {
                const size_t fileNameMinLen = HL_MIN(fileNameLen1, fileNameLen2);
                const int nameSortWeight = hlNStrNICmp(fileMeta1->name,
                    fileMeta2->name, fileNameMinLen);

                if (nameSortWeight != 0)
                {
                    return nameSortWeight;
                }
                else
                {
                    return (fileNameMinLen == fileNameLen1) ?
                        (0 - (int)fileMeta2->name[fileNameLen2 - 1]) :
                        ((int)fileMeta1->name[fileNameLen1 - 1] - 0);
                }
            }
        }
    }

    /* Otherwise, sort by type weight. */
    return typeSortWeight;
}

static HlResult hlINPACxV2WriteNodeTree(HlU32 nodeCount, HlBINAEndianFlag endianFlag,
    HlOffTable* HL_RESTRICT offTable, HlStream* HL_RESTRICT stream)
{
    /* Generate node tree. */
    const size_t eofPos = hlStreamTell(stream);
    HlPACxV2NodeTree nodeTree =
    {
        nodeCount,                                      /* nodeCount */
        (HlU32)(eofPos + sizeof(HlPACxV2NodeTree))      /* nodes */
    };

    HlResult result;

    /* Swap node tree if necessary. */
    if (hlBINANeedsSwap(endianFlag))
    {
        hlPACxV2NodeTreeSwap(&nodeTree, HL_TRUE);
    }

    /* Write node tree. */
    result = hlStreamWrite(stream, sizeof(nodeTree), &nodeTree, NULL);
    if (HL_FAILED(result)) return result;

    /* Add nodes offset to offset table. */
    result = HL_LIST_PUSH(*offTable, eofPos +
        offsetof(HlPACxV2NodeTree, nodes));

    if (HL_FAILED(result)) return result;

    /* Write placeholder type nodes and return result. */
    return hlStreamWriteNulls(stream, sizeof(HlPACxV2NodeTree) * nodeCount, NULL);
}

static HlResult hlINPACxV2WriteFileData(
    const HlINPACxV2FileMetadata* HL_RESTRICT fileMetadata,
    HlBINAEndianFlag endianFlag, HlBool isHere, HlPackedFileIndex* HL_RESTRICT pfi,
    HlStrTable* HL_RESTRICT strTable, HlOffTable* HL_RESTRICT offTable,
    HlStream* HL_RESTRICT stream)
{
    HlPACxV2DataEntry dataEntry =
    {
        (HlU32)fileMetadata->entry->size,       /* dataSize */
        0,                                      /* unknown1 */
        0,                                      /* unknown2 */
        (isHere) ? HL_PACXV2_DATA_FLAGS_NONE :  /* flags */
            HL_PACXV2_DATA_FLAGS_NOT_HERE,
        0,                                      /* padding1 */
        0                                       /* padding2 */
    };

    const void* entryData;
    void* tmpEntryDataBuf = NULL;
    const size_t dataPos = (hlStreamTell(stream) + sizeof(HlPACxV2DataEntry));
    HlResult result;

    /* Get entry data pointer. */
    if (fileMetadata->entry->data == 0)
    {
        /* This entry is a file reference; load data into memory. */
        result = hlFileLoad(fileMetadata->entry->path, &tmpEntryDataBuf, NULL);
        if (HL_FAILED(result)) return result;

        entryData = tmpEntryDataBuf;
    }
    else
    {
        entryData = (const void*)((HlUPtr)fileMetadata->entry->data);
    }

    /* Merge data if necessary. */
    if (isHere && fileMetadata->pacxExt->flags == HL_PACX_EXT_FLAGS_V2_MERGED_TYPE)
    {
        /* Create a copy of the data if necessary so we can fix offsets and such. */
        if (!tmpEntryDataBuf)
        {
            void* copyOfEntryData = hlAlloc(dataEntry.dataSize);
            if (!copyOfEntryData)
            {
                result = HL_ERROR_OUT_OF_MEMORY;
                goto end;
            }

            memcpy(copyOfEntryData, entryData, dataEntry.dataSize);

            entryData = copyOfEntryData;
            tmpEntryDataBuf = copyOfEntryData;
        }

        if (hlBINAHasV2Header(entryData))
        {
            const HlBINAV2Header* header = (const HlBINAV2Header*)entryData;
            const HlBINAV2BlockDataHeader* dataBlock;

            /*
               Fix BINA header, data block, and offsets in temporary copy of data.

               NOTE: We won't be writing the BINA header or data block header to the
               pac file, and we will be replacing all of the offsets in the temporary
               data with the values they will need once written to the pac, so this is
               alright.
            */
            result = hlBINAV2Fix(tmpEntryDataBuf, dataEntry.dataSize);
            if (HL_FAILED(result)) goto end;

            /* Get BINA data block. */
            dataBlock = hlBINAV2GetDataBlock(tmpEntryDataBuf);

            /* If file has BINA data block, merge its offsets/strings. */
            if (dataBlock)
            {
                /* Get pointers. */
                const HlU8* dataPtr = HL_ADD_OFFC(dataBlock + 1, dataBlock->relativeDataOffset);
                const HlU8* strs = (const HlU8*)hlOff32Get(&dataBlock->stringTableOffset);
                const HlU32 strOffsPos = (HlU32)(strs - dataPtr);
                const HlU8* offsets = (strs + dataBlock->stringTableSize);
                const HlU8* eof = (offsets + dataBlock->offsetTableSize);
                const HlU8* curOffsetPos = offsets;
                const HlU32* curOffset = (const HlU32*)dataPtr;

                /* Set new data pointer. */
                entryData = dataPtr;

                /* Set new data size. */
                dataEntry.dataSize = (HlU32)(strs - dataPtr);

                /* Merge offsets/strings. */
                while (curOffsetPos < eof)
                {
                    size_t dstOffPos;
                    HlU32 srcOffPos, srcOffVal;

                    if (!hlBINAOffsetsNext(&curOffsetPos, &curOffset))
                        break;

                    srcOffPos = (HlU32)((const HlU8*)curOffset - dataPtr);
                    dstOffPos = (dataPos + srcOffPos);
                    srcOffVal = (HlU32)(((const HlU8*)hlOff32Get(curOffset)) - dataPtr);

                    if (srcOffVal >= strOffsPos)
                    {
                        /* Get string pointer. */
                        const char* str = (const char*)(dataPtr + srcOffVal);

                        /* Add string to string table. */
                        result = hlStrTableAddStrUTF8(strTable, str, dstOffPos);
                        if (HL_FAILED(result)) goto end;
                    }
                    else
                    {
                        /* Get destination offset pointer and compute destination offset value. */
                        HlU32* dstOff = (HlU32*)HL_ADD_OFF(entryData, srcOffPos);
                        HlU32 dstOffVal = (HlU32)(dataPos + srcOffVal);

                        /* Fix offset. */
                        *dstOff = (hlBINANeedsSwap(endianFlag)) ?
                            hlSwapU32(dstOffVal) : dstOffVal;

                        /* Add offset to offset table. */
                        result = HL_LIST_PUSH(*offTable, dstOffPos);
                        if (HL_FAILED(result)) goto end;
                    }
                }
            }
        }
        else
        {
            /* Backwards-compatibility with PacPack; check if this file has PacPack metadata. */
            const HlU8* dataPtr = (const HlU8*)entryData;
            const HlU8* pacPackMetadata = hlBINAGetPacPackMetadata(
                dataPtr, fileMetadata->entry->size);

            /* If file has PacPack metadata, merge its offsets/strings. */
            if (pacPackMetadata)
            {
                const HlU32* offsets = (const HlU32*)(pacPackMetadata + 16);
                const HlU8* strs;
                HlU32 offCount = *(offsets++), strOffsPos, i;

                /* Get new data size. */
                dataEntry.dataSize = (HlU32)(pacPackMetadata - dataPtr);

                /* Swap offset count if necessary. */
                if (hlBINANeedsSwap(endianFlag))
                {
                    hlSwapU32P(&offCount);
                }

                /* Get strings pointer and offset position. */
                strs = (const HlU8*)(&offsets[offCount]);
                strOffsPos = (HlU32)(strs - dataPtr);

                /* Merge offsets/strings. */
                for (i = 0; i < offCount; ++i)
                {
                    const HlU32* srcOff;
                    size_t dstOffPos;
                    HlU32 srcOffPos = offsets[i], srcOffVal;

                    /* Swap offset data as necessary. */
                    if (hlBINANeedsSwap(endianFlag))
                    {
                        hlSwapU32P(&srcOffPos);
                        srcOff = (const HlU32*)(dataPtr + srcOffPos);
                        srcOffVal = hlSwapU32(*srcOff);
                    }
                    else
                    {
                        srcOff = (const HlU32*)(dataPtr + srcOffPos);
                        srcOffVal = *srcOff;
                    }

                    dstOffPos = (dataPos + srcOffPos);

                    if (srcOffVal >= strOffsPos)
                    {
                        /* Get string pointer. */
                        const char* str = (const char*)(dataPtr + srcOffVal);

                        /* Add string to string table. */
                        result = hlStrTableAddStrUTF8(strTable, str, dstOffPos);
                        if (HL_FAILED(result)) goto end;
                    }
                    else
                    {
                        /* Get destination offset pointer and compute destination offset value. */
                        HlU32* dstOff = (HlU32*)HL_ADD_OFF(tmpEntryDataBuf, srcOffPos);
                        HlU32 dstOffVal = (HlU32)(dataPos + srcOffVal);

                        /* Fix offset. */
                        *dstOff = (hlBINANeedsSwap(endianFlag)) ?
                            hlSwapU32(dstOffVal) : dstOffVal;

                        /* Add offset to offset table. */
                        result = HL_LIST_PUSH(*offTable, dstOffPos);
                        if (HL_FAILED(result)) goto end;
                    }
                }
            }
        }
    }

    /* Write data entry to stream. */
    {
        const HlU32 dataSize = dataEntry.dataSize;

        /* Endian-swap data entry if necessary. */
        if (hlBINANeedsSwap(endianFlag))
        {
            hlPACxV2DataEntrySwap(&dataEntry);
        }

        /* Write data entry header to stream. */
        result = hlStreamWrite(stream, sizeof(dataEntry), &dataEntry, NULL);
        if (HL_FAILED(result)) goto end;

        /* Write data to stream if necessary. */
        if (isHere)
        {
            result = hlStreamWrite(stream, dataSize, entryData, NULL);
        }
    }

    /* Add packed file entry to packed file index if necessary. */
    if (pfi)
    {
#ifdef HL_IN_WIN32_UNICODE
        const size_t fileNameUTF8Size = hlStrGetReqLenNativeToUTF8(
            fileMetadata->name, 0);
#else
        const size_t fileNameUTF8Size = hlNStrLen(fileMetadata->name);
#endif

        /* Generate packed file entry. */
        HlPackedFileEntry packedEntry =
        {
            HL_ALLOC_ARR(char, fileNameUTF8Size),   /* name */
            (HlU32)dataPos,                         /* dataPos */
            dataEntry.dataSize                      /* dataSize */
        };

        if (!packedEntry.name)
        {
            result = HL_ERROR_OUT_OF_MEMORY;
            goto end;
        }

        /* Convert name to UTF-8 and copy into packed file entry name buffer. */
        if (!hlStrConvNativeToUTF8NoAlloc(fileMetadata->name,
            packedEntry.name, 0, fileNameUTF8Size))
        {
            hlPackedFileEntryDestruct(&packedEntry);
            goto end;
        }

        /* Add packed file entry to packed file index. */
        result = HL_LIST_PUSH(pfi->entries, packedEntry);
        if (HL_FAILED(result))
        {
            hlPackedFileEntryDestruct(&packedEntry);
            goto end;
        }
    }

end:
    /* Free data as necessary and return result. */
    hlFree(tmpEntryDataBuf);
    return result;
}

static HlResult hlINPACxV2WriteSplitTableData(const HlStrList* HL_RESTRICT splitNames,
    HlBINAEndianFlag endianFlag, HlStrTable* HL_RESTRICT strTable,
    HlOffTable* HL_RESTRICT offTable, HlStream* HL_RESTRICT stream)
{
    HlResult result;

    /* Write data entry header. */
    {
        const HlU32 dataSize = (HlU32)(sizeof(HlPACxV2SplitTable) +
            (sizeof(HL_OFF32_STR) * splitNames->count));

        HlPACxV2DataEntry dataEntry =
        {
            dataSize,                   /* dataSize */
            0,                          /* unknown1 */
            0,                          /* unknown2 */
            HL_PACXV2_DATA_FLAGS_NONE,  /* flags */
            0,                          /* padding1 */
            0                           /* padding2 */
        };

        /* Endian-swap data entry if necessary. */
        if (hlBINANeedsSwap(endianFlag))
        {
            hlPACxV2DataEntrySwap(&dataEntry);
        }

        /* Write data entry header. */
        result = hlStreamWrite(stream, sizeof(dataEntry), &dataEntry, NULL);
        if (HL_FAILED(result)) return result;
    }

    /* Write split table. */
    {
        /* Generate split table. */
        const size_t dataPos = hlStreamTell(stream);
        const size_t splitNamesOffPos = (dataPos +
            offsetof(HlPACxV2SplitTable, splitNames));

        HlPACxV2SplitTable splitTable =
        {
            (HlU32)(dataPos + sizeof(HlPACxV2SplitTable)),  /* splitNames */
            (HlU32)splitNames->count                        /* splitCount */
        };

        /* Endian-swap split table if necessary. */
        if (hlBINANeedsSwap(endianFlag))
        {
            hlPACxV2SplitTableSwap(&splitTable, HL_TRUE);
        }

        /* Write split table. */
        result = hlStreamWrite(stream, sizeof(splitTable), &splitTable, NULL);
        if (HL_FAILED(result)) return result;

        /* Add splitNames offset to offset table. */
        result = HL_LIST_PUSH(*offTable, splitNamesOffPos);
        if (HL_FAILED(result)) return result;
    }

    /* Write placeholder split names and return result. */
    return hlStreamWriteNulls(stream, sizeof(HL_OFF32_STR) * splitNames->count, NULL);
}

static HlResult hlINPACxV2ProxyEntryTableWrite(
    const HlINPACxV2TypeMetadataList* HL_RESTRICT typeMetadata,
    HlU32 proxyEntryCount, HlBINAEndianFlag endianFlag,
    HlStrTable* HL_RESTRICT strTable, HlOffTable* HL_RESTRICT offTable,
    HlStream* HL_RESTRICT stream)
{
    size_t curOffPos = hlStreamTell(stream);
    HlResult result;

    /* Write proxy entry table. */
    {
        /* Generate proxy entry table. */
        HlPACxV2ProxyEntryTable proxyEntryTable =
        {
            proxyEntryCount,                        /* proxyEntryCount */
            (HlU32)(curOffPos +                     /* proxyEntries */
                sizeof(HlPACxV2ProxyEntryTable))
        };

        /* Endian-swap proxy entry table if necessary. */
        if (hlBINANeedsSwap(endianFlag))
        {
            hlPACxV2ProxyEntryTableSwap(&proxyEntryTable, HL_TRUE);
        }

        /* Write proxy entry table. */
        result = hlStreamWrite(stream, sizeof(proxyEntryTable), &proxyEntryTable, NULL);
        if (HL_FAILED(result)) return result;

        /* Increase current offset position. */
        curOffPos += offsetof(HlPACxV2ProxyEntryTable, proxyEntries);

        /* Add proxyEntries offset to offset table. */
        result = HL_LIST_PUSH(*offTable, curOffPos);
        if (HL_FAILED(result)) return result;

        /* Increase current offset position. */
        curOffPos += 4;
    }

    /* Write proxy entries. */
    {
        size_t i;
        for (i = 0; i < typeMetadata->count; ++i)
        {
            const HlINPACxV2TypeMetadata* curTypeMetadata = &typeMetadata->data[i];
            size_t i2;

            for (i2 = 0; i2 < curTypeMetadata->fileCount; ++i2)
            {
                const HlINPACxV2FileMetadata* curFileMetadata = &curTypeMetadata->files[i2];

                /* Generate proxy entry. */
                HlPACxV2ProxyEntry proxyEntry =
                {
                    0,          /* type */
                    0,          /* name */
                    (HlU32)i2   /* nodeIndex */
                };

                /* Skip file if not a proxy entry. */
                if ((curFileMetadata->pacxExt->flags & HL_PACX_EXT_FLAGS_TYPE_MASK) ==
                    HL_PACX_EXT_FLAGS_ROOT_TYPE)
                {
                    continue;
                }

                /* Endian-swap proxy entry if necessary. */
                if (hlBINANeedsSwap(endianFlag))
                {
                    hlPACxV2ProxyEntrySwap(&proxyEntry, HL_FALSE);
                }

                /* Write proxy entry. */
                result = hlStreamWrite(stream, sizeof(proxyEntry), &proxyEntry, NULL);
                if (HL_FAILED(result)) return result;

                /* Add type string to string table. */
                result = hlStrTableAddStrRefUTF8(strTable,
                    curTypeMetadata->typeStr, curOffPos);

                if (HL_FAILED(result)) return result;

                /* Increase current offset position. */
                curOffPos += 4;

                /* Add name string to string table. */
                result = hlStrTableAddStrRefNativeEx(strTable, curFileMetadata->name,
                    curOffPos, (size_t)(curFileMetadata->ext - curFileMetadata->name));

                if (HL_FAILED(result)) return result;

                /* Increase current offset position. */
                curOffPos += 8;
            }
        }
    }

    return result;
}

static HlResult hlINPACxV2DataBlockWrite(unsigned short splitIndex,
    const HlINPACxV2TypeMetadataList* HL_RESTRICT typeMetadata,
    HlU32 splitLimit, HlU32 dataAlignment, HlBINAEndianFlag endianFlag,
    const HlStrList* HL_RESTRICT splitNames, HlPackedFileIndex* HL_RESTRICT pfi,
    HlStream* HL_RESTRICT stream)
{
    HlStrTable strTable, tmpMergedStrTable;
    HlOffTable offTable;
    const size_t treesPos = (sizeof(HlPACxV2Header) + sizeof(HlPACxV2BlockDataHeader));
    size_t i, curOffPos, typeNodeCount, dataEntriesPos, splitEntriesPos = 0, proxyTablePos;
    HlU32 proxyEntryCount = 0;
    HlResult result;
    const HlBool isRoot = (splitIndex == USHRT_MAX);

    /* Initialize string and offset tables. */
    HL_LIST_INIT(strTable);
    HL_LIST_INIT(tmpMergedStrTable);
    HL_LIST_INIT(offTable);

    /* Start writing data block. */
    result = hlPACxV2DataBlockStartWrite(stream);
    if (HL_FAILED(result)) goto failed;

    /* Compute required type node count. */
    if (isRoot)
    {
        typeNodeCount = typeMetadata->count;
    }
    else
    {
        typeNodeCount = 0;
        for (i = 0; i < typeMetadata->count; ++i)
        {
            const HlINPACxV2TypeMetadata* curTypeMetadata = &typeMetadata->data[i];
            size_t i2;

            if ((curTypeMetadata->files->pacxExt->flags &
                HL_PACX_EXT_FLAGS_SPLIT_TYPE) == 0)
            {
                continue;
            }

            for (i2 = 0; i2 < curTypeMetadata->fileCount; ++i2)
            {
                const HlINPACxV2FileMetadata* curFileMetadata = &curTypeMetadata->files[i2];
                if (curFileMetadata->splitIndex == splitIndex)
                {
                    ++typeNodeCount;
                    break;
                }
            }
        }
    }

    /* Write type tree and placeholder type nodes. */
    result = hlINPACxV2WriteNodeTree((HlU32)typeNodeCount,
        endianFlag, &offTable, stream);

    if (HL_FAILED(result)) goto failed;

    /* Write file trees and fill-in type nodes. */
    curOffPos = (treesPos + sizeof(HlPACxV2NodeTree));
    for (i = 0; i < typeMetadata->count; ++i)
    {
        const HlINPACxV2TypeMetadata* curTypeMetadata = &typeMetadata->data[i];
        const size_t fileTreePos = hlStreamTell(stream);
        size_t fileCount;

        /* Compute file count. */
        if (isRoot)
        {
            fileCount = curTypeMetadata->fileCount;
        }
        else
        {
            size_t i2;

            /* Skip non-split types. */
            if ((curTypeMetadata->files->pacxExt->flags &
                HL_PACX_EXT_FLAGS_SPLIT_TYPE) == 0)
            {
                continue;
            }

            /* Compute file count for this type and this split. */
            fileCount = 0;
            for (i2 = 0; i2 < curTypeMetadata->fileCount; ++i2)
            {
                const HlINPACxV2FileMetadata* curFileMetadata = &curTypeMetadata->files[i2];
                if (curFileMetadata->splitIndex == splitIndex)
                {
                    ++fileCount;
                }
            }
        }

        if (!fileCount) continue;

        /* Queue type string for writing. */
        result = hlStrTableAddStrRefUTF8(&strTable,
            curTypeMetadata->typeStr, curOffPos);

        if (HL_FAILED(result)) goto failed;

        /* Jump to type node data offset. */
        curOffPos += 4;
        result = hlStreamJumpTo(stream, curOffPos);
        if (HL_FAILED(result)) goto failed;

        /* Fill-in type node data offset. */
        result = hlStreamWriteOff32(stream, 0, fileTreePos,
            hlBINANeedsSwap(endianFlag), &offTable);

        if (HL_FAILED(result)) goto failed;

        /* Jump to file tree position (end of file). */
        result = hlStreamJumpTo(stream, fileTreePos);
        if (HL_FAILED(result)) goto failed;

        /* Increase current offset position. */
        curOffPos += 4;

        /* Write file tree and placeholder file nodes. */
        result = hlINPACxV2WriteNodeTree((HlU32)fileCount,
            endianFlag, &offTable, stream);

        if (HL_FAILED(result)) goto failed;
    }

    /* Write data entries and fill-in file nodes. */
    dataEntriesPos = hlStreamTell(stream);
    for (i = 0; i < typeMetadata->count; ++i)
    {
        const HlINPACxV2TypeMetadata* curTypeMetadata = &typeMetadata->data[i];
        size_t i2;
        HlBool accountedForNodeTree = HL_FALSE;

        /* If this is a split, skip types that aren't in this split. */
        if (!isRoot && (curTypeMetadata->files->pacxExt->flags &
            HL_PACX_EXT_FLAGS_SPLIT_TYPE) == 0)
        {
            continue;
        }

        for (i2 = 0; i2 < curTypeMetadata->fileCount; ++i2)
        {
            const HlINPACxV2FileMetadata* curFileMetadata = &curTypeMetadata->files[i2];
            size_t dataEntryPos;

            if (!isRoot && curFileMetadata->splitIndex != splitIndex)
                continue;

            /* Increase current offset position. */
            if (!accountedForNodeTree)
            {
                curOffPos += sizeof(HlPACxV2NodeTree);
                accountedForNodeTree = HL_TRUE;
            }

            /* Queue file name string for writing. */
            {
                const HlNChar* ext = (curFileMetadata->entry) ?
                    curFileMetadata->ext : hlPathGetExts(curFileMetadata->name);

                result = hlStrTableAddStrRefNativeEx(&strTable, curFileMetadata->name,
                    curOffPos, (size_t)(ext - curFileMetadata->name));

                if (HL_FAILED(result)) goto failed;
            }

            /* Pad data entry to requested data alignment. */
            result = hlStreamPad(stream, dataAlignment);
            if (HL_FAILED(result)) goto failed;

            /* Get data entry position. */
            dataEntryPos = hlStreamTell(stream);

            /* Jump to type node data offset. */
            curOffPos += 4;
            result = hlStreamJumpTo(stream, curOffPos);
            if (HL_FAILED(result)) goto failed;

            /* Fill-in file node data offset. */
            result = hlStreamWriteOff32(stream, 0, dataEntryPos,
                hlBINANeedsSwap(endianFlag), &offTable);

            if (HL_FAILED(result)) goto failed;

            /* Jump to data entry position. */
            result = hlStreamJumpTo(stream, dataEntryPos);
            if (HL_FAILED(result)) goto failed;

            /* Increase current offset position. */
            curOffPos += 4;

            /* Write data entry. */
            if (curFileMetadata->entry)
            {
                /* TODO: Let user optionally write mixed types to root. */
                const HlBool isHere = ((!isRoot) ? HL_TRUE : (!splitLimit ||
                    ((curFileMetadata->pacxExt->flags & HL_PACX_EXT_FLAGS_TYPE_MASK) ==
                    HL_PACX_EXT_FLAGS_ROOT_TYPE)));

                result = hlINPACxV2WriteFileData(curFileMetadata,
                    endianFlag, isHere, pfi, &tmpMergedStrTable,
                    &offTable, stream);

                if (!isHere) ++proxyEntryCount;
            }

            /* Write split table data and placeholder split entries. */
            else
            {
                /* Store split entries position for later. */
                splitEntriesPos = (dataEntryPos + sizeof(HlPACxV2DataEntry) +
                    sizeof(HlPACxV2SplitTable));

                /* Write split table data and placeholder split entries. */
                result = hlINPACxV2WriteSplitTableData(splitNames,
                    endianFlag, &strTable, &offTable, stream);
            }

            if (HL_FAILED(result)) goto failed;
        }
    }

    /* Fill-in split entries and write proxy entry table if necessary. */
    proxyTablePos = hlStreamTell(stream);
    if (isRoot && splitLimit)
    {
        /* Fill-in split entries if necessary. */
        if (splitEntriesPos)
        {
            curOffPos = splitEntriesPos;
            for (i = 0; i < splitNames->count; ++i)
            {
                /* Fill-in split entry. */
                result = hlStrTableAddStrRefUTF8(&strTable,
                    splitNames->data[i], curOffPos);

                if (HL_FAILED(result)) goto failed;

                /* Increase current offset position. */
                curOffPos += 4;
            }
        }

        /* Write proxy entry table if necessary. */
        if (proxyEntryCount)
        {
            result = hlINPACxV2ProxyEntryTableWrite(typeMetadata,
                proxyEntryCount, endianFlag, &strTable, &offTable,
                stream);

            if (HL_FAILED(result)) goto failed;
        }
    }

    /* Add contents of temporary merged string table to final string table if necessary. */
    if (isRoot)
    {
        const size_t prevStrTableCount = strTable.count;

        /* Reserve enough space to merge string tables together. */
        result = HL_LIST_RESERVE(strTable, strTable.count +
            tmpMergedStrTable.count);

        if (HL_FAILED(result)) goto failed;

        /* Merge contents of temporary merged string table into final string table. */
        for (i = 0; i < tmpMergedStrTable.count; ++i)
        {
            result = HL_LIST_PUSH(strTable, tmpMergedStrTable.data[i]);
            if (HL_FAILED(result))
            {
                /* HACK: Set count to previous value so strings don't get freed twice. */
                strTable.count = prevStrTableCount;
                goto failed;
            }
        }

        /*
           HACK: Set count to 0 so strings don't get freed twice.
           List data will still be freed correctly.
        */
        tmpMergedStrTable.count = 0;
    }

    /* Finish writing root data block. */
    result = hlPACxV2DataBlockFinishWrite(0,            /* headerPos */
        sizeof(HlPACxV2Header),                         /* dataBlockPos */
        (HlU32)(dataEntriesPos - treesPos),             /* treesSize */
        (HlU32)(proxyTablePos - dataEntriesPos),        /* dataEntriesSize */
        (HlU32)(hlStreamTell(stream) - proxyTablePos),  /* proxyTableSize */
        endianFlag, &strTable, &offTable, stream);      /* (self-explanatory) */

    if (HL_FAILED(result)) goto failed;

failed:
    /* Free resources and return result. */
    HL_LIST_FREE(offTable);
    hlStrTableDestruct(&strTable);
    hlStrTableDestruct(&tmpMergedStrTable);

    return result;
}

static HlResult hlINPACxV2SaveSplits(unsigned short splitCount,
    const HlINPACxV2TypeMetadataList* HL_RESTRICT typeMetadata,
    size_t filePathLen, HlU32 splitLimit, HlBINAEndianFlag endianFlag,
    HlU32 dataAlignment, HlPackedFileIndex* HL_RESTRICT pfi,
    HlStrList* HL_RESTRICT splitNames, HlNChar* HL_RESTRICT pathBuf)
{
    HlFileStream* splitFile;
    const HlNChar* rootName = hlPathGetName(pathBuf);
    HlNChar* lastCharPtr = &pathBuf[filePathLen + 2];
    const size_t rootNameSize = (hlNStrLen(rootName) + 1);
    const size_t splitNameSize = (hlStrGetReqLenNativeToUTF8(rootName, rootNameSize) + 3);
    unsigned short splitIndex;
    HlResult result;

    /* Reserve space in split name list. */
    result = HL_LIST_RESERVE(*splitNames, splitCount);
    if (HL_FAILED(result)) return result;

    /* Copy initial split extension into path buffer. */

    /*
       HACK: '/' == 47, '0' == 48. ".0/" will be incremented to ".00"
       in the first call to hlINArchiveNextSplit2.
    */
    memcpy(&pathBuf[filePathLen], HL_NTEXT(".0/"), sizeof(HlNChar) * 4);

    /* Save splits as needed. */
    for (splitIndex = 0; splitIndex < splitCount; ++splitIndex)
    {
        char* splitName;

        /* Get the next split file path. */
        if (!hlINArchiveNextSplit2(lastCharPtr))
            return HL_ERROR_OUT_OF_RANGE;

        /* Open the next split file for writing. */
        result = hlFileStreamOpen(pathBuf, HL_FILE_MODE_WRITE, &splitFile);
        if (HL_FAILED(result)) return result;

        /* Start writing split header. */
        result = hlPACxV2StartWrite(endianFlag, splitFile);
        if (HL_FAILED(result))
        {
            hlFileStreamClose(splitFile);
            return result;
        }

        /* Write data block. */
        result = hlINPACxV2DataBlockWrite(splitIndex, typeMetadata,
            splitLimit, dataAlignment, endianFlag, splitNames,
            pfi, splitFile);

        if (HL_FAILED(result))
        {
            hlFileStreamClose(splitFile);
            return result;
        }

        /* Finish writing split header. */
        result = hlPACxV2FinishWrite(0, 1, endianFlag, splitFile);
        if (HL_FAILED(result))
        {
            hlFileStreamClose(splitFile);
            return result;
        }

        /* Close split file. */
        result = hlFileStreamClose(splitFile);
        if (HL_FAILED(result)) return result;

        /* Allocate new buffer for split name. */
        splitName = HL_ALLOC_ARR(char, splitNameSize);
        if (!splitName) return HL_ERROR_OUT_OF_MEMORY;

        /* Convert split name to UTF-8 and copy into split name buffer. */
        if (!hlStrConvNativeToUTF8NoAlloc(rootName, splitName,
            rootNameSize + 3, splitNameSize))
        {
            hlFree(splitName);
            return HL_ERROR_UNKNOWN;
        }

        /* Add split file name to split names list. */
        result = HL_LIST_PUSH(*splitNames, splitName);
        if (HL_FAILED(result))
        {
            hlFree(splitName);
            return result;
        }
    }

    return result;
}

static HlResult hlINPACxV2FileMetadataSplitUp(HlINPACxV2TypeMetadataList* HL_RESTRICT typeMetadata,
    HlINPACxV2FileMetadata* HL_RESTRICT fileMetadata, size_t fileMetadataCount,
    HlU32 splitLimit, HlU32 dataAlignment, unsigned short* HL_RESTRICT splitCount)
{
    HlU32 splitDataEntriesSizes[100] = { 0 };
    HlINPACxV2FileMetadata** filesSortedForSplits;
    size_t i;
    unsigned short splitIndex = 0;
    HlResult result = HL_RESULT_SUCCESS;

    /* Allocate buffer to sort file metadata for splits. */
    filesSortedForSplits = HL_ALLOC_ARR(HlINPACxV2FileMetadata*, fileMetadataCount);
    if (!filesSortedForSplits) return HL_ERROR_OUT_OF_MEMORY;

    /* Put pointers to fileMetadata in filesSortedForSplits. */
    for (i = 0; i < fileMetadataCount; ++i)
    {
        filesSortedForSplits[i] = &fileMetadata[i];
    }

    /* Sort filesSortedForSplits. */
    qsort(filesSortedForSplits, fileMetadataCount,
        sizeof(HlINPACxV2FileMetadata*),
        hlINPACxV2SortFileMetadataPtrsSplit);

    /* Place files in splits. */
    for (i = 0; i < fileMetadataCount; ++i)
    {
        HlINPACxV2FileMetadata* curFileMetadata = filesSortedForSplits[i];
        HlU32 dataEntrySize;

        if ((curFileMetadata->pacxExt->flags & HL_PACX_EXT_FLAGS_TYPE_MASK) ==
            HL_PACX_EXT_FLAGS_ROOT_TYPE)
        {
            break;
        }

        dataEntrySize = (HlU32)curFileMetadata->entry->size;

        /* Check if adding this file to the current split would exceed the split limit. */
        if ((splitDataEntriesSizes[splitIndex] + dataEntrySize) > splitLimit &&
            splitDataEntriesSizes[splitIndex] != 0)
        {
            /* Increase split index and ensure we haven't exceeded 99 splits. */
            if (++splitIndex > 99)
            {
                result = HL_ERROR_OUT_OF_RANGE;
                goto end;
            }
        }

        /* Account for data entry size. */
        splitDataEntriesSizes[splitIndex] += dataEntrySize;

        /* Set splitIndex. */
        curFileMetadata->splitIndex = splitIndex;
    }

end:
    *splitCount = (splitIndex + 1);
    hlFree(filesSortedForSplits);
    return result;
}

HlResult hlPACxV2SaveEx(const HlArchive* HL_RESTRICT arc,
    HlU32 splitLimit, HlU32 dataAlignment, HlBINAEndianFlag endianFlag,
    const HlPACxSupportedExt* HL_RESTRICT exts, const size_t extCount,
    HlPackedFileIndex* HL_RESTRICT pfi, const HlNChar* HL_RESTRICT filePath)
{
    HlNChar pathBuf[255];
    HlNChar* pathBufPtr = pathBuf;
    size_t pathBufCap = 255;

    HlINPACxV2TypeMetadataList typeMetadata;
    HlINPACxV2FileMetadata* fileMetadata = NULL;
    HlFileStream* rootFile = NULL;
    const size_t filePathLen = hlNStrLen(filePath);
    size_t fileMetadataCount = 0;
    unsigned short splitCount = 0;
    HlResult result = HL_RESULT_SUCCESS;

    /* Verify that dataAlignment is a multiple of 4. */
    if ((dataAlignment % 4) != 0) return HL_ERROR_INVALID_ARGS;

    /* Setup path buffer. */
    {
        const size_t filePathSize = (filePathLen + 1);
        const size_t reqPathBufSize = (splitLimit) ?
            (filePathSize + 3) :    /* Account for path + split extension. */
            filePathSize;           /* Account for path. */

        /* Reallocate path buffer if necessary. */
        if (reqPathBufSize > pathBufCap)
        {
            pathBufPtr = HL_ALLOC_ARR(HlNChar, reqPathBufSize);
            if (!pathBufPtr) return HL_ERROR_OUT_OF_MEMORY;
        }

        /* Copy filePath and null terminator into path buffer. */
        memcpy(pathBufPtr, filePath, sizeof(HlNChar) * filePathSize);
    }

    /* Generate file and type metadata. */
    HL_LIST_INIT(typeMetadata);
    if (arc->entries.count > 0)
    {
        HlINPACxV2TypeMetadata curTypeMetadata;
        size_t i;
        HlBool needsSplits = HL_FALSE;

        /* Allocate file metadata array. */
        fileMetadata = HL_ALLOC_ARR(HlINPACxV2FileMetadata, arc->entries.count +
            ((splitLimit) ? 1 : 0));

        if (!fileMetadata) goto failed_root_not_open;

        /* Generate file metadata. */
        for (i = 0; i < arc->entries.count; ++i)
        {
            HlINPACxV2FileMetadata* curFileMetadata = &fileMetadata[fileMetadataCount];
            const HlNChar* ext;

            /* Set entry pointer. */
            curFileMetadata->entry = &arc->entries.data[i];

            /* Skip streaming and directory entries. */
            if (!hlArchiveEntryIsRegularFile(curFileMetadata->entry))
                continue;

            /* Set name and extension pointers. */
            curFileMetadata->name = hlArchiveEntryGetName(curFileMetadata->entry);
            curFileMetadata->ext = ext = hlPathGetExts(curFileMetadata->name);

            /* Skip dot in extension if necessary. */
            if (*ext == HL_NTEXT('.')) ++ext;

            /* Get PACx extension and sort weights. */
            curFileMetadata->pacxExt = hlINPACxGetSupportedExt(
                ext, exts, extCount);

            curFileMetadata->rootSortWeight = curFileMetadata->pacxExt->rootSortWeight;
            curFileMetadata->splitSortWeight = curFileMetadata->pacxExt->splitSortWeight;

            /* Skip ResPacDepend files. */
            if (curFileMetadata->pacxExt->pacxDataTypeIndex == ResPacDepend)
                continue;

            /* Set default splitIndex and priority. */
            curFileMetadata->splitIndex = USHRT_MAX;
            curFileMetadata->priority = 0;

            /* Check if we need splits. */
            /* TODO: Let user write mixed types to root. */
            if (splitLimit && curFileMetadata->pacxExt->flags & HL_PACX_EXT_FLAGS_SPLIT_TYPE)
            {
                needsSplits = HL_TRUE;
            }

            /* Increase file metadata count. */
            ++fileMetadataCount;
        }

        if (needsSplits)
        {
            HlINPACxV2FileMetadata* curFileMetadata = &fileMetadata[fileMetadataCount];

            /* Get pointers. */
            curFileMetadata->entry = NULL;
            curFileMetadata->name = hlPathGetName(pathBufPtr);
            curFileMetadata->ext = HL_NTEXT("pac.d");

            /* Get PACx extension and sort weights. */
            curFileMetadata->pacxExt = hlINPACxGetSupportedExt(
                curFileMetadata->ext, exts, extCount);

            curFileMetadata->rootSortWeight = curFileMetadata->pacxExt->rootSortWeight;
            curFileMetadata->splitSortWeight = curFileMetadata->pacxExt->splitSortWeight;

            /* Set default splitIndex and priority. */
            curFileMetadata->splitIndex = USHRT_MAX;
            curFileMetadata->priority = 0;

            /* Increase file metadata count. */
            ++fileMetadataCount;
        }

        if (fileMetadataCount)
        {
            /* Sort file metadata based on type indices. */
            qsort(fileMetadata, fileMetadataCount,
                sizeof(HlINPACxV2FileMetadata),
                hlINPACxV2SortFileMetadataRoot);

            /* Setup type metadata for first type. */
            curTypeMetadata.typeStr = hlINPACxV2CreateTypeStr(&fileMetadata[0]);
            curTypeMetadata.files = &fileMetadata[0];
            curTypeMetadata.fileCount = 1;

            if (!curTypeMetadata.typeStr)
            {
                result = HL_ERROR_UNKNOWN;
                goto failed_root_not_open;
            }

            /* Setup and add type metadata for subsequent types. */
            for (i = 1; i < fileMetadataCount; ++i)
            {
                if (hlNStrsEqual(curTypeMetadata.files->ext, fileMetadata[i].ext))
                {
                    ++curTypeMetadata.fileCount;
                }
                else
                {
                    /* Add current type metadata to type metadata list. */
                    result = HL_LIST_PUSH(typeMetadata, curTypeMetadata);
                    if (HL_FAILED(result))
                    {
                        hlFree(curTypeMetadata.typeStr);
                        goto failed_root_not_open;
                    }

                    /* Setup type metadata for next type. */
                    curTypeMetadata.typeStr = hlINPACxV2CreateTypeStr(&fileMetadata[i]);
                    curTypeMetadata.files = &fileMetadata[i];
                    curTypeMetadata.fileCount = 1;

                    if (!curTypeMetadata.typeStr)
                    {
                        result = HL_ERROR_UNKNOWN;
                        goto failed_root_not_open;
                    }
                }
            }

            /* Add final type metadata to type metadata list. */
            result = HL_LIST_PUSH(typeMetadata, curTypeMetadata);
            if (HL_FAILED(result))
            {
                hlFree(curTypeMetadata.typeStr);
                goto failed_root_not_open;
            }

            /* Set file priorities. */
            result = hlINPACxV2TypeMetadatalistSetFilePriorities(
                &typeMetadata, endianFlag);

            if (HL_FAILED(result)) goto failed_root_not_open;
        }

        /* Split data up. */
        if (needsSplits)
        {
            result = hlINPACxV2FileMetadataSplitUp(&typeMetadata,
                fileMetadata, fileMetadataCount, splitLimit,
                dataAlignment, &splitCount);

            if (HL_FAILED(result)) goto failed_root_not_open;
        }
    }

    /* Open root file. */
    result = hlFileStreamOpen(pathBufPtr, HL_FILE_MODE_WRITE, &rootFile);
    if (HL_FAILED(result)) goto failed_root_not_open;

    /* Start writing root header. */
    result = hlPACxV2StartWrite(endianFlag, rootFile);
    if (HL_FAILED(result)) goto failed;

    /* Write splits and root data block if necessary. */
    if (fileMetadataCount)
    {
        HlStrList splitNames;
        HL_LIST_INIT(splitNames);

        /* Save splits if necessary. */
        if (splitCount)
        {
            /* Disable PFI generation if we're writing splits. */
            pfi = NULL;

            /* Save splits. */
            result = hlINPACxV2SaveSplits(splitCount, &typeMetadata,
                filePathLen, splitLimit, endianFlag, dataAlignment,
                pfi, &splitNames, pathBufPtr);

            if (HL_FAILED(result))
            {
                hlStrListDestruct(&splitNames);
                goto failed;
            }
        }

        /* Write root data block. */
        result = hlINPACxV2DataBlockWrite(USHRT_MAX, &typeMetadata,
            splitLimit, dataAlignment, endianFlag, &splitNames,
            pfi, rootFile);

        hlStrListDestruct(&splitNames);
        if (HL_FAILED(result)) goto failed;
    }

    /* Finish writing root header. */
    result = hlPACxV2FinishWrite(0, (fileMetadataCount) ? 1 : 0,
        endianFlag, rootFile);

    if (HL_FAILED(result)) goto failed;

    /* Close root file and return result. */
    result = hlFileStreamClose(rootFile);
    goto end;

failed:
    /* Free everything as necessary and return result. */
    hlFileStreamClose(rootFile);

end:
failed_root_not_open:
    hlFree(fileMetadata);
    hlINPACxV2TypeMetadataListFree(&typeMetadata);

    if (pathBufPtr != pathBuf) hlFree(pathBufPtr);
    return result;
}

void hlPACxV3Fix(void* rawData)
{
    HlPACxV3Header* header = (HlPACxV3Header*)rawData;
    if (hlBINANeedsSwap(header->endianFlag))
    {
        /* TODO: Swap header. */
        HL_ASSERT(HL_FALSE);
    }

    /* Fix offsets. */
    {
        const void* offsets = hlPACxV3GetOffsetTable(rawData);
        hlBINAOffsetsFix64(offsets, header->endianFlag,
            header->offsetTableSize, rawData);
    }

    if (hlBINANeedsSwap(header->endianFlag))
    {
        HlPACxV3NodeTree* typeTree = (HlPACxV3NodeTree*)hlPACxV3GetTypeTree(rawData);
        /* TODO: Endian swap EVERYTHING if necessary. */
        /* TODO: Support endian swapping "v4" stuff as well. */
    }
}

const HlPACxV3Node* hlPACxV3GetChildNode(const HlPACxV3Node* node,
    const HlPACxV3Node* nodes, const char* HL_RESTRICT name)
{
    const HlS32* childIndices = (const HlS32*)hlOff64Get(&node->childIndices);
    HlU16 i = 0, childCount = node->childCount;

    while (i < childCount)
    {
        const HlPACxV3Node* childNode = &nodes[childIndices[i]];
        const char* nodeName = (const char*)hlOff64Get(&childNode->name);
        size_t nodeNameLen;

        if (childNode->hasData) return childNode;
        if (!nodeName) continue;

        nodeNameLen = strlen(nodeName);

        if (!strncmp(name, nodeName, nodeNameLen))
        {
            /* Increase name pointer. */
            name += nodeNameLen;

            /* If the entire name matched, return the node pointer. */
            if (*name == '\0')
            {
                return childNode;
            }

            /* Start looping through this child node's children instead. */
            childIndices = (const HlS32*)hlOff64Get(&childNode->childIndices);
            childCount = childNode->childCount;
            i = 0;
        }
        else
        {
            /*
               TODO: Optimize this by returning NULL early if the first
               character of nodeName is greater than the first character of name.
            */
        }

        ++i;
    }

    return NULL;
}

const HlPACxV3Node* hlPACxV3GetNode(
    const HlPACxV3NodeTree* HL_RESTRICT nodeTree,
    const char* HL_RESTRICT name)
{
    const HlPACxV3Node* nodes = (const HlPACxV3Node*)hlOff64Get(&nodeTree->nodes);
    HlU32 i = 0;

    for (i = 0; i < nodeTree->nodeCount; ++i)
    {
        const char* nodeName = (const char*)hlOff64Get(&nodes[i].name);
        size_t nodeNameLen;

        if (nodes[i].hasData) return NULL;
        if (!nodeName) continue;

        nodeNameLen = strlen(nodeName);

        if (!strncmp(name, nodeName, nodeNameLen))
        {
            /* Increase name pointer. */
            name += nodeNameLen;

            /* Recurse through child nodes. */
            return hlPACxV3GetChildNode(&nodes[i], nodes, name);
        }
        else
        {
            /*
               TODO: Optimize this by returning NULL early if the first
               character of nodeName is greater than the first character of name.
            */
        }
    }

    return NULL;
}

const HlPACxV3SplitTable* hlPACxV3GetSplitTable(const void* rawData)
{
    return (const HlPACxV3SplitTable*)HL_ADD_OFFC(          /* splitTable = */
        hlPACxV3GetTypeTree(rawData),                       /* (typeTree + */
        ((const HlPACxV3Header*)rawData)->treesSize);       /* treesSize) */
}

const HlPACxV3DataEntry* hlPACxV3GetDataEntries(const void* rawData)
{
    return (const HlPACxV3DataEntry*)HL_ADD_OFFC(           /* dataEntries = */
        hlPACxV3GetSplitTable(rawData),                     /* (splitTable + */
        ((const HlPACxV3Header*)rawData)->splitTableSize);  /* splitTableSize) */
}

const char* hlPACxV3GetStringTable(const void* rawData)
{
    return (const char*)HL_ADD_OFFC(                        /* stringTable = */
        hlPACxV3GetDataEntries(rawData),                    /* (dataEntries + */
        ((const HlPACxV3Header*)rawData)->dataEntriesSize); /* dataEntriesSize) */
}

const void* hlPACxV3GetFileData(const void* rawData)
{
    return HL_ADD_OFFC(hlPACxV3GetStringTable(rawData),     /* fileData = (stringTable + */
        ((const HlPACxV3Header*)rawData)->stringTableSize); /* stringTableSize) */
}

const HlU8* hlPACxV3GetOffsetTable(const void* rawData)
{
    return HL_ADD_OFFC(hlPACxV3GetFileData(rawData),        /* offsetTable = (fileData + */
        ((const HlPACxV3Header*)rawData)->fileDataSize);    /* fileDataSize) */
}

static HlResult hlINPACxV3FileNodesSetupEntries(const HlPACxV3Node* fileNodes,
    const HlPACxV3Node* curFileNode, HlBool skipProxies,
    char* HL_RESTRICT pathBuf, HlArchive* HL_RESTRICT hlArc)
{
    const HlS32* childIndices = (const HlS32*)hlOff64Get(&curFileNode->childIndices);
    HlU16 i;
    HlResult result;

    if (curFileNode->hasData)
    {
        const HlPACxV3DataEntry* dataEntry = (const HlPACxV3DataEntry*)
            hlOff64Get(&curFileNode->data);

        /* Skip proxies if requested. */
        if (!skipProxies || dataEntry->dataType != HL_PACXV3_DATA_TYPE_NOT_HERE)
        {
            HlArchiveEntry entry;
            const char* ext = (const char*)hlOff64Get(&dataEntry->extension);
            size_t nameLen, extSize, pathBufSize;

            /* Ensure node name length is > 0. */
            if (!curFileNode->bufStartIndex) return HL_ERROR_INVALID_DATA;

            /* Get required length for native path buffer. */
            nameLen = hlStrGetReqLenUTF8ToNative(pathBuf,
                curFileNode->bufStartIndex);

            if (!nameLen) return HL_ERROR_INVALID_DATA;

            extSize = hlStrGetReqLenUTF8ToNative(ext, 0);
            if (!extSize) return HL_ERROR_INVALID_DATA;

            /* Allocate native path buffer, convert fileName, and store the result. */

            /*
              NOTE: We add one to account for the dot in the extension.
              The null-terminator is already accounted for by hlStrGetReqLenUTF8ToNative.
            */
            pathBufSize = (nameLen + extSize + 1);
            entry.path = HL_ALLOC_ARR(HlNChar, pathBufSize);

            if (!entry.path) return HL_ERROR_OUT_OF_MEMORY;

            if (!hlStrConvUTF8ToNativeNoAlloc(pathBuf, entry.path,
                curFileNode->bufStartIndex, pathBufSize))
            {
                hlFree(entry.path);
                return HL_ERROR_UNKNOWN;
            }

            /* Convert extension to native encoding and copy into buffer. */
            entry.path[nameLen] = HL_NTEXT('.');

            if (!hlStrConvUTF8ToNativeNoAlloc(ext,
                &entry.path[nameLen + 1], 0, pathBufSize -
                (nameLen + 1)))
            {
                hlFree(entry.path);
                return HL_ERROR_UNKNOWN;
            }

            /* Replace extension dot with null terminator if file has no extension. */
            if (entry.path[nameLen + 1] == HL_NTEXT('\0'))
            {
                entry.path[nameLen] = HL_NTEXT('\0');
            }

            /* Setup rest of archive entry and copy data if necessary. */
            entry.size = (size_t)dataEntry->dataSize;

            if (dataEntry->dataType != HL_PACXV3_DATA_TYPE_NOT_HERE)
            {
                /* Allocate data buffer. */
                void* dstDataBuf = hlAlloc(entry.size);
                if (!dstDataBuf)
                {
                    hlFree(dstDataBuf);
                    hlFree(entry.path);
                    return HL_ERROR_OUT_OF_MEMORY;
                }

                /* Copy data. */
                memcpy(dstDataBuf, hlOff64Get(&dataEntry->data), entry.size);

                /* Set meta and data within archive entry. */
                entry.meta = 0;
                entry.data = (HlUMax)((HlUPtr)dstDataBuf);
            }
            else
            {
                /* Set meta and data within archive entry. */
                entry.meta = HL_ARC_ENTRY_STREAMING_FLAG;
                entry.data = (HlUMax)((HlUPtr)NULL); /* TODO: Set this to something else? */
            }

            /* Add entry to archive. */
            result = HL_LIST_PUSH(hlArc->entries, entry);
            if (HL_FAILED(result))
            {
                hlArchiveEntryDestruct(&entry);
                return result;
            }
        }
    }
    else if (curFileNode->name)
    {
        /* Copy name into path buffer. */
        strcpy(&pathBuf[curFileNode->bufStartIndex],
            (const char*)hlOff64Get(&curFileNode->name));
    }

    /* Recurse through children. */
    for (i = 0; i < curFileNode->childCount; ++i)
    {
        result = hlINPACxV3FileNodesSetupEntries(fileNodes,
            &fileNodes[childIndices[i]], skipProxies, pathBuf,
            hlArc);

        if (HL_FAILED(result)) return result;
    }

    return HL_RESULT_SUCCESS;
}

static HlResult hlINPACxV3ParseInto(const HlPACxV3Header* HL_RESTRICT pac,
    HlBool skipProxies, HlArchive* HL_RESTRICT hlArc)
{
    /* Get pointers. */
    char pathBuf[256]; /* PACxV3 names are hard-limited to 255, not including null terminator. */
    const HlPACxV3NodeTree* typeTree = hlPACxV3GetTypeTree(pac);
    const HlPACxV3Node* typeNodes = (const HlPACxV3Node*)hlOff64Get(&typeTree->nodes);
    const HlS32* typeDataIndices = (const HlS32*)hlOff64Get(&typeTree->dataNodeIndices);
    HlU64 i;
    HlResult result = HL_RESULT_SUCCESS;

    /* Setup archive entries. */
    for (i = 0; i < typeTree->dataNodeCount; ++i)
    {
        /* Setup archive entries. */
        const HlPACxV3NodeTree* fileTree = (const HlPACxV3NodeTree*)
            hlOff64Get(&typeNodes[typeDataIndices[i]].data);

        const HlPACxV3Node* fileNodes = (const HlPACxV3Node*)
            hlOff64Get(&fileTree->nodes);

        result = hlINPACxV3FileNodesSetupEntries(fileNodes,
            fileNodes, skipProxies, pathBuf, hlArc);

        if (HL_FAILED(result)) return result;
    }

    return result;
}

HlResult hlPACxV3ParseInto(const HlPACxV3Header* HL_RESTRICT pac,
    HlBool skipProxies, HlArchive* HL_RESTRICT hlArc)
{
    return hlINPACxV3ParseInto(pac, skipProxies, hlArc);
}

HlResult hlPACxV3ReadInto(void* HL_RESTRICT rawData,
    HlBool skipProxies, HlArchive* HL_RESTRICT hlArc)
{
    /* Fix PACxV3 data. */
    hlPACxV3Fix(rawData);

    /* Parse PACxV3 data into HlArchive and return result. */
    return hlPACxV3ParseInto((const HlPACxV3Header*)rawData,
        skipProxies, hlArc);
}

HlResult hlPACxV3LoadSingleInto(const HlNChar* HL_RESTRICT filePath,
    HlBool skipProxies, HlBlobList* HL_RESTRICT pacs,
    HlArchive* HL_RESTRICT hlArc)
{
    HlBlob* pac;
    HlResult result;

    /* Load archive. */
    result = hlBlobLoad(filePath, &pac);
    if (HL_FAILED(result)) return result;

    /* Fix PACxV3 data. */
    hlPACxV3Fix(pac->data);

    /* Add this archive's blob to the blobs list if necessary. */
    if (pacs)
    {
        result = HL_LIST_PUSH(*pacs, pac);
        if (HL_FAILED(result))
        {
            hlBlobFree(pac);
            return result;
        }
    }

    /* Parse blob into HlArchive, free blob if necessary, and return. */
    if (hlArc)
    {
        result = hlPACxV3ParseInto(
            (const HlPACxV3Header*)pac->data,
            skipProxies, hlArc);
    }

    if (!pacs) hlBlobFree(pac);
    return result;
}

static HlResult hlINPACxV3LoadAllInto(
    HlBlob* HL_RESTRICT rootPac, HlBool loadSplits,
    HlNChar* pathBuf, HlNChar* HL_RESTRICT * HL_RESTRICT pathBufPtr,
    size_t* HL_RESTRICT pathBufCap, HlBlobList* HL_RESTRICT pacs,
    HlArchive* HL_RESTRICT hlArc)
{
    const HlPACxV3Header* header;
    const HlPACxV3SplitTable* splitTable;
    const HlPACxV3SplitEntry* splitEntries;
    size_t dirLen;
    HlU64 i;
    HlResult result = HL_RESULT_SUCCESS;

    /* Fix root PACxV3 data. */
    hlPACxV3Fix(rootPac->data);

    /* Get header pointer. */
    header = (const HlPACxV3Header*)rootPac->data;

    /* Add this archive's blob to the blobs list if necessary. */
    if (pacs)
    {
        result = HL_LIST_PUSH(*pacs, rootPac);
        if (HL_FAILED(result)) return result;
    }

    /* Parse root pac if necessary. */
    if (hlArc)
    {
        result = hlINPACxV3ParseInto(header, HL_TRUE, hlArc);
        if (HL_FAILED(result)) return result;
    }

    /* Return early if not loading splits or if the root pac has no dependencies ("splits"). */
    if (!loadSplits || !header->splitCount) return result;

    /* Compute directory (including path separator, if any) length from root path. */
    dirLen = (size_t)(hlPathGetName(*pathBufPtr) - *pathBufPtr);

    /* Get split table. */
    splitTable = hlPACxV3GetSplitTable(header);

    /* Get split entries pointer. */
    splitEntries = (const HlPACxV3SplitEntry*)hlOff64Get(
        &splitTable->splitEntries);

    /* Load all splits referenced in the split table. */
    for (i = 0; i < splitTable->splitCount; ++i)
    {
        const char* splitName = (const char*)hlOff64Get(&splitEntries[i].name);
        const size_t splitNameSize = hlStrGetReqLenUTF8ToNative(splitName, 0);

        /* Account for directory. */
        const size_t reqPathBufSize = (splitNameSize + dirLen);

        /* Resize path buffer if necessary. */
        if (reqPathBufSize > *pathBufCap)
        {
            if (*pathBufPtr != pathBuf)
            {
                /* Switch from stack buffer to heap buffer. */
                *pathBufPtr = HL_ALLOC_ARR(HlNChar, reqPathBufSize);
            }
            else
            {
                /* Resize existing heap buffer. */
                *pathBufPtr = HL_RESIZE_ARR(HlNChar, reqPathBufSize, *pathBufPtr);
            }

            /* Return error if allocation failed. */
            if (!(*pathBufPtr)) return HL_ERROR_OUT_OF_MEMORY;

            /* Set new path buffer capacity. */
            *pathBufCap = reqPathBufSize;
        }

        /* Copy split name into buffer. */
        if (!hlStrConvUTF8ToNativeNoAlloc(splitName,
            &((*pathBufPtr)[dirLen]), 0, *pathBufCap))
        {
            return HL_ERROR_UNKNOWN;
        }

        /* Load split pac. */
        result = hlPACxV3LoadSingleInto(*pathBufPtr, HL_TRUE, pacs, hlArc);
        if (HL_FAILED(result)) return result;
    }

    return result;
}

HlResult hlPACxV3LoadAllInto(const HlNChar* HL_RESTRICT filePath,
    HlBlobList* HL_RESTRICT pacs, HlArchive* HL_RESTRICT hlArc)
{
    HlNChar pathBuf[255];
    HlNChar* pathBufPtr = pathBuf;
    HlBlob* rootPac;
    size_t pathBufCap = 255;
    HlResult result;

    /* Get root pac path and store it in our stack buffer, or in a heap buffer if necessary. */
    result = hlINPACxGetRootPathStackBuf(filePath, &pathBufCap, &pathBufPtr);
    if (HL_FAILED(result)) return result;

    /* Load root pac. */
    result = hlBlobLoad(pathBufPtr, &rootPac);
    if (HL_FAILED(result)) goto end;

    /* Load all pacs. */
    result = hlINPACxV3LoadAllInto(rootPac, HL_TRUE,
        pathBuf, &pathBufPtr, &pathBufCap, pacs, hlArc);

    if (!pacs) hlBlobFree(rootPac);

end:
    /* Free path buffer if necessary and return result. */
    if (pathBufPtr != pathBuf) hlFree(pathBufPtr);
    return result;
}

HlResult hlPACxV3Load(const HlNChar* HL_RESTRICT filePath,
    HlBool loadSplits, HlBlobList* HL_RESTRICT pacs,
    HlArchive* HL_RESTRICT * HL_RESTRICT hlArc)
{
    HlArchive* hlArcBuf;
    HlResult result;

    /* Allocate HlArchive buffer. */
    hlArcBuf = HL_ALLOC_OBJ(HlArchive);
    if (!hlArcBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Setup archive. */
    HL_LIST_INIT(hlArcBuf->entries);

    /*
       Add all the files from all the splits or simply a
       single archive based on user request.
    */
    result = (loadSplits) ?
        hlPACxV3LoadAllInto(filePath, pacs, hlArcBuf) :
        hlPACxV3LoadSingleInto(filePath, HL_TRUE, pacs, hlArcBuf);

    if (HL_FAILED(result)) return result;

    /* Set pointer and return result. */
    *hlArc = hlArcBuf;
    return result;
}

void hlPACxV4Fix(void* rawData)
{
    /* Swap header and root chunks if necessary. */
    HlPACxV4Header* header = (HlPACxV4Header*)rawData;
    if (hlBINANeedsSwap(header->endianFlag))
    {
        /* TODO: Swap header. */
        /* TODO: Swap root chunks. */
        HL_ASSERT(HL_FALSE);
    }

    /* Fix offsets. */
    hlOff32Fix(&header->rootOffset, header);
}

HlResult hlPACxV402DecompressNoAlloc(const void* HL_RESTRICT compressedData,
    const HlPACxV402Chunk* HL_RESTRICT chunks, HlU32 chunkCount,
    HlU32 compressedSize, HlU32 uncompressedSize,
    void* HL_RESTRICT uncompressedData)
{
    const char* compressedPtr = (const char*)compressedData;
    char* uncompressedPtr = (char*)uncompressedData;
    HlU32 i;

    /* If the data is already uncompressed, just copy it and return success. */
    if (compressedSize == uncompressedSize)
    {
        memcpy(uncompressedPtr, compressedPtr, uncompressedSize);
        return HL_RESULT_SUCCESS;
    }

    /* Otherwise, decompress the data chunk-by-chunk. */
    for (i = 0; i < chunkCount; ++i)
    {
        /* Decompress the current chunk. */
        int r = LZ4_decompress_safe(compressedPtr,
            uncompressedPtr, chunks[i].compressedSize,
            uncompressedSize);

        /* Return HL_ERROR_UNKNOWN if decompressing failed. */
        if (r < 0 || (HlU32)r < chunks[i].uncompressedSize)
            return HL_ERROR_UNKNOWN;

        /* Substract from uncompressedSize. */
        uncompressedSize -= chunks[i].uncompressedSize;

        /* Increment pointers. */
        compressedPtr += chunks[i].compressedSize;
        uncompressedPtr += chunks[i].uncompressedSize;
    }

    return HL_RESULT_SUCCESS;
}

HlResult hlPACxV402Decompress(const void* HL_RESTRICT compressedData,
    const HlPACxV402Chunk* HL_RESTRICT chunks, HlU32 chunkCount,
    HlU32 compressedSize, HlU32 uncompressedSize,
    void* HL_RESTRICT * HL_RESTRICT uncompressedData)
{
    void* uncompressedDataBuf;
    HlResult result;

    /* Allocate a buffer to hold the uncompressed data. */
    uncompressedDataBuf = hlAlloc(uncompressedSize);
    if (!uncompressedDataBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Decompress the data. */
    result = hlPACxV402DecompressNoAlloc(compressedData, chunks,
        chunkCount, compressedSize, uncompressedSize,
        uncompressedDataBuf);

    if (HL_FAILED(result)) return result;

    /* Set uncompressedData pointer and return success. */
    *uncompressedData = uncompressedDataBuf;
    return HL_RESULT_SUCCESS;
}

HlResult hlPACxV402DecompressBlob(const void* HL_RESTRICT compressedData,
    const HlPACxV402Chunk* HL_RESTRICT chunks, HlU32 chunkCount,
    HlU32 compressedSize, HlU32 uncompressedSize,
    HlBlob* HL_RESTRICT * HL_RESTRICT uncompressedBlob)
{
    HlBlob* uncompressedBlobBuf;
    HlResult result;

    /* Allocate a buffer to hold the uncompressed data. */
    uncompressedBlobBuf = (HlBlob*)hlAlloc(HL_BLOB_ALIGNED_SIZE + uncompressedSize);
    if (!uncompressedBlobBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Setup blob. */
    uncompressedBlobBuf->data = HL_ADD_OFF(uncompressedBlobBuf, HL_BLOB_ALIGNED_SIZE);
    uncompressedBlobBuf->size = (size_t)uncompressedSize;

    /* Decompress the data. */
    result = hlPACxV402DecompressNoAlloc(compressedData, chunks,
        chunkCount, compressedSize, uncompressedSize,
        uncompressedBlobBuf->data);

    if (HL_FAILED(result)) return result;

    /* Set uncompressedBlob pointer and return success. */
    *uncompressedBlob = uncompressedBlobBuf;
    return HL_RESULT_SUCCESS;
}

HlResult hlPACxV403DecompressNoAlloc(const void* HL_RESTRICT compressedData,
    HlU32 compressedSize, HlU32 uncompressedSize,
    void* HL_RESTRICT uncompressedData)
{
    return hlZlibDecompressNoAlloc(compressedData,
        compressedSize, uncompressedSize, uncompressedData);
}

HlResult hlPACxV403Decompress(const void* HL_RESTRICT compressedData,
    HlU32 compressedSize, HlU32 uncompressedSize,
    void* HL_RESTRICT * HL_RESTRICT uncompressedData)
{
    return hlDecompress(HL_COMPRESS_TYPE_ZLIB,
        compressedData, compressedSize,
        uncompressedSize, uncompressedData);
}

HlResult hlPACxV403DecompressBlob(const void* HL_RESTRICT compressedData,
    HlU32 compressedSize, HlU32 uncompressedSize,
    HlBlob* HL_RESTRICT * HL_RESTRICT uncompressedBlob)
{
    return hlDecompressBlob(HL_COMPRESS_TYPE_ZLIB,
        compressedData, compressedSize,
        uncompressedSize, uncompressedBlob);
}

HlResult hlPACxV4ReadInto(void* HL_RESTRICT rawData,
    HlBool parseSplits, HlBlobList* HL_RESTRICT uncompressedPacs,
    HlArchive* HL_RESTRICT hlArc)
{
    const HlPACxV4Header* header = (const HlPACxV4Header*)rawData;
    const HlBool isV403 = (header->version[1] == '0' && header->version[2] == '3');
    const HlPACxV3Header* uncompressedRootHeader;
    HlBlob* uncompressedRoot;
    HlResult result;
    HlBool hasSplits;

    /* Fix PACxV4 data. */
    hlPACxV4Fix(rawData);

    /* Decompress root PAC as necessary. */
    if (isV403)
    {
        /* PACx403 uses Deflate with no separate chunks. */
        const HlPACxV403Header* headerV403 = (const HlPACxV403Header*)header;

        result = hlPACxV403DecompressBlob(hlOff32Get(&headerV403->rootOffset),
            headerV403->rootCompressedSize, headerV403->rootUncompressedSize,
            &uncompressedRoot);
    }
    else
    {
        /* PACx402 uses LZ4 with separate chunks. */
        const HlPACxV402Header* headerV402 = (const HlPACxV402Header*)header;

        result = hlPACxV402DecompressBlob(hlOff32Get(&headerV402->rootOffset),
            hlPACxV402GetRootChunks(headerV402), headerV402->chunkCount,
            headerV402->rootCompressedSize, headerV402->rootUncompressedSize,
            &uncompressedRoot);
    }

    if (HL_FAILED(result)) return result;

    /* Fix uncompressed root PAC. */
    hlPACxV3Fix(uncompressedRoot->data);

    /* Get root header pointer and whether there are any splits. */
    uncompressedRootHeader = (const HlPACxV3Header*)uncompressedRoot->data;
    hasSplits = (uncompressedRootHeader->splitCount > 0);

    /* Add this archive's blob to the blobs list if necessary. */
    if (uncompressedPacs)
    {
        result = HL_LIST_PUSH(*uncompressedPacs, uncompressedRoot);
        if (HL_FAILED(result))
        {
            hlBlobFree(uncompressedRoot);
            return result;
        }
    }

    /* Parse root pac if necessary. */
    if (hlArc)
    {
        result = hlPACxV3ParseInto(uncompressedRootHeader,
            hasSplits, hlArc);

        if (HL_FAILED(result))
        {
            if (!uncompressedPacs) hlBlobFree(uncompressedRoot);
            return result;
        }
    }

    /* Parse split pacs as necessary. */
    if (parseSplits && uncompressedRootHeader->splitCount)
    {
        const HlPACxV3SplitTable* splitTable =
            hlPACxV3GetSplitTable(uncompressedRoot->data);

        const void* splitEntries = hlOff64Get(&splitTable->splitEntries);
        HlBlob* curUncompressedPac;
        HlU64 i;

        /* Ensure split count will fit within a size_t. */
        if (splitTable->splitCount > HL_SIZE_MAX)
        {
            result = HL_ERROR_OUT_OF_RANGE;
            goto end;
        }

        /* Setup split PAC entries, decompressing if necessary. */
        for (i = 0; i < splitTable->splitCount; ++i)
        {
            /* Decompress split PAC. */
            if (isV403)
            {
                const HlPACxV403SplitEntry* splitEntriesV403 =
                    (const HlPACxV403SplitEntry*)splitEntries;

                result = hlPACxV403DecompressBlob(HL_ADD_OFFC(header, splitEntriesV403[i].offset),
                    splitEntriesV403[i].compressedSize, splitEntriesV403[i].uncompressedSize,
                    &curUncompressedPac);
            }

            else
            {
                const HlPACxV402SplitEntry* splitEntriesV402 =
                    (const HlPACxV402SplitEntry*)splitEntries;

                const HlPACxV402Chunk* chunks = (const HlPACxV402Chunk*)
                    hlOff64Get(&splitEntriesV402[i].chunksOffset);

                result = hlPACxV402DecompressBlob(HL_ADD_OFFC(header, splitEntriesV402[i].offset),
                    chunks, splitEntriesV402[i].chunkCount, splitEntriesV402[i].compressedSize,
                    splitEntriesV402[i].uncompressedSize, &curUncompressedPac);
            }

            if (HL_FAILED(result)) goto end;

            /* Fix split PAC. */
            hlPACxV3Fix(curUncompressedPac->data);

            /* Add this archive's blob to the blobs list if necessary. */
            if (uncompressedPacs)
            {
                result = HL_LIST_PUSH(*uncompressedPacs, curUncompressedPac);
                if (HL_FAILED(result))
                {
                    hlBlobFree(curUncompressedPac);
                    goto end;
                }
            }

            /* Parse root pac if necessary. */
            if (hlArc)
            {
                result = hlPACxV3ParseInto((const HlPACxV3Header*)
                    curUncompressedPac->data, hasSplits, hlArc);

                if (HL_FAILED(result))
                {
                    if (!uncompressedPacs) hlBlobFree(curUncompressedPac);
                    goto end;
                }
            }

            /* Free split blob if necessary. */
            if (!uncompressedPacs) hlBlobFree(curUncompressedPac);
        }
    }

end:
    /* Free root blob if necessary and return result. */
    if (!uncompressedPacs) hlBlobFree(uncompressedRoot);
    return result;
}

HlResult hlPACxV4LoadInto(const HlNChar* HL_RESTRICT filePath,
    HlBool parseSplits, HlBlobList* HL_RESTRICT uncompressedPacs,
    HlArchive* HL_RESTRICT hlArc)
{
    HlBlob* pac;
    HlResult result;

    /* Load archive. */
    result = hlBlobLoad(filePath, &pac);
    if (HL_FAILED(result)) return result;

    /* Parse blob into HlArchive, free blob, and return. */
    result = hlPACxV4ReadInto(pac->data,
        parseSplits, uncompressedPacs, hlArc);

    hlBlobFree(pac);
    return result;
}

HlResult hlPACxV4Load(const HlNChar* HL_RESTRICT filePath,
    HlBool parseSplits, HlBlobList* HL_RESTRICT uncompressedPacs,
    HlArchive* HL_RESTRICT * HL_RESTRICT hlArc)
{
    HlArchive* hlArcBuf;
    HlResult result;

    /* Allocate HlArchive buffer. */
    hlArcBuf = HL_ALLOC_OBJ(HlArchive);
    if (!hlArcBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Setup archive. */
    HL_LIST_INIT(hlArcBuf->entries);

    /* Load the pac data into the blobs and HlArchive as requested. */
    result = hlPACxV4LoadInto(filePath, parseSplits,
        uncompressedPacs, hlArcBuf);

    if (HL_FAILED(result)) return result;

    /* Set pointer and return result. */
    *hlArc = hlArcBuf;
    return result;
}

HlResult hlPACxLoadInto(const HlNChar* HL_RESTRICT filePath,
    HlU8 majorVersion, HlBool loadSplits, HlBlobList* HL_RESTRICT pacs,
    HlArchive* HL_RESTRICT hlArc)
{
    HlNChar pathBuf[255];
    HlNChar* pathBufPtr = pathBuf;
    HlBlob* rootPac = NULL;
    size_t dirLen, pathBufCap = 255;
    HlResult result;
    HlBool pathBufOnHeap = HL_FALSE;

    /* Get root pac path and store it in our stack buffer, or in a heap buffer if necessary. */
    result = hlINPACxGetRootPathStackBuf(filePath, &pathBufCap, &pathBufPtr);
    if (HL_FAILED(result)) return result;

    /* Get directory length. */
    dirLen = (hlPathGetName(pathBufPtr) - pathBufPtr);

    /* Load the root pac. */
    result = hlBlobLoad(pathBufPtr, &rootPac);
    if (HL_FAILED(result)) goto end;

    /* Get version automatically if none was specified. */
    if (!majorVersion)
    {
        const HlBINAV2Header* header = (const HlBINAV2Header*)rootPac->data;

        /* Ensure this is, in fact, a PACx file. */
        if (header->signature != HL_PACX_SIG)
        {
            result = HL_ERROR_INVALID_DATA;
            hlBlobFree(rootPac);
            goto end;
        }

        /* Get major version number. */
        majorVersion = header->version[0];
    }

    /* Parse pacs into single HlArchive based on version. */
    switch (majorVersion)
    {
    case '2':
        result = hlINPACxV2LoadAllInto(rootPac, loadSplits,
            pathBuf, &pathBufPtr, &pathBufCap, pacs, hlArc);

        if (!pacs) hlBlobFree(rootPac);
        break;

    case '3':
        result = hlINPACxV3LoadAllInto(rootPac, loadSplits,
            pathBuf, &pathBufPtr, &pathBufCap, pacs, hlArc);

        if (!pacs) hlBlobFree(rootPac);
        break;

    case '4':
        result = hlPACxV4ReadInto(rootPac->data,
            loadSplits, pacs, hlArc);

        hlBlobFree(rootPac);
        break;

    default:
        result = HL_ERROR_UNSUPPORTED;
        break;
    }

end:
    /* Free path buffer if necessary and return result. */
    if (pathBufOnHeap) hlFree(pathBufPtr);
    return result;
}

HlResult hlPACxLoad(const HlNChar* HL_RESTRICT filePath,
    HlU8 majorVersion, HlBool loadSplits, HlBlobList* HL_RESTRICT pacs,
    HlArchive* HL_RESTRICT * HL_RESTRICT hlArc)
{
    HlArchive* hlArcBuf;
    HlResult result;

    /* Allocate HlArchive buffer. */
    hlArcBuf = HL_ALLOC_OBJ(HlArchive);
    if (!hlArcBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Setup archive. */
    HL_LIST_INIT(hlArcBuf->entries);

    /* Load the pac data into the blobs and HlArchive as requested. */
    result = hlPACxLoadInto(filePath, majorVersion,
        loadSplits, pacs, hlArcBuf);

    if (HL_FAILED(result)) return result;

    /* Set pointer and return result. */
    *hlArc = hlArcBuf;
    return result;
}

#ifndef HL_NO_EXTERNAL_WRAPPERS
const HlPACxV2NodeTree* hlPACxV2DataGetTypeTreeExt(
    const HlPACxV2BlockDataHeader* dataBlock)
{
    return hlPACxV2DataGetTypeTree(dataBlock);
}

const HlPACxV3NodeTree* hlPACxV3GetTypeTreeExt(const void* rawData)
{
    return hlPACxV3GetTypeTree(rawData);
}

const HlPACxV402Chunk* hlPACxV402GetRootChunksExt(const void* rawData)
{
    return hlPACxV402GetRootChunks(rawData);
}
#endif
