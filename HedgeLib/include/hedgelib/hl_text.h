#ifndef HL_TEXT_H_INCLUDED
#define HL_TEXT_H_INCLUDED
#include "hl_internal.h"
#include <string>
#include <memory>
#include <algorithm>

namespace hl
{
/* Native character type */
#if defined(_WIN32) && !defined(HL_WIN32_FORCE_ANSI)
using nchar = wchar_t;
#define HL_IN_WIN32_UNICODE
#endif

#ifndef HL_IN_WIN32_UNICODE
using nchar = char;
#endif

/* Native string type */
using nstring = std::basic_string<nchar>;

/* Text helper macros */
#ifdef HL_IN_WIN32_UNICODE
#define HL_NTEXT(txt) L##txt
#else
#define HL_NTEXT(txt) txt
#endif

namespace text
{
enum class encoding
{
    utf8 = 0,
    utf16_be,
    utf16_le,
    utf32_be,
    utf32_le
};

template<typename char_t>
inline std::size_t len(const char_t* str)
{
    return std::char_traits<char_t>::length(str);
}

template<typename char_t>
inline std::size_t size(const char_t* str)
{
    return (len(str) + 1);
}

template<typename char_t>
constexpr bool is_digit(char_t c) noexcept
{
    return (c <= static_cast<char_t>('9') &&
        c >= static_cast<char_t>('0'));
}

template<typename char_t>
constexpr char_t to_upper(char_t c) noexcept
{
    return ((c >= 'a' && c <= 'z') ? (c - 32) : c);
}

template<typename char_t>
constexpr char_t to_lower(char_t c) noexcept
{
    return ((c >= 'A' && c <= 'Z') ? (c + 32) : c);
}

template<typename char_t, typename traits_t = std::char_traits<char_t>>
inline int compare(const char_t* str1, const char_t* str2, std::size_t count)
{
    return traits_t::compare(str1, str2, count);
}

template<typename char_t, typename traits_t = std::char_traits<char_t>>
inline int compare(const char_t* str1, const char_t* str2)
{
    while (*str1 && traits_t::eq(*str1, *str2))
    {
        ++str1;
        ++str2;
    }

    return (static_cast<int>(traits_t::to_int_type(*str1)) -
        static_cast<int>(traits_t::to_int_type(*str2)));
}

template<typename char_t, typename traits_t = std::char_traits<char_t>>
inline int icompare_as_upper(const char_t* str1, const char_t* str2, std::size_t count)
{
    typename traits_t::int_type c1 = 0, c2 = 0;
    while (count && (c2 = traits_t::to_int_type(*str2),
        c1 = traits_t::to_int_type(*str1)))
    {
        if (c1 != c2)
        {
            if (c1 >= static_cast<typename traits_t::int_type>('A') &&
                c2 >= static_cast<typename traits_t::int_type>('A'))
            {
                // If c1 is lower-cased, upper-case it.
                if (c1 >= static_cast<typename traits_t::int_type>('a') &&
                    c1 <= static_cast<typename traits_t::int_type>('z'))
                {
                    c1 -= 32;
                }

                // If c2 is lower-cased, upper-case it.
                if (c2 >= static_cast<typename traits_t::int_type>('a') &&
                    c2 <= static_cast<typename traits_t::int_type>('z'))
                {
                    c2 -= 32;
                }

                if (c1 != c2) goto end;
            }
            else goto end;
        }

        ++str1;
        ++str2;
        --count;
    }

end:
    if (count == 0) return 0;

    return (static_cast<int>(c1) - static_cast<int>(c2));
}

template<typename char_t, typename traits_t = std::char_traits<char_t>>
inline int icompare_as_upper(const char_t* str1, const char_t* str2)
{
    typename traits_t::int_type c1 = 0, c2 = 0;
    while ((c2 = traits_t::to_int_type(*str2),
        c1 = traits_t::to_int_type(*str1)))
    {
        if (c1 != c2)
        {
            if (c1 >= static_cast<typename traits_t::int_type>('A') &&
                c2 >= static_cast<typename traits_t::int_type>('A'))
            {
                // If c1 is lower-cased, upper-case it.
                if (c1 >= static_cast<typename traits_t::int_type>('a') &&
                    c1 <= static_cast<typename traits_t::int_type>('z'))
                {
                    c1 -= 32;
                }

                // If c2 is lower-cased, upper-case it.
                if (c2 >= static_cast<typename traits_t::int_type>('a') &&
                    c2 <= static_cast<typename traits_t::int_type>('z'))
                {
                    c2 -= 32;
                }

                if (c1 != c2) goto end;
            }
            else goto end;
        }

        ++str1;
        ++str2;
    }

end:
    return (static_cast<int>(c1) - static_cast<int>(c2));
}

template<typename char_t, typename traits_t = std::char_traits<char_t>>
inline int icompare_as_lower(const char_t* str1, const char_t* str2, std::size_t count)
{
    typename traits_t::int_type c1 = 0, c2 = 0;
    while (count && (c2 = traits_t::to_int_type(*str2),
        c1 = traits_t::to_int_type(*str1)))
    {
        if (c1 != c2)
        {
            if (c1 <= static_cast<typename traits_t::int_type>('z') &&
                c2 <= static_cast<typename traits_t::int_type>('z'))
            {
                // If c1 is upper-cased, lower-case it.
                if (c1 >= static_cast<typename traits_t::int_type>('A') &&
                    c1 <= static_cast<typename traits_t::int_type>('Z'))
                {
                    c1 += 32;
                }

                // If c2 is upper-cased, lower-case it.
                if (c2 >= static_cast<typename traits_t::int_type>('A') &&
                    c2 <= static_cast<typename traits_t::int_type>('Z'))
                {
                    c2 += 32;
                }

                if (c1 != c2) goto end;
            }
            else goto end;
        }

        ++str1;
        ++str2;
        --count;
    }

end:
    if (count == 0) return 0;

    return (static_cast<int>(c1) - static_cast<int>(c2));
}

template<typename char_t, typename traits_t = std::char_traits<char_t>>
inline int icompare_as_lower(const char_t* str1, const char_t* str2)
{
    typename traits_t::int_type c1 = 0, c2 = 0;
    while ((c2 = traits_t::to_int_type(*str2),
        c1 = traits_t::to_int_type(*str1)))
    {
        if (c1 != c2)
        {
            if (c1 <= static_cast<typename traits_t::int_type>('z') &&
                c2 <= static_cast<typename traits_t::int_type>('z'))
            {
                // If c1 is upper-cased, lower-case it.
                if (c1 >= static_cast<typename traits_t::int_type>('A') &&
                    c1 <= static_cast<typename traits_t::int_type>('Z'))
                {
                    c1 += 32;
                }

                // If c2 is upper-cased, lower-case it.
                if (c2 >= static_cast<typename traits_t::int_type>('A') &&
                    c2 <= static_cast<typename traits_t::int_type>('Z'))
                {
                    c2 += 32;
                }

                if (c1 != c2) goto end;
            }
            else goto end;
        }

        ++str1;
        ++str2;
    }

end:
    return (static_cast<int>(c1) - static_cast<int>(c2));
}

template<typename char_t>
inline bool equal(const char_t* str1, const char_t* str2, std::size_t count)
{
    return (compare(str1, str2, count) == 0);
}

template<typename char_t>
inline bool equal(const char_t* str1, const char_t* str2)
{
    return (compare(str1, str2) == 0);
}

template<typename char_t>
inline bool iequal(const char_t* str1, const char_t* str2, std::size_t count)
{
    return (icompare_as_lower(str1, str2, count) == 0);
}

template<typename char_t>
inline bool iequal(const char_t* str1, const char_t* str2)
{
    return (icompare_as_lower(str1, str2) == 0);
}

template<typename char_t>
inline std::size_t copy(const char_t* src, char_t* dst)
{
    // Copy the string from src into dst.
    // (While condition is wrapped in double parenthesis so
    // clang/gcc/etc. know that the single = was intentional.)
    const char_t* ptr = src;
    while ((*dst++ = *ptr++));

    // Return the length of the string we just copied.
    // (We subtract 1 so the null terminator isn't factored in.)
    return (static_cast<std::size_t>(ptr - src) - 1);
}

template<typename char_t>
inline bool copy_s(const char_t* src, char_t* dst,
    std::size_t dstBufLen, std::size_t& copiedCount)
{
    // Copy up to dstBufLen characters from src into dst.
    std::size_t copied = 0;
    while (copied < dstBufLen)
    {
        ++copied;
        if (!(*dst++ = *src++))
        {
            copiedCount = copied;
            return true;
        }
    }

    copiedCount = copied;
    return false;
}

template<typename char_t>
inline bool copy_s(const char_t* src, char_t* dst, std::size_t dstBufLen)
{
    // Copy up to dstBufLen characters from src into dst.
    std::size_t copied = 0;
    while (copied < dstBufLen)
    {
        ++copied;
        if (!(*dst++ = *src++))
        {
            return true;
        }
    }

    return false;
}

template<typename char_t>
inline std::unique_ptr<char_t[]> make_copy(const char_t* str, std::size_t strLen)
{
    // Create copy of string.
    std::unique_ptr<char_t[]> strCopy(new char_t[strLen + 1]);
    std::copy(str, str + strLen, strCopy.get());

    // Set null terminator and return copy of string.
    strCopy[strLen] = static_cast<char_t>('\0');
    return strCopy;
}

template<typename char_t>
inline std::unique_ptr<char_t[]> make_copy(const char_t* str)
{
    return make_copy(str, len(str));
}

/* String encoding conversion */
struct native_to_native
{
    using src_char_t = nchar;
    using dst_char_t = nchar;

