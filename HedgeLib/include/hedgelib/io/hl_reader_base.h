#ifndef HL_READER_BASE_H_INCLUDED
#define HL_READER_BASE_H_INCLUDED

#include <rad/rad_vector.h>
#include <rad/rad_stream.h>

#include "../hl_internal.h"

namespace hl::io
{
class reader_base
{
protected:
    rad::stream*    stream_;

public:
    inline rad::stream& stream() const noexcept
    {
        return *stream_;
    }

    HL_API u8 read_u8();

    HL_API s8 read_s8();

    HL_API rad::vector<unsigned char> read_bytes(
        std::size_t size,
        rad::allocator& allocator = rad::default_allocator
    );

    inline reader_base(rad::stream& stream) noexcept
        : stream_(&stream)
    {
    }
};
}

#endif
