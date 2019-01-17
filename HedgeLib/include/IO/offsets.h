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
		inline void EndianSwap();

		constexpr void Fix(const std::uintptr_t origin,
			const bool swapEndianness = false) noexcept;

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

		inline void EndianSwap()
		{
			HedgeLib::IO::Endian::SwapRecursive(
				*(this->Get()));
		}

		constexpr void Fix(const std::uintptr_t origin,
			const bool swapEndianness = false) noexcept
		{
			if (swapEndianness)
				HedgeLib::IO::Endian::SwapRecursive(this->o);

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

		inline void EndianSwap()
		{
			HedgeLib::IO::Endian::SwapRecursive(
				*(this->Get()));
		}

		constexpr void Fix(const std::uintptr_t origin,
			const bool swapEndianness = false) noexcept
		{
			if (swapEndianness)
				HedgeLib::IO::Endian::SwapRecursive(this->o);

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

	template<typename OffsetType, typename DataType>
	struct DataOffset : public OffsetBase<OffsetType, DataType>
	{
		constexpr DataOffset() = default;
		constexpr DataOffset(const OffsetType offset) :
			OffsetBase<OffsetType, DataType>(offset) {}
		constexpr DataOffset(std::nullptr_t) :
			OffsetBase<OffsetType, DataType>(nullptr) {}

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

		constexpr OffsetType& GetInternalOffset() noexcept
		{
			return this->o;
		}

		inline void FixOffset(const HedgeLib::IO::File& file,
			const long origin, const std::uintptr_t endPtr, long& eof,
			std::vector<std::uint32_t>* offsets) const
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
			offsets->push_back(static_cast<std::uint32_t>(offPos - origin));

			// Seek to end of file for future writing
			file.Seek(eof);
		}

		inline void WriteOffset(const HedgeLib::IO::File& file,
			const long origin, const std::uintptr_t endPtr, long eof,
			std::vector<std::uint32_t>* offsets) const
		{
			// Fix offset and write object pointed to by offset
			FixOffset(file, origin, endPtr, eof, offsets);
			HedgeLib::WriteRecursive(file, origin, endPtr,
				eof, offsets, *(this->Get()));
		}
	};

	template<typename DataType>
	using DataOffset32 = DataOffset<HEDGELIB_OFFSET32_TYPE, DataType>;

	template<typename DataType>
	using DataOffset64 = DataOffset<std::uint64_t, DataType>;

	template<typename OffsetType, typename DataType, typename CountType>
	struct ArrOffset
	{
	protected:
		CountType count;
		DataOffset<OffsetType, DataType> o;

	public:
		constexpr ArrOffset() = default;
		constexpr ArrOffset(const OffsetType offset) :
			count(0), o(offset) {}
		constexpr ArrOffset(std::nullptr_t) :
			count(0), o(nullptr) {}

		inline operator DataType*() const noexcept
		{
			return o.Get();
		}

		inline DataType& operator* () const noexcept
		{
			return *(o.Get());
		}

		inline DataType* operator-> () const noexcept
		{
			return o.Get();
		}

		inline DataType& operator[] (const int index) const noexcept
		{
			return (o.Get())[index];
		}

		constexpr CountType Count() const noexcept
		{
			return count;
		}

		inline DataType* Get() const noexcept
		{
			return o.Get();
		}

		inline void EndianSwap()
		{
			HedgeLib::IO::Endian::SwapRecursive(count);
			for (CountType i = 0; i < count; ++i)
			{
				HedgeLib::IO::Endian::SwapRecursive(
					operator[](static_cast<int>(i)));
			}
		}

		constexpr void Fix(const std::uintptr_t origin,
			const bool swapEndianness = false) noexcept
		{
			o.Fix(origin, swapEndianness);
		}

		inline void Set(const DataType* ptr, const CountType count)
		{
			o.Set(ptr);
			this->count = count;
		}

		template<typename CastedType>
		inline CastedType* GetAs() const noexcept
		{
			return o.template GetAs<CastedType>();
		}

		inline void WriteOffset(const HedgeLib::IO::File& file,
			const long origin, const std::uintptr_t endPtr, long eof,
			std::vector<std::uint32_t>* offsets) const
		{
			o.FixOffset(file, origin, endPtr, eof, offsets);
			for (CountType i = 0; i < count; ++i)
			{
				HedgeLib::WriteRecursive(file, origin, endPtr,
					eof, offsets, (o.Get())[i]);
			}
		}
	};

	template<typename DataType, typename CountType = std::uint32_t>
	using ArrOffset32 = ArrOffset<HEDGELIB_OFFSET32_TYPE, DataType, CountType>;

	template<typename DataType, typename CountType = std::uint64_t>
	using ArrOffset64 = ArrOffset<std::uint64_t, DataType, CountType>;
}
#endif