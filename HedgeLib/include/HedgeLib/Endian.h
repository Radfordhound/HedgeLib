#pragma once
#include <cstdint>
#include <utility>
#include <limits>
#include <type_traits>

#ifdef _WIN32
#include <intrin.h>
#endif

// non-clang compilers die if you don't do this
#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

namespace hl
{
    inline void Swap(std::uint16_t& v)
    {
#ifdef _WIN32
        // Use MSVC intrinsic
        v = _byteswap_ushort(v);
#elif (defined(__clang__) && __has_builtin(__builtin_bswap16)) || \
(defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)))
        // Use Clang/GCC 4.8+ intrinsic
        v = __builtin_bswap16(v);
#else
        // Endian-swap by hand. Most compilers should hopefully
        // be smart enough to turn this into an intrinsic anyway.
        v = static_cast<std::uint16_t>(
            ((v & 0xFF) << 8) | ((v & 0xFF00) >> 8));
#endif
    }

    inline void Swap(std::int16_t& v)
    {
        // Swap signed value as though it's unsigned
        Swap(reinterpret_cast<std::uint16_t&>(v));
    }

    inline void Swap(std::uint32_t& v)
    {
#ifdef _WIN32
        // Use MSVC intrinsic
        v = _byteswap_ulong(v);
#elif (defined(__clang__) && __has_builtin(__builtin_bswap32)) || \
(defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)))
        // Use Clang/GCC 4.3+ intrinsic
        v = __builtin_bswap32(v);
#else
        // Endian-swap by hand. Most compilers should hopefully
        // be smart enough to turn this into an intrinsic anyway.
        v = static_cast<std::uint32_t>(((v & 0xFF) << 24) |
            ((v & 0xFF00) << 8) | ((v & 0xFF0000) >> 8) |
            ((v & 0xFF000000) >> 24));
#endif
    }

    inline void Swap(std::int32_t& v)
    {
        // Swap signed value as though it's unsigned
        Swap(reinterpret_cast<std::uint32_t&>(v));
    }

    static_assert(std::numeric_limits<float>::is_iec559,
        "Floats must follow the IEEE_754 standard for HedgeLib to properly compile.");

    static_assert(std::numeric_limits<float>::digits == 24,
        "Floats must be 32-bit values for HedgeLib to properly compile.");

    inline void Swap(float& v)
    {
        // Swap float as though it's an unsigned 32-bit integer
        Swap(reinterpret_cast<std::uint32_t&>(v));
    }

    inline void Swap(std::uint64_t& v)
    {
#ifdef _WIN32
        // Use MSVC intrinsic
        v = _byteswap_uint64(v);
#elif (defined(__clang__) && __has_builtin(__builtin_bswap64)) || \
(defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)))
        // Use Clang/GCC 4.3+ intrinsic
        v = __builtin_bswap64(v);
#else
        // Endian-swap by hand. Most compilers should hopefully
        // be smart enough to turn this into an intrinsic anyway.
        v = ((v & 0x00000000FFFFFFFF) << 32 |
            (v & 0xFFFFFFFF00000000) >> 32);
        v = ((v & 0x0000FFFF0000FFFF) << 16 |
            (v & 0xFFFF0000FFFF0000) >> 16);
        v = ((v & 0x00FF00FF00FF00FF) << 8 |
            (v & 0xFF00FF00FF00FF00) >> 8);
#endif
    }

    inline void Swap(std::int64_t& v)
    {
        // Swap signed value as though it's unsigned
        Swap(reinterpret_cast<std::uint64_t&>(v));
    }

    static_assert(std::numeric_limits<double>::is_iec559,
        "Doubles must follow the IEEE_754 standard for HedgeLib to properly compile.");

    static_assert(std::numeric_limits<double>::digits == 53,
        "Doubles must be 64-bit values for HedgeLib to properly compile.");

    inline void Swap(double& v)
    {
        // Swap double as though it's an unsigned 64-bit integer
        Swap(reinterpret_cast<std::uint64_t&>(v));
    }

    namespace internal
    {
        // Adapted from "Possible implementation" section of this page:
        // https://en.cppreference.com/w/cpp/experimental/nonesuch

        // TODO: Once std::nonesuch is added to the C++ standard, add an ifdef that
        // checks if it's present, and if so, aliases it instead of using this.
        struct nonesuch
        {
            ~nonesuch() = delete;
            nonesuch(nonesuch const&) = delete;
            void operator=(nonesuch const&) = delete;
        };

        // Adapted from "Possible implementation" section of this page:
        // https://en.cppreference.com/w/cpp/experimental/is_detected

        // TODO: Once std::is_detected is added to the C++ standard, add an ifdef that
        // checks if it's present, and if so, aliases it instead of using this.

        template <class Default, class AlwaysVoid,
            template<class...> class Op, class... Args>
        struct Detector
        {
            using value_t = std::false_type;
            using type = Default;
        };

        template <class... _Types>
        using void_t =
#ifdef __cpp_lib_void_t
        std::void_t<_Types...>;
#else
        void;
#endif

        template <class Default, template<class...> class Op, class... Args>
        struct Detector<Default, void_t<Op<Args...>>, Op, Args...>
        {
            using value_t = std::true_type;
            using type = Op<Args...>;
        };

        template <template<class...> class Op, class... Args>
        using is_detected = typename Detector<nonesuch, void, Op, Args...>::value_t;

        template<template<class...> class Op, class... Args>
        constexpr bool is_detected_v = is_detected<Op, Args...>::value;

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

#ifdef __cpp_if_constexpr
    template<typename T>
    inline void Swap(T& value)
    {
        if constexpr (internal::HasEndianSwapFunction<T>)
        {
            value.EndianSwap();
        }
    }

    template<typename T>
    inline void SwapRecursive(bool isBigEndian, T& value)
    {
        if constexpr (internal::HasEndianSwapRecursiveFunction<T>)
        {
            value.EndianSwapRecursive(isBigEndian);
        }
        else
        {
            Swap(value);
        }
    }
#else
    template<typename T>
    inline typename std::enable_if<internal::HasEndianSwapFunction<T>, void>::type
        Swap(T& value)
    {
        value.EndianSwap();
    }

    template<typename T>
    inline typename std::enable_if<!internal::HasEndianSwapFunction<T>, void>::type
        Swap(T& value) {}

    template<typename T>
    inline typename std::enable_if<internal::HasEndianSwapRecursiveFunction<T>, void>::type
        SwapRecursive(bool isBigEndian, T& value)
    {
        value.EndianSwapRecursive(isBigEndian);
    }

    template<typename T>
    inline typename std::enable_if<!internal::HasEndianSwapRecursiveFunction<T>, void>::type
        SwapRecursive(bool isBigEndian, T& value)
    {
        Swap(value);
    }
#endif

    template<typename T, typename... Args>
    inline void Swap(T& value, Args& ... args)
    {
        Swap(value);
        Swap(args...);
    }

    template<typename T, typename... Args>
    inline void SwapRecursive(bool isBigEndian, T& value, Args& ... args)
    {
        SwapRecursive(isBigEndian, value);
        SwapRecursive(isBigEndian, args...);
    }
}
