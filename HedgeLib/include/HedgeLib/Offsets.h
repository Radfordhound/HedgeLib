#pragma once
#include "Endian.h"
#include <vector>
#include <stdexcept>
#include <cstddef>

namespace hl
{
    template<typename T>
    class DataOffset32
    {
        std::uint32_t off;

        // For any architecture
        template<std::size_t PtrSize = sizeof(std::uintptr_t)>
        inline typename std::enable_if<(PtrSize > 4 &&
            sizeof(const T*) > 4), const T*>::type INGet() const
        {
            return (off) ? reinterpret_cast<const T*>(
                reinterpret_cast<std::intptr_t>(&off) +
                static_cast<std::intptr_t>(static_cast<
                std::int32_t>(off))) : nullptr;
        }

        template<std::size_t PtrSize = sizeof(std::uintptr_t)>
        inline typename std::enable_if<(PtrSize > 4 &&
            sizeof(T*) > 4), T*>::type INGet()
        {
            return (off) ? reinterpret_cast<T*>(
                reinterpret_cast<std::intptr_t>(&off) +
                static_cast<std::intptr_t>(static_cast<
                std::int32_t>(off))) : nullptr;
        }

        template<std::size_t PtrSize = sizeof(std::uintptr_t)>
        inline typename std::enable_if<(PtrSize > 4 &&
            sizeof(T*) > 4)>::type INSet(std::uintptr_t ptr)
        {
            // Make offset relative to itself
            std::intptr_t addr = reinterpret_cast<std::intptr_t>(&off);
            if (ptr == addr)
            {
                throw std::runtime_error(
                    "Cannot have a 32-bit offset that points to itself on this platform.");
            }

            addr = (static_cast<std::intptr_t>(ptr) - addr);

            if (addr > INT32_MAX || addr < INT32_MIN)
            {
                throw std::overflow_error(
                    "Cannot have a 32-bit offset that can't fit within 32-bits.");
            }

            off = static_cast<std::uint32_t>(addr);
        }

        // For 32-bit architecture
        template<std::size_t PtrSize = sizeof(std::uintptr_t)>
        inline typename std::enable_if<(PtrSize <= 4 &&
            sizeof(const T*) <= 4), const T*>::type INGet() const
        {
            static_assert((sizeof(std::uintptr_t) <= 4 && sizeof(const T*) <= 4),
                "Attempted to call 32-bit Get on unsupported architecture. This should not happen.");

            return reinterpret_cast<const T*>(
                static_cast<std::uintptr_t>(off));
        }

        template<std::size_t PtrSize = sizeof(std::uintptr_t)>
        inline typename std::enable_if<(PtrSize <= 4 &&
            sizeof(T*) <= 4), T*>::type INGet()
        {
            static_assert((sizeof(std::uintptr_t) <= 4 && sizeof(T*) <= 4),
                "Attempted to call 32-bit Get on unsupported architecture. This should not happen.");

            return reinterpret_cast<T*>(
                static_cast<std::uintptr_t>(off));
        }

        template<std::size_t PtrSize = sizeof(std::uintptr_t)>
        inline typename std::enable_if<(PtrSize <= 4 &&
            sizeof(T*) <= 4)>::type INSet(std::uintptr_t ptr)
        {
            static_assert((sizeof(std::uintptr_t) <= 4 && sizeof(T*) <= 4),
                "Attempted to call 32-bit Set on unsupported architecture. This should not happen.");

            off = static_cast<std::uint32_t>(ptr);
        }

    public:
        constexpr DataOffset32() = default;

        inline const T* Get() const
        {
            return INGet();
        }

        inline T* Get()
        {
            return INGet();
        }

        template<typename CastType>
        inline const CastType* GetAs() const
        {
            return reinterpret_cast<const CastType*>(Get());
        }

        template<typename CastType>
        inline CastType* GetAs()
        {
            return reinterpret_cast<CastType*>(Get());
        }

        inline void Set(std::uintptr_t ptr)
        {
            INSet(ptr);
        }

        inline void Set(T* ptr)
        {
            INSet(reinterpret_cast<uintptr_t>(ptr));
        }

        inline DataOffset32(std::uintptr_t ptr)
        {
            Set(ptr);
        }

        inline DataOffset32(T* ptr)
        {
            Set(ptr);
        }

        inline operator const T* () const
        {
            return Get();
        }

        inline operator T* ()
        {
            return Get();
        }

        inline const T* operator*() const
        {
            return Get();
        }

        inline T* operator*()
        {
            return Get();
        }

        inline const T* operator->() const
        {
            return Get();
        }

        inline T* operator->()
        {
            return Get();
        }

        inline void EndianSwapRecursive(bool isBigEndian)
        {
            T* ptr = Get();
            if (!ptr)
            {
                throw std::runtime_error(
                    "Attempted to endian-swap the value of a null pointer.");
            }

            SwapRecursive(isBigEndian, *ptr);
        }

