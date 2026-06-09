#include "hedgelib/io/hl_endian_writers.h"
#include <rad/rad_endian.h>
#include <rad/rad_stream.h>

namespace hl::io
{
template<typename T>
static void write_big_endian_value_(rad::stream& stream, T val)
{
    stream.write_as(rad::endian::big_to_native(val));
}

void big_endian_writer::write_u16(u16 val)
{
    write_big_endian_value_(*stream_, val);
}

void big_endian_writer::write_s16(s16 val)
{
    write_big_endian_value_(*stream_, val);
}

void big_endian_writer::write_u32(u32 val)
{
    write_big_endian_value_(*stream_, val);
}

void big_endian_writer::write_s32(s32 val)
{
    write_big_endian_value_(*stream_, val);
}

void big_endian_writer::write_u64(u64 val)
{
    write_big_endian_value_(*stream_, val);
}

void big_endian_writer::write_s64(s64 val)
{
    write_big_endian_value_(*stream_, val);
}

void big_endian_writer::write_f32(float val)
{
    write_big_endian_value_(*stream_, val);
}

void big_endian_writer::write_f64(double val)
{
    write_big_endian_value_(*stream_, val);
}

template<typename T>
static void write_little_endian_value_(rad::stream& stream, T val)
{
    stream.write_as(rad::endian::little_to_native(val));
}

void little_endian_writer::write_u16(u16 val)
{
    write_little_endian_value_(*stream_, val);
}

void little_endian_writer::write_s16(s16 val)
{
    write_little_endian_value_(*stream_, val);
}

void little_endian_writer::write_u32(u32 val)
{
    write_little_endian_value_(*stream_, val);
}

void little_endian_writer::write_s32(s32 val)
{
    write_little_endian_value_(*stream_, val);
}

void little_endian_writer::write_u64(u64 val)
{
    write_little_endian_value_(*stream_, val);
}

void little_endian_writer::write_s64(s64 val)
{
    write_little_endian_value_(*stream_, val);
}

void little_endian_writer::write_f32(float val)
{
    write_little_endian_value_(*stream_, val);
}

void little_endian_writer::write_f64(double val)
{
    write_little_endian_value_(*stream_, val);
}
}
