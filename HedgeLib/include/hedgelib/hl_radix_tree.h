/**
* An implementation of "The Adaptive Radix Tree" (https://db.in.tum.de/~leis/papers/ART.pdf).
*/
#ifndef HL_RADIX_TREE_H_INCLUDED
#define HL_RADIX_TREE_H_INCLUDED
#include "hl_internal.h"
#include <string>
#include <algorithm>
#include <utility>
#include <vector>
#include <array>

namespace hl
{
namespace internal
{
enum class in_radix_node_type : u8
{
    /* Masks */
    child_count_mask = 0x3F,
    type_mask = 0xC0,

    /* Types */
    node4 = 0,
    node16 = 0x40,
    node48 = 0x80,
    node256 = 0xC0
};

HL_ENUM_CLASS_DEF_BITWISE_OPS(in_radix_node_type)

constexpr u8 in_radix_max_prefix_len = 10;

struct in_radix_node
{
    u8 flags;
    u8 prefixLen = 0;
    std::array<char, in_radix_max_prefix_len> prefix;

    inline bool is_leaf() const noexcept
    {
        return (flags == UINT8_MAX);
    }

    inline in_radix_node_type type() const noexcept
    {
        return static_cast<in_radix_node_type>(flags &
            static_cast<u8>(in_radix_node_type::type_mask));
    }

    inline u8 small_node_child_count() const noexcept
    {
        return (flags & static_cast<u8>(in_radix_node_type::child_count_mask));
    }

    HL_API u8 get_prefix_match_len(const char* key) const;

    HL_API void** get_ptr_to_child_ptr(u8 key);

    HL_API void* get_child_ptr(u8 key) const;

    HL_API void destroy() noexcept;

    HL_API in_radix_node(in_radix_node_type type) noexcept;

    HL_API in_radix_node(in_radix_node_type newType, const in_radix_node& other) noexcept;
};

struct in_radix_node4 : public in_radix_node
{
    std::array<u8, 4> keys;
    std::array<void*, 4> children;

    inline void set_child_unchecked(u8 index, u8 key, void* child)
    {
        keys[index] = key;
        children[index] = child;
    }

    HL_API in_radix_node4() noexcept;
};

struct in_radix_node16 : public in_radix_node
{
    alignas(16) std::array<u8, 16> keys;
    std::array<void*, 16> children;

    inline void set_child_unchecked(u8 index, u8 key, void* child)
    {
        keys[index] = key;
        children[index] = child;
    }

    HL_API in_radix_node16() noexcept;

    HL_API in_radix_node16(const in_radix_node4& node4) noexcept;
};

struct in_radix_node48 : public in_radix_node
{
    std::array<u8, 256> childIndices;
    std::array<void*, 48> children;

    inline void set_child_unchecked(u8 index, u8 key, void* child)
    {
        childIndices[key] = index;
        children[index] = child;
    }

    HL_API u8 get_first_unused_child_index() const noexcept;

    HL_API in_radix_node48() noexcept;

    HL_API in_radix_node48(const in_radix_node16& node16) noexcept;
};

struct in_radix_node256 : public in_radix_node
{
    std::array<void*, 256> children;

    HL_API in_radix_node256() noexcept;

    HL_API in_radix_node256(const in_radix_node48& node48) noexcept;
};

struct in_radix_leaf
{
    // AA 00 00 00  BB BB BB BB  CC CC CC CC  CC CC CC CC
    u8 type;
    u32 keyLen;
    std::size_t leafIndex;

    constexpr static std::size_t compute_data_off(std::size_t dataAlignment) noexcept
    {
        return (align(__STDCPP_DEFAULT_NEW_ALIGNMENT__ +
            sizeof(in_radix_leaf), dataAlignment) -
            __STDCPP_DEFAULT_NEW_ALIGNMENT__);
    }

    constexpr static std::size_t compute_size(std::size_t dataSize,
        std::size_t dataAlignment) noexcept
    {
        return (align(align(__STDCPP_DEFAULT_NEW_ALIGNMENT__ +
            sizeof(in_radix_leaf), dataAlignment) + dataSize,
            alignof(char)) - __STDCPP_DEFAULT_NEW_ALIGNMENT__);
    }

