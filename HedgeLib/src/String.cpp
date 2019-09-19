#include "HedgeLib/String.h"
#include "HedgeLib/Endian.h"
#include "INString.h"
#include "INCP932Mappings.h"
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

template<typename char_t>
size_t hl_INStringGetReqUnicodeBufferCountCP932(
    const char* str, size_t len)
{
    // Get CP-932 buffer length if necessary
    // TODO: Optimize-out strlen here by using while loop
    if (!len) len = (strlen(str) + 1);

    // Get amount of code units necessary to encode the
    // CP-932 string in UTF-8, UTF-16, or UTF-32
    size_t reqBufLen = 0;
    for (size_t i = 0; i < len; ++i)
    {
        // ASCII characters
        if (str[i] >= 0) ++reqBufLen;
        else
        {
            // JIS X 0201 characters
            uint16_t c;
            if (str[i] < -32 && str[i] > -96)
            {
                c = hl_INJIS0201ToUnicode[(
                    static_cast<unsigned char>(str[i]) - 0xA1)];
            }

            // Other characters
            else
            {
                // Get two-byte CP-932 character
                c = *reinterpret_cast<const uint16_t*>(str + i);
                hl_SwapUInt16(&c); // CP-932 strings are always big endian

                // Find matching Unicode codepoint to use, if any
                if (c <= 0x84BE && c >= 0x8140)
                {
                    c = hl_INCP932ToUnicode1[c - 0x8140];
                }
                else if (c <= 0x9FFC && c >= 0x8740)
                {
                    c = hl_INCP932ToUnicode2[c - 0x8740];
                }
                else if (c <= 0xEAA4 && c >= 0xE040)
                {
                    c = hl_INCP932ToUnicode3[c - 0xE040];
                }
                else if (c <= 0xEEFC && c >= 0xED40)
                {
                    c = hl_INCP932ToUnicode4[c - 0xED40];
                }
                else if (c <= 0xFC4B && c >= 0xFA40)
                {
                    c = hl_INCP932ToUnicode5[c - 0xFA40];
                }
                else
                {
                    // Unicode replacement character
                    c = 0xFFFD;
                }

                ++i;
            }

            // Get the number of code units required to encode this codepoint
            if constexpr (sizeof(char_t) == sizeof(char))
            {
                // UTF-8
                reqBufLen += hl_StringGetReqUTF8UnitCount(c);
            }
            else if constexpr (sizeof(char_t) == sizeof(uint16_t))
            {
                // UTF-16
                reqBufLen += hl_StringGetReqUTF16UnitCount(c);
            }
            else if constexpr (sizeof(char_t) == sizeof(uint32_t))
            {
                // UTF-32
                ++reqBufLen;
            }
        }
    }

    return reqBufLen;
}

size_t hl_StringGetReqUTF8BufferCountCP932(const char* str, size_t len)
{
    return (str) ? hl_INStringGetReqUnicodeBufferCountCP932<char>(str, len) : 0;
}

size_t hl_StringGetReqUTF16BufferCountCP932(const char* str, size_t len)
{
    return (str) ? hl_INStringGetReqUnicodeBufferCountCP932<uint16_t>(str, len) : 0;
}

template<typename char_t>
size_t hl_INStringGetReqCP932BufferCountUnicode(
    const char_t* str, size_t len)
{
    // Convert CP-932 string to UTF-8, UTF-16, or UTF-32 string
    size_t reqBufLen = (len) ? 0 : 1;
    bool breakOnNull = (!len);

    while ((breakOnNull && *str) || len)
    {
        if (len) --len;

        // Get current unicode code point
        uint32_t c;
        if constexpr (sizeof(char_t) == sizeof(char))
        {
            // UTF-8
            c = hl_INStringDecodeCodepointUTF8(
                reinterpret_cast<const char*&>(str));
        }
        else if constexpr (sizeof(char_t) == sizeof(uint16_t))
        {
            // UTF-16
            c = hl_INStringDecodeCodepointUTF16(
                reinterpret_cast<const uint16_t*&>(str));
        }
        else if constexpr (sizeof(char_t) == sizeof(uint32_t))
        {
            // UTF-32
            c = static_cast<uint32_t>(*str++);
        }
        else
        {
            // TODO: Error
        }

        // Single-byte characters
        if (c < 0x80 || (c > 0xFF61 && c < 0xFF9F))
        {
            ++reqBufLen;
            continue;
        }

        // Multi-byte characters
        else if (c <= 0xF7 && c >= 0xA7)
        {
            c = hl_INUnicodeToCP9321[c - 0xA7];
        }
        else if (c <= 0x451 && c >= 0x391)
        {
            c = hl_INUnicodeToCP9322[c - 0x391];
        }
        else if (c <= 0x266F && c >= 0x2010)
        {
            c = hl_INUnicodeToCP9323[c - 0x2010];
        }
        else if (c <= 0x33CD && c >= 0x3000)
        {
            c = hl_INUnicodeToCP9324[c - 0x3000];
        }
        else if (c <= 0x9FA0 && c >= 0x4E00)
        {
            c = hl_INUnicodeToCP9325[c - 0x4E00];
        }
        else if (c <= 0xFA2D && c >= 0xF929)
        {
            c = hl_INUnicodeToCP9326[c - 0xF929];
        }
        else if (c <= 0xFFE5 && c >= 0xFF01)
        {
            c = hl_INUnicodeToCP9327[c - 0xFF01];
        }

        // Invalid characters
        else
        {
            // Use a standard ASCII ? instead
            ++reqBufLen;
            continue;
        }

        // Increase buffer length by required amount to store CP-932 character
        reqBufLen += (c > 0xFF) ? 2 : 1;
    }

    return reqBufLen;
}

