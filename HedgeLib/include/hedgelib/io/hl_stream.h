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

    HL_API void read_all(std::size_t size, void* buf);
    HL_API void write_all(std::size_t size, const void* buf);

    template<typename T>
    void read_obj(T& obj)
    {
        read_all(sizeof(T), &obj);
    }

    template<typename T>
    void write_obj(const T& obj)
    {
        write_all(sizeof(T), &obj);
    }

    template<typename T>
    void read_arr(std::size_t count, T* arr)
    {
        read_all(sizeof(T) * count, arr);
    }

    template<typename T>
    void write_arr(std::size_t count, const T* arr)
    {
        write_all(sizeof(T) * count, arr);
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
} // hl
#endif
