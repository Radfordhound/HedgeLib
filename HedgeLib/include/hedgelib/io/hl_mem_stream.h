#ifndef HL_MEM_STREAM_H_INCLUDED
#define HL_MEM_STREAM_H_INCLUDED
#include "hl_stream.h"
#include "../hl_blob.h"

namespace hl
{
class readonly_mem_stream : public stream
{
protected:
    const u8* m_handle;
    std::size_t m_dataSize;

public:
    std::size_t read(std::size_t size, void* buf) override;
    std::size_t write(std::size_t size, const void* buf) override;
    void seek(seek_mode mode, long long offset) override;
    void jump_to(std::size_t pos) override;
    void flush() override;
    std::size_t get_size() override;
    ~readonly_mem_stream() override;

    template<typename T = void>
    inline const T* get_data_ptr() const noexcept
    {
        return reinterpret_cast<const T*>(m_handle);
    }

    inline blob get_data() const
    {
        return blob(m_dataSize, m_handle);
    }

    inline readonly_mem_stream(const void* data,
        std::size_t dataSize) : stream(),
        m_handle(static_cast<const u8*>(data)),
        m_dataSize(dataSize) {}
};

class mem_stream : public readonly_mem_stream
{
    std::size_t m_dataCap;

    void in_grow(std::size_t reqCap);

public:
    std::size_t write(std::size_t size, const void* buf) override;
    ~mem_stream() override;

    template<typename T = void>
    inline T* get_data_ptr() noexcept
    {
        return reinterpret_cast<T*>(const_cast<u8*>(m_handle));
    }

    HL_API void* release() noexcept;

    HL_API void close();
    
    HL_API void reopen() noexcept;
    
    HL_API void reopen(std::size_t initialBufCapacity);

    HL_API void reopen(const void* initialData,
        std::size_t initialDataSize);

    inline mem_stream() : readonly_mem_stream(nullptr, 0),
        m_dataCap(0) {}

    inline mem_stream(std::size_t initialBufCapacity) :
        readonly_mem_stream(new u8[initialBufCapacity], 0),
        m_dataCap(initialBufCapacity) {}

    HL_API mem_stream(const void* initialData,
        std::size_t initialDataSize);
};
} // hl
#endif
