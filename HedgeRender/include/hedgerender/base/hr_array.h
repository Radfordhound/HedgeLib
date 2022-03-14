#ifndef HR_ARRAY_H_INCLUDED
#define HR_ARRAY_H_INCLUDED
#include <algorithm>
#include <type_traits>
#include <stdexcept>
#include <cstddef>
#include <cstring>
#include <utility>

namespace hr
{
struct no_default_construct_t
{
    constexpr explicit no_default_construct_t() noexcept = default;
};

constexpr const no_default_construct_t no_default_construct;

template<typename T, typename count_t = std::size_t>
class fixed_array
{
protected:
    T* m_data = nullptr;
    count_t m_count = 0;

    template<typename U>
    static typename std::enable_if<std::is_trivially_default_constructible<U>::value>::type
        in_default_construct(count_t count, U* data)
    {
        std::memset(data, 0, sizeof(U) * count);
    }

    template<typename U>
    static typename std::enable_if<!std::is_trivially_default_constructible<U>::value &&
        std::is_default_constructible<U>::value>::type
        in_default_construct(count_t count, U* data)
    {
        for (count_t i = 0; i < count; ++i)
        {
            new (data + i) U();
        }
    }

    template<bool defaultConstruct = true>
    static typename std::enable_if<defaultConstruct, T*>::type
        in_allocate(count_t count)
    {
        T* data = static_cast<T*>(::operator new[](sizeof(T) * count));
        in_default_construct(count, data);
        return data;
    }

    template<bool defaultConstruct>
    static typename std::enable_if<!defaultConstruct, T*>::type
        in_allocate(count_t count)
    {
        return static_cast<T*>(::operator new[](sizeof(T) * count));
    }

    template<typename U>
    static typename std::enable_if<std::is_trivially_copyable<U>::value>::type
        in_copy(const U* src, count_t count, U* dst)
    {
        std::memcpy(dst, src, sizeof(U) * count);
    }

    template<typename U>
    static typename std::enable_if<!std::is_trivially_copyable<U>::value>::type
        in_copy(const U* src, count_t count, U* dst)
    {
        std::copy(src, src + count, dst);
    }

    template<typename U>
    static typename std::enable_if<std::is_trivially_copy_constructible<U>::value>::type
        in_copy_construct(const U* src, count_t count, U* dst)
    {
        std::memcpy(dst, src, sizeof(U) * count);
    }

    template<typename U>
    static typename std::enable_if<!std::is_trivially_copy_constructible<U>::value>::type
        in_copy_construct(const U* src, count_t count, U* dst)
    {
        for (count_t i = 0; i < count; ++i)
        {
            new (dst + i) U(src[i]);
        }
    }

    template<typename U>
    static typename std::enable_if<std::is_trivially_destructible<U>::value>::type
        in_destroy(U* data, count_t count)
    {
        ::operator delete[](data);
    }

    template<typename U>
    static typename std::enable_if<!std::is_trivially_destructible<U>::value>::type
        in_destroy(U* data, count_t count)
    {
        for (count_t i = 0; i < count; ++i)
        {
            data[i].~U();
        }

        ::operator delete[](data);
    }

public:
    using value_type = T;
    using size_type = count_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = value_type*;
    using const_iterator = const value_type*;

    inline const_iterator cbegin() const noexcept
    {
        return m_data;
    }

    inline const_iterator begin() const noexcept
    {
        return m_data;
    }

    inline iterator begin() noexcept
    {
        return m_data;
    }

    inline const_iterator cend() const noexcept
    {
        return (m_data + m_count);
    }

    inline const_iterator end() const noexcept
    {
        return (m_data + m_count);
    }

    inline iterator end() noexcept
    {
        return (m_data + m_count);
    }

    inline const_reference front() const noexcept
    {
        return *m_data;
    }

    inline reference front() noexcept
    {
        return *m_data;
    }

    inline const_reference back() const noexcept
    {
        return m_data[m_count - 1];
    }

    inline reference back() noexcept
    {
        return m_data[m_count - 1];
    }

    inline const T* data() const noexcept
    {
        return m_data;
    }

    inline T* data() noexcept
    {
        return m_data;
    }

