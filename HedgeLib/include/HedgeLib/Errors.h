#pragma once
#include "String.h"

#ifdef __cplusplus
extern "C" {
#endif

// Static asserts
#if defined(__cplusplus) && (__cplusplus >= 201103L || defined(__cpp_static_assert))
#define HL_STATIC_ASSERT(exp, msg) static_assert(exp, msg)
#elif __STDC_VERSION__ >= 201112L
#include <assert.h>
#define HL_STATIC_ASSERT(exp, msg) static_assert(exp, msg)
#else
// TODO: Maybe we should do one of the many static_assert hacks instead?
#define HL_STATIC_ASSERT(exp, msg)
#endif

#define HL_STATIC_ASSERT_SIZE(type, size) HL_STATIC_ASSERT(sizeof(type) == size, \
    "sizeof(" #type ") != expected size (" #size ").")

HL_API const char* hl_GetResultString(HL_RESULT result);
HL_API const hl_NativeChar* hl_GetResultStringNative(HL_RESULT result);

#ifdef __cplusplus
}
#endif
