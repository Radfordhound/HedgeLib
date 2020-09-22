#include "hedgelib/io/hl_bina.h"
#include "hedgelib/hl_endian.h"
#include "../hl_in_assert.h"

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
        curBlock = hlBINAV2BlockGetNext(curBlock);

        /* Fix this block's header. */
        hlBINAV2BlockHeaderFix(curBlock, endianFlag, is64Bit);
    }
}

HlBool hlBINAOffsetsNext(const HlU8** HL_RESTRICT curOffsetPosPtr,
    const HlU32** HL_RESTRICT curOffsetPtr)
{
    const HlU8* curOffsetPos = *curOffsetPosPtr;
    HlU32 relOffPos;

    switch (*curOffsetPos & HL_BINA_OFF_SIZE_MASK)
    {
    default:
        /* A size of 0 indicates that we've reached the end of the offset table. */
        return HL_FALSE;

    case HL_BINA_OFF_SIZE_SIX_BIT:
        *curOffsetPtr += (*curOffsetPos++ & HL_BINA_OFF_DATA_MASK);
        break;

    case HL_BINA_OFF_SIZE_FOURTEEN_BIT:
        relOffPos = (((HlU32)*curOffsetPos++ & HL_BINA_OFF_DATA_MASK) << 8);
        relOffPos |= ((HlU32)*curOffsetPos++);

        *curOffsetPtr += relOffPos;
        break;

    case HL_BINA_OFF_SIZE_THIRTY_BIT:
        relOffPos = (((HlU32)*curOffsetPos++ & HL_BINA_OFF_DATA_MASK) << 24);
        relOffPos |= (((HlU32)*curOffsetPos++) << 16);
        relOffPos |= (((HlU32)*curOffsetPos++) << 8);
        relOffPos |= ((HlU32)*curOffsetPos++);

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

void hlBINAV1Fix(HlBlob* blob)
{
    /* Fix header. */
    HlBINAV1Header* header = (HlBINAV1Header*)blob->data;
    hlBINAV1HeaderFix(header, header->endianFlag);

    /* Fix offsets. */
    {
        void* data = (header + 1);
        hlBINAOffsetsFix32(hlOff32Get(&header->offsetTableOffset),
            header->endianFlag, header->offsetTableSize, data);
    }
}

void hlBINAV2Fix(HlBlob* blob)
{
    /* Swap header if necessary. */
    HlBINAV2Header* header = (HlBINAV2Header*)blob->data;
    if (hlBINANeedsSwap(header->endianFlag))
    {
        hlBINAV2HeaderSwap(header);
    }

    /* Fix blocks. */
    {
        HlBINAV2BlockHeader* blocks = (HlBINAV2BlockHeader*)(header + 1);
        const HlBool is64Bit = hlBINAIs64Bit(hlBINAGetVersion(blob));

        hlBINAV2BlocksFix(blocks, header->blockCount,
            header->endianFlag, is64Bit);
    }
}

void hlBINAFix(HlBlob* blob)
{
    if (hlBINAHasV2Header(blob))
    {
        hlBINAV2Fix(blob);
    }
    else
    {
        hlBINAV1Fix(blob);
    }
}

HlU32 hlBINAGetVersion(const HlBlob* blob)
{
    if (hlBINAHasV2Header(blob))
    {
        const HlBINAV2Header* header = (const HlBINAV2Header*)blob->data;
        return ((((HlU32)header->version[0]) << 16) |   /* Major version */
            (((HlU32)header->version[1]) << 8) |        /* Minor version */
            ((HlU32)header->version[2]));               /* Revision version */
    }
    else
    {
        const HlBINAV1Header* header = (const HlBINAV1Header*)blob->data;
        return ((HlU32)header->version << 16);
    }
}

HlBINAV2BlockDataHeader* hlBINAV2GetDataBlock(const HlBlob* blob)
{
    HlBINAV2Header* header = (HlBINAV2Header*)blob->data;
    HlBINAV2BlockHeader* curBlock = (HlBINAV2BlockHeader*)(header + 1);
    HlU16 i;

    for (i = 0; i < header->blockCount; ++i)
    {
        /* If this block is a data block, return it. */
        if (curBlock->signature == HL_BINAV2_BLOCK_TYPE_DATA)
        {
            return (HlBINAV2BlockDataHeader*)curBlock;
        }

        /* Otherwise, get the next block. */
        curBlock = hlBINAV2BlockGetNext(curBlock);
    }

    return NULL;
}

void* hlBINAV2GetData(const HlBlob* blob)
{
    /* Get data block, returning NULL if there is no data block. */
    HlBINAV2BlockDataHeader* dataBlock = hlBINAV2GetDataBlock(blob);
    if (!dataBlock) return NULL;

    /* Get data. */
    return HL_ADD_OFF(dataBlock + 1, dataBlock->relativeDataOffset);
}

void* hlBINAGetData(const HlBlob* blob)
{
    return (hlBINAHasV2Header(blob)) ?
        hlBINAV2GetData(blob) : hlBINAV1GetData(blob);
}

#define hlINBINAStringIsDuplicate(str1, str2)\
    ((str1).len == (str2).len &&    /* length matches AND... */\
    ((str1).str == (str2).str ||    /* (pointer matches OR... */\
    !memcmp((str1).str, (str2).str, /* string matches) */\
        (str1).len)))

HlResult hlBINAStringsWrite32(size_t dataPos, const HlStrTable* HL_RESTRICT strTable,
    HlOffTable* HL_RESTRICT offTable, HlFile* HL_RESTRICT file)
{
    HlBool skipStrBuf[255];
    HlBool* skipStr = skipStrBuf;
    size_t i;
    HlResult result;

    /* Allocate string entry skip buffer on heap if necessary. */
    if (strTable->count > 255)
    {
        skipStr = HL_ALLOC_ARR(HlBool, strTable->count);
        if (!skipStr) return HL_ERROR_OUT_OF_MEMORY;
    }

    /* Write strings and fix offsets in string entries. */
    for (i = 0; i < strTable->count; ++i)
    {
        size_t i2, curStrPos;
        HlU32 off;

        /* Skip the current string entry if it's a duplicate. */
        if (skipStr[i]) continue;

        /* Get current string position. */
        curStrPos = hlFileTell(file);

        /* Compute offset. */
        off = (HlU32)(curStrPos - dataPos);

        /* Add offset value to offset table. */
        result = HL_LIST_PUSH(*offTable, strTable->data[i].offPos);
        if (HL_FAILED(result)) return result;

        /* Jump to offset position. */
        result = hlFileJumpTo(file, strTable->data[i].offPos);
        if (HL_FAILED(result)) return result;

        /* Fix offset. */
        result = hlFileWrite(file, sizeof(off), &off, NULL);
        if (HL_FAILED(result)) return result;

        /* Mark duplicate string entries and fix duplicate offsets. */
        for (i2 = (i + 1); i2 < strTable->count; ++i2)
        {
            /* If we've found a duplicate string entry... */
            if (hlINBINAStringIsDuplicate(strTable->data[i], strTable->data[i2]))
            {
                /* Add offset value to offset table. */
                result = HL_LIST_PUSH(*offTable, strTable->data[i2].offPos);
                if (HL_FAILED(result)) return result;

                /* Jump to offset position. */
                result = hlFileJumpTo(file, strTable->data[i2].offPos);
                if (HL_FAILED(result)) return result;

                /* Fix offset. */
                result = hlFileWrite(file, sizeof(off), &off, NULL);
                if (HL_FAILED(result)) return result;

                /* Mark this string entry as a duplicate. */
                skipStr[i2] = HL_TRUE;
            }
        }

        /* Jump to string position. */
        result = hlFileJumpTo(file, curStrPos);
        if (HL_FAILED(result)) return result;

        /* Write string. */
        result = hlFileWrite(file, strTable->data[i].len,
            strTable->data[i].str, NULL);

        if (HL_FAILED(result)) return result;

        /* Write null terminator. */
        result = hlFileWriteNulls(file, 1, NULL);
        if (HL_FAILED(result)) return result;
    }

    /* Write padding. */
    result = hlFilePad(file, 4);

    /* Free string entry skip buffer if necessary and return result. */
    if (skipStr != skipStrBuf) hlFree(skipStr);
    return result;
}

HlResult hlBINAStringsWrite64(size_t dataPos, const HlStrTable* HL_RESTRICT strTable,
    HlOffTable* HL_RESTRICT offTable, HlFile* HL_RESTRICT file)
{
    HlBool skipStrBuf[255];
    HlBool* skipStr = skipStrBuf;
    size_t i;
    HlResult result;

    /* Allocate string entry skip buffer on heap if necessary. */
    if (strTable->count > 255)
    {
        skipStr = HL_ALLOC_ARR(HlBool, strTable->count);
        if (!skipStr) return HL_ERROR_OUT_OF_MEMORY;
    }

    /* Write strings and fix offsets in string entries. */
    for (i = 0; i < strTable->count; ++i)
    {
        size_t i2, curStrPos;
        HlU64 off;

        /* Skip the current string entry if it's a duplicate. */
        if (skipStr[i]) continue;

        /* Get current string position. */
        curStrPos = hlFileTell(file);

        /* Compute offset. */
        off = (HlU64)(curStrPos - dataPos);

        /* Add offset value to offset table. */
        result = HL_LIST_PUSH(*offTable, strTable->data[i].offPos);
        if (HL_FAILED(result)) return result;

        /* Jump to offset position. */
        result = hlFileJumpTo(file, strTable->data[i].offPos);
        if (HL_FAILED(result)) return result;

        /* Fix offset. */
        result = hlFileWrite(file, sizeof(off), &off, NULL);
        if (HL_FAILED(result)) return result;

        /* Mark duplicate string entries and fix duplicate offsets. */
        for (i2 = (i + 1); i2 < strTable->count; ++i2)
        {
            /* If we've found a duplicate string entry... */
            if (hlINBINAStringIsDuplicate(strTable->data[i], strTable->data[i2]))
            {
                /* Add offset value to offset table. */
                result = HL_LIST_PUSH(*offTable, strTable->data[i2].offPos);
                if (HL_FAILED(result)) return result;

                /* Jump to offset position. */
                result = hlFileJumpTo(file, strTable->data[i2].offPos);
                if (HL_FAILED(result)) return result;

                /* Fix offset. */
                result = hlFileWrite(file, sizeof(off), &off, NULL);
                if (HL_FAILED(result)) return result;

                /* Mark this string entry as a duplicate. */
                skipStr[i2] = HL_TRUE;
            }
        }

        /* Jump to string position. */
        result = hlFileJumpTo(file, curStrPos);
        if (HL_FAILED(result)) return result;

        /* Write string. */
        result = hlFileWrite(file, strTable->data[i].len,
            strTable->data[i].str, NULL);

        if (HL_FAILED(result)) return result;

        /* Write null terminator. */
        result = hlFileWriteNulls(file, 1, NULL);
        if (HL_FAILED(result)) return result;
    }

    /* Write padding. */
    result = hlFilePad(file, 4);

    /* Free string entry skip buffer if necessary and return result. */
    if (skipStr != skipStrBuf) hlFree(skipStr);
    return result;
}

HlResult hlBINAOffsetsWriteSorted(size_t dataPos,
    const HlOffTable* HL_RESTRICT offTable, HlFile* HL_RESTRICT file)
{
    size_t i, lastOffPos = dataPos;
    for (i = 0; i < offTable->count; ++i)
    {
        const size_t curOffVal = ((offTable->data[i] - lastOffPos) >> 2);
        size_t curOffSize;
        HlU8 tmpOffVal[4];
        HlResult result;

        /* Ensure offset fits within thirty bits. */
        HL_ASSERT(curOffVal <= 0x3FFFFFFF);

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
        result = hlFileWrite(file, curOffSize, tmpOffVal, NULL);
        if (HL_FAILED(result)) return result;

        /* Set lastOffPos for next iteration. */
        lastOffPos = offTable->data[i];
    }

    /* Write padding and return result. */
    return hlFilePad(file, 4);
}

static int hlINOffTableCompareOffsets(const void* a, const void* b)
{
    const size_t off1 = *(const size_t*)a;
    const size_t off2 = *(const size_t*)b;

    return ((off1 > off2) - (off1 < off2));
}

HlResult hlBINAOffsetsWrite(size_t dataPos,
    HlOffTable* HL_RESTRICT offTable, HlFile* HL_RESTRICT file)
{
    /* Sort offsets in offset table. */
    qsort(offTable->data, offTable->count,
        sizeof(size_t), hlINOffTableCompareOffsets);

    /* Write sorted offsets. */
    return hlBINAOffsetsWriteSorted(dataPos, offTable, file);
}

HlResult hlBINAV1StartWrite(HlBINAEndianFlag endianFlag, HlFile* file)
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
    return hlFileWrite(file, sizeof(header), &header, NULL);
}

HlResult hlBINAV2StartWrite(HlBool use64BitOffsets,
    HlBINAEndianFlag endianFlag, HlFile* file)
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
    return hlFileWrite(file, sizeof(header), &header, NULL);
}

