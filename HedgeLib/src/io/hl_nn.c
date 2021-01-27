#include "hedgelib/io/hl_nn.h"
#include "hedgelib/io/hl_stream.h"
#include "hedgelib/hl_blob.h"
#include "hedgelib/hl_endian.h"

void hlNNTexCoordSwap(HlNNTexCoord* texCoord)
{
    hlSwapFloatP(&texCoord->u);
    hlSwapFloatP(&texCoord->v);
}

void hlNNRGBSwap(HlNNRGB* rgb)
{
    hlSwapFloatP(&rgb->r);
    hlSwapFloatP(&rgb->g);
    hlSwapFloatP(&rgb->b);
}

void hlNNRotateA32Swap(HlNNRotateA32* rot)
{
    hlSwapS32P(&rot->x);
    hlSwapS32P(&rot->y);
    hlSwapS32P(&rot->z);
}

void hlNNRotateA16Swap(HlNNRotateA16* rot)
{
    hlSwapS16P(&rot->x);
    hlSwapS16P(&rot->y);
    hlSwapS16P(&rot->z);
}

void hlNNDataHeaderSwap(HlNNBinCnkDataHeader* header)
{
    hlSwapU32P(&header->mainDataOffset);
    hlSwapU32P(&header->version);
}

void hlNNFileHeaderSwap(HlNNBinCnkFileHeader* header)
{
    hlSwapU32P(&header->chunkCount);
    hlSwapU32P(&header->dataOffset);
    hlSwapU32P(&header->dataSize);
    hlSwapU32P(&header->NOF0Offset);
    hlSwapU32P(&header->NOF0Size);
    hlSwapU32P(&header->version);
}

void hlNNOffsetsHeaderSwap(HlNNBinCnkNOF0Header* header)
{
    hlSwapU32P(&header->offsetCount);
    hlSwapU32P(&header->padding);
}

void hlNNDataHeaderFix(HlNNBinCnkDataHeader* dataHeader, void* base)
{
    /* Get platform ID from chunk ID. */
    HlNNPlatform platform = hlNNGetPlatform(dataHeader->id);

    /* Endian swap if necessary for the given platform. */
    if (hlNNPlatformNeedsSwap(platform))
    {
        hlNNDataHeaderSwap(dataHeader);
    }

    /* Fix offsets. */
    hlOff32Fix(&dataHeader->nextIDOffset, HL_ADD_OFF(dataHeader, 8));
    hlOff32Fix(&dataHeader->mainDataOffset, base);
}

void hlNNFixHeader(HlNNBinCnkFileHeader* header)
{
    /* Get platform ID from chunk ID. */
    const HlNNPlatform platform = hlNNGetPlatform(header->id);

#ifdef HL_IS_BIG_ENDIAN
    /* This value is always little-endian; endian swap on big endian platforms. */
    hlSwapU32P(&header->nextIDOffset);
#endif

    /* Endian swap if necessary for the given platform. */
    if (hlNNPlatformNeedsSwap(platform))
    {
        hlNNFileHeaderSwap(header);
    }

    /* Fix header offsets. */
    hlOff32Fix(&header->nextIDOffset, HL_ADD_OFF(header, 8));
    hlOff32Fix(&header->dataOffset, header);
    hlOff32Fix(&header->NOF0Offset, header);
}

void hlNNFixDataChunks(void* data, HlU32 dataChunkCount)
{
    HlNNBinCnkDataHeader* curChunk;
    HlU32 i;

    /* Return early if there are no data chunks to fix. */
    if (!dataChunkCount) return;

    /* Fix the first chunk's data header. */
    curChunk = (HlNNBinCnkDataHeader*)data;
    hlNNDataHeaderFix(curChunk, data);

    /* Fix subsequent chunk data headers, if any. */
    for (i = 1; i < dataChunkCount; ++i)
    {
        /* Get the next chunk. */
        curChunk = hlNNGetNextChunk(curChunk);

        /* Fix this chunk's data header. */
        hlNNDataHeaderFix(curChunk, data);
    }
}