    inline static std::size_t conv(
        const src_char_t* src, std::size_t srcLen,
        dst_char_t* dst, std::size_t dstBufLen)
    {
        if (!dstBufLen)
        {
            return (srcLen) ? srcLen : len(src);
        }
        else if (!srcLen)
        {
            std::size_t copiedCount;

            // Copy until the null terminator, or until dstBufLen is exceeded.
            if (!copy_s(src, dst, dstBufLen, copiedCount))
            {
                // dstBufLen was exceeded; raise an error.
                HL_ERROR(error_type::out_of_range);
            }

            return (copiedCount - 1);
        }
        else
        {
            // Ensure we have enough space in the dst buffer.
            if (srcLen > dstBufLen)
            {
                HL_ERROR(error_type::out_of_range);
            }

            // Copy characters.
            std::copy(src, src + srcLen, dst);
            return srcLen;
        }
    }
};

struct utf8_to_utf16
{
    using src_char_t = char;
    using dst_char_t = char16_t;

    HL_API static std::size_t conv(
        const src_char_t* src, std::size_t srcLen,
        dst_char_t* dst, std::size_t dstBufLen);
};

struct utf8_to_utf32
{
    using src_char_t = char;
    using dst_char_t = char32_t;

    HL_API static std::size_t conv(
        const src_char_t* src, std::size_t srcLen,
        dst_char_t* dst, std::size_t dstBufLen);
};

struct utf8_to_native
{
    using src_char_t = char;
    using dst_char_t = nchar;

