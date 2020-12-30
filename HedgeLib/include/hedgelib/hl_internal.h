#ifndef HL_INTERNAL_H_INCLUDED
#define HL_INTERNAL_H_INCLUDED
#include <stddef.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Define one macro for Windows */
#if defined(__CYGWIN__) && !defined(_WIN32)
#define _WIN32
#endif

/*
   Shared library macros
   (Adapted from https://gcc.gnu.org/wiki/Visibility)
*/
#ifndef HL_API
#ifdef HL_IS_DLL
/* HedgeLib is a DLL. */
#ifdef _WIN32
/* We're targetting Windows; use __declspec */
#ifdef HL_IS_BUILDING_DLL
/* We're building a DLL; export the given symbol. */
#define HL_API __declspec(dllexport)
#else
/* We're using a pre-built DLL; import the given symbol. */
#define HL_API __declspec(dllimport)
#endif
#elif defined(__GNUC__) && __GNUC__ >= 4
/* We're not targetting Windows and we're using gcc; use __attribute__ */
#define HL_API __attribute__ ((visibility ("default")))
#else
/* We don't know the target platform/compiler; assume it doesn't require any keywords. */
#define HL_API
#endif
#else
/* HedgeLib is a static library; no keyword(s) are needed. */
#define HL_API
#endif
#endif

/* Macro for restrict keyword */
#ifndef HL_RESTRICT
#if !defined(__cplusplus) && (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#define HL_RESTRICT restrict
#elif defined(_MSC_VER) && _MSC_VER >= 1310 && !defined(__STDC__)
/* MSVC-specific __restrict keyword. */
#define HL_RESTRICT __restrict
#elif (defined(__GNUC__) && __GNUC__ >= 3) || defined(__clang__)
/* GCC 3.0+/Clang-specific __restrict__ keyword. */
#define HL_RESTRICT __restrict__
#else
/* We assume no restrict keyword support. Define HL_RESTRICT yourself if wanted. */
#define HL_RESTRICT
#endif
#endif

/* Fixed-width integral types */
#if (defined(__cplusplus) && __cplusplus >= 201103L) ||\
    (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) ||\
    (defined(_MSC_VER) && _MSC_VER >= 1600)

/* Include C99/C++11 stdint.h */
#include <stdint.h>

/* Define typedefs for stdint fixed-width integral types. */
#ifdef INT8_MAX
typedef int8_t HlS8;
#endif
#ifdef INT_LEAST8_MAX
typedef int_least8_t HlLeastS8;
#endif
#ifdef UINT8_MAX
typedef uint8_t HlU8;
#endif
#ifdef UINT_LEAST8_MAX
typedef uint_least8_t HlLeastU8;
#endif
#ifdef INT16_MAX
typedef int16_t HlS16;
#endif
#ifdef INT_LEAST16_MAX
typedef int_least16_t HlLeastS16;
#endif
#ifdef UINT16_MAX
typedef uint16_t HlU16;
#endif
#ifdef UINT_LEAST16_MAX
typedef uint_least16_t HlLeastU16;
#endif
#ifdef INT32_MAX
typedef int32_t HlS32;
#endif
#ifdef INT_LEAST32_MAX
typedef int_least32_t HlLeastS32;
#endif
#ifdef UINT32_MAX
typedef uint32_t HlU32;
#endif
#ifdef UINT_LEAST32_MAX
typedef uint_least32_t HlLeastU32;
#endif
#ifdef INT64_MAX
typedef int64_t HlS64;
#endif
#ifdef INT_LEAST64_MAX
typedef int_least64_t HlLeastS64;
#endif
#ifdef UINT64_MAX
typedef uint64_t HlU64;
#endif
#ifdef UINT_LEAST64_MAX
typedef uint_least64_t HlLeastU64;
#endif

/* Define pointer integral types/macros. */
#ifdef INTPTR_MAX
typedef intptr_t HlSPtr;
#endif
#ifdef UINTPTR_MAX
typedef uintptr_t HlUPtr;
#endif

#define HL_SPTR_MIN INTPTR_MIN
#define HL_SPTR_MAX INTPTR_MAX
#define HL_UPTR_MAX UINTPTR_MAX

/* Define size macros. */
#define HL_SIZE_MAX SIZE_MAX

/* Define max integral types/macros. */
#ifdef INTMAX_MAX
typedef intmax_t HlSMax;
#endif
#ifdef UINTMAX_MAX
typedef uintmax_t HlUMax;
#endif

