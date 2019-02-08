#ifndef HOFFSETS_H_INCLUDED
#define HOFFSETS_H_INCLUDED
#include "endian.h"
#include "reflect.h"
#include "file.h"
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <cstddef>

namespace HedgeLib::IO
{
	using OffsetTable = std::vector<std::uint32_t>;

	template<typename OffsetType, typename DataType>
	struct OffsetBase
	{
	protected:
		OffsetType o = 0;

	public:
		constexpr OffsetBase() = default;
		constexpr OffsetBase(std::nullptr_t) : o(0) {}

		inline void Set(const DataType* ptr) noexcept;
		inline OffsetBase(const DataType* ptr) noexcept;

		inline DataType* Get() const noexcept;
		CUSTOM_ENDIAN_SWAP_RECURSIVE_TWOWAY;

		constexpr void Fix(const std::uintptr_t origin,
			const bool swapEndianness = false) noexcept;

		template<typename CastedType>
		inline CastedType* GetAs() const noexcept;
	};

#ifdef x64
	namespace detail
	{
		std::uint32_t Addx64Pointer(
			const std::uintptr_t ptr) noexcept;
		std::uintptr_t Getx64Pointer(
			const std::uint32_t index) noexcept;
		void Setx64Pointer(const std::uint32_t index,
			const std::uintptr_t ptr) noexcept;
		void Removex64Pointer(
			const std::uint32_t index) noexcept;
	}
#endif

	template<typename DataType>
	struct OffsetBase<std::uint32_t, DataType>
	{
	protected:
		std::uint32_t o = 0;

	public:
		constexpr OffsetBase() = default;
		constexpr OffsetBase(std::nullptr_t) : o(0) {}

		inline void Set(const DataType* ptr) noexcept
		{
#ifdef x86
			o = static_cast<std::uint32_t>(
				reinterpret_cast<std::uintptr_t>(ptr));
#elif x64
			if (o == 0)
			{
				o = detail::Addx64Pointer(reinterpret_cast
					<std::uintptr_t>(ptr));
			}
			else
			{
				detail::Setx64Pointer(o, reinterpret_cast
					<std::uintptr_t>(ptr));
			}
#endif
		}

		inline OffsetBase(const DataType* ptr) noexcept
		{
#ifdef x86
			Set(ptr);
#elif x64
			o = detail::Addx64Pointer(reinterpret_cast
				<std::uintptr_t>(ptr));
#endif
		}

#ifdef x64
		inline ~OffsetBase() noexcept
		{
			detail::Removex64Pointer(o);
		}

		void operator= (const OffsetBase& v) noexcept
		{
			if (v.o != 0)
			{
				o = detail::Addx64Pointer(
					detail::Getx64Pointer(v.o));
			}
		}
#endif

		inline DataType* Get() const noexcept
		{
#ifdef x86
			return reinterpret_cast<DataType*>(o);
#elif x64
			return reinterpret_cast<DataType*>(
				detail::Getx64Pointer(o));
#endif
		}

		CUSTOM_ENDIAN_SWAP_RECURSIVE_TWOWAY
		{
			HedgeLib::IO::Endian::SwapRecursiveTwoWay(
				isBigEndian, *(this->Get()));
		}

		constexpr void Fix(const std::uintptr_t origin,
			const bool swapEndianness = false) noexcept
		{
			if (swapEndianness)
				HedgeLib::IO::Endian::Swap32(o);

#ifdef x86
			o += origin;
#elif x64
			o = detail::Addx64Pointer(origin +
				static_cast<std::uintptr_t>(o));
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
		constexpr OffsetBase(std::nullptr_t) : o(0) {}

		inline void Set(const DataType* ptr) noexcept
		{
			o = static_cast<std::uint64_t>(
				reinterpret_cast<std::uintptr_t>(ptr));
		}

		inline OffsetBase(const DataType* ptr) noexcept
		{
			Set(ptr);
		}

		inline DataType* Get() const noexcept
		{
			return reinterpret_cast<DataType*>(o);
		}

		CUSTOM_ENDIAN_SWAP_RECURSIVE_TWOWAY
		{
			HedgeLib::IO::Endian::SwapRecursiveTwoWay(
				isBigEndian, *(this->Get()));
		}

		constexpr void Fix(const std::uintptr_t origin,
			const bool swapEndianness = false) noexcept
		{
			if (swapEndianness)
				HedgeLib::IO::Endian::Swap64(o);

			o += origin;
		}

		template<typename CastedType>
		inline CastedType* GetAs() const noexcept
		{
			return reinterpret_cast<CastedType*>(o);
		}
	};

	// Foward-declarations for some BINA stuff
	namespace BINA
	{
		class BINAStringTable;

		template<typename T>
		inline void WriteObjectBINA(const File& file,
			const long origin, const std::uintptr_t endPtr, long eof,
			OffsetTable* offsets, BINAStringTable* stringTable, const T& value);

		template<typename T>
		inline void WriteChildrenBINA(const File& file,
			const long origin, OffsetTable* offsets,
			BINAStringTable* stringTable, const T& value);