    inline static std::size_t conv(
        const src_char_t* src, std::size_t srcLen,
        dst_char_t* dst, std::size_t dstBufLen)
    {
#ifdef HL_IN_WIN32_UNICODE
        return utf8_to_utf16::conv(src, srcLen,
            reinterpret_cast<char16_t*>(dst), dstBufLen);
#else
        return native_to_native::conv(src, srcLen,
            dst, dstBufLen);
#endif
    }
};

struct utf16_to_utf8
{
    using src_char_t = char16_t;
    using dst_char_t = char;

    HL_API static std::size_t conv(
        const src_char_t* src, std::size_t srcLen,
        dst_char_t* dst, std::size_t dstBufLen);
};

struct utf16_to_utf32
{
    using src_char_t = char16_t;
    using dst_char_t = char32_t;

    HL_API static std::size_t conv(
        const src_char_t* src, std::size_t srcLen,
        dst_char_t* dst, std::size_t dstBufLen);
};

struct utf16_to_native
{
    using src_char_t = char16_t;
    using dst_char_t = nchar;

    inline static std::size_t conv(
        const src_char_t* src, std::size_t srcLen,
        dst_char_t* dst, std::size_t dstBufLen)
    {
#ifdef HL_IN_WIN32_UNICODE
        return native_to_native::conv(
            reinterpret_cast<const wchar_t*>(src),
            srcLen, dst, dstBufLen);
#else
        return utf16_to_utf8::conv(src, srcLen,
            dst, dstBufLen);
#endif
    }
};

struct utf32_to_utf8
{
    using src_char_t = char32_t;
    using dst_char_t = char;

    HL_API static std::size_t conv(
        const src_char_t* src, std::size_t srcLen,
        dst_char_t* dst, std::size_t dstBufLen);
};

struct utf32_to_utf16
{
    using src_char_t = char32_t;
    using dst_char_t = char16_t;

    HL_API static std::size_t conv(
        const src_char_t* src, std::size_t srcLen,
        dst_char_t* dst, std::size_t dstBufLen);
};

struct utf32_to_native
{
    using src_char_t = char32_t;
    using dst_char_t = nchar;

    inline static std::size_t conv(
        const src_char_t* src, std::size_t srcLen,
        dst_char_t* dst, std::size_t dstBufLen)
    {
#ifdef HL_IN_WIN32_UNICODE
        return utf32_to_utf16::conv(src, srcLen,
            reinterpret_cast<char16_t*>(dst), dstBufLen);
#else
        return utf32_to_utf8::conv(src, srcLen,
            dst, dstBufLen);
#endif
    }
};

struct native_to_utf8
{
    using src_char_t = nchar;
    using dst_char_t = char;

