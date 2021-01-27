#include "hedgelib/io/hl_stream.h"
#include "hedgelib/hl_endian.h"
#include "hedgelib/hl_text.h"

const static HlU8 hlINStreamNulls[1024] = { 0 };

HlResult hlStreamWriteNulls(HlStream* HL_RESTRICT stream,
    size_t amount, size_t* HL_RESTRICT writtenByteCount)
{
    HlResult result;
    if (amount > sizeof(hlINStreamNulls))
    {
        /* Allocate a buffer large enough to hold all of the nulls we want to write. */
        void* nulls = hlAlloc(amount);
        if (!nulls) return HL_ERROR_OUT_OF_MEMORY;

        /* Zero-out the allocated memory in the buffer. */
        memset(nulls, 0, amount);

        /* Write the nulls to the stream and free the buffer. */
        result = hlStreamWrite(stream, amount, nulls, writtenByteCount);
        hlFree(nulls);
    }
    else
    {
        /* Write the given amount of nulls to the stream using our static nulls buffer. */
        result = hlStreamWrite(stream, amount,
            hlINStreamNulls, writtenByteCount);
    }

    return result;
}

HlResult hlStreamWriteOff32(HlStream* HL_RESTRICT stream,
    size_t basePos, size_t offVal, HlBool doSwap, 
    HlOffTable* HL_RESTRICT offTable)
{
    /* Compute offset. */
    HlU32 off = (HlU32)(offVal - basePos);

    /* Swap offset if necessary. */
    if (doSwap) hlSwapU32P(&off);

    /* Add offset position to offset table if requested by user. */
    if (offTable)
    {
        HlResult result = HL_LIST_PUSH(*offTable, hlStreamTell(stream));
        if (HL_FAILED(result)) return result;
    }

    /* Write offset to stream and return result. */
    return hlStreamWrite(stream, sizeof(off), &off, NULL);
}

HlResult hlStreamWriteOff64(HlStream* HL_RESTRICT stream,
    size_t basePos, size_t offVal, HlBool doSwap,
    HlOffTable* HL_RESTRICT offTable)
{
    /* Compute offset. */
    HlU64 off = (HlU64)(offVal - basePos);

    /* Swap offset if necessary. */
    if (doSwap) hlSwapU64P(&off);

    /* Add offset position to offset table if requested by user. */
    if (offTable)
    {
        HlResult result = HL_LIST_PUSH(*offTable, hlStreamTell(stream));
        if (HL_FAILED(result)) return result;
    }

    /* Write offset to stream and return result. */
    return hlStreamWrite(stream, sizeof(off), &off, NULL);
}

HlResult hlStreamWriteStringUTF8(HlStream* HL_RESTRICT stream,
    const char* str, size_t* HL_RESTRICT writtenByteCount)
{
    return hlStreamWrite(stream,
        ((strlen(str) + 1) * sizeof(char)),
        str, writtenByteCount);
}

HlResult hlStreamWriteStringNative(HlStream* HL_RESTRICT stream,
    const HlNChar* str, size_t* HL_RESTRICT writtenByteCount)
{
    return hlStreamWrite(stream,
        ((hlNStrLen(str) + 1) * sizeof(HlNChar)),
        str, writtenByteCount);
}

HlResult hlStreamAlign(HlStream* stream, size_t stride)
{
    /* If stride is < 2, we don't need to align; return success. */
    size_t pos;
    if (stride-- < 2) return HL_RESULT_SUCCESS;

    /* Get the current stream position. */
    pos = hlStreamTell(stream);

    /*
       Compute the closest position in the stream that's aligned
       by the given stride, and jump to that position.
    */
    return hlStreamJumpTo(stream, ((pos + stride) & ~stride));
}

HlResult hlStreamPad(HlStream* stream, size_t stride)
{
    /* If stride is < 2, we don't need to pad; return success. */
    size_t pos;
    if (stride-- < 2) return HL_RESULT_SUCCESS;

    /* Get the current stream position. */
    pos = hlStreamTell(stream);

    /*
       Compute the amount of nulls we need to write to align the
       stream with the given stride, and write that many nulls.
    */
    return hlStreamWriteNulls(stream, ((pos + stride) & ~stride) - pos, 0);
}