void hlNNFixOffsets(HlNNBinCnkNOF0Header* HL_RESTRICT NOF0Header,
    HlNNPlatform platform, void* HL_RESTRICT data)
{
    HlU32* offsets;
    HlU32* curOff;
    HlU32 i;

    /* Endian swap if necessary for the given platform. */
    if (hlNNPlatformNeedsSwap(platform))
    {
        hlNNOffsetsHeaderSwap(NOF0Header);
    }

    /* Get offset positions stored within NOF0 chunk. */
    offsets = (HlU32*)HL_ADD_OFF(NOF0Header, sizeof(*NOF0Header));

    /* Fix each offset. */
    for (i = 0; i < NOF0Header->offsetCount; ++i)
    {
        /* Endian swap offset position if necessary for the given platform. */
        if (hlNNPlatformNeedsSwap(platform))
        {
            hlSwapU32P(&offsets[i]);
        }

        /* Get a pointer to the offset at the given position. */
        curOff = (HlU32*)HL_ADD_OFF(data, offsets[i]);

        /* Endian swap offset if necessary for the given platform. */
        if (hlNNPlatformNeedsSwap(platform))
        {
            hlSwapU32P(curOff);
        }

        /* Fix the offset at the given position. */
        hlOff32Fix(curOff, data);
    }
}

void hlNNFix(HlBlob* blob)
{
    HlNNBinCnkNOF0Header* NOF0Header;
    void* data;
    HlU32 dataChunkCount;

    /* Get platform ID from chunk ID. */
    const HlNNPlatform platform = hlNNGetPlatform(
        ((HlNNBinCnkFileHeader*)blob->data)->id);

    /* Fix header. */
    {
        /* Get header pointer. */
        HlNNBinCnkFileHeader* header =
            (HlNNBinCnkFileHeader*)blob->data;

        /* Fix header. */
        hlNNFixHeader(header);

        /* Store values from the header that we'll need later. */
        data = hlOff32Get(&header->dataOffset);
        NOF0Header = (HlNNBinCnkNOF0Header*)hlOff32Get(&header->NOF0Offset);
        dataChunkCount = header->chunkCount;
    }

    /* Fix all chunk data headers. */
    hlNNFixDataChunks(data, dataChunkCount);

    /* Fix all offsets referenced in the offset table stored within NOF0 chunk. */
    hlNNFixOffsets(NOF0Header, platform, data);
}

HlResult hlNNStartWrite(HlNNPlatform platform, HlStream* stream)
{
    /* Construct chunk file header, leaving values we can't properly set yet as 0. */
    HlNNBinCnkFileHeader header = HL_NN_INIT_CHUNK_HEADER(HL_NN_ID_HEADER, platform, 0x18);
    header.version = 1;

    /* Endian-swap if necessary for the given platform. */
    if (hlNNPlatformNeedsSwap(platform))
    {
        hlNNFileHeaderSwap(&header);
    }

    /* Write chunk file header to stream. */
    return hlStreamWrite(stream, sizeof(header), &header, NULL);
}