		template<typename T>
		inline void WriteRecursiveBINA(const File& file,
			const long origin, const std::uintptr_t endPtr, long eof,
			OffsetTable* offsets, BINAStringTable* stringTable, const T& value);
	}

	template<typename OffsetType, typename DataType>
	struct DataOffset : public OffsetBase<OffsetType, DataType>
	{
		constexpr DataOffset() = default;
		constexpr DataOffset(std::nullptr_t) :
			OffsetBase<OffsetType, DataType>(nullptr) {}
		constexpr DataOffset(const DataType* ptr) :
			OffsetBase<OffsetType, DataType>(ptr) {}

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

		inline void FixOffsetRel(const File& file,
			const long origin, const std::uintptr_t endPtr, long& eof,
			OffsetTable* offsets) const
		{
			// Compute offset position
			long offPos = (eof - static_cast<long>((
				endPtr - reinterpret_cast<std::uintptr_t>(this))));

			eof = file.Tell();

			// Fix the offset
			file.FixOffsetNoEOFSeek(offPos, static_cast
				<OffsetType>(eof - origin), *offsets);

			// Seek to end of file for future writing
			file.Seek(eof);
		}

		inline void WriteOffset(const File& file,
			const long origin, const std::uintptr_t endPtr, long eof,
			OffsetTable* offsets) const
		{
			// Fix offset and write object pointed to by offset
			FixOffsetRel(file, origin, endPtr, eof, offsets);
			HedgeLib::Reflect::WriteRecursive(file, origin, endPtr,
				eof, offsets, *(this->Get()));
		}

		inline void WriteOffsetBINA(const File& file,
			const long origin, const std::uintptr_t endPtr, long eof,
			OffsetTable* offsets, BINA::BINAStringTable* stringTable) const
		{
			// Fix offset and write object pointed to by offset
			FixOffsetRel(file, origin, endPtr, eof, offsets);
			BINA::WriteRecursiveBINA(file, origin, endPtr,
				eof, offsets, stringTable, *(this->Get()));
		}
	};

	template<typename DataType>
	using DataOffset32 = DataOffset<std::uint32_t, DataType>;

	template<typename DataType>
	using DataOffset64 = DataOffset<std::uint64_t, DataType>;

	using StringOffset32 = DataOffset32<char>;
	using StringOffset64 = DataOffset64<char>;

	template<typename OffsetType, typename DataType, typename CountType>
	struct ArrOffset
	{
	protected:
		CountType count;
		DataOffset<OffsetType, DataType> o;

	public:
		constexpr ArrOffset() = default;
		constexpr ArrOffset(std::nullptr_t) :
			count(0), o(nullptr) {}
		constexpr ArrOffset(const DataType* ptr) :
			count(0), o(ptr) {}

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

		CUSTOM_ENDIAN_SWAP
		{
			HedgeLib::IO::Endian::Swap(count);
		}

		CUSTOM_ENDIAN_SWAP_RECURSIVE_TWOWAY
		{
			if (isBigEndian)
				HedgeLib::IO::Endian::Swap(count);

			for (CountType i = 0; i < count; ++i)
			{
				HedgeLib::IO::Endian::SwapRecursiveTwoWay(
					isBigEndian, operator[](static_cast<int>(i)));
			}

			if (!isBigEndian)
				HedgeLib::IO::Endian::Swap(count);
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

		inline void FixOffsetRel(const File& file,
			const long origin, const std::uintptr_t endPtr, long& eof,
			OffsetTable* offsets) const
		{
			o.FixOffsetRel(file, origin, endPtr, eof, offsets);
		}

		inline void WriteOffset(const File& file,
			const long origin, const std::uintptr_t endPtr, long eof,
			OffsetTable* offsets) const
		{
			o.FixOffsetRel(file, origin, endPtr, eof, offsets);
			for (CountType i = 0; i < count; ++i)
			{
				HedgeLib::Reflect::WriteObject(file, origin,
					endPtr, eof, offsets, (o.Get())[i]);
			}

			for (CountType i = 0; i < count; ++i)
			{
				HedgeLib::Reflect::WriteChildren(
					file, origin, offsets, (o.Get())[i]);
			}
		}

		inline void WriteOffsetBINA(const File& file,
			const long origin, const std::uintptr_t endPtr, long eof,
			OffsetTable* offsets, BINA::BINAStringTable* stringTable) const
		{
			o.FixOffsetRel(file, origin, endPtr, eof, offsets);
			for (CountType i = 0; i < count; ++i)
			{
				BINA::WriteObjectBINA(file, origin, endPtr, eof,
					offsets, stringTable, (o.Get())[i]);
			}

			for (CountType i = 0; i < count; ++i)
			{
				BINA::WriteChildrenBINA(file, origin, offsets,
					stringTable, (o.Get())[i]);
			}
		}
	};

	template<typename DataType, typename CountType = std::uint32_t>
	using ArrOffset32 = ArrOffset<std::uint32_t, DataType, CountType>;

	template<typename DataType, typename CountType = std::uint64_t>
	using ArrOffset64 = ArrOffset<std::uint64_t, DataType, CountType>;
}
#endif