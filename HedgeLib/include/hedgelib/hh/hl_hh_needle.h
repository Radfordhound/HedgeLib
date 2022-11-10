#include "../hl_text.h"

namespace hl
{
namespace hh
{
namespace needle
{
constexpr u64 signature_archive_v1 = make_sig("NEDARCV1");
constexpr u64 signature_lodinfo_v1 = make_sig("NEDLDIV1");
constexpr u64 signature_model_v5 = make_sig("NEDMDLV5");

struct raw_archive_entry
{
    u8 signature[8]; // TODO: Replace with new hl::sig64 type

    template<bool swapOffsets = true>
    inline void endian_swap() noexcept {}

    inline const char* extension() const noexcept
    {
        return reinterpret_cast<const char*>(this + 1);
    }

    inline char* extension() noexcept
    {
        return reinterpret_cast<char*>(this + 1);
    }

    HL_API const u32& size() const;

    HL_API u32& size();

    template<typename T = void>
    inline const T* data() const
    {
        return reinterpret_cast<const T*>(&size() + 1);
    }

    template<typename T = void>
    inline T* data()
    {
        return reinterpret_cast<T*>(&size() + 1);
    }

    HL_API void extract(const nchar* path) const;
};

HL_STATIC_ASSERT_SIZE(raw_archive_entry, 8);

class raw_archive_entry_const_iterator
{
    const raw_archive_entry* m_curRawArcEntry = nullptr;

public:
    using this_t                = raw_archive_entry_const_iterator;
    using iterator_category     = std::forward_iterator_tag;
    using difference_type       = std::ptrdiff_t;
    using value_type            = const raw_archive_entry;
    using pointer               = value_type*;
    using reference             = value_type&;

    inline reference operator*() const
    {
        return *m_curRawArcEntry;
    }

    inline pointer operator->() const
    {
        return m_curRawArcEntry;
    }

    HL_API this_t& operator++();

    inline this_t operator++(int) noexcept
    {
        const auto tmp = *this;
        ++(*this);
        return tmp;
    }

    inline friend bool operator==(const this_t& a, const this_t& b) noexcept
    {
        return a.m_curRawArcEntry == b.m_curRawArcEntry;
    }

    inline friend bool operator!=(const this_t& a, const this_t& b) noexcept
    {
        return a.m_curRawArcEntry != b.m_curRawArcEntry;
    }

    inline raw_archive_entry_const_iterator() noexcept = default;

    inline raw_archive_entry_const_iterator(pointer ptr) noexcept :
        m_curRawArcEntry(ptr) {}
};

class raw_archive_entry_iterator
{
    raw_archive_entry* m_curRawArcEntry = nullptr;

public:
    using this_t                = raw_archive_entry_iterator;
    using iterator_category     = std::forward_iterator_tag;
    using difference_type       = std::ptrdiff_t;
    using value_type            = raw_archive_entry;
    using pointer               = value_type*;
    using reference             = value_type&;

    inline reference operator*() const
    {
        return *m_curRawArcEntry;
    }

    inline pointer operator->() const
    {
        return m_curRawArcEntry;
    }

    HL_API this_t& operator++();

    inline this_t operator++(int) noexcept
    {
        const auto tmp = *this;
        ++(*this);
        return tmp;
    }

    inline friend bool operator==(const this_t& a, const this_t& b) noexcept
    {
        return a.m_curRawArcEntry == b.m_curRawArcEntry;
    }

    inline friend bool operator!=(const this_t& a, const this_t& b) noexcept
    {
        return a.m_curRawArcEntry != b.m_curRawArcEntry;
    }

    inline raw_archive_entry_iterator() noexcept = default;

    inline raw_archive_entry_iterator(pointer ptr) noexcept :
        m_curRawArcEntry(ptr) {}
};

struct raw_archive
{
    /** @brief "NEDARCV1" signature. */
    u8 signature[8];
    /** @brief The total size of this Needle archive, **including** the size of this field. */
    u32 size;

    template<bool swapOffsets = true>
    inline void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(size);
    }

    inline const char* extension() const noexcept
    {
        return reinterpret_cast<const char*>(this + 1);
    }

    inline char* extension() noexcept
    {
        return reinterpret_cast<char*>(this + 1);
    }

    HL_API const raw_archive_entry* entries_sec() const;

    HL_API raw_archive_entry* entries_sec();

    inline raw_archive_entry_const_iterator begin() const
    {
        return entries_sec();
    }

    inline raw_archive_entry_iterator begin()
    {
        return entries_sec();
    }

    inline raw_archive_entry_const_iterator end() const
    {
        return ptradd<raw_archive_entry>(&size, size);
    }

    inline raw_archive_entry_iterator end()
    {
        return ptradd<raw_archive_entry>(&size, size);
    }
};

HL_STATIC_ASSERT_SIZE(raw_archive, 12);
} // needle
} // hh
} // hl
