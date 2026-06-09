#ifndef HL_ENDIAN_READERS_H_INCLUDED
#define HL_ENDIAN_READERS_H_INCLUDED

#include "hl_reader_base.h"

namespace hl::io
{
class big_endian_reader
    : public reader_base
{
public:
    HL_API u16 read_u16();

    HL_API s16 read_s16();

    HL_API u32 read_u32();

    HL_API s32 read_s32();

    HL_API u64 read_u64();

    HL_API s64 read_s64();

    HL_API float read_f32();

    HL_API double read_f64();

    inline big_endian_reader(rad::stream& stream) noexcept
        : reader_base(stream)
    {
    }
};

class little_endian_reader
    : public reader_base
{
public:
    HL_API u16 read_u16();

    HL_API s16 read_s16();

    HL_API u32 read_u32();

    HL_API s32 read_s32();

    HL_API u64 read_u64();

    HL_API s64 read_s64();

    HL_API float read_f32();

    HL_API double read_f64();

    inline little_endian_reader(rad::stream& stream) noexcept
        : reader_base(stream)
    {
    }
};
}

#endif
