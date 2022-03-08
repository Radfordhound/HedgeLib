#ifndef HL_INTERNAL_H_INCLUDED
#define HL_INTERNAL_H_INCLUDED
#include <cstdint>
#include <cstddef>
#include <climits>
#include <type_traits>
#include <exception>
#include <stdexcept>

#ifdef _MSC_VER
#include <intrin.h>
#if _MSC_VER >= 1310
#include <stdlib.h>
#endif
#endif

namespace hl
{
/* Windows platform macro */
#if defined(__CYGWIN__) && !defined(_WIN32)
#define _WIN32
#endif

/* Common architecture macros */
#ifndef HL_X86
#if (defined(i386) || defined(__i386) || defined(__i386__) ||\
    defined(_M_IX86) || defined(__X86__) || defined(_X86_) ||\
    defined(__I86__) || defined(__INTEL__) || defined(__386))
#define HL_X86
#endif
#endif

#ifndef HL_X64
#if (defined(__amd64__) || defined(__amd64) || defined(__x86_64__) ||\
    defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64))
#define HL_X64
#endif
#endif

#ifndef HL_ARM
#if (defined(__arm__) || defined(__TARGET_ARCH_ARM) || defined(_ARM) ||\
    defined(_M_ARM) || defined(__arm))
#define HL_ARM
#endif
#endif

#ifndef HL_ARM64
#if (defined(__aarch64__) || defined(_M_ARM64))
#define HL_ARM64
#endif
#endif

/*
   Shared library macros
   (Adapted from https://gcc.gnu.org/wiki/Visibility)
*/
#ifndef HL_API
#ifdef HL_IS_DLL
// HedgeLib is a DLL.
#ifdef _WIN32
// We're targetting Windows; use __declspec
#ifdef HL_IS_BUILDING_DLL
// We're building a DLL; export the given symbol.
#define HL_API __declspec(dllexport)
#else
// We're using a pre-built DLL; import the given symbol.
#define HL_API __declspec(dllimport)
#endif
#elif defined(__GNUC__) && __GNUC__ >= 4
// We're not targetting Windows and we're using gcc; use __attribute__ 
#define HL_API __attribute__ ((visibility ("default")))
#else
// We don't know the target platform/compiler; assume it doesn't require any keywords.
#define HL_API
#endif
#else
// HedgeLib is a static library; no keyword(s) are needed. 
#define HL_API
#endif
#endif

/* Fixed-width integral type aliases */
using s8 = std::int8_t;
using u8 = std::uint8_t;
using s16 = std::int16_t;
using u16 = std::uint16_t;
using s32 = std::int32_t;
using u32 = std::uint32_t;
using s64 = std::int64_t;
using u64 = std::uint64_t;

/* Static assert macros */
#ifdef __cpp_static_assert
/* C++11 static assert */
#define HL_STATIC_ASSERT(expression, msg) static_assert(expression, msg)
#else
/* No static assert */
#define HL_STATIC_ASSERT(expression, msg)
#endif

#define HL_STATIC_ASSERT_SIZE(type, size) HL_STATIC_ASSERT(sizeof(type) == size,\
    "sizeof(" #type ") != expected size (" #size ").")

/* Pointer addition helpers */
template<typename ret_t = u8, typename off_t>
inline ret_t* ptradd(void* ptr, off_t off) noexcept
{
    return reinterpret_cast<ret_t*>(
        (static_cast<u8*>(ptr) + off));
}

template<typename ret_t = u8, typename off_t>
inline const ret_t* ptradd(const void* ptr, off_t off) noexcept
{
    return reinterpret_cast<const ret_t*>(
        (static_cast<const u8*>(ptr) + off));
}

/* Bit helpers */
template<typename T>
constexpr std::size_t bit_count() noexcept
{
    return (sizeof(T) * 8);
}

template<typename T>
constexpr T bit_flag(T index) noexcept
{
    return (static_cast<T>(1) << index);
}

inline unsigned int bit_ctz(unsigned int v) noexcept
{
#if defined(_MSC_VER)
    unsigned long r;
    _BitScanForward(&r, v);
    return static_cast<unsigned int>(r);
#elif (defined(__clang__) && defined(__has_builtin))
#if __has_builtin(__builtin_ctz)
    return static_cast<unsigned int>(__builtin_ctz(v));
#endif
#elif defined(__GNUC__)
    return static_cast<unsigned int>(__builtin_ctz(v));
#else
    unsigned int i;
    for (i = 0; i < 32; ++i)
    {
        if ((v & (1 << i)) != 0) break;
    }

    return i;
#endif
}

