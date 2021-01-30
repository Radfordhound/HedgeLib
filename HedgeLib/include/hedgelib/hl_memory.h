#ifndef HL_MEMORY_H_INCLUDED
#define HL_MEMORY_H_INCLUDED
#include "hl_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HlStream HlMemStream;
typedef struct HlBlob HlBlob;

typedef void* (*HlAllocFunc)(size_t size);
typedef void* (*HlReallocFunc)(void* ptr, size_t size);
typedef void (*HlFreeFunc)(void* ptr);

HL_API extern HlAllocFunc HlAllocPtr;
HL_API extern HlReallocFunc HlReallocPtr;
HL_API extern HlFreeFunc HlFreePtr;

HL_API void hlSetAllocators(HlAllocFunc alloc,
    HlReallocFunc realloc, HlFreeFunc free);

#define hlAlloc(size)           HlAllocPtr(size)
#define hlRealloc(ptr, size)    HlReallocPtr(ptr, size)
#define hlFree(ptr)             HlFreePtr(ptr)

HL_API void* hlCAlloc(size_t size);
HL_API void* hlCRealloc(void* ptr, size_t size);

HL_API HlResult hlMemStreamOpenRead(const void* HL_RESTRICT data,
    size_t dataSize, HlMemStream* HL_RESTRICT * HL_RESTRICT mem);

HL_API HlResult hlMemStreamOpenWrite(HlMemStream** mem);
HL_API void* hlMemStreamGetDataPtr(const HlMemStream* HL_RESTRICT mem,
    size_t* HL_RESTRICT bufSize);

HL_API HlResult hlMemStreamGetData(const HlMemStream* HL_RESTRICT mem,
    HlBlob* HL_RESTRICT * HL_RESTRICT blob);

HL_API HlResult hlMemStreamClose(HlMemStream* mem);

#ifndef HL_NO_EXTERNAL_WRAPPERS
HL_API void* hlAllocExt(size_t size);
HL_API void* hlReallocExt(void* ptr, size_t size);
HL_API void hlFreeExt(void* ptr);
#endif

#define HL_ALLOC_OBJ(type)              (type*)(hlAlloc(sizeof(type)))
#define HL_ALLOC_ARR(type, count)       (type*)(hlAlloc(sizeof(type) * (count)))
#define HL_RESIZE_ARR(type, count, arr) (type*)(hlRealloc(arr, sizeof(type) * (count)))

#ifdef __cplusplus
}

/* C++-specific stuff. */
#include <stddef.h>
#include <stdexcept>
#include <limits>

/* TODO: Is this MSVC version correct? I just guessed it lol. */
#if __cplusplus >= 201103 || (defined(_MSC_VER) && _MSC_VER >= 1700)
#define HL_IN_HAS_CPP11
#define HL_IN_NOEXCEPT noexcept
#else
#define HL_IN_NOEXCEPT throw()
#endif

/*
   C++-specific allocator for std::string, std::vector, etc.

   (If you want to know why I generally prefer C to C++ btw, just
   take a look at all of this crap you have to write just to have
   custom allocators and still be able to use std stuff.)
*/
template<class T>
class HlAllocator
{
public:
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

#ifndef HL_IN_HAS_CPP11
    template<class U>
    struct rebind
    {
        typedef HlAllocator<U> other;
    };
#endif

    inline HlAllocator() HL_IN_NOEXCEPT {};

    template<class U>
    inline HlAllocator(HlAllocator<U> const& other) HL_IN_NOEXCEPT {}

#ifdef HL_IN_HAS_CPP11
    inline T* allocate(size_t n)
#else
    inline pointer allocate(size_type n, const void* hint = 0)
#endif
    {
        pointer ptr = static_cast<pointer>(hlAlloc(sizeof(value_type) * n));
        if (!ptr) throw std::bad_alloc();
        return ptr;
    }

    inline void deallocate(pointer p, size_t n)
    {
        hlFree(p);
    }

#ifndef HL_IN_HAS_CPP11
    inline void construct(pointer p, const_reference val)
    {
        ::new(p) value_type(val);
    }

    inline void destroy(pointer p)
    {
        p->~T();
    }

    inline size_type max_size() const throw()
    {
        return (std::numeric_limits<size_type>::max() / sizeof(value_type));
    }

    inline pointer address(reference x) const
    {
        return &x;
    }

    inline const_pointer address(const_reference x) const
    {
        return &x;
    }
#endif

    template<typename T2>
    inline bool operator==(const HlAllocator<T2>& other) noexcept
    {
        /*
           (This function is required for custom allocators for some reason)
           HlAllocator has no state, so two HlAllocators are always equal to each other.
        */
        return true;
    }

    template<typename T2>
    inline bool operator!=(const HlAllocator<T2>& other) noexcept
    {
        /*
           (This function is required for custom allocators for some reason)
           HlAllocator has no state, so two HlAllocators are always equal to each other.
        */
        return false;
    }
};

#ifdef HL_IN_HAS_CPP11
#include <memory>
#include <type_traits>
#include <utility>

/* C++11 helper functions */
template<typename T, typename ...args_t>
inline T* hlCreateObj(args_t&&... args)
{
    /*
       Allocate enough memory to hold an object of the given type, then
       call its constructor on that memory with the given arguments.
    */
    T* obj = static_cast<T*>(hlAlloc(sizeof(T)));
    if (obj) new (obj) T(std::forward<args_t>(args)...);
    return obj;
}

template <typename T>
inline typename std::enable_if<std::is_trivially_destructible<T>::value, void>::type
hlDeleteObj(T* obj)
{
    /* Trivially-destructible types have no destructors; just free the memory. */
    hlFree(obj);
}

template <typename T>
inline typename std::enable_if<!std::is_trivially_destructible<T>::value, void>::type
hlDeleteObj(T* obj)
{
    /* Non-trivially-destructible types have destructors; call it, then free the memory. */
    if (!obj) return;
    obj->~T();
    hlFree(obj);
}

template <typename T>
inline typename std::enable_if<std::is_trivially_destructible<T>::value, void>::type
hlDeleteArr(T* arr, size_t count)
{
    /* Trivially-destructible types have no destructors; just free the memory. */
    hlFree(arr);
}

template <typename T>
inline typename std::enable_if<!std::is_trivially_destructible<T>::value, void>::type
hlDeleteArr(T* arr, size_t count)
{
    /* Non-trivially-destructible types have destructors; call them, then free the memory. */
    if (!arr) return;
    for (size_t i = 0; i < count; ++i)
    {
        arr[i].~T();
    }

    hlFree(arr);
}

/* C++11 smart pointers */
template<typename T>
struct HlSmartPtrDeleter
{
    /* TODO: Test HlSmartPtr with arrays, and test this static assert */
    static_assert(!std::is_array<T>::value || std::is_trivially_destructible<
        std::remove_all_extents<T>::type>::value,
        "HlSPtrs don't support arrays of non-trivially-destructible types.");

    inline void operator()(T* ptr) const
    {
        hlDeleteObj(ptr);
    }
};

template<typename T>
using HlSmartPtr = std::unique_ptr<T, HlSmartPtrDeleter<T>>;

#undef HL_IN_HAS_CPP11
#undef HL_IN_NOEXCEPT
#endif
#endif
#endif
