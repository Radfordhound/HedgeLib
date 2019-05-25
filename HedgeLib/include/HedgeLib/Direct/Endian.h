#pragma once
#include "Errors.h"
#include "HedgeLib.h"
#include <stdint.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef _WIN32
#include <intrin.h>
#endif

inline void hl_SwapUInt16(uint16_t* v)
{
#ifdef _WIN32
    *v = _byteswap_ushort(*v);
#elif __GNUC__
    *v = __builtin_bswap16(*v);
#else
    *v = (uint16_t)(((*v & 0xFF) << 8) |
        ((*v & 0xFF00) >> 8));
#endif
}

inline void hl_SwapInt16(int16_t* v)
{
    hl_SwapUInt16((uint16_t*)v);
}

inline void hl_SwapUInt32(uint32_t* v)
{
#ifdef _WIN32
    *v = _byteswap_ulong(*v);
#elif __GNUC__
    *v = __builtin_bswap32(*v);
#else
    *v = (uint32_t)(((*v & 0xFF) << 24) |
        ((*v & 0xFF00) << 8) | ((*v & 0xFF0000) >> 8) |
        ((*v & 0xFF000000) >> 24));
#endif
}

inline void hl_SwapInt32(int32_t* v)
{
    hl_SwapUInt32((uint32_t*)v);
}

inline void hl_SwapFloat(float* v)
{
    hl_SwapUInt32((uint32_t*)v);
}

HL_STATIC_ASSERT_SIZE(float, 4);

inline void hl_SwapUInt64(uint64_t* v)
{
#ifdef _WIN32
    *v = _byteswap_uint64(*v);
#elif __GNUC__
    *v = __builtin_bswap64(*v);
#else
    *v = (*v & 0x00000000FFFFFFFF) << 32 |
        (*v & 0xFFFFFFFF00000000) >> 32;
    *v = (*v & 0x0000FFFF0000FFFF) << 16 |
        (*v & 0xFFFF0000FFFF0000) >> 16;
    *v = (*v & 0x00FF00FF00FF00FF) << 8 |
        (*v & 0xFF00FF00FF00FF00) >> 8;
#endif
}

inline void hl_SwapInt64(int64_t* v)
{
    hl_SwapUInt64((uint64_t*)v);
}

inline void hl_SwapDouble(double* v)
{
    hl_SwapUInt64((uint64_t*)v);
}

HL_STATIC_ASSERT_SIZE(double, 8);

#define HL_ENDIAN_SWAP_NAME(type) type##_EndianSwap
#define HL_ENDIAN_SWAP_RECURSIVE_NAME(type) type##_EndianSwapRecursive

#define HL_ENDIAN_SWAP(type, v) HL_ENDIAN_SWAP_NAME(type)(v)
#define HL_ENDIAN_SWAP_RECURSIVE(type, v, be) HL_ENDIAN_SWAP_RECURSIVE_NAME(type)(v, be)

#define HL_DECL_ENDIAN_SWAP(type) \
    HL_API void HL_ENDIAN_SWAP_NAME(type)(struct type* v)

#define HL_DECL_ENDIAN_SWAP_RECURSIVE(type) \
    HL_API void HL_ENDIAN_SWAP_RECURSIVE_NAME(type)(struct type* v, bool be);

#define HL_IMPL_ENDIAN_SWAP(type, v) \
    void HL_ENDIAN_SWAP_NAME(type)(struct type* v)

#define HL_IMPL_ENDIAN_SWAP_RECURSIVE(type, v, be) \
    void HL_ENDIAN_SWAP_RECURSIVE_NAME(type)(struct type* v, bool be)

#define HL_INLN_ENDIAN_SWAP(type, v) \
    inline HL_IMPL_ENDIAN_SWAP(type, v)

#define HL_INLN_ENDIAN_SWAP_RECURSIVE(type, v, be) \
    inline HL_IMPL_ENDIAN_SWAP_RECURSIVE(type, v, be)

#ifdef __cplusplus
#define HL_DECL_ENDIAN_SWAP_CPP() HL_API void EndianSwap()

#define HL_DECL_ENDIAN_SWAP_RECURSIVE_CPP() \
    HL_API void EndianSwapRecursive(bool isBigEndian)

#define HL_INLN_ENDIAN_SWAP_CPP() inline void EndianSwap()

#define HL_INLN_ENDIAN_SWAP_RECURSIVE_CPP() \
    inline void EndianSwapRecursive(bool isBigEndian)

#define HL_IMPL_ENDIAN_SWAP_CPP(type) \
    void type::EndianSwap() { HL_ENDIAN_SWAP(type, this); }

#define HL_IMPL_ENDIAN_SWAP_RECURSIVE_CPP(type) \
    void type::EndianSwapRecursive(bool isBigEndian) { \
        HL_ENDIAN_SWAP_RECURSIVE(type, this, isBigEndian); }
#else
#define HL_DECL_ENDIAN_SWAP_CPP()
#define HL_DECL_ENDIAN_SWAP_RECURSIVE_CPP()
#define HL_INLN_ENDIAN_SWAP_CPP()
#define HL_INLN_ENDIAN_SWAP_RECURSIVE_CPP()
#define HL_IMPL_ENDIAN_SWAP_CPP(type)
#define HL_IMPL_ENDIAN_SWAP_RECURSIVE_CPP(type)
#endif
