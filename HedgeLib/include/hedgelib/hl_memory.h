#ifndef HL_MEMORY_H_INCLUDED
#define HL_MEMORY_H_INCLUDED
#include "hl_internal.h"
#include <new>
#include <atomic>
#include <memory>
#include <utility>
#include <initializer_list>
#include <cstdlib>

#ifndef _MSC_VER
#include <cstring>
#endif

namespace hl
{
namespace internal
{
/**
 * @brief Return an rvalue reference to x if To can be nothrow
 * constructed from From&& or if To can be constructed from const From&.
 * Otherwise, returns a constant lvalue reference to x.
 * 
 * Usage: `::new (&a) TypeOfA(in_move_if_noexcept_construct<TypeOfA>(b));`
 * 
 * @tparam To The type to check against to see if it can be nothrow-constructed from x.
 * @tparam From The type of x.
 * @param x The value to obtain an rvalue or lvalue reference to.
 * @return An rvalue reference to x if To can be nothrow constructed
 * from From&& or if To can be constructed from const From&. Otherwise, a
 * constant lvalue reference to x.
 */
template<typename To, typename From>
[[nodiscard]] constexpr std::conditional_t<
    !std::is_nothrow_constructible_v<To, std::add_rvalue_reference_t<From>> &&
    std::is_constructible_v<To, std::add_lvalue_reference_t<std::add_const_t<From>>>,
    const From&, From&&>
    in_move_if_noexcept_construct(From& x) noexcept
{
    return std::move(x);
}

/**
 * @brief Return an rvalue reference to x if From&& is nothrow
 * assignable to To& or if const From& is not assignable to To&.
 * Otherwise, returns a constant lvalue reference to x.
 * 
 * Usage: `a = in_move_if_noexcept_assign<TypeOfA>(b);`
 * 
 * @tparam To The type to check against to see if it can be nothrow assigned from x.
 * @tparam From The type of x.
 * @param x The value to obtain an rvalue or lvalue reference to.
 * @return An rvalue reference to x if From&& is nothrow assignable to
 * To& or if const From& is not assignable to To&. Otherwise, a constant
 * lvalue reference to x.
 */
template<typename To, typename From>
[[nodiscard]] constexpr std::conditional_t<
    !std::is_nothrow_assignable_v<std::add_lvalue_reference_t<To>, std::add_rvalue_reference_t<From>> &&
    std::is_assignable_v<std::add_lvalue_reference_t<To>, std::add_lvalue_reference_t<std::add_const_t<From>>>,
    const From&, From&&>
    in_move_if_noexcept_assign(From& x) noexcept
{
    return std::move(x);
}

template<typename InputIt, typename OutputIt = InputIt>
struct in_is_nothrow_uninitialized_movable : std::bool_constant<
    std::is_nothrow_constructible_v<
        typename std::iterator_traits<OutputIt>::value_type,
        std::add_rvalue_reference_t<
            typename std::iterator_traits<InputIt>::value_type>> ||

    std::is_nothrow_constructible_v<
        typename std::iterator_traits<OutputIt>::value_type,
        std::add_lvalue_reference_t<std::add_const_t<
            typename std::iterator_traits<InputIt>::value_type>>>> {};

template<typename InputIt, typename OutputIt = InputIt>
inline constexpr bool in_is_nothrow_uninitialized_movable_v =
    in_is_nothrow_uninitialized_movable<InputIt, OutputIt>::value;

template<typename InputIt, typename OutputIt = InputIt>
struct in_is_nothrow_movable : std::bool_constant<
    std::is_nothrow_assignable_v<
        std::add_lvalue_reference_t<
            typename std::iterator_traits<OutputIt>::value_type>,
        std::add_rvalue_reference_t<
            typename std::iterator_traits<InputIt>::value_type>> ||

