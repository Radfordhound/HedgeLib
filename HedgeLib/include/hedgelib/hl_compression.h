#ifndef HL_COMPRESSION_H_INCLUDED
#define HL_COMPRESSION_H_INCLUDED
#include "hl_blob.h"

namespace hl
{
enum class compress_type
{
    /** @brief No compression. */
    none = 0,
    /** @brief CAB-Compressed. Generally used on Windows. */
    cab,
    /** @brief X-Compressed. Generally used on Xbox 360. */
    x,
    /** @brief SEGS-Compressed. Generally used on PlayStation 3. */
    segs,
    /** @brief LZ4-Compressed. Generally used on Switch and Windows. */
    lz4,
    /** @brief Deflate-Compressed. Generally used on Windows. */
    deflate
};

HL_API void lz4_decompress_no_alloc(std::size_t srcSize,
    const void* src, std::size_t dstSize, void* dst);

HL_API void deflate_decompress_no_alloc(std::size_t srcSize, 
    const void* src, std::size_t dstSize, void* dst);

HL_API void decompress_no_alloc(compress_type type, std::size_t srcSize,
    const void* src, std::size_t dstSize, void* dst);

HL_API std::unique_ptr<u8[]> decompress(compress_type type,
    std::size_t srcSize, const void* src, std::size_t dstSize);

HL_API blob decompress_blob(compress_type type, std::size_t srcSize,
    const void* src, std::size_t dstSize);

HL_API std::size_t lz4_compress_bound(std::size_t uncompressedSize) noexcept;

HL_API std::size_t deflate_compress_bound(std::size_t uncompressedSize) noexcept;

HL_API std::size_t compress_bound(compress_type type,
    std::size_t uncompressedSize) noexcept;

HL_API std::size_t lz4_compress_no_alloc(std::size_t srcSize,
    const void* src, std::size_t dstBufSize, void* dst);

HL_API std::size_t deflate_compress_no_alloc(std::size_t srcSize,
    const void* src, std::size_t dstBufSize, void* dst);

HL_API std::size_t compress_no_alloc(compress_type type,
    std::size_t srcSize, const void* src,
    std::size_t dstBufSize, void* dst);

HL_API std::unique_ptr<u8[]> compress(compress_type type,
    std::size_t srcSize, const void* src, std::size_t& dstSize);

HL_API blob compress_blob(compress_type type,
    std::size_t srcSize, const void* src);
} // hl
#endif