#define HL_SMAX_MAX INTMAX_MAX
#define HL_UMAX_MAX UINTMAX_MAX
#elif defined(_MSC_VER)
/* Define typedefs for MSVC-specific fixed-width integral types. */
typedef __int8 HlS8;
typedef HlS8 HlLeastS8;
typedef unsigned __int8 HlU8;
typedef HlU8 HlLeastU8;
typedef __int16 HlS16;
typedef HlS16 HlLeastS16;
typedef unsigned __int16 HlU16;
typedef HlU16 HlLeastU16;
typedef __int32 HlS32;
typedef HlS32 HlLeastS32;
typedef unsigned __int32 HlU32;
typedef HlU32 HlLeastU32;
typedef __int64 HlS64;
typedef HlS64 HlLeastS64;
typedef unsigned __int64 HlU64;
typedef HlU64 HlLeastU64;

/* Define pointer integral types/macros. */
#ifdef _WIN64
typedef HlS64 HlSPtr;
typedef HlU64 HlUPtr;
#define HL_SPTR_MIN (-9223372036854775807i64 - 1)
#define HL_SPTR_MAX (9223372036854775807i64)
#define HL_UPTR_MAX 0xffffffffffffffffu
#else
typedef HlS32 HlSPtr;
typedef HlU32 HlUPtr;
#define HL_SPTR_MIN (-2147483647i32 - 1)
#define HL_SPTR_MAX (2147483647i32)
#define HL_UPTR_MAX 0xffffffffu
#endif

/* Define size macros. */
#define HL_SIZE_MAX HL_UPTR_MAX

/* Define max integral types/macros. */
typedef HlSPtr HlSMax;
typedef HlUPtr HlUMax;

#define HL_SMAX_MAX HL_SPTR_MAX
#define HL_UMAX_MAX HL_UPTR_MAX
#else
/* Include hl_in_c89_stdint.h, which defines all the values we need. */
#include "hl_in_c89_stdint.h"
#endif

/* Native character type */
#ifndef HL_WIN32_FORCE_ANSI
#if defined(_WIN32) && (defined(_UNICODE) || defined(UNICODE))
/* Assume we have C95 wchar_t support (we already included stddef.h). */
typedef wchar_t HlNChar;
#define HL_IN_WIN32_UNICODE
#endif
#endif

#ifndef HL_IN_WIN32_UNICODE
typedef char HlNChar;
#endif

/* Unicode character types */
#ifdef __cpp_unicode_characters
/* C++11 unicode character types */
typedef char16_t HlChar16;
typedef char32_t HlChar32;
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L &&\
 !(defined(__APPLE__) && defined(__MACH__)) /* macOS "supports" C11 but doesn't include uchar.h */
/* C11 unicode character types */
#include <uchar.h>
typedef char16_t HlChar16;
typedef char32_t HlChar32;
#else
/* Custom unicode character types */
typedef HlLeastU16 HlChar16;
typedef HlLeastU32 HlChar32;
#endif

/* Text helper macros */
#ifdef HL_IN_WIN32_UNICODE
#define HL_NTEXT(txt) L##txt
#else
#define HL_NTEXT(txt) txt
#endif

/* Boolean type */
#ifdef __cplusplus
/* C++ bool type */
typedef bool HlBool;
#define HL_TRUE true
#define HL_FALSE false
#elif (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) ||\
    (defined(_MSC_VER) && _MSC_VER >= 1800 && !defined(__STDC__))
/* C99+ bool type */
typedef _Bool HlBool;
#define HL_TRUE 1
#define HL_FALSE 0
#else
/* Pre-C99 bool type */
typedef unsigned char HlBool;
#define HL_TRUE 1
#define HL_FALSE 0
#endif

/* Reference types */
typedef enum HlRefType
{
    HL_REF_TYPE_PTR,
    HL_REF_TYPE_NAME
}
HlRefType;

#define HL_REF(type) struct { HlRefType refType; union {\
    type* ptr; const char* name; } data; }

/* Static assert macros */
#ifdef __cpp_static_assert
/* C++11 static assert */
#define HL_STATIC_ASSERT(expression, msg) static_assert(expression, msg)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ == 201112L
/* C11 static assert */
#define HL_STATIC_ASSERT(expression, msg) _Static_assert(expression, msg)
#else
/* No static assert */
#define HL_STATIC_ASSERT(expression, msg)
#endif

#define HL_STATIC_ASSERT_SIZE(type, size) HL_STATIC_ASSERT(sizeof(type) == size,\
    "sizeof(" #type ") != expected size (" #size ").")