    std::is_nothrow_assignable_v<
        std::add_lvalue_reference_t<
            typename std::iterator_traits<OutputIt>::value_type>,
        std::add_lvalue_reference_t<std::add_const_t<
            typename std::iterator_traits<InputIt>::value_type>>>> {};

template<typename InputIt, typename OutputIt = InputIt>
inline constexpr bool in_is_nothrow_movable_v =
    in_is_nothrow_movable<InputIt, OutputIt>::value;

template<typename InputIt, typename OutputIt = InputIt>
struct in_is_nothrow_iterable : std::bool_constant<
    noexcept(++std::declval<InputIt&>()) &&                         // nothrow ++input
    noexcept(++std::declval<OutputIt&>()) &&                        // nothrow ++output
    noexcept(*std::declval<InputIt&>()) &&                          // nothrow *input
    noexcept(*std::declval<OutputIt&>()) &&                         // nothrow *output
    noexcept(std::declval<InputIt&>() != std::declval<InputIt&>())  // nothrow input != input
    > {};

template<typename InputIt, typename OutputIt = InputIt>
inline constexpr bool in_is_nothrow_iterable_v = in_is_nothrow_iterable<InputIt, OutputIt>::value;
} // internal

template<typename T>
inline void destruct(T& obj) noexcept
{
    // NOTE: This function only exists because
    // std::destroy_at doesn't handle arrays pre-C++20.
    if constexpr (std::is_array_v<T>)
    {
        for (auto& v : obj)
        {
            destruct(v);
        }
    }
    else
    {
        obj.~T();
    }
}

template<typename Iterator>
void destruct(Iterator begin, Iterator end) noexcept(
    internal::in_is_nothrow_iterable_v<Iterator>)
{
    // NOTE: This function only exists because
    // std::destroy doesn't handle arrays pre-C++20.
    for (; begin != end; ++begin)
    {
        destruct(*begin);
    }
}

template<typename Iterator, typename NoThrowForwardIt>
NoThrowForwardIt uninitialized_move_strong(Iterator begin,
    Iterator end, NoThrowForwardIt dst) noexcept(
        internal::in_is_nothrow_uninitialized_movable_v<Iterator, NoThrowForwardIt> &&
        internal::in_is_nothrow_iterable_v<Iterator, NoThrowForwardIt>)
{
    using namespace internal;
    using T = typename std::iterator_traits<NoThrowForwardIt>::value_type;

    /* nothrow */
    if constexpr (
        in_is_nothrow_uninitialized_movable_v<Iterator, NoThrowForwardIt> &&
        in_is_nothrow_iterable_v<Iterator, NoThrowForwardIt>)
    {
        for (; begin != end; ++begin, (void)++dst)
        {
            ::new (const_cast<void*>(static_cast<const volatile void*>(
                std::addressof(*dst)))) T(in_move_if_noexcept_construct<T>(*begin));
        }

        return dst;
    }

    /* maybe throw */
    else
    {
        NoThrowForwardIt it(dst); // NOTE: Iterator copy-construction itself can throw.
        try
        {
            for (; begin != end; ++begin, (void)++it)
            {
                ::new (const_cast<void*>(static_cast<const volatile void*>(
                    std::addressof(*it)))) T(in_move_if_noexcept_construct<T>(*begin));
            }

            return it;
        }
        catch (...)
        {
            // TODO: What if the iterators throw here?
            destruct(dst, it);
            throw;
        }
    }
}

template<typename Iterator, typename... Args>
void uninitialized_direct_construct(Iterator begin, Iterator end, const Args&... args)
    noexcept(internal::in_is_nothrow_iterable_v<Iterator> && std::is_nothrow_constructible_v<
        typename std::iterator_traits<Iterator>::value_type, const Args&...>)
{
    using namespace internal;
    using T = typename std::iterator_traits<Iterator>::value_type;

    /* nothrow */
    if constexpr (in_is_nothrow_iterable_v<Iterator> && std::is_nothrow_constructible_v<
        typename std::iterator_traits<Iterator>::value_type, const Args&...>)
    {
        for (; begin != end; ++begin)
        {
            ::new (const_cast<void*>(static_cast<const volatile void*>(
                std::addressof(*begin)))) T(args...);
        }
    }

    /* maybe throw */
    else
    {
        Iterator it(begin); // NOTE: Iterator copy-construction itself can throw.
        try
        {
            for (; it != end; ++it)
            {
                ::new (const_cast<void*>(static_cast<const volatile void*>(
                    std::addressof(*it)))) T(args...);
            }
        }
        catch (...)
        {
            // TODO: What if the iterators throw here?
            destruct(begin, it);
            throw;
        }
    }
}

/**
 * @brief Moves the elements from the given input range to
 * the given output destination, with strong exception guarantee.
 * 
 * Like std::move from <algorithm>, except with a strong exception guarantee.
 * 
 * The input range will be move-assigned to the output, unless the input is not
 * nothrow-move-assignable to the output, in which case, the input will be copy-assigned
 * to the output instead, unless the input can ONLY be move-assigned AND is not
 * nothrow-move-assignable, in which case, the input must be move-assigned and the
 * strong exception guarantee will be waived.
 * 
 * If the input is nothrow-move-assignable or nothrow-copy-assignable to the output, and
 * the input and output iterators do not throw, this function is marked as noexcept, and
 * is guaranteed to never throw.
 * 
 * Otherwise, this function may throw, but guarantees that if this happens, the input will
 * ALWAYS be left in the same state as it was before this function was called, unless the
 * strong exception guarantee was waived as described above.
 * 
 * @tparam InputIt The type of iterator used to iterate over the given input range.
 * @tparam OutputIt The type of iterator used to iterate over the given output.
 * @param begin The first element in the input range.
 * @param end One past the last element in the input range.
 * @param dst The first element in the output range.
 * @return OutputIt One past the last element in the output range.
 */
template<typename InputIt, typename OutputIt>
OutputIt move_strong(InputIt begin, InputIt end, OutputIt dst) noexcept(
    internal::in_is_nothrow_movable_v<InputIt, OutputIt> &&
    internal::in_is_nothrow_iterable_v<InputIt, OutputIt>)
{
    using T = typename std::iterator_traits<OutputIt>::value_type;

    for (; begin != end; ++begin, (void)++dst)
    {
        // Try to nothrow move-assign. Fallback to copy-assign, unless
        // type is not copy-assignable, in which case, fallback again
        // to move-assign and waive strong exception guarantee.
        *dst = internal::in_move_if_noexcept_assign<T>(*begin);
    }

    return dst;
}

[[nodiscard]] inline void* allocate(std::size_t size, const std::nothrow_t) noexcept
{
#ifdef _MSC_VER
    return _aligned_malloc(size, __STDCPP_DEFAULT_NEW_ALIGNMENT__);
#else
    return std::malloc(size);
#endif
}

[[nodiscard]] inline void* allocate(std::size_t size)
{
    const auto mem = allocate(size, std::nothrow);
    if (!mem)
    {
        throw std::bad_alloc();
    }

    return mem;
}

[[nodiscard]] inline void* allocate(std::size_t size, std::size_t alignment, const std::nothrow_t) noexcept
{
#ifdef _MSC_VER
    return _aligned_malloc(size, alignment);
#else
    return std::aligned_alloc(alignment, size);
#endif
}

[[nodiscard]] inline void* allocate(std::size_t size, std::size_t alignment)
{
    const auto mem = allocate(size, alignment, std::nothrow);
    if (!mem)
    {
        throw std::bad_alloc();
    }

    return mem;
}

template<typename T>
[[nodiscard]] inline T* allocate(std::size_t count, const std::nothrow_t) noexcept
{
    if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
    {
        return static_cast<T*>(allocate(sizeof(T) * count,
            alignof(T), std::nothrow));
    }
    else
    {
        return static_cast<T*>(allocate(sizeof(T) * count,
            std::nothrow));
    }
}

template<typename T>
[[nodiscard]] inline T* allocate(std::size_t count)
{
    if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
    {
        return static_cast<T*>(allocate(sizeof(T) * count, alignof(T)));
    }
    else
    {
        return static_cast<T*>(allocate(sizeof(T) * count));
    }
}

[[nodiscard]] inline void* reallocate(void* ptr, std::size_t size, const std::nothrow_t) noexcept
{
#ifdef _MSC_VER
    return _aligned_realloc(ptr, size, __STDCPP_DEFAULT_NEW_ALIGNMENT__);
#else
    return std::realloc(ptr, size);
#endif
}

[[nodiscard]] inline void* reallocate(void* ptr, std::size_t size)
{
    const auto mem = reallocate(ptr, size, std::nothrow);
    if (!mem)
    {
        throw std::bad_alloc();
    }

    return mem;
}

[[nodiscard]] inline void* reallocate(void* ptr, std::size_t size,
    std::size_t alignment, const std::nothrow_t) noexcept
{
#ifdef _MSC_VER
    return _aligned_realloc(ptr, size, alignment);
#else
    const auto mem = std::aligned_alloc(alignment, size);
    if (mem)
    {
        std::memcpy(mem, ptr, size);
        std::free(ptr);
    }

    return mem;
#endif
}

[[nodiscard]] inline void* reallocate(void* ptr, std::size_t size, std::size_t alignment)
{
    const auto mem = reallocate(ptr, size, alignment, std::nothrow);
    if (!mem)
    {
        throw std::bad_alloc();
    }

    return mem;
}

template<typename T>
[[nodiscard]] inline T* reallocate(T* ptr, std::size_t count, const std::nothrow_t) noexcept
{
    if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
    {
        return static_cast<T*>(reallocate(
            static_cast<void*>(ptr),
            sizeof(T) * count, alignof(T),
            std::nothrow));
    }
    else
    {
        return static_cast<T*>(reallocate(
            static_cast<void*>(ptr),
            sizeof(T) * count,
            std::nothrow));
    }
}

template<typename T>
[[nodiscard]] inline T* reallocate(T* ptr, std::size_t count)
{
    if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
    {
        return static_cast<T*>(reallocate(
            static_cast<void*>(ptr),
            sizeof(T) * count, alignof(T)));
    }
    else
    {
        return static_cast<T*>(reallocate(
            static_cast<void*>(ptr),
            sizeof(T) * count));
    }
}

inline void free(void* ptr) noexcept
{
#ifdef _MSC_VER
    _aligned_free(ptr);
#else
    std::free(ptr);
#endif
}

template<typename T>
[[nodiscard]] T* create(const no_value_init_t, std::size_t count)
{
    // Allocate memory.
    const auto data = allocate<T>(count);

    // Default-construct objects if necessary.
    if constexpr (!std::is_trivially_default_constructible_v<T>)
    {
        try
        {
            std::uninitialized_default_construct(data, data + count);
        }
        catch (...)
        {
            hl::free(data);
            throw;
        }
    }

    return data;
}

template<typename T>
[[nodiscard]] T* create(std::size_t count)
{
    // Allocate memory.
    const auto data = allocate<T>(count);

    // Value-construct objects.
    try
    {
        std::uninitialized_value_construct(data, data + count);
    }
    catch (...)
    {
        hl::free(data);
        throw;
    }

    return data;
}

template<typename T, typename... Args>
[[nodiscard]] T* create(std::size_t count, const Args&... args)
{
    // Allocate memory.
    const auto data = allocate<T>(count);

    // Direct-construct objects.
    try
    {
        uninitialized_direct_construct(data, data + count, args...);
    }
    catch (...)
    {
        hl::free(data);
        throw;
    }

    return data;
}

template<typename Iterator>
[[nodiscard]] typename std::iterator_traits<Iterator>::value_type*
    create(Iterator begin, Iterator end)
{
    // Allocate memory.
    using T = typename std::iterator_traits<Iterator>::value_type;
    const auto data = allocate<T>(static_cast<std::size_t>(end - begin));

    // Copy-construct objects.
    try
    {
        std::uninitialized_copy(begin, end, data);
    }
    catch (...)
    {
        hl::free(data);
        throw;
    }

    return data;
}

template<typename T>
[[nodiscard]] T* resize(const no_value_init_t,
    T* arr, std::size_t oldCount, std::size_t newCount)
{
    // For trivially-copyable types, just reallocate memory.
    if constexpr (std::is_trivially_copyable_v<T>)
    {
        return reallocate<T>(arr, newCount);
    }

    // For non-trivially-copyable types, allocate a new memory
    // block and move the existing data into it if necessary.
    else if (newCount > oldCount)
    {
        // Allocate a new memory block.
        const auto newArr = allocate<T>(newCount);

        // Default-construct new elements.
        // NOTE: We do this BEFORE move-constructing existing elements
        // as this allows us to avoid having to move back if we throw.
        try
        {
            std::uninitialized_default_construct(
                newArr + oldCount, newArr + newCount);
        }
        catch (...)
        {
            hl::free(newArr);
            throw;
        }

        // Move-construct existing elements into the new memory block.
        try
        {
            uninitialized_move_strong(arr, arr + oldCount, newArr);
        }
        catch (...)
        {
            destruct(newArr + oldCount, newArr + newCount);
            hl::free(newArr);
            throw;
        }

        // Destruct existing elements and return new memory block.
        destruct(arr, arr + oldCount);
        return newArr;
    }

    // Otherwise, just destruct the extra elements.
    else
    {
        destruct(arr + newCount, arr + oldCount);
        return arr;
    }
}

template<typename T, typename... Args>
[[nodiscard]] T* resize(T* arr, std::size_t oldCount,
    std::size_t newCount, const Args&... args)
{
    // For types that are trivially-copyable and trivially-direct-constructible,
    // just reallocate memory and direct-construct new elements.
    if constexpr (std::is_trivially_copyable_v<T> && noexcept(
        uninitialized_direct_construct(arr, arr, args...)))
    {
        // Re-allocate the memory block.
        const auto newArr = reallocate<T>(arr, newCount);

        // Direct-construct new elements.
        if (newCount > oldCount)
        {
            uninitialized_direct_construct(newArr + oldCount,
                newArr + newCount, args...);
        }

        return newArr;
    }

    // For other types, allocate a new memory block and
    // move the existing data into it if necessary.
    else if (newCount > oldCount)
    {
        // Allocate a new memory block.
        const auto newArr = allocate<T>(newCount);

        // Direct-construct new elements.
        // NOTE: We do this BEFORE move-constructing existing elements
        // as this allows us to avoid having to move back if we throw.
        try
        {
            uninitialized_direct_construct(newArr + oldCount,
                newArr + newCount, args...);
        }
        catch (...)
        {
            hl::free(newArr);
            throw;
        }

        // Move-construct existing elements into the new memory block.
        try
        {
            uninitialized_move_strong(arr, arr + oldCount, newArr);
        }
        catch (...)
        {
            destruct(newArr + oldCount, newArr + newCount);
            hl::free(newArr);
            throw;
        }

        // Destruct existing elements and return new memory block.
        destruct(arr, arr + oldCount);
        return newArr;
    }

    // Otherwise, just destruct the extra elements.
    else
    {
        destruct(arr + newCount, arr + oldCount);
        return arr;
    }
}

template<typename T>
void destroy_unchecked(T* obj) noexcept
{
    // Destruct object if necessary.
    if constexpr (!std::is_trivially_destructible_v<T>)
    {
        destruct(obj);
    }

    // Free memory.
    hl::free(obj);
}

template<typename T>
void destroy_unchecked(T* arr, std::size_t count) noexcept
{
    // Destruct objects if necessary.
    if constexpr (!std::is_trivially_destructible_v<T>)
    {
        destruct(arr, arr + count);
    }

    // Free memory.
    hl::free(arr);
}

template<typename T>
void destroy(T* obj) noexcept
{
    // Destruct object if necessary.
    if constexpr (!std::is_trivially_destructible_v<T>)
    {
        if (obj)
        {
            destruct(obj);
        }
    }

    // Free memory.
    hl::free(obj);
}

template<typename T>
void destroy(T* arr, std::size_t count) noexcept
{
    // Destruct objects if necessary.
    if constexpr (!std::is_trivially_destructible_v<T>)
    {
        if (arr)
        {
            destruct(arr, arr + count);
        }
    }

    // Free memory.
    hl::free(arr);
}

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

template<typename T, std::size_t MaxStackCount>
class stack_or_heap_memory
{
    T*                  m_data;
    union
    {
        T               m_stackBuf[MaxStackCount];
        std::size_t     m_heapCount;
    };

