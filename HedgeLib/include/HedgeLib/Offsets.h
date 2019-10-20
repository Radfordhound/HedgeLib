#pragma once
#include "HedgeLib.h"
#include "Errors.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
#include <vector>

// hl_SwapRecursive forward declaration
template<typename T>
void hl_SwapRecursive(bool isBigEndian, T& value);

extern "C" {
#else
#include <stdbool.h>
#endif

// Offsets
typedef uint32_t hl_DataOff32;
typedef uint64_t hl_DataOff64;

HL_API void hl_FixOffset32(hl_DataOff32* off, const void* data, bool isBigEndian);
HL_API void hl_FixOffset64(hl_DataOff64* off, const void* data, bool isBigEndian);

// Arrays
struct hl_ArrOff32
{
    uint32_t Count;
    hl_DataOff32 Offset;
};

struct hl_ArrOff64
{
    uint64_t Count;
    hl_DataOff64 Offset;
};

// Offset Table
#ifdef __cplusplus
using hl_OffsetTable = std::vector<long>;
#else
typedef struct hl_OffsetTable hl_OffsetTable;
#endif

HL_API hl_OffsetTable* hl_CreateOffsetTable();
HL_API HL_RESULT hl_AddOffset(hl_OffsetTable* offTable, long offset);
HL_API long* hl_GetOffsetsPtr(hl_OffsetTable* offTable);
HL_API size_t hl_GetOffsetCount(const hl_OffsetTable* offTable);
HL_API void hl_GetOffsets(hl_OffsetTable* offTable,
    long** offsets, size_t* count);

HL_API void hl_DestroyOffsetTable(hl_OffsetTable* offTable);

// Get/Set Macros & x64 Macros
#ifdef x86
#define HL_GETPTR32(type, off) ((type*)((uintptr_t)(off)))
#define HL_SETPTR32(off, ptr) off = (hl_DataOff32)((uintptr_t)(ptr))
#elif x64
#define HL_GETPTR32(type, off) ((type*)((uintptr_t)(&(off)) + (hl_DataOff32)(off)))
#define HL_SETPTR32(off, ptr) off = (hl_DataOff32)((uintptr_t)(ptr) - (hl_DataOff32)(&(off)))
#endif

#define HL_GETPTR64(type, off) ((type*)((uintptr_t)(off)))
#define HL_SETPTR64(off, ptr) off = (hl_DataOff64)((uintptr_t)(ptr))

#define HL_GETVAL32(type, off) *HL_GETPTR32(type, off)
#define HL_GETVAL64(type, off) *HL_GETPTR64(type, off)

#define HL_GETABS(type, baseAddress, offset) ((type*)((uint8_t*)(baseAddress) + (offset)))
#ifndef __cplusplus
// C Offset/Array Macros
#define HL_OFF32(type) hl_DataOff32
#define HL_OFF64(type) hl_DataOff64
#define HL_ARR32(type) hl_ArrOff32
#define HL_ARR64(type) hl_ArrOff64
#else
}

template<typename T, typename OffsetType>
inline T* hl_GetAbs(void* baseAddress, OffsetType offset)
{
    return reinterpret_cast<T*>(static_cast<uint8_t*>(
        baseAddress) + offset);
}

template<typename T, typename OffsetType>
inline const T* hl_GetAbs(const void* baseAddress, OffsetType offset)
{
    return reinterpret_cast<const T*>(static_cast<const uint8_t*>(
        baseAddress) + offset);
}

// C++ DataOffset template classes
template<typename T>
class hl_DataOffset32
{
    hl_DataOff32 off;

public:
    constexpr hl_DataOffset32() = default;
    constexpr hl_DataOffset32(hl_DataOff32 off) : off(off) {}

    inline T* Get()
    {
        return HL_GETPTR32(T, off);
    }

    inline const T* Get() const
    {
        return HL_GETPTR32(const T, off);
    }

    inline void Set(uintptr_t ptr)
    {
        HL_SETPTR32(off, ptr);
    }

    inline void Set(T* ptr)
    {
        Set(reinterpret_cast<uintptr_t>(ptr));
    }

    inline void Set(const T* ptr)
    {
        Set(reinterpret_cast<uintptr_t>(ptr));
    }

    inline operator hl_DataOff32&()
    {
        return off;
    }

    inline operator const hl_DataOff32& () const
    {
        return off;
    }

    inline operator T*()
    {
        return HL_GETPTR32(T, off);
    }

    inline operator const T* () const
    {
        return HL_GETPTR32(const T, off);
    }

    inline T* operator*()
    {
        return HL_GETPTR32(T, off);
    }

    inline const T* operator*() const
    {
        return HL_GETPTR32(const T, off);
    }

    inline T* operator->()
    {
        return HL_GETPTR32(T, off);
    }

    inline const T* operator->() const
    {
        return HL_GETPTR32(const T, off);
    }

    inline void EndianSwapRecursive(bool isBigEndian)
    {
        hl_SwapRecursive(isBigEndian, *HL_GETPTR32(T, off));
    }
};

template<typename T>
class hl_DataOffset64
{
    hl_DataOff64 off;

public:
    constexpr hl_DataOffset64() = default;
    constexpr hl_DataOffset64(hl_DataOff64 off) : off(off) {}
    inline hl_DataOffset64(T* ptr) : off(HL_GETOFF64(ptr)) {}

    inline T* Get()
    {
        return HL_GETPTR64(T, off);
    }

    inline const T* Get() const
    {
        return HL_GETPTR64(const T, off);
    }

    inline void Set(uintptr_t ptr)
    {
        HL_SETPTR64(off, ptr);
    }

