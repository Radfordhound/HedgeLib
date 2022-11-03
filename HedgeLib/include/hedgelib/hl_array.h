#ifndef HL_ARRAY_H_INCLUDED
#define HL_ARRAY_H_INCLUDED
#include "hl_internal.h"
#include <initializer_list>
#include <iterator>
#include <memory>
#include <utility>

namespace hl
{
template<typename T, typename count_t = std::size_t>
class fixed_array
{
    T* m_data = nullptr;
    count_t m_count = 0;

    static T* in_create(std::size_t count, const T& val)
    {
        std::unique_ptr<T[]> data(new T[count]);
        std::uninitialized_fill(data.get(), data.get() + count, val);
        return data.release();
    }

    template<typename Iterator>
    static T* in_create(Iterator first, Iterator last)
    {
        std::unique_ptr<T[]> data(new T[static_cast<std::size_t>(last - first)]);
        std::uninitialized_copy(first, last, data.get());
        return data.release();
    }

public:
    using value_type = T;
    using size_type = count_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    inline const T* data() const noexcept
    {
        return m_data;
    }

    inline T* data() noexcept
    {
        return m_data;
    }

    inline size_type size() const noexcept
    {
        return m_count;
    }

    inline bool empty() const noexcept
    {
        return (m_count == 0);
    }

    inline const_reference front() const
    {
        return *m_data;
    }

    inline reference front()
    {
        return *m_data;
    }

    inline const_reference back() const
    {
        return m_data[m_count - 1];
    }

    inline reference back()
    {
        return m_data[m_count - 1];
    }

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

    inline const_reverse_iterator crbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }

    inline const_reverse_iterator rbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }

    inline reverse_iterator rbegin() noexcept
    {
        return reverse_iterator(end());
    }

    inline const_reverse_iterator crend() const noexcept
    {
        return const_reverse_iterator(begin());
    }

    inline const_reverse_iterator rend() const noexcept
    {
        return const_reverse_iterator(begin());
    }

    inline reverse_iterator rend() noexcept
    {
        return reverse_iterator(begin());
    }

    void clear() noexcept
    {
        // Destroy array contents.
        delete[] m_data;

        // Reset array to empty state.
        m_data = nullptr;
        m_count = 0;
    }

    void assign(size_type count, const T& val)
    {
        // Destruct array contents if necessary.
        // NOTE: This check is only necessary for debug-mode builds.
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            destruct(begin(), end());
        }

        // Mark array as being empty to keep the array's destructor from
        // calling destruct on the memory again if reallocate throws.
        m_count = 0;

        // Re-allocate array memory.
        m_data = new T[count];

        // Fill-construct new array values.
        std::uninitialized_fill(m_data, m_data + count, val);

        // Set new array count.
        m_count = count;
    }

    template<typename Iterator>
    void assign(Iterator first, Iterator last)
    {
        // Destruct array contents if necessary.
        // NOTE: This check is only necessary for debug-mode builds.
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            destruct(begin(), end());
        }

        // Mark array as being empty to keep the array's destructor from
        // calling destruct on the memory again if reallocate throws.
        m_count = 0;

        // Re-allocate array memory.
        const auto newCount = static_cast<count_t>(last - first);
        m_data = new T[newCount];

        // Copy-construct new array values.
        std::uninitialized_copy(first, last, m_data);

        // Set new array count.
        m_count = newCount;
    }

    inline void assign(std::initializer_list<T> ilist)
    {
        assign(ilist.begin(), ilist.end());
    }

    void swap(fixed_array& other) noexcept
    {
        if (&other != this)
        {
            std::swap(m_data, other.m_data);
            std::swap(m_count, other.m_count);
        }
    }

    const_reference at(size_type pos) const
    {
        if (pos >= m_count)
        {
            throw std::out_of_range(
                "Attempted to access a value outside the range of the array");
        }

        return m_data[pos];
    }

    reference at(size_type pos)
    {
        if (pos >= m_count)
        {
            throw std::out_of_range(
                "Attempted to access a value outside the range of the array");
        }

        return m_data[pos];
    }

    inline const_reference operator[](size_type pos) const
    {
        return m_data[pos];
    }

    inline reference operator[](size_type pos)
    {
        return m_data[pos];
    }

    fixed_array& operator=(const fixed_array& other)
    {
        if (&other != this)
        {
            assign(other.begin(), other.end());
        }

        return *this;
    }

    fixed_array& operator=(fixed_array&& other) noexcept
    {
        if (&other != this)
        {
            delete[] m_data;

            m_data = other.m_data;
            m_count = other.m_count;

            other.m_data = nullptr;
            other.m_count = 0;
        }

        return *this;
    }

    inline fixed_array& operator=(std::initializer_list<T> ilist)
    {
        assign(ilist);
        return *this;
    }

    fixed_array() noexcept = default;

    explicit fixed_array(const no_value_init_t, size_type count) :
        m_data(new T[count]),
        m_count(count) {}

    fixed_array(size_type count) :
        m_data(new T[count]()),
        m_count(count) {}

    fixed_array(size_type count, const T& val) :
        m_data(in_create(count, val)),
        m_count(count) {}

    template<typename Iterator>
    fixed_array(Iterator first, Iterator last) :
        m_data(in_create(first, last)),
        m_count(static_cast<std::size_t>(last - first)) {}

    fixed_array(std::initializer_list<T> ilist) :
        m_data(in_create(ilist.begin(), ilist.end())),
        m_count(ilist.size()) {}

    fixed_array(const fixed_array& other) :
        m_data(in_create(other.begin(), other.end())),
        m_count(other.m_count) {}

    fixed_array(fixed_array&& other) noexcept :
        m_data(other.m_data),
        m_count(other.m_count)
    {
        other.m_data = nullptr;
        other.m_count = 0;
    }

    inline ~fixed_array()
    {
        delete[] m_data;
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
} // hl

namespace std
{
template<typename T, typename count_t>
void swap(hl::fixed_array<T, count_t>& a,
    hl::fixed_array<T, count_t>& b) noexcept
{
    a.swap(b);
}
}
#endif
