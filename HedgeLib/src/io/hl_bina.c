#include "../hl_in_assert.h"
#include "hedgelib/hl_endian.h"
#include "hedgelib/io/hl_bina.h"
#include "hedgelib/io/hl_stream.h"
#include "hedgelib/io/hl_hh.h"
#include <stdlib.h>
#include <string.h>

void hlBINAV1HeaderSwap(HlBINAV1Header* header, HlBool swapOffsets)
{
    hlSwapU32P(&header->fileSize);
    if (swapOffsets) hlSwapU32P(&header->offsetTableOffset);
    hlSwapU32P(&header->offsetTableSize);
    hlSwapU32P(&header->unknown1);
    hlSwapU16P(&header->unknownFlag1);
    hlSwapU16P(&header->unknownFlag2);
    hlSwapU16P(&header->unknown2);
}

void hlBINAV1HeaderFix(HlBINAV1Header* header, HlU8 endianFlag)
{
    /* Endian swap if necessary. */
    if (hlBINANeedsSwap(endianFlag))
    {
        hlBINAV1HeaderSwap(header, HL_TRUE);
    }

    /* Fix offsets. */
    {
        void* data = (header + 1);
        hlOff32Fix(&header->offsetTableOffset, data);
    }
}

void hlBINAV2HeaderSwap(HlBINAV2Header* header)
{
    hlSwapU32P(&header->fileSize);
    hlSwapU16P(&header->blockCount);
}

void hlBINAV2BlockHeaderSwap(HlBINAV2BlockHeader* blockHeader)
{
    hlSwapU32P(&blockHeader->size);
}

void hlBINAV2BlockHeaderFix(HlBINAV2BlockHeader* blockHeader,
    HlU8 endianFlag, HlBool is64Bit)
{
    switch (blockHeader->signature)
    {
    case HL_BINAV2_BLOCK_TYPE_DATA:
    {
        /* Fix block data header. */
        HlBINAV2BlockDataHeader* dataHeader = (HlBINAV2BlockDataHeader*)blockHeader;
        hlBINAV2DataHeaderFix(dataHeader, endianFlag);

        /* Fix offsets. */
        {
            void* data = HL_ADD_OFF(dataHeader + 1, dataHeader->relativeDataOffset);
            void* offsets = HL_ADD_OFF(hlOff32Get(&dataHeader->stringTableOffset),
                dataHeader->stringTableSize);

            if (is64Bit)
            {
                hlBINAOffsetsFix64(offsets, endianFlag,
                    dataHeader->offsetTableSize, data);
            }
            else
            {
                hlBINAOffsetsFix32(offsets, endianFlag,
                    dataHeader->offsetTableSize, data);
            }
        }
        break;
    }

    default:
        HL_ASSERT(HL_FALSE);
        break;
    }
}

void hlBINAV2DataHeaderSwap(HlBINAV2BlockDataHeader* dataHeader, HlBool swapOffsets)
{
    hlSwapU32P(&dataHeader->size);
    if (swapOffsets) hlSwapU32P(&dataHeader->stringTableOffset);
    hlSwapU32P(&dataHeader->stringTableSize);
    hlSwapU32P(&dataHeader->offsetTableSize);

    /* This doesn't count as an "offset" despite its name. */
    hlSwapU16P(&dataHeader->relativeDataOffset);
}

void hlBINAV2DataHeaderFix(HlBINAV2BlockDataHeader* dataHeader, HlU8 endianFlag)
{
    /* Endian swap if necessary. */
    if (hlBINANeedsSwap(endianFlag))
    {
        hlBINAV2DataHeaderSwap(dataHeader, HL_TRUE);
    }

    /* Fix offsets. */
    {
        void* data = HL_ADD_OFF(dataHeader + 1, dataHeader->relativeDataOffset);
        hlOff32Fix(&dataHeader->stringTableOffset, data);
    }
}

