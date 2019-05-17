#pragma once
#include "Pointers.h"
#include <variant>

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
        inline ParentObject() : ptr(std::unique_ptr<T>(new T())) {}
        inline ParentObject(T* ptr) : ptr(std::unique_ptr<T>(ptr)) {}

        inline T* Get() const noexcept
        {
            return IsCustomPtr() ?
                PGetter<T>()(std::get<P>(ptr).get()) :    // Get custom pointer
                std::get<std::unique_ptr<T>>(ptr).get();  // Get normal pointer
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
