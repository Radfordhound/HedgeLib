#pragma once
#include "HedgeLib/Direct/Offsets.h"
#include "Helpers.h"
#include <cstdlib>
#include <vector>

struct hl_OffsetTable : public std::vector<long>
{
    using std::vector<long>::vector;
};

namespace HedgeLib
{
#ifdef x64
    template<typename T>
    using x64AddAbsPtrs32_t = decltype(std::declval<T&>().x64AddAbsPtrs32());

    template<typename T>
    constexpr bool Hasx64AddAbsPtrs32Function = is_detected_v<x64AddAbsPtrs32_t, T>;

    template<typename T>
    using x64RemoveAbsPtrs32_t = decltype(std::declval<T&>().x64RemoveAbsPtrs32());

    template<typename T>
    constexpr bool Hasx64RemoveAbsPtrs32Function = is_detected_v<x64RemoveAbsPtrs32_t, T>;
#endif

    using OffsetTable = hl_OffsetTable;

    template<typename T, typename OffsetType>
    inline T* GetAbs(void* baseAddress, OffsetType offset)
    {
        return reinterpret_cast<T*>(static_cast<uint8_t*>(
            baseAddress) + offset);
    }

    // TODO: Embed DirectType within T??
    template<typename T, typename DirectType>
    class Array
    {
        DirectType* direct;
        T* managed;
        std::size_t count;
        bool deleteDirect = false;

    public:
        inline Array(DirectType* arr, std::size_t count) :
            direct(arr), count(count),
            managed(static_cast<T*>(std::malloc(
                sizeof(T) * count)))
        {
            // Placement new on each object within the managed array
            for (std::size_t i = 0; i < count; ++i)
            {
                new (managed + i) T(&direct[i]);
            }
        }

        inline Array(std::size_t count) :
            Array(new DirectType[count], count)
        {
            deleteDirect = true;

#ifdef x64
            // Add 32-bit absolute pointers if necessary
            if constexpr (Hasx64AddAbsPtrs32Function<DirectType>)
            {
                for (std::size_t i = 0; i < count; ++i)
                {
                    direct[i].x64AddAbsPtrs32();
                }
            }
#endif
        }

        inline Array(const hl_ArrOff32& arr) :
            Array(HL_GETPTR32(DirectType, arr.Offset), arr.Count) {}

        inline Array(const hl_ArrOff64& arr) :
            Array(HL_GETPTR64(DirectType, arr.Offset), arr.Count) {}

        inline ~Array()
        {
            // Free managed objects
            for (std::size_t i = 0; i < count; ++i)
            {
                managed[i].~T();
            }

            std::free(managed);

            // Free direct objects
            if (deleteDirect)
            {
#ifdef x64
                // Remove 32-bit absolute pointers if necessary
                if constexpr (Hasx64RemoveAbsPtrs32Function<DirectType>)
                {
                    for (std::size_t i = 0; i < count; ++i)
                    {
                        direct[i].x64RemoveAbsPtrs32();
                    }
                }
#endif

                delete[] direct;
            }
        }

        inline std::size_t Count() const noexcept
        {
            return count;
        }

        inline DirectType* GetDirect() noexcept
        {
            return direct;
        }

        inline const DirectType* GetDirect() const noexcept
        {
            return direct;
        }

        inline T* Get() noexcept
        {
            return managed;
        }

        inline const T* Get() const noexcept
        {
            return managed;
        }

        inline operator T* () noexcept
        {
            return managed;
        }

        inline operator const T* () const noexcept
        {
            return managed;
        }

        // TODO: Are these functions named clearly enough?
        inline void SetArr32(hl_ArrOff32& arr) const
        {
            arr.Count = static_cast<uint32_t>(count);
            HL_SETPTR32(arr.Offset, direct);
        }

        inline void SetArr64(hl_ArrOff64& arr) const
        {
            arr.Count = static_cast<uint64_t>(count);
            HL_SETPTR64(arr.Offset, direct);
        }

        // TODO: Do we need an indexer operator? The T* operator might suffice.
        // TODO: Iterators?
    };

    // TODO: Custom vector type?
}
