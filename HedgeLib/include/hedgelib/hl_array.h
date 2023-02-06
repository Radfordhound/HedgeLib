#ifndef HL_ARRAY_H_INCLUDED
#define HL_ARRAY_H_INCLUDED
#include "hl_memory.h"

namespace hl
{
template<typename T, typename count_t = std::size_t>
class fixed_array
{
    T* m_data = nullptr;
    count_t m_count = 0;

    inline void in_destruct() noexcept
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

    [[nodiscard]] inline bool empty() const noexcept
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
        destroy(m_data, m_count);

        // Reset array to empty state.
        m_data = nullptr;
        m_count = 0;
    }

    template<typename... Args>
    void assign(size_type count, const Args&... args)
    {
        // Destruct array contents if necessary.
        in_destruct();

        // Re-allocate array memory.
        m_data = reallocate<T>(m_data, count);

        // Direct-construct new array values.
        uninitialized_direct_construct(m_data, m_data + count, args...);

        // Set new array count.
        m_count = count;
    }

    template<typename Iterator>
    void assign(Iterator first, Iterator last)
    {
        // Destruct array contents if necessary.
        in_destruct();

        // Re-allocate array memory.
        const auto newCount = static_cast<count_t>(last - first);
        m_data = reallocate<T>(m_data, newCount);

        // Copy-construct new array values.
        std::uninitialized_copy(first, last, m_data);

        // Set new array count.
        m_count = newCount;
    }

    inline void assign(std::initializer_list<T> ilist)
    {
        assign(ilist.begin(), ilist.end());
    }

    void resize(const no_value_init_t, size_type count)
    {
        // Resize array memory.
        m_data = hl::resize(no_value_init, m_data, m_count, count);

        // Set new array count.
        m_count = count;
    }

    template<typename... Args>
    void resize(size_type count, const Args&... args)
    {
        // Resize array memory.
        m_data = hl::resize(m_data, m_count, count, args...);

        // Set new array count.
        m_count = count;
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
            destroy(m_data, m_count);

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
        m_data(create<T>(no_value_init, count)),
        m_count(count) {}

    fixed_array(size_type count) :
        m_data(create<T>(count)),
        m_count(count) {}

    template<typename... Args>
    fixed_array(size_type count, const Args&... args) :
        m_data(create(count, args...)),
        m_count(count) {}

    template<typename Iterator>
    fixed_array(Iterator first, Iterator last) :
        m_data(create(first, last)),
        m_count(static_cast<std::size_t>(last - first)) {}

    fixed_array(std::initializer_list<T> ilist) :
        m_data(create(ilist.begin(), ilist.end())),
        m_count(ilist.size()) {}

    fixed_array(const fixed_array& other) :
        m_data(create(other.begin(), other.end())),
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
        destroy(m_data, m_count);
    }
};

template<typename T, typename count_t>
void swap(fixed_array<T, count_t>& a,
    fixed_array<T, count_t>& b) noexcept
{
    a.swap(b);
}
} // hl
#endif
