#ifndef HREFLECT_H_INCLUDED
#define HREFLECT_H_INCLUDED
#include "helpers.h"
#include "IO/endian.h"
#include "IO/file.h"
#include <cstdint>
#include <vector>
#include <utility>

#define CUSTOM_ENDIAN_SWAP inline void EndianSwap()
#define CUSTOM_ENDIAN_SWAP_RECURSIVE inline void EndianSwapRecursive()
#define CUSTOM_ENDIAN_SWAP_TWOWAY inline void EndianSwap(bool isBigEndian)
#define CUSTOM_ENDIAN_SWAP_RECURSIVE_TWOWAY inline void EndianSwapRecursive(bool isBigEndian)

#define ENDIAN_SWAP_OBJECT(...) CUSTOM_ENDIAN_SWAP_TWOWAY {\
	HedgeLib::IO::Endian::SwapTwoWay(isBigEndian, __VA_ARGS__); }

#define ENDIAN_SWAP_RECURSIVE(...) CUSTOM_ENDIAN_SWAP_RECURSIVE_TWOWAY {\
	HedgeLib::IO::Endian::SwapRecursiveTwoWay(isBigEndian, __VA_ARGS__); }

#define ENDIAN_SWAP(...) ENDIAN_SWAP_OBJECT(__VA_ARGS__)\
	ENDIAN_SWAP_RECURSIVE(__VA_ARGS__)

namespace HedgeLib::Reflect
{
	template<typename T>
	using Write_t = decltype(std::declval<T&>().Write(
		HedgeLib::IO::File(), 0, nullptr));

	template<typename T>
	constexpr bool HasWriteFunction = is_detected_v<Write_t, T>;

	template<typename T>
	using WriteChildren_t = decltype(std::declval<T&>().WriteChildren(
		HedgeLib::IO::File(), 0, nullptr));

	template<typename T>
	constexpr bool HasWriteChildrenFunction = is_detected_v<WriteChildren_t, T>;

	template<typename T>
	using WriteOffset_t = decltype(std::declval<T&>().WriteOffset(
		HedgeLib::IO::File(), 0, 0, 0, nullptr));

	template<typename T>
	constexpr bool HasWriteOffsetFunction = is_detected_v<WriteOffset_t, T>;

	template<typename T>
	inline void WriteObject(const HedgeLib::IO::File& file,
		const long origin, const std::uintptr_t endPtr, long eof,
		HedgeLib::IO::OffsetTable* offsets, const T& value)
	{
		if constexpr (HasWriteOffsetFunction<T>)
		{
			value.WriteOffset(file, origin, endPtr, eof, offsets);
		}
		else
		{
			file.Write(&value);
		}
	}

	template<typename T>
	inline void WriteChildren(const HedgeLib::IO::File& file,
		const long origin, HedgeLib::IO::OffsetTable* offsets,
		const T& value)
	{
		if constexpr (HasWriteChildrenFunction<T>)
		{
			value.WriteChildren(file, origin, offsets);
		}
	}

	template<typename T>
	inline void WriteRecursive(const HedgeLib::IO::File& file,
		const long origin, const std::uintptr_t endPtr, long eof,
		HedgeLib::IO::OffsetTable* offsets, const T& value)
	{
		WriteObject(file, origin, endPtr, eof, offsets, value);
		WriteChildren(file, origin, offsets, value);
	}

	template<typename T, typename... Args>
	inline void WriteRecursive(const HedgeLib::IO::File& file,
		const long origin, const std::uintptr_t endPtr, long eof,
		HedgeLib::IO::OffsetTable* offsets,
		const T& value, const Args&... args)
	{
		WriteRecursive(file, origin, endPtr, eof, offsets, value);
		WriteRecursive(file, origin, endPtr, eof, offsets, args...);
	}

#define CUSTOM_OFFSETS inline void WriteChildren(\
	const HedgeLib::IO::File& file, const long origin,\
	HedgeLib::IO::OffsetTable* offsets) const

#define CUSTOM_WRITE_OFFSETS(endPtr, eof, ...) HedgeLib::Reflect::WriteRecursive(\
	file, origin, endPtr, eof, offsets, __VA_ARGS__)

#define WRITE_OFFSETS(...) CUSTOM_WRITE_OFFSETS(\
	reinterpret_cast<std::uintptr_t>(this + 1), file.Tell(), __VA_ARGS__)

#define OFFSETS(...) CUSTOM_OFFSETS\
	{\
		WRITE_OFFSETS(__VA_ARGS__);\
	}
}
#endif