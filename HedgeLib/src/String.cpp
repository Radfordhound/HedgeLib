#include "HedgeLib/String.h"
#include "INString.h"
#include <algorithm>

#ifdef _WIN32
#include <Windows.h>
#endif

const char* const hl_EmptyString = "";

#ifdef _WIN32
const hl_NativeStr const hl_EmptyStringNative = L"";
#endif

void hl_INStringEncodeCodepointUTF8(uint32_t cp, char*& u8str)
{
    // 1-byte characters
    if (cp < 0x80)* u8str++ = static_cast<char>(cp);

    // 2-byte characters
    else if (cp < 0x800)
    {
        *u8str++ = (0b11000000 | ((cp & 0b11111000000) >> 6));
        *u8str++ = (0b10000000 | (cp & 0b111111));
    }

    // 3-byte characters
    else if (cp < 0x10000)
    {
        *u8str++ = (0b11100000 | ((cp & 0b1111000000000000) >> 12));
        *u8str++ = (0b10000000 | ((cp & 0b111111000000) >> 6));
        *u8str++ = (0b10000000 | (cp & 0b111111));
    }

    // 4-byte characters
    else if (cp < 0x110000)
    {
        *u8str++ = (0b11110000 | ((cp & 0b111000000000000000000) >> 18));
        *u8str++ = (0b10000000 | ((cp & 0b111111000000000000) >> 12));
        *u8str++ = (0b10000000 | ((cp & 0b111111000000) >> 6));
        *u8str++ = (0b10000000 | (cp & 0b111111));
    }

    // Invalid characters
    else
    {
        // Use the replacement character
        *u8str++ = -17; // 0xEF
        *u8str++ = -65; // 0xBF
        *u8str++ = -67; // 0xBD
    }
}

size_t hl_StringEncodeCodepointUTF8(char* u8str, uint32_t codepoint)
{
    if (!u8str) return 0;

    char* start = u8str;
    hl_INStringEncodeCodepointUTF8(codepoint, u8str);
    return static_cast<size_t>(u8str - start);
}

void hl_INStringEncodeCodepointUTF16(uint32_t cp, uint16_t*& u16str)
{
    // Basic Multilingual Plane characters
    if (cp < 0xD800 || (cp < 0x10000 && cp >= 0xE000))
    {
        *u16str++ = static_cast<uint16_t>(cp);
    }

    // Supplementary Plane characters
    else if (cp >= 0x10000 && cp < 0x110000)
    {
        cp -= 0x10000;
        *u16str++ = (0xD800 | ((cp & 0xFFC00) >> 10));
        *u16str++ = (0xDC00 | (cp & 0x3FF));
    }

    // Invalid characters
    else
    {
        // Use the replacement character
        *u16str++ = 0xFFFD;
    }
}

size_t hl_StringEncodeCodepointUTF16(uint16_t* u16str, uint32_t codepoint)
{
    if (!u16str) return 0;

    uint16_t* start = u16str;
    hl_INStringEncodeCodepointUTF16(codepoint, u16str);
    return static_cast<size_t>(u16str - start);
}

uint32_t hl_INStringDecodeCodepointUTF8(const char*& u8str)
{
    uint32_t cp, u = static_cast<uint32_t>(*u8str++);
    if (u > 0x7F)
    {
        // 2-byte characters
        if ((u & 0b11100000) == 0b11000000)
        {
            cp = (((u & 0b11111) << 6) | (*u8str++ & 0b111111));
        }

        // 3-byte characters
        else if ((u & 0b11110000) == 0b11100000)
        {
            cp = (((u & 0b1111) << 12) | ((*u8str++ & 0b111111) << 6));
            cp |= (*u8str++ & 0b111111);
        }

        // 4-byte characters
        else if ((u & 0b11111000) == 0b11110000)
        {
            cp = (((u & 0b111) << 18) | ((*u8str++ & 0b111111) << 12));
            cp |= ((*u8str++ & 0b111111) << 6);
            cp |= (*u8str++ & 0b111111);
        }

        // Invalid characters
        else
        {
            // Unicode replacement character
            cp = 0xFFFD;
        }

        return cp;
    }

    // 1-byte characters
    return u;
}

