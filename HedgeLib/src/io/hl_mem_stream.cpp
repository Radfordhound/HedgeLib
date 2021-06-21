#include "hedgelib/io/hl_mem_stream.h"
#include <cstring>
#include <cstdlib>

namespace hl
{
std::size_t readonly_mem_stream::read(std::size_t size, void* buf)
{
    // Get current data pointer and determine how many bytes we can safely read.
    const u8* ptr = (m_handle + m_curPos);
    const std::size_t canRead = (m_dataSize - m_curPos);
    const std::size_t readBytes = std::min(size, canRead);

    // Copy as many bytes as we safely can into the given buffer.
    std::memcpy(buf, ptr, readBytes);

    // Increase stream current position.
    m_curPos += readBytes;

    // Return read byte count.
    return readBytes;
}

std::size_t readonly_mem_stream::write(std::size_t size, const void* buf)
{
    HL_ERROR(error_type::unsupported);
}

void readonly_mem_stream::seek(seek_mode mode, long long offset)
{
    // Get base position.
    std::size_t pos;
    switch (mode)
    {
    default:
    case seek_mode::beg:
        pos = 0;
        break;

    case seek_mode::cur:
        pos = m_curPos;
        break;

    case seek_mode::end:
        pos = m_dataSize;
        break;
    }

    // Subtract from pos if offset is negative.
    if (offset < 0)
    {
        // Get positive version of offset and ensure we can subtract it from pos safely.
        const std::size_t dif = static_cast<std::size_t>(std::abs(offset));
        if (dif > pos)
        {
            HL_ERROR(error_type::out_of_range);
        }

        // Subtract dif from pos.
        pos -= dif;
    }

    // Add to pos if offset is positive.
    else
    {
        // Add offset to pos.
        pos += offset;

        // Ensure we can add offset to m_curPos safely.
        if (pos > m_dataSize)
        {
            HL_ERROR(error_type::out_of_range);
        }
    }

    // Set stream current position.
    m_curPos = pos;
}

void readonly_mem_stream::jump_to(std::size_t pos)
{
    // Ensure this position is contained within the data buffer.
    if (pos > m_dataSize)
    {
        HL_ERROR(error_type::out_of_range);
    }

    // Set stream current position.
    m_curPos = pos;
}

void readonly_mem_stream::flush() {}

std::size_t readonly_mem_stream::get_size()
{
    return m_dataSize;
}

readonly_mem_stream::~readonly_mem_stream() {}

void mem_stream::in_grow(std::size_t reqCap)
{
    // Compute new buffer capacity.
    // (NOTE: We purposely allocate more than necessary
    // to avoid subsequent reallocations.)
    std::size_t newDataCap = ((m_dataCap + reqCap) * 2);
    
    // Align new buffer capacity to 1024 to further avoid subsequent reallocations.
    newDataCap = hl::align(newDataCap, 1024);

    // Allocate new data buffer.
    std::unique_ptr<u8[]> newDataBuf(new u8[newDataCap]);
    u8* dataBuf = const_cast<u8*>(m_handle);
    
    // Copy existing data into new buffer.
    std::memcpy(newDataBuf.get(), dataBuf, m_dataSize);

    // Free existing data buffer.
    delete[] dataBuf;

    // Set new buffer pointer and capacity.
    m_handle = newDataBuf.release();
    m_dataCap = newDataCap;
}

std::size_t mem_stream::write(std::size_t size, const void* buf)
{
    // Get current data pointer and determine how many bytes we can write.
    u8* dataBuf = const_cast<u8*>(m_handle);
    const std::size_t canWrite = (m_dataCap - m_curPos);

    // Enlarge data buffer if necessary.
    if (size > canWrite)
    {
        in_grow(m_curPos + size);
        dataBuf = const_cast<u8*>(m_handle);
    }

    // Copy data into buffer.
    std::memcpy(dataBuf + m_curPos, buf, size);

    // Increase stream current position.
    m_curPos += size;

    // Increase stream buffer size if necessary.
    if (m_curPos > m_dataSize)
    {
        m_dataSize = m_curPos;
    }

    // Return written byte count.
    return size;
}

mem_stream::~mem_stream()
{
    close();
}

void* mem_stream::release() noexcept
{
    // Store current data pointer.
    u8* ptr = const_cast<u8*>(m_handle);

    // Reset the memory stream.
    m_curPos = m_dataSize = 0;
    m_handle = nullptr;

    // Return data pointer; it is now the user's responsibility to free it.
    return ptr;
}

void mem_stream::close()
{
    // Return early if data has already been freed.
    u8* dataBuf = const_cast<u8*>(m_handle);
    if (!dataBuf) return;

    // Free data and set m_handle to null so we can detect it on any subsequent calls to close.
    delete[] dataBuf;
    m_handle = nullptr;
}
    
void mem_stream::reopen() noexcept
{
    // Reset the memory stream.
    m_curPos = m_dataSize = 0;
}

void mem_stream::reopen(std::size_t initialBufCapacity)
{
    // Reset the memory stream.
    reopen();

    // Enlarge data buffer if necessary.
    if (initialBufCapacity > m_dataCap)
    {
        in_grow(initialBufCapacity);
    }
}

void mem_stream::reopen(const void* initialData,
    std::size_t initialDataSize)
{
    // Reset the memory stream and enlarge data buffer if necessary.
    reopen(initialDataSize);

    // Copy initial data into internal data buffer.
    u8* dataBuf = const_cast<u8*>(m_handle);
    std::memcpy(dataBuf, initialData, initialDataSize);

    // Set data size.
    m_dataSize = initialDataSize;
}

mem_stream::mem_stream(const void* initialData, std::size_t initialDataSize) :
    readonly_mem_stream(new u8[initialDataSize], initialDataSize),
    m_dataCap(initialDataSize)
{
    // Copy initial data into internal data buffer.
    u8* dataBuf = const_cast<u8*>(m_handle);
    std::memcpy(dataBuf, initialData, initialDataSize);
}
} // hl
