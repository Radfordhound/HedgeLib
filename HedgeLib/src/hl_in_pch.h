#ifndef HL_IN_PCH_H_INCLUDED
#define HL_IN_PCH_H_INCLUDED

// Intrinsics
#ifdef HL_DISABLE_INTRINSICS

// Disable intrinsics in DirectXMath.
#define _XM_NO_INTRINSICS_

#else

// SSE2 Intrinsics
#if defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP == 2)
#include <emmintrin.h>
#define HL_IN_HAS_SSE2
#endif

#endif

// Standard library
#include <algorithm>
#include <utility>

#endif