    // HACK: If T is not trivially default-constructible or is not
    // trivially destructible, use only the [-count .. count] range
    // of the stack buffer to avoid unnecessary construction/destruction.
    static constexpr bool in_use_entire_stack_buf = (
        std::is_trivially_default_constructible_v<T> &&
        std::is_trivially_destructible_v<T>);

    inline std::size_t in_get_stack_buf_range() const noexcept
    {
        if constexpr (in_use_entire_stack_buf)
        {
            return MaxStackCount;
        }
        else
        {
            return (std::end(m_stackBuf) - m_data);
        }
    }

    inline T* in_alloc_stack_buf_range(std::size_t count) noexcept
    {
        // Use entire stack buffer (all elements HAVE to be default-constructed).
        if constexpr (in_use_entire_stack_buf)
        {
            return m_stackBuf;
        }

        // Use only [-count .. count] range of stack buffer.
        else
        {
            return (std::end(m_stackBuf) - count);
        }
    }

    [[nodiscard]] T* in_alloc_memory(std::size_t count)
    {
        // Use the stack buffer if possible.
        if (count <= MaxStackCount)
        {
            return in_alloc_stack_buf_range(count);
        }

        // Otherwise, allocate uninitialized memory on
        // the heap with the proper size and alignment.
        else
        {
            m_heapCount = count;
            return allocate<T>(count);
        }
    }