#define HL_ENUM_CLASS_DEF_BITWISE_OPS(enumClass)\
    constexpr enumClass operator&(enumClass a, enumClass b) noexcept\
    {\
        using int_t = std::underlying_type<enumClass>::type;\
        return static_cast<enumClass>(static_cast<int_t>(a) & static_cast<int_t>(b));\
    }\
\
    constexpr enumClass operator|(enumClass a, enumClass b) noexcept\
    {\
        using int_t = std::underlying_type<enumClass>::type;\
        return static_cast<enumClass>(static_cast<int_t>(a) | static_cast<int_t>(b));\
    }\
\
    constexpr enumClass operator^(enumClass a, enumClass b) noexcept\
    {\
        using int_t = std::underlying_type<enumClass>::type;\
        return static_cast<enumClass>(static_cast<int_t>(a) ^ static_cast<int_t>(b));\
    }\
\
    constexpr enumClass operator~(enumClass a) noexcept\
    {\
        using int_t = std::underlying_type<enumClass>::type;\
        return static_cast<enumClass>(~static_cast<int_t>(a));\
    }\
\
    template<typename shift_t>\
    constexpr enumClass operator<<(enumClass a, shift_t b) noexcept\
    {\
        using int_t = std::underlying_type<enumClass>::type;\
        return static_cast<enumClass>(static_cast<int_t>(a) << b);\
    }\
\
    template<typename shift_t>\
    constexpr enumClass operator>>(enumClass a, shift_t b) noexcept\
    {\
        using int_t = std::underlying_type<enumClass>::type;\
        return static_cast<enumClass>(static_cast<int_t>(a) >> b);\
    }\
\
    inline enumClass& operator&=(enumClass& a, enumClass b) noexcept\
    {\
        return ((a = (a & b)));\
    }\
\
    inline enumClass& operator|=(enumClass& a, enumClass b) noexcept\
    {\
        return ((a = (a | b)));\
    }\
\
    inline enumClass& operator^=(enumClass& a, enumClass b) noexcept\
    {\
        return ((a = (a ^ b)));\
    }\
\
    template<typename shift_t>\
    inline enumClass& operator>>=(enumClass a, shift_t b) noexcept\
    {\
        return ((a = (a >> b)));\
    }\
\
    template<typename shift_t>\
    inline enumClass& operator<<=(enumClass a, shift_t b) noexcept\
    {\
        return ((a = (a << b)));\
    }
    
/* Endianness helpers */
#ifndef HL_IS_BIG_ENDIAN
#ifdef __BYTE_ORDER__
// GCC big endian platform macro
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define HL_IS_BIG_ENDIAN
#endif
#elif defined(__BIG_ENDIAN__) && !defined(__LITTLE_ENDIAN__)
// Clang big endian platform macro
#define HL_IS_BIG_ENDIAN
#endif
#endif

/* Error handling */
enum class error_type
{
    unknown,
    unsupported,
    out_of_memory,
    invalid_args,
    out_of_range,
    already_exists,
    not_found,
    invalid_data,
    no_more_entries,
    sharing_violation
};

HL_API std::exception make_exception(error_type err);

HL_API std::exception make_exception(error_type err,
    const char* what_arg);

#ifndef HL_ERROR
#define HL_ERROR(...) throw make_exception(__VA_ARGS__)
#endif

/* Memory helpers */
HL_API void* aligned_malloc(std::size_t size, std::size_t alignment);
HL_API void aligned_free(void* ptr);

/* Endian swap intrinsic macros */
#if defined(_MSC_VER) && _MSC_VER >= 1310

/* MSVC intrinsics */
#define HL_SWAP_U16(v) static_cast<::hl::u16>(_byteswap_ushort(static_cast<unsigned short>(v)))
#define HL_SWAP_U32(v) static_cast<::hl::u32>(_byteswap_ulong(static_cast<unsigned long>(v)))
#define HL_SWAP_U64(v) static_cast<::hl::u64>(_byteswap_uint64(static_cast<unsigned __int64>(v)))