HlResult hlBINAV1FinishWrite(size_t headerPos,
    HlOffTable* HL_RESTRICT offTable, HlBINAEndianFlag endianFlag,
    HlFile* HL_RESTRICT file)
{
    const size_t dataPos = (headerPos + sizeof(HlBINAV1Header));
    size_t offTablePos, eofPos;
    HlResult result;

    /* Get offset table position. */
    offTablePos = hlFileTell(file);
    
    /* Write offset table. */
    result = hlBINAOffsetsWrite(dataPos, offTable, file);
    if (HL_FAILED(result)) return result;

    /* Get end of file position. */
    eofPos = hlFileTell(file);

    /* Jump to header fileSize position. */
    result = hlFileJumpTo(file, headerPos);
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
        result = hlFileWrite(file, sizeof(values), &values, NULL);
        if (HL_FAILED(result)) return result;
    }

    /* Jump to end of file and return. */
    return hlFileJumpTo(file, eofPos);
}

HlResult hlBINAV2FinishWrite(size_t headerPos, HlU16 blockCount,
    HlBINAEndianFlag endianFlag, HlFile* file)
{
    size_t curPos;
    HlResult result;
    HlU32 fileSize;

    /* Get current file position. */
    curPos = hlFileTell(file);

    /* Jump to header fileSize position. */
    result = hlFileJumpTo(file, headerPos + 8);
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
    result = hlFileWrite(file, sizeof(HlU32), &fileSize, NULL);
    if (HL_FAILED(result)) return result;

    /* Fill-in block count. */
    result = hlFileWrite(file, sizeof(HlU16), &blockCount, NULL);
    if (HL_FAILED(result)) return result;

    /* Jump back to end of file and return. */
    return hlFileJumpTo(file, curPos);
}

