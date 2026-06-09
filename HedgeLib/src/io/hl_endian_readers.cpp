#include "hedgelib/io/hl_endian_readers.h"
#include <rad/rad_endian.h>
#include <rad/rad_stream.h>

namespace hl::io
{
template<typename T>
static T read_big_endian_value_(rad::stream& stream)
{
    T val;
    stream.read_as(val);
    return rad::endian::big_to_native(val);
}

u16 big_endian_reader::read_u16()
{
    return read_big_endian_value_<u16>(*stream_);
}

s16 big_endian_reader::read_s16()
{
    return read_big_endian_value_<s16>(*stream_);
}

u32 big_endian_reader::read_u32()
{
    return read_big_endian_value_<u32>(*stream_);
}

s32 big_endian_reader::read_s32()
{
    return read_big_endian_value_<s32>(*stream_);
}

u64 big_endian_reader::read_u64()
{
    return read_big_endian_value_<u64>(*stream_);
}

s64 big_endian_reader::read_s64()
{
    return read_big_endian_value_<s64>(*stream_);
}

float big_endian_reader::read_f32()
{
    return read_big_endian_value_<float>(*stream_);
}

double big_endian_reader::read_f64()
{
    return read_big_endian_value_<double>(*stream_);
}

template<typename T>
static T read_little_endian_value_(rad::stream& stream)
{
    T val;
    stream.read_as(val);
    return rad::endian::little_to_native(val);
}

u16 little_endian_reader::read_u16()
{
    return read_little_endian_value_<u16>(*stream_);
}

s16 little_endian_reader::read_s16()
{
    return read_little_endian_value_<s16>(*stream_);
}

u32 little_endian_reader::read_u32()
{
    return read_little_endian_value_<u32>(*stream_);
}

s32 little_endian_reader::read_s32()
{
    return read_little_endian_value_<s32>(*stream_);
}

u64 little_endian_reader::read_u64()
{
    return read_little_endian_value_<u64>(*stream_);
}

s64 little_endian_reader::read_s64()
{
    return read_little_endian_value_<s64>(*stream_);
}

float little_endian_reader::read_f32()
{
    return read_little_endian_value_<float>(*stream_);
}

double little_endian_reader::read_f64()
{
    return read_little_endian_value_<double>(*stream_);
}
}
