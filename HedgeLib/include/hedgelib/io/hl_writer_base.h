#ifndef HL_WRITER_BASE_H_INCLUDED
#define HL_WRITER_BASE_H_INCLUDED

#include <rad/rad_stream.h>

#include "../hl_internal.h"

namespace hl::io
{
class writer_base
{
protected:
    rad::stream*    stream_;

public:
    inline rad::stream& stream() const noexcept
    {
        return *stream_;
    }

    HL_API void write_u8(u8 val);

    HL_API void write_s8(s8 val);

    inline writer_base(rad::stream& stream) noexcept
        : stream_(&stream)
    {
    }
};
}

#endif
