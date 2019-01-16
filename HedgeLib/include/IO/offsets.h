#ifndef HOFFSETS_H_INCLUDED
#define HOFFSETS_H_INCLUDED
#include "endian.h"
#include "reflect.h"
#include "file.h"
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

#ifdef x86
#define HEDGELIB_OFFSET32_TYPE std::uint32_t
#elif x64
#define HEDGELIB_OFFSET32_TYPE std::int32_t
#endif

	template<typename DataType>
	struct OffsetBase<HEDGELIB_OFFSET32_TYPE, DataType>
	{
	protected:
		HEDGELIB_OFFSET32_TYPE o = 0;

	public:
		constexpr OffsetBase() = default;
		constexpr OffsetBase(const HEDGELIB_OFFSET32_TYPE offset) : o(offset) {}
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
			o = static_cast<HEDGELIB_OFFSET32_TYPE>(((
				origin + static_cast<std::uint32_t>(o)) -
				reinterpret_cast<std::uintptr_t>(this)));
#endif
		}

		inline void Set(const DataType* ptr)
		{
#ifdef x86
			o = static_cast<HEDGELIB_OFFSET32_TYPE>(
				reinterpret_cast<std::uintptr_t>(ptr));
#elif x64
			std::intptr_t t = reinterpret_cast<std::intptr_t>(this);
			std::intptr_t p = reinterpret_cast<std::intptr_t>(ptr);

			o = static_cast<HEDGELIB_OFFSET32_TYPE>(p - t);
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

		constexpr operator OffsetType() const noexcept
		{
			return this->o; // TODO: Should we have this?
		}

		inline operator DataType*() const noexcept
		{
			return this->Get();
		}

		inline DataType& operator* () const noexcept
		{
			return *(this->Get());
		}

		inline DataType* operator-> () const noexcept
		{
			return this->Get();
		}

		inline DataType& operator[] (const int index) const noexcept
		{
			return (this->Get())[index];
		}

		inline void EndianSwap()
		{
			if constexpr (!isArray)
			{
				HedgeLib::IO::Endian::SwapRecursive(
					this->o, *(this->Get()));
			}

			// TODO: Make an array variant!!
		}

		inline void WriteOffset(const HedgeLib::IO::File& file,
			const long origin, const std::uintptr_t endPtr, long eof,
			std::vector<std::uint32_t>& offsets) const
		{
			if constexpr (!isArray)
			{
				// Seek to the offset
				long offPos = (eof - static_cast<long>((
					endPtr - reinterpret_cast<std::uintptr_t>(this))));

				eof = file.Tell();
				file.Seek(offPos);

				// Fix it
				OffsetType off = static_cast<OffsetType>(eof - origin);
				file.Write(&off, sizeof(off), 1);

				// Add it to the list of offsets
				offsets.push_back(static_cast<std::uint32_t>(offPos - origin));

				// Write object pointed to by offset
				file.Seek(eof);
				HedgeLib::WriteRecursive(file, origin, endPtr,
					eof, offsets, *(this->Get()));
			}

			// TODO: Make an array variant!!
		}
	};

	template<typename DataType>
	using DataOffset32 = DataOffset<HEDGELIB_OFFSET32_TYPE, DataType, false>;

	template<typename DataType>
	using DataOffset64 = DataOffset<std::uint64_t, DataType, false>;

	template<typename DataType>
	using ArrOffset32 = DataOffset<HEDGELIB_OFFSET32_TYPE, DataType, true>;

	template<typename DataType>
	using ArrOffset64 = DataOffset<std::uint64_t, DataType, true>;
}
#endif