HlResult hlNNFinishWrite(size_t headerPos, size_t dataPos,
    size_t dataChunkCount, const HlOffTable* HL_RESTRICT offTable,
    const char* HL_RESTRICT optionalFileName, HlNNPlatform platform,
    HlStream* HL_RESTRICT stream)
{
    size_t dataEndPos, nof0EndPos, eof;
    HlResult result;

    /* Fix padding. */
    result = hlStreamPad(stream, hlNNPlatformGetPadSize(platform));
    if (HL_FAILED(result)) return result;

    /* Get data end position. */
    dataEndPos = hlStreamTell(stream);

    /* Write NOF0 chunk header. */
    {
        HlNNBinCnkNOF0Header nof0Header = { HL_NN_ID_OFFSET_LIST, 0,
            (hlNNPlatformNeedsSwap(platform) ? hlSwapU32((HlU32)offTable->count) :
            (HlU32)offTable->count)
        };

        result = hlStreamWrite(stream, sizeof(nof0Header), &nof0Header, NULL);
        if (HL_FAILED(result)) return result;
    }

    /* Write offset table. */
    {
        size_t i;
        for (i = 0; i < offTable->count; ++i)
        {
            /* Compute this offset's position relative to the data position. */
            HlU32 off = (HlU32)(offTable->data[i] - dataPos);

            /* Endian-swap if necessary for the given platform. */
            if (hlNNPlatformNeedsSwap(platform))
            {
                off = hlSwapU32(off);
            }

            /* Write relative offset position. */
            result = hlStreamWrite(stream, sizeof(off), &off, NULL);
            if (HL_FAILED(result)) return result;
        }
    }

    /* Fix padding. */
    result = hlStreamPad(stream, hlNNPlatformGetPadSize(platform));
    if (HL_FAILED(result)) return result;

    /* Get NOF0 end position. */
    nof0EndPos = hlStreamTell(stream);

    /* Fill-in NOF0 next ID offset. */
    {
        /* Jump to NOF0 next ID offset position. */
        HlU32 nof0NextIDOff;
        result = hlStreamJumpTo(stream, dataEndPos + 4);
        if (HL_FAILED(result)) return result;

        /* Compute value for NOF0 next ID offset. */
        nof0NextIDOff = (HlU32)(nof0EndPos - (dataEndPos + 8));

#ifdef HL_IS_BIG_ENDIAN
        /* This value is always little-endian; endian swap on big endian platforms. */
        nof0NextIDOff = hlSwapU32(nof0NextIDOff);
#endif

        /* Fill-in NOF0 next ID offset. */
        result = hlStreamWrite(stream, sizeof(nof0NextIDOff), &nof0NextIDOff, NULL);
        if (HL_FAILED(result)) return result;
    }

    /* Jump to NOF0 end position. */
    result = hlStreamJumpTo(stream, nof0EndPos);
    if (HL_FAILED(result)) return result;

    /* Write NFN0 chunk if requested. */
    if (optionalFileName)
    {
        /* Get NFN0 chunk position. */
        size_t nfn0Pos = hlStreamTell(stream);

        /* Write NFN0 chunk header. */
        HlNNBinCnkDataHeader nfn0Header = { HL_NN_ID_FILENAME };
        result = hlStreamWrite(stream, sizeof(nfn0Header), &nfn0Header, NULL);
        if (HL_FAILED(result)) return result;

        /* Write file name. */
        result = hlStreamWriteStringUTF8(stream, optionalFileName, NULL);
        if (HL_FAILED(result)) return result;

        /* Fix padding. */
        result = hlStreamPad(stream, hlNNPlatformGetPadSize(platform));
        if (HL_FAILED(result)) return result;

        /* Get end of stream position. */
        eof = hlStreamTell(stream);

        /* Jump to NFN0 next ID offset position. */
        result = hlStreamJumpTo(stream, nfn0Pos + 4);
        if (HL_FAILED(result)) return result;

        /* Compute value for NFN0 next ID offset. */
        nfn0Header.nextIDOffset = (HlU32)(eof - (nfn0Pos + 8));

#ifdef HL_IS_BIG_ENDIAN
        /* This value is always little-endian; endian swap on big endian platforms. */
        nfn0Header.nextIDOffset = hlSwapU32(nfn0Header.nextIDOffset);
#endif

        /* Fill-in NFN0 next ID offset. */
        result = hlStreamWrite(stream, 4, &nfn0Header.nextIDOffset, NULL);
        if (HL_FAILED(result)) return result;

        /* Jump to end of stream. */
        result = hlStreamJumpTo(stream, eof);
        if (HL_FAILED(result)) return result;
    }

    /* Write NEND chunk. */
    {
        HlNNBinCnkDataHeader nendHeader = { HL_NN_ID_END,
#ifdef HL_IS_BIG_ENDIAN
            /* This value is always little-endian; endian swap on big endian platforms. */
            hlSwapU32(hlNNPlatformGetENDChunkSize(platform))
#else
            hlNNPlatformGetENDChunkSize(platform)
#endif
        };

        result = hlStreamWrite(stream, sizeof(nendHeader), &nendHeader, NULL);
        if (HL_FAILED(result)) return result;

        /* Write padding if necessary. */
        if (platform == HL_NN_PLATFORM_GAMECUBE)
        {
            result = hlStreamWriteNulls(stream, 16, NULL);
            if (HL_FAILED(result)) return result;
        }
    }

    /* Get end of stream position. */
    eof = hlStreamTell(stream);

    /* Jump to header position. */
    result = hlStreamJumpTo(stream, headerPos + 8);
    if (HL_FAILED(result)) return result;

    /* Fill-in header values. */
    {
        /* Generate header. */
        HlNNBinCnkFileHeader header;
        header.chunkCount = (HlU32)dataChunkCount;
        header.dataOffset = (HlU32)(dataPos - headerPos);
        header.dataSize = (HlU32)(dataEndPos - dataPos);
        header.NOF0Offset = (HlU32)(dataEndPos - headerPos);
        header.NOF0Size = (HlU32)(nof0EndPos - dataEndPos);

        /* Endian-swap if necessary for the given platform. */
        if (hlNNPlatformNeedsSwap(platform))
        {
            hlNNFileHeaderSwap(&header);
        }

        /* Fill-in header values. */
        result = hlStreamWrite(stream, 0x14, &header.chunkCount, NULL);
        if (HL_FAILED(result)) return result;
    }

    /* Jump to end of stream and return. */
    return hlStreamJumpTo(stream, eof);
}

