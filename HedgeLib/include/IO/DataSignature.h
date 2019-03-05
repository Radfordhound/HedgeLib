#ifndef HSIGNATURE_H_INCLUDED
#define HSIGNATURE_H_INCLUDED
#include <cstdint>
#include <string_view>
#include <array>
#include <stdexcept>

namespace HedgeLib::IO
{
	namespace
	{
		inline constexpr std::array<std::uint8_t, 4>
			ToArray32(const std::string_view v)
		{
			if (v.size() < 4)
				throw std::logic_error("DataSignature32s must be >= 4 bytes long");

			return {
				static_cast<std::uint8_t>(v[0]),
				static_cast<std::uint8_t>(v[1]),
				static_cast<std::uint8_t>(v[2]),
				static_cast<std::uint8_t>(v[3])
			};
		}

		inline constexpr std::array<std::uint8_t, 8>
			ToArray64(const std::string_view v)
		{
			if (v.size() < 8)
				throw std::logic_error("DataSignature64s must be >= 8 bytes long");

			return {
				static_cast<std::uint8_t>(v[0]),
				static_cast<std::uint8_t>(v[1]),
				static_cast<std::uint8_t>(v[2]),
				static_cast<std::uint8_t>(v[3]),
				static_cast<std::uint8_t>(v[4]),
				static_cast<std::uint8_t>(v[5]),
				static_cast<std::uint8_t>(v[6]),
				static_cast<std::uint8_t>(v[7]),
			};
		}
	}

	struct DataSignature32
	{
		// We use an array of 4 bytes instead of a
		// uint32_t to avoid issues with endianness
		std::array<std::uint8_t, 4> Data { 0, 0, 0, 0 };

		constexpr DataSignature32() = default;
		constexpr DataSignature32(const std::array<std::uint8_t, 4> v) noexcept : Data(v) {}
		constexpr DataSignature32(const std::uint32_t v) noexcept : Data({
				static_cast<std::uint8_t>(v & 0xFF),
				static_cast<std::uint8_t>((v & 0xFF00) >> 8),
				static_cast<std::uint8_t>((v & 0xFF0000) >> 16),
				static_cast<std::uint8_t>((v & 0xFF000000) >> 24)
			}) {}

		constexpr DataSignature32(const std::string_view v) : Data(ToArray32(v)) {}
		constexpr DataSignature32(const char* v) : Data(ToArray32(v)) {}

		constexpr const std::uint8_t& operator[] (const int index) const noexcept
		{
			return Data[index];
		}
	};

	struct DataSignature64
	{
		// We use an array of 8 bytes instead of a
		// uint64_t to avoid issues with endianness
		std::array<std::uint8_t, 8> Data{ 0, 0, 0, 0, 0, 0, 0, 0 };

		constexpr DataSignature64() = default;
		constexpr DataSignature64(const std::array<std::uint8_t, 8> v) noexcept : Data(v) {}
		constexpr DataSignature64(const std::uint64_t v) noexcept : Data({
				static_cast<std::uint8_t>(v & 0xFF),
				static_cast<std::uint8_t>((v & 0xFF00) >> 8),
				static_cast<std::uint8_t>((v & 0xFF0000) >> 16),
				static_cast<std::uint8_t>((v & 0xFF000000) >> 24),
				static_cast<std::uint8_t>((v & 0xFF00000000) >> 32),
				static_cast<std::uint8_t>((v & 0xFF0000000000) >> 40),
				static_cast<std::uint8_t>((v & 0xFF000000000000) >> 48),
				static_cast<std::uint8_t>((v & 0xFF00000000000000) >> 56)
			}) {}

		constexpr DataSignature64(const std::string_view v) : Data(ToArray64(v)) {}
		constexpr DataSignature64(const char* v) : Data(ToArray64(v)) {}

		constexpr const std::uint8_t& operator[] (const int index) const noexcept
		{
			return Data[index];
		}
	};

	constexpr bool operator== (const DataSignature32& a, const DataSignature32& b) noexcept
	{
		// TODO: Maybe cast a.Data/b.Data to a std::uint32* and compare like that instead?
		return (a.Data[0] == b.Data[0] && a.Data[1] == b.Data[1] &&
			a.Data[2] == b.Data[2] && a.Data[3] == b.Data[3]);
	}

	constexpr bool operator!= (const DataSignature32& a, const DataSignature32& b) noexcept
	{
		return !(a == b);
	}

	constexpr bool operator== (const DataSignature64& a, const DataSignature64& b) noexcept
	{
		// TODO: Maybe cast a.Data/b.Data to a std::uint64* and compare like that instead?
		return (a.Data[0] == b.Data[0] && a.Data[1] == b.Data[1] &&
			a.Data[2] == b.Data[2] && a.Data[3] == b.Data[3] &&
			a.Data[4] == b.Data[4] && a.Data[5] == b.Data[5] &&
			a.Data[6] == b.Data[6] && a.Data[7] == b.Data[7]);
	}

	constexpr bool operator!= (const DataSignature64& a, const DataSignature64& b) noexcept
	{
		return !(a == b);
	}
}
#endif