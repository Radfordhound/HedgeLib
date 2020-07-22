#ifndef HL_IN_ASSERT_H_INCLUDED
#define HL_IN_ASSERT_H_INCLUDED
#if !defined(HL_DISABLE_ASSERTS) && !defined(NDEBUG)
/* This is a debug build; enable asserts. */
#include <assert.h>

#define HL_ASSERT(cond)                  assert(cond)
#define HL_ASSERT_RANGE(index, count)    HL_ASSERT((count) > (index))
#else
/* This is a release build; disable all asserts. */
#define HL_ASSERT(cond)
#define HL_ASSERT_RANGE(index, count)
#endif
#endif