void hlBINAV2BlocksFix(HlBINAV2BlockHeader* curBlock, HlU16 blockCount,
    HlU8 endianFlag, HlBool is64Bit)
{
    HlU16 i;

    /* Return early if there are no blocks to fix. */
    if (!blockCount) return;

    /* Fix the first block's header. */
    hlBINAV2BlockHeaderFix(curBlock, endianFlag, is64Bit);

    /* Fix subsequent block headers, if any. */
    for (i = 1; i < blockCount; ++i)
    {
        /* Get next block. */
        curBlock = (HlBINAV2BlockHeader*)hlBINAV2BlockGetNext(curBlock);

        /* Fix this block's header. */
        hlBINAV2BlockHeaderFix(curBlock, endianFlag, is64Bit);
    }
}

HlBool hlBINAOffsetsNext(const HlU8* HL_RESTRICT * HL_RESTRICT curOffsetPosPtr,
    const HlU32* HL_RESTRICT * HL_RESTRICT curOffsetPtr)
{
    const HlU8* curOffsetPos = *curOffsetPosPtr;
    HlU32 relOffPos;

    switch (*curOffsetPos & HL_BINA_OFF_SIZE_MASK)
    {
    default:
        /* A size of 0 indicates that we've reached the end of the offset table. */
        return HL_FALSE;

    case HL_BINA_OFF_SIZE_SIX_BIT:
        *curOffsetPtr += (*(curOffsetPos++) & HL_BINA_OFF_DATA_MASK);
        break;

    case HL_BINA_OFF_SIZE_FOURTEEN_BIT:
        relOffPos = (((HlU32)*(curOffsetPos++) & HL_BINA_OFF_DATA_MASK) << 8);
        relOffPos |= ((HlU32)*(curOffsetPos++));

        *curOffsetPtr += relOffPos;
        break;

    case HL_BINA_OFF_SIZE_THIRTY_BIT:
        relOffPos = (((HlU32)*(curOffsetPos++) & HL_BINA_OFF_DATA_MASK) << 24);
        relOffPos |= (((HlU32)*(curOffsetPos++)) << 16);
        relOffPos |= (((HlU32)*(curOffsetPos++)) << 8);
        relOffPos |= ((HlU32)*(curOffsetPos++));

        *curOffsetPtr += relOffPos;
        break;
    }

    /* Set curOffsetPosPtr and return. */
    *curOffsetPosPtr = curOffsetPos;
    return HL_TRUE;
}

void hlBINAOffsetsFix32(const void* HL_RESTRICT offsets, HlU8 endianFlag,
    HlU32 offsetTableSize, void* HL_RESTRICT data)
{
    /* Get pointers. */
    const HlU8* curOffsetPos = (const HlU8*)offsets;
    const HlU8* eof = (curOffsetPos + offsetTableSize);
    HlU32* curOffset = (HlU32*)data;

    /* Fix offsets. */
    while (curOffsetPos < eof)
    {
        /*
           Get the next offset's address - return early
           if we've reached the end of the offset table.
        */
        if (!hlBINAOffsetsNext(&curOffsetPos, (const HlU32**)&curOffset))
            return;

        /* Endian swap the offset if necessary. */
        if (hlBINANeedsSwap(endianFlag))
        {
            hlSwapU32P(curOffset);
        }

        /* Fix the offset. */
        hlOff32Fix(curOffset, data);
    }
}

void hlBINAOffsetsFix64(const void* HL_RESTRICT offsets, HlU8 endianFlag,
    HlU32 offsetTableSize, void* HL_RESTRICT data)
{
    /* Get pointers. */
    const HlU8* curOffsetPos = (const HlU8*)offsets;
    const HlU8* eof = (curOffsetPos + offsetTableSize);
    HlU64* curOffset = (HlU64*)data;

    /* Fix offsets. */
    while (curOffsetPos < eof)
    {
        /*
           Get the next offset's address - return early
           if we've reached the end of the offset table.
        */
        if (!hlBINAOffsetsNext(&curOffsetPos, (const HlU32**)(&curOffset)))
            return;

        /* Endian swap the offset if necessary. */
        if (hlBINANeedsSwap(endianFlag))
        {
            hlSwapU64P(curOffset);
        }

        /* Fix the offset. */
        hlOff64Fix(curOffset, data);
    }
}

