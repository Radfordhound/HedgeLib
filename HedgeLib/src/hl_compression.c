#include "hedgelib/hl_compression.h"
#include "hedgelib/hl_memory.h"
#include "depends/zlib/zlib.h"
#include <string.h>

HlResult hlZlibDecompressNoAlloc(const void* HL_RESTRICT compressedData,
    size_t compressedSize, size_t uncompressedSize,
    void* HL_RESTRICT uncompressedData)
{
    z_stream stream = { 0 };
    int result;

    /* If the data is already uncompressed, just copy it and return success. */
    if (compressedSize == uncompressedSize)
    {
        memcpy(uncompressedData, compressedData, uncompressedSize);
        return HL_RESULT_SUCCESS;
    }

    /* Otherwise, decompress the data. */
    result = inflateInit2(&stream, -MAX_WBITS);
    if (result < Z_OK) return HL_ERROR_UNKNOWN; /* TODO: Return more specific errors? */

    stream.next_in = (z_const Bytef*)compressedData;
    stream.avail_in = (uInt)compressedSize;

    stream.next_out = (Bytef*)uncompressedData;
    stream.avail_out = (uInt)uncompressedSize;

    result = inflate(&stream, Z_SYNC_FLUSH);

    return (result < Z_OK) ? /* TODO: Return more specific errors? */
        HL_ERROR_UNKNOWN : HL_RESULT_SUCCESS;
}

HlResult hlDecompressNoAlloc(HlCompressType compressionType,
    const void* HL_RESTRICT compressedData,
    size_t compressedSize, size_t uncompressedSize,
    void* HL_RESTRICT uncompressedData)
{
    switch (compressionType)
    {
    case HL_COMPRESS_TYPE_NONE:
        /* Ensure data can fit within destination buffer. */
        if (compressedSize > uncompressedSize) return HL_ERROR_OUT_OF_RANGE;

        /* Copy data to destination buffer and return success. */
        memcpy(uncompressedData, compressedData, compressedSize);
        return HL_RESULT_SUCCESS;

    /* TODO: Support all HlCompressType values. */

    case HL_COMPRESS_TYPE_ZLIB:
        return hlZlibDecompressNoAlloc(compressedData,
            compressedSize, uncompressedSize, uncompressedData);

    default: return HL_ERROR_UNSUPPORTED;
    }
}

HlResult hlDecompress(HlCompressType compressionType,
    const void* HL_RESTRICT compressedData,
    size_t compressedSize, size_t uncompressedSize,
    void* HL_RESTRICT * HL_RESTRICT uncompressedData)
{
    void* uncompressedDataBuf;
    HlResult result;

    /* Allocate a buffer to hold the uncompressed data. */
    uncompressedDataBuf = hlAlloc(uncompressedSize);
    if (!uncompressedDataBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Decompress the data. */
    result = hlDecompressNoAlloc(compressionType, compressedData,
        compressedSize, uncompressedSize, uncompressedDataBuf);

    if (HL_FAILED(result))
    {
        hlFree(uncompressedDataBuf);
        return result;
    }

    /* Set uncompressedData pointer and return success. */
    *uncompressedData = uncompressedDataBuf;
    return HL_RESULT_SUCCESS;
}

HlResult hlDecompressBlob(HlCompressType compressionType,
    const void* HL_RESTRICT compressedData,
    size_t compressedSize, size_t uncompressedSize,
    HlBlob* HL_RESTRICT * HL_RESTRICT uncompressedBlob)
{
    HlBlob* uncompressedBlobBuf;
    HlResult result;

    /* Create a blob to hold the uncompressed data. */
    result = hlBlobCreate(NULL, uncompressedSize,
        &uncompressedBlobBuf);

    if (HL_FAILED(result)) return result;

    /* Decompress the data. */
    result = hlDecompressNoAlloc(compressionType, compressedData,
        compressedSize, uncompressedSize, uncompressedBlobBuf->data);

    if (HL_FAILED(result))
    {
        hlBlobFree(uncompressedBlobBuf);
        return result;
    }

    /* Set uncompressedBlob pointer and return success. */
    *uncompressedBlob = uncompressedBlobBuf;
    return HL_RESULT_SUCCESS;
}

HlResult hlZlibCompressNoAlloc(const void* HL_RESTRICT uncompressedData,
    size_t uncompressedSize, size_t compressedBufSize,
    size_t* HL_RESTRICT compressedSize, void* HL_RESTRICT compressedBuf)
{
    /* TODO */
    return HL_ERROR_UNSUPPORTED;
}

HlResult hlCompressNoAlloc(HlCompressType compressionType,
    const void* HL_RESTRICT uncompressedData,
    size_t uncompressedSize, size_t compressedBufSize,
    size_t* HL_RESTRICT compressedSize, void* HL_RESTRICT compressedBuf)
{
    switch (compressionType)
    {
    case HL_COMPRESS_TYPE_NONE:
        /* Ensure data can fit within destination buffer. */
        if (uncompressedSize > compressedBufSize) return HL_ERROR_OUT_OF_RANGE;

        /* Copy data to destination buffer. */
        memcpy(compressedBuf, uncompressedData, uncompressedSize);

        /* Set compressedSize and return success. */
        if (compressedSize) *compressedSize = uncompressedSize;
        return HL_RESULT_SUCCESS;

    /* TODO: Support all HlCompressType values. */

    case HL_COMPRESS_TYPE_ZLIB:
        return hlZlibCompressNoAlloc(uncompressedData,
            uncompressedSize, compressedBufSize,
            compressedSize, compressedBuf);

    default: return HL_ERROR_UNSUPPORTED;
    }
}

HlResult hlCompress(HlCompressType compressionType,
    const void* HL_RESTRICT uncompressedData,
    size_t uncompressedSize, size_t* HL_RESTRICT compressedSize,
    void* HL_RESTRICT * HL_RESTRICT compressedData)
{
    void* compressedDataBuf;
    HlResult result;

    /* Allocate a buffer to hold the compressed data. */
    compressedDataBuf = hlAlloc(uncompressedSize);
    if (!compressedDataBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Compress the data. */
    result = hlCompressNoAlloc(compressionType, uncompressedData,
        uncompressedSize, uncompressedSize, compressedSize,
        compressedDataBuf);

    if (HL_FAILED(result))
    {
        hlFree(compressedDataBuf);
        return result;
    }

    /* Set compressedData pointer and return success. */
    *compressedData = compressedDataBuf;
    return HL_RESULT_SUCCESS;
}

HlResult hlCompressBlob(HlCompressType compressionType,
    const void* HL_RESTRICT uncompressedData,
    size_t uncompressedSize,
    HlBlob* HL_RESTRICT * HL_RESTRICT compressedBlob)
{
    HlBlob* compressedBlobBuf;
    HlResult result;

    /* Create a blob to hold the compressed data. */
    result = hlBlobCreate(NULL, uncompressedSize,
        &compressedBlobBuf);

    if (HL_FAILED(result)) return result;

    /* Compress the data. */
    result = hlCompressNoAlloc(compressionType, uncompressedData,
        uncompressedSize, uncompressedSize, &compressedBlobBuf->size,
        compressedBlobBuf->data);

    if (HL_FAILED(result))
    {
        hlBlobFree(compressedBlobBuf);
        return result;
    }

    /* Set compressedBlob pointer and return success. */
    *compressedBlob = compressedBlobBuf;
    return HL_RESULT_SUCCESS;
}
