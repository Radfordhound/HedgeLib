#pragma once
#include "Offsets.h"
#include <stdint.h>

#ifdef __cplusplus
#include "Helpers.h"
extern "C" {
#else
#include <stdbool.h>
#endif

#ifdef _WIN32
#include <intrin.h>
#endif

inline void hl_SwapUInt16(uint16_t* v)
{
#ifdef _WIN32
    * v = _byteswap_ushort(*v);
#elif __GNUC__
    * v = __builtin_bswap16(*v);
#else
    * v = (uint16_t)(((*v & 0xFF) << 8) |
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
    * v = _byteswap_ulong(*v);
#elif __GNUC__
    * v = __builtin_bswap32(*v);
#else
    * v = (uint32_t)(((*v & 0xFF) << 24) |
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
    * v = _byteswap_uint64(*v);
#elif __GNUC__
    * v = __builtin_bswap64(*v);
#else
    * v = (*v & 0x00000000FFFFFFFF) << 32 |
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

#define HL_IMPL_ENDIAN_SWAP(type) \
    void HL_ENDIAN_SWAP_NAME(type)(struct type* v)

#define HL_IMPL_ENDIAN_SWAP_RECURSIVE(type) \
    void HL_ENDIAN_SWAP_RECURSIVE_NAME(type)(struct type* v, bool be)

#define HL_INLN_ENDIAN_SWAP(type) \
    inline HL_IMPL_ENDIAN_SWAP(type)

#define HL_INLN_ENDIAN_SWAP_RECURSIVE(type) \
    inline HL_IMPL_ENDIAN_SWAP_RECURSIVE(type)

#define HL_ENDIAN_SWAP_ARR32(type, v, be, swapCall) {\
    if (be) hl_SwapUint32(&(v.Count));\
\
    type* ptr = HL_GETPTR32(type, v.Offset);\
    for (uint32_t i = 0; i < v.Count; ++i)\
    {\
        swapCall;\
    }\
\
    if (!be) hl_SwapUint32(&(v.Count));\
}

#define HL_ENDIAN_SWAP_ARR64(type, v, be, swapCall) {\
    if (be) hl_SwapUint64(&(v.Count));\
\
    type* ptr = HL_GETPTR64(type, v.Offset);\
    for (uint64_t i = 0; i < v.Count; ++i)\
    {\
        swapCall;\
    }\
\
    if (!be) hl_SwapUint64(&(v.Count));\
}

#define HL_ENDIAN_SWAP_ARR32_RECURSIVE(type, v, be) HL_ENDIAN_SWAP_ARR32(\
    type, v, be, HL_ENDIAN_SWAP_RECURSIVE(type, v, be))

#define HL_ENDIAN_SWAP_ARR64_RECURSIVE(type, v, be) HL_ENDIAN_SWAP_ARR64(\
    type, v, be, HL_ENDIAN_SWAP_RECURSIVE(type, v, be))

// C++ Specific
#ifdef __cplusplus
}

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

namespace internal_impl
{
    template<typename T>
    using EndianSwap_t = decltype(std::declval<T&>().EndianSwap());

    template<typename T>
    constexpr bool HasEndianSwapFunction = is_detected_v<EndianSwap_t, T>;

    template<typename T>
    using EndianSwapRecursive_t = decltype(std::declval
        <T&>().EndianSwapRecursive(true));

    template<typename T>
    constexpr bool HasEndianSwapRecursiveFunction =
        is_detected_v<EndianSwapRecursive_t, T>;
}

template<typename T>
inline void hl_Swap(T& value)
{
    if constexpr (internal_impl::HasEndianSwapFunction<T>)
    {
        value.EndianSwap();
    }
}

inline void hl_Swap(uint16_t& value)
{
    hl_SwapUInt16(&value);
}

inline void hl_Swap(int16_t& value)
{
    hl_SwapInt16(&value);
}

inline void hl_Swap(uint32_t& value)
{
    hl_SwapUInt32(&value);
}

inline void hl_Swap(int32_t& value)
{
    hl_SwapInt32(&value);
}

inline void hl_Swap(float& value)
{
    hl_SwapFloat(&value);
}

inline void hl_Swap(uint64_t& value)
{
    hl_SwapUInt64(&value);
}

inline void hl_Swap(int64_t& value)
{
    hl_SwapInt64(&value);
}

inline void hl_Swap(double& value)
{
    hl_SwapDouble(&value);
}

inline void hl_Swap(hl_ArrOff32& value)
{
    hl_SwapUInt32(&value.Count);
}

inline void hl_Swap(hl_ArrOff64& value)
{
    hl_SwapUInt64(&value.Count);
}

template<typename T, typename... Args>
inline void hl_Swap(T& value, Args& ... args)
{
    hl_Swap(value);
    hl_Swap(args...);
}

template<typename T>
inline void hl_SwapRecursive(bool isBigEndian, T& value)
{
    if constexpr (internal_impl::HasEndianSwapRecursiveFunction<T>)
    {
        value.EndianSwapRecursive(isBigEndian);
    }
    else
    {
        hl_Swap(value);
    }
}

template<typename T, typename... Args>
inline void hl_SwapRecursive(bool isBigEndian, T& value, Args& ... args)
{
    hl_SwapRecursive(isBigEndian, value);
    hl_SwapRecursive(isBigEndian, args...);
}

template<typename T>
inline void hl_SwapArray(T* ptr, size_t count, bool isBigEndian)
{
    for (size_t i = 0; i < count; ++i)
    {
        hl_SwapRecursive(isBigEndian, ptr[i]);
    }
}

template<typename T>
inline void hl_SwapRecursive(bool isBigEndian, hl_ArrOff32& value)
{
    if (isBigEndian) hl_SwapUInt32(&value.Count);

    hl_SwapArray<T>(HL_GETPTR32(T, value.Offset),
        static_cast<size_t>(value.Count), isBigEndian);

    if (!isBigEndian) hl_SwapUInt32(&value.Count);
}

template<typename T>
inline void hl_SwapRecursive(bool isBigEndian, hl_ArrOff64& value)
{
    if (isBigEndian) hl_SwapUInt64(&value.Count);

    hl_SwapArray<T>(HL_GETPTR64(T, value.Offset),
        static_cast<size_t>(value.Count), isBigEndian);

    if (!isBigEndian) hl_SwapUInt64(&value.Count);
}
#else
#define HL_DECL_ENDIAN_SWAP_CPP()
#define HL_DECL_ENDIAN_SWAP_RECURSIVE_CPP()
#define HL_INLN_ENDIAN_SWAP_CPP()
#define HL_INLN_ENDIAN_SWAP_RECURSIVE_CPP()
#define HL_IMPL_ENDIAN_SWAP_CPP(type)
#define HL_IMPL_ENDIAN_SWAP_RECURSIVE_CPP(type)
#endif