const HlNChar* hlNNPlatformGetFriendlyName(HlNNPlatform platform)
{
    switch (platform)
    {
    case HL_NN_PLATFORM_XBOX:           return HL_NTEXT("Xbox");
    case HL_NN_PLATFORM_PS2:            return HL_NTEXT("PlayStation2");
    case HL_NN_PLATFORM_GAMECUBE:       return HL_NTEXT("Gamecube");
    case HL_NN_PLATFORM_PS3:            return HL_NTEXT("PlayStation3");
    case HL_NN_PLATFORM_MOBILE:         return HL_NTEXT("Mobile");
    case HL_NN_PLATFORM_XBOX_EXTENDED:  return HL_NTEXT("Xbox_Extended");
    case HL_NN_PLATFORM_OTHER:          return HL_NTEXT("Other");
    default:                            return NULL;
    }
}

HlNNBinCnkDataHeader* hlNNGetDataChunk(const HlBlob* blob, HlU32 id)
{
    /* Get pointer to first data chunk in the file, if any. */
    const HlNNBinCnkFileHeader* header = (const HlNNBinCnkFileHeader*)blob->data;
    HlNNBinCnkDataHeader* curChunk = (HlNNBinCnkDataHeader*)hlOff32Get(&header->dataOffset);
    size_t i;

    /* Loop through all data chunks in file. */
    for (i = 0; i < header->chunkCount; ++i)
    {
        /* If this chunk's id matches, we found the chunk! Return it. */
        if (curChunk->id == id) return curChunk;
        
        /* Otherwise, loop through subsequent data chunks. */
        curChunk = hlNNGetNextChunk(curChunk);
    }

    /* We couldn't find a chunk with the requested id; return null. */
    return NULL;
}

#ifndef HL_NO_EXTERNAL_WRAPPERS
size_t hlNNPlatformGetPadSizeExt(HlNNPlatform platform)
{
    return hlNNPlatformGetPadSize(platform);
}

HlU32 hlNNPlatformGetENDChunkSizeExt(HlNNPlatform platform)
{
    return hlNNPlatformGetENDChunkSize(platform);
}

HlBool hlNNPlatformIsBigEndianExt(HlNNPlatform platform)
{
    return hlNNPlatformIsBigEndian(platform);
}

HlBool hlNNPlatformNeedsSwapExt(HlNNPlatform platform)
{
    return hlNNPlatformNeedsSwap(platform);
}

HlU32 hlNNPlatformMakeCnkIDExt(HlNNCnkID id, HlNNPlatform platform)
{
    return hlNNPlatformMakeCnkID(id, platform);
}

HlNNPlatform hlNNGetPlatformExt(HlNNCnkID id)
{
    return hlNNGetPlatform(id);
}

HlNNBinCnkDataHeader* hlNNGetDataHeaderExt(const HlBlob* blob)
{
    return hlNNGetDataHeader(blob);
}

void* hlNNGetDataExt(const HlBlob* blob)
{
    return hlNNGetData(blob);
}

HlNNBinCnkDataHeader* hlNNGetNextChunkExt(const HlNNBinCnkDataHeader* chunk)
{
    return hlNNGetNextChunk(chunk);
}

HlBool hlNNIsEndChunkExt(const HlNNBinCnkDataHeader* chunk)
{
    return hlNNIsEndChunk(chunk);
}
#endif
