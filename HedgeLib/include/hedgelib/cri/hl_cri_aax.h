#ifndef HL_CRI_AAX_H_INCLUDED
#define HL_CRI_AAX_H_INCLUDED

#include "../hl_internal.h"
#include <rad/rad_stack_or_heap_array.h>

namespace rad
{
class stream;
}

namespace hl::cri_new::audio
{
struct aax_entry
{
    unsigned long long      dataPosition;
    unsigned long           dataSize;
    bool                    doesLoop;
};

HL_API rad::stack_or_heap_array<aax_entry, 2> read_aax_entries(
    rad::stream& stream
);
}

#endif
