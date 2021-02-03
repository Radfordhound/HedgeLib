#ifndef HL_COMPRESSION_H_INCLUDED
#define HL_COMPRESSION_H_INCLUDED
#include "hl_blob.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum HlCompressType
{
    /** @brief No compression. */
    HL_COMPRESS_TYPE_NONE = 0,
    /** @brief CAB-Compressed. Generally used on Windows. */
    HL_COMPRESS_TYPE_CAB,
    /** @brief X-Compressed. Generally used on Xbox 360. */
    HL_COMPRESS_TYPE_XCOMPRESS,
    /** @brief SEGS-Compressed. Generally used on PlayStation 3. */
    HL_COMPRESS_TYPE_SEGS,
    /** @brief LZ4-Compressed. Generally used on Switch and Windows. */
    HL_COMPRESS_TYPE_LZ4,
    /** @brief zlib/Deflate-Compressed. Generally used on Windows. */
    HL_COMPRESS_TYPE_ZLIB
}
HlCompressType;

HL_API HlResult hlLZ4DecompressNoAlloc(const void* HL_RESTRICT compressedData,
    size_t compressedSize, size_t uncompressedSize,
    void* HL_RESTRICT uncompressedData);

HL_API HlResult hlZlibDecompressNoAlloc(const void* HL_RESTRICT compressedData,
    size_t compressedSize, size_t uncompressedSize,
    void* HL_RESTRICT uncompressedData);

HL_API HlResult hlDecompressNoAlloc(HlCompressType compressionType,
    const void* HL_RESTRICT compressedData,
    size_t compressedSize, size_t uncompressedSize,
    void* HL_RESTRICT uncompressedData);

HL_API HlResult hlDecompress(HlCompressType compressionType,
    const void* HL_RESTRICT compressedData,
    size_t compressedSize, size_t uncompressedSize,
    void* HL_RESTRICT * HL_RESTRICT uncompressedData);

HL_API HlResult hlDecompressBlob(HlCompressType compressionType,
    const void* HL_RESTRICT compressedData,
    size_t compressedSize, size_t uncompressedSize,
    HlBlob* HL_RESTRICT * HL_RESTRICT uncompressedBlob);

HL_API size_t hlLZ4CompressBound(size_t uncompressedSize);

HL_API HlResult hlLZ4CompressNoAlloc(const void* HL_RESTRICT uncompressedData,
    size_t uncompressedSize, size_t compressedBufSize,
    size_t* HL_RESTRICT compressedSize, void* HL_RESTRICT compressedBuf);

HL_API size_t hlZlibCompressBound(size_t uncompressedSize);

HL_API HlResult hlZlibCompressNoAlloc(const void* HL_RESTRICT uncompressedData,
    size_t uncompressedSize, size_t compressedBufSize,
    size_t* HL_RESTRICT compressedSize, void* HL_RESTRICT compressedBuf);

HL_API size_t hlCompressBound(HlCompressType compressionType,
    size_t uncompressedSize);

HL_API HlResult hlCompressNoAlloc(HlCompressType compressionType,
    const void* HL_RESTRICT uncompressedData,
    size_t uncompressedSize, size_t compressedBufSize,
    size_t* HL_RESTRICT compressedSize, void* HL_RESTRICT compressedBuf);

HL_API HlResult hlCompress(HlCompressType compressionType,
    const void* HL_RESTRICT uncompressedData,
    size_t uncompressedSize, size_t* HL_RESTRICT compressedSize,
    void* HL_RESTRICT * HL_RESTRICT compressedData);

HL_API HlResult hlCompressBlob(HlCompressType compressionType,
    const void* HL_RESTRICT uncompressedData,
    size_t uncompressedSize,
    HlBlob* HL_RESTRICT * HL_RESTRICT compressedBlob);

#ifdef __cplusplus
}
#endif
#endif
