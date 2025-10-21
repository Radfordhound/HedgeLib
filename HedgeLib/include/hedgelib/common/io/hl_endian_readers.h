#ifndef HL_ENDIAN_READERS_H_INCLUDED
#define HL_ENDIAN_READERS_H_INCLUDED

#include "../../hl_internal.h"
#include <rad/rad_stream.h>

namespace hl
{
class big_endian_reader
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

    HL_API u16 read_u16();

    HL_API s16 read_s16();

    HL_API u32 read_u32();

    HL_API s32 read_s32();

    HL_API u64 read_u64();

    HL_API s64 read_s64();

    HL_API float read_f32();

    HL_API double read_f64();

    inline big_endian_reader(rad::stream& stream) noexcept
        : stream_(&stream)
    {
    }
};

class little_endian_reader
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

    HL_API u16 read_u16();

    HL_API s16 read_s16();

    HL_API u32 read_u32();

    HL_API s32 read_s32();

    HL_API u64 read_u64();

    HL_API s64 read_s64();

    HL_API float read_f32();

    HL_API double read_f64();

    inline little_endian_reader(rad::stream& stream) noexcept
        : stream_(&stream)
    {
    }
};
}

#endif
