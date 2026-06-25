#ifndef HL_HH_MIRAGE_IMPL_H_INCLUDED
#define HL_HH_MIRAGE_IMPL_H_INCLUDED

#include <stdexcept>

#include <rad/rad_vector.h>
#include <rad/rad_stream.h>
#include <rad/rad_memory_stream.h>
#include <rad/rad_endian.h>

#include "hedgelib/hl_internal.h"
#include "hedgelib/hh_new/hl_hh_mirage.h"

namespace hl::hh_new::mirage
{
template<class T>
file_info read_mirage_data_(
    T& obj,
    rad::stream& stream,
    off_read_mode offsetReadMode,
    rad::allocator& tmpAllocator)
{
    if (stream.capabilities().can_nocost_read() &&
        stream.capabilities().can_seek())
    {
        // Directly read the data from the stream.
        return obj.read_inner(stream, offsetReadMode, tmpAllocator);
    }
    else
    {
        // Read file size.
        u32 rawMirageFileSize;
        stream.read_as(rawMirageFileSize);

        auto mirageFileSize = rad::endian::native_to_big(rawMirageFileSize);
        if (mirageFileSize & sample_chunk::marker)
        {
            mirageFileSize &= ~sample_chunk::marker;
        }

        if (mirageFileSize < 4)
        {
            throw std::runtime_error("Mirage file size was too small");
        }

        // Read all of the mirage container data from the stream into
        // a buffer, then parse all of the data in the buffer.
        rad::vector<unsigned char> mirageData(
            rad::no_value_init,
            tmpAllocator,
            mirageFileSize
        );

        *reinterpret_cast<u32*>(mirageData.data()) = rawMirageFileSize;
        stream.read(mirageData.data() + 4, mirageFileSize - 4);

        rad::readonly_memory_stream mirageDataStream(
            mirageData.data(),
            mirageFileSize
        );

        return obj.read_inner(mirageDataStream, offsetReadMode, tmpAllocator);
    }
}
}

#endif
