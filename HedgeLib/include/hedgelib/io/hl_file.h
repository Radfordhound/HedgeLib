#ifndef HL_FILE_H_INCLUDED
#define HL_FILE_H_INCLUDED
#include "hl_stream.h"

namespace hl
{
class blob;

namespace file
{
enum class mode
{
    /* Masks */
    flag_mask = 0xf,
    mode_mask = 0xf0,

    /* Flags */
    flag_none = 0,
    flag_shared = 1,
    flag_update = 2,

    /* Binary modes */
    read = 16,
    write = 32,
    read_write = (read | write)

    // TODO: Text modes
};

HL_ENUM_CLASS_DEF_BITWISE_OPS(mode)

HL_API std::unique_ptr<u8[]> load(const nchar* filePath,
    std::size_t* dataSize = nullptr);

inline std::unique_ptr<u8[]> load(const nstring& filePath,
    std::size_t* dataSize = nullptr)
{
    return load(filePath.c_str(), dataSize);
}

HL_API void save(const void* data, std::size_t dataSize, const nchar* filePath);

inline void save(const void* data, std::size_t dataSize, const nstring& filePath)
{
    return save(data, dataSize, filePath.c_str());
}

HL_API void save(const blob& fileData, const nchar* filePath);

inline void save(const blob& fileData, const nstring& filePath)
{
    save(fileData, filePath.c_str());
}
} // file

class file_stream : public stream
{
    std::uintmax_t m_handle = 0;

    HL_API void in_open(const nchar* filePath, file::mode mode);

public:
    std::size_t read(std::size_t size, void* buf) override;
    std::size_t write(std::size_t size, const void* buf) override;
    void seek(seek_mode mode, long long offset) override;
    void jump_to(std::size_t pos) override;
    void flush() override;
    std::size_t get_size() override;
    ~file_stream() override;

    HL_API void close();

    HL_API void reopen(const nchar* filePath, file::mode mode);

    inline void reopen(const nstring& filePath, file::mode mode)
    {
        reopen(filePath.c_str(), mode);
    }

    inline file_stream(const nchar* filePath, file::mode mode) : stream()
    {
        in_open(filePath, mode);
    }

    inline file_stream(const nstring& filePath, file::mode mode) : stream()
    {
        in_open(filePath.c_str(), mode);
    }
};
} // hl
#endif