void hlBINAV1Fix(void* rawData)
{
    /* Fix header. */
    HlBINAV1Header* header = (HlBINAV1Header*)rawData;
    hlBINAV1HeaderFix(header, header->endianFlag);

    /* Fix offsets. */
    {
        void* data = (header + 1);
        hlBINAOffsetsFix32(hlOff32Get(&header->offsetTableOffset),
            header->endianFlag, header->offsetTableSize, data);
    }
}

void hlBINAPacPackMetadataFix(void* rawData,
    void* pacPackMetadata, size_t dataSize)
{
    HlU32* offsets = (HlU32*)HL_ADD_OFF(pacPackMetadata, 16);
    HlU32* eof = (HlU32*)HL_ADD_OFF(rawData, dataSize);
    HlU32* offCount = offsets++;
    HlU32 i;

    /* Auto-detect endianness. */
    const HlBool needsSwap = (*offCount > (HlU32)(eof - offsets));

    /* Swap offset count if necessary. */
    if (needsSwap) hlSwapU32P(offCount);

    /* Fix offsets. */
    for (i = 0; i < *offCount; ++i)
    {
        HlU32* curOff;

        /* Endian swap offset position if necessary. */
        if (needsSwap) hlSwapU32P(&offsets[i]);

        /* Get pointer to current offset. */
        curOff = (HlU32*)HL_ADD_OFF(rawData, offsets[i]);

        /* Endian swap offset if necessary. */
        if (needsSwap) hlSwapU32P(curOff);

        /* Fix current offset. */
        hlOff32Fix(curOff, rawData);
    }
}

HlResult hlBINAV2Fix(void* rawData, size_t dataSize)
{
    if (hlBINAHasV2Header(rawData))
    {
        /* Swap header if necessary. */
        HlBINAV2Header* header = (HlBINAV2Header*)rawData;
        if (hlBINANeedsSwap(header->endianFlag))
        {
            hlBINAV2HeaderSwap(header);
        }

        /* Fix blocks. */
        {
            HlBINAV2BlockHeader* blocks = (HlBINAV2BlockHeader*)(header + 1);
            const HlBool is64Bit = hlBINAIs64Bit(hlBINAGetVersion(rawData));

            hlBINAV2BlocksFix(blocks, header->blockCount,
                header->endianFlag, is64Bit);
        }
    }
    else
    {
        /*
           Compatibility with files extracted using PacPack; check if
           this file has PacPack metadata and fix it if so.
        */
        void* pacPackMetadata = (void*)hlBINAGetPacPackMetadata(rawData, dataSize);
        if (!pacPackMetadata) return HL_ERROR_UNSUPPORTED;
        
        hlBINAPacPackMetadataFix(rawData, pacPackMetadata, dataSize);
    }

    return HL_RESULT_SUCCESS;
}

HlResult hlBINAFix(void* rawData, size_t dataSize)
{
    if (dataSize >= sizeof(HlBINAV1Header) && hlBINAHasV1Header(rawData))
    {
        hlBINAV1Fix(rawData);
        return HL_RESULT_SUCCESS;
    }
    else
    {
        /* NOTE: hlBINAV2Fix also handles PACPACK_METADATA. */
        return hlBINAV2Fix(rawData, dataSize);
    }
}

const void* hlBINAGetPacPackMetadata(const void* rawData, size_t dataSize)
{
    const HlU8* curDataPtr = (const HlU8*)rawData;
    const HlU8* endDataPtr = (curDataPtr + dataSize);

    /* 20 == (16 for "PACPACK_METADATA" + 4 for offsetCount). */
    while ((curDataPtr + 20) < endDataPtr)
    {
        if (memcmp(curDataPtr, "PACPACK_METADATA", 16) == 0)
        {
            return curDataPtr;
        }

        ++curDataPtr;
    }

    return NULL;
}

