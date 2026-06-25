#ifndef HL_HH_PACKED_FILE_INFO_H_INCLUDED
#define HL_HH_PACKED_FILE_INFO_H_INCLUDED

#include <rad/rad_vector.h>
#include <rad/rad_string.h>

#include "../hl_internal.h"
#include "hl_hh_mirage.h"

namespace hl::hh_new::mirage
{
struct packed_file_entry
{
    rad::string name;
    u32 dataPos;
    u32 dataSize;

    packed_file_entry(rad::string name, u32 dataPos, u32 dataSize) noexcept
        : name(std::move(name))
        , dataPos(dataPos)
        , dataSize(dataSize)
    {
    }
};

struct packed_file_info
{
    rad::vector<packed_file_entry> entries;

    enum data_version : u32
    {
        /// @brief The earliest known version.
        /// @details This version was introduced in Sonic Unleashed.
        V0 = 0,
    };

    enum class serialize_mode : unsigned char
    {
        /// @brief The usual serialize mode.
        /// @details First seen in Sonic Unleashed (Xbox 360).
        normal_2008,

        /// @brief A newer serialize mode used in Sonic Generations (2024).
        /// @details This mode writes the data in a different order than
        /// `normal_2008`, but is otherwise identical to it.
        /// First seen in Sonic Generations (2024).
        normal_2024,
    };

    inline void clear() noexcept
    {
        entries.clear();
    }

    HL_API file_info read_inner(
        rad::stream& stream,
        off_read_mode offsetReadMode = off_read_mode::auto_detect,
        rad::allocator& tmpAllocator = rad::default_allocator
    );

    HL_API file_info read(
        rad::stream& stream,
        off_read_mode offsetReadMode = off_read_mode::auto_detect,
        rad::allocator& tmpAllocator = rad::default_allocator
    );

    HL_API void write(
        rad::stream& stream,
        file_info fileInfo = { container_type::simple, off_type::u32, V0 },
        serialize_mode serializeMode = serialize_mode::normal_2008,
        rad::allocator& tmpAllocator = rad::default_allocator
    ) const;

    explicit packed_file_info(
        rad::allocator& allocator = rad::default_allocator) noexcept
        : entries(allocator)
    {
    }

    explicit packed_file_info(
        const rad::vector<packed_file_entry>& entries)
        : entries(entries)
    {
    }

    explicit packed_file_info(
        rad::vector<packed_file_entry>&& entries) noexcept
        : entries(std::move(entries))
    {
    }

    explicit packed_file_info(
        rad::stream& stream,
        off_read_mode offsetReadMode = off_read_mode::auto_detect,
        rad::allocator& tmpAllocator = rad::default_allocator,
        rad::allocator& allocator = rad::default_allocator)
        : entries(allocator)
    {
        read(stream, offsetReadMode, tmpAllocator);
    }
};
}

#endif
