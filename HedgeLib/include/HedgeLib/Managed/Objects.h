#pragma once
#include "Pointers.h"
#include "Helpers.h"
#include <variant>

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

    template<typename T, typename P,
        template<typename> typename PGetter>
    class ParentObject
    {
    protected:
        std::variant<P, std::unique_ptr<T>> ptr;

        inline bool IsCustomPtr() const noexcept
        {
            return std::holds_alternative<P>(ptr);
        }

    public:
        inline ParentObject() : ptr(std::unique_ptr<T>(new T()))
        {
#ifdef x64
            // Add 32-bit absolute pointers if necessary
            if constexpr (Hasx64AddAbsPtrs32Function<T>)
            {
                std::get<std::unique_ptr<T>>(ptr)->x64AddAbsPtrs32();
            }
#endif
        }

        inline ParentObject(T* ptr) : ptr(std::unique_ptr<T>(ptr)) {}

        inline T* Get() const noexcept
        {
            return IsCustomPtr() ?
                PGetter<T>()(std::get<P>(ptr).get()) :    // Get custom pointer
                std::get<std::unique_ptr<T>>(ptr).get();  // Get normal pointer
        }

#ifdef x64
        inline ~ParentObject()
        {
            // Remove 32-bit absolute pointers if necessary
            if constexpr (Hasx64RemoveAbsPtrs32Function<T>)
            {
                if (!IsCustomPtr())
                {
                    std::get<std::unique_ptr<T>>(
                        ptr)->x64RemoveAbsPtrs32();
                }
            }
        }
#endif

        inline void EndianSwap()
        {
            ptr->EndianSwap();
        }

        inline void EndianSwapRecursive(bool isBigEndian)
        {
            ptr->EndianSwapRecursive(isBigEndian);
        }
    };

    template<typename T>
    class ChildObject
    {
    protected:
        T* ptr = nullptr;
        bool deletePtr = false;

    public:
        inline ChildObject() :
            ptr(new T()),
            deletePtr(true) {}

        inline ChildObject(T* ptr) : ptr(ptr) {}

        inline ~ChildObject()
        {
            if (deletePtr) delete ptr;
        }

        inline T* Get() const noexcept
        {
            return ptr;
        }

        inline void EndianSwap()
        {
            ptr->EndianSwap();
        }

        inline void EndianSwapRecursive(bool isBigEndian)
        {
            ptr->EndianSwapRecursive(isBigEndian);
        }
    };

    template<typename T>
    using HHParentObject = ParentObject<T, HHBlobPointer, HHBlobGetter>;
}
