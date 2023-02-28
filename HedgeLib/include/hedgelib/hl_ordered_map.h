#ifndef HL_ORDERED_MAP_H_INCLUDED
#define HL_ORDERED_MAP_H_INCLUDED
#include <robin_hood.h>
#include <vector>
#include <cassert>

namespace hl
{
template<typename Key, typename Value>
class ordered_map
{
    using in_map_type   = robin_hood::unordered_map<Key, Value>;
    using in_vec_type   = std::vector<typename in_map_type::value_type*>;
    using in_this_type  = ordered_map<Key, Value>;

    in_map_type m_data;
    in_vec_type m_orderedPtrs;

    template<bool IsConst>
    class in_iterator
    {
        friend in_this_type;

        using in_inner_type = std::conditional_t<IsConst,
            typename in_vec_type::const_iterator,
            typename in_vec_type::iterator>;

        in_inner_type m_current;

    public:
        using difference_type   = std::ptrdiff_t;
        using value_type        = typename in_map_type::value_type;

        using reference         = std::conditional_t<
            IsConst, const value_type&, value_type&>;

        using pointer           = std::conditional_t<
            IsConst, const value_type*, value_type*>;

        using iterator_category = std::random_access_iterator_tag;

        inline void swap(in_iterator& other) noexcept
        {
            using std::swap;
            swap(m_current, other.m_current);
        }

        inline in_iterator& operator++() noexcept
        {
            ++m_current;
            return *this;
        }

        inline in_iterator operator++(int) noexcept
        {
            const auto tmp = *this;
            ++(*this);
            return tmp;
        }

        inline in_iterator& operator--() noexcept
        {
            --m_current;
            return *this;
        }

        inline in_iterator operator--(int) noexcept
        {
            const auto tmp = *this;
            --(*this);
            return tmp;
        }

        inline in_iterator& operator+=(difference_type amount) noexcept
        {
            m_current += amount;
            return *this;
        }

        inline in_iterator& operator-=(difference_type amount) noexcept
        {
            m_current -= amount;
            return *this;
        }

        inline reference operator*() const
        {
            return **m_current;
        }

        inline pointer operator->() const
        {
            return *m_current;
        }

        inline reference operator[](difference_type pos) const noexcept
        {
            return m_current[pos];
        }

        inline bool operator==(in_iterator other) const noexcept
        {
            return m_current == other.m_current;
        }

        inline bool operator!=(in_iterator other) const noexcept
        {
            return m_current != other.m_current;
        }

        inline bool operator<(in_iterator other) const noexcept
        {
            return m_current < other.m_current;
        }

        inline bool operator>(in_iterator other) const noexcept
        {
            return m_current > other.m_current;
        }

        inline bool operator<=(in_iterator other) const noexcept
        {
            return m_current <= other.m_current;
        }

        inline bool operator>=(in_iterator other) const noexcept
        {
            return m_current >= other.m_current;
        }

        inline friend void swap(in_iterator& a, in_iterator& b) noexcept
        {
            a.swap(b);
        }

        inline friend in_iterator operator+(in_iterator it, difference_type amount) noexcept
        {
            return (it.m_current + amount);
        }

        inline friend in_iterator operator+(difference_type amount, in_iterator it) noexcept
        {
            return (amount + it.m_current);
        }

        inline friend in_iterator operator-(in_iterator it, difference_type amount) noexcept
        {
            return (it.m_current - amount);
        }

        inline friend difference_type operator-(in_iterator b, in_iterator a) noexcept
        {
            return (b - a);
        }

        inline in_iterator() noexcept = default;

        inline in_iterator(in_inner_type it) noexcept :
            m_current(std::move(it)) {}
    };

public:
    using key_type          = typename in_map_type::key_type;
    using mapped_type       = typename in_map_type::mapped_type;
    using value_type        = typename in_map_type::value_type;
    using size_type         = typename in_map_type::size_type;
    //using difference_type   = in_map_type::difference_type;
    using hasher            = typename in_map_type::hasher;
    using key_equal         = typename in_map_type::key_equal;
    //using allocator_type    = in_map_type::allocator_type;
    using reference         = value_type&;
    using const_reference   = const value_type;
    using pointer           = value_type*;
    using const_pointer     = const value_type*;
    using iterator          = in_iterator<false>;
    using const_iterator    = in_iterator<true>;

    inline const_iterator cbegin() const noexcept
    {
        return m_orderedPtrs.begin();
    }

    inline const_iterator begin() const noexcept
    {
        return cbegin();
    }

    inline iterator begin() noexcept
    {
        return m_orderedPtrs.begin();
    }

    inline const_iterator cend() const noexcept
    {
        return m_orderedPtrs.end();
    }

    inline const_iterator end() const noexcept
    {
        return cend();
    }

    inline iterator end()
    {
        return m_orderedPtrs.end();
    }

