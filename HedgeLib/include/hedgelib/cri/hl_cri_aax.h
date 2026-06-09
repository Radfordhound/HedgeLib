#ifndef HL_CRI_AAX_H_INCLUDED
#define HL_CRI_AAX_H_INCLUDED

#include <rad/rad_stack_or_heap_array.h>
#include "../hl_internal.h"

namespace rad
{
class stream;
}

namespace hl::cri::audio
{
struct raw_aax_entry
{
    unsigned long long      dataPosition;
    u32                     dataSize;
    bool                    doesLoop;
};

// TODO: Accept a rad::allocator for possible heap allocations
HL_API rad::stack_or_heap_array<raw_aax_entry, 2> read_aax_entries(
    rad::stream& stream
);
}

#endif
