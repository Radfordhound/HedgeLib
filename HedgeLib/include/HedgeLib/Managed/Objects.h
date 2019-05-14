#pragma once
#include "Pointers.h"
#include <variant>

namespace HedgeLib
{
    template<typename T, template<typename> typename P,
        template<typename> typename PGetter>
    class ParentObject
    {
    protected:
        //P<T> ptr = nullptr;
        std::variant<P<T>, std::unique_ptr<T>> ptr;

        inline bool IsCustomPtr() const noexcept
        {
            return std::holds_alternative<P<T>>(ptr);
        }

    public:
        // TODO: Should we use new instead of malloc?
        // Are there any instances where we need C code to clean this up?
        inline ParentObject() : ptr(std::unique_ptr<T>(new T())) {}
            /*ptr(static_cast<T*>(
            std::malloc(sizeof(T)))) {};*/

        inline ParentObject(T* ptr) : ptr(ptr) {}

        inline T* Get() const noexcept
        {
            return IsCustomPtr() ?
                PGetter<T>()(std::get<P<T>>(ptr).get()) : // Get custom pointer
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
        // TODO: Should we use new instead of malloc?
        // Are there any instances where we need C code to clean this up?
        inline ChildObject() :
            ptr(new T()),
            //ptr(static_cast<T*>(std::malloc(sizeof(T)))),
            deletePtr(true) {}

        inline ChildObject(T* ptr) : ptr(ptr) {}

        inline ~ChildObject()
        {
            if (deletePtr) delete ptr;
            //if (deletePtr) std::free(ptr);
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