    /* Non-heap-allocating move operation */
    template<std::size_t OtherMaxStackCount>
    std::enable_if_t<(OtherMaxStackCount <= MaxStackCount)>
        in_take_ownership(stack_or_heap_memory<T, OtherMaxStackCount>&& other)
        noexcept(internal::in_is_nothrow_uninitialized_movable_v<T*>)
    {
        if (other.is_stack())
        {
            // "Allocate" enough room in the stack buffer to take ownership of the data.
            const auto count = other.in_get_stack_buf_range();
            m_data = in_alloc_stack_buf_range(count);

            // Move-construct stack buffer from other stack buffer.
            if constexpr (internal::in_is_nothrow_uninitialized_movable_v<T*>)
            {
                uninitialized_move_strong(other.m_data,
                    other.m_data + count, m_data);
            }
            else
            {
                try
                {
                    uninitialized_move_strong(other.m_data,
                        other.m_data + count, m_data);
                }
                catch (...)
                {
                    m_data = nullptr;
                    m_heapCount = 0;
                    throw;
                }
            }
        }
        else
        {
            // Take ownership of other heap memory.
            m_data = other.m_data;
            m_heapCount = other.m_heapCount;

            other.m_data = nullptr;
            other.m_heapCount = 0;
        }
    }

    /* Maybe-heap-allocating move operation */
    template<std::size_t OtherMaxStackCount>
    std::enable_if_t<(OtherMaxStackCount > MaxStackCount)>
        in_take_ownership(stack_or_heap_memory<T, OtherMaxStackCount>&& other)
    {
        if (other.is_stack())
        {
            // Allocate enough memory to take ownership of the buffer.
            const auto count = other.in_get_stack_buf_range();
            try
            {
                m_data = in_alloc_memory(count);
            }
            catch (...)
            {
                m_data = nullptr;
                m_heapCount = 0;
                throw;
            }

            // Move-construct from other stack buffer.
            try
            {
                uninitialized_move_strong(other.m_data,
                    other.m_data + count, m_data);
            }
            catch (...)
            {
                in_free_memory_if_heap(count);
                m_data = nullptr;
                m_heapCount = 0;
                throw;
            }
        }
        else
        {
            // Take ownership of other heap memory.
            m_data = other.m_data;
            m_heapCount = other.m_heapCount;

            other.m_data = nullptr;
            other.m_heapCount = 0;
        }
    }

