#ifndef HL_BLOB_H_INCLUDED
#define HL_BLOB_H_INCLUDED
#include "hl_text.h"

namespace hl
{
class blob
{
    friend struct in_blob_size_setter;

protected:
    /** @brief Pointer to the data this blob contains. */
    u8* m_data;
    /** @brief Size of the data this blob contains, in bytes. */
    std::size_t m_size;

    blob() noexcept : m_data(nullptr), m_size(0U) {}

public:
    template<typename T = void>
    inline const T* data() const noexcept
    {
        return reinterpret_cast<const T*>(m_data);
    }

    template<typename T = void>
    inline T* data() noexcept
    {
        return reinterpret_cast<T*>(m_data);
    }

    inline std::size_t size() const noexcept
    {
        return m_size;
    }

    inline operator const void*() const noexcept
    {
        return m_data;
    }

    inline operator void*() noexcept
    {
        return m_data;
    }

    HL_API blob& operator=(const blob& other);
    HL_API blob& operator=(blob&& other) noexcept;

    HL_API blob(std::size_t size, const void* initialData = nullptr);
    HL_API blob(const nchar* filePath);
    inline blob(const nstring& filePath) : blob(filePath.c_str()) {}

    HL_API blob(const blob& other);
    HL_API blob(blob&& other) noexcept;
    HL_API ~blob();
};

struct nullable_blob : public blob
{
    inline bool empty() const noexcept
    {
        return (m_size == 0U);
    }

    explicit operator bool() const noexcept
    {
        return (m_size != 0U);
    }

    nullable_blob() noexcept : blob() {}

    nullable_blob(std::size_t size, const void* initialData = nullptr) :
        blob(size, initialData) {}

    nullable_blob(const nchar* filePath) : blob(filePath) {}

    nullable_blob(const nstring& filePath) :
        nullable_blob(filePath.c_str()) {}

};
} // hl
#endif
