#ifndef HOFFSETS_H_INCLUDED
#define HOFFSETS_H_INCLUDED
#include <cstdint>
#include <stdexcept>
#include <cstddef>

namespace HedgeLib::IO
{
	template<typename OffsetType>
	struct DataOffset
	{
	protected:
		OffsetType o = 0;

	public:
		constexpr DataOffset() = default;
		constexpr DataOffset(const OffsetType offset) : o(offset) {}
		constexpr DataOffset(std::nullptr_t) : o(0) {}

		constexpr operator OffsetType() const noexcept
		{
			return o;
		}
	};

	template<typename DataType>
	struct DataOffset32 : public DataOffset<std::uint32_t>
	{
		constexpr DataOffset32() = default;
		constexpr DataOffset32(const std::uint32_t offset) :
			DataOffset<std::uint32_t>(offset) {}
		constexpr DataOffset32(std::nullptr_t) :
			DataOffset<std::uint32_t>(nullptr) {}

		DataOffset32(const DataType* ptr) { Set(ptr); }

		template<typename CastedType>
		inline CastedType* GetAs() const noexcept
		{
			return reinterpret_cast<CastedType*>(Get());
		}

		inline DataType* Get() const noexcept
		{
#ifdef x86
			return reinterpret_cast<DataType*>(o);
#elif x64
			return reinterpret_cast<DataType*>(reinterpret_cast
				<const std::uintptr_t>(this) + o);
#endif
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
    		return Get()[index];
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

		constexpr void Set(const DataType* ptr)
		{
#ifdef x86
			o = static_cast<std::uint32_t>(
				reinterpret_cast<std::uintptr_t>(ptr));
#elif x64
			std::uintptr_t t = reinterpret_cast<std::uintptr_t>(this);
			std::uintptr_t p = reinterpret_cast<std::uintptr_t>(ptr);

			if (p < t) // TODO: There has to be some better way to do this?
				throw std::runtime_error("Cannot store negative values in offset!");

			o = static_cast<std::uint32_t>(p - t);
#endif
		}
	};

	template<typename DataType>
	struct DataOffset64 : public DataOffset<std::uint64_t>
	{
		constexpr DataOffset64() = default;
		constexpr DataOffset64(const std::uint64_t offset) :
			DataOffset<std::uint64_t>(offset) {}
		constexpr DataOffset64(std::nullptr_t) :
			DataOffset<std::uint64_t>(nullptr) {}

		DataOffset64(const DataType* ptr) { Set(ptr); }

		template<typename CastedType>
		inline CastedType* GetAs() const noexcept
		{
			return reinterpret_cast<CastedType*>(o);
		}

		inline DataType* Get() const noexcept
		{
			return reinterpret_cast<DataType*>(o);
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
    		return Get()[index];
		}

		constexpr void Fix(const std::uintptr_t origin) noexcept
		{
			o += origin;
		}

		constexpr void Set(const DataType* ptr)
		{
			o = static_cast<std::uint64_t>(
				reinterpret_cast<std::uintptr_t>(ptr));
		}
	};

	template<typename DataType>
	struct ArrOffset32 : public DataOffset<std::uint32_t>
	{
		constexpr ArrOffset32() = default;
		constexpr ArrOffset32(const std::uint32_t offset) :
			DataOffset<std::uint32_t>(offset) {}
		constexpr ArrOffset32(std::nullptr_t) :
			DataOffset<std::uint32_t>(nullptr) {}

		inline DataType* Get() const noexcept
		{
#ifdef x86
			return reinterpret_cast<DataType*>(o);
#elif x64
			return reinterpret_cast<DataType*>(reinterpret_cast
				<const std::uintptr_t>(this) + o);
#endif
		}
		
		inline operator DataType*() const noexcept
		{
			return &(Get()[0]);
		}

		inline DataType& operator* () const noexcept
		{
			return Get()[0];
		}

		inline DataType* operator-> () const noexcept
		{
			return &(Get()[0]);
		}

		inline DataType& operator[] (const int index) const noexcept
		{
    		return (Get())[index];
		}

		constexpr void Fix(const std::uintptr_t origin) noexcept
		{
#ifdef x86
			o += origin;
#elif x64
			o = ((origin + o) - reinterpret_cast<std::uintptr_t>(this));
#endif
		}
	};

	template<typename DataType>
	struct ArrOffset64 : public DataOffset<std::uint64_t>
	{
		constexpr ArrOffset64() = default;
		constexpr ArrOffset64(const std::uint64_t offset) :
			DataOffset<std::uint64_t>(offset) {}
		constexpr ArrOffset64(std::nullptr_t) :
			DataOffset<std::uint64_t>(nullptr) {}

		inline DataType** Get() const noexcept
		{
			return reinterpret_cast<DataType**>(o);
		}

		inline operator DataType*() const noexcept
		{
			return &(Get()[0]);
		}

		inline DataType& operator* () const noexcept
		{
			return Get()[0];
		}

		inline DataType* operator-> () const noexcept
		{
			return &(Get()[0]);
		}

		inline DataType& operator[] (const int index) const noexcept
		{
    		return *(Get()[index]);
		}

		constexpr void Fix(const std::uintptr_t origin) noexcept
		{
			o += origin;
		}
	};
}
#endif