    void in_destroy() noexcept
    {
        // Free memory if it was allocated on the heap.
        if (!is_stack())
        {
            // Destruct objects if necessary.
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                destruct(m_data, m_data + m_heapCount);
            }

            // Free memory.
            hl::free(m_data);
        }
        
        // Destruct elements if necessary.
        // NOTE: If in_use_entire_stack_buf is true, then
        // the data is trivially destructible and does
        // not need to be destructed.
        else if constexpr (!in_use_entire_stack_buf)
        {
            destruct(m_data, std::end(m_stackBuf));
        }
    }

    inline void in_free_memory_if_heap(std::size_t count) noexcept
    {
        if (count > MaxStackCount)
        {
            hl::free(m_data);
        }
    }

public:
    using value_type                = T;
    using pointer                   = T*;
    using const_pointer             = const T*;
    using iterator                  = pointer;
    using const_iterator            = const_pointer;
    using reverse_iterator          = std::reverse_iterator<iterator>;
    using const_reverse_iterator    = std::reverse_iterator<const_iterator>;

    inline const T* data() const noexcept
    {
        return m_data;
    }

    inline T* data() noexcept
    {
        return m_data;
    }

    inline std::size_t size() const noexcept
    {
        return (is_stack()) ?
            in_get_stack_buf_range() :
            m_heapCount;
    }