HlU32 hlBINAGetVersion(const void* rawData)
{
    if (hlBINAHasV2Header(rawData))
    {
        const HlBINAV2Header* header = (const HlBINAV2Header*)rawData;
        return ((((HlU32)header->version[0]) << 16) |   /* Major version */
            (((HlU32)header->version[1]) << 8) |        /* Minor version */
            ((HlU32)header->version[2]));               /* Revision version */
    }
    else if (hlBINAHasV1Header(rawData))
    {
        const HlBINAV1Header* header = (const HlBINAV1Header*)rawData;
        return ((HlU32)header->version << 16);          /* Major version */
    }
    else
    {
        return 0;
    }
}

const HlBINAV2BlockDataHeader* hlBINAV2GetDataBlock(const void* rawData)
{
    const HlBINAV2Header* header = (const HlBINAV2Header*)rawData;
    const HlBINAV2BlockHeader* curBlock = (const HlBINAV2BlockHeader*)(header + 1);
    HlU16 i;

    /* If we don't have a BINA V2 header, assume we have no DATA block either. */
    if (!hlBINAHasV2Header(rawData)) return NULL;

    /* Otherwise, go through all blocks in the file. */
    for (i = 0; i < header->blockCount; ++i)
    {
        /* If this block is a data block, return it. */
        if (curBlock->signature == HL_BINAV2_BLOCK_TYPE_DATA)
        {
            return (const HlBINAV2BlockDataHeader*)curBlock;
        }

        /* Otherwise, get the next block. */
        curBlock = hlBINAV2BlockGetNext(curBlock);
    }

    return NULL;
}

const void* hlBINAV2GetData(const void* rawData)
{
    const HlBINAV2Header* header = (const HlBINAV2Header*)rawData;
    const HlBINAV2BlockDataHeader* dataBlock;

    /* If we don't have a BINA V2 header, just return the rawData pointer. */
    if (!hlBINAHasV2Header(rawData)) return rawData;
    
    /* Get data block, returning NULL if there is no data block. */
    dataBlock = hlBINAV2GetDataBlock(rawData);
    if (!dataBlock) return NULL;

    /* Get data. */
    return HL_ADD_OFF(dataBlock + 1, dataBlock->relativeDataOffset);
}

const void* hlBINAGetData(const void* rawData)
{
    if (hlBINAHasV2Header(rawData))
    {
        return hlBINAV2GetData(rawData);
    }
    else if (hlBINAHasV1Header(rawData))
    {
        return hlBINAV1GetData(rawData);
    }
    else
    {
        return rawData;
    }
}

#define hlINBINAStringIsDuplicate(str1, str2)\
    ((str1).len == (str2).len &&    /* length matches AND... */\
    ((str1).str == (str2).str ||    /* (pointer matches OR... */\
    !memcmp((str1).str, (str2).str, /* string matches) */\
        (str1).len)))

