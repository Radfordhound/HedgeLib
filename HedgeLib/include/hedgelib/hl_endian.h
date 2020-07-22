#ifndef HL_ENDIAN_H_INCLUDED
#define HL_ENDIAN_H_INCLUDED
#include "hl_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1310
/* Use MSVC intrinsics. */
#include <stdlib.h>
#define hlSwapU16(v) ((HlU16)(_byteswap_ushort((unsigned short)(v))))
#define hlSwapU32(v) ((HlU32)(_byteswap_ulong((unsigned long)(v))))
#define hlSwapU64(v) ((HlU64)(_byteswap_uint64((unsigned __int64)(v))))
#else
#if (defined(__clang__) && defined(__has_builtin))
#if __has_builtin(__builtin_bswap16)
/* Use Clang intrinsics. */
#define hlSwapU16(v) ((HlU16)(__builtin_bswap16((HlU16)(v))))
#endif
#elif (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)))
/* Use GCC 4.8+ intrinsics. */
#define hlSwapU16(v) ((HlU16)(__builtin_bswap16((HlU16)(v))))
#else

/*
   Endian-swap manually. Most compilers should (hopefully)
   be smart enough to turn this into an intrinsic anyway.
*/
#define hlSwapU16(v) ((HlU16)(((((HlU16)(v)) & 0xFFU) << 8) |\
    ((((HlU16)(v)) & 0xFF00U) >> 8)))
#endif

#if (defined(__clang__) && defined(__has_builtin))
#if __has_builtin(__builtin_bswap16)
/* Use Clang intrinsics. */
#define hlSwapU32(v) ((HlU32)(__builtin_bswap32((HlU32)(v))))
#define hlSwapU64(v) ((HlU64)(__builtin_bswap64((HlU64)(v))))
#endif
#elif (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)))
/* Use GCC 4.3+ intrinsics. */
#define hlSwapU32(v) ((HlU32)(__builtin_bswap32((HlU32)(v))))
#define hlSwapU64(v) ((HlU64)(__builtin_bswap64((HlU64)(v))))
#else

/*
   Endian-swap manually. Most compilers should (hopefully)
   be smart enough to turn this into an intrinsic anyway.
*/
#define hlSwapU32(v) ((HlU32)(((((HlU32)(v)) & 0xFFU) << 24) |\
    ((((HlU32)(v)) & 0xFF00U) << 8) | ((((HlU32)(v)) & 0xFF0000U) >> 8) |\
    ((((HlU32)(v)) & 0xFF000000U) >> 24)))

#define hlSwapU64(v) ((HlU64)(((((HlU64)(v)) & 0xFFU) << 56) |\
    ((((HlU64)(v)) & 0xFF00U) << 40) | ((((HlU64)(v)) & 0xFF0000U) << 24) |\
    ((((HlU64)(v)) & 0xFF000000U) << 8) | ((((HlU64)(v)) & 0xFF00000000U) >> 8) |\
    ((((HlU64)(v)) & 0xFF0000000000U) >> 24) | ((((HlU64)(v)) & 0xFF000000000000U) >> 40)|\
    ((((HlU64)(v)) & 0xFF00000000000000U) >> 56)))
#endif
#endif

#define hlSwapS16(v) (HlS16)hlSwapU16(v)
#define hlSwapS32(v) (HlS32)hlSwapU32(v)
#define hlSwapS64(v) (HlS64)hlSwapU64(v)

HL_API float hlSwapFloat(float v);
HL_API double hlSwapDouble(double v);

#define hlSwapU16P(ptr) *(ptr) = hlSwapU16(*(ptr))
#define hlSwapU32P(ptr) *(ptr) = hlSwapU32(*(ptr))
#define hlSwapU64P(ptr) *(ptr) = hlSwapU64(*(ptr))
#define hlSwapS16P(ptr) *(ptr) = hlSwapS16(*(ptr))
#define hlSwapS32P(ptr) *(ptr) = hlSwapS32(*(ptr))
#define hlSwapS64P(ptr) *(ptr) = hlSwapS64(*(ptr))
#define hlSwapFloatP(ptr) *(ptr) = hlSwapFloat(*(ptr))
#define hlSwapDoubleP(ptr) *(ptr) = hlSwapDouble(*(ptr))

#ifndef HL_NO_EXTERNAL_WRAPPERS
HL_API HlU16 hlSwapU16Ext(HlU16 v);
HL_API HlU32 hlSwapU32Ext(HlU32 v);
HL_API HlU64 hlSwapU64Ext(HlU64 v);
HL_API HlS16 hlSwapS16Ext(HlS16 v);
HL_API HlS32 hlSwapS32Ext(HlS32 v);
HL_API HlS64 hlSwapS64Ext(HlS64 v);
HL_API void hlSwapU16PExt(HlU16* ptr);
HL_API void hlSwapU32PExt(HlU32* ptr);
HL_API void hlSwapU64PExt(HlU64* ptr);
HL_API void hlSwapS16PExt(HlS16* ptr);
HL_API void hlSwapS32PExt(HlS32* ptr);
HL_API void hlSwapS64PExt(HlS64* ptr);
HL_API void hlSwapFloatPExt(float* ptr);
HL_API void hlSwapDoublePExt(double* ptr);
#endif

#ifdef __cplusplus
}
#endif
#endif