    HL_API static in_radix_leaf* create(std::size_t size,
        std::size_t index, const char* key);
};

using in_radix_sort_func = int (*)(unsigned char a, unsigned char b);

class in_radix_tree
{
protected:
    void* m_rootNode = nullptr;
    in_radix_sort_func m_sortFuncPtr;
    std::vector<in_radix_leaf*> m_leafNodes;

    using in_const_iterator = std::vector<in_radix_leaf*>::const_iterator;
    using in_iterator = std::vector<in_radix_leaf*>::iterator;

    inline in_const_iterator in_get_leaf_it(std::size_t index) const noexcept
    {
        return (m_leafNodes.begin() + index);
    }

    inline in_iterator in_get_leaf_it(std::size_t index) noexcept
    {
        return (m_leafNodes.begin() + index);
    }

    HL_API in_iterator in_add_leaf(in_radix_leaf& leaf);

    HL_API std::pair<in_iterator, bool> in_insert(
        const char* key, std::size_t leafSize);

    HL_API const in_radix_leaf* in_find_leaf(
        const char* key, std::size_t leafSize) const;

    inline in_radix_leaf* in_find_leaf(
        const char* key, std::size_t leafSize)
    {
        return const_cast<in_radix_leaf*>(const_cast<const in_radix_tree*>(
            this)->in_find_leaf(key, leafSize));
    }

    HL_API void in_destroy() noexcept;

    in_radix_tree& operator=(const in_radix_tree& other) = delete;

    HL_API in_radix_tree& operator=(in_radix_tree&& other) noexcept;

    HL_API in_radix_tree() noexcept;

    in_radix_tree(in_radix_sort_func sortFuncPtr) noexcept :
        m_sortFuncPtr(sortFuncPtr) {}

    HL_API in_radix_tree(in_radix_tree&& other) noexcept;

    inline ~in_radix_tree()
    {
        in_destroy();
    }
};
} // internal

template<typename T>
class radix_tree : public internal::in_radix_tree
{
protected:
    inline static const char* in_get_key_ptr(const internal::in_radix_leaf& leaf) noexcept
    {
        return ptradd<char>(&leaf, leaf_size());
    }

    inline static const T* in_get_data_ptr(const internal::in_radix_leaf& leaf) noexcept
    {
        return ptradd<T>(&leaf, leaf_data_off());
    }

    inline static T* in_get_data_ptr(internal::in_radix_leaf& leaf) noexcept
    {
        return ptradd<T>(&leaf, leaf_data_off());
    }

    inline static const internal::in_radix_leaf* in_get_leaf_ptr(const T* dataPtr) noexcept
    {
        return reinterpret_cast<const internal::in_radix_leaf*>(
            reinterpret_cast<const u8*>(dataPtr) - leaf_data_off());
    }

    inline static internal::in_radix_leaf* in_get_leaf_ptr(T* dataPtr) noexcept
    {
        return reinterpret_cast<internal::in_radix_leaf*>(
            reinterpret_cast<u8*>(dataPtr) - leaf_data_off());
    }

    void in_assign_nodes(const radix_tree& other)
    {
        reserve(other.size());

        for (const auto it : other)
        {
            insert(it.first, it.second);
        }
    }

    template<typename U = T>
    std::enable_if_t<!std::is_trivially_destructible_v<U>>
        in_destroy_data() noexcept
    {
        for (const auto leaf : m_leafNodes)
        {
            in_get_data_ptr(*leaf)->~T();
        }
    }

    template<typename U = T>
    inline std::enable_if_t<std::is_trivially_destructible_v<U>>
        in_destroy_data() noexcept {}

public:
    using value_type = std::pair<const char*, T&>;

    class const_iterator
    {
        friend radix_tree;

        in_const_iterator m_it;

        inline const_iterator(in_const_iterator it) noexcept :
            m_it(it) {}

