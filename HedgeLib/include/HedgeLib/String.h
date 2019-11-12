#pragma once
#include "HedgeLib.h"
#include <cstddef>
#include <cstring>
#include <string>
#include <memory>

namespace hl
{
#ifdef _WIN32
#define HL_NTEXT(str) L##str
    using nchar = wchar_t;
#else
#define HL_NTEXT(str) str
    using nchar = char;
#endif

    using nstring = std::basic_string<nchar, std::char_traits<nchar>, std::allocator<nchar>>;

#define HL_TOLOWERASCII(c) ((c >= '[' || c <= '@') ? c : (c + 32))

    HL_API extern const char* const EmptyString;
    HL_API extern const nchar* const EmptyStringNative;

    HL_API std::size_t StringEncodeCodepointUTF8(
        char* u8str, char32_t codepoint);

    HL_API std::size_t StringEncodeCodepointUTF16(
        char16_t* u16str, char32_t codepoint);

    HL_API std::size_t StringDecodeCodepointUTF8(
        const char* u8str, char32_t& codepoint);

    HL_API std::size_t StringDecodeCodepointUTF16(
        const char16_t* u16str, char32_t& codepoint);

    inline std::size_t StringLength(const char* str)
    {
        return std::strlen(str);
    }

    inline char* StringCopy(const char* src, char* dst)
    {
        return std::strcpy(dst, src);
    }

    inline bool StringsEqual(const char* str1, const char* str2)
    {
        return !std::strcmp(str1, str2);
    }

    HL_API bool StringsEqualInvASCII(const char* str1, const char* str2);

#ifdef _WIN32
    inline std::size_t StringLength(const nchar* str)
    {
        return std::wcslen(str);
    }

    inline nchar* StringCopy(const nchar* src, nchar* dst)
    {
        return std::wcscpy(dst, src);
    }

    inline bool StringsEqual(const nchar* str1, const nchar* str2)
    {
        return !std::wcscmp(str1, str2);
    }

    HL_API bool StringsEqualInvASCII(const nchar* str1, const nchar* str2);
    HL_API bool StringsEqualInvASCII(const char* str1, const nchar* str2);
    HL_API bool StringsEqualInvASCII(const nchar* str1, const char* str2);
#endif

    inline std::size_t StringGetReqUTF8UnitCount(char32_t codepoint)
    {
        // Return the amount of code units required to store the given
        // codepoint, or 3 if the codepoint is invalid. (Invalid characters
        // are replaced with 0xFFFD which requires 3 units to encode)
        return ((codepoint < 0x80) ? 1 : (codepoint < 0x800) ? 2 :
            (codepoint < 0x10000) ? 3 : (codepoint < 0x110000) ? 4 : 3);
    }

    inline std::size_t StringGetReqUTF16UnitCount(char32_t codepoint)
    {
        // Return 2 if the given codepoint is not in the BMP, otherwise 1.
        // (Invalid characters are replaced with 0xFFFD which is in the BMP)
        return (codepoint >= 0x10000 && codepoint < 0x110000) ? 2 : 1;
    }

    HL_API std::size_t StringGetReqUTF16BufferCountUTF8(
        const char* str, std::size_t len = 0);

    inline std::size_t StringGetReqNativeBufferCountUTF8(
        const char* str, std::size_t len = 0)
    {
#ifdef _WIN32
        return StringGetReqUTF16BufferCountUTF8(str, len);
#else
        return (len) ? len : (std::strlen(str) + 1);
#endif
    }

    HL_API std::size_t StringGetReqUTF8BufferCountUTF16(
        const char16_t* str, std::size_t len = 0);

    HL_API std::size_t StringGetReqUTF8BufferCountCP932(
        const char* str, std::size_t len = 0);

    HL_API std::size_t StringGetReqUTF16BufferCountCP932(
        const char* str, std::size_t len = 0);

    HL_API std::size_t StringGetReqCP932BufferCountUTF8(
        const char* str, std::size_t len = 0);

    HL_API std::size_t StringGetReqCP932BufferCountUTF16(
        const char16_t* str, std::size_t len = 0);

    inline std::size_t StringGetReqNativeBufferCountUTF16(
        const char16_t* str, std::size_t len = 0)
    {
#ifdef _WIN32
        return (len) ? (std::wcslen(reinterpret_cast<const wchar_t*>(str)) + 1) : len;
#else
        return StringGetReqUTF8BufferCountUTF16(str, len);
#endif
    }

    HL_API std::unique_ptr<char16_t[]> StringConvertUTF8ToUTF16Ptr(
        const char* u8str, std::size_t u8bufLen = 0);

    HL_API std::u16string StringConvertUTF8ToUTF16(
        const char* u8str, std::size_t u8bufLen = 0);

    HL_API std::unique_ptr<char[]> StringConvertUTF8ToCP932Ptr(
        const char* u8str, std::size_t u8bufLen = 0);

    HL_API std::string StringConvertUTF8ToCP932(
        const char* u8str, std::size_t u8bufLen = 0);

    HL_API std::unique_ptr<char[]> StringConvertUTF16ToCP932Ptr(
        const char16_t* u16str, std::size_t u16bufLen = 0);

    HL_API std::string StringConvertUTF16ToCP932(
        const char16_t* u16str, std::size_t u16bufLen = 0);

    HL_API std::unique_ptr<nchar[]> StringConvertUTF8ToNativePtr(
        const char* u8str, std::size_t u8bufLen = 0);

    HL_API nstring StringConvertUTF8ToNative(
        const char* u8str, std::size_t u8bufLen = 0);

    HL_API std::unique_ptr<char[]> StringConvertUTF16ToUTF8Ptr(
        const char16_t* u16str, std::size_t u16bufLen = 0);

    HL_API std::string StringConvertUTF16ToUTF8(
        const char16_t* u16str, std::size_t u16bufLen = 0);

    HL_API std::unique_ptr<nchar[]> StringConvertUTF16ToNativePtr(
        const char16_t* u16str, std::size_t u16bufLen = 0);

    HL_API nstring StringConvertUTF16ToNative(
        const char16_t* u16str, std::size_t u16bufLen = 0);
    
    HL_API std::unique_ptr<char[]> StringConvertCP932ToUTF8Ptr(
        const char* cp932str, std::size_t cp932bufLen = 0);

    HL_API std::string StringConvertCP932ToUTF8(
        const char* cp932str, std::size_t cp932bufLen = 0);

    HL_API std::unique_ptr<char16_t[]> StringConvertCP932ToUTF16Ptr(
        const char* cp932str, std::size_t cp932bufLen = 0);

    HL_API std::u16string StringConvertCP932ToUTF16(
        const char* cp932str, std::size_t cp932bufLen = 0);

    HL_API std::unique_ptr<nchar[]> StringConvertCP932ToNativePtr(
        const char* cp932str, std::size_t cp932bufLen = 0);

    HL_API nstring StringConvertCP932ToNative(
        const char* cp932str, std::size_t cp932bufLen = 0);

    HL_API std::unique_ptr<char[]> StringJoinPtr(
        const char* str1, const char* str2);

    HL_API std::string StringJoin(const char* str1, const char* str2);

#ifdef _WIN32
    HL_API std::unique_ptr<nchar[]> StringJoinPtr(
        const nchar* str1, const nchar* str2);

    HL_API nstring StringJoin(const nchar* str1, const nchar* str2);
#endif
}
