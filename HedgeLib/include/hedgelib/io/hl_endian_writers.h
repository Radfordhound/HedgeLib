#ifndef HL_ENDIAN_WRITERS_H_INCLUDED
#define HL_ENDIAN_WRITERS_H_INCLUDED

#include "hl_writer_base.h"

namespace hl::io
{
class big_endian_writer
    : public writer_base
{
public:
    HL_API void write_u16(u16 val);

    HL_API void write_s16(s16 val);

    HL_API void write_u32(u32 val);

    HL_API void write_s32(s32 val);

    HL_API void write_u64(u64 val);

    HL_API void write_s64(s64 val);

    HL_API void write_f32(float val);

    HL_API void write_f64(double val);

    inline big_endian_writer(rad::stream& stream) noexcept
        : writer_base(stream)
    {
    }
};

class little_endian_writer
    : public writer_base
{
public:
    HL_API void write_u16(u16 val);

    HL_API void write_s16(s16 val);

    HL_API void write_u32(u32 val);

    HL_API void write_s32(s32 val);

    HL_API void write_u64(u64 val);

    HL_API void write_s64(s64 val);

    HL_API void write_f32(float val);

    HL_API void write_f64(double val);

    inline little_endian_writer(rad::stream& stream) noexcept
        : writer_base(stream)
    {
    }
};
}

#endif
