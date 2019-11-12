#include "HedgeLib/String.h"
#include "HedgeLib/Endian.h"
#include "INCP932Mappings.h"
#include <algorithm>
#include <stdexcept>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace hl
{
    const char* const EmptyString = "";
    const nchar* const EmptyStringNative = HL_NTEXT("");

    void INStringEncodeCodepointUTF8(char*& u8str, char32_t cp)
    {
        // 1-byte characters
        if (cp < 0x80)*u8str++ = static_cast<char>(cp);

        // 2-byte characters
        else if (cp < 0x800)
        {
            *u8str++ = static_cast<char>(0b11000000 | ((cp & 0b11111000000) >> 6));
            *u8str++ = static_cast<char>(0b10000000 | (cp & 0b111111));
        }

        // 3-byte characters
        else if (cp < 0x10000)
        {
            *u8str++ = static_cast<char>(0b11100000 | ((cp & 0b1111000000000000) >> 12));
            *u8str++ = static_cast<char>(0b10000000 | ((cp & 0b111111000000) >> 6));
            *u8str++ = static_cast<char>(0b10000000 | (cp & 0b111111));
        }

        // 4-byte characters
        else if (cp < 0x110000)
        {
            *u8str++ = static_cast<char>(0b11110000 | ((cp & 0b111000000000000000000) >> 18));
            *u8str++ = static_cast<char>(0b10000000 | ((cp & 0b111111000000000000) >> 12));
            *u8str++ = static_cast<char>(0b10000000 | ((cp & 0b111111000000) >> 6));
            *u8str++ = static_cast<char>(0b10000000 | (cp & 0b111111));
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

    std::size_t StringEncodeCodepointUTF8(char* u8str, char32_t codepoint)
    {
        if (!u8str) return 0;

        char* start = u8str;
        INStringEncodeCodepointUTF8(u8str, codepoint);
        return static_cast<std::size_t>(u8str - start);
    }

    void INStringEncodeCodepointUTF16(char16_t*& u16str, char32_t cp)
    {
        // Basic Multilingual Plane characters
        if (cp < 0xD800 || (cp < 0x10000 && cp >= 0xE000))
        {
            *u16str++ = static_cast<char16_t>(cp);
        }

        // Supplementary Plane characters
        else if (cp >= 0x10000 && cp < 0x110000)
        {
            cp -= 0x10000;
            *u16str++ = static_cast<char16_t>(0xD800 | ((cp & 0xFFC00) >> 10));
            *u16str++ = static_cast<char16_t>(0xDC00 | (cp & 0x3FF));
        }

        // Invalid characters
        else
        {
            // Use the replacement character
            *u16str++ = 0xFFFD;
        }
    }

    std::size_t StringEncodeCodepointUTF16(char16_t* u16str, char32_t codepoint)
    {
        if (!u16str) return 0;

        char16_t* start = u16str;
        INStringEncodeCodepointUTF16(u16str, codepoint);
        return static_cast<std::size_t>(u16str - start);
    }

    char32_t INStringDecodeCodepointUTF8(const char*& u8str)
    {
        char32_t cp, u = static_cast<char32_t>(*u8str++);
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

    std::size_t StringDecodeCodepointUTF8(const char* u8str, char32_t& codepoint)
    {
        if (!u8str) return 0;

        const char* start = u8str;
        codepoint = INStringDecodeCodepointUTF8(u8str);
        return static_cast<std::size_t>(u8str - start);
    }

    char32_t INStringDecodeCodepointUTF16(const char16_t*& u16str)
    {
        // Supplementary Plane characters
        char32_t u = static_cast<char32_t>(*u16str++);
        if (u > 0xD7FF && u < 0xE000)
        {
            // TODO: Check if second character is invalid (i.e. its first 6 bits are not 0b110111)
            return ((((u & 0x3FF) << 10) | (*u16str++ & 0x3FF)) + 0x10000);
        }

        // Basic Multilingual Plane characters
        return u;
    }

    std::size_t StringDecodeCodepointUTF16(const char16_t* u16str, char32_t& codepoint)
    {
        if (!u16str) return 0;

        const char16_t* start = u16str;
        codepoint = INStringDecodeCodepointUTF16(u16str);
        return static_cast<std::size_t>(u16str - start);
    }

    template<typename str1_t, typename str2_t>
    bool INStringsEqualInvASCII(const str1_t* str1, const str2_t* str2)
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

    template bool INStringsEqualInvASCII<char, char>(const char* str1, const char* str2);

    bool StringsEqualInvASCII(const char* str1, const char* str2)
    {
        return INStringsEqualInvASCII(str1, str2);
    }

#ifdef _WIN32
    template bool INStringsEqualInvASCII<nchar, nchar>(
        const nchar* str1, const nchar* str2);
    template bool INStringsEqualInvASCII<char, nchar>(
        const char* str1, const nchar* str2);
    template bool INStringsEqualInvASCII<nchar, char>(
        const nchar* str1, const char* str2);

    bool StringsEqualInvASCII(const nchar* str1, const nchar* str2)
    {
        return INStringsEqualInvASCII(str1, str2);
    }

    bool StringsEqualInvASCII(const char* str1, const nchar* str2)
    {
        return INStringsEqualInvASCII(str1, str2);
    }

    bool StringsEqualInvASCII(const nchar* str1, const char* str2)
    {
        return INStringsEqualInvASCII(str1, str2);
    }
#endif

    std::size_t INStringGetReqUTF16BufferCountUTF8(const char* str, std::size_t len)
    {
#ifdef _WIN32
        // Figure out the amount of characters in the UTF-8 string
        int strLen = MultiByteToWideChar(CP_UTF8, 0, str,
            (len == 0) ? -1 : static_cast<int>(len), nullptr, 0);

        return static_cast<std::size_t>(strLen);
#else
        // TODO: Support for non-Windows platforms
        throw std::logic_error(
            "This function has not yet been implemented on non-Windows platforms.");
#endif
    }

    std::size_t StringGetReqUTF16BufferCountUTF8(const char* str, std::size_t len)
    {
        return (str) ? INStringGetReqUTF16BufferCountUTF8(str, len) : 0;
    }

    std::size_t INStringGetReqUTF8BufferCountUTF16(const char16_t* str, std::size_t len)
    {
#ifdef _WIN32
        // Figure out the amount of characters in the UTF-16 string
        int strLen = WideCharToMultiByte(CP_UTF8, 0,
            reinterpret_cast<const wchar_t*>(str), (len == 0) ?
            -1 : static_cast<int>(len), nullptr, 0, NULL, NULL);

        return static_cast<std::size_t>(strLen);
#else
        // TODO: Support for non-Windows platforms
        throw std::logic_error(
            "This function has not yet been implemented on non-Windows platforms.");
#endif
    }

    std::size_t StringGetReqUTF8BufferCountUTF16(const char16_t* str, std::size_t len)
    {
        return (str) ? INStringGetReqUTF8BufferCountUTF16(str, len) : 0;
    }

    template<typename char_t>
    std::size_t INStringGetReqUnicodeBufferCountCP932(
        const char* str, std::size_t len)
    {
        // Get CP-932 buffer length if necessary
        // TODO: Optimize-out strlen here by using while loop
        if (!len) len = (std::strlen(str) + 1);

        // Get amount of code units necessary to encode the
        // CP-932 string in UTF-8, UTF-16, or UTF-32
        std::size_t reqBufLen = 0;
        for (std::size_t i = 0; i < len; ++i)
        {
            // ASCII characters
            if (str[i] >= 0) ++reqBufLen;
            else
            {
                // JIS X 0201 characters
                char16_t c;
                if (str[i] < -32 && str[i] > -96)
                {
                    c = INJIS0201ToUnicode[(
                        static_cast<unsigned char>(str[i]) - 0xA1)];
                }

                // Other characters
                else
                {
                    // Get two-byte CP-932 character
                    c = *reinterpret_cast<const char16_t*>(str + i);
                    Swap(c); // CP-932 strings are always big endian

                    // Find matching Unicode codepoint to use, if any
                    if (c <= 0x84BE && c >= 0x8140)
                    {
                        c = INCP932ToUnicode1[c - 0x8140];
                    }
                    else if (c <= 0x9FFC && c >= 0x8740)
                    {
                        c = INCP932ToUnicode2[c - 0x8740];
                    }
                    else if (c <= 0xEAA4 && c >= 0xE040)
                    {
                        c = INCP932ToUnicode3[c - 0xE040];
                    }
                    else if (c <= 0xEEFC && c >= 0xED40)
                    {
                        c = INCP932ToUnicode4[c - 0xED40];
                    }
                    else if (c <= 0xFC4B && c >= 0xFA40)
                    {
                        c = INCP932ToUnicode5[c - 0xFA40];
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
                    reqBufLen += StringGetReqUTF8UnitCount(c);
                }
                else if constexpr (sizeof(char_t) == sizeof(char16_t))
                {
                    // UTF-16
                    reqBufLen += StringGetReqUTF16UnitCount(c);
                }
                else if constexpr (sizeof(char_t) == sizeof(char32_t))
                {
                    // UTF-32
                    ++reqBufLen;
                }
            }
        }

        return reqBufLen;
    }

    std::size_t StringGetReqUTF8BufferCountCP932(const char* str, std::size_t len)
    {
        return (str) ? INStringGetReqUnicodeBufferCountCP932<char>(str, len) : 0;
    }

    std::size_t StringGetReqUTF16BufferCountCP932(const char* str, std::size_t len)
    {
        return (str) ? INStringGetReqUnicodeBufferCountCP932<char16_t>(str, len) : 0;
    }

    template<typename char_t>
    std::size_t INStringGetReqCP932BufferCountUnicode(
        const char_t* str, std::size_t len)
    {
        // Convert CP-932 string to UTF-8, UTF-16, or UTF-32 string
        std::size_t reqBufLen = (len) ? 0 : 1;
        bool breakOnNull = (!len);

        while ((breakOnNull && *str) || len)
        {
            if (len) --len;

            // Get current unicode code point
            char32_t c;
            if constexpr (sizeof(char_t) == sizeof(char))
            {
                // UTF-8
                c = INStringDecodeCodepointUTF8(
                    reinterpret_cast<const char*&>(str));
            }
            else if constexpr (sizeof(char_t) == sizeof(char16_t))
            {
                // UTF-16
                c = INStringDecodeCodepointUTF16(
                    reinterpret_cast<const char16_t*&>(str));
            }
            else if constexpr (sizeof(char_t) == sizeof(char32_t))
            {
                // UTF-32
                c = static_cast<char32_t>(*str++);
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
                c = INUnicodeToCP9321[c - 0xA7];
            }
            else if (c <= 0x451 && c >= 0x391)
            {
                c = INUnicodeToCP9322[c - 0x391];
            }
            else if (c <= 0x266F && c >= 0x2010)
            {
                c = INUnicodeToCP9323[c - 0x2010];
            }
            else if (c <= 0x33CD && c >= 0x3000)
            {
                c = INUnicodeToCP9324[c - 0x3000];
            }
            else if (c <= 0x9FA0 && c >= 0x4E00)
            {
                c = INUnicodeToCP9325[c - 0x4E00];
            }
            else if (c <= 0xFA2D && c >= 0xF929)
            {
                c = INUnicodeToCP9326[c - 0xF929];
            }
            else if (c <= 0xFFE5 && c >= 0xFF01)
            {
                c = INUnicodeToCP9327[c - 0xFF01];
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

    std::size_t StringGetReqCP932BufferCountUTF8(const char* str, std::size_t len)
    {
        if (!str) throw std::invalid_argument("str was null");
        return INStringGetReqCP932BufferCountUnicode(str, len);
    }

    std::size_t StringGetReqCP932BufferCountUTF16(const char16_t* str, std::size_t len)
    {
        if (!str) throw std::invalid_argument("str was null");
        return INStringGetReqCP932BufferCountUnicode(str, len);
    }

    void INStringConvertUTF8ToUTF16NoAlloc(const char* u8str,
        char16_t* u16str, std::size_t u16bufLen, std::size_t u8bufLen)
    {
#ifdef _WIN32
        // Convert the UTF-8 string to UTF-16 and store it in the buffer
        MultiByteToWideChar(CP_UTF8, 0, u8str,
            (u8bufLen == 0) ? -1 : static_cast<int>(u8bufLen),
            reinterpret_cast<wchar_t*>(u16str),
            static_cast<int>(u16bufLen)); // TODO: Error checking?
#else
        // TODO: Support for non-Windows platforms
        throw std::logic_error(
            "This function has not yet been implemented on non-Windows platforms.");
#endif
    }

    void INStringConvertUTF8ToUTF16NoAlloc(const char* u8str,
        char16_t* u16str, std::size_t u8bufLen)
    {
        // Determine the size of the buffer required to hold the string's UTF-16 equivalent
        std::size_t u16bufLen = INStringGetReqUTF16BufferCountUTF8(
            u8str, u8bufLen);

        // Convert to UTF-16
        INStringConvertUTF8ToUTF16NoAlloc(u8str,
            u16str, u16bufLen, u8bufLen);
    }

    std::unique_ptr<char16_t[]> INStringConvertUTF8ToUTF16Ptr(const char* u8str,
        std::size_t u16bufLen, std::size_t u8bufLen)
    {
        // Allocate a buffer big enough to hold the UTF-16 string
        std::unique_ptr<char16_t[]> u16str = std::unique_ptr<char16_t[]>(
            new char16_t[u16bufLen]);

        // Convert to UTF-16
        INStringConvertUTF8ToUTF16NoAlloc(u8str,
            u16str.get(), u16bufLen, u8bufLen);

        return u16str;
    }

    std::unique_ptr<char16_t[]> INStringConvertUTF8ToUTF16Ptr(
        const char* u8str, std::size_t u8bufLen)
    {
        // Determine the size of the buffer required to hold the string's UTF-16 equivalent
        std::size_t u16bufLen = INStringGetReqUTF16BufferCountUTF8(
            u8str, u8bufLen);

        // Convert to UTF-16
        return INStringConvertUTF8ToUTF16Ptr(u8str, u16bufLen, u8bufLen);
    }

    std::unique_ptr<char16_t[]> StringConvertUTF8ToUTF16Ptr(
        const char* u8str, std::size_t u8bufLen)
    {
        if (!u8str) throw std::invalid_argument("u8str was null");
        return INStringConvertUTF8ToUTF16Ptr(u8str, u8bufLen);
    }

    std::u16string StringConvertUTF8ToUTF16(const char* u8str, std::size_t u8bufLen)
    {
        if (!u8str) throw std::invalid_argument("u8str was null");

        // Determine the size of the buffer required to hold the string's UTF-16 equivalent
        std::size_t u16bufLen = INStringGetReqUTF16BufferCountUTF8(
            u8str, u8bufLen);

        // Allocate a UTF-16 string
        std::u16string u16str;
        u16str.resize(u16bufLen);

        // Convert to UTF-16
        INStringConvertUTF8ToUTF16NoAlloc(u8str,
            u16str.data(), u16bufLen, u8bufLen);

        return u16str;
    }

    template<typename char_t>
    void INStringConvertUnicodeToCP932NoAlloc(
        const char_t* ustr, char* cp932str, std::size_t ubufLen)
    {
        // Convert CP-932 string to UTF-8, UTF-16, or UTF-32 string
        bool breakOnNull = (!ubufLen);
        while ((breakOnNull && *ustr) || ubufLen)
        {
            if (ubufLen) --ubufLen;

            // Get current unicode code point
            char32_t c;
            if constexpr (sizeof(char_t) == sizeof(char))
            {
                // UTF-8
                c = INStringDecodeCodepointUTF8(
                    reinterpret_cast<const char*&>(ustr));
            }
            else if constexpr (sizeof(char_t) == sizeof(char16_t))
            {
                // UTF-16
                c = INStringDecodeCodepointUTF16(
                    reinterpret_cast<const char16_t*&>(ustr));
            }
            else if constexpr (sizeof(char_t) == sizeof(char32_t))
            {
                // UTF-32
                c = static_cast<char32_t>(*ustr++);
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
                    INUnicodeToJIS0201[c - 0xFF61]);
                continue;
            }

            // Multi-byte characters
            else if (c <= 0xF7 && c >= 0xA7)
            {
                c = INUnicodeToCP9321[c - 0xA7];
            }
            else if (c <= 0x451 && c >= 0x391)
            {
                c = INUnicodeToCP9322[c - 0x391];
            }
            else if (c <= 0x266F && c >= 0x2010)
            {
                c = INUnicodeToCP9323[c - 0x2010];
            }
            else if (c <= 0x33CD && c >= 0x3000)
            {
                c = INUnicodeToCP9324[c - 0x3000];
            }
            else if (c <= 0x9FA0 && c >= 0x4E00)
            {
                c = INUnicodeToCP9325[c - 0x4E00];
            }
            else if (c <= 0xFA2D && c >= 0xF929)
            {
                c = INUnicodeToCP9326[c - 0xF929];
            }
            else if (c <= 0xFFE5 && c >= 0xFF01)
            {
                c = INUnicodeToCP9327[c - 0xFF01];
            }

            // Invalid characters
            else
            {
                // Use a standard ASCII ? instead
                *cp932str++ = '?';
                continue;
            }

            // Store CP-932 codepoint as two-byte character
            *reinterpret_cast<char16_t*>(cp932str) = static_cast<char16_t>(c);
            cp932str += 2;
        }

        // Copy the null terminator too if necessary
        if (breakOnNull) *cp932str = '\0';
    }

    template<typename char_t>
    std::unique_ptr<char[]> INStringConvertUnicodeToCP932Ptr(const char_t* ustr,
        std::size_t cp932bufLen, std::size_t ubufLen)
    {
        // Allocate a buffer big enough to hold the CP-932 string
        std::unique_ptr<char[]> cp932str = std::unique_ptr<char[]>(
            new char[cp932bufLen]);

        // Convert to CP-932
        INStringConvertUnicodeToCP932NoAlloc(
            ustr, cp932str.get(), ubufLen);

        return cp932str;
    }

    template<typename char_t>
    std::unique_ptr<char[]> INStringConvertUnicodeToCP932Ptr(
        const char_t* ustr, std::size_t ubufLen)
    {
        // Determine the size of the buffer required to hold the string's CP-932 equivalent
        std::size_t cp932bufLen = INStringGetReqCP932BufferCountUnicode(
            ustr, ubufLen);

        // Convert to CP-932
        return INStringConvertUnicodeToCP932Ptr(
            ustr, cp932bufLen, ubufLen);
    }

    template<typename char_t>
    std::string INStringConvertUnicodeToCP932(const char_t* ustr, std::size_t ubufLen)
    {
        // Determine the size of the buffer required to hold the string's CP-932 equivalent
        std::size_t cp932bufLen = INStringGetReqCP932BufferCountUnicode(
            ustr, ubufLen);

        // Create a string big enough to hold the CP-932 data
        std::string cp932str;
        cp932str.resize(cp932bufLen);

        // Convert to CP-932
        INStringConvertUnicodeToCP932NoAlloc(ustr,
            cp932str.data(), ubufLen);

        return cp932str;
    }

    std::unique_ptr<char[]> StringConvertUTF8ToCP932Ptr(
        const char* u8str, std::size_t u8bufLen)
    {
        if (!u8str) throw std::invalid_argument("u8str was null");
        return INStringConvertUnicodeToCP932Ptr(u8str, u8bufLen);
    }

    std::string StringConvertUTF8ToCP932(const char* u8str, std::size_t u8bufLen)
    {
        if (!u8str) throw std::invalid_argument("u8str was null");
        return INStringConvertUnicodeToCP932(u8str, u8bufLen);
    }

    std::unique_ptr<char[]> StringConvertUTF16ToCP932Ptr(
        const char16_t* u16str, std::size_t u16bufLen)
    {
        if (!u16str) throw std::invalid_argument("u16str was null");
        return INStringConvertUnicodeToCP932Ptr(u16str, u16bufLen);
    }

    std::string StringConvertUTF16ToCP932(
        const char16_t* u16str, std::size_t u16bufLen)
    {
        if (!u16str) throw std::invalid_argument("u16str was null");
        return INStringConvertUnicodeToCP932(u16str, u16bufLen);
    }

    std::unique_ptr<nchar[]> INStringConvertUTF8ToNativePtr(
        const char* u8str, std::size_t u8bufLen)
    {
#ifdef _WIN32
        // Determine the size of the buffer required to hold the string's UTF-16 equivalent
        std::size_t u16bufLen = INStringGetReqUTF16BufferCountUTF8(
            u8str, u8bufLen);

        // Allocate a buffer big enough to hold the UTF-16 string
        std::unique_ptr<nchar[]> nstr = std::unique_ptr<nchar[]>(
            new nchar[u16bufLen]);

        // Convert to UTF-16
        INStringConvertUTF8ToUTF16NoAlloc(u8str,
            reinterpret_cast<char16_t*>(nstr.get()),
            u16bufLen, u8bufLen);

        return nstr;
#else
        // Allocate a buffer big enough to hold a copy of the string
        if (u8bufLen == 0) u8bufLen = (std::strlen(u8str) + 1);
        std::unique_ptr<nchar[]> nstr = std::unique_ptr<nchar[]>(
            new nchar[u8bufLen]);

        // Copy the string and return
        std::copy(u8str, u8str + u8bufLen, nstr.get());
        return nstr;
#endif
    }

    std::unique_ptr<nchar[]> StringConvertUTF8ToNativePtr(
        const char* u8str, std::size_t u8bufLen)
    {
        if (!u8str) throw std::invalid_argument("u8str was null");
        return INStringConvertUTF8ToNativePtr(u8str, u8bufLen);
    }

    nstring StringConvertUTF8ToNative(
        const char* u8str, std::size_t u8bufLen)
    {
        if (!u8str) throw std::invalid_argument("u8str was null");
#ifdef _WIN32
        // Determine the size of the buffer required to hold the string's UTF-16 equivalent
        std::size_t u16bufLen = INStringGetReqUTF16BufferCountUTF8(
            u8str, u8bufLen);

        // Allocate a UTF-16 string
        nstring nstr;
        nstr.resize(u16bufLen);

        // Convert to UTF-16
        INStringConvertUTF8ToUTF16NoAlloc(u8str,
            reinterpret_cast<char16_t*>(nstr.data()),
            u16bufLen, u8bufLen);

        return nstr;
#else
        // Return a copy of the UTF-8 string
        return nstring(u8str);
#endif
    }

    void INStringConvertUTF16ToUTF8NoAlloc(const char16_t* u16str,
        char* u8str, std::size_t u8bufLen, std::size_t u16bufLen)
    {
#ifdef _WIN32
        // Convert the UTF-16 string to UTF-8 and store it in the buffer
        WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<const wchar_t*>(u16str),
            (u16bufLen == 0) ? -1 : static_cast<int>(u16bufLen),
            u8str, static_cast<int>(u8bufLen), NULL, NULL); // TODO: Error checking?
#else
        // TODO: Support for non-Windows platforms
        throw std::logic_error(
            "This function has not yet been implemented on non-Windows platforms.");
#endif
    }

    void INStringConvertUTF16ToUTF8NoAlloc(const char16_t* u16str,
        char* u8str, std::size_t u16bufLen)
    {
        // Determine the size of the buffer required to hold the string's UTF-8 equivalent
        std::size_t u8bufLen = INStringGetReqUTF8BufferCountUTF16(
            u16str, u16bufLen);

        // Convert to UTF-8
        INStringConvertUTF16ToUTF8NoAlloc(u16str,
            u8str, u8bufLen, u16bufLen);
    }

    std::unique_ptr<char[]> INStringConvertUTF16ToUTF8Ptr(const char16_t* u16str,
        std::size_t u8bufLen, std::size_t u16bufLen)
    {
        // Allocate a buffer big enough to hold the UTF-8 string
        std::unique_ptr<char[]> u8str = std::unique_ptr<char[]>(
            new char[u8bufLen]);

        // Convert to UTF-8
        INStringConvertUTF16ToUTF8NoAlloc(
            u16str, u8str.get(), u8bufLen, u16bufLen);

        return u8str;
    }

    std::unique_ptr<char[]> INStringConvertUTF16ToUTF8Ptr(
        const char16_t* u16str, std::size_t u16bufLen)
    {
        // Determine the size of the buffer required to hold the string's UTF-8 equivalent
        std::size_t u8bufLen = INStringGetReqUTF8BufferCountUTF16(
            u16str, u16bufLen);

        // Convert to UTF-8
        return INStringConvertUTF16ToUTF8Ptr(
            u16str, u8bufLen, u16bufLen);
    }

    std::unique_ptr<char[]> StringConvertUTF16ToUTF8Ptr(
        const char16_t* u16str, std::size_t u16bufLen)
    {
        if (!u16str) throw std::invalid_argument("u16str was null");
        return INStringConvertUTF16ToUTF8Ptr(u16str, u16bufLen);
    }

    std::string StringConvertUTF16ToUTF8(
        const char16_t* u16str, std::size_t u16bufLen)
    {
        if (!u16str) throw std::invalid_argument("u16str was null");

        // Determine the size of the buffer required to hold the string's UTF-8 equivalent
        std::size_t u8bufLen = INStringGetReqUTF8BufferCountUTF16(
            u16str, u16bufLen);

        // Allocate a UTF-8 string
        std::string u8str;
        u8str.resize(u8bufLen);

        // Convert to UTF-8
        INStringConvertUTF16ToUTF8NoAlloc(u16str,
            u8str.data(), u8bufLen, u16bufLen);

        return u8str;
    }

    std::unique_ptr<nchar[]> INStringConvertUTF16ToNativePtr(
        const char16_t* u16str, std::size_t u16bufLen)
    {
#ifdef _WIN32
        // Allocate a buffer big enough to hold a copy of the string
        if (u16bufLen == 0)
        {
            u16bufLen = (std::wcslen(
                reinterpret_cast<const wchar_t*>(u16str)) + 1);
        }

        std::unique_ptr<nchar[]> nstr = std::unique_ptr<nchar[]>(
            new nchar[u16bufLen]);

        // Copy the string and return
        std::copy(u16str, u16str + u16bufLen, nstr.get());
        return nstr;
#else
        return INStringConvertUTF16ToUTF8Ptr(
            u16str, u16bufLen);
#endif
    }

    std::unique_ptr<nchar[]> StringConvertUTF16ToNativePtr(
        const char16_t* u16str, std::size_t u16bufLen)
    {
        if (!u16str) throw std::invalid_argument("u16str was null");
        return INStringConvertUTF16ToNativePtr(u16str, u16bufLen);
    }

    nstring StringConvertUTF16ToNative(const char16_t* u16str, std::size_t u16bufLen)
    {
        if (!u16str) throw std::invalid_argument("u16str was null");
#ifdef _WIN32
        // Return a copy of the UTF-16 string
        return nstring(reinterpret_cast<const wchar_t*>(u16str));
#else
        // Determine the size of the buffer required to hold the string's UTF-8 equivalent
        std::size_t u8bufLen = INStringGetReqUTF8BufferCountUTF16(
            u16str, u16bufLen);

        // Allocate a UTF-8 string
        nstring u8str;
        u8str.resize(u8bufLen);

        // Convert to UTF-8
        INStringConvertUTF16ToUTF8NoAlloc(u16str,
            u8str.data(), u8bufLen, u16bufLen);

        return u8str;
#endif
    }

    template<typename char_t>
    void INStringConvertCP932ToUnicodeNoAlloc(
        const char* cp932str, char_t* ustr, std::size_t cp932bufLen)
    {
        // Get CP-932 buffer length if necessary
        // TODO: Optimize-out this strlen using a while loop like in INStringConvertUnicodeToCP932NoAlloc
        if (!cp932bufLen) cp932bufLen = (std::strlen(cp932str) + 1);

        // Convert CP-932 string to UTF-8, UTF-16, or UTF-32 string
        for (std::size_t i = 0; i < cp932bufLen; ++i)
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
                char16_t c;
                if (cp932str[i] < -32 && cp932str[i] > -96)
                {
                    c = INJIS0201ToUnicode[(
                        static_cast<unsigned char>(cp932str[i]) - 0xA1)];
                }

                // Other characters
                else
                {

                    // Get two-byte CP-932 character
                    c = *reinterpret_cast<const char16_t*>(cp932str + i);
                    Swap(c); // CP-932 strings are always big endian

                    // Find matching Unicode codepoint to use, if any
                    if (c <= 0x84BE && c >= 0x8140)
                    {
                        c = INCP932ToUnicode1[c - 0x8140];
                    }
                    else if (c <= 0x9FFC && c >= 0x8740)
                    {
                        c = INCP932ToUnicode2[c - 0x8740];
                    }
                    else if (c <= 0xEAA4 && c >= 0xE040)
                    {
                        c = INCP932ToUnicode3[c - 0xE040];
                    }
                    else if (c <= 0xEEFC && c >= 0xED40)
                    {
                        c = INCP932ToUnicode4[c - 0xED40];
                    }
                    else if (c <= 0xFC4B && c >= 0xFA40)
                    {
                        c = INCP932ToUnicode5[c - 0xFA40];
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
                    INStringEncodeCodepointUTF8(
                        reinterpret_cast<char*&>(ustr), c);
                }
                else if constexpr (sizeof(char_t) == sizeof(char16_t))
                {
                    // UTF-16
                    INStringEncodeCodepointUTF16(
                        reinterpret_cast<char16_t*&>(ustr), c);
                }
                else if constexpr (sizeof(char_t) == sizeof(char32_t))
                {
                    // UTF-32
                    *ustr++ = static_cast<char_t>(c);
                }
            }
        }
    }

    std::unique_ptr<char[]> INStringConvertCP932ToUTF8Ptr(const char* cp932str,
        std::size_t u8bufLen, std::size_t cp932bufLen)
    {
        // Allocate a buffer big enough to hold the UTF-8 string
        std::unique_ptr<char[]> u8str = std::unique_ptr<char[]>(
            new char[u8bufLen]);

        // Convert to UTF-8
        INStringConvertCP932ToUnicodeNoAlloc(
            cp932str, u8str.get(), cp932bufLen);

        return u8str;
    }

    std::unique_ptr<char[]> INStringConvertCP932ToUTF8Ptr(
        const char* cp932str, std::size_t cp932bufLen)
    {
        // Determine the size of the buffer required to hold the string's UTF-8 equivalent
        std::size_t u8bufLen = INStringGetReqUnicodeBufferCountCP932<char>(
            cp932str, cp932bufLen);

        // Convert to UTF-8
        return INStringConvertCP932ToUTF8Ptr(
            cp932str, u8bufLen, cp932bufLen);
    }

    std::unique_ptr<char[]> StringConvertCP932ToUTF8Ptr(
        const char* cp932str, std::size_t cp932bufLen)
    {
        if (!cp932str) throw std::invalid_argument("cp932str was null");
        return INStringConvertCP932ToUTF8Ptr(cp932str, cp932bufLen);
    }

    std::string StringConvertCP932ToUTF8(
        const char* cp932str, std::size_t cp932bufLen)
    {
        if (!cp932str) throw std::invalid_argument("cp932str was null");

        // Determine the size of the buffer required to hold the string's UTF-8 equivalent
        std::size_t u8bufLen = INStringGetReqUnicodeBufferCountCP932<char>(
            cp932str, cp932bufLen);

        // Create a UTF-8 string
        std::string u8str;
        u8str.resize(u8bufLen);

        // Convert to UTF-8
        INStringConvertCP932ToUnicodeNoAlloc(
            cp932str, u8str.data(), cp932bufLen);

        return u8str;
    }

    std::unique_ptr<char16_t[]> INStringConvertCP932ToUTF16Ptr(const char* cp932str,
        std::size_t u16bufLen, std::size_t cp932bufLen)
    {
        // Allocate a buffer big enough to hold the UTF-16 string
        std::unique_ptr<char16_t[]> u16str = std::unique_ptr<char16_t[]>(
            new char16_t[u16bufLen]);

        // Convert to UTF-16
        INStringConvertCP932ToUnicodeNoAlloc(
            cp932str, u16str.get(), cp932bufLen);

        return u16str;
    }

    std::unique_ptr<char16_t[]> INStringConvertCP932ToUTF16Ptr(
        const char* cp932str, std::size_t cp932bufLen)
    {
        // Determine the size of the buffer required to hold the string's UTF-16 equivalent
        std::size_t u16bufLen = INStringGetReqUnicodeBufferCountCP932<char16_t>(
            cp932str, cp932bufLen);

        // Convert to UTF-16
        return INStringConvertCP932ToUTF16Ptr(
            cp932str, u16bufLen, cp932bufLen);
    }

    std::unique_ptr<char16_t[]> StringConvertCP932ToUTF16Ptr(
        const char* cp932str, std::size_t cp932bufLen)
    {
        if (!cp932str) throw std::invalid_argument("cp932str was null");
        return INStringConvertCP932ToUTF16Ptr(cp932str, cp932bufLen);
    }

    std::u16string StringConvertCP932ToUTF16(
        const char* cp932str, std::size_t cp932bufLen)
    {
        if (!cp932str) throw std::invalid_argument("cp932str was null");
        
        // Determine the size of the buffer required to hold the string's UTF-16 equivalent
        std::size_t u16bufLen = INStringGetReqUnicodeBufferCountCP932<char16_t>(
            cp932str, cp932bufLen);

        // Allocate a buffer big enough to hold the UTF-16 string
        std::u16string u16str;
        u16str.resize(u16bufLen);

        // Convert to UTF-16
        INStringConvertCP932ToUnicodeNoAlloc(
            cp932str, u16str.data(), cp932bufLen);

        return u16str;
    }

    std::unique_ptr<nchar[]> INStringConvertCP932ToNativePtr(
        const char* cp932str, std::size_t cp932bufLen)
    {
#ifdef _WIN32
        // Determine the size of the buffer required to hold the string's UTF-16 equivalent
        std::size_t u16bufLen = INStringGetReqUnicodeBufferCountCP932<char16_t>(
            cp932str, cp932bufLen);

        // Allocate a buffer big enough to hold the UTF-16 string
        std::unique_ptr<nchar[]> u16str = std::unique_ptr<nchar[]>(
            new nchar[u16bufLen]);

        // Convert to UTF-16
        INStringConvertCP932ToUnicodeNoAlloc(cp932str,
            reinterpret_cast<char16_t*>(u16str.get()), cp932bufLen);

        return u16str;
#else
        return INStringConvertCP932ToUTF8Ptr(
            cp932str, cp932bufLen);
#endif
    }

    std::unique_ptr<nchar[]> StringConvertCP932ToNativePtr(
        const char* cp932str, std::size_t cp932bufLen)
    {
        if (!cp932str) throw std::invalid_argument("cp932str was null");
        return INStringConvertCP932ToNativePtr(cp932str, cp932bufLen);
    }

    template<typename char_t>
    nstring INStringConvertCP932ToNative(
        const char* cp932str, std::size_t cp932bufLen)
    {
        // Determine the size of the buffer required to hold the string's native equivalent
        std::size_t nbufLen = INStringGetReqUnicodeBufferCountCP932<char_t>(
            cp932str, cp932bufLen);

        // Create a native string
        nstring nstr;
        nstr.resize(nbufLen);

        // Convert to native
        INStringConvertCP932ToUnicodeNoAlloc(cp932str,
            reinterpret_cast<char_t*>(nstr.data()), cp932bufLen);

        return nstr;
    }

    nstring StringConvertCP932ToNative(
        const char* cp932str, std::size_t cp932bufLen)
    {
        if (!cp932str) throw std::invalid_argument("cp932str was null");
#ifdef _WIN32
        return INStringConvertCP932ToNative<char16_t>(cp932str, cp932bufLen);
#else
        return INStringConvertCP932ToNative<char>(cp932str, cp932bufLen);
#endif
    }

    template<typename char_t>
    std::unique_ptr<char_t[]> INStringJoinPtr(const char_t* str1, const char_t* str2)
    {
        // Get lengths of strings
        std::size_t strLen1 = StringLength(str1);
        std::size_t strLen2 = (StringLength(str2) + 1);

        // Allocate buffer big enough to hold both strings combined
        std::unique_ptr<char_t[]> result = std::unique_ptr<char_t[]>(
            new char_t[(strLen1 + strLen2)]);

        // Copy both strings into new buffer and return
        std::copy(str1, str1 + strLen1, result.get());
        std::copy(str2, str2 + strLen2, result.get() + strLen1);
        return result;
    }

    template<typename str_t, typename char_t>
    str_t INStringJoin(const char_t* str1, const char_t* str2)
    {
        // Get lengths of strings
        std::size_t strLen1 = StringLength(str1);
        std::size_t strLen2 = (StringLength(str2) + 1);

        // Create string big enough to hold both strings combined
        str_t result;
        result.resize(strLen1 + strLen2);

        // Copy both strings into new string and return
        std::copy(str1, str1 + strLen1, result.data());
        std::copy(str2, str2 + strLen2, result.data() + strLen1);
        return result;
    }

    std::unique_ptr<char[]> StringJoinPtr(const char* str1, const char* str2)
    {
        if (!str1 || !str2) throw std::invalid_argument("str1 and/or str2 were null");
        return INStringJoinPtr(str1, str2);
    }

    std::string StringJoin(const char* str1, const char* str2)
    {
        if (!str1 || !str2) throw std::invalid_argument("str1 and/or str2 were null");
        return INStringJoin<std::string>(str1, str2);
    }

#ifdef _WIN32
    std::unique_ptr<nchar[]> StringJoinPtr(const nchar* str1, const nchar* str2)
    {
        if (!str1 || !str2) throw std::invalid_argument("str1 and/or str2 were null");
        return INStringJoinPtr(str1, str2);
    }

    nstring StringJoin(const nchar* str1, const nchar* str2)
    {
        if (!str1 || !str2) throw std::invalid_argument("str1 and/or str2 were null");
        return INStringJoin<nstring>(str1, str2);
    }
#endif
}