static HlResult hlINBINAStringsWrite(const HlBool is64Bit,
    size_t dataPos, HlBINAEndianFlag endianFlag,
    const HlStrTable* HL_RESTRICT strTable,
    HlOffTable* HL_RESTRICT offTable, HlStream* HL_RESTRICT stream)
{
    HlBool duplicateStrsBuf[255];
    HlBool* duplicateStrsPtr = duplicateStrsBuf;
    size_t i;
    HlResult result;

    /* Allocate duplicate strings buffer on heap if necessary. */
    if (strTable->count > 255)
    {
        duplicateStrsPtr = HL_ALLOC_ARR(HlBool, strTable->count);
        if (!duplicateStrsPtr) return HL_ERROR_OUT_OF_MEMORY;
    }

    /* Clear the duplicate strings buffer so we don't have garbage data. */
    memset(duplicateStrsPtr, 0, sizeof(HlBool) * strTable->count);

    /* Write strings and fix offsets in string entries. */
    for (i = 0; i < strTable->count; ++i)
    {
        size_t i2, curStrPos;
        union
        {
            HlU32 u32;
            HlU64 u64;
        }
        off;

        /* Skip the current string entry if it's a duplicate. */
        if (duplicateStrsPtr[i]) continue;

        /* Get current string position. */
        curStrPos = hlStreamTell(stream);

        /* Add offset value to offset table. */
        result = HL_LIST_PUSH(*offTable, strTable->data[i].offPos);
        if (HL_FAILED(result)) goto end;

        /* Jump to offset position. */
        result = hlStreamJumpTo(stream, strTable->data[i].offPos);
        if (HL_FAILED(result)) goto end;

        /* Compute and fix offset. */
        if (is64Bit)
        {
            /* Compute offset. */
            off.u64 = (HlU64)(curStrPos - dataPos);

            /* Swap offset if necessary. */
            if (hlBINANeedsSwap(endianFlag))
            {
                hlSwapU64P(&off.u64);
            }

            /* Fix offset. */
            result = hlStreamWrite(stream, sizeof(off.u64), &off.u64, NULL);
            if (HL_FAILED(result)) goto end;
        }
        else
        {
            /* Compute offset. */
            off.u32 = (HlU32)(curStrPos - dataPos);

            /* Swap offset if necessary. */
            if (hlBINANeedsSwap(endianFlag))
            {
                hlSwapU32P(&off.u32);
            }

            /* Fix offset. */
            result = hlStreamWrite(stream, sizeof(off.u32), &off.u32, NULL);
            if (HL_FAILED(result)) goto end;
        }

        /* Mark duplicate string entries and fix duplicate offsets. */
        for (i2 = (i + 1); i2 < strTable->count; ++i2)
        {
            /* If we've found a duplicate string entry... */
            if (hlINBINAStringIsDuplicate(strTable->data[i], strTable->data[i2]))
            {
                /* Add offset value to offset table. */
                result = HL_LIST_PUSH(*offTable, strTable->data[i2].offPos);
                if (HL_FAILED(result)) goto end;

                /* Jump to offset position. */
                result = hlStreamJumpTo(stream, strTable->data[i2].offPos);
                if (HL_FAILED(result)) goto end;

                /* Fix offset. */
                result = (is64Bit) ?
                    hlStreamWrite(stream, sizeof(off.u64), &off.u64, NULL) :
                    hlStreamWrite(stream, sizeof(off.u32), &off.u32, NULL);

                if (HL_FAILED(result)) goto end;

                /* Mark this string entry as a duplicate. */
                duplicateStrsPtr[i2] = HL_TRUE;
            }
        }

        /* Jump to string position. */
        result = hlStreamJumpTo(stream, curStrPos);
        if (HL_FAILED(result)) goto end;

        /* Write string. */
        result = hlStreamWrite(stream, strTable->data[i].len,
            strTable->data[i].str, NULL);

        if (HL_FAILED(result)) goto end;

        /* Write null terminator. */
        result = hlStreamWriteNulls(stream, 1, NULL);
        if (HL_FAILED(result)) goto end;
    }

    /* Write padding. */
    result = hlStreamPad(stream, 4);

end:
    /* Free resources as necessary and return result. */
    if (duplicateStrsPtr != duplicateStrsBuf) hlFree(duplicateStrsPtr);
    return result;
}

HlResult hlBINAStringsWrite32(size_t dataPos, HlBINAEndianFlag endianFlag,
    const HlStrTable* HL_RESTRICT strTable, HlOffTable* HL_RESTRICT offTable,
    HlStream* HL_RESTRICT stream)
{
    return hlINBINAStringsWrite(HL_FALSE, dataPos,
        endianFlag, strTable, offTable, stream);
}

HlResult hlBINAStringsWrite64(size_t dataPos, HlBINAEndianFlag endianFlag,
    const HlStrTable* HL_RESTRICT strTable, HlOffTable* HL_RESTRICT offTable,
    HlStream* HL_RESTRICT stream)
{
    return hlINBINAStringsWrite(HL_TRUE, dataPos,
        endianFlag, strTable, offTable, stream);
}

