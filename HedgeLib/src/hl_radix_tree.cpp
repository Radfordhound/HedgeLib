#include "hedgelib/hl_radix_tree.h"
#include "hedgelib/hl_text.h"
#include <cstring>
#include <cassert>

namespace hl
{
namespace internal
{
struct in_radix_leaf_deleter
{
    inline void operator()(in_radix_leaf* leaf) const
    {
        ::operator delete(leaf);
    }
};

using in_radix_leaf_unique_ptr = std::unique_ptr<
    in_radix_leaf, in_radix_leaf_deleter>;

u8 in_radix_node::get_prefix_match_len(const char* key) const
{
    hl::u8 i = 0;
    for (;
        i < prefixLen && prefix[i] == key[i];
        ++i) {}

    return i;
}

void** in_radix_node::get_ptr_to_child_ptr(u8 key)
{
    switch (type())
    {
    case in_radix_node_type::node4:
    {
        auto& node4 = *static_cast<in_radix_node4*>(this);
        for (u8 i = 0; i < node4.small_node_child_count(); ++i)
        {
            if (node4.keys[i] == key)
            {
                return &node4.children[i];
            }
        }

        break;
    }

    case in_radix_node_type::node16:
    {
        auto& node16 = *static_cast<in_radix_node16*>(this);
        // TODO: Do an SSE2 comparison instead if possible.
        for (u8 i = 0; i < node16.small_node_child_count(); ++i)
        {
            if (node16.keys[i] == key)
            {
                return &node16.children[i];
            }
        }

        break;
    }

    case in_radix_node_type::node48:
    {
        auto& node48 = *static_cast<in_radix_node48*>(this);
        if (node48.childIndices[key] != UINT8_MAX)
        {
            return &node48.children[node48.childIndices[key]];
        }

        break;
    }

    case in_radix_node_type::node256:
    {
        auto& node256 = *static_cast<in_radix_node256*>(this);
        return &node256.children[key];
    }
    }

    return nullptr;
}

void* in_radix_node::get_child_ptr(u8 key) const
{
    const auto childPtrPtr = const_cast<in_radix_node*>(
        this)->get_ptr_to_child_ptr(key);

    return (childPtrPtr) ? *childPtrPtr : nullptr;
}

void in_radix_node::destroy() noexcept
{
    // Delete leaf nodes.
    if (is_leaf())
    {
        ::operator delete(this);
        return;
    }

    // Recursively delete child nodes.
    switch (type())
    {
    case in_radix_node_type::node4:
    {
        const auto node4 = static_cast<in_radix_node4*>(this);
        for (u8 i = 0; i < node4->small_node_child_count(); ++i)
        {
            static_cast<in_radix_node*>(node4->children[i])->destroy();
        }

        delete node4;
        break;
    }

    case in_radix_node_type::node16:
    {
        const auto node16 = static_cast<in_radix_node16*>(this);
        for (u8 i = 0; i < node16->small_node_child_count(); ++i)
        {
            static_cast<in_radix_node*>(node16->children[i])->destroy();
        }

        delete node16;
        break;
    }

    case in_radix_node_type::node48:
    {
        const auto node48 = static_cast<in_radix_node48*>(this);
        for (auto child : node48->children)
        {
            if (child) static_cast<in_radix_node*>(child)->destroy();
        }

        delete node48;
        break;
    }

    case in_radix_node_type::node256:
    {
        const auto node256 = static_cast<in_radix_node256*>(this);
        for (auto child : node256->children)
        {
            if (child) static_cast<in_radix_node*>(child)->destroy();
        }

        delete node256;
        break;
    }
    }
}

in_radix_node::in_radix_node(in_radix_node_type type) noexcept :
    flags(static_cast<u8>(type)),
    prefix() {}

in_radix_node::in_radix_node(in_radix_node_type newType, const in_radix_node& other) noexcept :
    flags(static_cast<u8>(newType) | other.small_node_child_count()),
    prefixLen(other.prefixLen),
    prefix(other.prefix) {}

in_radix_node4::in_radix_node4() noexcept :
    in_radix_node(in_radix_node_type::node4),
    keys(),
    children() {}

in_radix_node16::in_radix_node16() noexcept :
    in_radix_node(in_radix_node_type::node16),
    keys(),
    children() {}

in_radix_node16::in_radix_node16(const in_radix_node4& node4) noexcept :
    in_radix_node(in_radix_node_type::node16, node4)
{
    std::memcpy(keys.data(), node4.keys.data(), node4.keys.size());
    std::memcpy(children.data(), node4.children.data(),
        sizeof(void*) * node4.children.size());
}

u8 in_radix_node48::get_first_unused_child_index() const noexcept
{
    u8 i;
    for (i = 0; i < children.size() && children[i]; ++i) {}

    return i;
}

in_radix_node48::in_radix_node48() noexcept :
    in_radix_node(in_radix_node_type::node48),
    children({})
{
    // Setup default child indices.
    childIndices.fill(UINT8_MAX);
}

in_radix_node48::in_radix_node48(const in_radix_node16& node16) noexcept :
    in_radix_node(in_radix_node_type::node48, node16),
    children({})
{
    // Setup default child indices.
    childIndices.fill(UINT8_MAX);

    // Copy node16 child indices.
    u8 childCount = 0;
    for (const auto key : node16.keys)
    {
        childIndices[key] = childCount++;
    }

    // Copy child pointers.
    std::memcpy(children.data(), node16.children.data(),
        sizeof(void*) * node16.children.size());
}

in_radix_node256::in_radix_node256() noexcept :
    in_radix_node(in_radix_node_type::node256),
    children({}) {}

in_radix_node256::in_radix_node256(const in_radix_node48& node48) noexcept :
    in_radix_node(in_radix_node_type::node256, node48),
    children()
{
    // Copy child pointers.
    for (std::size_t i = 0; i < node48.childIndices.size(); ++i)
    {
        children[i] = (node48.childIndices[i] != UINT8_MAX) ?
            node48.children[node48.childIndices[i]] :
            nullptr;
    }
}

in_radix_leaf* in_radix_leaf::create(std::size_t size,
    std::size_t index, const char* key)
{
    // Get key length.
    const auto keyLen = std::strlen(key);

    // Ensure key length can fit within the keyLen field.
    assert(keyLen <= UINT32_MAX &&
        "The given key was too long!");

    // Allocate leaf node memory.
    const auto leaf = static_cast<in_radix_leaf*>(
        ::operator new(size + keyLen + 1));

    // Set node type, key length, and leaf index.
    leaf->type = UINT8_MAX;
    leaf->keyLen = static_cast<u32>(keyLen);
    leaf->leafIndex = index;

    // Copy key into leaf node buffer and return leaf node.
    std::memcpy(ptradd<char>(leaf, size), key, keyLen + 1);
    return leaf;
}

in_radix_tree::in_iterator in_radix_tree::in_add_leaf(in_radix_leaf& leaf)
{
    const auto leafNodeIndex = m_leafNodes.size();
    m_leafNodes.push_back(&leaf);
    return in_get_leaf(leafNodeIndex);
}

static void in_add_child_node(void** nodePtrPtr,
    in_radix_sort_func sortFuncPtr, u8 key, void* child)
{
    assert(!static_cast<in_radix_node*>(*nodePtrPtr)->is_leaf() &&
        "Cannot add children to leaf nodes!");

    switch (static_cast<in_radix_node*>(*nodePtrPtr)->type())
    {
    case in_radix_node_type::node4:
    {
        const auto node4 = static_cast<in_radix_node4*>(*nodePtrPtr);
        const auto childCount = node4->small_node_child_count();

        if (childCount < node4->children.size())
        {
            // Determine the index the child should be placed at using the correct sorting order.
            u8 i;
            for (i = 0;
                i < childCount && sortFuncPtr(key, node4->keys[i]) >= 0;
                ++i) {}

            // Shift key and child arrays 1 element to the right to make room for the child.
            std::move(node4->keys.begin() + i, node4->keys.end() - 1,
                node4->keys.begin() + i + 1);

            std::move(node4->children.begin() + i, node4->children.end() - 1,
                node4->children.begin() + i + 1);

            // Add child to the node4.
            node4->set_child_unchecked(i, key, child);
            ++node4->flags;
        }
        else
        {
            // Create new node16 from existing node4.
            auto node16 = std::unique_ptr<in_radix_node16>(
                new in_radix_node16(*node4));

            // Add child to the new node16.
            void* node16Ptr = node16.get();
            in_add_child_node(&node16Ptr, sortFuncPtr, key, child);
            node16.release();

            // Set the node pointer to the new node16.
            *nodePtrPtr = node16Ptr;

            // Delete the existing node4.
            delete node4;
        }

        break;
    }

    case in_radix_node_type::node16:
    {
        const auto node16 = static_cast<in_radix_node16*>(*nodePtrPtr);
        const auto childCount = node16->small_node_child_count();

        if (childCount < node16->children.size())
        {
            // TODO: Do an SSE2 comparison instead if possible.

            // Determine the index the child should be placed at using the correct sorting order.
            u8 i;
            for (i = 0;
                i < childCount && sortFuncPtr(key, node16->keys[i]) >= 0;
                ++i) {}

            // Shift key and child arrays 1 element to the right to make room for the child.
            std::move(node16->keys.begin() + i, node16->keys.end() - 1,
                node16->keys.begin() + i + 1);

            std::move(node16->children.begin() + i, node16->children.end() - 1,
                node16->children.begin() + i + 1);

            // Add child to the node16.
            node16->set_child_unchecked(i, key, child);
            ++node16->flags;
        }
        else
        {
            // Create new node48 from existing node16.
            auto node48 = std::unique_ptr<in_radix_node48>(
                new in_radix_node48(*node16));

            // Add child to the new node48.
            node48->set_child_unchecked(16, key, child);
            ++node48->flags;

            // Set the node pointer to the new node48.
            *nodePtrPtr = node48.release();

            // Delete the existing node16.
            delete node16;
        }

        break;
    }

    case in_radix_node_type::node48:
    {
        const auto node48 = static_cast<in_radix_node48*>(*nodePtrPtr);
        const auto childCount = node48->small_node_child_count();

        if (childCount < node48->children.size())
        {
            // Get the index of the first unused child.
            const auto childIndex = node48->get_first_unused_child_index();

            // Add child to the node48.
            node48->set_child_unchecked(childIndex, key, child);
            ++node48->flags;
        }
        else
        {
            // Create new node256 from existing node48.
            auto node256 = std::unique_ptr<in_radix_node256>(
                new in_radix_node256(*node48));

            // Add child to the new node256.
            node256->children[key] = child;

            // Delete the existing node48.
            delete node48;

            // Set the node pointer to the new node256.
            *nodePtrPtr = node256.release();
        }

        break;
    }

    case in_radix_node_type::node256:
    {
        // Add child to the existing node256.
        auto& node256 = *static_cast<in_radix_node256*>(*nodePtrPtr);
        node256.children[key] = child;
    }
    }
}

static std::unique_ptr<in_radix_node4> in_create_expanded_node(
    in_radix_sort_func sortFuncPtr,
    const char* key, const char* leafKey,
    in_radix_leaf& leaf, in_radix_leaf& newLeaf)
{
    // Create new node4.
    auto newNodePtr = std::unique_ptr<in_radix_node4>(new in_radix_node4());

    // Setup new node prefix.
    auto newPrefix = newNodePtr->prefix.begin();
    for (;
        newPrefix != newNodePtr->prefix.end() && *key == *leafKey;
        ++newPrefix, ++key, ++leafKey)
    {
        *newPrefix = *key;
    }

    // Setup new node prefix length.
    const auto newPrefixLen = static_cast<u8>(
        newPrefix - newNodePtr->prefix.begin());

    newNodePtr->prefixLen = newPrefixLen;

    // Add children to new node in the correct sorting order.
    const auto cmp = sortFuncPtr(*key, *leafKey);
    if (cmp > 0)
    {
        newNodePtr->set_child_unchecked(0, *leafKey, &leaf);
        newNodePtr->set_child_unchecked(1, *key, &newLeaf);
        newNodePtr->flags += 2;
    }
    else if (cmp < 0)
    {
        newNodePtr->set_child_unchecked(0, *key, &newLeaf);
        newNodePtr->set_child_unchecked(1, *leafKey, &leaf);
        newNodePtr->flags += 2;
    }
    else
    {
        assert(*key);

        auto childNodePtr = in_create_expanded_node(sortFuncPtr,
            key + 1, leafKey + 1, leaf, newLeaf);

        newNodePtr->set_child_unchecked(0, *key, childNodePtr.release());
        ++newNodePtr->flags;
    }

    return newNodePtr;
}

std::pair<in_radix_tree::in_iterator, bool> in_radix_tree::in_insert(
    const char* key, std::size_t leafSize)
{
    const auto origKey = key;
    void** nodePtrPtr = &m_rootNode;
    u32 depth = 0;

    while (*nodePtrPtr)
    {
        // If the current existing node is a leaf node, expand it.
        auto& node = *static_cast<in_radix_node*>(*nodePtrPtr);
        if (node.is_leaf())
        {
            // Get leaf and leaf key pointers.
            auto& leaf = *static_cast<in_radix_leaf*>(*nodePtrPtr);
            const char* leafKey = (ptradd<char>(&leaf, leafSize) + depth);

            // If key and leaf key are equal, just return the existing leaf.
            if (text::equal(key, leafKey))
            {
                return { in_get_leaf(leaf.leafIndex), false };
            }

            // Create a new leaf node.
            in_radix_leaf_unique_ptr newLeaf(in_radix_leaf::create(
                leafSize, m_leafNodes.size(), origKey));

            // Create a new expanded node.
            auto newNodePtr = in_create_expanded_node(m_sortFuncPtr,
                key, leafKey, leaf, *newLeaf);

            // Add new leaf to tree, update existing node pointer, and return new leaf iterator.
            const auto newLeafIt = in_add_leaf(*newLeaf);
            *nodePtrPtr = newNodePtr.release();
            newLeaf.release();
            return { newLeafIt, true };
        }

        // Otherwise, if the node's prefix is only a partial match, split the node.
        const auto prefixMatchLen = node.get_prefix_match_len(key);
        if (prefixMatchLen != node.prefixLen)
        {
            // Create a new node4.
            auto newNodePtr = std::unique_ptr<in_radix_node4>(new in_radix_node4());

            // Setup new node prefix.
            char* oldPrefix = node.prefix.data();
            char* newPrefix = newNodePtr->prefix.data();
            const char oldKey = oldPrefix[prefixMatchLen];

            std::memcpy(newPrefix, oldPrefix, prefixMatchLen);
            newNodePtr->prefixLen = prefixMatchLen;

            // Trim matching part of prefix from the old node's prefix.
            node.prefixLen -= (prefixMatchLen + 1);
            std::memmove(oldPrefix, oldPrefix + prefixMatchLen + 1, node.prefixLen);

            // Trim matching part from beginning of old prefix and key pointers.
            oldPrefix += prefixMatchLen;
            key += prefixMatchLen;

            // Create a new leaf node.
            in_radix_leaf_unique_ptr newLeaf(in_radix_leaf::create(
                leafSize, m_leafNodes.size(), origKey));

            // Add children to new node in the correct sorting order.
            if (m_sortFuncPtr(*key, oldKey) >= 0)
            {
                newNodePtr->set_child_unchecked(0, oldKey, &node);
                newNodePtr->set_child_unchecked(1, *key, newLeaf.get());
            }
            else
            {
                newNodePtr->set_child_unchecked(0, *key, newLeaf.get());
                newNodePtr->set_child_unchecked(1, oldKey, &node);
            }

            // Increase the new node's child count.
            newNodePtr->flags += 2;

            // Add new leaf to tree, update existing node pointer, and return new leaf iterator.
            const auto newLeafIt = in_add_leaf(*newLeaf);
            *nodePtrPtr = newNodePtr.release();
            newLeaf.release();
            return { newLeafIt, true };
        }

        // Increase depth.
        depth += node.prefixLen;
        key += node.prefixLen;

        // If there is no matching child node, add the leaf.
        const auto nextNodePtrPtr = node.get_ptr_to_child_ptr(*key);
        if (!nextNodePtrPtr)
        {
            in_radix_leaf_unique_ptr newLeaf(in_radix_leaf::create(
                leafSize, m_leafNodes.size(), origKey));

            in_add_child_node(nodePtrPtr, m_sortFuncPtr, *key, newLeaf.get());
            const auto newLeafIt = in_add_leaf(*newLeaf);
            newLeaf.release();

            return { newLeafIt, true };
        }

        // Otherwise, increase depth and recurse.
        else
        {
            nodePtrPtr = nextNodePtrPtr;
            if (*key)
            {
                ++depth;
                ++key;
            }
        }
    }

    // Add new leaf to tree, update existing node pointer, and return new leaf iterator.
    in_radix_leaf_unique_ptr newLeaf(in_radix_leaf::create(
        leafSize, m_leafNodes.size(), origKey));

    const auto newLeafIt = in_add_leaf(*newLeaf);
    *nodePtrPtr = newLeaf.release();
    return { newLeafIt, true };
}

const in_radix_leaf* in_radix_tree::in_find_leaf(
    const char* key, std::size_t leafSize) const
{
    void* nodePtr = m_rootNode;
    u32 depth = 0;

    while (nodePtr)
    {
        auto& node = *static_cast<const in_radix_node*>(nodePtr);
        if (node.is_leaf())
        {
            const auto leaf = static_cast<in_radix_leaf*>(nodePtr);
            const char* leafKey = (ptradd<char>(leaf, leafSize) + depth);

            return (text::equal(key, leafKey)) ? leaf : nullptr;
        }
        else if (node.get_prefix_match_len(key) != node.prefixLen)
        {
            return nullptr;
        }

        depth += node.prefixLen;
        key += node.prefixLen;

        nodePtr = node.get_child_ptr(*key);
        if (*key)
        {
            ++depth;
            ++key;
        }
    }

    return nullptr;
}

void in_radix_tree::in_destroy() noexcept
{
    if (m_rootNode)
    {
        static_cast<in_radix_node*>(m_rootNode)->destroy();
    }
}

in_radix_tree& in_radix_tree::operator=(in_radix_tree&& other) noexcept
{
    if (&other != this)
    {
        in_destroy();

        m_rootNode = other.m_rootNode;
        m_sortFuncPtr = other.m_sortFuncPtr;
        m_leafNodes = std::move(other.m_leafNodes);
        
        other.m_rootNode = nullptr;
    }
    
    return *this;
}

in_radix_tree::in_radix_tree() noexcept :
    m_sortFuncPtr(text::compare<unsigned char>) {}

in_radix_tree::in_radix_tree(in_radix_tree&& other) noexcept :
    m_rootNode(other.m_rootNode),
    m_sortFuncPtr(other.m_sortFuncPtr),
    m_leafNodes(std::move(other.m_leafNodes))
{
    other.m_rootNode = nullptr;
}
} // internal
} // hl
