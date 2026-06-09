#include "hl_in_blob.h"
#include "hedgelib/hl_compression.h"

#include <mspack.h>
#include <lzx.h>
#include <mszip.h>
#include <qtm.h>

// Hack to make the warning about "register" being deprecated shut up.
#define register
#include <system.c>
#include <lzxd.c>
#undef BITS_TYPE
#undef BITS_VAR
#undef BITS_ORDER_MSB
#undef BITS_ORDER_LSB
#undef BITS_LSB_TABLE
#undef READ_BYTES
#undef TABLEBITS
#undef MAXSYMBOLS
#undef HUFF_TABLE
#undef HUFF_LEN
#undef HUFF_ERROR
#undef HUFF_MAXBITS
#undef PEEK_BITS
#undef REMOVE_BITS
#undef INJECT_BITS
#undef HUFF_TRAVERSE
#undef MSPACK_READBITS_H
#undef MSPACK_READHUFF_H
#define read_input mszipd_read_input
#define make_decode_table mszip_make_decode_table
#include <mszipd.c>
#undef make_decode_table
#undef read_input
#include <cabd.c>
#undef register

#include <lz4.h>

#define ZLIB_CONST
#ifdef ZLIB_DLL
#undef ZLIB_DLL
#endif
#include <zlib.h>

#include <cstring>

struct qtmd_stream* qtmd_init(struct mspack_system*,
    struct mspack_file*, struct mspack_file*, int, int)
{
    return nullptr;
}

int qtmd_decompress(struct qtmd_stream*, off_t)
{
    return MSPACK_ERR_DECRUNCH;
}

void qtmd_free(struct qtmd_stream*)
{
}

