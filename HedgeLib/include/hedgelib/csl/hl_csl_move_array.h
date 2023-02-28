#ifndef HL_CSL_MOVE_ARRAY_H_INCLUDED
#define HL_CSL_MOVE_ARRAY_H_INCLUDED
#include "../hl_internal.h"

namespace hl
{
namespace csl
{
template<typename T>
struct move_array32
{
    using value_type = T;
    using size_type = u32;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = typename off32<T>::reference;
    using const_reference = typename off32<T>::const_reference;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    off32<T> dataPtr;
    u32 count;
    u32 capacity;
    off32<void> allocatorPtr;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(dataPtr);
        hl::endian_swap<swapOffsets>(count);
        hl::endian_swap<swapOffsets>(capacity);
        hl::endian_swap<swapOffsets>(allocatorPtr);
    }

    inline u32 size() const noexcept
    {
        return count;
    }

    inline const_pointer data() const noexcept
    {
        return dataPtr.get();
    }

    inline pointer data() noexcept
    {
        return dataPtr.get();
    }

    inline bool empty() const noexcept
    {
        return (count != 0);
    }

    inline const_iterator begin() const noexcept
    {
        return dataPtr.get();
    }

    inline iterator begin() noexcept
    {
        return dataPtr.get();
    }

    inline const_iterator end() const noexcept
    {
        return (dataPtr.get() + count);
    }

    inline iterator end() noexcept
    {
        return (dataPtr.get() + count);
    }

    inline const_reference front() const
    {
        return *dataPtr;
    }

    inline reference front()
    {
        return *dataPtr;
    }

    inline const_reference back() const
    {
        return dataPtr[count - 1];
    }

    inline reference back()
    {
        return dataPtr[count - 1];
    }

    inline const_reference operator[](size_type pos) const
    {
        return (dataPtr.get() + pos);
    }

    inline reference operator[](size_type pos)
    {
        return (dataPtr.get() + pos);
    }

    inline bool operator==(const move_array32& other) const noexcept
    {
        return (dataPtr == other.dataPtr && count == other.count);
    }

    inline bool operator!=(const move_array32& other) const noexcept
    {
        return (dataPtr != other.dataPtr || count != other.count);
    }

    inline move_array32() noexcept = default;

    move_array32(off32<T> dataPtr, u32 count) noexcept :
        dataPtr(dataPtr),
        count(count),
        capacity(count),
        allocatorPtr(nullptr) {}

    move_array32(off32<T> dataPtr, u32 count, u32 capacity,
        off32<void> allocatorPtr = nullptr) noexcept :
        dataPtr(dataPtr),
        count(count),
        capacity(capacity),
        allocatorPtr(allocatorPtr) {} 
};

template<typename T>
struct move_array64
{
    using value_type = T;
    using size_type = u64;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = typename off64<T>::reference;
    using const_reference = typename off64<T>::const_reference;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    off64<T> dataPtr;
    u64 count;
    u64 capacity;
    off64<void> allocatorPtr;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(dataPtr);
        hl::endian_swap<swapOffsets>(count);
        hl::endian_swap<swapOffsets>(capacity);
        hl::endian_swap<swapOffsets>(allocatorPtr);
    }

    inline u64 size() const noexcept
    {
        return count;
    }

    inline const_pointer data() const noexcept
    {
        return dataPtr.get();
    }

    inline pointer data() noexcept
    {
        return dataPtr.get();
    }

    inline bool empty() const noexcept
    {
        return (count != 0);
    }

    inline const_iterator begin() const noexcept
    {
        return dataPtr.get();
    }

    inline iterator begin() noexcept
    {
        return dataPtr.get();
    }

    inline const_iterator end() const noexcept
    {
        return (dataPtr.get() + count);
    }

    inline iterator end() noexcept
    {
        return (dataPtr.get() + count);
    }

    inline const_reference front() const
    {
        return *dataPtr;
    }

    inline reference front()
    {
        return *dataPtr;
    }

    inline const_reference back() const
    {
        return dataPtr[count - 1];
    }

    inline reference back()
    {
        return dataPtr[count - 1];
    }

    inline const_reference operator[](size_type pos) const
    {
        return (dataPtr.get() + pos);
    }

    inline reference operator[](size_type pos)
    {
        return (dataPtr.get() + pos);
    }

    inline bool operator==(const move_array64& other) const noexcept
    {
        return (dataPtr == other.dataPtr && count == other.count);
    }

    inline bool operator!=(const move_array64& other) const noexcept
    {
        return (dataPtr != other.dataPtr || count != other.count);
    }

    inline move_array64() noexcept = default;

    move_array64(off64<T> dataPtr, u64 count) noexcept :
        dataPtr(dataPtr),
        count(count),
        capacity(count),
        allocatorPtr(nullptr) {}

    move_array64(off64<T> dataPtr, u64 count, u64 capacity,
        off64<void> allocatorPtr = nullptr) noexcept :
        dataPtr(dataPtr),
        count(count),
        capacity(capacity),
        allocatorPtr(allocatorPtr) {} 
};
} // csl
} // hl
#endif