size_t hl_StringGetReqCP932BufferCountUTF8(const char* str, size_t len)
{
    return (str) ? hl_INStringGetReqCP932BufferCountUnicode(str, len) : 0;
}

size_t hl_StringGetReqCP932BufferCountUTF16(const uint16_t* str, size_t len)
{
    return (str) ? hl_INStringGetReqCP932BufferCountUnicode(str, len) : 0;
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

template<typename char_t>
HL_RESULT hl_INStringConvertUnicodeToCP932NoAlloc(
    const char_t* ustr, char* cp932str, size_t ubufLen)
{
    // Convert CP-932 string to UTF-8, UTF-16, or UTF-32 string
    bool breakOnNull = (!ubufLen);
    while ((breakOnNull && *ustr) || ubufLen)
    {
        if (ubufLen) --ubufLen;

        // Get current unicode code point
        uint32_t c;
        if constexpr (sizeof(char_t) == sizeof(char))
        {
            // UTF-8
            c = hl_INStringDecodeCodepointUTF8(
                reinterpret_cast<const char*&>(ustr));
        }
        else if constexpr (sizeof(char_t) == sizeof(uint16_t))
        {
            // UTF-16
            c = hl_INStringDecodeCodepointUTF16(
                reinterpret_cast<const uint16_t * &>(ustr));
        }
        else if constexpr (sizeof(char_t) == sizeof(uint32_t))
        {
            // UTF-32
            c = static_cast<uint32_t>(*ustr++);
        }
        else
        {
            // TODO: Error
        }

        // ASCII characters
        if (c < 0x80)
        {
            *cp932str++ = static_cast<char>(c);
            continue;
        }

        // JIS X 0201 characters
        else if (c > 0xFF61 && c < 0xFF9F)
        {
            *cp932str++ = static_cast<char>(
                hl_INUnicodeToJIS0201[c - 0xFF61]);
            continue;
        }

        // Multi-byte characters
        else if (c <= 0xF7 && c >= 0xA7)
        {
            c = hl_INUnicodeToCP9321[c - 0xA7];
        }
        else if (c <= 0x451 && c >= 0x391)
        {
            c = hl_INUnicodeToCP9322[c - 0x391];
        }
        else if (c <= 0x266F && c >= 0x2010)
        {
            c = hl_INUnicodeToCP9323[c - 0x2010];
        }
        else if (c <= 0x33CD && c >= 0x3000)
        {
            c = hl_INUnicodeToCP9324[c - 0x3000];
        }
        else if (c <= 0x9FA0 && c >= 0x4E00)
        {
            c = hl_INUnicodeToCP9325[c - 0x4E00];
        }
        else if (c <= 0xFA2D && c >= 0xF929)
        {
            c = hl_INUnicodeToCP9326[c - 0xF929];
        }
        else if (c <= 0xFFE5 && c >= 0xFF01)
        {
            c = hl_INUnicodeToCP9327[c - 0xFF01];
        }

        // Invalid characters
        else
        {
            // Use a standard ASCII ? instead
            *cp932str++ = '?';
            continue;
        }

        // Store CP-932 codepoint as two-byte character
        *reinterpret_cast<uint16_t*>(cp932str) = static_cast<uint16_t>(c);
        cp932str += 2;
    }

    // Copy the null terminator too if necessary
    if (breakOnNull) *cp932str = '\0';

    return HL_SUCCESS;
}

HL_RESULT hl_INStringConvertUTF8ToCP932(const char* u8str,
    char** cp932str, size_t cp932bufLen, size_t u8bufLen)
{
    // Allocate a buffer big enough to hold the CP-932 string
    *cp932str = static_cast<char*>(malloc(cp932bufLen));
    if (!*cp932str) return HL_ERROR_OUT_OF_MEMORY;

    // Convert to CP-932
    return hl_INStringConvertUnicodeToCP932NoAlloc(
        u8str, *cp932str, u8bufLen);
}

HL_RESULT hl_INStringConvertUTF8ToCP932(const char* u8str,
    char** cp932str, size_t u8bufLen)
{
    // Determine the size of the buffer required to hold the string's CP-932 equivalent
    size_t cp932bufLen = hl_INStringGetReqCP932BufferCountUnicode(
        u8str, u8bufLen);

    // Convert to CP-932
    return hl_INStringConvertUTF8ToCP932(
        u8str, cp932str, cp932bufLen, u8bufLen);
}

enum HL_RESULT hl_StringConvertUTF8ToCP932(
    const char* u8str, char** cp932str, size_t u8bufLen)
{
    if (!u8str || !cp932str) return HL_ERROR_UNKNOWN;
    return hl_INStringConvertUTF8ToCP932(u8str, cp932str, u8bufLen);
}

HL_RESULT hl_INStringConvertUTF16ToCP932(const uint16_t* u16str,
    char** cp932str, size_t cp932bufLen, size_t u16bufLen)
{
    // Allocate a buffer big enough to hold the CP-932 string
    *cp932str = static_cast<char*>(malloc(cp932bufLen));
    if (!*cp932str) return HL_ERROR_OUT_OF_MEMORY;

    // Convert to CP-932
    return hl_INStringConvertUnicodeToCP932NoAlloc(
        u16str, *cp932str, u16bufLen);
}

HL_RESULT hl_INStringConvertUTF16ToCP932(const uint16_t* u16str,
    char** cp932str, size_t u16bufLen)
{
    // Determine the size of the buffer required to hold the string's CP-932 equivalent
    size_t cp932bufLen = hl_INStringGetReqCP932BufferCountUnicode(
        u16str, u16bufLen);

    // Convert to CP-932
    return hl_INStringConvertUTF16ToCP932(
        u16str, cp932str, cp932bufLen, u16bufLen);
}

enum HL_RESULT hl_StringConvertUTF16ToCP932(
    const uint16_t* u16str, char** cp932str, size_t u16bufLen)
{
    if (!u16str || !cp932str) return HL_ERROR_UNKNOWN;
    return hl_INStringConvertUTF16ToCP932(u16str, cp932str, u16bufLen);
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

template<typename char_t>
HL_RESULT hl_INStringConvertCP932ToUnicodeNoAlloc(
    const char* cp932str, char_t* ustr, size_t cp932bufLen)
{
    // Get CP-932 buffer length if necessary
    // TODO: Optimize-out this strlen using a while loop like in hl_INStringConvertUnicodeToCP932NoAlloc
    if (!cp932bufLen) cp932bufLen = (strlen(cp932str) + 1);

    // Convert CP-932 string to UTF-8, UTF-16, or UTF-32 string
    for (size_t i = 0; i < cp932bufLen; ++i)
    {
        // ASCII characters
        if (cp932str[i] >= 0)
        {
            // ASCII characters are the same in CP-932 and Unicode; just copy them
            *ustr++ = static_cast<char_t>(cp932str[i]);
        }
        else
        {
            // JIS X 0201 characters
            uint16_t c;
            if (cp932str[i] < -32 && cp932str[i] > -96)
            {
                c = hl_INJIS0201ToUnicode[(
                    static_cast<unsigned char>(cp932str[i]) - 0xA1)];
            }

            // Other characters
            else
            {

                // Get two-byte CP-932 character
                c = *reinterpret_cast<const uint16_t*>(cp932str + i);
                hl_SwapUInt16(&c); // CP-932 strings are always big endian

                // Find matching Unicode codepoint to use, if any
                if (c <= 0x84BE && c >= 0x8140)
                {
                    c = hl_INCP932ToUnicode1[c - 0x8140];
                }
                else if (c <= 0x9FFC && c >= 0x8740)
                {
                    c = hl_INCP932ToUnicode2[c - 0x8740];
                }
                else if (c <= 0xEAA4 && c >= 0xE040)
                {
                    c = hl_INCP932ToUnicode3[c - 0xE040];
                }
                else if (c <= 0xEEFC && c >= 0xED40)
                {
                    c = hl_INCP932ToUnicode4[c - 0xED40];
                }
                else if (c <= 0xFC4B && c >= 0xFA40)
                {
                    c = hl_INCP932ToUnicode5[c - 0xFA40];
                }
                else
                {
                    // Unicode replacement character
                    c = 0xFFFD;
                }

                ++i;
            }

            // Encode the codepoint
            if constexpr (sizeof(char_t) == sizeof(char))
            {
                // UTF-8
                hl_INStringEncodeCodepointUTF8(c,
                    reinterpret_cast<char*&>(ustr));
            }
            else if constexpr (sizeof(char_t) == sizeof(uint16_t))
            {
                // UTF-16
                hl_INStringEncodeCodepointUTF16(c,
                    reinterpret_cast<uint16_t * &>(ustr));
            }
            else if constexpr (sizeof(char_t) == sizeof(uint32_t))
            {
                // UTF-32
                *ustr++ = static_cast<char_t>(c);
            }
        }
    }

    return HL_SUCCESS;
}

HL_RESULT hl_INStringConvertCP932ToUTF8(const char* cp932str,
    char** u8str, size_t u8bufLen, size_t cp932bufLen)
{
    // Allocate a buffer big enough to hold the UTF-8 string
    *u8str = static_cast<char*>(malloc(u8bufLen));
    if (!*u8str) return HL_ERROR_OUT_OF_MEMORY;

    // Convert to UTF-8
    return hl_INStringConvertCP932ToUnicodeNoAlloc(
        cp932str, *u8str, cp932bufLen);
}

HL_RESULT hl_INStringConvertCP932ToUTF8(const char* cp932str,
    char** u8str, size_t cp932bufLen)
{
    // Determine the size of the buffer required to hold the string's UTF-8 equivalent
    size_t u8bufLen = hl_INStringGetReqUnicodeBufferCountCP932<char>(
        cp932str, cp932bufLen);

    // Convert to UTF-8
    return hl_INStringConvertCP932ToUTF8(
        cp932str, u8str, u8bufLen, cp932bufLen);
}

enum HL_RESULT hl_StringConvertCP932ToUTF8(
    const char* cp932str, char** u8str, size_t cp932bufLen)
{
    if (!cp932str || !u8str) return HL_ERROR_UNKNOWN;
    return hl_INStringConvertCP932ToUTF8(cp932str, u8str, cp932bufLen);
}

HL_RESULT hl_INStringConvertCP932ToUTF16(const char* cp932str,
    uint16_t** u16str, size_t u16bufLen, size_t cp932bufLen)
{
    // Allocate a buffer big enough to hold the UTF-16 string
    *u16str = static_cast<uint16_t*>(malloc(
        u16bufLen * sizeof(uint16_t)));

    if (!*u16str) return HL_ERROR_OUT_OF_MEMORY;

    // Convert to UTF-16
    return hl_INStringConvertCP932ToUnicodeNoAlloc(
        cp932str, *u16str, cp932bufLen);
}

HL_RESULT hl_INStringConvertCP932ToUTF16(const char* cp932str,
    uint16_t** u16str, size_t cp932bufLen)
{
    // Determine the size of the buffer required to hold the string's UTF-16 equivalent
    size_t u16bufLen = hl_INStringGetReqUnicodeBufferCountCP932<uint16_t>(
        cp932str, cp932bufLen);

    // Convert to UTF-16
    return hl_INStringConvertCP932ToUTF16(
        cp932str, u16str, u16bufLen, cp932bufLen);
}

enum HL_RESULT hl_StringConvertCP932ToUTF16(
    const char* cp932str, uint16_t** u16str, size_t cp932bufLen)
{
    if (!cp932str || !u16str) return HL_ERROR_UNKNOWN;
    return hl_INStringConvertCP932ToUTF16(cp932str, u16str, cp932bufLen);
}

HL_RESULT hl_INStringConvertCP932ToNative(
    const char* cp932str, hl_NativeStr* nativeStr, size_t cp932bufLen)
{
#ifdef _WIN32
    return hl_INStringConvertCP932ToUTF16(cp932str,
        reinterpret_cast<uint16_t**>(nativeStr), cp932bufLen);
#else
    return hl_INStringConvertCP932ToUTF8(
        cp932str, nativeStr, cp932bufLen);
#endif
}

enum HL_RESULT hl_StringConvertCP932ToNative(
    const char* cp932str, hl_NativeStr* nativeStr, size_t cp932bufLen)
{
    if (!cp932str || !nativeStr) return HL_ERROR_UNKNOWN;
    return hl_INStringConvertCP932ToNative(cp932str, nativeStr, cp932bufLen);
}