    inline const_iterator cbegin() const noexcept
    {
        return data();
    }

    inline const_iterator begin() const noexcept
    {
        return data();
    }

    inline iterator begin() noexcept
    {
        return data();
    }

    inline const_iterator cend() const noexcept
    {
        return (data() + size());
    }

    inline const_iterator end() const noexcept
    {
        return (data() + size());
    }

    inline iterator end() noexcept
    {
        return (data() + size());
    }

    inline bool is_stack() const noexcept
    {
        if constexpr (in_use_entire_stack_buf)
        {
            return (m_data == m_stackBuf);
        }
        else
        {
            // HACK: This check is only valid on platforms which use a
            // flat memory model (though this is basically everything).
            const auto data = reinterpret_cast<std::uintptr_t>(m_data);
            return (data >= reinterpret_cast<std::uintptr_t>(m_stackBuf) &&
                data < reinterpret_cast<std::uintptr_t>(std::end(m_stackBuf)));
        }
    }

    // TODO: Make an assign function.

    inline const T& operator[](std::size_t pos) const
    {
        return m_data[pos];
    }

    inline T& operator[](std::size_t pos)
    {
        return m_data[pos];
    }

    stack_or_heap_memory& operator=(const stack_or_heap_memory& other)
    {
        if (&other != this)
        {
            in_destroy();

            const auto count = other.size();

            try
            {
                m_data = in_alloc_memory(count);
            }
            catch (...)
            {
                m_data = nullptr;
                m_heapCount = 0;
                throw;
            }

            try
            {
                std::uninitialized_copy(other.m_data,
                    other.m_data + count, m_data);
            }
            catch (...)
            {
                in_free_memory_if_heap(count);
                m_data = nullptr;
                m_heapCount = 0;
                throw;
            }
        }

        return *this;
    }

