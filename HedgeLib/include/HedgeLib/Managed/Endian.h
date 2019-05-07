#pragma once
#include "HedgeLib/Direct/Endian.h"
#include "HedgeLib/Direct/Offsets.h"
#include "Helpers.h"
#include <cstdint>
#include <cstddef>
#include <utility>

namespace HedgeLib::Endian
{
    inline void Swap16(std::uint16_t& v)
    {
        return hl_SwapUInt16(&v);
    }

    inline void Swap16(std::int16_t& v)
    {
        return hl_SwapInt16(&v);
    }

    inline void Swap32(std::uint32_t& v)
    {
        return hl_SwapUInt32(&v);
    }

    inline void Swap32(std::int32_t& v)
    {
        return hl_SwapInt32(&v);
    }

    inline void Swap32(float& v)
    {
        return hl_SwapFloat(&v);
    }

    inline void Swap64(std::uint64_t& v)
    {
        return hl_SwapUInt64(&v);
    }

    inline void Swap64(std::int64_t& v)
    {
        return hl_SwapInt64(&v);
    }

    inline void Swap64(double& v)
    {
        return hl_SwapDouble(&v);
    }

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

    template<typename T>
    inline void Swap(T& value)
    {
        if constexpr (HasEndianSwapFunction<T>)
        {
            value.EndianSwap();
        }
    }

    inline void Swap(std::uint16_t& value)
    {
        Swap16(value);
    }

    inline void Swap(std::int16_t& value)
    {
        Swap16(value);
    }

    inline void Swap(std::uint32_t& value)
    {
        Swap32(value);
    }

    inline void Swap(std::int32_t& value)
    {
        Swap32(value);
    }

    inline void Swap(float& value)
    {
        Swap32(value);
    }

    inline void Swap(std::uint64_t& value)
    {
        Swap64(value);
    }

    inline void Swap(std::int64_t& value)
    {
        Swap64(value);
    }

    inline void Swap(double& value)
    {
        Swap64(value);
    }

    inline void Swap(hl_ArrOff32& value)
    {
        Swap32(value.Count);
    }

    inline void Swap(hl_ArrOff64& value)
    {
        Swap64(value.Count);
    }

    template<typename T, typename... Args>
    inline void Swap(T& value, Args&... args)
    {
        Swap(value);
        Swap(args...);
    }

    template<typename T>
    inline void SwapRecursive(bool isBigEndian, T& value)
    {
        if constexpr (HasEndianSwapRecursiveFunction<T>)
        {
            value.EndianSwapRecursive(isBigEndian);
        }
        else
        {
            Swap(value);
        }
    }

    template<typename T, typename... Args>
    inline void SwapRecursive(bool isBigEndian, T& value, Args& ... args)
    {
        SwapRecursive(isBigEndian, value);
        SwapRecursive(isBigEndian, args...);
    }

    template<typename T>
    inline void SwapArray(T* ptr, std::size_t count, bool isBigEndian)
    {
        for (std::size_t i = 0; i < count; ++i)
        {
            SwapRecursive(isBigEndian, ptr[i]);
        }
    }

    template<typename T>
    inline void SwapArray(hl_ArrOff32& value, bool isBigEndian)
    {
        if (isBigEndian) hl_SwapUInt32(&value.Count);

        SwapArray<T>(HL_GETPTR32(T, value.Offset),
            static_cast<std::size_t>(value.Count), isBigEndian);

        if (!isBigEndian) hl_SwapUInt32(&value.Count);
    }

    template<typename T>
    inline void SwapArray(hl_ArrOff64& value, bool isBigEndian)
    {
        if (isBigEndian) hl_SwapUInt64(&value.Count);

        SwapArray<T>(HL_GETPTR64(T, value.Offset),
            static_cast<std::size_t>(value.Count), isBigEndian);

        if (!isBigEndian) hl_SwapUInt64(&value.Count);
    }
}