namespace hl
{
struct in_mspack_read_stream
{
    const u8* data = nullptr;
    int size = 0; // Size from every compressed block.
};

// libmspack interface implementation for Xbox decompression.
static int in_mspack_read(mspack_file* file, void* buffer, int bytes)
{
    in_mspack_read_stream* stream = reinterpret_cast<in_mspack_read_stream*>(file);

    if (stream->size == 0)
    {
        u16 size = *reinterpret_cast<const u16*>(stream->data);
        stream->data += sizeof(u16);

#ifndef HL_IS_BIG_ENDIAN
        hl::endian_swap(size);
#endif

        // This indicates there is an uncompressed block size available. We don't need it so we skip it.
        if ((size & 0xFF00) == 0xFF00)
        {
            stream->data += 1;
            size = *reinterpret_cast<const u16*>(stream->data);
            stream->data += sizeof(u16);

#ifndef HL_IS_BIG_ENDIAN
            hl::endian_swap(size);
#endif
        }

        stream->size = size;
    }

    int sizeToRead = std::min(stream->size, bytes);

    memcpy(buffer, stream->data, sizeToRead);
    stream->data += sizeToRead;
    stream->size -= sizeToRead;

    return sizeToRead;
}

struct in_mspack_write_stream
{
    u8* data = nullptr;
    std::size_t size = 0; // Remaining available space in the stream.
};

static int in_mspack_write(mspack_file* file, void* buffer, int bytes)
{
    in_mspack_write_stream* stream = reinterpret_cast<in_mspack_write_stream*>(file);

    std::size_t sizeToWrite = std::min(stream->size, static_cast<std::size_t>(bytes));

    memcpy(stream->data, buffer, sizeToWrite);
    stream->data += sizeToWrite;
    stream->size -= sizeToWrite;

    return static_cast<int>(sizeToWrite);
}

static void* in_mspack_alloc(mspack_system* self, size_t bytes)
{
    return operator new(bytes);
}

static void in_mspack_free(void* ptr)
{
    operator delete(ptr);
}

static void in_mspack_copy(void* src, void* dst, size_t bytes)
{
    memcpy(dst, src, bytes);
}

static mspack_system in_lzx_system =
{
    nullptr,
    nullptr,
    in_mspack_read,
    in_mspack_write,
    nullptr,
    nullptr,
    nullptr,
    in_mspack_alloc,
    in_mspack_free,
    in_mspack_copy
};

static constexpr u32 in_cab_signature = 0x4643534D;

struct in_mspack_cab_in_stream
{
    const u8* data = nullptr;
    std::size_t size = 0;
};

struct in_mspack_cab_out_stream
{
    u8* data = nullptr;
    std::size_t size = 0;
    std::size_t written = 0;
};

struct in_mspack_cab_stream
{
    u8* data = nullptr;
    std::size_t size = 0;
    std::size_t pos = 0;
    std::size_t* written = nullptr;
    bool isRead = false;
};

static mspack_file* in_mspack_cab_open(mspack_system* self,
    const char* filename, int mode)
{
    in_mspack_cab_stream* stream = static_cast<in_mspack_cab_stream*>(
        in_mspack_alloc(self, sizeof(in_mspack_cab_stream)));

    stream->pos = 0;
    stream->written = nullptr;
    stream->isRead = (mode == MSPACK_SYS_OPEN_READ);

    if (stream->isRead)
    {
        const in_mspack_cab_in_stream* inStream =
            reinterpret_cast<const in_mspack_cab_in_stream*>(filename);

        stream->data = const_cast<u8*>(inStream->data);
        stream->size = inStream->size;
    }
    else
    {
        in_mspack_cab_out_stream* outStream =
            reinterpret_cast<in_mspack_cab_out_stream*>(const_cast<char*>(filename));

        stream->data = outStream->data;
        stream->size = outStream->size;
        stream->written = &outStream->written;

        if (mode == MSPACK_SYS_OPEN_APPEND)
        {
            stream->pos = outStream->written;
        }
        else
        {
            outStream->written = 0;
        }
    }

    return reinterpret_cast<mspack_file*>(stream);
}

static void in_mspack_cab_close(mspack_file* file)
{
    in_mspack_free(file);
}

static int in_mspack_cab_read(mspack_file* file, void* buffer, int bytes)
{
    in_mspack_cab_stream* stream = reinterpret_cast<in_mspack_cab_stream*>(file);

    if (!stream->isRead)
    {
        return -1;
    }

    const std::size_t sizeToRead = std::min(
        stream->size - stream->pos, static_cast<std::size_t>(bytes));

    std::memcpy(buffer, stream->data + stream->pos, sizeToRead);
    stream->pos += sizeToRead;
    return static_cast<int>(sizeToRead);
}

static int in_mspack_cab_write(mspack_file* file, void* buffer, int bytes)
{
    in_mspack_cab_stream* stream = reinterpret_cast<in_mspack_cab_stream*>(file);

    if (stream->isRead)
    {
        return -1;
    }

    const std::size_t sizeToWrite = std::min(
        stream->size - stream->pos, static_cast<std::size_t>(bytes));

    std::memcpy(stream->data + stream->pos, buffer, sizeToWrite);
    stream->pos += sizeToWrite;

    if (stream->written && *stream->written < stream->pos)
    {
        *stream->written = stream->pos;
    }

    return static_cast<int>(sizeToWrite);
}

static int in_mspack_cab_seek(mspack_file* file, off_t offset, int mode)
{
    in_mspack_cab_stream* stream = reinterpret_cast<in_mspack_cab_stream*>(file);
    off_t targetPos = 0;

    switch (mode)
    {
    case MSPACK_SYS_SEEK_START:
        targetPos = offset;
        break;

    case MSPACK_SYS_SEEK_CUR:
        targetPos = static_cast<off_t>(stream->pos) + offset;
        break;

    case MSPACK_SYS_SEEK_END:
        targetPos = static_cast<off_t>(stream->size) + offset;
        break;

    default:
        return 1;
    }

    if (targetPos < 0 || static_cast<std::size_t>(targetPos) > stream->size)
    {
        return 1;
    }

    stream->pos = static_cast<std::size_t>(targetPos);
    return 0;
}

static off_t in_mspack_cab_tell(mspack_file* file)
{
    const in_mspack_cab_stream* stream =
        reinterpret_cast<const in_mspack_cab_stream*>(file);

    return static_cast<off_t>(stream->pos);
}

static void in_mspack_cab_message(mspack_file*, const char*, ...)
{
}

static mspack_system in_cab_system =
{
    in_mspack_cab_open,
    in_mspack_cab_close,
    in_mspack_cab_read,
    in_mspack_cab_write,
    in_mspack_cab_seek,
    in_mspack_cab_tell,
    in_mspack_cab_message,
    in_mspack_alloc,
    in_mspack_free,
    in_mspack_copy
};

static mscabd_file* in_mspack_cab_get_file(mscabd_cabinet* cab)
{
    mscabd_file* file = cab->files;
    return (file && !file->next) ? file : nullptr;
}

struct in_cab_context
{
    in_mspack_cab_in_stream srcStream;
    mscab_decompressor* cabd = nullptr;
    mscabd_cabinet* cab = nullptr;
    mscabd_file* file = nullptr;

