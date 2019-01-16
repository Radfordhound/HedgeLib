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
		std::vector<std::uint32_t>* offsets, T& value)
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
		T& value, Args&... args)
	{
		WriteRecursive(file, origin, endPtr, eof, offsets, value);
		WriteRecursive(file, origin, endPtr, eof, offsets, args...);
	}

#define OFFSETS(...) inline void WriteRecursive(\
	const HedgeLib::IO::File& file, const long origin,\
	std::vector<std::uint32_t>* offsets) const\
	{\
		file.Write(this, sizeof(*this), 1);\
		HedgeLib::WriteRecursive(file, origin, reinterpret_cast\
			<std::uintptr_t>(this + 1), file.Tell(),\
			offsets, __VA_ARGS__);\
	}
}
#endif