    inline static std::size_t conv(
        const src_char_t* src, std::size_t srcLen,
        dst_char_t* dst, std::size_t dstBufLen)
    {
#ifdef HL_IN_WIN32_UNICODE
        return utf16_to_utf8::conv(
            reinterpret_cast<const char16_t*>(src),
            srcLen, dst, dstBufLen);
#else
        return native_to_native::conv(src, srcLen,
            dst, dstBufLen);
#endif
    }
};

struct native_to_utf16
{
    using src_char_t = nchar;
    using dst_char_t = char16_t;

    inline static std::size_t conv(
        const src_char_t* src, std::size_t srcLen,
        dst_char_t* dst, std::size_t dstBufLen)
    {
#ifdef HL_IN_WIN32_UNICODE
        return native_to_native::conv(src, srcLen,
            reinterpret_cast<wchar_t*>(dst), dstBufLen);
#else
        return utf8_to_utf16::conv(src, srcLen,
            dst, dstBufLen);
#endif
    }
};

struct native_to_utf32
{
    using src_char_t = nchar;
    using dst_char_t = char32_t;

    inline static std::size_t conv(
        const src_char_t* src, std::size_t srcLen,
        dst_char_t* dst, std::size_t dstBufLen)
    {
#ifdef HL_IN_WIN32_UNICODE
        return utf16_to_utf32::conv(
            reinterpret_cast<const char16_t*>(src),
            srcLen, dst, dstBufLen);
#else
        return utf8_to_utf32::conv(src, srcLen,
            dst, dstBufLen);
#endif
    }
};

template<typename converter_t>
inline std::size_t conv_no_alloc(const typename converter_t::src_char_t* src,
    std::size_t srcLen, typename converter_t::dst_char_t* dst = nullptr,
    std::size_t dstBufLen = 0)
{
    return converter_t::conv(src, srcLen, dst, dstBufLen);
}

template<typename converter_t>
inline std::size_t conv_no_alloc(const typename converter_t::src_char_t* src,
    typename converter_t::dst_char_t* dst = nullptr, std::size_t dstBufLen = 0)
{
    return converter_t::conv(src, 0, dst, dstBufLen);
}

template<typename converter_t>
inline std::size_t conv_no_alloc(
    const std::basic_string<typename converter_t::src_char_t>& src,
    typename converter_t::dst_char_t* dst = nullptr, std::size_t dstBufLen = 0)
{
    return converter_t::conv(src.c_str(), src.length(), dst, dstBufLen);
}

template<typename converter_t>
inline std::unique_ptr<typename converter_t::dst_char_t[]> conv_unique_ptr(
    const typename converter_t::src_char_t* src, std::size_t srcLen = 0)
{
    // Compute required buffer length.
    std::size_t dstBufLen = converter_t::conv(src, srcLen, nullptr, 0);
    if (!srcLen) ++dstBufLen;

    // Create a buffer big enough to hold the converted text.
    std::unique_ptr<typename converter_t::dst_char_t[]> result(
        new typename converter_t::dst_char_t[dstBufLen]);

    // Convert the text and return the buffer.
    converter_t::conv(src, srcLen, result.get(), dstBufLen); // TODO: Check return value?
    return result;
}

template<typename converter_t>
inline std::unique_ptr<typename converter_t::dst_char_t[]> conv_unique_ptr(
    const std::basic_string<typename converter_t::src_char_t>& src)
{
    return conv_unique_ptr<converter_t>(src.c_str(), src.length());
}

template<typename converter_t>
inline std::basic_string<typename converter_t::dst_char_t> conv(
    const typename converter_t::src_char_t* src, std::size_t srcLen = 0)
{
    // Compute required string length.
    const std::size_t dstLen = converter_t::conv(src, srcLen, nullptr, 0);

    // Create a string big enough to hold the converted text.
    std::basic_string<typename converter_t::dst_char_t> result(dstLen,
        static_cast<typename converter_t::dst_char_t>('\0'));

    // Convert the text and return the string.
    converter_t::conv(src, srcLen, &result[0], dstLen + 1); // TODO: Check return value?
    return result;
}

template<typename converter_t>
inline std::basic_string<typename converter_t::dst_char_t> conv(
    const std::basic_string<typename converter_t::src_char_t>& src)
{
    return conv<converter_t>(src.c_str(), src.length());
}
} // text

enum class language
{
// Auto-generate this array.
#define HL_LANGUAGE_AUTOGEN(languageID) languageID,
#include "hl_languages_autogen.h"
};

HL_API language get_default_language();
} // hl
#endif
