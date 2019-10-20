#pragma once
#ifdef __cplusplus
#include <memory>
#include <cstdlib>
#include <cstddef>
#include <utility>

template <class T>
class hl_CPtr
{
    T* ptr;

public:
    constexpr hl_CPtr() noexcept : ptr(nullptr) {}
    constexpr hl_CPtr(std::nullptr_t) noexcept : ptr(nullptr) {}
    inline hl_CPtr(T* ptr) noexcept : ptr(ptr) {}
    hl_CPtr(const hl_CPtr&) = delete;

    inline ~hl_CPtr()
    {
        std::free(ptr);
    }

    inline T* Get() const noexcept
    {
        return ptr;
    }

    inline T* Release() noexcept
    {
#ifdef __cpp_lib_exchange_function
        return std::exchange(ptr, nullptr);
#else
        T* old = ptr;
        ptr = nullptr;
        return old;
#endif
    }

    inline void Swap(hl_CPtr& other)
    {
        std::swap(ptr, other.ptr);
    }

    inline void Reset(T* v = nullptr) noexcept
    {
        // Free the data first if necessary
        if (ptr) std::free(ptr);

        // Reset the pointer to the given value
        ptr = v;
    }

    hl_CPtr& operator=(const hl_CPtr&) = delete;
    inline operator T* () const noexcept
    {
        return ptr;
    }

    inline T* operator->() const noexcept
    {
        return ptr;
    }

    inline hl_CPtr& operator=(std::nullptr_t) noexcept
    {
        Reset();
        return *this;
    }

    inline hl_CPtr& operator=(T* other) noexcept
    {
        if (ptr) std::free(ptr);
        ptr = other;
        return *this;
    }

    inline hl_CPtr& operator=(hl_CPtr&& other) noexcept
    {
        if (this != std::addressof(other))
        {
            if (ptr) std::free(ptr);
            ptr = std::move(other.ptr);
        }

        return *this;
    }

    inline const T** operator&() const noexcept
    {
        return &ptr;
    }
};
#endif