    public:
        const value_type operator*() const
        {
            const auto leaf = *m_it;
            return { in_get_key_ptr(*leaf), *in_get_data_ptr(*leaf) };
        }

        ptr_proxy<const value_type> operator->() const
        {
            const auto leaf = *m_it;
            return { in_get_key_ptr(*leaf), *in_get_data_ptr(*leaf) };
        }

        inline const_iterator& operator++()
        {
            ++m_it;
            return *this;
        }

        inline const_iterator operator++(int)
        {
            return m_it++;
        }

        inline bool operator==(const const_iterator& other) const noexcept
        {
            return (m_it == other.m_it);
        }

        inline bool operator!=(const const_iterator& other) const noexcept
        {
            return (m_it != other.m_it);
        }
    };

    class iterator
    {
        friend radix_tree;

        in_iterator m_it;

        inline iterator(in_iterator it) noexcept :
            m_it(it) {}

    public:
        value_type operator*() const
        {
            const auto leaf = *m_it;
            return { in_get_key_ptr(*leaf), *in_get_data_ptr(*leaf) };
        }

        ptr_proxy<value_type> operator->() const
        {
            const auto leaf = *m_it;
            return { in_get_key_ptr(*leaf), *in_get_data_ptr(*leaf) };
        }

        inline iterator& operator++()
        {
            ++m_it;
            return *this;
        }

        inline iterator operator++(int)
        {
            return m_it++;
        }

        inline bool operator==(const iterator& other) const noexcept
        {
            return (m_it == other.m_it);
        }

        inline bool operator!=(const iterator& other) const noexcept
        {
            return (m_it != other.m_it);
        }
    };

    inline static constexpr std::size_t leaf_size() noexcept
    {
        return internal::in_radix_leaf::compute_size(sizeof(T), alignof(T));
    }

    inline static constexpr std::size_t leaf_data_off() noexcept
    {
        return internal::in_radix_leaf::compute_data_off(alignof(T));
    }

    inline const void* root_node() const noexcept
    {
        return m_rootNode;
    }

    inline void* root_node() noexcept
    {
        return m_rootNode;
    }

    inline std::size_t size() const noexcept
    {
        return m_leafNodes.size();
    }

    [[nodiscard]] inline bool empty() const noexcept
    {
        return m_leafNodes.empty();
    }

    inline const_iterator begin() const noexcept
    {
        return m_leafNodes.begin();
    }

    inline iterator begin() noexcept
    {
        return m_leafNodes.begin();
    }

    inline const_iterator end() const noexcept
    {
        return m_leafNodes.end();
    }

    inline iterator end() noexcept
    {
        return m_leafNodes.end();
    }

    inline void reserve(std::size_t capacity)
    {
        m_leafNodes.reserve(capacity);
    }

    template<typename... args_t>
    std::pair<iterator, bool> insert(const char* key, args_t&&... args)
    {
        // Insert a node with the given key, or just return it if one already exists.
        const auto p = in_insert(key, leaf_size());

        // If this is a new node, construct the data within the node.
        if (p.second)
        {
            new (in_get_data_ptr(*(*p.first))) T(std::forward<args_t>(args)...);
        }

        // Return a new pair using the new iterator type.
        return { p.first, p.second };
    }

    template<typename... args_t>
    inline std::pair<iterator, bool> insert(const std::string& key, args_t&&... args)
    {
        return insert(key.c_str(), std::forward<args_t>(args)...);
    }

    template<typename... args_t>
    std::pair<iterator, bool> replace(const char* key, args_t&&... args)
    {
        // Insert or replace a node with the given key.
        const auto p = in_insert(key, leaf_size());

        // If this is a new node, construct the data within the node.
        const auto dataPtr = in_get_data_ptr(*(*p.first));
        if (p.second)
        {
            new (dataPtr) T(std::forward<args_t>(args)...);
        }

        // Otherwise, replace the data.
        else
        {
            *dataPtr = std::move(T(std::forward<args_t>(args)...));
        }

        // Return a new pair using the new iterator type.
        return { p.first, p.second };
    }