    ~in_cab_context()
    {
        if (cabd && cab) cabd->close(cabd, cab);
        if (cabd) mspack_destroy_cab_decompressor(cabd);
    }
};

static bool in_cab_open_single_file(std::size_t srcSize,
    const void* src, in_cab_context& context)
{
    context.srcStream.data = static_cast<const u8*>(src);
    context.srcStream.size = srcSize;

    context.cabd = mspack_create_cab_decompressor(&in_cab_system);
    if (!context.cabd)
    {
        return false;
    }

    context.cab = context.cabd->open(context.cabd,
        reinterpret_cast<const char*>(&context.srcStream));

    if (!context.cab)
    {
        return false;
    }

    context.file = in_mspack_cab_get_file(context.cab);
    return (context.file != nullptr);
}

// Xbox Compression header definitions.
static constexpr u32 x_compress_signature = 0xFF512EE;

struct x_compress_header
{
    u32 signature;
    u32 field04;
    u32 field08;
    u32 field0C;
    u32 windowSize;
    u32 compressedBlockSize;
    u64 uncompressedSize;
    u64 compressedSize;
    u32 uncompressedBlockSize;
    u32 field2C;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(signature);
        hl::endian_swap(field04);
        hl::endian_swap(field08);
        hl::endian_swap(field0C);
        hl::endian_swap(windowSize);
        hl::endian_swap(compressedBlockSize);
        hl::endian_swap(uncompressedSize);
        hl::endian_swap(compressedSize);
        hl::endian_swap(uncompressedBlockSize);
        hl::endian_swap(field2C);
    }
};

static x_compress_header in_parse_x_compress_header(const void* src)
{
    x_compress_header header = *reinterpret_cast<const x_compress_header*>(src);
#ifndef HL_IS_BIG_ENDIAN
    header.endian_swap();
#endif
    return header;
}

bool cab_check_signature(std::size_t srcSize, const void* src)
{
    if (srcSize < sizeof(u32))
    {
        return false;
    }

    u32 sig = 0;
    std::memcpy(&sig, src, sizeof(sig));
    return sig == in_cab_signature;
}

std::size_t cab_get_uncompressed_size(std::size_t srcSize, const void* src)
{
    in_cab_context context;
    if (!in_cab_open_single_file(srcSize, src, context))
        return 0;

    return context.file->length;
}

blob cab_decompress(std::size_t srcSize, const void* src)
{
    in_cab_context context;
    if (!in_cab_open_single_file(srcSize, src, context))
    {
        throw std::out_of_range("");
    }

    blob dst(context.file->length);
    in_mspack_cab_out_stream dstStream;
    dstStream.data = static_cast<u8*>(dst.data());
    dstStream.size = dst.size();

    const int r = context.cabd->extract(context.cabd, context.file,
        reinterpret_cast<const char*>(&dstStream));

    if (r != MSPACK_ERR_OK)
    {
        throw std::out_of_range("");
    }

    return dst;
}

void cab_decompress_no_alloc(std::size_t srcSize,
    const void* src, std::size_t dstSize, void* dst)
{
    in_cab_context context;
    if (!in_cab_open_single_file(srcSize, src, context))
        return;

    in_mspack_cab_out_stream dstStream;
    dstStream.data = static_cast<u8*>(dst);
    dstStream.size = dstSize;

    if (context.file->length > dstSize)
    {
        throw std::out_of_range("");
    }

    const int r = context.cabd->extract(context.cabd, context.file,
        reinterpret_cast<const char*>(&dstStream));

    if (r != MSPACK_ERR_OK)
    {
        throw std::out_of_range("");
    }
}

bool x_check_signature(std::size_t srcSize, const void* src)
{
    // Should at least be the header size.
    if (srcSize >= sizeof(x_compress_header))
    {
        x_compress_header header = in_parse_x_compress_header(src);
        return header.signature == x_compress_signature;
    }

    return false;
}

std::size_t x_get_uncompressed_size(std::size_t srcSize, const void* src)
{
    x_compress_header header = in_parse_x_compress_header(src);
    return header.uncompressedSize;
}

