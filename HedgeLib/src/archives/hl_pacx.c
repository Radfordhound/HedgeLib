#include "hedgelib/archives/hl_pacx.h"
#include "hedgelib/io/hl_path.h"
#include "hedgelib/hl_endian.h"
#include "hedgelib/hl_compression.h"
#include "hedgelib/hl_text.h"
#include "../io/hl_in_path.h"
#include "../hl_in_assert.h"
#include "../depends/lz4/lz4.h"

static const char* const HlINPACxV2DependsType = "ResPacDepend";
const HlNChar HL_PACX_EXT[5] = HL_NTEXT(".pac");

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
            HlU32 i2;

            /* Swap file tree. */
            hlPACxV2NodeTreeSwap(fileTree, HL_FALSE);

            /* Swap data entries. */
            if (strcmp(strchr(typeStr, ':') + 1, HlINPACxV2DependsType))
            {
                for (i2 = 0; i2 < fileTree->nodeCount; ++i2)
                {
                    HlPACxV2DataEntry* dataEntry = (HlPACxV2DataEntry*)
                        hlOff32Get(&fileNodes[i2].data);

                    hlPACxV2DataEntrySwap(dataEntry);
                }
            }

            /* Swap split tables. */
            else
            {
                for (i2 = 0; i2 < fileTree->nodeCount; ++i2)
                {
                    HlPACxV2SplitTable* splitTable = (HlPACxV2SplitTable*)(
                        ((HlPACxV2DataEntry*)hlOff32Get(&fileNodes[i2].data)) + 1);

                    hlPACxV2SplitTableSwap(splitTable, HL_FALSE);
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
        curBlock = hlBINAV2BlockGetNext(curBlock);
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
                    Get the next offset's address - return early
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
    const HlPACxV2BlockDataHeader* dataBlock =
        (const HlPACxV2BlockDataHeader*)hlBINAV2GetDataBlock(pac);

    const HlU32* firstDataOffset;
    const char* strings;
    const HlU8 *offsets, *eof;
    HlResult result = HL_RESULT_SUCCESS;

    if (!dataBlock) return HL_ERROR_INVALID_DATA;

    /*
       Store NULL in depsFileTree pointer if a pointer was requested.
       
       (We'll replace it with the actual pointer later if a ResPacDepend
       file tree is found.)
    */
    if (depsFileTree) *depsFileTree = NULL;

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
            if (!strcmp(colonPtr + 1, HlINPACxV2DependsType))
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
            (const HlPACxV2BlockDataHeader*)hlBINAV2GetDataBlock(rootPac);

        splitFileTree = hlPACxV2DataGetFileTree(
            dataBlock, HlINPACxV2DependsType);
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
        break;

    case '3':
        result = hlINPACxV3LoadAllInto(rootPac, loadSplits,
            pathBuf, &pathBufPtr, &pathBufCap, pacs, hlArc);
        break;

    case '4':
        result = hlPACxV4ReadInto(rootPac->data,
            loadSplits, pacs, hlArc);
        break;

    default:
        result = HL_ERROR_UNSUPPORTED;
        break;
    }

end:
    /* Free path buffer if necessary and return result. */
    if (!pacs) hlBlobFree(rootPac);
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
