#include "hl_in_archive.h"
#include "../hl_in_assert.h"
#include "../io/hl_in_path.h"
#include "hedgelib/effects/hl_grif.h"
#include "hedgelib/archives/hl_pacx.h"
#include "hedgelib/io/hl_path.h"
#include "hedgelib/io/hl_file.h"
#include "hedgelib/hl_radix_tree.h"
#include "hedgelib/hl_endian.h"
#include "hedgelib/hl_compression.h"
#include "hedgelib/hl_math.h"
#include "hedgelib/hl_text.h"
#include <stdlib.h>

#define HL_IN_PACX_VER_301      HL_BINA_MAKE_VER('3', '0', '1')
#define HL_IN_PACX_VER_402      HL_BINA_MAKE_VER('4', '0', '2')
#define HL_IN_PACX_VER_403      HL_BINA_MAKE_VER('4', '0', '3')

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
const HlPACxSupportedExt HlPACxLWExts[] =
{
#define HL_IN_PACX_LW_AUTOGEN(ext, pacxType, type, rootSortWeight, splitSortWeight)\
    { HL_NTEXT(ext), pacxType, HL_PACX_EXT_FLAGS_##type##_TYPE, rootSortWeight, splitSortWeight },

#include "hl_in_pacx_type_autogen.h"

    { 0 }
};

const size_t HlPACxLWExtCount = ((sizeof(HlPACxLWExts) /
    sizeof(const HlPACxSupportedExt)) - 1);

const HlPACxSupportedExt HlPACxRioExts[] =
{
#define HL_IN_PACX_RIO_AUTOGEN(ext, pacxType, type, rootSortWeight, splitSortWeight)\
    { HL_NTEXT(ext), pacxType, HL_PACX_EXT_FLAGS_##type##_TYPE, rootSortWeight, splitSortWeight },

#include "hl_in_pacx_type_autogen.h"

    { 0 }
};

const size_t HlPACxRioExtCount = ((sizeof(HlPACxRioExts) /
    sizeof(const HlPACxSupportedExt)) - 1);

const HlPACxSupportedExt HlPACxForcesExts[] =
{
#define HL_IN_PACX_FORCES_AUTOGEN(ext, pacxType, type)\
    { HL_NTEXT(ext), pacxType, HL_PACX_EXT_FLAGS_##type##_TYPE },

#include "hl_in_pacx_type_autogen.h"

    { 0 }
};

const size_t HlPACxForcesExtCount = ((sizeof(HlPACxForcesExts) /
    sizeof(const HlPACxSupportedExt)) - 1);

const HlPACxSupportedExt HlPACxTokyo1Exts[] =
{
#define HL_IN_PACX_TOKYO1_AUTOGEN(ext, pacxType, type)\
    { HL_NTEXT(ext), pacxType, HL_PACX_EXT_FLAGS_##type##_TYPE },

#include "hl_in_pacx_type_autogen.h"

    { 0 }
};

const size_t HlPACxTokyo1ExtCount = ((sizeof(HlPACxTokyo1Exts) /
    sizeof(const HlPACxSupportedExt)) - 1);

const HlPACxSupportedExt HlPACxTokyo2Exts[] =
{
#define HL_IN_PACX_TOKYO2_AUTOGEN(ext, pacxType, type)\
    { HL_NTEXT(ext), pacxType, HL_PACX_EXT_FLAGS_##type##_TYPE },

#include "hl_in_pacx_type_autogen.h"

    { 0 }
};

const size_t HlPACxTokyo2ExtCount = ((sizeof(HlPACxTokyo2Exts) /
    sizeof(const HlPACxSupportedExt)) - 1);

const HlPACxSupportedExt HlPACxSakuraExts[] =
{
#define HL_IN_PACX_SAKURA_AUTOGEN(ext, pacxType, type)\
    { HL_NTEXT(ext), pacxType, HL_PACX_EXT_FLAGS_##type##_TYPE },

#include "hl_in_pacx_type_autogen.h"

    { 0 }
};

const size_t HlPACxSakuraExtCount = ((sizeof(HlPACxSakuraExts) /
    sizeof(const HlPACxSupportedExt)) - 1);

const HlPACxSupportedExt HlPACxPPT2Exts[] =
{
#define HL_IN_PACX_PPT2_AUTOGEN(ext, pacxType, type)\
    { HL_NTEXT(ext), pacxType, HL_PACX_EXT_FLAGS_##type##_TYPE },

#include "hl_in_pacx_type_autogen.h"

    { 0 }
};

const size_t HlPACxPPT2ExtCount = ((sizeof(HlPACxPPT2Exts) /
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

static HlResult hlINPACxSaveSetupPathBuffer(const HlNChar* HL_RESTRICT filePath,
    HlU32 splitLimit, const size_t filePathLen, size_t pathBufCap,
    HlNChar* HL_RESTRICT * HL_RESTRICT pathBufPtr)
{
    const size_t filePathSize = (filePathLen + 1);
    const size_t reqPathBufSize = (splitLimit) ?
        (filePathSize + 3) :    /* Account for path + split extension. */
        filePathSize;           /* Account for path. */

    /* Reallocate path buffer if necessary. */
    if (reqPathBufSize > pathBufCap)
    {
        *pathBufPtr = HL_ALLOC_ARR(HlNChar, reqPathBufSize);
        if (!(*pathBufPtr)) return HL_ERROR_OUT_OF_MEMORY;
    }

    /* Copy filePath and null terminator into path buffer. */
    memcpy(*pathBufPtr, filePath, sizeof(HlNChar) * filePathSize);
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
    /** @brief The extension(s) of this file, without the initial dot (e.g. "dds"). */
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

static int hlINPACxCompareFileNames(
    const HlNChar* HL_RESTRICT fileName1, const HlNChar* HL_RESTRICT fileName2,
    size_t fileNameLen1, size_t fileNameLen2)
{
    const size_t fileNameMinLen = HL_MIN(fileNameLen1, fileNameLen2);
    const int nameSortWeight = hlNStrNCmp(fileName1,
        fileName2, fileNameMinLen);

    if (nameSortWeight == 0 && fileNameLen1 != fileNameLen2)
    {
        if (fileNameMinLen == fileNameLen1)
        {
            const size_t lastCharPos = (fileNameLen2 > 0) ?
                (fileNameLen2 - 1) : 0;

            return (0 - (int)fileName2[lastCharPos]);
        }
        else
        {
            const size_t lastCharPos = (fileNameLen1 > 0) ?
                (fileNameLen1 - 1) : 0;

            return ((int)fileName1[lastCharPos] - 0);
        }
    }

    return nameSortWeight;
}

static int hlINPACxV2FileMetadataCompareNames(
    const HlINPACxV2FileMetadata* HL_RESTRICT fileMeta1,
    const HlINPACxV2FileMetadata* HL_RESTRICT fileMeta2)
{
    return hlINPACxCompareFileNames(fileMeta1->name, fileMeta2->name,
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

        if (hlINPACxCompareFileNames(curFileMetadata->name, fileName,
            (curFileMetadata->ext - curFileMetadata->name), fileNameLen) == 0)
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

static void hlINPACxV2TypeMetadataListDestruct(HlINPACxV2TypeMetadataList* typeMetadata)
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
            const int extSortWeight = hlNStrICmpAsUpperCase(
                fileMeta1->ext, fileMeta2->ext);

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
                return hlNStrNICmpAsUpperCase(fileMeta1->name,
                    fileMeta2->name, fileNameLen1);
            }
            else
            {
                const size_t fileNameMinLen = HL_MIN(fileNameLen1, fileNameLen2);
                const int nameSortWeight = hlNStrNICmpAsUpperCase(
                    fileMeta1->name, fileMeta2->name, fileNameMinLen);

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

    /* Get entry data pointer if necessary. */
    if (isHere)
    {
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

        if (fileMetadata->entry->size >= sizeof(HlBINAV2Header) &&
            hlBINAHasV2Header(entryData))
        {
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
            /* Write data to stream. */
            result = hlStreamWrite(stream, dataSize, entryData, NULL);
            if (HL_FAILED(result)) goto end;

            /* Add packed file entry to packed file index if necessary. */
            if (pfi)
            {
                /* Generate packed file entry. */
                HlPackedFileEntry packedEntry;
                result = hlPackedFileEntryConstruct(fileMetadata->name,
                    (HlU32)dataPos, dataEntry.dataSize, &packedEntry);

                if (HL_FAILED(result)) goto end;

                /* Add packed file entry to packed file index. */
                result = HL_LIST_PUSH(pfi->entries, packedEntry);
                if (HL_FAILED(result))
                {
                    hlPackedFileEntryDestruct(&packedEntry);
                    goto end;
                }
            }
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
    size_t filePathLen, HlU32 splitLimit, HlU32 dataAlignment,
    HlBINAEndianFlag endianFlag, HlPackedFileIndex* HL_RESTRICT pfi,
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

static HlResult hlINPACxV2FileMetadataSplitUp(
    HlINPACxV2TypeMetadataList* HL_RESTRICT typeMetadata,
    HlINPACxV2FileMetadata* HL_RESTRICT fileMetadata, size_t fileMetadataCount,
    HlU32 splitLimit, unsigned short* HL_RESTRICT splitCount)
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
    result = hlINPACxSaveSetupPathBuffer(filePath,
        splitLimit, filePathLen, pathBufCap, &pathBufPtr);

    if (HL_FAILED(result)) return result;

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

        if (!fileMetadata)
        {
            result = HL_ERROR_OUT_OF_MEMORY;
            goto failed_root_not_open;
        }

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

        /* Add split table file ("ResPacDepend") to file metadata. */
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

        /* Generate type metadata. */
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
                &splitCount);

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
                filePathLen, splitLimit, dataAlignment, endianFlag,
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
    hlINPACxV2TypeMetadataListDestruct(&typeMetadata);

    if (pathBufPtr != pathBuf) hlFree(pathBufPtr);
    return result;
}

void hlPACxV3HeaderSwap(HlPACxV3Header* header, HlBool swapOffsets)
{
    hlSwapU32P(&header->uid);
    hlSwapU32P(&header->fileSize);
    hlSwapU32P(&header->treesSize);
    hlSwapU32P(&header->splitTableSize);
    hlSwapU32P(&header->dataEntriesSize);
    hlSwapU32P(&header->stringTableSize);
    hlSwapU32P(&header->fileDataSize);
    hlSwapU32P(&header->offsetTableSize);
    hlSwapU16P(&header->type);
    hlSwapU16P(&header->unknown2);
    hlSwapU32P(&header->splitCount);
}

void hlPACxV3Fix(void* rawData)
{
    HlPACxV3Header* header = (HlPACxV3Header*)rawData;
    if (hlBINANeedsSwap(header->endianFlag))
    {
        hlPACxV3HeaderSwap(header, HL_FALSE);
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
    const HlS32* childIndices = (const HlS32*)hlOff64Get(&node->childIndicesOffset);
    HlU16 i = 0, childCount = node->childCount;

    while (i < childCount)
    {
        const HlPACxV3Node* childNode = &nodes[childIndices[i]];
        const char* nodeName = (const char*)hlOff64Get(&childNode->nameOffset);
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
            childIndices = (const HlS32*)hlOff64Get(&childNode->childIndicesOffset);
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
    const HlPACxV3Node* nodes = (const HlPACxV3Node*)hlOff64Get(&nodeTree->nodesOffset);
    HlU32 i = 0;

    for (i = 0; i < nodeTree->nodeCount; ++i)
    {
        const char* nodeName = (const char*)hlOff64Get(&nodes[i].nameOffset);
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
    const HlS32* childIndices = (const HlS32*)hlOff64Get(
        &curFileNode->childIndicesOffset);

    HlU16 i;
    HlResult result;

    if (curFileNode->hasData)
    {
        const HlPACxV3DataEntry* dataEntry = (const HlPACxV3DataEntry*)
            hlOff64Get(&curFileNode->dataOffset);

        /* Skip proxies if requested. */
        if (!skipProxies || dataEntry->dataType != HL_PACXV3_DATA_TYPE_NOT_HERE)
        {
            HlArchiveEntry entry;
            const char* ext = (const char*)hlOff64Get(&dataEntry->extOffset);
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
                memcpy(dstDataBuf, hlOff64Get(&dataEntry->dataOffset), entry.size);

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
    else if (curFileNode->nameOffset)
    {
        /* Copy name into path buffer. */
        strcpy(&pathBuf[curFileNode->bufStartIndex],
            (const char*)hlOff64Get(&curFileNode->nameOffset));
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
    const HlPACxV3Node* typeNodes = (const HlPACxV3Node*)hlOff64Get(&typeTree->nodesOffset);
    const HlS32* typeDataIndices = (const HlS32*)hlOff64Get(&typeTree->dataNodeIndicesOffset);
    HlU64 i;
    HlResult result = HL_RESULT_SUCCESS;

    /* Setup archive entries. */
    for (i = 0; i < typeTree->dataNodeCount; ++i)
    {
        /* Setup archive entries. */
        const HlPACxV3NodeTree* fileTree = (const HlPACxV3NodeTree*)
            hlOff64Get(&typeNodes[typeDataIndices[i]].dataOffset);

        const HlPACxV3Node* fileNodes = (const HlPACxV3Node*)
            hlOff64Get(&fileTree->nodesOffset);

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
        const char* splitName = (const char*)hlOff64Get(&splitEntries[i].nameOffset);
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

HlU32 hlPACxV3GenerateUID(void)
{
    return ((((HlU32)rand() % 256) << 24) |
        (((HlU32)rand() % 256) << 16) |
        (((HlU32)rand() % 256) << 8) |
        ((HlU32)rand() % 256));
}

HlResult hlPACxV3StartWrite(HlU32 version, HlU32 uid,
    HlU16 type, HlBINAEndianFlag endianFlag, HlStream* stream)
{
    /* Generate PACxV3 header. */
    HlPACxV3Header header =
    {
        HL_PACX_SIG,                                        /* signature */
        {                                                   /* version */
            hlBINAGetMajorVersionChar(version),
            hlBINAGetMinorVersionChar(version),
            hlBINAGetRevisionVersionChar(version)
        },
        (HlU8)endianFlag,                                   /* endianFlag */
        uid,                                                /* uid */
        0,                                                  /* fileSize */
        0,                                                  /* treesSize */
        0,                                                  /* splitTableSize */
        0,                                                  /* dataEntriesSize */
        0,                                                  /* stringTableSize */
        0,                                                  /* fileDataSize */
        0,                                                  /* offsetTableSize */
        type,                                               /* type */
        0x108,                                              /* unknown2 */
        0                                                   /* splitCount */
    };

    /* Endian-swap header if necessary. */
    if (hlBINANeedsSwap(endianFlag))
    {
        hlPACxV3HeaderSwap(&header, HL_FALSE);
    }

    /* Write PACxV3 header and return result. */
    return hlStreamWrite(stream, sizeof(header), &header, NULL);
}

HlResult hlPACxV3FinishWrite(size_t headerPos, HlU32 treesSize,
    HlU32 splitTableSize, HlU32 dataEntriesSize, HlU32 stringTableSize,
    size_t fileDataPos, HlU32 splitCount, HlBINAEndianFlag endianFlag,
    HlOffTable* HL_RESTRICT offTable, HlStream* HL_RESTRICT stream)
{
    size_t offTablePos, eofPos;
    HlResult result;

    /* Write padding. */
    result = hlStreamPad(stream, 8);
    if (HL_FAILED(result)) return result;

    /* Get offset table position. */
    offTablePos = hlStreamTell(stream);

    /* Write offset table. */
    result = hlBINAOffsetsWrite64(headerPos, offTable, stream);
    if (HL_FAILED(result)) return result;

    /* Get end of stream position. */
    eofPos = hlStreamTell(stream);

    /* Jump to header fileSize position. */
    result = hlStreamJumpTo(stream, headerPos + offsetof(HlPACxV3Header, fileSize));
    if (HL_FAILED(result)) return result;

    /* Fill-in header values. */
    {
        struct
        {
            HlU32 fileSize;
            HlU32 treesSize;
            HlU32 splitTableSize;
            HlU32 dataEntriesSize;
            HlU32 stringTableSize;
            HlU32 fileDataSize;
            HlU32 offsetTableSize;
        }
        values;

        /* Compute header values. */
        values.fileSize = (HlU32)(eofPos - headerPos);
        values.treesSize = treesSize;
        values.splitTableSize = splitTableSize;
        values.dataEntriesSize = dataEntriesSize;
        values.stringTableSize = stringTableSize;
        values.fileDataSize = (HlU32)(offTablePos - fileDataPos);
        values.offsetTableSize = (HlU32)(eofPos - offTablePos);

        /* Endian-swap header values if necessary. */
        if (hlBINANeedsSwap(endianFlag))
        {
            hlSwapU32P(&values.fileSize);
            hlSwapU32P(&values.treesSize);
            hlSwapU32P(&values.splitTableSize);
            hlSwapU32P(&values.dataEntriesSize);
            hlSwapU32P(&values.stringTableSize);
            hlSwapU32P(&values.fileDataSize);
            hlSwapU32P(&values.offsetTableSize);
        }

        /* Fill-in header values. */
        result = hlStreamWrite(stream, sizeof(values), &values, NULL);
        if (HL_FAILED(result)) return result;

        /* Jump to header splitCount position. */
        result = hlStreamJumpTo(stream, headerPos + offsetof(HlPACxV3Header, splitCount));
        if (HL_FAILED(result)) return result;

        /* Endian-swap splitCount if necessary. */
        if (hlBINANeedsSwap(endianFlag))
        {
            hlSwapU32P(&splitCount);
        }

        /* Fill-in splitCount. */
        result = hlStreamWrite(stream, sizeof(splitCount), &splitCount, NULL);
        if (HL_FAILED(result)) return result;
    }

    /* Jump to end of stream and return. */
    return hlStreamJumpTo(stream, eofPos);
}

typedef struct HlINPACxV3FileMetadata
{
    const HlArchiveEntry* entry;
    const HlNChar* name;
    /** @brief The extension(s) of this file, without the initial dot (e.g. "dds"). */
    const HlNChar* ext;
    const HlPACxSupportedExt* pacxExt;
    unsigned short splitIndex;
}
HlINPACxV3FileMetadata;

typedef struct HlINPACxV3TypeMetadata
{
    const HlINPACxV3FileMetadata* files;
    size_t fileCount;
    HlRadixTree tmpFileTree;
    HlS32* tmpFileDataNodeIndices;
    HlU32 tmpFileDataNodeCount;
}
HlINPACxV3TypeMetadata;

typedef HL_LIST(HlINPACxV3TypeMetadata) HlINPACxV3TypeMetadataList;

typedef struct HlINPACxV3PacMetadata
{
    char* name;
    size_t uncompressedSize;
    HlBlob* blob;
    HlPACxV402ChunkList chunks;
}
HlINPACxV3PacMetadata;

typedef HL_LIST(HlINPACxV3PacMetadata) HlINPACxV3PacMetadataList;

static int hlINPACxV3FileMetadataCompareNames(
    const HlINPACxV3FileMetadata* HL_RESTRICT fileMeta1,
    const HlINPACxV3FileMetadata* HL_RESTRICT fileMeta2)
{
    return hlINPACxCompareFileNames(fileMeta1->name, fileMeta2->name,
        (fileMeta1->ext - fileMeta1->name),
        (fileMeta2->ext - fileMeta2->name));
}

static void hlINPACxV3TypeMetadataListDestruct(HlINPACxV3TypeMetadataList* typeMetadata)
{
    size_t i;

    /* Destruct temporary file radix trees and data node indices. */
    for (i = 0; i < typeMetadata->count; ++i)
    {
        hlRadixTreeDestruct(&typeMetadata->data[i].tmpFileTree);
        hlFree(typeMetadata->data[i].tmpFileDataNodeIndices);
    }

    /* Free type metadata list. */
    HL_LIST_FREE(*typeMetadata);
}

static HlResult hlINPACxV3PacMetadataConstruct(
    HlINPACxV3PacMetadata* HL_RESTRICT pacMetadata,
    const HlNChar* HL_RESTRICT rootName, size_t rootNameSize,
    size_t splitNameSize)
{
    /* Setup name as necessary. */
    if (rootName)
    {
        /* Allocate new buffer for split name. */
        pacMetadata->name = HL_ALLOC_ARR(char, splitNameSize);
        if (!pacMetadata->name) return HL_ERROR_OUT_OF_MEMORY;

        /* Convert split name to UTF-8 and copy into split name buffer. */
        if (!hlStrConvNativeToUTF8NoAlloc(rootName, pacMetadata->name,
            rootNameSize + 4, splitNameSize))
        {
            hlFree(pacMetadata->name);
            return HL_ERROR_UNKNOWN;
        }
    }
    else
    {
        pacMetadata->name = NULL;
    }

    /* Construct the rest of the pac metadata. */
    pacMetadata->uncompressedSize = 0;
    pacMetadata->blob = NULL;
    HL_LIST_INIT(pacMetadata->chunks);

    return HL_RESULT_SUCCESS;
}

static void hlINPACxV3PacMetadataDestruct(HlINPACxV3PacMetadata* pacMetadata)
{
    hlFree(pacMetadata->name);
    hlBlobFree(pacMetadata->blob);
    HL_LIST_FREE(pacMetadata->chunks);
}

static void hlINPACxV3PacMetadataListDestruct(HlINPACxV3PacMetadataList* pacMetadata)
{
    size_t i;

    /* Destruct pac metadata. */
    for (i = 0; i < pacMetadata->count; ++i)
    {
        hlINPACxV3PacMetadataDestruct(&pacMetadata->data[i]);
    }

    /* Free pac metadata list. */
    HL_LIST_FREE(*pacMetadata);
}

static HlResult hlINPACxV3WriteNodes(const HlRadixNode* HL_RESTRICT radixNode,
    HlS32 parentIndex, HlU8 bufStartIndex, HlBINAEndianFlag endianFlag,
    HlS32* HL_RESTRICT globalIndex, HlS32* HL_RESTRICT globalDataIndex,
    HlS32* HL_RESTRICT dataNodeIndices, HlStrTable* HL_RESTRICT strTable,
    HlOffTable* HL_RESTRICT offTable, HlStream* HL_RESTRICT stream)
{
    /* Generate node. */
    const size_t nodePos = hlStreamTell(stream);
    HlPACxV3Node node =
    {
        0,                                          /* name */
        0,                                          /* data */
        0,                                          /* childIndices */
        parentIndex,                                /* parentIndex */
        *globalIndex,                               /* globalIndex */
        -1,                                         /* dataIndex */
        (HlU16)radixNode->children.count,           /* childCount */
        0,                                          /* hasData */
        bufStartIndex                               /* bufStartIndex */
    };

    size_t i;
    HlResult result;

    /* Ensure total radix node name length does not exceed 255 characters. */
    if ((((size_t)bufStartIndex) + radixNode->nameLen) > 255)
    {
        return HL_ERROR_OUT_OF_RANGE;
    }

    /* Increase child count to account for data node if necessary. */
    if (radixNode->customData) ++node.childCount;

    /* Endian-swap node if necessary. */
    if (hlBINANeedsSwap(endianFlag))
    {
        /* TODO: Endian-swap node. */
    }

    /* Write node. */
    result = hlStreamWrite(stream, sizeof(node), &node, NULL);
    if (HL_FAILED(result)) return result;

    /* Add node name to string table if necessary. */
    if (radixNode->name)
    {
        result = hlStrTableAddStrRefUTF8Ex(strTable, radixNode->name,
            nodePos, radixNode->nameLen);

        if (HL_FAILED(result)) return result;
    }

    /* Increase indices. */
    parentIndex = *globalIndex;
    ++(*globalIndex);
    bufStartIndex += (HlU8)radixNode->nameLen;

    /* Write data node if necessary. */
    if (radixNode->customData)
    {
        /* Generate data node. */
        node.parentIndex = parentIndex;
        node.globalIndex = *globalIndex;
        node.dataIndex = *globalDataIndex;
        node.childCount = 0;
        node.hasData = 1;
        node.bufStartIndex = bufStartIndex;

        /* Endian-swap data node if necessary. */
        if (hlBINANeedsSwap(endianFlag))
        {
            /* TODO: Endian-swap node. */
        }

        /* Write data node. */
        result = hlStreamWrite(stream, sizeof(node), &node, NULL);
        if (HL_FAILED(result)) return result;
        
        /*
           Store data node index in buffer.
           
           NOTE: This value is already in the endianness required to
           write it to the file; don't try to read it later as this 
           may not match the current platform's endianness!
        */
        dataNodeIndices[*globalDataIndex] = node.globalIndex;

        /* Increase indices. */
        ++(*globalIndex);
        ++(*globalDataIndex);
    }

    /* Recurse through child nodes and return result. */
    for (i = 0; i < radixNode->children.count; ++i)
    {
        result = hlINPACxV3WriteNodes(&radixNode->children.data[i],
            parentIndex, bufStartIndex, endianFlag, globalIndex,
            globalDataIndex, dataNodeIndices, strTable, offTable,
            stream);

        if (HL_FAILED(result)) return result;
    }
    
    return result;
}

static HlResult hlINPACxV3WriteNodeTree(const HlRadixTree* HL_RESTRICT radixTree,
    HlU32 dataNodeCount, HlBINAEndianFlag endianFlag,
    HlS32* HL_RESTRICT dataNodeIndices, HlStrTable* HL_RESTRICT strTable,
    HlOffTable* HL_RESTRICT offTable, HlStream* HL_RESTRICT stream)
{
    /* Generate node tree. */
    const size_t eofPos = hlStreamTell(stream);
    HlPACxV3NodeTree nodeTree =
    {
        (HlU32)(radixTree->totalNodeCount + dataNodeCount), /* nodeCount */
        dataNodeCount,                                      /* dataNodeCount */
        (HlU64)(eofPos + sizeof(HlPACxV3NodeTree)),         /* nodesOffset */
        0                                                   /* dataNodeIndicesOffset */
    };

    HlS32 globalIndex = 0, globalDataIndex = 0;
    HlResult result;

    /* Endian-swap node tree if necessary. */
    if (hlBINANeedsSwap(endianFlag))
    {
        /* TODO: Endian-swap type tree. */
    }

    /* Write node tree. */
    result = hlStreamWrite(stream, sizeof(nodeTree), &nodeTree, NULL);
    if (HL_FAILED(result)) return result;

    /* Add nodes offset to offset table. */
    result = HL_LIST_PUSH(*offTable, eofPos +
        offsetof(HlPACxV3NodeTree, nodesOffset));

    if (HL_FAILED(result)) return result;

    /* Recursively write placeholder nodes and return result. */
    return hlINPACxV3WriteNodes(&radixTree->rootNode,
        -1, 0, endianFlag, &globalIndex, &globalDataIndex,
        dataNodeIndices, strTable, offTable, stream);
}

static HlResult hlINPACxV3FillInTypeNodes(const HlRadixNode* HL_RESTRICT radixNode,
    unsigned short splitIndex, HlBINAEndianFlag endianFlag, size_t* HL_RESTRICT curOffPos,
    char* HL_RESTRICT tmpFileNameBuf, HlStrTable* HL_RESTRICT strTable,
    HlOffTable* HL_RESTRICT offTable, HlStream* HL_RESTRICT stream)
{
    size_t i;
    HlResult result = HL_RESULT_SUCCESS;

    /* Increase current offset position. */
    *curOffPos += sizeof(HlPACxV3Node);

    /* Write file tree and fill in type node data offset if necessary. */
    if (radixNode->customData)
    {
        HlINPACxV3TypeMetadata* curTypeMetadata = (HlINPACxV3TypeMetadata*)(
            (HlUPtr)radixNode->customData);

        HlU32 fileDataNodeCount = 0;

        /* Clear temporary file tree in type metadata. */
        hlRadixTreeReset(&curTypeMetadata->tmpFileTree);

        /* Generate file nodes. */
        for (i = 0; i < curTypeMetadata->fileCount; ++i)
        {
            const HlINPACxV3FileMetadata* curFileMetadata = &curTypeMetadata->files[i];
            size_t nativeFileNameLen, utf8FileNameLen;

            /* Skip this file if we're writing a split and this file is not in it. */
            if (splitIndex != USHRT_MAX && curFileMetadata->splitIndex != splitIndex)
            {
                continue;
            }

            /* Compute file name length. */
            nativeFileNameLen = (curFileMetadata->ext - curFileMetadata->name);

            /* Convert file name to UTF-8. */
            utf8FileNameLen = hlStrConvNativeToUTF8NoAlloc(curFileMetadata->name,
                tmpFileNameBuf, nativeFileNameLen, 255);

            if (!utf8FileNameLen)
            {
                return HL_ERROR_UNKNOWN;
            }

            /* Append null terminator to UTF-8 file name. */
            tmpFileNameBuf[utf8FileNameLen] = '\0';

            /* Insert UTF-8 file name into file tree. */
            result = hlRadixTreeInsert(&curTypeMetadata->tmpFileTree,
                tmpFileNameBuf, (HlUMax)((HlUPtr)curFileMetadata), NULL);

            if (HL_FAILED(result)) return result;

            /* Increase data node count. */
            ++fileDataNodeCount;
        }

        /* Allocate file data node indices buffer. */
        curTypeMetadata->tmpFileDataNodeIndices = HL_RESIZE_ARR(HlS32,
            fileDataNodeCount, curTypeMetadata->tmpFileDataNodeIndices);

        if (!curTypeMetadata->tmpFileDataNodeIndices) return HL_ERROR_OUT_OF_MEMORY;

        /* Store data node count in type metadata. */
        curTypeMetadata->tmpFileDataNodeCount = fileDataNodeCount;

        /* Fix type node data offset. */
        result = hlStreamFixOff64(stream, 0, *curOffPos +
            offsetof(HlPACxV3Node, dataOffset), hlStreamTell(stream),
            hlBINANeedsSwap(endianFlag), offTable);

        if (HL_FAILED(result)) return result;

        /* Write file tree and placeholder file nodes. */
        result = hlINPACxV3WriteNodeTree(&curTypeMetadata->tmpFileTree,
            fileDataNodeCount, endianFlag, curTypeMetadata->tmpFileDataNodeIndices,
            strTable, offTable, stream);

        if (HL_FAILED(result)) return result;

        /* Increase current offset position. */
        *curOffPos += sizeof(HlPACxV3Node);
    }

    /* Recurse through child nodes and return result. */
    for (i = 0; i < radixNode->children.count; ++i)
    {
        result = hlINPACxV3FillInTypeNodes(&radixNode->children.data[i],
            splitIndex, endianFlag, curOffPos, tmpFileNameBuf,
            strTable, offTable, stream);

        if (HL_FAILED(result)) return result;
    }

    return result;
}

static HlResult hlINPACxV3FillInDataNodeIndices(const HlRadixTree* HL_RESTRICT radixTree,
    const HlS32* HL_RESTRICT dataNodeIndices, HlU32 dataNodeCount,
    HlBINAEndianFlag endianFlag, size_t* HL_RESTRICT curOffPos,
    HlOffTable* HL_RESTRICT offTable, HlStream* HL_RESTRICT stream)
{
    HlResult result;

    /* Fix data node indices offset. */
    result = hlStreamFixOff64(stream, 0, *curOffPos + offsetof(
        HlPACxV3NodeTree, dataNodeIndicesOffset), hlStreamTell(stream),
        hlBINANeedsSwap(endianFlag), offTable);

    if (HL_FAILED(result)) return result;

    /* Increase current offset position. */
    *curOffPos += sizeof(HlPACxV3NodeTree);
    *curOffPos += (sizeof(HlPACxV3Node) * (radixTree->totalNodeCount +
        dataNodeCount));

    /* Write data node indices. */
    result = hlStreamWrite(stream, sizeof(HlS32) * dataNodeCount,
        dataNodeIndices, NULL);

    if (HL_FAILED(result)) return result;

    /* Write padding and return result. */
    return hlStreamPad(stream, 8);
}

static HlResult hlINPACxV3FillInFileDataNodeIndices(
    const HlRadixNode* HL_RESTRICT radixNode,
    HlBINAEndianFlag endianFlag, size_t* HL_RESTRICT curOffPos,
    HlOffTable* HL_RESTRICT offTable, HlStream* HL_RESTRICT stream)
{
    size_t i;
    HlResult result = HL_RESULT_SUCCESS;

    /* Fill-in file tree data node indices if this type node has data. */
    if (radixNode->customData)
    {
        const HlINPACxV3TypeMetadata* curTypeMetadata =
            (const HlINPACxV3TypeMetadata*)((HlUPtr)radixNode->customData);

        /* Fill-in file tree data node indices. */
        result = hlINPACxV3FillInDataNodeIndices(&curTypeMetadata->tmpFileTree,
            curTypeMetadata->tmpFileDataNodeIndices,
            curTypeMetadata->tmpFileDataNodeCount,
            endianFlag, curOffPos, offTable, stream);
        
        if (HL_FAILED(result)) return result;
    }

    /* Recurse through child nodes and return result. */
    for (i = 0; i < radixNode->children.count; ++i)
    {
        result = hlINPACxV3FillInFileDataNodeIndices(
            &radixNode->children.data[i], endianFlag,
            curOffPos, offTable, stream);

        if (HL_FAILED(result)) return result;
    }

    return result;
}

static size_t hlINPACxV3GetTotalChildNodeCount(const HlRadixNode* radixNode)
{
    size_t i, totalChildCount = radixNode->children.count;

    if (radixNode->customData)
    {
        ++totalChildCount;
    }

    for (i = 0; i < radixNode->children.count; ++i)
    {
        totalChildCount += hlINPACxV3GetTotalChildNodeCount(
            &radixNode->children.data[i]);
    }

    return totalChildCount;
}

static HlResult hlINPACxV3FillInChildNodeIndices(
    const HlRadixNode* HL_RESTRICT radixNode,
    HlS32 globalIndex, HlBINAEndianFlag endianFlag,
    size_t* HL_RESTRICT curOffPos, HlOffTable* HL_RESTRICT offTable,
    HlStream* HL_RESTRICT stream)
{
    size_t i;
    HlS32 childIndex;
    HlResult result;

    /* Don't write anything if there aren't any child nodes. */
    if (radixNode->children.count || radixNode->customData)
    {
        /* Fix child node indices offset. */
        result = hlStreamFixOff64(stream, 0, *curOffPos + offsetof(
            HlPACxV3Node, childIndicesOffset), hlStreamTell(stream),
            hlBINANeedsSwap(endianFlag), offTable);

        if (HL_FAILED(result)) return result;
    }

    /* Increase current offset position. */
    *curOffPos += sizeof(HlPACxV3Node);

    /* Write data node child node index. */
    if (radixNode->customData)
    {
        /* Endian-swap child node index if necessary. */
        HlS32 childIndexSwapped = ++globalIndex;
        if (hlBINANeedsSwap(endianFlag))
        {
            hlSwapS32P(&childIndexSwapped);
        }

        /* Write child node index. */
        result = hlStreamWrite(stream, sizeof(childIndexSwapped),
            &childIndexSwapped, NULL);

        if (HL_FAILED(result)) return result;

        /* Increase current offset position. */
        *curOffPos += sizeof(HlPACxV3Node);
    }

    /* Get child index. */
    childIndex = ++globalIndex;

    /* Write child node indices. */
    for (i = 0; i < radixNode->children.count; ++i)
    {
        /* Endian-swap child node index if necessary. */
        HlS32 childIndexSwapped = childIndex;
        if (hlBINANeedsSwap(endianFlag))
        {
            hlSwapS32P(&childIndexSwapped);
        }

        /* Write child node index. */
        result = hlStreamWrite(stream, sizeof(childIndexSwapped),
            &childIndexSwapped, NULL);

        if (HL_FAILED(result)) return result;

        /* Increase child index. */
        childIndex += (HlS32)(hlINPACxV3GetTotalChildNodeCount(
            &radixNode->children.data[i]) + 1);
    }

    /* Write padding. */
    result = hlStreamPad(stream, 8);
    if (HL_FAILED(result)) return result;

    /* Recurse through child nodes and return result. */
    for (i = 0; i < radixNode->children.count; ++i)
    {
        /* Recurse through child nodes */
        result = hlINPACxV3FillInChildNodeIndices(&radixNode->children.data[i],
            globalIndex, endianFlag, curOffPos, offTable, stream);

        if (HL_FAILED(result)) return result;

        /* Increase global index. */
        globalIndex += (HlS32)(hlINPACxV3GetTotalChildNodeCount(
            &radixNode->children.data[i]) + 1);
    }

    return result;
}

static HlResult hlINPACxV3FillInFileChildNodeIndices(
    const HlRadixNode* HL_RESTRICT radixNode,
    HlBINAEndianFlag endianFlag, size_t* HL_RESTRICT curOffPos,
    HlOffTable* HL_RESTRICT offTable, HlStream* HL_RESTRICT stream)
{
    size_t i;
    HlResult result = HL_RESULT_SUCCESS;

    /* Fill-in file tree child node indices if this type node has data. */
    if (radixNode->customData)
    {
        const HlINPACxV3TypeMetadata* curTypeMetadata =
            (const HlINPACxV3TypeMetadata*)((HlUPtr)radixNode->customData);

        /* Increase current offset position. */
        *curOffPos += sizeof(HlPACxV3NodeTree);

        /* Fill-in file tree child node indices. */
        result = hlINPACxV3FillInChildNodeIndices(
            &curTypeMetadata->tmpFileTree.rootNode,
            0, endianFlag, curOffPos, offTable, stream);

        if (HL_FAILED(result)) return result;
    }

    /* Recurse through child nodes and return result. */
    for (i = 0; i < radixNode->children.count; ++i)
    {
        result = hlINPACxV3FillInFileChildNodeIndices(
            &radixNode->children.data[i], endianFlag,
            curOffPos, offTable, stream);

        if (HL_FAILED(result)) return result;
    }

    return result;
}

static HlResult hlINPACxV402WritePlaceholderChunks(HlU32 maxChunkSize,
    const HlINPACxV3PacMetadata* HL_RESTRICT pacMetadata,
    HlStream* HL_RESTRICT stream)
{
    size_t uncompressedSize = pacMetadata->uncompressedSize,
        placeholderChunkSize = 0;

    /* Compute placeholder chunk size. */
    while (uncompressedSize > 0)
    {
        /* Compute current chunk size. */
        const size_t curChunkSize = HL_MIN(uncompressedSize, maxChunkSize);

        /* Account for chunk. */
        placeholderChunkSize += sizeof(HlPACxV402Chunk);

        /* Decrease sizes. */
        uncompressedSize -= curChunkSize;
    }

    /* If no placeholder chunks needs to be written, return early. */
    if (!placeholderChunkSize) return HL_RESULT_SUCCESS;

    /* Write placeholder chunks and return result. */
    return hlStreamWriteNulls(stream, placeholderChunkSize, NULL);
}

static HlResult hlINPACxV3WriteSplitTable(HlU32 version, HlU32 maxChunkSize,
    HlINPACxV3PacMetadataList* HL_RESTRICT splitMetadata,
    HlBINAEndianFlag endianFlag, HlStrTable* HL_RESTRICT strTable,
    HlOffTable* HL_RESTRICT offTable, HlStream* HL_RESTRICT stream)
{
    HlResult result;

    /* Write split table. */
    {
        /* Generate split table. */
        const size_t splitTablePos = hlStreamTell(stream);
        const size_t splitEntriesOffPos = (splitTablePos +
            offsetof(HlPACxV3SplitTable, splitEntries));

        HlPACxV3SplitTable splitTable =
        {
            (HlU64)splitMetadata->count,                        /* splitCount */
            (HlU64)(splitTablePos + sizeof(HlPACxV3SplitTable)) /* splitEntries */
        };

        /* Endian-swap split table if necessary. */
        if (hlBINANeedsSwap(endianFlag))
        {
            /* TODO: Endian-swap split table. */
        }

        /* Write split table. */
        result = hlStreamWrite(stream, sizeof(splitTable), &splitTable, NULL);
        if (HL_FAILED(result)) return result;

        /* Add split entries offset to offset table. */
        result = HL_LIST_PUSH(*offTable, splitEntriesOffPos);
        if (HL_FAILED(result)) return result;
    }

    /* Write split entries. */
    {
        size_t curOffPos = hlStreamTell(stream), i;
        switch (version)
        {
        case HL_IN_PACX_VER_301:
            /* Write placeholder offsets. */
            result = hlStreamWriteNulls(stream, sizeof(HlPACxV3SplitEntry) *
                splitMetadata->count, NULL);

            if (HL_FAILED(result)) return result;

            /* Fill-in string offsets and return result. */
            for (i = 0; i < splitMetadata->count; ++i)
            {
                /* Add string offset to string table. */
                result = hlStrTableAddStrRefUTF8(strTable,
                    splitMetadata->data[i].name, curOffPos);

                if (HL_FAILED(result)) return result;

                /* Increase current offset position. */
                curOffPos += sizeof(HlPACxV3SplitEntry);
            }

            return result;

        case HL_IN_PACX_VER_402:
            /* Write placeholder split entries. */
            for (i = 0; i < splitMetadata->count; ++i)
            {
                /* Generate split entry. */
                HlPACxV402SplitEntry splitEntry =
                {
                    0,                                              /* name */
                    (HlU32)splitMetadata->data[i].blob->size,       /* compressedSize */
                    (HlU32)splitMetadata->data[i].uncompressedSize, /* uncompressedSize */
                    0,                                              /* splitPos */
                    (HlU32)splitMetadata->data[i].chunks.count,     /* chunkCount */
                    0                                               /* chunksOffset */
                };

                /* Endian-swap if necessary. */
                if (hlBINANeedsSwap(endianFlag))
                {
                    /* TODO Endian-swap split entry. */
                }

                /* Write split entry. */
                result = hlStreamWrite(stream, sizeof(splitEntry), &splitEntry, NULL);
                if (HL_FAILED(result)) return result;
            }

            /* Fill-in offsets, write chunks, and return result. */
            for (i = 0; i < splitMetadata->count; ++i)
            {
                HlINPACxV3PacMetadata* curSplitMetadata = &splitMetadata->data[i];
                const size_t chunksPos = hlStreamTell(stream);

                /* Add string offset to string table. */
                result = hlStrTableAddStrRefUTF8(strTable,
                    curSplitMetadata->name, curOffPos +
                    offsetof(HlPACxV402SplitEntry, nameOffset));

                if (HL_FAILED(result)) return result;

                /* Fix chunks offset. */
                result = hlStreamFixOff64(stream, 0, curOffPos + offsetof(
                    HlPACxV402SplitEntry, chunksOffset), chunksPos,
                    hlBINANeedsSwap(endianFlag), offTable);

                if (HL_FAILED(result)) return result;

                /*
                   Write placeholder chunks.

                   NOTE: Yes, this often results in plenty of extra nulls
                   being written for no reason at all. For some reason
                   this is how Sonic Team actually does it.
                */
                result = hlINPACxV402WritePlaceholderChunks(maxChunkSize,
                    curSplitMetadata, stream);

                if (HL_FAILED(result)) return result;

                /* Write real chunks if this split is compressed. */
                if (curSplitMetadata->blob->size != curSplitMetadata->uncompressedSize)
                {
                    const size_t eof = hlStreamTell(stream);

                    /* Endian-swap chunks if necessary. */
                    if (hlBINANeedsSwap(endianFlag))
                    {
                        size_t i2;
                        for (i2 = 0; i2 < curSplitMetadata->chunks.count; ++i2)
                        {
                            /* TODO: Endian-swap chunk */
                        }
                    }

                    /* Jump to chunks position. */
                    result = hlStreamJumpTo(stream, chunksPos);
                    if (HL_FAILED(result)) return result;

                    /* Write chunks. */
                    result = hlStreamWrite(stream, sizeof(HlPACxV402Chunk) *
                        curSplitMetadata->chunks.count, curSplitMetadata->chunks.data,
                        NULL);

                    if (HL_FAILED(result)) return result;

                    /* Jump to end of stream position. */
                    result = hlStreamJumpTo(stream, eof);
                    if (HL_FAILED(result)) return result;
                }

                /* Increase current offset position. */
                curOffPos += sizeof(HlPACxV402SplitEntry);
            }

            return result;

        default: return HL_ERROR_UNSUPPORTED;
        }
    }
}

static HlResult hlINPACxV3WriteDataEntry(
    const HlINPACxV3FileMetadata* HL_RESTRICT fileMetadata,
    HlU32 version, HlU32 uid, HlU64 dataType, HlBINAEndianFlag endianFlag,
    HlStrTable* HL_RESTRICT strTable, HlStream* HL_RESTRICT stream)
{
    /* Generate placeholder data entry. */
    const size_t dataEntryPos = hlStreamTell(stream);
    const HlNChar* ext = fileMetadata->ext;
    const HlU32 dataUID = (hlBINAGetMajorVersionChar(version) >= '4') ?
        hlPACxV3GenerateUID() : uid;

    HlPACxV3DataEntry dataEntry =
    {
        dataUID,                            /* uid */
        (HlU32)fileMetadata->entry->size,   /* dataSize */
        0,                                  /* unknown2 */
        0,                                  /* dataOffset */
        0,                                  /* unknown3 */
        0,                                  /* extOffset */
        dataType                            /* dataType */
    };

    HlResult result;

    /* Endian-swap data entry if necessary. */
    if (hlBINANeedsSwap(endianFlag))
    {
        /* TODO */
    }

    /* Write data entry. */
    result = hlStreamWrite(stream, sizeof(dataEntry), &dataEntry, NULL);
    if (HL_FAILED(result)) return result;

    /* Skip beginning dot in extension, if any. */
    if (*ext == HL_NTEXT('.'))
    {
        ++ext;
    }

    /* Add file extension to string table and return result. */
    return hlStrTableAddStrRefNative(strTable, ext, dataEntryPos +
        offsetof(HlPACxV3DataEntry, extOffset));
}

static HlResult hlINPACxV3WriteFileDataEntries(
    const HlRadixNode* HL_RESTRICT radixNode, unsigned short splitIndex,
    HlU32 version, HlU32 uid, HlBINAEndianFlag endianFlag, size_t* HL_RESTRICT curOffPos,
    HlStrTable* HL_RESTRICT strTable, HlOffTable* HL_RESTRICT offTable,
    HlStream* HL_RESTRICT stream)
{
    size_t i;
    HlResult result = HL_RESULT_SUCCESS;

    /* Increase current offset position. */
    *curOffPos += sizeof(HlPACxV3Node);

    if (radixNode->customData)
    {
        const HlINPACxV3FileMetadata* curFileMetadata = (const HlINPACxV3FileMetadata*)
            ((HlUPtr)radixNode->customData);

        const HlBool isHere = (curFileMetadata->splitIndex == splitIndex);

        /* Fix file node data offset. */
        result = hlStreamFixOff64(stream, 0, *curOffPos + offsetof(
            HlPACxV3Node, dataOffset), hlStreamTell(stream),
            hlBINANeedsSwap(endianFlag), offTable);

        if (HL_FAILED(result)) return result;

        /* Write placeholder data entry. */
        result = hlINPACxV3WriteDataEntry(curFileMetadata, version, uid,
            (isHere) ? HL_PACXV3_DATA_TYPE_REGULAR_FILE : HL_PACXV3_DATA_TYPE_NOT_HERE,
            endianFlag, strTable, stream);

        if (HL_FAILED(result)) return result;

        /* Increase current offset position. */
        *curOffPos += sizeof(HlPACxV3Node);
    }
    
    /* Recurse through child nodes and return result. */
    for (i = 0; i < radixNode->children.count; ++i)
    {
        result = hlINPACxV3WriteFileDataEntries(
            &radixNode->children.data[i], splitIndex, version,
            uid, endianFlag, curOffPos, strTable, offTable,
            stream);

        if (HL_FAILED(result)) return result;
    }

    return result;
}

static HlResult hlINPACxV3WriteDataEntries(
    const HlRadixNode* HL_RESTRICT radixNode, unsigned short splitIndex,
    HlU32 version, HlU32 uid, HlBINAEndianFlag endianFlag, size_t* HL_RESTRICT curOffPos,
    HlStrTable* HL_RESTRICT strTable, HlOffTable* HL_RESTRICT offTable,
    HlStream* HL_RESTRICT stream)
{
    size_t i;
    HlResult result = HL_RESULT_SUCCESS;

    /* Write data entries and fill-in file nodes if this type node has data. */
    if (radixNode->customData)
    {
        const HlINPACxV3TypeMetadata* curTypeMetadata =
            (const HlINPACxV3TypeMetadata*)((HlUPtr)radixNode->customData);

        /* Increase current offset position. */
        *curOffPos += sizeof(HlPACxV3NodeTree);

        /* Write placeholder data entries and fill-in file nodes. */
        result = hlINPACxV3WriteFileDataEntries(&curTypeMetadata->tmpFileTree.rootNode,
            splitIndex, version, uid, endianFlag, curOffPos, strTable, offTable, stream);

        if (HL_FAILED(result)) return result;
    }

    /* Recurse through child nodes and return result. */
    for (i = 0; i < radixNode->children.count; ++i)
    {
        result = hlINPACxV3WriteDataEntries(
            &radixNode->children.data[i], splitIndex, version,
            uid, endianFlag, curOffPos, strTable, offTable,
            stream);

        if (HL_FAILED(result)) return result;
    }

    return result;
}

static HlResult hlINPACxV3FillInDataEntry(size_t dataEntryPos,
    const HlINPACxV3FileMetadata* HL_RESTRICT fileMetadata,
    HlU64 fileDataPos, HlU64 dataType, HlBINAEndianFlag endianFlag,
    HlOffTable* HL_RESTRICT offTable, HlStream* HL_RESTRICT stream)
{
    const size_t eof = hlStreamTell(stream);
    HlResult result;

    /* Endian-swap values if necessary. */
    if (hlBINANeedsSwap(endianFlag))
    {
        hlSwapU64P(&fileDataPos);
        hlSwapU64P(&dataType);
    }

    /* Jump to data entry dataOffset position. */
    result = hlStreamJumpTo(stream, dataEntryPos + offsetof(
        HlPACxV3DataEntry, dataOffset));

    if (HL_FAILED(result)) return result;

    /* Fill-in dataOffset. */
    result = hlStreamWrite(stream, sizeof(fileDataPos), &fileDataPos, NULL);
    if (HL_FAILED(result)) return result;

    /* Add dataOffset to offset table. */
    result = HL_LIST_PUSH(*offTable, dataEntryPos + offsetof(
        HlPACxV3DataEntry, dataOffset));

    if (HL_FAILED(result)) return result;

    /* Jump to data entry dataType position. */
    result = hlStreamJumpTo(stream, dataEntryPos + offsetof(
        HlPACxV3DataEntry, dataType));

    if (HL_FAILED(result)) return result;

    /* Fill-in dataType. */
    result = hlStreamWrite(stream, sizeof(dataType), &dataType, NULL);
    if (HL_FAILED(result)) return result;

    /* Jump to end of stream position. */
    return hlStreamJumpTo(stream, eof);
}

static HlResult hlINPACxV3WriteFileData(
    const HlINPACxV3FileMetadata* HL_RESTRICT fileMetadata,
    size_t dataEntryPos, HlBINAEndianFlag endianFlag,
    HlU32 dataAlignment, HlPackedFileIndex* HL_RESTRICT pfi,
    HlOffTable* HL_RESTRICT offTable, HlStream* HL_RESTRICT stream)
{
    const void* entryData;
    void* tmpEntryDataBuf = NULL;
    size_t fileDataPos;
    HlU64 dataType = HL_PACXV3_DATA_TYPE_REGULAR_FILE;
    HlResult result;

    /* Pad data entry to requested data alignment. */
    result = hlStreamPad(stream, dataAlignment);
    if (HL_FAILED(result)) return result;

    /* Get file data position. */
    fileDataPos = hlStreamTell(stream);

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

    /* Mark whether this data is BINA data or not. */
    /* TODO: Do these games actually support BINAV1? */
    if ((fileMetadata->entry->size >= sizeof(HlBINAV2Header) &&
        hlBINAHasV2Header(entryData)) || (fileMetadata->entry->size >=
            sizeof(HlBINAV1Header) && hlBINAHasV1Header(entryData)))
    {
        dataType = HL_PACXV3_DATA_TYPE_BINA_FILE;
    }

    /* Write data, then free it as necessary. */
    result = hlStreamWrite(stream, fileMetadata->entry->size, entryData, NULL);
    hlFree(tmpEntryDataBuf);

    if (HL_FAILED(result)) return result;

    /* Add packed file entry to packed file index if necessary. */
    if (pfi)
    {
        /* Generate packed file entry. */
        HlPackedFileEntry packedEntry;
        result = hlPackedFileEntryConstruct(fileMetadata->name,
            (HlU32)fileDataPos, (HlU32)fileMetadata->entry->size,
            &packedEntry);

        if (HL_FAILED(result)) return result;

        /* Add packed file entry to packed file index. */
        result = HL_LIST_PUSH(pfi->entries, packedEntry);
        if (HL_FAILED(result))
        {
            hlPackedFileEntryDestruct(&packedEntry);
            return result;
        }
    }

    /* Fill-in data entry and return result. */
    return hlINPACxV3FillInDataEntry(dataEntryPos, fileMetadata,
        fileDataPos, dataType, endianFlag, offTable, stream);
}

static HlResult hlINPACxV4FixSplitPositions(HlU32 version,
    size_t splitEntriesPos, HlU32 maxChunkSize, HlBINAEndianFlag endianFlag,
    HlINPACxV3PacMetadataList* HL_RESTRICT splitMetadata,
    HlStream* HL_RESTRICT stream)
{
    const size_t eof = hlStreamTell(stream);
    size_t i;
    HlResult result = HL_RESULT_SUCCESS;

    switch (hlBINAGetRevisionVersionChar(version))
    {
    case '2':
    {
        size_t uncompressedSize = eof;
        size_t curSplitPos = sizeof(HlPACxV402Header);

        /* Compute first split position. */
        while (uncompressedSize > 0)
        {
            /* Compute current chunk size. */
            const size_t curChunkSize = HL_MIN(uncompressedSize, maxChunkSize);

            /* Account for chunk. */
            curSplitPos += sizeof(HlPACxV402Chunk);

            /* Increase pointer and decrease size. */
            uncompressedSize -= curChunkSize;
        }

        /* Fix split positions. */
        for (i = 0; i < splitMetadata->count; ++i)
        {
            HlU32 curSplitPosSwapped;

            /* Account for padding. */
            curSplitPos = HL_ALIGN(curSplitPos, 16);

            /* Jump to split position. */
            result = hlStreamJumpTo(stream, splitEntriesPos +
                offsetof(HlPACxV402SplitEntry, splitPos));

            if (HL_FAILED(result)) return result;

            /* Endian-swap current split position if necessary. */
            curSplitPosSwapped = (HlU32)curSplitPos;
            if (hlBINANeedsSwap(endianFlag))
            {
                hlSwapU32P(&curSplitPosSwapped);
            }

            /* Fix split position. */
            result = hlStreamWrite(stream, sizeof(curSplitPosSwapped),
                &curSplitPosSwapped, NULL);

            if (HL_FAILED(result)) return result;

            /* Account for split data. */
            curSplitPos += splitMetadata->data[i].blob->size;
            
            /* Increase split entry position. */
            splitEntriesPos += sizeof(HlPACxV402SplitEntry);
        }

        break;
    }

    default: return HL_ERROR_UNSUPPORTED;
    }

    return result;
}

static HlResult hlINPACxV3Write(unsigned short splitIndex, HlU32 version,
    HlU32 uid, HlINPACxV3TypeMetadataList* HL_RESTRICT typeMetadata,
    HlU32 splitLimit, HlU32 dataAlignment, HlU32 maxChunkSize,
    HlBINAEndianFlag endianFlag, HlINPACxV3PacMetadataList* HL_RESTRICT splitMetadata,
    HlPackedFileIndex* HL_RESTRICT pfi, HlStream* HL_RESTRICT stream)
{
    char tmpFileNameBuf[256]; /* PACxV3 file names can only be up to 255 characters in length. */
    HlRadixTree typeTree;
    HlStrTable strTable;
    HlOffTable offTable;
    HlS32* typeDataNodeIndices = NULL;

    size_t i, treesPos, splitTablePos, dataEntriesPos,
        stringTablePos, fileDataPos, curOffPos, firstFileTreePos;

    HlU32 typeDataNodeCount = 0;
    HlResult result;

    const HlBool isRoot = (splitIndex == USHRT_MAX);
    HlU16 type = ((isRoot) ? HL_PACXV3_TYPE_IS_ROOT :
        HL_PACXV3_TYPE_IS_SPLIT);

    /* Initialize string and offset tables. */
    HL_LIST_INIT(strTable);
    HL_LIST_INIT(offTable);

    /* Construct type tree. */
    result = hlRadixTreeConstruct(0, &typeTree);
    if (HL_FAILED(result)) return result;

    /* Generate type nodes. */
    for (i = 0; i < typeMetadata->count; ++i)
    {
        HlINPACxV3TypeMetadata* curTypeMetadata = &typeMetadata->data[i];
        const char* pacxDataType;

        if (!isRoot)
        {
            /* Skip this type if we're writing a split and a file of this type is not in it. */
            if ((curTypeMetadata->files->pacxExt->flags & HL_PACX_EXT_FLAGS_TYPE_MASK) !=
                HL_PACX_EXT_FLAGS_ROOT_TYPE)
            {
                size_t i2;
                HlBool hasAtLeastOneFileOfThisType = HL_FALSE;

                for (i2 = 0; i2 < curTypeMetadata->fileCount; ++i2)
                {
                    if (curTypeMetadata->files[i2].splitIndex == splitIndex)
                    {
                        hasAtLeastOneFileOfThisType = HL_TRUE;
                        break;
                    }
                }

                if (!hasAtLeastOneFileOfThisType) continue;
            }
            else
            {
                continue;
            }
        }

        /* Get PACx data type. */
        pacxDataType = HlPACxDataTypes[curTypeMetadata->files->pacxExt->pacxDataTypeIndex];

        /* Insert PACx data type string into type tree. */
        result = hlRadixTreeInsert(&typeTree, pacxDataType,
            (HlUMax)((HlUPtr)curTypeMetadata), NULL);

        if (HL_FAILED(result)) goto failed;

        /* Mark roots has having splits if we have a split type and a split limit is set. */
        if (isRoot && splitLimit && (curTypeMetadata->files->pacxExt->flags &
            HL_PACX_EXT_FLAGS_SPLIT_TYPE))
        {
            type |= HL_PACXV3_TYPE_HAS_SPLITS;
        }

        /* Increase data node count. */
        ++typeDataNodeCount;
    }

    /* Allocate type data node indices buffer. */
    typeDataNodeIndices = HL_ALLOC_ARR(HlS32, typeDataNodeCount);
    if (!typeDataNodeIndices)
    {
        result = HL_ERROR_OUT_OF_MEMORY;
        goto failed;
    }

    /* Put unknown flag in type if necessary. */
    if (isRoot && hlBINAGetMajorVersionChar(version) >= '4')
    {
        type |= HL_PACXV3_TYPE_UNKNOWN;
    }

    /* Start writing PACx data. */
    result = hlPACxV3StartWrite(version, uid, type, endianFlag, stream);
    if (HL_FAILED(result)) goto failed;

    /* Get trees position. */
    treesPos = hlStreamTell(stream);

    /* Write type tree and placeholder type nodes (and get type data node indices). */
    result = hlINPACxV3WriteNodeTree(&typeTree,
        typeDataNodeCount, endianFlag, typeDataNodeIndices,
        &strTable, &offTable, stream);

    if (HL_FAILED(result)) goto failed;

    /* Write file trees and fill-in type nodes (and get file data node indices). */
    curOffPos = (treesPos + sizeof(HlPACxV3NodeTree));
    result = hlINPACxV3FillInTypeNodes(&typeTree.rootNode, splitIndex,
        endianFlag, &curOffPos, tmpFileNameBuf, &strTable, &offTable,
        stream);

    if (HL_FAILED(result)) goto failed;

    /* Write type data node indices and fill-in type trees. */
    curOffPos = treesPos;
    result = hlINPACxV3FillInDataNodeIndices(&typeTree,
        typeDataNodeIndices, typeDataNodeCount, endianFlag,
        &curOffPos, &offTable, stream);

    if (HL_FAILED(result)) goto failed;

    /* Write file data node indices and fill-in file trees. */
    result = hlINPACxV3FillInFileDataNodeIndices(&typeTree.rootNode,
        endianFlag, &curOffPos, &offTable, stream);

    if (HL_FAILED(result)) goto failed;

    /* Write type child node indices and fill-in type nodes. */
    curOffPos = (treesPos + sizeof(HlPACxV3NodeTree));
    result = hlINPACxV3FillInChildNodeIndices(&typeTree.rootNode,
        0, endianFlag, &curOffPos, &offTable, stream);

    if (HL_FAILED(result)) goto failed;

    /* Write file child node indices and fill-in file nodes. */
    firstFileTreePos = curOffPos;
    result = hlINPACxV3FillInFileChildNodeIndices(&typeTree.rootNode,
        endianFlag, &curOffPos, &offTable, stream);

    if (HL_FAILED(result)) goto failed;

    /* Get split table position. */
    splitTablePos = hlStreamTell(stream);

    /* Write split table if necessary. */
    if (isRoot && splitMetadata && splitMetadata->count > 0)
    {
        result = hlINPACxV3WriteSplitTable(version, maxChunkSize,
            splitMetadata, endianFlag, &strTable, &offTable, stream);

        if (HL_FAILED(result)) goto failed;
    }

    /* Get data enties position. */
    dataEntriesPos = hlStreamTell(stream);

    /* Write placeholder data entries. */
    curOffPos = firstFileTreePos;
    result = hlINPACxV3WriteDataEntries(&typeTree.rootNode,
        splitIndex, version, uid, endianFlag, &curOffPos,
        &strTable, &offTable, stream);

    if (HL_FAILED(result)) goto failed;

    /* Get string table position. */
    stringTablePos = hlStreamTell(stream);

    /* Write string table. */
    result = hlBINAStringsWrite64(0, endianFlag, &strTable, &offTable, stream);
    if (HL_FAILED(result)) goto failed;

    /* Get file data position. */
    fileDataPos = hlStreamTell(stream);

    /* Write file data and fill-in data entries. */
    curOffPos = dataEntriesPos;
    for (i = 0; i < typeMetadata->count; ++i)
    {
        const HlINPACxV3TypeMetadata* curTypeMetadata = &typeMetadata->data[i];
        size_t i2;

        /* Skip types that aren't in this pac. */
        if (!isRoot && (curTypeMetadata->files->pacxExt->flags &
            HL_PACX_EXT_FLAGS_TYPE_MASK) == HL_PACX_EXT_FLAGS_ROOT_TYPE)
        {
            continue;
        }

        /* Write file data and fill-in data entries. */
        for (i2 = 0; i2 < curTypeMetadata->fileCount; ++i2)
        {
            const HlINPACxV3FileMetadata* curFileMetadata = &curTypeMetadata->files[i2];

            /* Write file data and fill-in data entry if this file is in this pac. */
            if (curFileMetadata->splitIndex == splitIndex)
            {
                result = hlINPACxV3WriteFileData(curFileMetadata, curOffPos,
                    endianFlag, dataAlignment, pfi, &offTable, stream);

                if (HL_FAILED(result)) goto failed;
            }

            /* Skip files that aren't in this pac. */
            else if (!isRoot)
            {
                continue;
            }

            /* Increase current offset position. */
            curOffPos += sizeof(HlPACxV3DataEntry);
        }
    }

    /* Finish writing PACx data. */
    result = hlPACxV3FinishWrite(0,                                     /* headerPos */
        (HlU32)(splitTablePos - treesPos),                              /* treesSize */
        (HlU32)(dataEntriesPos - splitTablePos),                        /* splitTableSize */
        (HlU32)(stringTablePos - dataEntriesPos),                       /* dataEntriesSize */
        (HlU32)(fileDataPos - stringTablePos),                          /* stringTableSize */
        fileDataPos,                                                    /* fileDataPos */
        (isRoot && splitMetadata) ? (HlU32)splitMetadata->count : 0,    /* splitCount */
        endianFlag, &offTable, stream);                                 /* (self-explanatory) */

    if (HL_FAILED(result)) goto failed;

    /* Fix split positions if necessary. */
    if (isRoot && hlBINAGetMajorVersionChar(version) >= '4' &&
        splitMetadata && splitMetadata->count > 0)
    {
        result = hlINPACxV4FixSplitPositions(version, splitTablePos +
            sizeof(HlPACxV3SplitTable), maxChunkSize, endianFlag,
            splitMetadata, stream);

        if (HL_FAILED(result)) goto failed;
    }

failed:
    /* Free resources and return result. */
    hlFree(typeDataNodeIndices);
    hlRadixTreeDestruct(&typeTree);
    HL_LIST_FREE(offTable);
    hlStrTableDestruct(&strTable);

    return result;
}

static HlResult hlINPACxV3SaveSplits(unsigned short splitCount,
    HlINPACxV3TypeMetadataList* HL_RESTRICT typeMetadata,
    size_t filePathLen, HlU32 uid, HlU32 splitLimit, HlU32 dataAlignment, 
    HlBINAEndianFlag endianFlag, HlPackedFileIndex* HL_RESTRICT pfi,
    HlINPACxV3PacMetadataList* HL_RESTRICT splitMetadata, HlNChar* HL_RESTRICT pathBuf)
{
    HlFileStream* splitFile;
    const HlNChar* rootName = hlPathGetName(pathBuf);
    HlNChar* lastCharPtr = &pathBuf[filePathLen + 3];
    const size_t rootNameSize = (hlNStrLen(rootName) + 1);
    const size_t splitNameSize = (hlStrGetReqLenNativeToUTF8(rootName, rootNameSize) + 4);
    unsigned short splitIndex;
    HlResult result;

    /* Reserve space in split metadata list. */
    result = HL_LIST_RESERVE(*splitMetadata, splitCount);
    if (HL_FAILED(result)) return result;

    /* Copy initial split extension into path buffer. */

    /*
       HACK: '/' == 47, '0' == 48. ".00/" will be incremented to ".000"
       in the first call to hlINArchiveNextSplit3.
    */
    memcpy(&pathBuf[filePathLen], HL_NTEXT(".00/"), sizeof(HlNChar) * 5);

    /* Save splits as needed. */
    for (splitIndex = 0; splitIndex < splitCount; ++splitIndex)
    {
        HlINPACxV3PacMetadata curSplitMetadata;

        /* Get the next split file path. */
        if (!hlINArchiveNextSplit3(lastCharPtr))
            return HL_ERROR_OUT_OF_RANGE;

        /* Open the next split file for writing. */
        result = hlFileStreamOpen(pathBuf, HL_FILE_MODE_WRITE, &splitFile);
        if (HL_FAILED(result)) return result;

        /* Write split. */
        result = hlINPACxV3Write(splitIndex, HL_IN_PACX_VER_301, uid,
            typeMetadata, splitLimit, dataAlignment, 0, endianFlag,
            NULL, pfi, splitFile);

        if (HL_FAILED(result))
        {
            hlFileStreamClose(splitFile);
            return result;
        }

        /* Close split file. */
        result = hlFileStreamClose(splitFile);
        if (HL_FAILED(result)) return result;

        /* Construct new pac metadata. */
        result = hlINPACxV3PacMetadataConstruct(&curSplitMetadata,
            rootName, rootNameSize, splitNameSize);

        if (HL_FAILED(result)) return result;

        /* Add pac metadata to split metadata list. */
        result = HL_LIST_PUSH(*splitMetadata, curSplitMetadata);
        if (HL_FAILED(result))
        {
            hlINPACxV3PacMetadataDestruct(&curSplitMetadata);
            return result;
        }
    }

    return result;
}

static int hlINPACxV3CustomStrCmp(const HlNChar* HL_RESTRICT str1,
    const HlNChar* HL_RESTRICT str2, size_t maxCount)
{
    HlNChar c1 = 0, c2 = 0;
    while (maxCount && (c2 = *str2, c1 = *str1))
    {
        if (c1 != c2)
        {
            if (c1 <= HL_NTEXT('z') && c2 <= HL_NTEXT('z'))
            {
                if (c1 >= HL_NTEXT('A'))
                {
                    /* If c1 is upper-cased, lower-case it. */
                    if (c1 <= HL_NTEXT('Z'))
                    {
                        c1 += 32;
                    }

                    /* If c1 is an underscore, sort it with low priority. */
                    else if (c1 == HL_NTEXT('_'))
                    {
                        c1 = HL_NTEXT('\0');
                    }
                }

                if (c2 >= HL_NTEXT('A'))
                {
                    /* If c2 is upper-cased, lower-case it. */
                    if (c2 <= HL_NTEXT('Z'))
                    {
                        c2 += 32;
                    }

                    /* If c2 is an underscore, sort it with low priority. */
                    else if (c2 == HL_NTEXT('_'))
                    {
                        c2 = HL_NTEXT('\0');
                    }
                }

                if (c1 != c2) goto end;
            }
            else goto end;
        }

        ++str1;
        ++str2;
        --maxCount;
    }

end:
    if (maxCount == 0) return 0;

    return (((int)c1) - ((int)c2));
}

static int hlINPACxV3SortFileMetadata(const void* a, const void* b)
{
    const HlINPACxV3FileMetadata* fileMeta1 = (const HlINPACxV3FileMetadata*)a;
    const HlINPACxV3FileMetadata* fileMeta2 = (const HlINPACxV3FileMetadata*)b;

    /* Sort by extensions if extensions are not the same. */
    {
        const int extSortWeight = hlNStrICmpAsLowerCase(fileMeta1->ext, fileMeta2->ext);
        if (extSortWeight != 0) return extSortWeight;
    }

    /* Otherwise, sort by file name. */
    {
        const size_t fileNameLen1 = (fileMeta1->ext - fileMeta1->name);
        const size_t fileNameLen2 = (fileMeta2->ext - fileMeta2->name);
        const size_t fileNameMinLen = HL_MIN(fileNameLen1, fileNameLen2);
        const int nameSortWeight = hlINPACxV3CustomStrCmp(fileMeta1->name,
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

static int hlINPACxV3SortTypeMetadata(const void* a, const void* b)
{
    const HlINPACxV3TypeMetadata* typeMeta1 = (const HlINPACxV3TypeMetadata*)a;
    const HlINPACxV3TypeMetadata* typeMeta2 = (const HlINPACxV3TypeMetadata*)b;

    /* Get PACx data types. */
    const char* dataType1 = HlPACxDataTypes[typeMeta1->files->pacxExt->pacxDataTypeIndex];
    const char* dataType2 = HlPACxDataTypes[typeMeta2->files->pacxExt->pacxDataTypeIndex];

    /* Sory type names alphabetically. */
    return hlStrICmpAsLowerCase(dataType1, dataType2);
}

static HlResult hlINPACxV3GenerateMetadata(const HlArchive* HL_RESTRICT arc,
    const HlPACxSupportedExt* HL_RESTRICT exts, const size_t extCount,
    HlINPACxV3FileMetadata* HL_RESTRICT * HL_RESTRICT fileMetadata,
    size_t* HL_RESTRICT fileMetadataCount, HlBool* HL_RESTRICT hasSplitTypes,
    HlINPACxV3TypeMetadataList* HL_RESTRICT typeMetadata)
{
    HlINPACxV3TypeMetadata curTypeMetadata;
    size_t i;
    HlResult result = HL_RESULT_SUCCESS;

    /* Set default values. */
    *fileMetadataCount = 0;
    *hasSplitTypes = HL_FALSE;

    /* Allocate file metadata array. */
    *fileMetadata = HL_ALLOC_ARR(HlINPACxV3FileMetadata, arc->entries.count);
    if (!(*fileMetadata)) return HL_ERROR_OUT_OF_MEMORY;

    /* Generate file metadata. */
    for (i = 0; i < arc->entries.count; ++i)
    {
        HlINPACxV3FileMetadata* curFileMetadata = (*fileMetadata + *fileMetadataCount);
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

        /* Set default splitIndex. */
        curFileMetadata->splitIndex = USHRT_MAX;

        /* State if we have any split types. */
        if (curFileMetadata->pacxExt->flags & HL_PACX_EXT_FLAGS_SPLIT_TYPE)
        {
            *hasSplitTypes = HL_TRUE;
        }

        /* Increase file metadata count. */
        ++(*fileMetadataCount);
    }

    /* Generate type metadata if necessary. */
    if (*fileMetadataCount)
    {
        /* Sort file metadata based on PACx types. */
        qsort(*fileMetadata, *fileMetadataCount,
            sizeof(HlINPACxV3FileMetadata),
            hlINPACxV3SortFileMetadata);

        /* Setup type metadata for first type. */
        curTypeMetadata.files = *fileMetadata;
        curTypeMetadata.fileCount = 1;
        result = hlRadixTreeConstruct(0, &curTypeMetadata.tmpFileTree);
        curTypeMetadata.tmpFileDataNodeIndices = NULL;

        if (HL_FAILED(result)) return result;

        /* Setup and add type metadata for subsequent types. */
        for (i = 1; i < *fileMetadataCount; ++i)
        {
            if (curTypeMetadata.files->pacxExt == (*fileMetadata + i)->pacxExt)
            {
                ++curTypeMetadata.fileCount;
            }
            else
            {
                /* Add current type metadata to type metadata list. */
                result = HL_LIST_PUSH(*typeMetadata, curTypeMetadata);
                if (HL_FAILED(result))
                {
                    hlRadixTreeDestruct(&curTypeMetadata.tmpFileTree);
                    return result;
                }

                /* Setup type metadata for next type. */
                curTypeMetadata.files = (*fileMetadata + i);
                curTypeMetadata.fileCount = 1;
                result = hlRadixTreeConstruct(0, &curTypeMetadata.tmpFileTree);
                curTypeMetadata.tmpFileDataNodeIndices = NULL;

                if (HL_FAILED(result)) return result;
            }
        }

        /* Add final type metadata to type metadata list. */
        result = HL_LIST_PUSH(*typeMetadata, curTypeMetadata);
        if (HL_FAILED(result))
        {
            hlRadixTreeDestruct(&curTypeMetadata.tmpFileTree);
            return result;
        }

        /* Sort type metadata alphabetically. */
        qsort(typeMetadata->data, typeMetadata->count,
            sizeof(HlINPACxV3TypeMetadata),
            hlINPACxV3SortTypeMetadata);
    }

    return result;
}

static HlResult hlINPACxV3FileMetadataSplitUp(
    HlINPACxV3TypeMetadataList* HL_RESTRICT typeMetadata,
    HlU32 splitLimit, HlU32 dataAlignment,
    unsigned short* HL_RESTRICT splitCount)
{
    size_t i;
    HlU32 curSplitDataSize = 0;
    unsigned short splitIndex = 0;
    HlResult result = HL_RESULT_SUCCESS;

    /* Place files in splits. */
    for (i = 0; i < typeMetadata->count; ++i)
    {
        HlINPACxV3TypeMetadata* curTypeMetadata = &typeMetadata->data[i];
        size_t i2;

        /* Skip root types. */
        if ((curTypeMetadata->files->pacxExt->flags &
            HL_PACX_EXT_FLAGS_SPLIT_TYPE) == 0)
        {
            continue;
        }

        /* Split up split-typed files. */
        for (i2 = 0; i2 < curTypeMetadata->fileCount; ++i2)
        {
            HlINPACxV3FileMetadata* curFileMetadata = (HlINPACxV3FileMetadata*)
                &curTypeMetadata->files[i2];

            const HlU32 dataSize = (HlU32)curFileMetadata->entry->size;

            /* Check if adding this file to the current split would exceed the split limit. */
            if ((curSplitDataSize + dataSize) > splitLimit && curSplitDataSize != 0)
            {
                /* Increase split index and ensure we haven't exceeded 999 splits. */
                if (++splitIndex > 999)
                {
                    result = HL_ERROR_OUT_OF_RANGE;
                    goto end;
                }

                /* Reset current split data size. */
                curSplitDataSize = 0;
            }

            /* Account for data size. */
            curSplitDataSize += dataSize;

            /* Set splitIndex. */
            curFileMetadata->splitIndex = splitIndex;

            /* Account for data alignment in current split size. */
            curSplitDataSize = HL_ALIGN(curSplitDataSize, dataAlignment);
        }
    }

end:
    *splitCount = (splitIndex + 1);
    return result;
}

HlResult hlPACxV3SaveEx(const HlArchive* HL_RESTRICT arc,
    HlU32 splitLimit, HlU32 dataAlignment, HlBINAEndianFlag endianFlag,
    const HlPACxSupportedExt* HL_RESTRICT exts, const size_t extCount,
    HlPackedFileIndex* HL_RESTRICT pfi, const HlNChar* HL_RESTRICT filePath)
{
    HlNChar pathBuf[255];
    HlNChar* pathBufPtr = pathBuf;
    size_t pathBufCap = 255;

    HlINPACxV3TypeMetadataList typeMetadata;
    HlINPACxV3FileMetadata* fileMetadata = NULL;
    HlFileStream* rootFile = NULL;
    const size_t filePathLen = hlNStrLen(filePath);
    size_t fileMetadataCount = 0;
    const HlU32 uid = hlPACxV3GenerateUID();
    unsigned short splitCount = 0;
    HlResult result = HL_RESULT_SUCCESS;

    /* Verify that dataAlignment is a multiple of 4. */
    if ((dataAlignment % 4) != 0) return HL_ERROR_INVALID_ARGS;

    /* Setup path buffer. */
    result = hlINPACxSaveSetupPathBuffer(filePath,
        splitLimit, filePathLen, pathBufCap, &pathBufPtr);

    if (HL_FAILED(result)) return result;

    /* Generate file and type metadata. */
    HL_LIST_INIT(typeMetadata);
    if (arc->entries.count > 0)
    {
        HlBool hasSplitTypes;
        result = hlINPACxV3GenerateMetadata(arc, exts, extCount,
            &fileMetadata, &fileMetadataCount, &hasSplitTypes,
            &typeMetadata);

        if (HL_FAILED(result)) goto failed_root_not_open;

        /* Split data up. */
        if (splitLimit && hasSplitTypes)
        {
            result = hlINPACxV3FileMetadataSplitUp(&typeMetadata,
                splitLimit, dataAlignment, &splitCount);

            if (HL_FAILED(result)) goto failed_root_not_open;
        }
    }

    /* Open root file. */
    result = hlFileStreamOpen(pathBufPtr, HL_FILE_MODE_WRITE, &rootFile);
    if (HL_FAILED(result)) goto failed_root_not_open;
    
    /* Write splits and root as necessary. */
    {
        HlINPACxV3PacMetadataList splitMetadata;
        HL_LIST_INIT(splitMetadata);

        /* Save splits if necessary. */
        if (splitCount)
        {
            /* Disable PFI generation if we're writing splits. */
            pfi = NULL;

            /* Save splits. */
            result = hlINPACxV3SaveSplits(splitCount, &typeMetadata,
                filePathLen, uid, splitLimit, dataAlignment, endianFlag,
                pfi, &splitMetadata, pathBufPtr);

            if (HL_FAILED(result))
            {
                hlINPACxV3PacMetadataListDestruct(&splitMetadata);
                goto failed;
            }
        }

        /* Write root. */
        result = hlINPACxV3Write(USHRT_MAX, HL_IN_PACX_VER_301, uid,
            &typeMetadata, splitLimit, dataAlignment, 0, endianFlag,
            &splitMetadata, pfi, rootFile);

        hlINPACxV3PacMetadataListDestruct(&splitMetadata);
        if (HL_FAILED(result)) goto failed;
    }

    /* Close root file and return result. */
    result = hlFileStreamClose(rootFile);
    goto end;

failed:
    /* Free everything as necessary and return result. */
    hlFileStreamClose(rootFile);

end:
failed_root_not_open:
    hlFree(fileMetadata);
    hlINPACxV3TypeMetadataListDestruct(&typeMetadata);

    if (pathBufPtr != pathBuf) hlFree(pathBufPtr);
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
    HlResult result;

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
        result = hlLZ4DecompressNoAlloc(compressedPtr,
            chunks[i].compressedSize, chunks[i].uncompressedSize,
            uncompressedPtr);

        if (HL_FAILED(result)) return result;

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

    if (HL_FAILED(result))
    {
        hlFree(uncompressedDataBuf);
        return result;
    }

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

size_t hlPACxV402CompressBound(size_t uncompressedSize)
{
    return hlLZ4CompressBound(uncompressedSize);
}

HlResult hlPACxV402CompressNoAlloc(const void* HL_RESTRICT uncompressedData,
    size_t uncompressedSize, size_t compressedBufSize, size_t maxChunkSize,
    size_t* HL_RESTRICT compressedSize, void* HL_RESTRICT compressedBuf,
    HlPACxV402ChunkList* HL_RESTRICT chunks)
{
    const HlU8* uncompressedPtr = (const HlU8*)uncompressedData;
    HlU8* compressedPtr = (HlU8*)compressedBuf;
    const HlU8* const endOfUncompressedData = (uncompressedPtr + uncompressedSize);
    size_t totalCompressedSize = 0;
    HlResult result = HL_RESULT_SUCCESS;

    while (uncompressedPtr < endOfUncompressedData)
    {
        /* Compute current chunk size. */
        const size_t curChunkSize = HL_MIN(uncompressedSize, maxChunkSize);
        size_t curCompressedSize;

        /* Compress chunk. */
        result = hlCompressNoAlloc(HL_COMPRESS_TYPE_LZ4, uncompressedPtr,
            curChunkSize, compressedBufSize, &curCompressedSize,
            compressedPtr);

        if (HL_FAILED(result)) return result;

        /* Add new chunk to chunks list if necessary. */
        if (chunks)
        {
            HlPACxV402Chunk curChunk =
            {
                (HlU32)curCompressedSize,   /* compressedSize */
                (HlU32)curChunkSize         /* uncompressedSize */
            };

            result = HL_LIST_PUSH(*chunks, curChunk);
            if (HL_FAILED(result)) return result;
        }

        /* Increase pointers and total compressed size. */
        uncompressedPtr += curChunkSize;
        compressedPtr += curCompressedSize;
        totalCompressedSize += curCompressedSize;

        /* Decrease sizes. */
        compressedBufSize -= curCompressedSize;
        uncompressedSize -= curChunkSize;
    }
    
    /* Set compressedSize and return result. */
    *compressedSize = totalCompressedSize;
    return result;
}

HlResult hlPACxV402Compress(const void* HL_RESTRICT uncompressedData,
    size_t uncompressedSize, size_t maxChunkSize, size_t* HL_RESTRICT compressedSize,
    void* HL_RESTRICT * HL_RESTRICT compressedData, HlPACxV402ChunkList* HL_RESTRICT chunks)
{
    void* compressedDataBuf;
    HlResult result;

    /* Allocate a buffer to hold the compressed data. */
    compressedDataBuf = hlAlloc(uncompressedSize);
    if (!compressedDataBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Compress the data. */
    result = hlPACxV402CompressNoAlloc(uncompressedData, uncompressedSize,
        uncompressedSize, maxChunkSize, compressedSize, compressedDataBuf,
        chunks);

    if (HL_FAILED(result))
    {
        hlFree(compressedDataBuf);
        return result;
    }

    /* Set compressedData pointer and return success. */
    *compressedData = compressedDataBuf;
    return HL_RESULT_SUCCESS;
}

HlResult hlPACxV402CompressBlob(const void* HL_RESTRICT uncompressedData,
    size_t uncompressedSize, size_t maxChunkSize,
    HlBlob* HL_RESTRICT * HL_RESTRICT compressedBlob,
    HlPACxV402ChunkList* HL_RESTRICT chunks)
{
    HlBlob* compressedBlobBuf;
    HlResult result;

    /* Create a blob to hold the compressed data. */
    result = hlBlobCreate(NULL, uncompressedSize,
        &compressedBlobBuf);

    if (HL_FAILED(result)) return result;

    /* Compress the data. */
    result = hlPACxV402CompressNoAlloc(uncompressedData,
        uncompressedSize, uncompressedSize, maxChunkSize,
        &compressedBlobBuf->size, compressedBlobBuf->data,
        chunks);

    if (HL_FAILED(result))
    {
        hlBlobFree(compressedBlobBuf);
        return result;
    }

    /* Set compressedBlob pointer and return success. */
    *compressedBlob = compressedBlobBuf;
    return HL_RESULT_SUCCESS;
}

size_t hlPACxV403CompressBound(size_t uncompressedSize)
{
    return hlZlibCompressBound(uncompressedSize);
}

HlResult hlPACxV403CompressNoAlloc(const void* HL_RESTRICT uncompressedData,
    size_t uncompressedSize, size_t compressedBufSize,
    size_t* HL_RESTRICT compressedSize, void* HL_RESTRICT compressedBuf)
{
    return hlCompressNoAlloc(HL_COMPRESS_TYPE_ZLIB,
        uncompressedData, uncompressedSize,
        compressedBufSize, compressedSize, compressedBuf);
}

HlResult hlPACxV403Compress(const void* HL_RESTRICT uncompressedData,
    size_t uncompressedSize, size_t* HL_RESTRICT compressedSize,
    void* HL_RESTRICT * HL_RESTRICT compressedData)
{
    return hlCompress(HL_COMPRESS_TYPE_ZLIB,
        uncompressedData, uncompressedSize,
        compressedSize, compressedData);
}

HlResult hlPACxV403CompressBlob(const void* HL_RESTRICT uncompressedData,
    size_t uncompressedSize, HlBlob* HL_RESTRICT * HL_RESTRICT compressedBlob)
{
    return hlCompressBlob(HL_COMPRESS_TYPE_ZLIB,
        uncompressedData, uncompressedSize,
        compressedBlob);
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

                result = hlPACxV403DecompressBlob(HL_ADD_OFFC(header, splitEntriesV403[i].splitPos),
                    splitEntriesV403[i].compressedSize, splitEntriesV403[i].uncompressedSize,
                    &curUncompressedPac);
            }

            else
            {
                const HlPACxV402SplitEntry* splitEntriesV402 =
                    (const HlPACxV402SplitEntry*)splitEntries;

                const HlPACxV402Chunk* chunks = (const HlPACxV402Chunk*)
                    hlOff64Get(&splitEntriesV402[i].chunksOffset);

                result = hlPACxV402DecompressBlob(HL_ADD_OFFC(header, splitEntriesV402[i].splitPos),
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

static HlResult hlINPACxV4CompressBlob(const void* HL_RESTRICT uncompressedData,
    size_t uncompressedSize, HlU32 version, HlU32 maxChunkSize,
    HlBlob* HL_RESTRICT * HL_RESTRICT blob, HlPACxV402ChunkList* HL_RESTRICT chunks)
{
    switch (hlBINAGetRevisionVersionChar(version))
    {
    case '2':
    {
        return hlPACxV402CompressBlob(uncompressedData, uncompressedSize,
            maxChunkSize, blob, chunks);
    }

    case '3':
    {
        return hlPACxV403CompressBlob(uncompressedData,
            uncompressedSize, blob);
    }

    default:
        return HL_ERROR_UNSUPPORTED;
    }
}

static HlResult hlINPACxV4GeneratePacData(unsigned short splitIndex, HlU32 version,
    HlU32 uid, HlINPACxV3TypeMetadataList* HL_RESTRICT typeMetadata,
    HlU32 splitLimit, HlU32 dataAlignment, HlU32 maxChunkSize, size_t* HL_RESTRICT totalSize,
    HlBINAEndianFlag endianFlag, HlINPACxV3PacMetadataList* HL_RESTRICT splitMetadata,
    HlINPACxV3PacMetadata* HL_RESTRICT rootMetadata)
{
    const HlBool isRoot = (splitIndex == USHRT_MAX);
    HlINPACxV3PacMetadata* curPacMetadata = (isRoot) ?
        rootMetadata : &splitMetadata->data[splitIndex];

    HlMemStream* internalFile;
    const void* uncompressedData;
    HlResult result;

    /* Open a memory stream for writing. */
    result = hlMemStreamOpenWrite(&internalFile);
    if (HL_FAILED(result)) return result;

    /* Write internal PACx data. */
    result = hlINPACxV3Write(splitIndex, version, uid,
        typeMetadata, splitLimit, dataAlignment, maxChunkSize,
        endianFlag, splitMetadata, NULL, internalFile);

    if (HL_FAILED(result))
    {
        hlMemStreamClose(internalFile);
        return result;
    }

    /* Compress PAC data if necessary. */
    if (totalSize)
    {
        /* Get uncompressed PACx data. */
        uncompressedData = hlMemStreamGetDataPtr(internalFile,
            &curPacMetadata->uncompressedSize);

        /* Increase total size if this is a split. */
        if (!isRoot)
        {
            *totalSize += curPacMetadata->uncompressedSize;
        }

        /* Compress data if the total size has exceeded maxChunkCount. */
        if (*totalSize > maxChunkSize)
        {
            /* Compress internal PACx data (and generate chunks if necessary). */
            result = hlINPACxV4CompressBlob(uncompressedData,
                curPacMetadata->uncompressedSize, version, maxChunkSize,
                &curPacMetadata->blob, &curPacMetadata->chunks);

            if (HL_FAILED(result))
            {
                hlMemStreamClose(internalFile);
                return result;
            }

            /* Close internal PACx memory stream and return result. */
            return hlMemStreamClose(internalFile);
        }
    }

    /* Otherwise, get copy of uncompressed PACx data. */
    result = hlMemStreamGetData(internalFile,
        &curPacMetadata->blob);

    if (HL_FAILED(result))
    {
        hlMemStreamClose(internalFile);
        return result;
    }

    /* Write PAC data uncompressed. */
    {
        HlPACxV402Chunk curChunk =
        {
            (HlU32)curPacMetadata->blob->size,  /* compressedSize */
            (HlU32)curPacMetadata->blob->size   /* uncompressedSize */
        };

        /* Set uncompressed size. */
        curPacMetadata->uncompressedSize = curPacMetadata->blob->size;

        /* Add new chunk to chunks list if necessary. */
        result = HL_LIST_PUSH(curPacMetadata->chunks, curChunk);
        if (HL_FAILED(result))
        {
            hlMemStreamClose(internalFile);
            return result;
        }

        /* Close internal PACx memory stream and return result. */
        return hlMemStreamClose(internalFile);
    }
}

static HlResult hlINPACxV4GenerateSplits(unsigned short splitCount,
    HlINPACxV3TypeMetadataList* HL_RESTRICT typeMetadata,
    size_t fileNameLen, HlU32 version, HlU32 uid, HlU32 splitLimit,
    HlU32 dataAlignment, HlU32 maxChunkSize, size_t* HL_RESTRICT totalSize,
    HlBINAEndianFlag endianFlag, HlINPACxV3PacMetadataList* HL_RESTRICT splitMetadata,
    HlNChar* HL_RESTRICT pathBuf)
{
    HlNChar* lastCharPtr = &pathBuf[fileNameLen + 3];
    const size_t rootNameSize = (hlNStrLen(pathBuf) + 1);
    const size_t splitNameSize = (hlStrGetReqLenNativeToUTF8(pathBuf, rootNameSize) + 4);
    unsigned short splitIndex;
    HlResult result;

    /* Reserve space in split metadata list. */
    result = HL_LIST_RESERVE(*splitMetadata, splitCount);
    if (HL_FAILED(result)) return result;

    /* Copy initial split extension into path buffer. */

    /*
       HACK: '/' == 47, '0' == 48. ".00/" will be incremented to ".000"
       in the first call to hlINArchiveNextSplit3.
    */
    memcpy(&pathBuf[fileNameLen], HL_NTEXT(".00/"), sizeof(HlNChar) * 5);

    /* Save splits as needed. */
    for (splitIndex = 0; splitIndex < splitCount; ++splitIndex)
    {
        HlINPACxV3PacMetadata curSplitMetadata;

        /* Get the next split file path. */
        if (!hlINArchiveNextSplit3(lastCharPtr))
            return HL_ERROR_OUT_OF_RANGE;

        /* Construct new pac metadata. */
        result = hlINPACxV3PacMetadataConstruct(&curSplitMetadata,
            pathBuf, rootNameSize, splitNameSize);

        if (HL_FAILED(result)) return result;

        /* Add split metadata to split metadata list. */
        result = HL_LIST_PUSH(*splitMetadata, curSplitMetadata);
        if (HL_FAILED(result))
        {
            hlINPACxV3PacMetadataDestruct(&curSplitMetadata);
            return result;
        }

        /* Generate split data. */
        result = hlINPACxV4GeneratePacData(splitIndex, version, uid,
            typeMetadata, splitLimit, dataAlignment, maxChunkSize,
            totalSize, endianFlag, splitMetadata, NULL);
        
        if (HL_FAILED(result)) return result;
    }

    return result;
}

static HlResult hlINPACxV4WriteAll(const HlArchive* HL_RESTRICT arc,
    HlU32 version, HlU32 splitLimit, HlU32 dataAlignment, HlU32 maxChunkSize,
    HlBool noCompress, HlBINAEndianFlag endianFlag,
    const HlPACxSupportedExt* HL_RESTRICT exts, const size_t extCount,
    const HlNChar* HL_RESTRICT fileName, HlStream* HL_RESTRICT stream)
{
    HlNChar pathBuf[255];
    HlNChar* pathBufPtr = pathBuf;
    size_t pathBufCap = 255;

    HlINPACxV3TypeMetadataList typeMetadata;
    HlINPACxV3FileMetadata* fileMetadata = NULL;
    HlINPACxV3PacMetadataList splitMetadata;
    HlINPACxV3PacMetadata rootMetadata = { 0 };
    const size_t fileNameLen = hlNStrLen(fileName);
    const size_t headerPos = hlStreamTell(stream);
    size_t totalSize = 0;
    size_t* totalSizePtr = (noCompress) ? NULL : &totalSize;
    size_t fileMetadataCount = 0, rootPos, eof;
    const HlU32 uid = hlPACxV3GenerateUID();
    unsigned short splitCount = 0;
    HlResult result = HL_RESULT_SUCCESS;

    /* Verify that dataAlignment is a multiple of 4. */
    if ((dataAlignment % 4) != 0) return HL_ERROR_INVALID_ARGS;

    /* Setup path buffer. */
    result = hlINPACxSaveSetupPathBuffer(fileName,
        splitLimit, fileNameLen, pathBufCap, &pathBufPtr);

    if (HL_FAILED(result)) return result;

    /* Generate file and type metadata. */
    HL_LIST_INIT(typeMetadata);
    HL_LIST_INIT(splitMetadata);

    if (arc->entries.count > 0)
    {
        HlBool hasSplitTypes;
        result = hlINPACxV3GenerateMetadata(arc, exts, extCount,
            &fileMetadata, &fileMetadataCount, &hasSplitTypes,
            &typeMetadata);

        if (HL_FAILED(result)) goto failed;

        /* Split data up if necessary. */
        if (splitLimit && hasSplitTypes)
        {
            /* Split data up. */
            result = hlINPACxV3FileMetadataSplitUp(&typeMetadata,
                splitLimit, dataAlignment, &splitCount);

            if (HL_FAILED(result)) goto failed;
        }
    }

    /* Generate splits if necessary. */
    if (splitCount)
    {
        result = hlINPACxV4GenerateSplits(splitCount, &typeMetadata,
            fileNameLen, version, uid, splitLimit, dataAlignment,
            maxChunkSize, totalSizePtr, endianFlag, &splitMetadata,
            pathBufPtr);

        if (HL_FAILED(result)) goto failed;
    }

    /* Generate root. */
    result = hlINPACxV4GeneratePacData(USHRT_MAX, version, uid,
        &typeMetadata, splitLimit, dataAlignment, maxChunkSize,
        totalSizePtr, endianFlag, &splitMetadata, &rootMetadata);

    if (HL_FAILED(result)) goto failed;

    /* Write header. */
    switch (hlBINAGetRevisionVersionChar(version))
    {
    case '2':
    {
        /* Generate header. */
        HlPACxV402Header header =
        {
            HL_PACX_SIG,                            /* signature */
            { '4', '0', '2' },                      /* version */
            endianFlag,                             /* endianFlag */
            uid,                                    /* uid */
            0,                                      /* fileSize */
            0,                                      /* rootOffset */
            (HlU32)rootMetadata.blob->size,         /* rootCompressedSize */
            (HlU32)rootMetadata.uncompressedSize,   /* rootUncompressedSize */
            HL_PACXV3_TYPE_IS_ROOT,                 /* flags */
            0x208,                                  /* unknown2 */
            (HlU32)rootMetadata.chunks.count        /* chunkCount */
        };

        size_t chunksPos, eof;

        /* Endian-swap header if necessary. */
        if (hlBINANeedsSwap(endianFlag))
        {
            /* TODO: Endian-swap */
        }

        /* Write header. */
        result = hlStreamWrite(stream, sizeof(header), &header, NULL);
        if (HL_FAILED(result)) goto failed;

        /* Store chunks position. */
        chunksPos = hlStreamTell(stream);

        /* Write placeholder chunks. */
        result = hlINPACxV402WritePlaceholderChunks(maxChunkSize,
            &rootMetadata, stream);

        if (HL_FAILED(result)) goto failed;

        /* Store end of stream position. */
        eof = hlStreamTell(stream);

        /* Jump to chunks position. */
        result = hlStreamJumpTo(stream, chunksPos);
        if (HL_FAILED(result)) goto failed;

        /* TODO: Endian-swap chunks if necessary. */

        /* Write chunks. */
        result = hlStreamWrite(stream, sizeof(HlPACxV402Chunk) *
            rootMetadata.chunks.count, rootMetadata.chunks.data,
            NULL);

        if (HL_FAILED(result)) goto failed;

        /* Jump to end of stream position. */
        result = hlStreamJumpTo(stream, eof);
        if (HL_FAILED(result)) goto failed;
        
        break;
    }

    default:
        result = HL_ERROR_UNSUPPORTED;
        goto failed;
    }

    /* Pad file. */
    result = hlStreamPad(stream, 16);
    if (HL_FAILED(result)) goto failed;

    /* Write split pacs. */
    {
        size_t i;
        for (i = 0; i < splitMetadata.count; ++i)
        {
            /* Write split data. */
            result = hlStreamWrite(stream, splitMetadata.data[i].blob->size,
                splitMetadata.data[i].blob->data, NULL);

            if (HL_FAILED(result)) goto failed;

            /* Pad file. */
            /* TODO: Is this necessary? */
            result = hlStreamPad(stream, 16);
            if (HL_FAILED(result)) goto failed;
        }
    }

    /* Store root position. */
    rootPos = hlStreamTell(stream);

    /* Write root data. */
    result = hlStreamWrite(stream, rootMetadata.blob->size,
        rootMetadata.blob->data, NULL);

    if (HL_FAILED(result)) goto failed;

    /* Pad file. */
    /* TODO: Is this necessary? */
    result = hlStreamPad(stream, 16);
    if (HL_FAILED(result)) goto failed;

    /* Store end of stream position. */
    eof = hlStreamTell(stream);

    /* Jump to header fileSize position. */
    result = hlStreamJumpTo(stream, headerPos +
        offsetof(HlPACxV4Header, fileSize));

    if (HL_FAILED(result)) goto failed;
    
    /* Fill-in header fileSize. */
    {
        HlU32 fileSize = (HlU32)(eof - headerPos);
        if (hlBINANeedsSwap(endianFlag))
        {
            hlSwapU32P(&fileSize);
        }

        result = hlStreamWrite(stream, sizeof(fileSize), &fileSize, NULL);
        if (HL_FAILED(result)) goto failed;
    }

    /* Fill-in header rootOffset. */
    {
        HlU32 rootOffset = (HlU32)(rootPos - headerPos);
        if (hlBINANeedsSwap(endianFlag))
        {
            hlSwapU32P(&rootOffset);
        }

        result = hlStreamWrite(stream, sizeof(rootOffset), &rootOffset, NULL);
        if (HL_FAILED(result)) goto failed;
    }

    /* Jump to end of stream and return result. */
    result = hlStreamJumpTo(stream, eof);
    goto end;

end:
failed:
    /* Free everything as necessary and return result. */
    hlINPACxV3PacMetadataListDestruct(&splitMetadata);
    hlINPACxV3PacMetadataDestruct(&rootMetadata);
    hlFree(fileMetadata);
    hlINPACxV3TypeMetadataListDestruct(&typeMetadata);

    if (pathBufPtr != pathBuf) hlFree(pathBufPtr);
    return result;
}

HlResult hlPACxV402Write(const HlArchive* HL_RESTRICT arc,
    HlU32 splitLimit, HlU32 dataAlignment, HlU32 maxChunkSize,
    HlBool noCompress, HlBINAEndianFlag endianFlag,
    const HlPACxSupportedExt* HL_RESTRICT exts, const size_t extCount,
    const HlNChar* HL_RESTRICT fileName, HlStream* HL_RESTRICT stream)
{
    return hlINPACxV4WriteAll(arc, HL_IN_PACX_VER_402, splitLimit,
        dataAlignment, maxChunkSize, noCompress, endianFlag, exts,
        extCount, fileName, stream);
}

HlResult hlPACxV403Write(const HlArchive* HL_RESTRICT arc,
    HlU32 splitLimit, HlU32 dataAlignment, HlBool noCompress,
    HlBINAEndianFlag endianFlag, const HlPACxSupportedExt* HL_RESTRICT exts,
    const size_t extCount, const HlNChar* HL_RESTRICT fileName,
    HlStream* HL_RESTRICT stream)
{
    return hlINPACxV4WriteAll(arc, HL_IN_PACX_VER_403, splitLimit,
        dataAlignment, 0, noCompress, endianFlag, exts,
        extCount, fileName, stream);
}

HlResult hlPACxV402SaveEx(const HlArchive* HL_RESTRICT arc,
    HlU32 splitLimit, HlU32 dataAlignment, HlU32 maxChunkSize,
    HlBool noCompress, HlBINAEndianFlag endianFlag,
    const HlPACxSupportedExt* HL_RESTRICT exts, const size_t extCount,
    const HlNChar* HL_RESTRICT filePath)
{
    HlFileStream* file;
    HlResult result;

    /* Open file for writing. */
    result = hlFileStreamOpen(filePath, HL_FILE_MODE_WRITE, &file);
    if (HL_FAILED(result)) return result;

    /* Write all PACxV402 data to file. */
    result = hlPACxV402Write(arc, splitLimit, dataAlignment,
        maxChunkSize, noCompress, endianFlag, exts, extCount,
        hlPathGetName(filePath), file);

    if (HL_FAILED(result))
    {
        hlFileStreamClose(file);
        return result;
    }

    /* Close file and return result. */
    return hlFileStreamClose(file);
}

HlResult hlPACxV403SaveEx(const HlArchive* HL_RESTRICT arc,
    HlU32 splitLimit, HlU32 dataAlignment, HlBool noCompress,
    HlBINAEndianFlag endianFlag, const HlPACxSupportedExt* HL_RESTRICT exts,
    const size_t extCount, const HlNChar* HL_RESTRICT filePath)
{
    HlFileStream* file;
    HlResult result;

    /* Open file for writing. */
    result = hlFileStreamOpen(filePath, HL_FILE_MODE_WRITE, &file);
    if (HL_FAILED(result)) return result;

    /* Write all PACxV403 data to file. */
    result = hlPACxV403Write(arc, splitLimit, dataAlignment,
        noCompress, endianFlag, exts, extCount,
        hlPathGetName(filePath), file);

    if (HL_FAILED(result))
    {
        hlFileStreamClose(file);
        return result;
    }

    /* Close file and return result. */
    return hlFileStreamClose(file);
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
