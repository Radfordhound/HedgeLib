/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#ifndef cglm_common_h
#define cglm_common_h

#ifndef _USE_MATH_DEFINES
#  define _USE_MATH_DEFINES       /* for windows */
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS /* for windows */
#endif

#include <stddef.h>
#include <math.h>
#include <float.h>

#if defined(_MSC_VER)
#  ifdef CGLM_STATIC
#    define CGLM_EXPORT
#  elif defined(CGLM_EXPORTS)
#    define CGLM_EXPORT __declspec(dllexport)
#  else
#    define CGLM_EXPORT __declspec(dllimport)
#  endif
#  ifndef __STDC__ /* defined if MSVC extensions are disabled */
#    define CGLM_INLINE __forceinline
#  endif
#else
#  define CGLM_EXPORT __attribute__((visibility("default")))
#  ifdef __GNUC__ /* should be defined on all compilers that support GCC extensions */
#    define CGLM_INLINE static inline __attribute((always_inline))
#  endif
#endif

#ifndef CGLM_INLINE
#  ifdef __cplusplus
#    define CGLM_INLINE inline
#  elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#    define CGLM_INLINE static inline
#  endif
#endif

#define GLM_SHUFFLE4(z, y, x, w) (((z) << 6) | ((y) << 4) | ((x) << 2) | (w))
#define GLM_SHUFFLE3(z, y, x)    (((z) << 4) | ((y) << 2) | (x))

#include "types.h"
#include "simd/intrin.h"

#ifndef CGLM_USE_DEFAULT_EPSILON
#  ifndef GLM_FLT_EPSILON
#    define GLM_FLT_EPSILON 1e-6
#  endif
#else
#  define GLM_FLT_EPSILON FLT_EPSILON
#endif

#endif /* cglm_common_h */
