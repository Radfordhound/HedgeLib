#ifndef HREFLECT_H_INCLUDED
#define HREFLECT_H_INCLUDED
#include "helpers.h"
#include "IO/endian.h"
#include "IO/file.h"
#include <cstdint>
#include <vector>
#include <utility>

namespace HedgeLib
{
#define ENDIAN_SWAP(...) inline void EndianSwap()\
	{ HedgeLib::IO::Endian::SwapRecursive(__VA_ARGS__); }

	template<typename T>
	using WriteRecursive_t = decltype(std::declval<T&>().WriteRecursive(
		HedgeLib::IO::File(), 0, nullptr));

	template<typename T>
	constexpr bool HasWriteRecursiveFunction = is_detected_v<WriteRecursive_t, T>;

	template<typename T>
	using WriteOffset_t = decltype(std::declval<T&>().WriteOffset(
		HedgeLib::IO::File(), 0, 0, 0, nullptr));

	template<typename T>
	constexpr bool HasWriteOffsetFunction = is_detected_v<WriteOffset_t, T>;

	template<typename T>
	inline void WriteRecursive(const HedgeLib::IO::File& file,
		const long origin, const std::uintptr_t endPtr, long eof,
		std::vector<std::uint32_t>* offsets, const T& value)
	{
		if constexpr (HasWriteOffsetFunction<T>)
		{
			value.WriteOffset(file, origin, endPtr, eof, offsets);
		}
		else if constexpr (HasWriteRecursiveFunction<T>)
		{
			value.WriteRecursive(file, origin, offsets);
		}
		else
		{
			file.Write(&value, sizeof(value), 1);
		}
	}

	template<typename T, typename... Args>
	inline void WriteRecursive(const HedgeLib::IO::File& file,
		const long origin, const std::uintptr_t endPtr, long eof,
		std::vector<std::uint32_t>* offsets,
		const T& value, const Args&... args)
	{
		WriteRecursive(file, origin, endPtr, eof, offsets, value);
		WriteRecursive(file, origin, endPtr, eof, offsets, args...);
	}

#define CUSTOM_WRITE inline void WriteRecursive(\
	const HedgeLib::IO::File& file, const long origin,\
	std::vector<std::uint32_t>* offsets) const

#define WRITE_BEGIN CUSTOM_WRITE {\
	file.Write(this, sizeof(*this), 1);\
	long eof = file.Tell();

#define CUSTOM_WRITE_OFFSETS(endPtr, eof, ...) HedgeLib::WriteRecursive(\
	file, origin, endPtr, eof, offsets, __VA_ARGS__)

#define WRITE_OFFSETS(...) CUSTOM_WRITE_OFFSETS(\
	reinterpret_cast<std::uintptr_t>(this + 1), eof, __VA_ARGS__)

#define WRITE_END }

#define OFFSETS(...) WRITE_BEGIN\
	WRITE_OFFSETS(__VA_ARGS__);\
	WRITE_END
}
#endif