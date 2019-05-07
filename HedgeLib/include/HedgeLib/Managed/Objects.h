#pragma once
#include "Pointers.h"

namespace HedgeLib
{
    template<typename T, template<typename> typename P,
        template<typename> typename PGetter = SPointerGetter>
    class ParentObject
    {
    protected:
        P<T> ptr = nullptr;

    public:
        inline ParentObject() = default;
        inline ParentObject(T* ptr) : ptr(ptr) {}

        inline T* Get() const noexcept
        {
            return PGetter<T>()(ptr.get());
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
        inline ChildObject() : ptr(std::malloc(sizeof(T))),
            deletePtr(true) {};

        inline ChildObject(T* ptr) : ptr(ptr) {}

        inline ~ChildObject()
        {
            if (deletePtr) std::free(ptr);
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
    using HHParentObject = ParentObject<T, HHPointer, HHPointerGetter>;
}
