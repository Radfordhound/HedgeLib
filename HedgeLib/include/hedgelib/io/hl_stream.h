#ifndef HL_STREAM_H_INCLUDED
#define HL_STREAM_H_INCLUDED
#include "../hl_tables.h"

namespace hl
{
enum class seek_mode
{
    /** @brief Seek starting from the beginning of the stream. */
    beg,
    /** @brief Seek starting from the current position in the stream. */
    cur,
    /** @brief Seek starting from the end of the stream. */
    end
};

/**
    @brief Custom stream class. Designed for reading/writing binary data
    much more easily than with std::fstream. Not designed for reading/writing
    text.
*/
class stream
{
protected:
    std::size_t m_curPos = 0;

    inline stream() = default;

public:
    virtual std::size_t read(std::size_t size, void* buf) = 0;

    virtual std::size_t write(std::size_t size, const void* buf) = 0;

    virtual void seek(seek_mode mode, long long offset) = 0;

    virtual void jump_to(std::size_t pos) = 0;

    virtual void flush() = 0;

    virtual std::size_t get_size() = 0;

    virtual ~stream() = 0;

    inline void jump_ahead(long long amount)
    {
        seek(seek_mode::cur, amount);
    }

    inline void jump_behind(long long amount)
    {
        seek(seek_mode::cur, -amount);
    }

    inline std::size_t tell() const noexcept
    {
        return m_curPos;
    }

    inline std::size_t tell(long off) const noexcept
    {
        return (m_curPos + off);
    }

    HL_API void read_all(std::size_t size, void* buf);

    HL_API void write_all(std::size_t size, const void* buf);

    template<typename T>
    inline void read_obj(T& obj)
    {
        read_all(sizeof(T), &obj);
    }

    template<typename T>
    void read_obj(T& obj, std::size_t alignment)
    {
        align((alignment) ? alignment : alignof(T));
        read_obj(obj);
    }

    template<typename T>
    inline void write_obj(const T& obj)
    {
        write_all(sizeof(T), &obj);
    }

    template<typename T>
    inline void write_obj(const T& obj, std::size_t alignment)
    {
        pad((alignment) ? alignment : alignof(T));
        write_obj(obj);
    }

    template<typename T>
    inline void read_arr(std::size_t count, T* arr)
    {
        read_all(sizeof(T) * count, arr);
    }

    template<typename T, std::size_t count>
    inline void read_arr(T(&arr)[count])
    {
        read_arr(count, arr);
    }

    template<typename T>
    void read_arr(std::size_t count, T* arr, std::size_t alignment)
    {
        align((alignment) ? alignment : alignof(T));
        read_arr(count, arr);
    }

    template<typename T, std::size_t count>
    inline void read_arr(T(&arr)[count], std::size_t alignment)
    {
        read_arr(count, arr, alignment);
    }

    template<typename T>
    inline void write_arr(std::size_t count, const T* arr)
    {
        write_all(sizeof(T) * count, arr);
    }

    template<typename T, std::size_t count>
    inline void write_arr(const T(&arr)[count])
    {
        write_arr(count, arr);
    }

    template<typename T>
    void write_arr(std::size_t count, const T* arr, std::size_t alignment)
    {
        pad((alignment) ? alignment : alignof(T));
        write_arr(count, arr);
    }

    template<typename T, std::size_t count>
    inline void write_arr(const T(&arr)[count], std::size_t alignment)
    {
        write_arr(count, arr, alignment);
    }

    HL_API void write_nulls(std::size_t amount);

    HL_API void write_off32(std::size_t basePos, std::size_t offVal,
        bool doSwap, off_table& offTable);

    HL_API void write_off64(std::size_t basePos, std::size_t offVal,
        bool doSwap, off_table& offTable);

    HL_API void fix_off32(std::size_t basePos, std::size_t offPos,
        bool doSwap, off_table& offTable);

    HL_API void fix_off64(std::size_t basePos, std::size_t offPos,
        bool doSwap, off_table& offTable);

    HL_API bool read_str(std::size_t bufSize, char* buf);

    HL_API std::string read_str();

    std::size_t write_str(const char* str)
    {
        const std::size_t size = text::size(str);
        write_arr(size, str);
        return size;
    }

    std::size_t write_str(const std::string& str)
    {
        const std::size_t size = (str.length() + 1);
        write_arr(size, str.c_str());
        return size;
    }

    HL_API void align(std::size_t stride);

    HL_API void pad(std::size_t stride);
};

namespace internal
{
class in_writer_base
{
protected:
    hl::stream* m_stream;

    inline in_writer_base(hl::stream& stream) noexcept :
        m_stream(&stream) {}

public:
    inline const hl::stream& stream() const noexcept
    {
        return *m_stream;
    }

    inline hl::stream& stream() noexcept
    {
        return *m_stream;
    }

    inline std::size_t write(std::size_t size, const void* buf)
    {
        return m_stream->write(size, buf);
    }

    inline void flush()
    {
        m_stream->flush();
    }

    inline std::size_t get_size()
    {
        return m_stream->get_size();
    }

    inline std::size_t tell() const noexcept
    {
        return m_stream->tell();
    }

    inline std::size_t tell(long off) const noexcept
    {
        return m_stream->tell(off);
    }

    inline void write_all(std::size_t size, const void* buf)
    {
        m_stream->write_all(size, buf);
    }

    template<typename T>
    inline void write_obj(const T& obj)
    {
        m_stream->write_obj(obj);
    }

    template<typename T>
    inline void write_obj(const T& obj, std::size_t alignment)
    {
        m_stream->write_obj(obj, alignment);
    }

    template<typename T>
    inline void write_arr(std::size_t count, const T* arr)
    {
        m_stream->write_arr(count, arr);
    }

    template<typename T, std::size_t count>
    inline void write_arr(const T(&arr)[count])
    {
        m_stream->write_arr(count, arr);
    }

    template<typename T>
    inline void write_arr(std::size_t count, const T* arr, std::size_t alignment)
    {
        m_stream->write_arr(count, arr, alignment);
    }

    template<typename T, std::size_t count>
    inline void write_arr(const T(&arr)[count], std::size_t alignment)
    {
        m_stream->write_arr(count, arr, alignment);
    }

    inline void write_nulls(std::size_t amount)
    {
        m_stream->write_nulls(amount);
    }

    inline std::size_t write_str(const char* str)
    {
        return m_stream->write_str(str);
    }

    inline std::size_t write_str(const std::string& str)
    {
        return m_stream->write_str(str);
    }

    inline void pad(std::size_t stride)
    {
        m_stream->pad(stride);
    }
};
} // internal
} // hl
#endif