    template<typename... args_t>
    inline std::pair<iterator, bool> replace(const std::string& key, args_t&&... args)
    {
        return replace(key.c_str(), std::forward<args_t>(args)...);
    }

    const_iterator find(const char* key) const
    {
        const auto leaf = in_find_leaf(key, leaf_size());
        return (!leaf) ? m_leafNodes.end() :
            (m_leafNodes.begin() + leaf->leafIndex);
    }

    inline const_iterator find(const std::string& key) const
    {
        return find(key.c_str());
    }

    iterator find(const char* key)
    {
        const auto leaf = in_find_leaf(key, leaf_size());
        return (!leaf) ? m_leafNodes.end() :
            (m_leafNodes.begin() + leaf->leafIndex);
    }

    inline iterator find(const std::string& key)
    {
        return find(key.c_str());
    }

    const T* get(const char* key) const
    {
        const auto leaf = in_find_leaf(key, leaf_size());
        return (leaf) ? in_get_data_ptr(*leaf) : nullptr;
    }

    inline const T* get(const std::string& key) const
    {
        return get(key.c_str());
    }

    inline T* get(const char* key)
    {
        return const_cast<T*>(const_cast<const radix_tree<T>*>(this)->get(key));
    }

    inline T* get(const std::string& key)
    {
        return get(key.c_str());
    }

    inline bool contains(const char* key) const
    {
        return (get(key) != nullptr);
    }

    inline bool contains(const std::string& key) const
    {
        return (get(key) != nullptr);
    }

    const T& at(const char* key) const
    {
        const auto leaf = in_find_leaf(key, leaf_size());
        if (!leaf) throw std::out_of_range("No element with the specified key was found");
        return *in_get_data_ptr(*leaf);
    }

    inline const T& at(const std::string& key) const
    {
        return at(key.c_str());
    }

    T& at(const char* key)
    {
        const auto leaf = in_find_leaf(key, leaf_size());
        if (!leaf) throw std::out_of_range("No element with the specified key was found");
        return *in_get_data_ptr(*leaf);
    }

    inline T& at(const std::string& key)
    {
        return at(key.c_str());
    }

    template<typename Predicate>
    void sort(iterator begin, iterator end, Predicate pred)
    {
        using namespace internal;

        std::sort(begin.m_it, end.m_it,
            [pred](const in_radix_leaf* a, const in_radix_leaf* b)
            {
                return pred(*in_get_data_ptr(*a), *in_get_data_ptr(*b));
            });
    }

    void clear() noexcept
    {
        in_destroy_data();
        m_rootNode = nullptr;
        m_leafNodes.clear();
    }

    radix_tree& operator=(const radix_tree& other)
    {
        if (&other != this)
        {
            clear();

            m_sortFuncPtr = other.m_sortFuncPtr;
            in_assign_nodes(other);
        }

        return *this;
    }

    radix_tree& operator=(radix_tree&& other) noexcept
    {
        if (&other != this)
        {
            in_destroy_data();

            in_radix_tree::operator=(std::move(other));
        }

        return *this;
    }

    const T& operator[](const char* key) const
    {
        const auto leaf = in_find_leaf(key, leaf_size());
        return *in_get_data_ptr(*leaf);
    }

    inline const T& operator[](const std::string& key) const
    {
        return operator[](key.c_str());
    }

    T& operator[](const char* key)
    {
        const auto leaf = in_find_leaf(key, leaf_size());
        return *in_get_data_ptr(*leaf);
    }

    inline T& operator[](const std::string& key)
    {
        return operator[](key.c_str());
    }

    radix_tree() noexcept = default;

    radix_tree(internal::in_radix_sort_func sortFunc) noexcept :
        in_radix_tree(sortFunc) {}

    radix_tree(const radix_tree& other) :
        in_radix_tree(other.m_sortFuncPtr)
    {
        in_assign_nodes(other);
    }

    radix_tree(radix_tree&& other) noexcept = default;

    inline ~radix_tree()
    {
        in_destroy_data();
    }
};
} // hl
#endif