size_t hl_StringDecodeCodepointUTF8(const char* u8str, uint32_t* codepoint)
{
    if (!u8str || !codepoint) return 0;

    const char* start = u8str;
    *codepoint = hl_INStringDecodeCodepointUTF8(u8str);
    return static_cast<size_t>(u8str - start);
}

uint32_t hl_INStringDecodeCodepointUTF16(const uint16_t*& u16str)
{
    // Supplementary Plane characters
    uint32_t u = static_cast<uint32_t>(*u16str++);
    if (u > 0xD7FF && u < 0xE000)
    {
        // TODO: Check if second character is invalid (i.e. its first 6 bits are not 0b110111)
        return ((((u & 0x3FF) << 10) | (*u16str++ & 0x3FF)) + 0x10000);
    }

    // Basic Multilingual Plane characters
    return u;
}

size_t hl_StringDecodeCodepointUTF16(const uint16_t* u16str, uint32_t* codepoint)
{
    if (!u16str || !codepoint) return 0;
    
    const uint16_t* start = u16str;
    *codepoint = hl_INStringDecodeCodepointUTF16(u16str);
    return static_cast<size_t>(u16str - start);
}

template<typename str1_t, typename str2_t>
bool hl_INStringsEqualInvASCII(const str1_t* str1, const str2_t* str2)
{
    if (str1 == reinterpret_cast<const str1_t*>(str2)) return true;
    if (!str1 || !*str1) return false;

    while (*str1 || *str2)
    {
        if (*str1 != static_cast<str1_t>(*str2) && HL_TOLOWERASCII(*str1) !=
            static_cast<str1_t>(HL_TOLOWERASCII(*str2)))
        {
            return false;
        }

        ++str1;
        ++str2;
    }
    
    return true;
}

template bool hl_INStringsEqualInvASCII<char, char>(const char* str1, const char* str2);
#ifdef _WIN32
template bool hl_INStringsEqualInvASCII<hl_NativeChar, hl_NativeChar>(
    const hl_NativeChar* str1, const hl_NativeChar* str2);
template bool hl_INStringsEqualInvASCII<char, hl_NativeChar>(
    const char* str1, const hl_NativeChar* str2);
template bool hl_INStringsEqualInvASCII<hl_NativeChar, char>(
    const hl_NativeChar* str1, const char* str2);
#endif

bool hl_StringsEqualInvASCII(const char* str1, const char* str2)
{
    return hl_INStringsEqualInvASCII(str1, str2);
}

bool hl_StringsEqualInvASCIINative(
    const hl_NativeStr str1, const hl_NativeStr str2)
{
    return hl_INStringsEqualInvASCII(str1, str2);
}

size_t hl_INStringGetReqUTF16BufferCountUTF8(const char* str, size_t len)
{
#ifdef _WIN32
    // Figure out the amount of characters in the UTF-8 string
    int strLen = MultiByteToWideChar(CP_UTF8, 0, str,
        (len == 0) ? -1 : static_cast<int>(len), nullptr, 0);

    return static_cast<size_t>(strLen);
#endif

    // TODO: Support for non-Windows platforms
    return 0;
}

size_t hl_StringGetReqUTF16BufferCountUTF8(const char* str, size_t len)
{
    return (str) ? hl_INStringGetReqUTF16BufferCountUTF8(str, len) : 0;
}

size_t hl_INStringGetReqUTF8BufferCountUTF16(const uint16_t* str, size_t len)
{
#ifdef _WIN32
    // Figure out the amount of characters in the UTF-16 string
    int strLen = WideCharToMultiByte(CP_UTF8, 0,
        reinterpret_cast<const wchar_t*>(str), (len == 0) ?
        -1 : static_cast<int>(len), nullptr, 0, NULL, NULL);

    return static_cast<size_t>(strLen);
#endif

    // TODO: Support for non-Windows platforms
    return 0;
}

size_t hl_StringGetReqUTF8BufferCountUTF16(const uint16_t* str, size_t len)
{
    return (str) ? hl_INStringGetReqUTF8BufferCountUTF16(str, len) : 0;
}

