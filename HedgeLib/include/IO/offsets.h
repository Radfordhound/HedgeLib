#ifndef HOFFSETS_H_INCLUDED
#define HOFFSETS_H_INCLUDED
#include "endian.h"
#include <cstdint>
#include <stdexcept>
#include <cstddef>

namespace HedgeLib::IO
{
	template<typename OffsetType, typename DataType>
	struct OffsetBase
	{
	protected:
		OffsetType o = 0;

	public:
		constexpr OffsetBase() = default;
		constexpr OffsetBase(const OffsetType offset) : o(offset) {}
		constexpr OffsetBase(std::nullptr_t) : o(0) {}

		inline DataType* Get() const noexcept;
		constexpr void Fix(const std::uintptr_t origin) noexcept;
		inline void Set(const DataType* ptr);

		template<typename CastedType>
		inline CastedType* GetAs() const noexcept;
	};

	template<typename DataType>
	struct OffsetBase<std::uint32_t, DataType>
	{
	protected:
		std::uint32_t o = 0;

	public:
		constexpr OffsetBase() = default;
		constexpr OffsetBase(const std::uint32_t offset) : o(offset) {}
		constexpr OffsetBase(std::nullptr_t) : o(0) {}

		inline DataType* Get() const noexcept
		{
#ifdef x86
			return reinterpret_cast<DataType*>(o);
#elif x64
			return reinterpret_cast<DataType*>(reinterpret_cast
				<const std::uintptr_t>(this) + o);
#endif
		}

		constexpr void Fix(const std::uintptr_t origin) noexcept
		{
#ifdef x86
			o += origin;
#elif x64
			o = static_cast<std::uint32_t>(((origin + o) -
				reinterpret_cast<std::uintptr_t>(this)));
#endif
		}

		inline void Set(const DataType* ptr)
		{
#ifdef x86
			o = static_cast<std::uint32_t>(
				reinterpret_cast<std::uintptr_t>(ptr));
#elif x64
			std::uintptr_t t = reinterpret_cast<std::uintptr_t>(this);
			std::uintptr_t p = reinterpret_cast<std::uintptr_t>(ptr);

			if (p < t) // TODO: Use signed intptr instead?
				throw std::runtime_error("Cannot store negative values in offset!");

			o = static_cast<std::uint32_t>(p - t);
#endif
		}

		template<typename CastedType>
		inline CastedType* GetAs() const noexcept
		{
			return reinterpret_cast<CastedType*>(Get());
		}
	};

	template<typename DataType>
	struct OffsetBase<std::uint64_t, DataType>
	{
	protected:
		std::uint64_t o = 0;

	public:
		constexpr OffsetBase() = default;
		constexpr OffsetBase(const std::uint64_t offset) : o(offset) {}
		constexpr OffsetBase(std::nullptr_t) : o(0) {}

		inline DataType* Get() const noexcept
		{
			return reinterpret_cast<DataType*>(o);
		}

		constexpr void Fix(const std::uintptr_t origin) noexcept
		{
			o += origin;
		}

		inline void Set(const DataType* ptr)
		{
			o = static_cast<std::uint64_t>(
				reinterpret_cast<std::uintptr_t>(ptr));
		}

		template<typename CastedType>
		inline CastedType* GetAs() const noexcept
		{
			return reinterpret_cast<CastedType*>(o);
		}
	};

	template<typename OffsetType, typename DataType, bool isArray = false>
	struct DataOffset : public OffsetBase<OffsetType, DataType>
	{
		constexpr DataOffset() = default;
		constexpr DataOffset(const OffsetType offset) :
			OffsetBase<OffsetType, DataType>(offset) {}
		constexpr DataOffset(std::nullptr_t) :
			OffsetBase<OffsetType, DataType>(nullptr) {}

		inline DataOffset(const DataType* ptr)
		{
			Set(ptr);
		}

		constexpr operator OffsetType() const noexcept
		{
			return o; // TODO: Should we have this?
		}

		inline operator DataType*() const noexcept
		{
			return Get();
		}

		inline DataType& operator* () const noexcept
		{
			return *Get();
		}

		inline DataType* operator-> () const noexcept
		{
			return Get();
		}

		inline DataType& operator[] (const int index) const noexcept
		{
			return (Get())[index];
		}

		inline void EndianSwap()
		{
			if constexpr (!isArray)
			{
				HedgeLib::IO::Endian::SwapRecursive(o, *Get());
			}

			// TODO: Make an array variant!!
		}
	};

	template<typename DataType>
	using DataOffset32 = DataOffset<std::uint32_t, DataType, false>;

	template<typename DataType>
	using DataOffset64 = DataOffset<std::uint64_t, DataType, false>;

	template<typename DataType>
	using ArrOffset32 = DataOffset<std::uint32_t, DataType, true>;

	template<typename DataType>
	using ArrOffset64 = DataOffset<std::uint64_t, DataType, true>;
}
#endif