HlResult hlBINAOffsetsWriteNoSort(size_t dataPos,
    const HlOffTable* HL_RESTRICT offTable, HlStream* HL_RESTRICT stream)
{
    size_t i, lastOffPos = dataPos;
    for (i = 0; i < offTable->count; ++i)
    {
        const size_t curOffVal = ((offTable->data[i] - lastOffPos) >> 2);
        size_t curOffSize;
        HlU8 tmpOffVal[4];
        HlResult result;

        /* Ensure offset fits within thirty bits. */
        if (curOffVal > 0x3FFFFFFFU) return HL_ERROR_OUT_OF_RANGE;

        /* Compute offset value. */
        if (curOffVal <= 0x3F)
        {
            tmpOffVal[0] = (HlU8)(curOffVal & HL_BINA_OFF_DATA_MASK);
            tmpOffVal[0] |= HL_BINA_OFF_SIZE_SIX_BIT;
            curOffSize = 1;
        }
        else if (curOffVal <= 0x3FFF)
        {
            tmpOffVal[0] = (HlU8)((curOffVal & 0x3F00) >> 8);
            tmpOffVal[0] |= HL_BINA_OFF_SIZE_FOURTEEN_BIT;

            tmpOffVal[1] = (HlU8)(curOffVal & 0xFF);
            curOffSize = 2;
        }
        else
        {
            tmpOffVal[0] = (HlU8)((curOffVal & 0x3F000000) >> 24);
            tmpOffVal[0] |= HL_BINA_OFF_SIZE_THIRTY_BIT;

            tmpOffVal[1] = (HlU8)((curOffVal & 0xFF0000) >> 16);
            tmpOffVal[2] = (HlU8)((curOffVal & 0xFF00) >> 8);
            tmpOffVal[3] = (HlU8)(curOffVal & 0xFF);
            curOffSize = 4;
        }

        /* Write offset value. */
        result = hlStreamWrite(stream, curOffSize, tmpOffVal, NULL);
        if (HL_FAILED(result)) return result;

        /* Set lastOffPos for next iteration. */
        lastOffPos = offTable->data[i];
    }

    /* Write padding and return result. */
    return hlStreamPad(stream, 4);
}

HlResult hlBINAOffsetsWrite(size_t dataPos,
    HlOffTable* HL_RESTRICT offTable, HlStream* HL_RESTRICT stream)
{
    /* Sort offsets in offset table. */
    hlOffTableSort(offTable);

    /* Write sorted offsets. */
    return hlBINAOffsetsWriteNoSort(dataPos, offTable, stream);
}

HlResult hlBINAV1StartWrite(HlBINAEndianFlag endianFlag, HlStream* stream)
{
    /* Generate BINAV1 header. */
    const HlBINAV1Header header =
    {
        0,                  /* fileSize */
        0,                  /* offsetTableOffset */
        0,                  /* offsetTableSize */
        0,                  /* unknown1 */
        0,                  /* unknownFlag1 */
        0,                  /* unknownFlag2 */
        0,                  /* unknown2 */
        '1',                /* version */
        (HlU8)endianFlag,   /* endianFlag */
        HL_BINA_SIG,        /* signature */
        0                   /* padding */
    };

    /*
       NOTE: We don't need to swap the header yet since the only values
       that ever need to be swapped are going to be filled-in later.
    */

    /* Write BINAV1 header and return result. */
    return hlStreamWrite(stream, sizeof(header), &header, NULL);
}

HlResult hlBINAV2StartWrite(HlBool use64BitOffsets,
    HlBINAEndianFlag endianFlag, HlStream* stream)
{
    /* Generate BINAV2 header. */
    const HlBINAV2Header header =
    {
        HL_BINA_SIG,                                        /* signature */
        { '2', (use64BitOffsets) ? '1' : '0', '0' },        /* version */
        (HlU8)endianFlag,                                   /* endianFlag */
        0,                                                  /* fileSize */
        0,                                                  /* blockCount */
        0                                                   /* padding */
    };

    /*
       NOTE: We don't need to swap the header yet since the only values
       that ever need to be swapped are going to be filled-in later.
    */

    /* Write BINAV2 header and return result. */
    return hlStreamWrite(stream, sizeof(header), &header, NULL);
}