#elif (defined(__clang__) && defined(__has_builtin))

/* Clang intrinsics */
#if __has_builtin(__builtin_bswap16)
#define HL_SWAP_U16(v) __builtin_bswap16(v)
#endif
#if __has_builtin(__builtin_bswap32)
#define HL_SWAP_U32(v) __builtin_bswap32(v)
#endif
#if __has_builtin(__builtin_bswap64)
#define HL_SWAP_U64(v) __builtin_bswap64(v)
#endif

#elif defined(__GNUC__)

/* GCC intrinsics */
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))
/* GCC 4.8+ intrinsics */
#define HL_SWAP_U16(v) __builtin_bswap16(v)
#endif

#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3))
/* Use GCC 4.3+ intrinsics */
#define HL_SWAP_U32(v) __builtin_bswap32(v)
#define HL_SWAP_U64(v) __builtin_bswap64(v)
#endif

#endif

/*
   Endian swap manual fallback macros

   Most compilers should (hopefully) be smart enough
   to turn these into intrinsics anyway.
*/
#ifndef HL_SWAP_U16
#define HL_SWAP_U16(v) ((((v) & 0xFFU) << 8) |\
    (((v) & 0xFF00U) >> 8))
#endif

#ifndef HL_SWAP_U32
#define HL_SWAP_U32(v) ((((v) & 0xFFU) << 24) |\
    (((v) & 0xFF00U) << 8) | (((v) & 0xFF0000U) >> 8) |\
    (((v) & 0xFF000000U) >> 24))
#endif

#ifndef HL_SWAP_U64
#define HL_SWAP_U64(v) ((((v) & 0xFFU) << 56) |\
    (((v) & 0xFF00U) << 40) | (((v) & 0xFF0000U) << 24) |\
    (((v) & 0xFF000000U) << 8) | (((v) & 0xFF00000000U) >> 8) |\
    (((v) & 0xFF0000000000U) >> 24) | (((v) & 0xFF000000000000U) >> 40)|\
    (((v) & 0xFF00000000000000U) >> 56))
#endif

inline void endian_swap(u8& v) noexcept {}
inline void endian_swap(s8& v) noexcept {}

inline void endian_swap(u16& v) noexcept
{
    v = HL_SWAP_U16(v);
}

inline void endian_swap(s16& v) noexcept
{
    endian_swap(reinterpret_cast<u16&>(v));
}

inline void endian_swap(u32& v) noexcept
{
    v = HL_SWAP_U32(v);
}

inline void endian_swap(s32& v) noexcept
{
    endian_swap(reinterpret_cast<u32&>(v));
}

inline void endian_swap(u64& v) noexcept
{
    v = HL_SWAP_U64(v);
}

inline void endian_swap(s64& v) noexcept
{
    endian_swap(reinterpret_cast<u64&>(v));
}

inline void endian_swap(float& v) noexcept
{
    HL_STATIC_ASSERT_SIZE(float, sizeof(u32));
    endian_swap(reinterpret_cast<u32&>(v));
}

inline void endian_swap(double& v) noexcept
{
    HL_STATIC_ASSERT_SIZE(double, sizeof(u64));
    endian_swap(reinterpret_cast<u64&>(v));
}

template<typename T>
inline void endian_swap(T& v) noexcept
{
    v.endian_swap();
}

template<bool swapOffsets, typename T>
inline void endian_swap(T& v) noexcept
{
    v.template endian_swap<swapOffsets>();
}

/* Offsets */
template<typename T>
class off32
{
public:
#if UINTPTR_MAX > UINT32_MAX
    using val_t = s32;
#else
    using val_t = u32;
#endif

    using this_t = off32<T>;
    using cthis_t = const this_t;
    using ptr_t = T*;
    using cptr_t = const T*;
    using ref_t = typename std::add_lvalue_reference<T>::type;
    using cref_t = typename std::add_lvalue_reference<const T>::type;

private:
    val_t m_val;

public:
    template<bool swapOffsets = true>
    typename std::enable_if<swapOffsets, void>::type
    	endian_swap() noexcept
    {
        hl::endian_swap(m_val);
    }