    template<std::size_t OtherMaxStackCount>
    stack_or_heap_memory& operator=(stack_or_heap_memory<T, OtherMaxStackCount>&& other)
        noexcept((OtherMaxStackCount <= MaxStackCount) &&
            std::is_nothrow_move_constructible_v<T>)
    {
        if (&other != this)
        {
            in_destroy();
            in_take_ownership(std::move(other));
        }

        return *this;
    }

    stack_or_heap_memory() noexcept :
        m_data(nullptr),
        m_heapCount(0) {}

    explicit stack_or_heap_memory(const no_value_init_t, std::size_t count) :
        m_data(in_alloc_memory(count))
    {
        if constexpr (!std::is_trivially_default_constructible_v<T>)
        {
            try
            {
                std::uninitialized_default_construct(m_data, m_data + count);
            }
            catch (...)
            {
                in_free_memory_if_heap(count);
                throw;
            }
        }
    }

    explicit stack_or_heap_memory(std::size_t count) :
        m_data(in_alloc_memory(count))
    {
        try
        {
            std::uninitialized_value_construct(m_data, m_data + count);
        }
        catch (...)
        {
            in_free_memory_if_heap(count);
            throw;
        }
    }

    template<typename... Args>
    stack_or_heap_memory(std::size_t count, const Args&... args) :
        m_data(in_alloc_memory(count))
    {
        try
        {
            uninitialized_direct_construct(m_data, m_data + count, args...);
        }
        catch (...)
        {
            in_free_memory_if_heap(count);
            throw;
        }
    }

    template<typename Iterator>
    stack_or_heap_memory(Iterator begin, Iterator end) :
        m_data(in_alloc_memory(static_cast<std::size_t>(end - begin)))
    {
        try
        {
            std::uninitialized_copy(begin, end, m_data);
        }
        catch (...)
        {
            in_free_memory_if_heap(static_cast<std::size_t>(end - begin));
            throw;
        }
    }

    stack_or_heap_memory(std::initializer_list<T> ilist) :
        m_data(in_alloc_memory(ilist.size()))
    {
        try
        {
            std::uninitialized_copy(ilist.begin(), ilist.end(), m_data);
        }
        catch (...)
        {
            in_free_memory_if_heap(ilist.size());
            throw;
        }
    }

    stack_or_heap_memory(const stack_or_heap_memory& other) :
        m_data(in_alloc_memory(other._size()))
    {
        const auto count = other._size();
        try
        {
            std::uninitialized_copy(other.m_data,
                other.m_data + count, m_data);
        }
        catch (...)
        {
            in_free_memory_if_heap(count);
            throw;
        }
    }

    template<std::size_t OtherMaxStackCount>
    inline stack_or_heap_memory(stack_or_heap_memory<T, OtherMaxStackCount>&& other)
        noexcept(in_take_ownership(std::declval<stack_or_heap_memory<T, OtherMaxStackCount>>()))
    {
        in_take_ownership(std::move(other));
    }

    inline ~stack_or_heap_memory()
    {
        in_destroy();
    }
};
} // hl
#endif
