#include "hedgelib/archives/hl_pacx.h"
#include "hedgelib/io/hl_path.h"
#include "hedgelib/hl_endian.h"
#include "hedgelib/hl_compression.h"
#include "hedgelib/hl_text.h"
#include "../io/hl_in_path.h"
#include "../hl_in_assert.h"
#include "../depends/lz4/lz4.h"

static const char* const HlINPACxV2SplitType = "ResPacDepend";
const HlNChar HL_PACX_EXT[5] = HL_NTEXT(".pac");

/* Forward-declaration for function that appears later in the file. */
HlResult hlINPACxLoadSplits(const HlNChar* HL_RESTRICT filePath,
    HlU8 majorVersion, HlArchive* HL_RESTRICT * HL_RESTRICT archive);

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
        HlPACxV2NodeTree* typeTree = hlPACxV2DataGetTypeTree(dataBlock);
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
            if (strcmp(strchr(typeStr, ':') + 1, HlINPACxV2SplitType))
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
        HlPACxV2ProxyEntryTable* proxyEntryTable =
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

void hlPACxV2BlockHeaderFix(HlPACxV2BlockHeader* HL_RESTRICT blockHeader,
    HlU8 endianFlag, HlPACxV2Header* HL_RESTRICT header)
{
    switch (blockHeader->signature)
    {
    case HL_BINAV2_BLOCK_TYPE_DATA:
    {
        /* Swap block data header if necessary. */
        HlPACxV2BlockDataHeader* dataHeader = (HlPACxV2BlockDataHeader*)blockHeader;
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

    /* Return early if there are no blocks to fix. */
    if (!blockCount) return;

    /* Fix the first block's header. */
    hlPACxV2BlockHeaderFix(curBlock, endianFlag, header);

    /* Fix subsequent block headers, if any. */
    for (i = 1; i < blockCount; ++i)
    {
        /* Get next block. */
        curBlock = hlBINAV2BlockGetNext(curBlock);

        /* Fix this block's header. */
        hlPACxV2BlockHeaderFix(curBlock, endianFlag, header);
    }
}

void hlPACxV2Fix(HlBlob* blob)
{
    /* Swap header if necessary. */
    HlPACxV2Header* header = (HlPACxV2Header*)blob->data;
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

HlPACxV2NodeTree* hlPACxV2DataGetFileTree(
    HlPACxV2BlockDataHeader* HL_RESTRICT dataBlock,
    const char* HL_RESTRICT resType)
{
    HlPACxV2NodeTree* typeTree = hlPACxV2DataGetTypeTree(dataBlock);
    HlPACxV2Node* typeNodes = (HlPACxV2Node*)hlOff32Get(&typeTree->nodes);
    HlU32 i;

    for (i = 0; i < typeTree->nodeCount; ++i)
    {
        const char* typeStr = (const char*)hlOff32Get(&typeNodes[i].name);
        if (!strcmp(strchr(typeStr, ':') + 1, resType))
        {
            return (HlPACxV2NodeTree*)hlOff32Get(
                &typeNodes[i].data);
        }
    }

    return NULL;
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
    size_t extSrcLen, size_t extDstLen, HlArchive* HL_RESTRICT hlArcBuf)
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
        HlArchiveEntry entry;

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
            entry.path = HL_ALLOC_ARR(HlNChar, fileNameSize + extDstLen + 1);
            if (!entry.path) return HL_ERROR_OUT_OF_MEMORY;

            if (!hlStrConvUTF8ToNativeNoAlloc(fileName,
                entry.path, 0, fileNameSize))
            {
                hlFree(entry.path);
                return HL_ERROR_UNKNOWN;
            }

            /* Convert extension to native encoding and copy into buffer. */
            entry.path[fileNameSize - 1] = HL_NTEXT('.');

            if (!hlStrConvUTF8ToNativeNoAlloc(typeStr,
                &entry.path[fileNameSize], extSrcLen, 0))
            {
                hlFree(entry.path);
                return HL_ERROR_UNKNOWN;
            }

            /* Set null terminator. */
            entry.path[fileNameSize + extDstLen] = HL_NTEXT('\0');
        }

        /* Set entry size and meta. */
        entry.size = (size_t)dataEntry->dataSize;
        entry.meta = 0;

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
                        entry.size += (sizeof(HlBINAV2Header) +
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
                                    {
                                        HlINPACxV2MergedStrEntry* oldDataPtr = mergedStrs.data;
                                        result = HL_LIST_PUSH(mergedStrs, mergedStr);

                                        if (HL_FAILED(result))
                                        {
                                            mergedStrs.data = oldDataPtr;
                                            HL_LIST_FREE(mergedStrs);
                                            hlFree(entry.path);
                                            return result;
                                        }
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
                    entry.size += strTableSize;
                    entry.size += offTableSize;

                    /* Allocate data buffer. */
                    dstDataBuf = (HlU8*)hlAlloc(entry.size);

                    if (!dstDataBuf)
                    {
                        HL_LIST_FREE(mergedStrs);
                        hlFree(entry.path);
                        return HL_ERROR_OUT_OF_MEMORY;
                    }

                    /* Set data pointer within file entry. */
                    entry.data = (HlUMax)((HlUPtr)dstDataBuf);

                    /* Setup BINA header. */
                    dstHeader = (HlBINAV2Header*)dstDataBuf;
                    dstHeader->signature = HL_BINA_SIG;
                    dstHeader->version[0] = '2';
                    dstHeader->version[1] = '0';
                    dstHeader->version[2] = '0';
                    dstHeader->endianFlag = endianFlag;
                    dstHeader->fileSize = (HlU32)entry.size;
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
                dstDataBuf = (HlU8*)hlAlloc(entry.size);

                if (!dstDataBuf)
                {
                    hlFree(entry.path);
                    return HL_ERROR_OUT_OF_MEMORY;
                }

                /* Copy data. */
                memcpy(dstDataBuf, data, entry.size);

                /* Set data pointer within file entry. */
                entry.data = (HlUMax)((HlUPtr)dstDataBuf);
            }
        }
        else
        {
            /* Set meta and data within file entry. */
            entry.meta = HL_ARC_ENTRY_STREAMING_FLAG;
            entry.data = (HlUMax)((HlUPtr)NULL);
        }

        /* Add entry to archive. */
        {
            HlArchiveEntry* oldDataPtr = hlArcBuf->entries.data;
            result = HL_LIST_PUSH(hlArcBuf->entries, entry);

            if (HL_FAILED(result))
            {
                hlArcBuf->entries.data = oldDataPtr;
                hlArchiveEntryDestruct(&entry);
                return result;
            }
        }
    }

    return HL_RESULT_SUCCESS;
}

