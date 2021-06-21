#ifndef HL_IN_BLOB_H_INCLUDED
#define HL_IN_BLOB_H_INCLUDED
#include "hedgelib/hl_blob.h"

namespace hl
{
struct in_blob_size_setter
{
    inline static void set_size(blob& b, std::size_t size) noexcept
    {
        b.m_size = size;
    }
};
} // hl
#endif