void x_decompress_no_alloc(std::size_t srcSize,
    const void* src, std::size_t dstSize, void* dst)
{
    x_compress_header header = *reinterpret_cast<const x_compress_header*>(src);
#ifndef HL_IS_BIG_ENDIAN
    header.endian_swap();
#endif

    if (header.uncompressedSize > dstSize)
    {
        throw std::out_of_range("Destination buffer is not large enough "
            "to contain uncompressed data");
    }

    const u8* srcBytes = hl::ptradd<u8>(src, sizeof(x_compress_header));

    in_mspack_write_stream dstStream;
    dstStream.data = reinterpret_cast<uint8_t*>(dst);
    dstStream.size = header.uncompressedSize;

    // libmspack wants the bit index. This value is always guaranteed to be a power of two,
    // so we can extract the bit index by counting the amount of leading zeroes.
    int windowBits = 0;
    u32 windowSize = header.windowSize;
    while ((windowSize & 0x1) == 0)
    {
        ++windowBits;
        windowSize >>= 1;
    }

    // Loop over compressed blocks.
    while (srcBytes < hl::ptradd<u8>(src, srcSize) && dstStream.data < hl::ptradd<u8>(dst, header.uncompressedSize))
    {
        u32 compressedSize = *reinterpret_cast<const u32*>(srcBytes);
#ifndef HL_IS_BIG_ENDIAN
        hl::endian_swap(compressedSize);
#endif
        srcBytes += sizeof(u32);

        in_mspack_read_stream srcStream;
        srcStream.data = srcBytes;

        std::size_t uncompressedBlockSize = std::min(static_cast<std::size_t>(header.uncompressedBlockSize), dstStream.size);

        lzxd_stream* lzx = lzxd_init(
            &in_lzx_system,
            reinterpret_cast<mspack_file*>(&srcStream),
            reinterpret_cast<mspack_file*>(&dstStream),
            windowBits,
            0,
            static_cast<int>(header.compressedBlockSize),
            static_cast<off_t>(uncompressedBlockSize),
            0);

        lzxd_decompress(lzx, uncompressedBlockSize);
        lzxd_free(lzx);

        srcBytes += compressedSize;
    }
}

// PS3 Compression header definitions.
static constexpr u32 segs_signature = 0x73656773;

struct segs_header
{
    uint32_t signature;
    uint16_t flags;
    uint16_t chunkCount;
    uint32_t uncompressedSize;
    uint32_t compressedSize;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(signature);
        hl::endian_swap(flags);
        hl::endian_swap(chunkCount);
        hl::endian_swap(uncompressedSize);
        hl::endian_swap(compressedSize);
    }
};

struct segs_chunk
{
    uint16_t compressedSize;
    uint16_t uncompressedSize;
    uint32_t dataOffset;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(compressedSize);
        hl::endian_swap(uncompressedSize);
        hl::endian_swap(dataOffset);
    }
};

static segs_header in_parse_segs_header(const void* src)
{
    segs_header header = *reinterpret_cast<const segs_header*>(src);
#ifndef HL_IS_BIG_ENDIAN
    header.endian_swap();
#endif
    return header;
}

bool segs_check_signature(std::size_t srcSize, const void* src)
{
    // Should at least be the header size.
    if (srcSize >= sizeof(segs_header))
    {
        segs_header header = in_parse_segs_header(src);
        return header.signature == segs_signature;
    }

    return false;
}

std::size_t segs_get_uncompressed_size(std::size_t srcSize, const void* src)
{
    segs_header header = in_parse_segs_header(src);
    return header.uncompressedSize;
}

void segs_decompress_no_alloc(std::size_t srcSize, const void* src, std::size_t dstSize, void* dst)
{
    segs_header header = *reinterpret_cast<const segs_header*>(src);
#ifndef HL_IS_BIG_ENDIAN
    header.endian_swap();
#endif

    if (header.uncompressedSize > dstSize)
    {
        throw std::out_of_range("Destination buffer is not large enough "
            "to contain uncompressed data");
    }

    const segs_chunk* chunks = ptradd<segs_chunk>(src, sizeof(segs_header));
    uint8_t* dstPtr = static_cast<uint8_t*>(dst);

    for (std::size_t i = 0; i < header.chunkCount; i++)
    {
        segs_chunk chunk = chunks[i];
#ifndef HL_IS_BIG_ENDIAN
        chunk.endian_swap();
#endif

        // Uncompressed chunk, just copy the data.
        if (chunk.compressedSize == chunk.uncompressedSize)
        {
            std::memcpy(dstPtr, ptradd<uint8_t>(src, chunk.dataOffset), chunk.compressedSize);
            dstPtr += chunk.compressedSize;
            continue;
        }

        // If the uncompressed size is 0, it's actually 65536.
        const std::size_t chunkSize = chunk.uncompressedSize == 0 ? 
            0x10000 : chunk.uncompressedSize;

        // Sonic Unleashed PS3 only uses deflate compression.
        deflate_decompress_no_alloc(chunk.compressedSize,
            ptradd<uint8_t>(src, chunk.dataOffset - 1), chunkSize, dstPtr);

        dstPtr += chunkSize;
    }
}

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
    r = inflate(&stream, Z_FINISH);
    inflateEnd(&stream);
    if (r != Z_STREAM_END)
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

    case compress_type::cab:
        cab_decompress_no_alloc(srcSize, src, dstSize, dst);
        break;

    case compress_type::x:
        x_decompress_no_alloc(srcSize, src, dstSize, dst);
        break;

    case compress_type::segs:
        segs_decompress_no_alloc(srcSize, src, dstSize, dst);
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
