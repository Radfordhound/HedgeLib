#pragma once
#include "Helpers.h"
#include <utility>
#include <cstdint>

#ifdef _WIN32
#include <intrin.h>
#endif

namespace HedgeLib::IO::Endian
{
	inline void Swap16(std::uint16_t& v)
	{
#ifdef _WIN32
		v = _byteswap_ushort(v);
#elif __GNUC__
		v = __builtin_bswap16(v);
#else
		v = static_cast<std::uint16_t>(
			((v & 0xFF) << 8) |
			((v & 0xFF00) >> 8));
#endif
	}

	inline void Swap16(std::int16_t& v)
	{
		Swap16(*reinterpret_cast<std::uint16_t*>(&v));
	}

	inline void Swap32(std::uint32_t& v)
	{
#ifdef _WIN32
		v = _byteswap_ulong(v);
#elif __GNUC__
		v = __builtin_bswap32(v);
#else
		v = static_cast<std::uint32_t>(((v & 0xFF) << 24) |
			((v & 0xFF00) << 8) | ((v & 0xFF0000) >> 8) |
			((v & 0xFF000000) >> 24));
#endif
	}

	inline void Swap32(std::int32_t& v)
	{
		Swap32(*reinterpret_cast<std::uint32_t*>(&v));
	}

	inline void Swap32(float& v)
	{
		Swap32(*reinterpret_cast<std::uint32_t*>(&v));
	}

	inline void Swap64(std::uint64_t& v)
	{
#ifdef _WIN32
		v = _byteswap_uint64(v);
#elif __GNUC__
		v = __builtin_bswap64(v);
#else
		v = (v & 0x00000000FFFFFFFF) << 32 | (v & 0xFFFFFFFF00000000) >> 32;
		v = (v & 0x0000FFFF0000FFFF) << 16 | (v & 0xFFFF0000FFFF0000) >> 16;
		v = (v & 0x00FF00FF00FF00FF) << 8 | (v & 0xFF00FF00FF00FF00) >> 8;
#endif
	}

	inline void Swap64(std::int64_t& v)
	{
		Swap64(*reinterpret_cast<std::uint64_t*>(&v));
	}

	inline void Swap64(double& v)
	{
		Swap64(*reinterpret_cast<std::uint64_t*>(&v));
	}

	template<typename T>
	using EndianSwap_t = decltype(std::declval<T&>().EndianSwap());

	template<typename T>
	constexpr bool HasEndianSwapFunction = is_detected_v<EndianSwap_t, T>;

	template<typename T>
	using EndianSwapTwoWay_t = decltype(std::declval<T&>().EndianSwap(true));

	template<typename T>
	constexpr bool HasEndianSwapTwoWayFunction =
		is_detected_v<EndianSwapTwoWay_t, T>;

	template<typename T>
	using EndianSwapRecursive_t = decltype(std::declval
		<T&>().EndianSwapRecursive());

	template<typename T>
	constexpr bool HasEndianSwapRecursiveFunction =
		is_detected_v<EndianSwapRecursive_t, T>;

	template<typename T>
	using EndianSwapRecursiveTwoWay_t = decltype(std::declval
		<T&>().EndianSwapRecursive(true));

	template<typename T>
	constexpr bool HasEndianSwapRecursiveTwoWayFunction =
		is_detected_v<EndianSwapRecursiveTwoWay_t, T>;

	template<typename T>
	inline void Swap(T& value)
	{
		if constexpr (HasEndianSwapFunction<T>)
		{
			value.EndianSwap();
		}
	}

	template<>
	inline void Swap<std::uint16_t>(std::uint16_t& value)
	{
		Swap16(value);
	}

	template<>
	inline void Swap<std::int16_t>(std::int16_t& value)
	{
		Swap16(value);
	}

	template<>
	inline void Swap<std::uint32_t>(std::uint32_t& value)
	{
		Swap32(value);
	}

	template<>
	inline void Swap<std::int32_t>(std::int32_t& value)
	{
		Swap32(value);
	}

	template<>
	inline void Swap<float>(float& value)
	{
		Swap32(value);
	}

	template<>
	inline void Swap<std::uint64_t>(std::uint64_t& value)
	{
		Swap64(value);
	}

	template<>
	inline void Swap<std::int64_t>(std::int64_t& value)
	{
		Swap64(value);
	}

	template<>
	inline void Swap<double>(double& value)
	{
		Swap64(value);
	}

	template<typename T, typename... Args>
	inline void Swap(T& value, Args&... args)
	{
		Swap(value);
		Swap(args...);
	}

	template<typename T>
	inline void SwapTwoWay(bool isBigEndian, T& value)
	{
		if constexpr (HasEndianSwapTwoWayFunction<T>)
		{
			value.EndianSwap(isBigEndian);
		}
		else
		{
			Swap(value);
		}
	}

	template<typename T, typename... Args>
	inline void SwapTwoWay(bool isBigEndian, T& value, Args&... args)
	{
		SwapTwoWay(isBigEndian, value);
		SwapTwoWay(isBigEndian, args...);
	}

	template<typename T>
	inline void SwapRecursive(T& value)
	{
		if constexpr (HasEndianSwapRecursiveFunction<T>)
		{
			value.EndianSwapRecursive();
		}
		else
		{
			Swap(value);
		}
	}

	template<typename T, typename... Args>
	inline void SwapRecursive(T& value, Args&... args)
	{
		SwapRecursive(value);
		SwapRecursive(args...);
	}

	template<typename T>
	inline void SwapRecursiveTwoWay(bool isBigEndian, T& value)
	{
		if constexpr (HasEndianSwapRecursiveTwoWayFunction<T>)
		{
			value.EndianSwapRecursive(isBigEndian);
		}
		else
		{
			SwapRecursive(value);
		}
	}

	template<typename T, typename... Args>
	inline void SwapRecursiveTwoWay(bool isBigEndian, T& value, Args&... args)
	{
		SwapRecursiveTwoWay(isBigEndian, value);
		SwapRecursiveTwoWay(isBigEndian, args...);
	}
}