HL_RESULT hl_INStringConvertUTF8ToUTF16NoAlloc(const char* u8str,
    uint16_t* u16str, size_t u16bufLen, size_t u8bufLen)
{
#ifdef _WIN32
    // Convert the UTF-8 string to UTF-16 and store it in the buffer
    MultiByteToWideChar(CP_UTF8, 0, u8str,
        (u8bufLen == 0) ? -1 : static_cast<int>(u8bufLen),
        reinterpret_cast<wchar_t*>(u16str),
        static_cast<int>(u16bufLen)); // TODO: Error checking?
#endif

    // TODO: Support for non-Windows platforms

    return HL_SUCCESS;
}

HL_RESULT hl_INStringConvertUTF8ToUTF16NoAlloc(
    const char* u8str, uint16_t* u16str, size_t u8bufLen)
{
#ifdef _WIN32
    // Determine the size of the buffer required to hold the string's UTF-16 equivalent
    size_t u16bufLen = hl_INStringGetReqUTF16BufferCountUTF8(
        u8str, u8bufLen);

    // Convert to UTF-16
    return hl_INStringConvertUTF8ToUTF16NoAlloc(u8str,
        u16str, u16bufLen, u8bufLen);
#endif

    // TODO: Support for non-Windows platforms

    return HL_SUCCESS;
}

HL_RESULT hl_INStringConvertUTF8ToUTF16(const char* u8str,
    uint16_t** u16str, size_t u16bufLen, size_t u8bufLen)
{
#ifdef _WIN32
    // Allocate a buffer big enough to hold the UTF-16 string
    *u16str = static_cast<uint16_t*>(malloc(
        u16bufLen * sizeof(uint16_t)));

    if (!*u16str) return HL_ERROR_OUT_OF_MEMORY;

    // Convert to UTF-16
    return hl_INStringConvertUTF8ToUTF16NoAlloc(
        u8str, *u16str, u16bufLen, u8bufLen);
#endif

    // TODO: Support for non-Windows platforms

    return HL_SUCCESS;
}

HL_RESULT hl_INStringConvertUTF8ToUTF16(const char* u8str,
    uint16_t** u16str, size_t u8bufLen)
{
#ifdef _WIN32
    // Determine the size of the buffer required to hold the string's UTF-16 equivalent
    size_t u16bufLen = hl_INStringGetReqUTF16BufferCountUTF8(
        u8str, u8bufLen);

    // Convert to UTF-16
    return hl_INStringConvertUTF8ToUTF16(
        u8str, u16str, u16bufLen, u8bufLen);
#endif
    
    // TODO: Support for non-Windows platforms

    return HL_SUCCESS;
}

enum HL_RESULT hl_StringConvertUTF8ToUTF16(const char* u8str,
    uint16_t** u16str, size_t u8bufLen)
{
    if (!u8str || !u16str) return HL_ERROR_UNKNOWN;
    return hl_INStringConvertUTF8ToUTF16(u8str, u16str, u8bufLen);
}

HL_RESULT hl_INStringConvertUTF8ToNative(const char* u8str,
    hl_NativeStr* nativeStr, size_t u8bufLen)
{
#ifdef _WIN32
    return hl_INStringConvertUTF8ToUTF16(u8str,
        reinterpret_cast<uint16_t**>(nativeStr), u8bufLen);
#else
    // Allocate a buffer big enough to hold a copy of the string
    if (u8bufLen == 0) u8bufLen = (strlen(u8str) + 1);
    *nativeStr = static_cast<hl_NativeStr>(malloc(u8bufLen));

    if (!*nativeStr) return HL_ERROR_OUT_OF_MEMORY;

    // Copy the string
    std::copy(u8str, u8str + u8bufLen, *nativeStr);
    return HL_SUCCESS;
#endif
}

enum HL_RESULT hl_StringConvertUTF8ToNative(const char* u8str,
    hl_NativeStr* nativeStr, size_t u8bufLen)
{
    if (!u8str || !nativeStr) return HL_ERROR_UNKNOWN;
    return hl_INStringConvertUTF8ToNative(u8str, nativeStr, u8bufLen);
}

