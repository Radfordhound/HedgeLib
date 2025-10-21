#ifndef HL_ENDIAN_WRITERS_H_INCLUDED
#define HL_ENDIAN_WRITERS_H_INCLUDED

#include "../../hl_internal.h"
#include <rad/rad_stream.h>

namespace hl
{
class big_endian_writer
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

    HL_API void write_u16(u16 val);

    HL_API void write_s16(s16 val);

    HL_API void write_u32(u32 val);

    HL_API void write_s32(s32 val);

    HL_API void write_u64(u64 val);

    HL_API void write_s64(s64 val);

    HL_API void write_f32(float val);

    HL_API void write_f64(double val);

    inline big_endian_writer(rad::stream& stream) noexcept
        : stream_(&stream)
    {
    }
};

class little_endian_writer
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

    HL_API void write_u16(u16 val);

    HL_API void write_s16(s16 val);

    HL_API void write_u32(u32 val);

    HL_API void write_s32(s32 val);

    HL_API void write_u64(u64 val);

    HL_API void write_s64(s64 val);

    HL_API void write_f32(float val);

    HL_API void write_f64(double val);

    inline little_endian_writer(rad::stream& stream) noexcept
        : stream_(&stream)
    {
    }
};
}

#endif
