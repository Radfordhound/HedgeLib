#include "hedgelib/hl_text.h"
#include <locale>

#ifdef _WIN32
#include "hl_in_win32.h"
#endif

namespace hl
{
namespace text
{
std::size_t utf8_to_utf16::conv(
    const src_char_t* src, std::size_t srcLen,
    dst_char_t* dst, std::size_t dstBufLen)
{
#ifdef _WIN32
    // Convert from UTF-8 to UTF-16.
    const std::size_t len = MultiByteToWideChar(
        CP_UTF8,                                    // CodePage
        0,                                          // dwFlags
        src,                                        // lpMultiByteStr
        (srcLen) ? static_cast<int>(srcLen) : -1,   // cbMultiByte
        reinterpret_cast<wchar_t*>(dst),            // lpWideCharStr
        static_cast<int>(dstBufLen));               // cchWideChar

    // Raise an error if necessary.
    if (len == 0)
    {
        HL_IN_WIN32_ERROR();
    }

    // Return length, not counting null terminator.
    return (srcLen) ? len : (len - 1);
#else
    // TODO
    HL_ERROR(error_type::unsupported);
    return 0;
#endif
}

std::size_t utf8_to_utf32::conv(
    const src_char_t* src, std::size_t srcLen,
    dst_char_t* dst, std::size_t dstBufLen)
{
    // TODO
    HL_ERROR(error_type::unsupported);
    return 0;
}

std::size_t utf16_to_utf8::conv(
    const src_char_t* src, std::size_t srcLen,
    dst_char_t* dst, std::size_t dstBufLen)
{
#ifdef _WIN32
    // Convert from UTF-16 to UTF-8.
    const std::size_t len = WideCharToMultiByte(
        CP_UTF8,                                    // CodePage
        0,                                          // dwFlags
        reinterpret_cast<const wchar_t*>(src),      // lpWideCharStr
        (srcLen) ? static_cast<int>(srcLen) : -1,   // cchWideChar
        dst,                                        // lpMultiByteStr
        static_cast<int>(dstBufLen),                // cbMultiByte
        0,                                          // lpDefaultChar
        0);                                         // lpUsedDefaultChar

    // Raise an error if necessary.
    if (len == 0)
    {
        HL_IN_WIN32_ERROR();
    }

    // Return length, not counting null terminator.
    return (srcLen) ? len : (len - 1);
#else
    // TODO
    HL_ERROR(error_type::unsupported);
    return 0;
#endif
}

std::size_t utf16_to_utf32::conv(
    const src_char_t* src, std::size_t srcLen,
    dst_char_t* dst, std::size_t dstBufLen)
{
    // TODO
    HL_ERROR(error_type::unsupported);
    return 0;
}

std::size_t utf32_to_utf8::conv(
    const src_char_t* src, std::size_t srcLen,
    dst_char_t* dst, std::size_t dstBufLen)
{
    // TODO
    HL_ERROR(error_type::unsupported);
    return 0;
}

std::size_t utf32_to_utf16::conv(
    const src_char_t* src, std::size_t srcLen,
    dst_char_t* dst, std::size_t dstBufLen)
{
    // TODO
    HL_ERROR(error_type::unsupported);
    return 0;
}
} // text

language get_default_language()
{
#ifdef _WIN32
    const LANGID lang = GetUserDefaultUILanguage();
    switch (PRIMARYLANGID(lang))
    {
    default:
    case LANG_ENGLISH:
        return language::english;
    }
#else
    std::locale locale = std::locale("");

    // TODO: Check the value of locale here.

    return language::english;
#endif
}
} // hl