HlResult hlBINAV1FinishWrite(size_t headerPos,
    HlOffTable* HL_RESTRICT offTable, HlBINAEndianFlag endianFlag,
    HlStream* HL_RESTRICT stream)
{
    const size_t dataPos = (headerPos + sizeof(HlBINAV1Header));
    size_t offTablePos, eofPos;
    HlResult result;

    /* Get offset table position. */
    offTablePos = hlStreamTell(stream);

    /* Write offset table. */
    result = hlBINAOffsetsWrite(dataPos, offTable, stream);
    if (HL_FAILED(result)) return result;

    /* Get end of stream position. */
    eofPos = hlStreamTell(stream);

    /* Jump to header fileSize position. */
    result = hlStreamJumpTo(stream, headerPos);
    if (HL_FAILED(result)) return result;

    /* Fill-in header values. */
    {
        struct
        {
            HlU32 fileSize;
            HlU32 offsetTableOffset;
            HlU32 offsetTableSize;
        }
        values;

        /* Compute header values. */
        values.fileSize = (HlU32)(eofPos - headerPos);
        values.offsetTableOffset = (HlU32)(offTablePos - dataPos);
        values.offsetTableSize = (HlU32)(eofPos - offTablePos);

        /* Endian-swap header values if necessary. */
        if (hlBINANeedsSwap(endianFlag))
        {
            hlSwapU32P(&values.fileSize);
            hlSwapU32P(&values.offsetTableOffset);
            hlSwapU32P(&values.offsetTableSize);
        }

        /* Fill-in header values. */
        result = hlStreamWrite(stream, sizeof(values), &values, NULL);
        if (HL_FAILED(result)) return result;
    }

    /* Jump to end of stream and return. */
    return hlStreamJumpTo(stream, eofPos);
}

HlResult hlBINAV2FinishWrite(size_t headerPos, HlU16 blockCount,
    HlBINAEndianFlag endianFlag, HlStream* stream)
{
    size_t curPos;
    HlResult result;
    HlU32 fileSize;

    /* Get current stream position. */
    curPos = hlStreamTell(stream);

    /* Jump to header fileSize position. */
    result = hlStreamJumpTo(stream, headerPos + 8);
    if (HL_FAILED(result)) return result;

    /* Compute file size. */
    fileSize = (HlU32)(curPos - headerPos);

    /* Swap file size and block count if necessary. */
    if (hlBINANeedsSwap(endianFlag))
    {
        hlSwapU32P(&fileSize);
        hlSwapU16P(&blockCount);
    }

    /* Fill-in file size. */
    result = hlStreamWrite(stream, sizeof(HlU32), &fileSize, NULL);
    if (HL_FAILED(result)) return result;

    /* Fill-in block count. */
    result = hlStreamWrite(stream, sizeof(HlU16), &blockCount, NULL);
    if (HL_FAILED(result)) return result;

    /* Jump back to end of stream and return. */
    return hlStreamJumpTo(stream, curPos);
}

HlResult hlBINAV2DataBlockStartWrite(HlBINAEndianFlag endianFlag, HlStream* stream)
{
    /* Generate data block header. */
    HlBINAV2BlockDataHeader dataBlock =
    {
        HL_BINAV2_BLOCK_TYPE_DATA,          /* signature */
        0,                                  /* size */
        0,                                  /* stringTableOffset */
        0,                                  /* stringTableSize */
        0,                                  /* offsetTableSize */
        sizeof(HlBINAV2BlockDataHeader),    /* relativeDataOffset */
        0                                   /* padding */
    };

    HlResult result;

    /* Endian swap if necessary. */
    if (hlBINANeedsSwap(endianFlag))
    {
        hlSwapU16P(&dataBlock.relativeDataOffset);
    }

    /* Write data block to file. */
    result = hlStreamWrite(stream, sizeof(dataBlock), &dataBlock, NULL);
    if (HL_FAILED(result)) return result;

    /* HACK: Re-use dataBlock memory to write padding. */
    dataBlock.signature = 0;
    dataBlock.relativeDataOffset = 0;

    /* Write padding and return result. */
    return hlStreamWrite(stream, sizeof(dataBlock), &dataBlock, NULL);
}