    inline bool empty() const noexcept
    {
        return (m_count == 0);
    }

    inline count_t size() const noexcept
    {
        return m_count;
    }

    const_reference at(count_t pos) const
    {
        if (pos >= m_count)
        {
            throw std::out_of_range("The given index was outside the range of the array");
        }

        return m_data[pos];
    }

    reference at(count_t pos)
    {
        if (pos >= m_count)
        {
            throw std::out_of_range("The given index was outside the range of the array");
        }

        return m_data[pos];
    }

    void swap(fixed_array& other) noexcept
    {
        if (&other != this)
        {
            T* tmpData = m_data;
            count_t tmpCount = m_count;

            m_data = other.m_data;
            m_count = other.m_count;

            other.m_data = tmpData;
            other.m_count = tmpCount;
        }
    }

    const_reference operator[](count_t index) const noexcept
    {
        return m_data[index];
    }

    reference operator[](count_t index) noexcept
    {
        return m_data[index];
    }

    fixed_array& operator=(const fixed_array& other)
    {
        if (&other != this)
        {
            in_destroy(m_data, m_count);

            m_data = in_allocate<false>(other.m_count);
            m_count = other.m_count;

            in_copy(other.begin(), other.m_count, m_data);
        }

        return *this;
    }

    fixed_array& operator=(fixed_array&& other) noexcept
    {
        if (&other != this)
        {
            in_destroy(m_data, m_count);

            m_data = other.m_data;
            m_count = other.m_count;

            other.m_data = nullptr;
            other.m_count = 0;
        }

        return *this;
    }

    fixed_array() noexcept = default;

    fixed_array(const fixed_array& other) :
        m_data(in_allocate<false>(other.m_count)),
        m_count(other.m_count)
    {
        in_copy_construct(other.begin(), other.m_count, m_data);
    }

    fixed_array(fixed_array&& other) noexcept :
        m_data(other.m_data),
        m_count(other.m_count)
    {
        other.m_data = nullptr;
        other.m_count = 0;
    }

    fixed_array(count_t count) :
        m_data(in_allocate<true>(count)),
        m_count(count) {}

    fixed_array(count_t count, const no_default_construct_t) :
        m_data(in_allocate<false>(count)),
        m_count(count) {}

    inline ~fixed_array()
    {
        in_destroy(m_data, m_count);
    }
};

template<typename T, std::size_t stackCount>
class stack_or_heap_buffer
{
    T m_stackData[stackCount];
    T* m_heapData = nullptr;
    T* m_data;

    T* in_alloc_data(std::size_t count)
    {
        if (count > stackCount)
        {
            m_heapData = new T[count];
            return m_heapData;
        }
        else
        {
            return m_stackData;
        }
    }

public:
    stack_or_heap_buffer& operator=(stack_or_heap_buffer&& other) noexcept
    {
        if (&other != this)
        {
            delete[] m_heapData;

            m_stackData = std::move(other.m_stackData);
            m_heapData = other.m_heapData;
            m_data = other.m_data;

            other.m_heapData = nullptr;
        }

        return *this;
    }

    inline const T& operator[](std::size_t index) const
    {
        return m_data[index];
    }

    inline T& operator[](std::size_t index)
    {
        return m_data[index];
    }

    inline operator const T*() const noexcept
    {
        return m_data;
    }

    inline operator T*() noexcept
    {
        return m_data;
    }

    stack_or_heap_buffer(std::size_t count) :
        m_data(in_alloc_data(count)) {}

    stack_or_heap_buffer(const stack_or_heap_buffer& other) = delete;

    stack_or_heap_buffer(stack_or_heap_buffer&& other) noexcept :
        m_stackData(std::move(other.m_stackData)),
        m_heapData(other.m_heapData),
        m_data(other.m_data)
    {
        other.m_heapData = nullptr;
    }

    ~stack_or_heap_buffer()
    {
        delete[] m_heapData;
    }
};
} // hr

namespace std
{
template<typename T, typename count_t>
void swap(hr::fixed_array<T, count_t>& a,
    hr::fixed_array<T, count_t>& b) noexcept
{
    a.swap(b);
}
}
#endif