/* Helper macros */
#define HL_EMPTY
#define HL_BIT_COUNT(v) (sizeof(v) * 8U)
#define HL_BIT_FLAG(index) ((size_t)1U << (size_t)(index))
#define HL_ADD_OFF(ptr, off) (((unsigned char*)(ptr)) + (off))
#define HL_ADD_OFFC(ptr, off) (((const unsigned char*)(ptr)) + (off))

#ifdef HL_IS_BIG_ENDIAN
#define HL_MAKE_SIG(c1, c2, c3, c4) (\
    ((unsigned int)((unsigned char)(c1)) << 24) |\
    ((unsigned int)((unsigned char)(c2)) << 16) |\
    ((unsigned int)((unsigned char)(c3)) << 8) |\
    ((unsigned int)((unsigned char)(c4))))
#else
#define HL_MAKE_SIG(c1, c2, c3, c4) (\
    ((unsigned int)((unsigned char)(c4)) << 24) |\
    ((unsigned int)((unsigned char)(c3)) << 16) |\
    ((unsigned int)((unsigned char)(c2)) << 8) |\
    ((unsigned int)((unsigned char)(c1))))
#endif

/*
   Offset macros; currently these exist only for code
   readability, but that may change in the future.
*/
#define HL_OFF32(type) HlU32
#define HL_OFF64(type) HlU64
#define HL_OFF32_STR HL_OFF32(char)
#define HL_OFF64_STR HL_OFF64(char)

/* Offset helpers. */
#if HL_UPTR_MAX > 0xffffffffu
/* Pointers don't fit within 32-bits, so use 32-bit offsets as relative pointers. */
HL_API void* hlOff32Get(const HlU32* offPtr);
HL_API void hlOff32Set(HlU32* offPtr, void* ptr);
#else
/* Pointers fit within 32-bits, so just use 32-bit offsets as pointers directly. */
#define hlOff32Get(offPtr)          ((void*)((HlUPtr)(*(offPtr))))
#define hlOff32Set(offPtr, ptr)     *(offPtr) = (HlU32)((HlUPtr)(ptr))
#endif

#if HL_UPTR_MAX > 0xffffffffffffffffu
/* Pointers don't fit within 64-bits, so use 64-bit offsets as relative pointers. */
HL_API void* hlOff64Get(const HlU64* offPtr);
HL_API void hlOff64Set(HlU64* offPtr, void* ptr);
#else
/* Pointers fit within 64-bits, so just use 64-bit offsets as pointers directly. */
#define hlOff64Get(offPtr)          ((void*)((HlUPtr)(*(offPtr))))
#define hlOff64Set(offPtr, ptr)     *(offPtr) = (HlU64)((HlUPtr)(ptr))
#endif

#define hlOff32Fix(offPtr, base)    hlOff32Set(offPtr, HL_ADD_OFF(base, *(offPtr)))
#define hlOff64Fix(offPtr, base)    hlOff64Set(offPtr, HL_ADD_OFF(base, *(offPtr)))

/* Alignment helpers. */
#define HL_ALIGN(value, stride)\
    (((stride) < 2) ? (value) :                         /* If stride is < 2, just return value. */\
    ((((value) + ((stride) - 1)) & ~((stride) - 1))))   /* Align value by stride and return. */

#ifndef HL_NO_EXTERNAL_WRAPPERS
HL_API void* hlOff32GetExt(const HlU32* offPtr);
HL_API void hlOff32SetExt(HlU32* offPtr, void* ptr);
HL_API void* hlOff64GetExt(const HlU64* offPtr);
HL_API void hlOff64SetExt(HlU64* offPtr, void* ptr);
HL_API void hlOff32FixExt(HlU32* offPtr, void* base);
HL_API void hlOff64FixExt(HlU64* offPtr, void* base);
#endif

#ifdef __cplusplus
}
#endif

/*#ifdef __cplusplus
#define HL_DEFVAL(v) = v
#else
#define HL_DEFVAL(v)
#endif*/

/* Result type. */
typedef enum HlResult
{
    HL_RESULT_SUCCESS = 0,
    HL_ERROR_UNKNOWN,
    HL_ERROR_UNSUPPORTED,
    HL_ERROR_OUT_OF_MEMORY,
    HL_ERROR_INVALID_ARGS,
    HL_ERROR_OUT_OF_RANGE,
    HL_ERROR_ALREADY_EXISTS,
    HL_ERROR_NOT_FOUND,
    HL_ERROR_INVALID_DATA
}
HlResult;

#define HL_OK(result)       (result == HL_RESULT_SUCCESS)
#define HL_FAILED(result)   !HL_OK(result)
#endif
