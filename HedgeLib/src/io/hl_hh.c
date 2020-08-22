#include "hedgelib/io/hl_hh.h"
#include "hedgelib/hl_endian.h"
#include "hedgelib/hl_blob.h"
#include "../hl_in_assert.h" /* TODO: Remove this? */

void hlHHStandardHeaderSwap(HlHHStandardHeader* header)
{
    hlSwapU32P(&header->fileSize);
    hlSwapU32P(&header->version);
    hlSwapU32P(&header->dataSize);
    hlSwapU32P(&header->dataOffset);
    hlSwapU32P(&header->offsetTableOffset);
    hlSwapU32P(&header->eofOffset);
}

void hlHHStandardHeaderFix(HlHHStandardHeader* header)
{
    /* Swap endianness if necessary. */
#ifndef HL_IS_BIG_ENDIAN
    hlHHStandardHeaderSwap(header);
#endif
    
    /* Fix offsets. */
    hlOff32Fix(&header->dataOffset, header);
    hlOff32Fix(&header->offsetTableOffset, header);
    hlOff32Fix(&header->eofOffset, header);
}

void hlHHOffsetsFix(HlU32* HL_RESTRICT offsets,
    HlU32 offsetCount, void* HL_RESTRICT data)
{
    /* Fix all the offsets in the offset table. */
    HlU32 i;
    for (i = 0; i < offsetCount; ++i)
    {
        HlU32* curOff;
        
        /* Endian swap offset position if necessary. */
#ifndef HL_IS_BIG_ENDIAN
        hlSwapU32P(&offsets[i]);
#endif

        /* Get pointer to current offset. */
        curOff = (HlU32*)HL_ADD_OFF(data, offsets[i]);

        /* Endian swap offset if necessary. */
#ifndef HL_IS_BIG_ENDIAN
        hlSwapU32P(curOff);
#endif

        /* Fix current offset. */
        hlOff32Fix(curOff, data);
    }
}

void hlHHFix(HlBlob* blob)
{
    HlHHStandardHeader* header = (HlHHStandardHeader*)blob->data;
    void* data;
    HlU32* offsets;
    HlU32 offsetCount;

    if (header->version == HL_HH_MIRAGE_MAGIC)
    {
        /* TODO: Support mirage headers. */
        HL_ASSERT(0);
    }
    else
    {
        /* Fix standard header. */
        hlHHStandardHeaderFix(header);

        /* Get data pointer. */
        data = hlOff32Get(&header->dataOffset);

        /* Get offsets table pointer. */
        offsets = hlOff32Get(&header->offsetTableOffset);

        /* Endian swap offset count if necessary. */
#ifndef HL_IS_BIG_ENDIAN
        hlSwapU32P(offsets);
#endif

        /* Get offset count and increase offsets pointer. */
        offsetCount = *offsets++;
    }

    /* Fix offsets. */
    hlHHOffsetsFix(offsets, offsetCount, data);
}

void* hlHHGetData(const HlBlob* blob)
{
    HlHHStandardHeader* header = (HlHHStandardHeader*)blob->data;
    if (header->version == HL_HH_MIRAGE_MAGIC)
    {
        /* TODO: Support mirage headers. */
        HL_ASSERT(0);
        return NULL;
    }
    else
    {
        return hlOff32Get(&header->dataOffset);
    }
}