    template<bool swapOffsets = true>
    typename std::enable_if<!swapOffsets, void>::type
    	endian_swap() noexcept {}

    cptr_t get() const noexcept
    {
#if UINTPTR_MAX > UINT32_MAX
        // Convert from relative pointer to absolute pointer and return result.
        return (m_val != 0) ? reinterpret_cast<cptr_t>(
            reinterpret_cast<std::intptr_t>(this) + m_val) :
            nullptr;
#else
        // Cast from number to pointer.
        return reinterpret_cast<cptr_t>(
            static_cast<std::uintptr_t>(m_val));
#endif
    }

    inline ptr_t get() noexcept
    {
        return const_cast<ptr_t>(const_cast<cthis_t*>(this)->get());
    }

    val_t set(ptr_t ptr)
    {
#if UINTPTR_MAX > UINT32_MAX
        // Just set m_val to 0 if ptr is null.
        if (!ptr)
        {
            m_val = 0;
            return m_val;
        }

        // Pointers are > 32 bits, so ptr will not fit within a 32-bit offset.
        // We have to convert ptr to a relative pointer instead, and
        // store that within the given offset.
        const std::intptr_t ptrAddr = reinterpret_cast<std::intptr_t>(ptr);
        std::intptr_t offAddr = reinterpret_cast<std::intptr_t>(this);

        // Ensure offset does not point to itself; we can't do that
        // with our relative offsets, since we still use 0 for null.
        if (ptrAddr == offAddr)
        {
            HL_ERROR(error_type::unsupported);
        }

        // Compute a relative offset which points to ptr.
        offAddr = (ptrAddr - offAddr);

        // Ensure relative offset can fit within a signed
        // 32-bit integer, since we're about to cast to that.
        if (offAddr > INT32_MAX || offAddr < INT32_MIN)
        {
            HL_ERROR(error_type::out_of_range, "off32::offAddr");
        }

        // Set the offset to the relative offset we just computed.
        m_val = static_cast<val_t>(offAddr);
#else
        m_val = static_cast<val_t>(
            reinterpret_cast<std::uintptr_t>(ptr));
#endif

        return m_val;
    }

    inline val_t fix(void* base)
    {
        return set((m_val) ? ptradd<T>(base, m_val) : nullptr);
    }

    inline this_t& operator=(ptr_t ptr)
    {
        set(ptr);
        return *this;
    }

    inline cptr_t operator->() const noexcept
    {
        return get();
    }

    inline ptr_t operator->() noexcept
    {
        return get();
    }

    inline cref_t operator*() const noexcept
    {
        return *get();
    }

    inline ref_t operator*() noexcept
    {
        return *get();
    }

    inline cref_t operator[](std::size_t i) const noexcept
    {
        return *(get() + i);
    }

    inline ref_t operator[](std::size_t i) noexcept
    {
        return *(get() + i);
    }

    inline bool operator<(cthis_t& other) const noexcept
    {
        return (get() < other.get());
    }

    inline bool operator>(cthis_t& other) const noexcept
    {
        return (get() > other.get());
    }

    inline bool operator<=(cthis_t& other) const noexcept
    {
        return (get() <= other.get());
    }

    inline bool operator>=(cthis_t& other) const noexcept
    {
        return (get() >= other.get());
    }

    inline bool operator==(cthis_t& other) const noexcept
    {
        return (get() == other.get());
    }

    inline bool operator!=(cthis_t& other) const noexcept
    {
        return (get() != other.get());
    }

    inline bool operator<(cptr_t ptr) const noexcept
    {
        return (get() < ptr);
    }

    inline bool operator>(cptr_t ptr) const noexcept
    {
        return (get() > ptr);
    }

    inline bool operator<=(cptr_t ptr) const noexcept
    {
        return (get() <= ptr);
    }

    inline bool operator>=(cptr_t ptr) const noexcept
    {
        return (get() >= ptr);
    }

    inline bool operator==(cptr_t ptr) const noexcept
    {
        return (get() == ptr);
    }

    inline bool operator!=(cptr_t ptr) const noexcept
    {
        return (get() != ptr);
    }

    inline explicit operator bool() const noexcept
    {
        return (m_val != 0);
    }