HlResult hlBINAV2DataBlockStartWrite(HlBINAEndianFlag endianFlag, HlFile* file)
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
    result = hlFileWrite(file, sizeof(dataBlock), &dataBlock, NULL);
    if (HL_FAILED(result)) return result;

    /* HACK: Re-use dataBlock memory to write padding. */
    dataBlock.signature = 0;
    dataBlock.relativeDataOffset = 0;

    /* Write padding and return result. */
    return hlFileWrite(file, sizeof(dataBlock), &dataBlock, NULL);
}

HlResult hlBINAV2DataBlockFinishWrite(size_t dataBlockPos, HlBool use64BitOffsets,
    const HlStrTable* HL_RESTRICT strTable, HlOffTable* HL_RESTRICT offTable,
    HlBINAEndianFlag endianFlag, HlFile* HL_RESTRICT file)
{
    const size_t dataPos = (dataBlockPos + (sizeof(HlBINAV2BlockDataHeader) * 2));
    size_t strTablePos, offTablePos, eofPos;
    HlResult result;

    /* Get string table position. */
    strTablePos = hlFileTell(file);

    /* Write string table. */
    result = (use64BitOffsets) ?
        hlBINAStringsWrite64(dataPos, strTable, offTable, file) :
        hlBINAStringsWrite32(dataPos, strTable, offTable, file);

    if (HL_FAILED(result)) return result;

    /* Get offset table position. */
    offTablePos = hlFileTell(file);

    /* Write offset table. */
    result = hlBINAOffsetsWrite(dataPos, offTable, file);
    if (HL_FAILED(result)) return result;

    /* Get end of file position. */
    eofPos = hlFileTell(file);

    /* Jump to data block size position. */
    result = hlFileJumpTo(file, dataBlockPos + 4);
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
        result = hlFileWrite(file, sizeof(values), &values, NULL);
        if (HL_FAILED(result)) return result;
    }

    /* Jump to end of file and return. */
    return hlFileJumpTo(file, eofPos);
}

#ifndef HL_NO_EXTERNAL_WRAPPERS
HlBool hlBINANeedsSwapExt(HlU8 endianFlag)
{
    return hlBINANeedsSwap(endianFlag);
}

HlBINAV2BlockHeader* hlBINAV2BlockGetNextExt(const HlBINAV2BlockHeader* block)
{
    return hlBINAV2BlockGetNext(block);
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

HlBool hlBINAHasV2HeaderExt(const HlBlob* blob)
{
    return hlBINAHasV2Header(blob);
}

HlBool hlBINAIs64BitExt(HlU32 version)
{
    return hlBINAIs64Bit(version);
}

void* hlBINAV1GetDataExt(const HlBlob* blob)
{
    return hlBINAV1GetData(blob);
}
#endif
