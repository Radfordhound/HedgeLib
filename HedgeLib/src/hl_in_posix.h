#ifndef HL_IN_POSIX_H_INCLUDED
#define HL_IN_POSIX_H_INCLUDED
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include "hedgelib/hl_internal.h"
#include <errno.h>

namespace hl
{
inline error_type in_posix_get_error(int err) noexcept
{
    switch (err)
    {
    case ENOMEM: return error_type::out_of_memory;
    case ERANGE: return error_type::out_of_range;
    case EEXIST: return error_type::already_exists;
    case ETXTBSY: return error_type::sharing_violation;
        
    // TODO: Add more.

    default: return error_type::unknown;
    }
}

inline error_type in_posix_get_last_error()
{
    return in_posix_get_error(errno);
}

#define HL_IN_POSIX_ERROR()\
    HL_ERROR(in_posix_get_last_error())
} // hl
#endif
#endif