HlResult hlPACxV2Read(const HlBlob* const HL_RESTRICT * HL_RESTRICT pacs,
    size_t pacCount, HlArchive* HL_RESTRICT * HL_RESTRICT archive)
{
    HlArchive* hlArcBuf;
    const HlBool skipProxies = (pacCount > 1);

    /* Allocate HlArchive buffer. */
    hlArcBuf = HL_ALLOC_OBJ(HlArchive);
    if (!hlArcBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Setup HlArchive. */
    HL_LIST_INIT(hlArcBuf->entries);

    /* Setup archive entries. */
    {
        size_t i;
        HlResult result;

        for (i = 0; i < pacCount; ++i)
        {
            /* Get data block pointer. */
            const HlPACxV2BlockDataHeader* dataBlock = (const
                HlPACxV2BlockDataHeader*)hlBINAV2GetDataBlock(pacs[i]);

            const HlU32* firstDataOffset;
            const char* strings;
            const HlU8 *offsets, *eof;

            if (!dataBlock) return HL_ERROR_INVALID_DATA;

            /* Get current offset pointer. */
            firstDataOffset = (const HlU32*)pacs[i]->data;

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

                    /* Skip ResPacDepend file trees. */
                    if (!strcmp(colonPtr + 1, HlINPACxV2SplitType)) continue;

                    /* Get fileTree pointer. */
                    fileTree = (const HlPACxV2NodeTree*)hlOff32Get(&typeNodes[i2].data);

                    /* Compute extension length. */
                    extSrcLen = (size_t)(colonPtr - typeStr);

                    /* Account for UTF-8 -> native conversion. */
                    extDstLen = hlStrGetReqLenUTF8ToNative(typeStr, extSrcLen);

                    /* Setup file entries. */
                    result = hlINPACxV2FileTreeSetupEntries(fileTree,
                        skipProxies, ((const HlPACxV2Header*)pacs[i]->data)->endianFlag,
                        firstDataOffset, strings, offsets, eof, typeStr, extSrcLen,
                        extDstLen, hlArcBuf);

                    if (HL_FAILED(result))
                    {
                        hlArchiveFree(hlArcBuf);
                        return result;
                    }
                }
            }
        }
    }

    /* Set archive pointer and return success. */
    *archive = hlArcBuf;
    return HL_RESULT_SUCCESS;
}

static HlResult hlINPACxV2LoadSingle(const HlNChar* HL_RESTRICT filePath,
    HlArchive* HL_RESTRICT * HL_RESTRICT archive)
{
    HlBlob* blob;
    HlResult result;

    /* Load archive. */
    result = hlBlobLoad(filePath, &blob);
    if (HL_FAILED(result)) return result;

    /* Fix PACxV2 data. */
    hlPACxV2Fix(blob);

    /* Parse blob into HlArchive, free blob, and return. */
    result = hlPACxV2Read((const HlBlob**)&blob, 1, archive);
    hlBlobFree(blob);
    return result;
}

