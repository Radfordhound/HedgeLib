#include "hl_in_blob.h"
#include "hedgelib/hl_compression.h"
#include <lz4.h>
#define ZLIB_CONST
#include <zlib.h>
#include <cstring>

namespace hl
{
void lz4_decompress_no_alloc(std::size_t srcSize,
    const void* src, std::size_t dstSize, void* dst)
{
    // Decompress lz4 data.
    const int r = LZ4_decompress_safe(static_cast<const char*>(src),
        static_cast<char*>(dst), static_cast<int>(srcSize),
        static_cast<int>(dstSize));

    // Throw if decompressing failed.
    if (r < 0 || static_cast<std::size_t>(r) < dstSize)
    {
        throw std::runtime_error("Failed to decompress LZ4 data");
    }
}

void deflate_decompress_no_alloc(std::size_t srcSize,
    const void* src, std::size_t dstSize, void* dst)
{
    // Setup zlib stream.
    z_stream stream = { 0 };
    int r = inflateInit2(&stream, -MAX_WBITS);
    if (r < Z_OK)
    {
        throw std::runtime_error("Failed to initialize deflate stream");
    }

    stream.next_in = static_cast<z_const Bytef*>(src);
    stream.avail_in = static_cast<uInt>(srcSize);
    stream.next_out = static_cast<Bytef*>(dst);
    stream.avail_out = static_cast<uInt>(dstSize);

    // Decompress deflate data.
    // TODO: Should we use Z_SYNC_FLUSH?
    r = inflate(&stream, Z_SYNC_FLUSH);
    if (r < Z_OK)
    {
        throw std::runtime_error("Failed to decompress deflate data");
    }
}

static void in_none_decompress_no_alloc(std::size_t srcSize,
    const void* src, std::size_t dstSize, void* dst)
{
    // Ensure data can fit within destination buffer.
    if (srcSize > dstSize)
    {
        throw std::out_of_range("Destination buffer is not large enough "
            "to contain uncompressed data");
    }

    // Copy data to destination buffer.
    std::memcpy(dst, src, srcSize);
}

void decompress_no_alloc(compress_type type, std::size_t srcSize,
    const void* src, std::size_t dstSize, void* dst)
{
    switch (type)
    {
    case compress_type::none:
        in_none_decompress_no_alloc(srcSize, src, dstSize, dst);
        break;

    case compress_type::lz4:
        lz4_decompress_no_alloc(srcSize, src, dstSize, dst);
        break;

    case compress_type::deflate:
        deflate_decompress_no_alloc(srcSize, src, dstSize, dst);
        break;

    // TODO: Support all compress_type values!

    default:
        throw std::runtime_error("Unknown or unsupported compression type");
    }
}

std::unique_ptr<u8[]> decompress(compress_type type,
    std::size_t srcSize, const void* src, std::size_t dstSize)
{
    std::unique_ptr<u8[]> dst(new u8[dstSize]);
    decompress_no_alloc(type, srcSize, src, dstSize, dst.get());
    return dst;
}

blob decompress_blob(compress_type type, std::size_t srcSize,
    const void* src, std::size_t dstSize)
{
    blob dst(dstSize);
    decompress_no_alloc(type, srcSize, src, dstSize, dst);
    return dst;
}

std::size_t lz4_compress_bound(std::size_t uncompressedSize) noexcept
{
    return static_cast<std::size_t>(LZ4_compressBound(
        static_cast<int>(uncompressedSize)));
}

std::size_t deflate_compress_bound(std::size_t uncompressedSize) noexcept
{
    return static_cast<std::size_t>(::compressBound(
        static_cast<uLong>(uncompressedSize)));
}

std::size_t compress_bound(compress_type type,
    std::size_t uncompressedSize) noexcept
{
    switch (type)
    {
    case compress_type::none:
        return uncompressedSize;

    case compress_type::lz4:
        return lz4_compress_bound(uncompressedSize);

    case compress_type::deflate:
        return deflate_compress_bound(uncompressedSize);

    // TODO: Support all compress_type values!

    default:
        return 0;
    }
}

std::size_t lz4_compress_no_alloc(std::size_t srcSize,
    const void* src, std::size_t dstBufSize, void* dst)
{
    // Compress data.
    const int r = LZ4_compress_default(static_cast<const char*>(src),
        static_cast<char*>(dst), static_cast<int>(srcSize),
        static_cast<int>(dstBufSize));

    // Throw error if compression failed.
    if (r <= 0)
    {
        throw std::runtime_error("Failed to compress lz4 data");
    }
    
    return static_cast<std::size_t>(r);
}

std::size_t deflate_compress_no_alloc(std::size_t srcSize,
    const void* src, std::size_t dstBufSize, void* dst)
{
    // Setup zlib stream.
    z_stream stream = { 0 };
    int r = deflateInit2(&stream, Z_DEFAULT_COMPRESSION,
        Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);

    if (r < Z_OK)
    {
        throw std::runtime_error("Failed to initialize deflate stream");
    }

    stream.next_in = static_cast<z_const Bytef*>(src);
    stream.avail_in = static_cast<uInt>(srcSize);
    stream.next_out = static_cast<Bytef*>(dst);
    stream.avail_out = static_cast<uInt>(dstBufSize);

    // Decompress deflate data.
    r = deflate(&stream, Z_FINISH);
    if (r < Z_OK)
    {
        throw std::runtime_error("Failed to decompress deflate data");
    }

    const std::size_t dstSize = stream.total_out;
    deflateEnd(&stream);
    return dstSize;

    /*uLongf dstSize = static_cast<uLongf>(dstBufSize);
    int r = ::compress(static_cast<Bytef*>(dst), &dstSize,
        static_cast<z_const Bytef*>(src),
        static_cast<uLong>(srcSize));

    if (r != Z_OK)
    {
        throw std::runtime_error("Failed to compress zlib data");
    }

    return dstSize;*/
}

static std::size_t in_none_compress_no_alloc(std::size_t srcSize,
    const void* src, std::size_t dstBufSize, void* dst)
{
    // Ensure data can fit within destination buffer.
    if (srcSize > dstBufSize)
    {
        throw std::out_of_range("Destination buffer is not large enough "
            "to contain compressed data");
    }

    // Copy data to destination buffer.
    std::memcpy(dst, src, srcSize);
    return srcSize;
}

std::size_t compress_no_alloc(compress_type type,
    std::size_t srcSize, const void* src,
    std::size_t dstBufSize, void* dst)
{
    switch (type)
    {
    case compress_type::none:
        return in_none_compress_no_alloc(srcSize, src, dstBufSize, dst);

    case compress_type::lz4:
        return lz4_compress_no_alloc(srcSize, src, dstBufSize, dst);

    case compress_type::deflate:
        return deflate_compress_no_alloc(srcSize, src, dstBufSize, dst);

    // TODO: Support all compress_type values!

    default:
        throw std::runtime_error("Unknown or unsupported compression type");
    }
}

std::unique_ptr<u8[]> compress(compress_type type,
    std::size_t srcSize, const void* src, std::size_t& dstSize)
{
    // Allocate buffer big enough to hold compressed data.
    const std::size_t dstBufSize = compress_bound(type, srcSize);
    std::unique_ptr<u8[]> dst(new u8[dstBufSize]);

    // Compress data.
    dstSize = compress_no_alloc(type, srcSize, src, dstBufSize, dst.get());
    return dst;
}

blob compress_blob(compress_type type,
    std::size_t srcSize, const void* src)
{
    // Allocate blob big enough to hold compressed data.
    const std::size_t dstBufSize = compress_bound(type, srcSize);
    blob dst(dstBufSize);

    // Compress data.
    const std::size_t dstSize = compress_no_alloc(
        type, srcSize, src, dstBufSize, dst.data());

    in_blob_size_setter::set_size(dst, dstSize);
    return dst;
}
} // hl
