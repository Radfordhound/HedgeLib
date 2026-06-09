#include "hedgelib/io/hl_writer_base.h"
#include <rad/rad_stream.h>

namespace hl::io
{
void writer_base::write_u8(u8 val)
{
    stream_->write_as(val);
}

void writer_base::write_s8(s8 val)
{
    stream_->write_as(val);
}
}