static HlResult hlINPACxV2LoadSplitBlobs(HlBlob* HL_RESTRICT rootPac,
    size_t dirLen, size_t pathBufCapacity, HlNChar* HL_RESTRICT * HL_RESTRICT pathBufPtr,
    HlBool* HL_RESTRICT pathBufOnHeap, HlBlob*** pacs,
    size_t* HL_RESTRICT pacCount)
{
    HlPACxV2BlockDataHeader* dataBlock;
    HlPACxV2NodeTree* splitFileTree;
    HlPACxV2Node* splitFileNodes;
    HlU32 i;
    HlResult result;

    /* Fix root pac. */
    hlPACxV2Fix(rootPac);

    /* Get data block. */
    dataBlock = (HlPACxV2BlockDataHeader*)hlBINAV2GetDataBlock(rootPac);
    if (!dataBlock) return HL_ERROR_INVALID_DATA;

    /* Get split file tree. */
    splitFileTree = hlPACxV2DataGetFileTree(
        dataBlock, HlINPACxV2SplitType);

    /* Get splits count. */
    if (splitFileTree)
    {
        /* Get split file nodes. */
        splitFileNodes = (HlPACxV2Node*)hlOff32Get(
            &splitFileTree->nodes);

        /* Get splits count. */
        for (i = 0; i < splitFileTree->nodeCount; ++i)
        {
            HlPACxV2SplitTable* splitTable = (HlPACxV2SplitTable*)(
                ((HlPACxV2DataEntry*)hlOff32Get(&splitFileNodes[i].data)) + 1);

            *pacCount = (size_t)splitTable->splitCount;
        }
    }
    else
    {
        *pacCount = 0;
    }

    /* Allocate splits pointer array. */
    *pacs = HL_ALLOC_ARR(HlBlob*, *pacCount + 1);
    if (!(*pacs)) return HL_ERROR_OUT_OF_MEMORY;

    /* Set root pointer. */
    (*pacs)[0] = rootPac;

    /*
       Reset pac count. We'll increment it again in the next loop.
       This is so freeing can always work correctly.
    */
    *pacCount = 1;

    /* Return early if there are no splits. */
    if (!splitFileTree) return HL_RESULT_SUCCESS;

    /* Load splits. */
    for (i = 0; i < splitFileTree->nodeCount; ++i)
    {
        HlPACxV2SplitTable* splitTable = (HlPACxV2SplitTable*)(
            ((HlPACxV2DataEntry*)hlOff32Get(&splitFileNodes[i].data)) + 1);

        HL_OFF32_STR* splitNames = (HL_OFF32_STR*)hlOff32Get(
            &splitTable->splitNames);

        HlU32 i2;

        for (i2 = 0; i2 < splitTable->splitCount; ++i2)
        {
            const char* splitName = (const char*)hlOff32Get(&splitNames[i2]);
            const size_t splitNameLenU8 = (strlen(splitName) + 1);
            const size_t splitNameLen = hlStrGetReqLenUTF8ToNative(
                splitName, splitNameLenU8);

            /* Account for directory. */
            const size_t pathBufLen = (splitNameLen + dirLen);

            /* Resize buffer if necessary. */
            if (pathBufLen > pathBufCapacity)
            {
                if (!(*pathBufOnHeap))
                {
                    /* Switch from stack buffer to heap buffer. */
                    *pathBufPtr = HL_ALLOC_ARR(HlNChar, pathBufLen);
                    *pathBufOnHeap = HL_TRUE;
                }
                else
                {
                    /* Resize heap buffer. */
                    *pathBufPtr = HL_RESIZE_ARR(HlNChar, pathBufLen, *pathBufPtr);
                }

                if (!(*pathBufPtr)) return HL_ERROR_OUT_OF_MEMORY;
                pathBufCapacity = pathBufLen;
            }

            /* Copy split name into buffer. */
            if (!hlStrConvUTF8ToNativeNoAlloc(splitName,
                &((*pathBufPtr)[dirLen]), splitNameLenU8,
                pathBufCapacity))
            {
                return HL_ERROR_UNKNOWN;
            }

            /* Load split. */
            result = hlBlobLoad(*pathBufPtr, &((*pacs)[*pacCount]));
            if (HL_FAILED(result)) return result;

            /* Fix split. */
            hlPACxV2Fix((*pacs)[(*pacCount)++]);
        }
    }

    return HL_RESULT_SUCCESS;
}

HlResult hlPACxV2Load(const HlNChar* HL_RESTRICT filePath,
    HlBool loadSplits, HlArchive* HL_RESTRICT * HL_RESTRICT archive)
{
    return (loadSplits) ?
        hlINPACxLoadSplits(filePath, '2', archive) :
        hlINPACxV2LoadSingle(filePath, archive);
}