    [[nodiscard]] inline bool empty() const noexcept
    {
        return m_data.empty();
    }

    inline size_type size() const noexcept
    {
        return m_data.size();
    }

    const Value* get(const Key& key) const
    {
        const auto it = m_data.find(key);
        return (it == m_data.end()) ? nullptr : &it->second;
    }

    Value* get(const Key& key)
    {
        const auto it = m_data.find(key);
        return (it == m_data.end()) ? nullptr : &it->second;
    }

    inline bool contains(const Key& key) const
    {
        return m_data.contains(key);
    }

    void reserve(size_type count)
    {
        m_data.reserve(count);
        m_orderedPtrs.reserve(count);
    }

    std::pair<value_type*, bool> insert(const value_type& value)
    {
        const auto p = m_data.insert(value);
        if (p.second)
        {
            m_orderedPtrs.push_back(&(*p.first));
        }

        return { &(*p.first), p.second };
    }

    std::pair<value_type*, bool> insert(value_type&& value)
    {
        const auto p = m_data.insert(std::move(value));
        if (p.second)
        {
            m_orderedPtrs.push_back(&(*p.first));
        }

        return { &(*p.first), p.second };
    }

    template<typename InputIt>
    void insert(InputIt first, InputIt last)
    {
        for (; first != last; ++first)
        {
            insert(*first);
        }
    }

    inline void insert(std::initializer_list<value_type> ilist)
    {
        insert(ilist.begin(), ilist.end());
    }

    template<typename... Args>
    std::pair<value_type*, bool> emplace(Args&&... args)
    {
        const auto p = m_data.emplace(std::forward<Args>(args)...);
        if (p.second)
        {
            m_orderedPtrs.push_back(&(*p.first));
        }

        return { &(*p.first), p.second };
    }

    template<typename Mapped>
    std::pair<value_type*, bool> insert_or_assign(const Key& key, Mapped&& obj)
    {
        const auto p = m_data.insert_or_assign(key, std::forward<Mapped>(obj));
        if (p.second)
        {
            m_orderedPtrs.push_back(&(*p.first));
        }

        return { &(*p.first), p.second };
    }

    template<typename Mapped>
    std::pair<Value*, bool> insert_or_assign(Key&& key, Mapped&& obj)
    {
        const auto p = m_data.insert_or_assign(std::move(key), std::forward<Mapped>(obj));
        if (p.second)
        {
            m_orderedPtrs.emplace_back(p.first->first, &p.first->second);
        }
        else
        {
            p.first->second = std::forward<Mapped>(obj);
        }

        return { &p.first->second, p.second };
    }

    iterator erase(const_iterator pos)
    {
        const auto it = m_orderedPtrs.erase(pos.m_current);
        m_data.erase(pos->first);
        return it;
    }

    iterator erase(iterator pos)
    {
        const auto it = m_orderedPtrs.erase(pos.m_current);
        m_data.erase(pos->first);
        return it;
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        const auto it = m_orderedPtrs.erase(first.m_current, last.m_current);
        for (; first != last; ++first)
        {
            m_data.erase((*first)->first);
        }

        return it;
    }

    void clear() noexcept
    {
        m_orderedPtrs.clear();
        m_data.clear();
    }

    void swap(ordered_map& other) noexcept
    {
        m_data.swap(other.m_data);
        m_orderedPtrs.swap(other.m_orderedPtrs);
    }

    inline const Value& operator[](const Key& key) const
    {
        const auto it = m_data.find(key);
        assert(it != m_data.end() &&
            "The given key was not found within the map");

        return it->second;
    }

    inline Value& operator[](const Key& key)
    {
        const auto it = m_data.find(key);
        assert(it != m_data.end() &&
            "The given key was not found within the map");

        return it->second;
    }

    ordered_map& operator=(std::initializer_list<value_type> ilist)
    {
        clear();
        insert(ilist);
        return *this;
    }

    ordered_map& operator=(const ordered_map& other)
    {
        if (&other != this)
        {
            clear();
            insert(other.begin(), other.end());
        }

        return *this;
    }

    ordered_map& operator=(ordered_map&& other) noexcept = default;

    ordered_map() noexcept(
        noexcept(typename in_map_type::Table()) &&
        noexcept(typename in_vec_type::vector())) = default;

    template<typename InputIt>
    ordered_map(InputIt first, InputIt last)
    {
        insert(first, last);
    }

    inline ordered_map(std::initializer_list<value_type> ilist) :
        ordered_map(ilist.begin(), ilist.end()) {}

    inline ordered_map(const ordered_map& other) :
        ordered_map(other.begin(), other.end()) {}

    ordered_map(ordered_map&& other) noexcept = default;
};

template<typename Key, typename Value>
inline void swap(ordered_map<Key, Value>& a,
    ordered_map<Key, Value>& b) noexcept
{
    a.swap(b);
}
} // hl
#endif
