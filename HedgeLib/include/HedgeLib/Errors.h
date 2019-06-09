#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Error codes
enum HL_RESULT
{
    HL_SUCCESS,
    HL_ERROR_UNKNOWN,
    HL_ERROR_OUT_OF_MEMORY,
    // TODO: Add more results
};

#define HL_FAILED(result) (result != HL_SUCCESS)
#define HL_OK(result) (result == HL_SUCCESS)

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

#ifdef __cplusplus
}
#endif
