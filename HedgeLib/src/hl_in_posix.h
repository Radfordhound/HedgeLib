#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#ifndef HL_IN_POSIX_H_INCLUDED
#define HL_IN_POSIX_H_INCLUDED
#include "hedgelib/hl_internal.h"
#include <errno.h>

HlResult hlINPosixGetResult(int err);

#define hlINPosixGetResultErrno() hlINPosixGetResult(errno);

#endif
#endif