    inline off32() noexcept = default;
    inline off32(std::nullptr_t) noexcept : m_val(0) {}
    inline off32(u32 v) noexcept : m_val(static_cast<val_t>(v)) {}
    inline off32(ptr_t ptr) : m_val(set(ptr)) {}
};

template<typename T>
class off64
{
public:
#if UINTPTR_MAX > UINT64_MAX
    using val_t = s64;
#else
    using val_t = u64;
#endif

    using this_t = off64<T>;
    using cthis_t = const this_t;
    using ptr_t = T*;
    using cptr_t = const T*;
    using ref_t = typename std::add_lvalue_reference<T>::type;
    using cref_t = typename std::add_lvalue_reference<const T>::type;

private:
    val_t m_val;

public:
    template<bool swapOffsets = true>
    typename std::enable_if<swapOffsets, void>::type
    	endian_swap() noexcept
    {
        hl::endian_swap(m_val);
    }

    template<bool swapOffsets = true>
    typename std::enable_if<!swapOffsets, void>::type
    	endian_swap() noexcept {}

    cptr_t get() const noexcept
    {
#if UINTPTR_MAX > UINT64_MAX
        // Convert from relative pointer to absolute pointer and return result.
        return (m_val != 0) ? reinterpret_cast<cptr_t>(
            reinterpret_cast<std::intptr_t>(this) + m_val) :
            nullptr;
#else
        // Cast from number to pointer.
        return reinterpret_cast<cptr_t>(
            static_cast<std::uintptr_t>(m_val));
#endif
    }

    inline ptr_t get() noexcept
    {
        return const_cast<ptr_t>(const_cast<cthis_t*>(this)->get());
    }

    val_t set(ptr_t ptr)
    {
#if UINTPTR_MAX > UINT64_MAX
        // Just set m_val to 0 if ptr is null.
        if (!ptr)
        {
            m_val = 0;
            return m_val;
        }

        // Pointers are > 64 bits, so ptr will not fit within a 64-bit offset.
        // We have to convert ptr to a relative pointer instead, and
        // store that within the given offset.
        const std::intptr_t ptrAddr = reinterpret_cast<std::intptr_t>(ptr);
        std::intptr_t offAddr = reinterpret_cast<std::intptr_t>(this);

        // Ensure offset does not point to itself; we can't do that
        // with our relative offsets, since we still use 0 for null.
        if (ptrAddr == offAddr)
        {
            HL_ERROR(error_type::unsupported);
        }

        // Compute a relative offset which points to ptr.
        offAddr = (ptrAddr - offAddr);

        // Ensure relative offset can fit within a signed
        // 32-bit integer, since we're about to cast to that.
        if (offAddr > INT64_MAX || offAddr < INT64_MIN)
        {
            HL_ERROR(error_type::out_of_range, "off64::offAddr");
        }

        // Set the offset to the relative offset we just computed.
        m_val = static_cast<val_t>(offAddr);
#else
        m_val = static_cast<val_t>(
            reinterpret_cast<std::uintptr_t>(ptr));
#endif

        return m_val;
    }

    inline val_t fix(void* base)
    {
        return set((m_val) ? ptradd<T>(base, m_val) : nullptr);
    }

    inline this_t& operator=(ptr_t ptr)
    {
        set(ptr);
        return *this;
    }

    inline cptr_t operator->() const noexcept
    {
        return get();
    }

    inline ptr_t operator->() noexcept
    {
        return get();
    }

    inline cref_t operator*() const noexcept
    {
        return *get();
    }

    inline ref_t operator*() noexcept
    {
        return *get();
    }

    inline cref_t operator[](std::size_t i) const noexcept
    {
        return *(get() + i);
    }

    inline ref_t operator[](std::size_t i) noexcept
    {
        return *(get() + i);
    }

    inline bool operator<(cthis_t& other) const noexcept
    {
        return (get() < other.get());
    }

    inline bool operator>(cthis_t& other) const noexcept
    {
        return (get() > other.get());
    }

    inline bool operator<=(cthis_t& other) const noexcept
    {
        return (get() <= other.get());
    }

    inline bool operator>=(cthis_t& other) const noexcept
    {
        return (get() >= other.get());
    }

    inline bool operator==(cthis_t& other) const noexcept
    {
        return (get() == other.get());
    }

    inline bool operator!=(cthis_t& other) const noexcept
    {
        return (get() != other.get());
    }

