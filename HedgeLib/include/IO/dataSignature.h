#ifndef HSIGNATURE_H_INCLUDED
#define HSIGNATURE_H_INCLUDED
#include <cstdint>
#include <string_view>
#include <array>
#include <stdexcept>

namespace HedgeLib::IO
{
	struct DataSignature
	{
		// We use an array of 4 bytes instead of a
		// uint32_t to avoid issues with endianness
		std::array<std::uint8_t, 4> Data { 0, 0, 0, 0 };

		constexpr DataSignature() = default;
		constexpr DataSignature(const std::array<std::uint8_t, 4> v) noexcept : Data(v) {}
		constexpr DataSignature(const std::uint32_t v) noexcept : Data({
				static_cast<std::uint8_t>(v & 0xFF),
				static_cast<std::uint8_t>((v & 0xFF00) >> 8),
				static_cast<std::uint8_t>((v & 0xFF0000) >> 16),
				static_cast<std::uint8_t>((v & 0xFF000000) >> 24)
			}) {}

		constexpr DataSignature(const std::string_view v) : Data(ToArray(v)) {}
		constexpr DataSignature(const char* v) : Data(ToArray(v)) {}

		constexpr const std::uint8_t& operator[] (const int index) const noexcept
		{
			return Data[index];
		}

	private:
		static inline constexpr std::array<std::uint8_t, 4>
			ToArray(const std::string_view v)
		{
			if (v.size() < 4)
				throw std::logic_error("DataSignatures must be >= 4 bytes long");

			return {
				static_cast<std::uint8_t>(v[0]),
				static_cast<std::uint8_t>(v[1]),
				static_cast<std::uint8_t>(v[2]),
				static_cast<std::uint8_t>(v[3])
			};
		}
	};

	constexpr bool operator== (const DataSignature& a, const DataSignature& b) noexcept
	{
		return (a.Data[0] == b.Data[0] && a.Data[1] == b.Data[1] &&
			a.Data[2] == b.Data[2] && a.Data[3] == b.Data[3]);
	}

	constexpr bool operator!= (const DataSignature& a, const DataSignature& b) noexcept
	{
		return !(a == b);
	}
}
#endif