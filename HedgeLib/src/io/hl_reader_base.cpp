#include "hedgelib/io/hl_reader_base.h"
#include <rad/rad_stream.h>

namespace hl::io
{
u8 reader_base::read_u8()
{
    u8 val;
    stream_->read_as(val);
    return val;
}

s8 reader_base::read_s8()
{
    s8 val;
    stream_->read_as(val);
    return val;
}

rad::vector<unsigned char> reader_base::read_bytes(
    std::size_t size,
    rad::allocator& allocator)
{
    rad::vector<unsigned char> data(rad::no_value_init, allocator, size);
    stream().read(data.data(), size);
    return data;
}
}
