#ifndef HL_MEMORY_H_INCLUDED
#define HL_MEMORY_H_INCLUDED
#include <atomic>
#include <memory>

namespace hl
{
template<typename T>
class atomic_unique_ptr
{
    std::atomic<T*> m_data;

public:
    using pointer = T*;
    using element_type = T;

    inline pointer get() const noexcept
    {
        return m_data.load();
    }

    inline pointer release() noexcept
    {
        return m_data.exchange(nullptr);
    }

    inline void reset(pointer ptr = pointer()) noexcept
    {
        delete m_data.exchange(ptr);
    }

    inline pointer exchange(pointer ptr = pointer()) noexcept
    {
        return m_data.exchange(ptr);
    }

    inline std::add_lvalue_reference_t<T> operator*() const noexcept
    {
        return *get();
    }

    inline pointer operator->() const noexcept
    {
        return get();
    }

    inline explicit operator bool() const noexcept
    {
        return (get() != nullptr);
    }

    template<typename U>
    atomic_unique_ptr& operator=(atomic_unique_ptr<U>&& other) noexcept
    {
        if (&other != this)
        {
            reset(other.release());
        }

        return *this;
    }

    template<typename U>
    atomic_unique_ptr& operator=(std::unique_ptr<U>&& other) noexcept
    {
        reset(other.release());
        return *this;
    }

    inline atomic_unique_ptr& operator=(std::nullptr_t) noexcept
    {
        reset();
        return *this;
    }

    constexpr atomic_unique_ptr() noexcept : 
        m_data(nullptr) {}

    constexpr atomic_unique_ptr(std::nullptr_t) noexcept :
        m_data(nullptr) {}

    explicit atomic_unique_ptr(pointer ptr) noexcept :
        m_data(ptr) {}

    template<typename U>
    atomic_unique_ptr(atomic_unique_ptr<U>&& other) noexcept :
        m_data(other.release()) {}

    template<typename U>
    atomic_unique_ptr(std::unique_ptr<U>&& other) noexcept :
        m_data(other.release()) {}

    inline ~atomic_unique_ptr()
    {
        reset();
    }
};

template<typename T>
class atomic_unique_ptr<T[]>
{
    std::atomic<T*> m_data;

public:
    using pointer = T*;
    using element_type = T;

    inline pointer get() const noexcept
    {
        return m_data.load();
    }

    inline pointer release() noexcept
    {
        return m_data.exchange(nullptr);
    }

    template<typename U>
    inline void reset(U ptr) noexcept
    {
        delete[] m_data.exchange(ptr);
    }

    inline void reset(std::nullptr_t = nullptr) noexcept
    {
        delete[] m_data.exchange(pointer());
    }

    template<typename U>
    inline pointer exchange(U ptr) noexcept
    {
        return m_data.exchange(ptr);
    }

    inline pointer exchange(std::nullptr_t = nullptr) noexcept
    {
        return m_data.exchange(pointer());
    }

    inline T& operator[](std::size_t pos) const
    {
        return get()[pos];
    }

    inline explicit operator bool() const noexcept
    {
        return (get() != nullptr);
    }

    template<typename U>
    atomic_unique_ptr& operator=(atomic_unique_ptr<U>&& other) noexcept
    {
        if (&other != this)
        {
            reset(other.release());
        }

        return *this;
    }

    template<typename U>
    atomic_unique_ptr& operator=(std::unique_ptr<U>&& other) noexcept
    {
        reset(other.release());
        return *this;
    }

    inline atomic_unique_ptr& operator=(std::nullptr_t) noexcept
    {
        reset();
        return *this;
    }

    constexpr atomic_unique_ptr() noexcept :
        m_data(nullptr) {}

    constexpr atomic_unique_ptr(std::nullptr_t) noexcept :
        m_data(nullptr) {}

    template<typename U>
    explicit atomic_unique_ptr(U ptr) noexcept :
        m_data(ptr) {}

    template<typename U>
    atomic_unique_ptr(atomic_unique_ptr<U>&& other) noexcept :
        m_data(other.release()) {}

    template<typename U>
    atomic_unique_ptr(std::unique_ptr<U>&& other) noexcept :
        m_data(other.release()) {}

    inline ~atomic_unique_ptr()
    {
        reset();
    }
};
} // hl
#endif