    inline void Set(T* ptr)
    {
        HL_SETPTR64(off, ptr);
    }

    inline operator hl_DataOff64&()
    {
        return off;
    }

    inline operator const hl_DataOff64& () const
    {
        return off;
    }

    inline operator T* ()
    {
        return HL_GETPTR64(T, off);
    }

    inline operator const T* () const
    {
        return HL_GETPTR64(const T, off);
    }

    inline T* operator*()
    {
        return HL_GETPTR64(T, off);
    }

    inline const T* operator*() const
    {
        return HL_GETPTR64(const T, off);
    }

    inline T* operator->()
    {
        return HL_GETPTR64(T, off);
    }

    inline const T* operator->() const
    {
        return HL_GETPTR64(const T, off);
    }

    inline void EndianSwapRecursive(bool isBigEndian)
    {
        hl_SwapRecursive(isBigEndian, *Get());
    }
};

// C++ ArrayOffset template classes
template<typename T>
struct hl_ArrayOffset32
{
    uint32_t Count;
    hl_DataOffset32<T> Offset;

    constexpr hl_ArrayOffset32() = default;
    constexpr hl_ArrayOffset32(uint32_t count, hl_DataOff32 off) : 
        Count(count), Offset(off) {}

    inline T* Get()
    {
        return Offset.Get();
    }

    inline const T* Get() const
    {
        return Offset.Get();
    }

    inline void Set(uintptr_t ptr)
    {
        Offset.Set(ptr);
    }

    inline void Set(T* ptr)
    {
        Offset.Set(ptr);
    }

    inline hl_ArrOff32& GetArray()
    {
        return reinterpret_cast<hl_ArrOff32&>(*this);
    }

    inline const hl_ArrOff32& GetArray() const
    {
        return reinterpret_cast<const hl_ArrOff32&>(*this);
    }

    inline operator hl_DataOff32& ()
    {
        return Offset;
    }

    inline operator const hl_DataOff32& () const
    {
        return Offset;
    }

    inline operator T* ()
    {
        return Offset.Get();
    }

    inline operator const T* () const
    {
        return Offset.Get();
    }

    inline T* operator->()
    {
        return Offset.Get();
    }

    inline const T* operator->() const
    {
        return Offset.Get();
    }

    /*inline void EndianSwap()
    {
        hl_Swap(GetArray());
    }

    inline void EndianSwapRecursive(bool isBigEndian)
    {
        hl_SwapRecursive<T>(isBigEndian, GetArray());
    }*/
};

template<typename T>
class hl_ArrayOffset64
{
    uint64_t Count;
    hl_DataOffset64<T> Offset;

    constexpr hl_ArrayOffset64() = default;
    constexpr hl_ArrayOffset64(uint64_t count, hl_DataOff64 off) :
        Count(count), Offset(off) {}

    inline T* Get()
    {
        return Offset.Get();
    }

    inline const T* Get() const
    {
        return Offset.Get();
    }

    inline void Set(uintptr_t ptr)
    {
        Offset.Set(ptr);
    }

    inline void Set(T* ptr)
    {
        Offset.Set(ptr);
    }

    inline hl_ArrOff64& GetArray()
    {
        return reinterpret_cast<hl_ArrOff64&>(*this);
    }

    inline const hl_ArrOff64& GetArray() const
    {
        return reinterpret_cast<const hl_ArrOff64&>(*this);
    }

    inline operator hl_DataOff64& ()
    {
        return Offset;
    }

    inline operator const hl_DataOff64& () const
    {
        return Offset;
    }

    inline operator T* ()
    {
        return Offset.Get();
    }

    inline operator const T* () const
    {
        return Offset.Get();
    }

    inline T* operator->()
    {
        return Offset.Get();
    }

    inline const T* operator->() const
    {
        return Offset.Get();
    }

    /*inline void EndianSwap()
    {
        hl_Swap(GetArray());
    }

    inline void EndianSwapRecursive(bool isBigEndian)
    {
        hl_SwapRecursive(isBigEndian, GetArray());
    }*/
};

inline void hl_FixOffset(hl_DataOff32* off, const void* data, bool isBigEndian)
{
    hl_FixOffset32(off, data, isBigEndian);
}

inline void hl_FixOffset(hl_DataOff64* off, const void* data, bool isBigEndian)
{
    hl_FixOffset64(off, data, isBigEndian);
}

// C++ Offset/Array Macros
#define HL_OFF32(type) hl_DataOffset32<type>
#define HL_OFF64(type) hl_DataOffset64<type>
#define HL_ARR32(type) hl_ArrayOffset32<type>
#define HL_ARR64(type) hl_ArrayOffset64<type>
#endif

// String macros
#define HL_STR32 HL_OFF32(char)
#define HL_STR64 HL_OFF64(char)

// Size asserts
#ifdef x64
HL_STATIC_ASSERT_SIZE(uintptr_t, 8);
HL_STATIC_ASSERT_SIZE(void*, 8);
HL_STATIC_ASSERT_SIZE(int*, 8);
#elif x86
HL_STATIC_ASSERT_SIZE(uintptr_t, 4);
HL_STATIC_ASSERT_SIZE(void*, 4);
HL_STATIC_ASSERT_SIZE(int*, 4);
#endif

HL_STATIC_ASSERT_SIZE(HL_OFF32(void), 4);
HL_STATIC_ASSERT_SIZE(HL_OFF32(int), 4);
HL_STATIC_ASSERT_SIZE(HL_OFF64(void), 8);
HL_STATIC_ASSERT_SIZE(HL_OFF64(int), 8);