HlResult hlBINAV2DataBlockFinishWrite(size_t dataBlockPos, HlBool use64BitOffsets,
    HlBINAEndianFlag endianFlag, const HlStrTable* HL_RESTRICT strTable,
    HlOffTable* HL_RESTRICT offTable, HlStream* HL_RESTRICT stream)
{
    const size_t dataPos = (dataBlockPos + (sizeof(HlBINAV2BlockDataHeader) * 2));
    size_t strTablePos, offTablePos, eofPos;
    HlResult result;

    /* Get string table position. */
    strTablePos = hlStreamTell(stream);

    /* Write string table. */
    result = (use64BitOffsets) ?
        hlBINAStringsWrite64(dataPos, endianFlag, strTable, offTable, stream) :
        hlBINAStringsWrite32(dataPos, endianFlag, strTable, offTable, stream);

    if (HL_FAILED(result)) return result;

    /* Get offset table position. */
    offTablePos = hlStreamTell(stream);

    /* Write offset table. */
    result = hlBINAOffsetsWrite(dataPos, offTable, stream);
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
            HL_OFF32_STR stringTableOffset;
            HlU32 stringTableSize;
            HlU32 offsetTableSize;
        }
        values;

        /* Compute data block header values. */
        values.blockSize = (HlU32)(eofPos - dataBlockPos);
        values.stringTableOffset = (HlU32)(strTablePos - dataPos);
        values.stringTableSize = (HlU32)(offTablePos - strTablePos);
        values.offsetTableSize = (HlU32)(eofPos - offTablePos);

        /* Endian-swap data block header values if necessary. */
        if (hlBINANeedsSwap(endianFlag))
        {
            hlSwapU32P(&values.blockSize);
            hlSwapU32P(&values.stringTableOffset);
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

#ifndef HL_NO_EXTERNAL_WRAPPERS
HlBool hlBINANeedsSwapExt(HlU8 endianFlag)
{
    return hlBINANeedsSwap(endianFlag);
}

const HlBINAV2BlockHeader* hlBINAV2BlockGetNextExt(const HlBINAV2BlockHeader* block)
{
    return hlBINAV2BlockGetNext(block);
}

HlBool hlBINAHasV1HeaderExt(const void* rawData)
{
    return hlBINAHasV1Header(rawData);
}

HlBool hlBINAHasV2HeaderExt(const void* rawData)
{
    return hlBINAHasV2Header(rawData);
}

HlBool hlBINAIs64BitExt(HlU32 version)
{
    return hlBINAIs64Bit(version);
}

char hlBINAGetMajorVersionCharExt(HlU32 version)
{
    return hlBINAGetMajorVersionChar(version);
}

char hlBINAGetMinorVersionCharExt(HlU32 version)
{
    return hlBINAGetMinorVersionChar(version);
}

char hlBINAGetRevisionVersionCharExt(HlU32 version)
{
    return hlBINAGetRevisionVersionChar(version);
}

HlU8 hlBINAGetMajorVersionExt(HlU32 version)
{
    return hlBINAGetMajorVersion(version);
}

HlU8 hlBINAGetMinorVersionExt(HlU32 version)
{
    return hlBINAGetMinorVersion(version);
}

HlU8 hlBINAGetRevisionVersionExt(HlU32 version)
{
    return hlBINAGetRevisionVersion(version);
}

const void* hlBINAV1GetDataExt(const void* rawData)
{
    return hlBINAV1GetData(rawData);
}
#endif
