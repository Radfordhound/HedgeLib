#include "hedgelib/io/hl_stream.h"
#include <memory>

namespace hl
{
stream::~stream() {}

void stream::read_all(std::size_t size, void* buf)
{
    const std::size_t readByteCount = read(size, buf);
    if (readByteCount != size)
    {
        // TODO: Throw a better error?
        throw unknown_exception();
    }
}

void stream::write_all(std::size_t size, const void* buf)
{
    const std::size_t writtenByteCount = write(size, buf);
    if (writtenByteCount != size)
    {
        // TODO: Throw a better error?
        throw unknown_exception();
    }
}

static const u8 in_stream_nulls_static_buffer[1024] = { 0 };

void stream::write_nulls(std::size_t amount)
{
    if (amount > sizeof(in_stream_nulls_static_buffer))
    {
        // Allocate a buffer large enough to hold all of the nulls we want to write.
        std::unique_ptr<u8[]> nulls(new u8[amount]());

        // Write the nulls to the stream.
        write_arr(amount, nulls.get());
    }
    else
    {
        // Write the given amount of nulls to the stream using our static nulls buffer.
        write_arr(amount, in_stream_nulls_static_buffer);
    }
}

void stream::write_off32(std::size_t basePos, std::size_t offVal,
    bool doSwap, off_table& offTable)
{
    // Compute offset.
    u32 off = static_cast<u32>(offVal - basePos);

    // Swap offset if necessary.
    if (doSwap) endian_swap(off);

    // Add offset position to offset table.
    offTable.push_back(tell());

    // Write offset to stream.
    write_obj(off);
}

void stream::write_off64(std::size_t basePos, std::size_t offVal,
    bool doSwap, off_table& offTable)
{
    // Compute offset.
    u64 off = static_cast<u64>(offVal - basePos);

    // Swap offset if necessary.
    if (doSwap) endian_swap(off);

    // Add offset position to offset table.
    offTable.push_back(tell());

    // Write offset to stream.
    write_obj(off);
}

void stream::fix_off32(std::size_t basePos, std::size_t offPos,
    bool doSwap, off_table& offTable)
{
    // Get end of stream position.
    const std::size_t endPos = tell();

    // Jump to the given offset position.
    jump_to(offPos);

    // Fix the offset.
    write_off32(basePos, endPos, doSwap, offTable);

    // Jump back to end of stream.
    jump_to(endPos);
}

void stream::fix_off64(std::size_t basePos, std::size_t offPos,
    bool doSwap, off_table& offTable)
{
    // Get end of stream position.
    const std::size_t endPos = tell();

    // Jump to the given offset position.
    jump_to(offPos);

    // Fix the offset.
    write_off64(basePos, endPos, doSwap, offTable);

    // Jump back to end of stream.
    jump_to(endPos);
}

bool stream::read_str(std::size_t bufSize, char* buf)
{
    constexpr std::size_t tmpBufSize = 20;
    char tmpBuf[tmpBufSize];

    // Return early if bufSize == 0
    if (!bufSize) return true;

    // Read string into buffer.
    while (true)
    {
        const std::size_t readByteCount = read(tmpBufSize, tmpBuf);
        const std::size_t safeWriteCount = std::min(readByteCount, bufSize);
        
        // Append a null terminator and return failure
        // if we've reached the end of the stream.
        if (readByteCount == 0)
        {
            *(buf++) = '\0';
            return false;
        }

        // Otherwise, append all of the bytes we just read into the temporary buffer.
        for (std::size_t i = 0; i < safeWriteCount; ++i)
        {
            *(buf++) = tmpBuf[i];

            // Jump to end of string and return success if we read a null terminator.
            if (tmpBuf[i] == '\0')
            {
                jump_behind(static_cast<long long>(readByteCount - (i + 1)));
                return true;
            }
        }

        // Decrease writable buffer size.
        bufSize -= safeWriteCount;

        // Raise an error if we've exceeded the buffer size.
        if (!bufSize)
        {
            throw out_of_range_exception();
        }
    }
}

std::string stream::read_str()
{
    constexpr std::size_t tmpBufSize = 20;
    std::string str;
    char tmpBuf[tmpBufSize];

    while (true)
    {
        const std::size_t readByteCount = read(tmpBufSize, tmpBuf);

        // Return if we've reached the end of the stream.
        if (readByteCount == 0) return str;

        // Otherwise, append all of the bytes we just read into the temporary buffer.
        for (std::size_t i = 0; i < readByteCount; ++i)
        {
            // Jump to end of string and return if we read a null terminator.
            if (tmpBuf[i] == '\0')
            {
                jump_behind(static_cast<long long>(readByteCount - (i + 1)));
                return str;
            }

            // Append characters.
            str += tmpBuf[i];
        }
    }
}

void stream::align(std::size_t stride)
{
    std::size_t pos;

    // If stride is < 2, we don't need to align.
    if (stride-- < 2) return;

    /* Get the current stream position. */
    pos = tell();

    // Compute the closest position in the stream that's aligned
    // by the given stride, and jump to that position.
    jump_to(((pos + stride) & ~stride));
}

void stream::pad(std::size_t stride)
{
    std::size_t pos;

    // If stride is < 2, we don't need to pad.
    if (stride-- < 2) return;

    // Get the current stream position.
    pos = tell();

    // Compute the amount of nulls we need to write to align the
    // stream with the given stride, and write that many nulls.
    write_nulls(((pos + stride) & ~stride) - pos);
}
} // hl