    inline bool operator<(cptr_t ptr) const noexcept
    {
        return (get() < ptr);
    }

    inline bool operator>(cptr_t ptr) const noexcept
    {
        return (get() > ptr);
    }

    inline bool operator<=(cptr_t ptr) const noexcept
    {
        return (get() <= ptr);
    }

    inline bool operator>=(cptr_t ptr) const noexcept
    {
        return (get() >= ptr);
    }

    inline bool operator==(cptr_t ptr) const noexcept
    {
        return (get() == ptr);
    }

    inline bool operator!=(cptr_t ptr) const noexcept
    {
        return (get() != ptr);
    }

    inline explicit operator bool() const noexcept
    {
        return (m_val != 0);
    }

    inline off64() noexcept = default;
    inline off64(std::nullptr_t) noexcept : m_val(0) {}
    inline off64(u64 v) noexcept : m_val(static_cast<val_t>(v)) {}
    inline off64(ptr_t ptr) : m_val(set(ptr)) {}
};

template<typename T>
class arr32
{
    using this_t = arr32<T>;
    using cthis_t = const this_t;
    using ptr_t = T*;
    using cptr_t = const T*;
    using ref_t = typename std::add_lvalue_reference<T>::type;
    using cref_t = typename std::add_lvalue_reference<const T>::type;

public:
    u32 count;
    off32<T> data;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(count);
        hl::endian_swap<swapOffsets>(data);
    }

    inline cptr_t get() const noexcept
    {
        return data.get();
    }

    inline ptr_t get() noexcept
    {
        return data.get();
    }

    inline cptr_t begin() const noexcept
    {
        return get();
    }

    inline ptr_t begin() noexcept
    {
        return get();
    }

    inline cptr_t end() const noexcept
    {
        return &data[count];
    }

    inline ptr_t end() noexcept
    {
        return &data[count];
    }

    inline cref_t operator[](u32 i) const noexcept
    {
        return data[i];
    }

    inline ref_t operator[](u32 i) noexcept
    {
        return data[i];
    }

    inline bool operator==(cthis_t& other) const noexcept
    {
        return (data == other.data &&
            data == other.count);
    }

    inline bool operator!=(cthis_t& other) const noexcept
    {
        return (data != other.data ||
            count != other.count);
    }

    inline arr32() noexcept = default;
    inline arr32(u32 count, off32<T> data) noexcept :
        count(count), data(data) {}
};

template<typename T>
class arr64
{
    using this_t = arr64<T>;
    using cthis_t = const this_t;
    using ptr_t = T*;
    using cptr_t = const T*;
    using ref_t = typename std::add_lvalue_reference<T>::type;
    using cref_t = typename std::add_lvalue_reference<const T>::type;

public:
    u64 count;
    off64<T> data;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(count);
        hl::endian_swap<swapOffsets>(data);
    }

    inline cptr_t get() const noexcept
    {
        return data.get();
    }

    inline ptr_t get() noexcept
    {
        return data.get();
    }

    inline cptr_t begin() const noexcept
    {
        return get();
    }

    inline ptr_t begin() noexcept
    {
        return get();
    }

    inline cptr_t end() const noexcept
    {
        return &data[static_cast<std::size_t>(count)];
    }

    inline ptr_t end() noexcept
    {
        return &data[static_cast<std::size_t>(count)];
    }

    inline cref_t operator[](u64 i) const noexcept
    {
        return data[static_cast<std::size_t>(i)];
    }

    inline ref_t operator[](u64 i) noexcept
    {
        return data[static_cast<std::size_t>(i)];
    }

    inline bool operator==(cthis_t& other) const noexcept
    {
        return (data == other.data &&
            count == other.count);
    }

    inline bool operator!=(cthis_t& other) const noexcept
    {
        return (data != other.data ||
            count != other.mount);
    }

    inline arr64() noexcept = default;
    inline arr64(u64 count, off64<T> data) noexcept :
        count(count), data(data) {}
};

/* Miscellaneous helpers */
#define HL_COUNT_OF(arr) (sizeof(arr) / sizeof(*(arr)))

