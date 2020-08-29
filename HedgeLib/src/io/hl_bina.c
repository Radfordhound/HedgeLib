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
