#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include "hl_in_posix.h"

HlResult hlINPosixGetResult(int err)
{
    switch (err)
    {
        case ENOMEM: return HL_ERROR_OUT_OF_MEMORY;
        case ERANGE: return HL_ERROR_OUT_OF_RANGE;
        case EEXIST: return HL_ERROR_ALREADY_EXISTS;
        /* TODO: Add more. */
        default: return HL_ERROR_UNKNOWN;
    }
}

#endif