/**
    @brief Creates a signature in the form of a 32-bit unsigned integer at compile-time.

    @param c1 The first character in the signature.
    @param c2 The second character in the signature.
    @param c3 The third character in the signature.
    @param c4 The fourth character in the signature.
    @return The signature in the form of a 32-bit unsigned integer.
*/
constexpr u32 make_sig(char c1, char c2,
    char c3, char c4) noexcept
{
#ifdef HL_IS_BIG_ENDIAN
    return ((static_cast<u32>(c1) << 24U) |
        (static_cast<u32>(c2) << 16U) |
        (static_cast<u32>(c3) << 8U) |
        static_cast<u32>(c4));
#else
    return ((static_cast<u32>(c4) << 24U) |
        (static_cast<u32>(c3) << 16U) |
        (static_cast<u32>(c2) << 8U) |
        static_cast<u32>(c1));
#endif
}

/**
    @brief Creates a signature in the form of a 32-bit unsigned integer at compile-time.

    @param arr The array of characters to create a signature from.
    @return The signature in the form of a 32-bit unsigned integer.
*/
constexpr u32 make_sig(const char(&arr)[4]) noexcept
{
    return make_sig(arr[0], arr[1], arr[2], arr[3]);
}

/**
    @brief Creates a signature in the form of a 32-bit unsigned integer at compile-time.

    This overload takes a 5-character array. It's meant to be used with 4-character
    strings, as they also have an additional 5th character - the null terminator.

    @param str The string to create a signature from.
    @return The signature in the form of a 32-bit unsigned integer.
*/
constexpr u32 make_sig(const char(&str)[5]) noexcept
{
    return make_sig(str[0], str[1], str[2], str[3]);
}

/**
    @brief Creates a signature in the form of a 64-bit unsigned integer at compile-time.

    @param c1 The first character in the signature.
    @param c2 The second character in the signature.
    @param c3 The third character in the signature.
    @param c4 The fourth character in the signature.
    @param c5 The fifth character in the signature.
    @param c6 The sixth character in the signature.
    @param c7 The seventh character in the signature.
    @param c8 The eigth character in the signature.
    @return The signature in the form of a 64-bit unsigned integer.
*/
constexpr u64 make_sig(char c1, char c2, char c3, char c4,
    char c5, char c6, char c7, char c8) noexcept
{
#ifdef HL_IS_BIG_ENDIAN
    return ((static_cast<u32>(c1) << 56U) |
        (static_cast<u32>(c2) << 48U) |
        (static_cast<u32>(c3) << 40U) |
        (static_cast<u32>(c4) << 32U) |
        (static_cast<u32>(c5) << 24U) |
        (static_cast<u32>(c6) << 16U) |
        (static_cast<u32>(c7) << 8U) |
        static_cast<u32>(c8));
#else
    return ((static_cast<u64>(c8) << 56U) |
        (static_cast<u64>(c7) << 48U) |
        (static_cast<u64>(c6) << 40U) |
        (static_cast<u64>(c5) << 32U) |
        (static_cast<u64>(c4) << 24U) |
        (static_cast<u64>(c3) << 16U) |
        (static_cast<u64>(c2) << 8U) |
        static_cast<u64>(c1));
#endif
}

/**
    @brief Creates a signature in the form of a 64-bit unsigned integer at compile-time.

    @param arr The array of characters to create a signature from.
    @return The signature in the form of a 64-bit unsigned integer.
*/
constexpr u64 make_sig(const char(&arr)[8]) noexcept
{
    return make_sig(arr[0], arr[1], arr[2], arr[3],
        arr[4], arr[5], arr[6], arr[7]);
}

/**
    @brief Creates a signature in the form of a 64-bit unsigned integer at compile-time.

    This overload takes a 9-character array. It's meant to be used with 8-character
    strings, as they also have an additional 9th character - the null terminator.

    @param str The string to create a signature from.
    @return The signature in the form of a 64-bit unsigned integer.
*/
constexpr u64 make_sig(const char(&str)[9]) noexcept
{
    return make_sig(str[0], str[1], str[2], str[3],
        str[4], str[5], str[6], str[7]);
}

template<typename val_t, typename stride_t>
constexpr val_t align(val_t value, stride_t stride) noexcept
{
    return (stride < 2) ? value :                   // If stride is < 2, just return value.
        ((value + (stride - 1)) & ~(stride - 1));   // Align value by stride and return.
}
} // hl
#endif