void hlPACxV3Fix(HlBlob* blob)
{
    HlPACxV3Header* header = (HlPACxV3Header*)blob->data;
    if (hlBINANeedsSwap(header->endianFlag))
    {
        /* TODO: Swap header. */
        HL_ASSERT(HL_FALSE);
    }

    /* Fix offsets. */
    {
        const void* offsets = hlPACxV3GetOffsetTable(header);
        hlBINAOffsetsFix64(offsets, header->endianFlag,
            header->offsetTableSize, header);
    }

    if (hlBINANeedsSwap(header->endianFlag))
    {
        HlPACxV3NodeTree* typeTree = hlPACxV3GetTypeTree(header);
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

static HlResult hlINPACxV3FileNodesSetupEntries(const HlPACxV3Node* fileNodes,
    const HlPACxV3Node* fileNode, HlBool skipProxies,
    char* HL_RESTRICT pathBuf, HlArchive* HL_RESTRICT hlArcBuf)
{
    const HlS32* childIndices = (const HlS32*)hlOff64Get(&fileNode->childIndices);
    HlU16 i;
    HlResult result;

    if (fileNode->hasData)
    {
        const HlPACxV3DataEntry* dataEntry = (const HlPACxV3DataEntry*)
            hlOff64Get(&fileNode->data);

        /* Skip proxies if requested. */
        if (!skipProxies || dataEntry->dataType != HL_PACXV3_DATA_TYPE_NOT_HERE)
        {
            HlArchiveEntry entry;
            const char* ext = (const char*)hlOff64Get(&dataEntry->extension);
            size_t nameLen, extSize, pathBufSize;

            /* Ensure node name length is > 0. */
            if (!fileNode->bufStartIndex) return HL_ERROR_INVALID_DATA;

            /* Get required length for native path buffer. */
            nameLen = hlStrGetReqLenUTF8ToNative(pathBuf,
                fileNode->bufStartIndex);

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

            if (!hlStrConvUTF8ToNativeNoAlloc(pathBuf,
                entry.path, fileNode->bufStartIndex, pathBufSize))
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

            /* Setup rest of archive entry and copy data if necessary. */
            entry.size = (size_t)dataEntry->dataSize;

            if (dataEntry->dataType != HL_PACXV3_DATA_TYPE_NOT_HERE)
            {
                /* Allocate data buffer. */
                void* dstDataBuf = (HlU8*)hlAlloc(entry.size);

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
            {
                HlArchiveEntry* oldDataPtr = hlArcBuf->entries.data;
                result = HL_LIST_PUSH(hlArcBuf->entries, entry);

                if (HL_FAILED(result))
                {
                    hlArcBuf->entries.data = oldDataPtr;
                    hlArchiveEntryDestruct(&entry);
                    return result;
                }
            }
        }
    }
    else if (fileNode->name)
    {
        /* Copy name into path buffer. */
        strcpy(&pathBuf[fileNode->bufStartIndex],
            (const char*)hlOff64Get(&fileNode->name));
    }

    /* Recurse through children. */
    for (i = 0; i < fileNode->childCount; ++i)
    {
        result = hlINPACxV3FileNodesSetupEntries(fileNodes,
            &fileNodes[childIndices[i]], skipProxies, pathBuf,
            hlArcBuf);

        if (HL_FAILED(result)) return result;
    }

    return HL_RESULT_SUCCESS;
}

HlResult hlPACxV3Read(const HlBlob* const HL_RESTRICT * HL_RESTRICT pacs,
    size_t pacCount, HlArchive* HL_RESTRICT * HL_RESTRICT archive)
{
    HlArchive* hlArcBuf;
    const HlBool skipProxies = (pacCount > 1);

    /* Allocate HlArchive buffer. */
    hlArcBuf = HL_ALLOC_OBJ(HlArchive);
    if (!hlArcBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Setup HlArchive. */
    HL_LIST_INIT(hlArcBuf->entries);

    /* Setup archive entries. */
    {
        char pathBuf[256]; /* PACxV3 names are hard-limited to 255, not including null terminator. */
        size_t i;
        HlResult result;

        for (i = 0; i < pacCount; ++i)
        {
            const HlPACxV3Header* header = (const HlPACxV3Header*)pacs[i]->data;
            const HlPACxV3NodeTree* typeTree = hlPACxV3GetTypeTree(header);
            const HlPACxV3Node* typeNodes = (const HlPACxV3Node*)hlOff64Get(&typeTree->nodes);
            const HlS32* typeDataIndices = (const HlS32*)hlOff64Get(&typeTree->dataNodeIndices);
            HlU64 i2;

            for (i2 = 0; i2 < typeTree->dataNodeCount; ++i2)
            {
                /* Setup archive entries. */
                const HlPACxV3NodeTree* fileTree = (const HlPACxV3NodeTree*)
                    hlOff64Get(&typeNodes[typeDataIndices[i2]].data);

                const HlPACxV3Node* fileNodes = (const HlPACxV3Node*)
                    hlOff64Get(&fileTree->nodes);

                result = hlINPACxV3FileNodesSetupEntries(fileNodes,
                    fileNodes, skipProxies, pathBuf, hlArcBuf);

                if (HL_FAILED(result))
                {
                    hlArchiveFree(hlArcBuf);
                    return result;
                }
            }
        }
    }

    /* Set archive pointer and return success. */
    *archive = hlArcBuf;
    return HL_RESULT_SUCCESS;
}

static HlResult hlINPACxV3LoadSingle(const HlNChar* HL_RESTRICT filePath,
    HlArchive* HL_RESTRICT * HL_RESTRICT archive)
{
    HlBlob* blob;
    HlResult result;

    /* Load archive. */
    result = hlBlobLoad(filePath, &blob);
    if (HL_FAILED(result)) return result;

    /* Fix PACxV3 data. */
    hlPACxV3Fix(blob);

    /* Parse blob into HlArchive, free blob, and return. */
    result = hlPACxV3Read((const HlBlob**)&blob, 1, archive);
    hlBlobFree(blob);
    return result;
}

static HlResult hlINPACxV3LoadSplitBlobs(HlBlob* HL_RESTRICT rootPac,
    size_t dirLen, size_t pathBufCapacity, HlNChar* HL_RESTRICT * HL_RESTRICT pathBufPtr,
    HlBool* HL_RESTRICT pathBufOnHeap, HlBlob*** pacs,
    size_t* HL_RESTRICT pacCount)
{
    HlPACxV3Header* header = (HlPACxV3Header*)rootPac->data;
    HlPACxV3SplitTable* splitTable;
    HlPACxV3SplitEntry* splitEntries;
    HlU64 i;
    HlResult result;

    /* Fix root pac. */
    hlPACxV3Fix(rootPac);

    /* Allocate splits pointer array. */
    *pacs = HL_ALLOC_ARR(HlBlob*, (size_t)header->splitCount + 1);
    if (!(*pacs)) return HL_ERROR_OUT_OF_MEMORY;

    /* Set root pointer. */
    (*pacs)[0] = rootPac;

    /*
       Reset pac count. We'll increment it again in the next loop.
       This is so freeing can always work correctly.
    */
    *pacCount = 1;

    /* If there are no splits, return early. */
    if (!header->splitCount) return HL_RESULT_SUCCESS;

    /* Get split table. */
    splitTable = hlPACxV3GetSplitTable(header);

    /* Get split entries pointer. */
    splitEntries = (HlPACxV3SplitEntry*)hlOff64Get(
        &splitTable->splitEntries);

    /* Load splits. */
    for (i = 0; i < splitTable->splitCount; ++i)
    {
        const char* splitName = (const char*)hlOff64Get(&splitEntries[i].name);
        const size_t splitNameLenU8 = (strlen(splitName) + 1);
        const size_t splitNameLen = hlStrGetReqLenUTF8ToNative(
            splitName, splitNameLenU8);

        /* Account for directory. */
        const size_t pathBufLen = (splitNameLen + dirLen);

        /* Resize buffer if necessary. */
        if (pathBufLen > pathBufCapacity)
        {
            if (!(*pathBufOnHeap))
            {
                /* Switch from stack buffer to heap buffer. */
                *pathBufPtr = HL_ALLOC_ARR(HlNChar, pathBufLen);
                *pathBufOnHeap = HL_TRUE;
            }
            else
            {
                /* Resize heap buffer. */
                *pathBufPtr = HL_RESIZE_ARR(HlNChar, pathBufLen, *pathBufPtr);
            }

            if (!(*pathBufPtr)) return HL_ERROR_OUT_OF_MEMORY;
            pathBufCapacity = pathBufLen;
        }

        /* Copy split name into buffer. */
        if (!hlStrConvUTF8ToNativeNoAlloc(splitName,
            &((*pathBufPtr)[dirLen]), splitNameLenU8,
            pathBufCapacity))
        {
            return HL_ERROR_UNKNOWN;
        }

        /* Load split. */
        result = hlBlobLoad(*pathBufPtr, &((*pacs)[*pacCount]));
        if (HL_FAILED(result)) return result;

        /* Fix split. */
        hlPACxV3Fix((*pacs)[(*pacCount)++]);
    }

    return HL_RESULT_SUCCESS;
}

HlResult hlPACxV3Load(const HlNChar* HL_RESTRICT filePath,
    HlBool loadSplits, HlArchive* HL_RESTRICT * HL_RESTRICT archive)
{
    return (loadSplits) ?
        hlINPACxLoadSplits(filePath, '3', archive) :
        hlINPACxV3LoadSingle(filePath, archive);
}

void hlPACxV4Fix(HlBlob* blob)
{
    /* Swap header and root chunks if necessary. */
    HlPACxV4Header* header = (HlPACxV4Header*)blob->data;
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
    const HlPACxV4Chunk* HL_RESTRICT chunks, HlU32 chunkCount,
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
    const HlPACxV4Chunk* HL_RESTRICT chunks, HlU32 chunkCount,
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
    const HlPACxV4Chunk* HL_RESTRICT chunks, HlU32 chunkCount,
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
    return hlZLibDecompressNoAlloc(compressedData,
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

static HlResult hlINPACxV4DecompressToBlobs(HlBlob* HL_RESTRICT pac,
    HlBlob*** pacs, size_t* HL_RESTRICT pacCount)
{
    const HlPACxV4Header* header = (const HlPACxV4Header*)pac->data;
    const HlPACxV3Header* rootHeader;
    HlBlob* root;
    HlResult result;

    /* PACx403 uses Deflate with no separate chunks. */
    const HlBool isV403 = (header->version[1] == '0' && header->version[2] == '3');

    /* Fix pac. */
    hlPACxV4Fix(pac);

    /* Setup root PAC entry, decompressing if necessary. */
    if (isV403)
    {
        result = hlPACxV403DecompressBlob(hlOff32Get(&header->rootOffset),
            header->rootCompressedSize, header->rootUncompressedSize, &root);
    }
    else
    {
        const HlPACxV402Header* headerV402 = (HlPACxV402Header*)header;

        result = hlPACxV402DecompressBlob(hlOff32Get(&headerV402->rootOffset),
            hlPACxV4GetRootChunks(headerV402), headerV402->chunkCount,
            headerV402->rootCompressedSize, headerV402->rootUncompressedSize,
            &root);
    }

    if (HL_FAILED(result)) return result;

    /* Fix root pac. */
    hlPACxV3Fix(root);

    /* Get root header pointer. */
    rootHeader = (const HlPACxV3Header*)root->data;

    /* Allocate splits pointer array. */
    *pacs = HL_ALLOC_ARR(HlBlob*, (size_t)rootHeader->splitCount + 1);
    if (!(*pacs))
    {
        hlBlobFree(root);
        return HL_ERROR_OUT_OF_MEMORY;
    }

    /* Set root pointer. */
    (*pacs)[0] = root;

    /*
       Reset pac count. We'll increment it again in the next loop.
       This is so freeing can always work correctly.
    */
    *pacCount = 1;

    /* Set split blob pointers. */
    if (rootHeader->splitCount)
    {
        HlPACxV3SplitTable* splitTable = hlPACxV3GetSplitTable(rootHeader);

        void* splitEntries = hlOff64Get(&splitTable->splitEntries);

        HlU64 i;

        /* Ensure split count will fit within a size_t. */
        HL_ASSERT(splitTable->splitCount <= HL_SIZE_MAX);

        /* Setup split PAC entries, decompressing if necessary. */
        for (i = 0; i < splitTable->splitCount; ++i)
        {
            if (isV403)
            {
                HlPACxV403SplitEntry* splitEntriesV403 = (HlPACxV403SplitEntry*)splitEntries;

                result = hlPACxV403DecompressBlob(HL_ADD_OFF(header,
                    splitEntriesV403[i].offset), splitEntriesV403[i].compressedSize,
                    splitEntriesV403[i].uncompressedSize, &((*pacs)[*pacCount]));
            }
            else 
            {
                HlPACxV402SplitEntry* splitEntriesV402 = (HlPACxV402SplitEntry*)splitEntries;

                const HlPACxV4Chunk* chunks = (const HlPACxV4Chunk*)
                    hlOff64Get(&splitEntriesV402[i].chunksOffset);

                /* Decompress split PAC. */
                result = hlPACxV402DecompressBlob(HL_ADD_OFF(header,
                    splitEntriesV402[i].offset), chunks, splitEntriesV402[i].chunkCount,
                    splitEntriesV402[i].compressedSize, splitEntriesV402[i].uncompressedSize,
                    &((*pacs)[*pacCount]));
            }


            if (HL_FAILED(result)) return result;

            /* Fix split. */
            hlPACxV3Fix((*pacs)[(*pacCount)++]);
        }
    }

    return HL_RESULT_SUCCESS;
}

HlResult hlPACxV4Read(HlBlob* HL_RESTRICT pac,
    HlBool loadSplits, HlArchive* HL_RESTRICT * HL_RESTRICT archive)
{
    const HlPACxV4Header* header = (const HlPACxV4Header*)pac->data;
    HlBlob** pacs = NULL;
    HlBlob root;
    size_t pacCount;
    HlResult result = HL_RESULT_SUCCESS;

    /* PACx403 uses Deflate with no separate chunks. */
    const HlBool isV403 = header->version[1] == '0' && header->version[2] == '3';

    /* Setup root PAC entry, decompressing if necessary. */
    if (isV403)
    {
        result = hlPACxV403Decompress(hlOff32Get(&header->rootOffset),
            header->rootCompressedSize, header->rootUncompressedSize, &root.data);
    }
    else
    {
        const HlPACxV402Header* headerV402 = (HlPACxV402Header*)header;

        result = hlPACxV402Decompress(hlOff32Get(&headerV402->rootOffset),
            hlPACxV4GetRootChunks(headerV402), headerV402->chunkCount,
            headerV402->rootCompressedSize, headerV402->rootUncompressedSize,
            &root.data);
    }

    if (HL_FAILED(result)) return result;

    /* Fix root PAC. */
    hlPACxV3Fix(&root);

    /* Generate HlArchive and return. */
    if (loadSplits && ((HlPACxV3Header*)root.data)->splitCount)
    {
        HlPACxV3SplitTable* splitTable = hlPACxV3GetSplitTable(
            (HlPACxV3Header*)root.data);

        void* splitEntries = hlOff64Get(&splitTable->splitEntries);

        HlU64 i;

        /* Ensure split count will fit within a size_t. */
        HL_ASSERT(splitTable->splitCount <= HL_SIZE_MAX);

        /* Set PAC count. */
        pacCount = (size_t)(splitTable->splitCount + 1);

        /* Allocate total PAC entries array. */
        pacs = (HlBlob**)hlAlloc((sizeof(HlBlob*) * pacCount) +
            (sizeof(HlBlob) * (size_t)splitTable->splitCount));

        if (!pacs)
        {
            result = HL_ERROR_OUT_OF_MEMORY;
            goto end;
        }

        /* Set root PAC blob pointer. */
        pacs[0] = &root;

        /* Setup split PAC blob pointers. */
        {
            HlBlob* splitBlobs = (HlBlob*)&pacs[pacCount];
            for (i = 0; i < splitTable->splitCount; ++i)
            {
                pacs[i + 1] = &splitBlobs[i];
            }
        }

        /* Setup split PAC entries, decompressing if necessary. */
        for (i = 0; i < splitTable->splitCount; ++i)
        {
            /* Decompress split PAC. */
            if (isV403)
            {
                HlPACxV403SplitEntry* splitEntriesV403 = (HlPACxV403SplitEntry*)splitEntries;

                result = hlPACxV403Decompress(HL_ADD_OFF(header, splitEntriesV403[i].offset),
                    splitEntriesV403[i].compressedSize, splitEntriesV403[i].uncompressedSize,
                    &pacs[i + 1]->data);
            }

            else 
            {
                HlPACxV402SplitEntry* splitEntriesV402 = (HlPACxV402SplitEntry*)splitEntries;

                const HlPACxV4Chunk* chunks = (const HlPACxV4Chunk*)
                    hlOff64Get(&splitEntriesV402[i].chunksOffset);

                result = hlPACxV402Decompress(HL_ADD_OFF(header,
                    splitEntriesV402[i].offset), chunks, splitEntriesV402[i].chunkCount,
                    splitEntriesV402[i].compressedSize, splitEntriesV402[i].uncompressedSize,
                    &pacs[i + 1]->data);
            }

            if (HL_FAILED(result)) goto end;

            /* Fix split PAC. */
            hlPACxV3Fix(pacs[i + 1]);
        }

        /* Generate HlArchive. */
        result = hlPACxV3Read((const HlBlob**)pacs, pacCount, archive);
    }
    else
    {
        /* Generate HlArchive. */
        HlBlob* tmpRootBlobPtr = &root;
        result = hlPACxV3Read((const HlBlob**)(&tmpRootBlobPtr), 1, archive);
    }

end:
    /* Free PAC entries and PAC data. */
    if (pacs)
    {
        HlU32 i;
        for (i = 0; i < pacCount; ++i)
        {
            if (pacs[i]->size) hlFree(pacs[i]->data);
        }

        hlFree(pacs);
    }
    else
    {
        if (root.size) hlFree(root.data);
    }

    return result;
}

HlResult hlPACxV4Load(const HlNChar* HL_RESTRICT filePath,
    HlBool loadSplits, HlArchive* HL_RESTRICT * HL_RESTRICT archive)
{
    HlBlob* blob;
    HlResult result;

    /* Load archive. */
    result = hlBlobLoad(filePath, &blob);
    if (HL_FAILED(result)) return result;

    /* Fix PACxV4 data. */
    hlPACxV4Fix(blob);

    /* Parse blob into HlArchive, free blob, and return. */
    result = hlPACxV4Read(blob, loadSplits, archive);
    hlBlobFree(blob);
    return result;
}

static HlResult hlINPACxLoadSingle(const HlNChar* HL_RESTRICT filePath,
    HlArchive* HL_RESTRICT * HL_RESTRICT archive)
{
    HlBlob* blob;
    HlResult result;

    /* Load archive. */
    result = hlBlobLoad(filePath, &blob);
    if (HL_FAILED(result)) return result;

    /* Fix and parse data based on version. */
    {
        const HlBINAV2Header* header = (const HlBINAV2Header*)blob->data;

        /* Ensure this is, in fact, a PACx file. */
        if (header->signature != HL_PACX_SIG)
        {
            result = HL_ERROR_INVALID_DATA;
            goto end;
        }

        /* Fix and parse data based on version. */
        switch (header->version[0])
        {
        case '2':
            /* Fix PACxV2 data. */
            hlPACxV2Fix(blob);

            /* Parse blob into HlArchive, free blob, and return. */
            result = hlPACxV2Read((const HlBlob**)&blob, 1, archive);
            goto end;

        case '3':
            /* Fix PACxV3 data. */
            hlPACxV3Fix(blob);

            /* Parse blob into HlArchive, free blob, and return. */
            result = hlPACxV3Read((const HlBlob**)&blob, 1, archive);
            goto end;

        case '4':
            /* Fix PACxV4 data. */
            hlPACxV4Fix(blob);

            /* Parse blob into HlArchive, free blob, and return. */
            result = hlPACxV4Read(blob, HL_FALSE, archive);
            goto end;

        default:
            result = HL_ERROR_UNSUPPORTED;
            goto end;
        }
    }

end:
    hlBlobFree(blob);
    return result;
}

HlResult hlINPACxLoadSplitBlobs(const HlNChar* HL_RESTRICT filePath,
    HlU8 majorVersion, HlBlob* HL_RESTRICT * HL_RESTRICT * HL_RESTRICT pacs,
    size_t* HL_RESTRICT pacCount)
{
    HlBlob** pacPtrs = NULL;
    HlBlob* rootPac = NULL;
    HlNChar pathBuf[255];
    HlNChar* pathBufPtr = pathBuf;
    size_t dirLen, pacPtrCount = 0, pathBufCapacity = 255;
    HlResult result;
    HlBool pathBufOnHeap = HL_FALSE;

    /* Allocate pac path buffer. */
    {
        const HlNChar* ext = hlPathGetExt(filePath);
        size_t splitExtLen, filePathLen = hlNStrLen(filePath);
        HlBool needsSep1;
        
        /* Get directory length. */
        dirLen = (hlPathGetName(filePath) - filePath);
        
        /* Get whether the first path needs a separator appended to it. */
        needsSep1 = hlINPathCombineNeedsSep1(filePath, dirLen);
        if (needsSep1) ++filePathLen;

        /* Determine whether this pac is a split. */
        if ((splitExtLen = hlArchiveExtIsSplit(ext)))
        {
            /* Create a copy of filePath big enough to hold just the root's name. */
            if ((filePathLen - splitExtLen) > pathBufCapacity)
            {
                pathBufPtr = HL_ALLOC_ARR(HlNChar, filePathLen - splitExtLen);
                if (!pathBufPtr) return HL_ERROR_OUT_OF_MEMORY;
                pathBufOnHeap = HL_TRUE;
            }

            filePathLen -= (splitExtLen + 1);
        }

        /* Otherwise, just assume it's a root pac. */
        else if ((filePathLen + 1) > pathBufCapacity)
        {
            /* Create a copy of filePath big enough to hold just the root's name. */
            pathBufPtr = HL_ALLOC_ARR(HlNChar, filePathLen + 1);
            if (!pathBufPtr) return HL_ERROR_OUT_OF_MEMORY;
            pathBufOnHeap = HL_TRUE;
        }

        /* Copy the root path into the buffer. */
        memcpy(pathBufPtr, filePath, filePathLen * sizeof(HlNChar));

        /* Set separator if necessary. */
        if (needsSep1) pathBufPtr[filePathLen - 1] = HL_PATH_SEP;

        /* Set null terminator. */
        pathBufPtr[filePathLen] = HL_NTEXT('\0');
    }

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
        result = hlINPACxV2LoadSplitBlobs(rootPac, dirLen, pathBufCapacity,
            &pathBufPtr, &pathBufOnHeap, &pacPtrs, &pacPtrCount);
        break;

    case '3':
        result = hlINPACxV3LoadSplitBlobs(rootPac, dirLen, pathBufCapacity,
            &pathBufPtr, &pathBufOnHeap, &pacPtrs, &pacPtrCount);
        break;

    case '4':
        result = hlINPACxV4DecompressToBlobs(rootPac, &pacPtrs, &pacPtrCount);
        break;

    default:
        result = HL_ERROR_UNSUPPORTED;
        break;
    }

    /* Free data if we failed. */
    if (HL_FAILED(result))
    {
        /* Free splits. */
        size_t i;
        for (i = 1; i < pacPtrCount; ++i)
        {
            hlBlobFree(pacPtrs[i]);
        }

        hlFree(pacPtrs);

        /* Free root pac. */
        hlBlobFree(rootPac);
    }

    /* Otherwise, set pointers. */
    else
    {
        *pacs = pacPtrs;
        *pacCount = pacPtrCount;
    }

end:
    /* Free path buffer if necessary and return result. */
    if (pathBufOnHeap) hlFree(pathBufPtr);
    return result;
}

HlResult hlPACxLoadBlobs(const HlNChar* HL_RESTRICT filePath,
    HlBlob* HL_RESTRICT * HL_RESTRICT * HL_RESTRICT pacs,
    size_t* HL_RESTRICT pacCount)
{
    return hlINPACxLoadSplitBlobs(filePath, 0, pacs, pacCount);
}

HlResult hlINPACxLoadSplits(const HlNChar* HL_RESTRICT filePath,
    HlU8 majorVersion, HlArchive* HL_RESTRICT * HL_RESTRICT archive)
{
    HlBlob** pacs;
    size_t pacCount;
    HlResult result;

    /* Load split blobs. */
    result = hlINPACxLoadSplitBlobs(filePath, majorVersion, &pacs, &pacCount);
    if (HL_FAILED(result)) return result;

    /* Get version automatically if none was specified. */
    if (!majorVersion)
    {
        const HlBINAV2Header* header = (const HlBINAV2Header*)pacs[0]->data;

        /* Ensure this is, in fact, a PACx file. */
        if (header->signature != HL_PACX_SIG)
        {
            result = HL_ERROR_INVALID_DATA;
            goto end;
        }

        /* Get major version number. */
        majorVersion = header->version[0];
    }

    /* Parse pacs into single HlArchive based on version. */
    switch (majorVersion)
    {
    case '2':
        result = hlPACxV2Read((const HlBlob**)pacs, pacCount, archive);
        break;

    case '3':
    case '4':
        result = hlPACxV3Read((const HlBlob**)pacs, pacCount, archive);
        break;

    default:
        result = HL_ERROR_UNSUPPORTED;
        break;
    }

    /* Free blobs and return result. */
end:
    {
        /* Free pac blobs. */
        size_t i;
        for (i = 0; i < pacCount; ++i)
        {
            hlBlobFree(pacs[i]);
        }

        hlFree(pacs);
    }
    
    return result;
}

HlResult hlPACxLoad(const HlNChar* HL_RESTRICT filePath,
    HlBool loadSplits, HlArchive* HL_RESTRICT * HL_RESTRICT archive)
{
    return (loadSplits) ?
        hlINPACxLoadSplits(filePath, 0, archive) :
        hlINPACxLoadSingle(filePath, archive);
}

#ifndef HL_NO_EXTERNAL_WRAPPERS
HlPACxV2NodeTree* hlPACxV2DataGetTypeTreeExt(HlPACxV2BlockDataHeader* dataBlock)
{
    return hlPACxV2DataGetTypeTree(dataBlock);
}

HlPACxV2DataEntry* hlPACxV2DataGetDataEntriesExt(HlPACxV2BlockDataHeader* dataBlock)
{
    return hlPACxV2DataGetDataEntries(dataBlock);
}

HlPACxV2ProxyEntryTable* hlPACxV2DataGetProxyEntryTableExt(
    HlPACxV2BlockDataHeader* dataBlock)
{
    return hlPACxV2DataGetProxyEntryTable(dataBlock);
}

char* hlPACxV2DataGetStringTableExt(HlPACxV2BlockDataHeader* dataBlock)
{
    return hlPACxV2DataGetStringTable(dataBlock);
}

const void* hlPACxV2DataGetOffsetTableExt(const HlPACxV2BlockDataHeader* dataBlock)
{
    return hlPACxV2DataGetOffsetTable(dataBlock);
}

HlPACxV3NodeTree* hlPACxV3GetTypeTreeExt(HlPACxV3Header* header)
{
    return hlPACxV3GetTypeTree(header);
}

HlPACxV3SplitTable* hlPACxV3GetSplitTableExt(HlPACxV3Header* header)
{
    return hlPACxV3GetSplitTable(header);
}

HlPACxV3DataEntry* hlPACxV3GetDataEntriesExt(HlPACxV3Header* header)
{
    return hlPACxV3GetDataEntries(header);
}

char* hlPACxV3GetStringTableExt(HlPACxV3Header* header)
{
    return hlPACxV3GetStringTable(header);
}

void* hlPACxV3GetDataExt(HlPACxV3Header* header)
{
    return hlPACxV3GetData(header);
}

const void* hlPACxV3GetOffsetTableExt(HlPACxV3Header* header)
{
    return hlPACxV3GetOffsetTable(header);
}

HlPACxV4Chunk* hlPACxV4GetRootChunksExt(HlPACxV4Header* header)
{
    return hlPACxV4GetRootChunks(header);
}
#endif