        inline void Fix(const void* data, bool isBigEndian)
        {
            // Null pointers should remain null
            if (!off) return;

            // Endian swap offset
            if (isBigEndian) Swap(off);

            // Get absolute pointer
            std::uintptr_t absPtr = (reinterpret_cast<std::uintptr_t>(data) +
                static_cast<std::uintptr_t>(off));

            // Fix offset
            Set(absPtr);
        }
    };

    template<typename T>
    class DataOffset64
    {
        static_assert((sizeof(std::uintptr_t) <= 8 &&
            sizeof(T*) <= 8 && sizeof(const T*) <= 8),
            "Unsupported architecture.");

        std::uint64_t off;

    public:
        constexpr DataOffset64() = default;
        constexpr DataOffset64(std::uintptr_t ptr) :
            off(static_cast<std::uint64_t>(ptr)) {}

        inline DataOffset64(T* ptr) :
            DataOffset64(reinterpret_cast<std::uintptr_t>(ptr)) {}

        inline const T* Get() const
        {
            return reinterpret_cast<const T*>(
                static_cast<std::uintptr_t>(off));
        }

        inline T* Get()
        {
            return reinterpret_cast<T*>(
                static_cast<std::uintptr_t>(off));
        }

        template<typename CastType>
        inline const CastType* GetAs() const
        {
            return reinterpret_cast<const CastType*>(Get());
        }

        template<typename CastType>
        inline CastType* GetAs()
        {
            return reinterpret_cast<CastType*>(Get());
        }

        inline void Set(std::uintptr_t ptr)
        {
            off = static_cast<std::uint64_t>(ptr);
        }

        inline void Set(T* ptr)
        {
            Set(reinterpret_cast<std::uintptr_t>(ptr));
        }

        inline operator const T* () const
        {
            return Get();
        }

        inline operator T* ()
        {
            return Get();
        }

        inline const T* operator*() const
        {
            return Get();
        }

        inline T* operator*()
        {
            return Get();
        }

        inline const T* operator->() const
        {
            return Get();
        }

        inline T* operator->()
        {
            return Get();
        }

        inline void EndianSwapRecursive(bool isBigEndian)
        {
            T* ptr = Get();
            if (!ptr)
            {
                throw std::runtime_error(
                    "Attempted to endian-swap the value of a null pointer.");
            }

            SwapRecursive(isBigEndian, *ptr);
        }

        inline void Fix(const void* data, bool isBigEndian)
        {
            // Null pointers should remain null
            if (!off) return;

            // Endian swap offset
            if (isBigEndian) Swap(off);

            // Get absolute pointer
            std::uintptr_t absPtr = (reinterpret_cast<std::uintptr_t>(data) +
                static_cast<std::uintptr_t>(off));

            // Set offset to absolute pointer
            off = static_cast<std::uint64_t>(absPtr);
        }
    };

    // Strings
    using StringOffset32 = DataOffset32<char>;
    using StringOffset64 = DataOffset64<char>;

    // Arrays
    template<typename T, typename count_t, template<typename> typename offset_t>
    struct ArrayOffset
    {
        count_t Count;
        offset_t<T> Offset;

        constexpr ArrayOffset() = default;
        constexpr ArrayOffset(count_t count, std::uintptr_t* ptr) :
            Count(count), Offset(ptr) {}

        inline ArrayOffset(count_t count, T* ptr) :
            Count(count), Offset(ptr) {}

        inline const T* Get() const
        {
            return Offset.Get();
        }

        inline T* Get()
        {
            return Offset.Get();
        }

        inline void Set(std::uintptr_t ptr)
        {
            Offset.Set(ptr);
        }

        inline void Set(T* ptr)
        {
            Offset.Set(ptr);
        }

        inline operator const T* () const
        {
            return Offset.Get();
        }

        inline operator T* ()
        {
            return Offset.Get();
        }

        inline const T* operator->() const
        {
            return Offset.Get();
        }

        inline T* operator->()
        {
            return Offset.Get();
        }

        inline void EndianSwap()
        {
            Swap(Count);
        }

        inline void EndianSwapRecursive(bool isBigEndian)
        {
            if (isBigEndian) Swap(Count);

            T* ptr = Offset.Get();
            for (count_t i = 0; i < Count; ++i)
            {
                SwapRecursive(isBigEndian, ptr[i]);
            }

            if (!isBigEndian) Swap(Count);
        }
    };

    template<typename T>
    using ArrayOffset32 = ArrayOffset<T, std::uint32_t, DataOffset32>;

    template<typename T>
    using ArrayOffset64 = ArrayOffset<T, std::uint64_t, DataOffset64>;

    // Offset Table
    using OffsetTable = std::vector<long>;

    // Helper functions
    template<typename T, typename offset_t>
    inline const T* GetAbs(const void* baseAddress, offset_t offset)
    {
        return reinterpret_cast<const T*>(static_cast<const uint8_t*>(
            baseAddress) + offset);
    }

    template<typename T, typename offset_t>
    inline T* GetAbs(void* baseAddress, offset_t offset)
    {
        return reinterpret_cast<T*>(static_cast<uint8_t*>(
            baseAddress) + offset);
    }
}