HL_RESULT hl_INStringConvertUTF16ToUTF8NoAlloc(const uint16_t* u16str,
    char* u8str, size_t u8bufLen, size_t u16bufLen)
{
#ifdef _WIN32
    // Convert the UTF-16 string to UTF-8 and store it in the buffer
    WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<const wchar_t*>(u16str),
        (u16bufLen == 0) ? -1 : static_cast<int>(u16bufLen),
        u8str, static_cast<int>(u8bufLen), NULL, NULL); // TODO: Error checking?
#endif

    // TODO: Support for non-Windows platforms

    return HL_SUCCESS;
}

HL_RESULT hl_INStringConvertUTF16ToUTF8NoAlloc(
    const uint16_t* u16str, char* u8str, size_t u16bufLen)
{
#ifdef _WIN32
    // Determine the size of the buffer required to hold the string's UTF-8 equivalent
    size_t u8bufLen = hl_INStringGetReqUTF8BufferCountUTF16(
        u16str, u16bufLen);

    // Convert to UTF-8
    return hl_INStringConvertUTF16ToUTF8NoAlloc(u16str,
        u8str, u8bufLen, u16bufLen);
#endif

    // TODO: Support for non-Windows platforms

    return HL_SUCCESS;
}

HL_RESULT hl_INStringConvertUTF16ToUTF8(const uint16_t* u16str,
    char** u8str, size_t u8bufLen, size_t u16bufLen)
{
#ifdef _WIN32
    // Allocate a buffer big enough to hold the UTF-8 string
    *u8str = static_cast<char*>(malloc(u8bufLen));
    if (!*u8str) return HL_ERROR_OUT_OF_MEMORY;

    // Convert to UTF-8
    return hl_INStringConvertUTF16ToUTF8NoAlloc(
        u16str, *u8str, u8bufLen, u16bufLen);
#endif

    // TODO: Support for non-Windows platforms

    return HL_SUCCESS;
}

HL_RESULT hl_INStringConvertUTF16ToUTF8(const uint16_t* u16str,
    char** u8str, size_t u16bufLen)
{
#ifdef _WIN32
    // Determine the size of the buffer required to hold the string's UTF-8 equivalent
    size_t u8bufLen = hl_INStringGetReqUTF8BufferCountUTF16(
        u16str, u16bufLen);

    // Convert to UTF-8
    return hl_INStringConvertUTF16ToUTF8(
        u16str, u8str, u8bufLen, u16bufLen);
#endif

    // TODO: Support for non-Windows platforms

    return HL_SUCCESS;
}

enum HL_RESULT hl_StringConvertUTF16ToUTF8(
    const uint16_t* u16str, char** u8str, size_t u16bufLen)
{
    if (!u16str || !u8str) return HL_ERROR_UNKNOWN;
    return hl_INStringConvertUTF16ToUTF8(u16str, u8str, u16bufLen);
}

HL_RESULT hl_INStringConvertUTF16ToNative(
    const uint16_t* u16str, hl_NativeStr* nativeStr, size_t u16bufLen)
{
#ifdef _WIN32
    // Allocate a buffer big enough to hold a copy of the string
    if (u16bufLen == 0)
    {
        u16bufLen = (wcslen(
            reinterpret_cast<const wchar_t*>(u16str)) + 1);
    }

    *nativeStr = static_cast<hl_NativeStr>(malloc(u16bufLen));
    if (!*nativeStr) return HL_ERROR_OUT_OF_MEMORY;

    // Copy the string
    std::copy(u16str, u16str + u16bufLen + 1, *nativeStr);
    return HL_SUCCESS;
#else
    return hl_INStringConvertUTF16ToUTF8(
        u16str, nativeStr, u16bufLen);
#endif
}

enum HL_RESULT hl_StringConvertUTF16ToNative(
    const uint16_t* u16str, hl_NativeStr* nativeStr, size_t u16bufLen)
{
    if (!u16str || !nativeStr) return HL_ERROR_UNKNOWN;
    return hl_INStringConvertUTF16ToNative(u16str, nativeStr, u16bufLen);
}
