#pragma once
#include "Offsets.h"
#include "Pointers.h"
#include "Helpers.h"
#include <variant>
#include <utility>

namespace HedgeLib
{
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
            deletePtr(true)
        {
#ifdef x64
            // Add 32-bit absolute pointers if necessary
            if constexpr (Hasx64AddAbsPtrs32Function<T>)
            {
                ptr->x64AddAbsPtrs32();
            }
#endif
        }

        inline ChildObject(T* ptr) : ptr(ptr) {}

        inline ChildObject(const ChildObject<T>& o) = default;
        inline ChildObject(ChildObject<T>&& o) noexcept = default;

        inline ChildObject& operator=(const ChildObject<T>& o)
        {
            return *this = ChildObject<T>(o);
        }

        inline ChildObject& operator=(ChildObject<T>&& o) noexcept
        {
            std::swap(ptr, o.ptr);
            std::swap(deletePtr, o.deletePtr);
            return *this;
        }

        inline ~ChildObject()
        {
            if (deletePtr)
            {
#ifdef x64
                // Remove 32-bit absolute pointers if necessary
                if constexpr (Hasx64RemoveAbsPtrs32Function<T>)
                {
                    ptr->x64RemoveAbsPtrs32();
                }
#endif

                delete ptr;
            }
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
