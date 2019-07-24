#include "HedgeLib/String.h"
#include "INString.h"
#include <cstdlib>

#ifdef _WIN32
#include <Windows.h>
#endif

const char* const hl_EmptyString = "";

bool hl_StringsEqualASCII(const char* str1, const char* str2)
{
    if (str1 == str2) return true;
    if (!str1 || !*str1) return false;

    while (*str1 || *str2)
    {
        if (*str1 != *str2 && HL_TOLOWERASCII(*str1) !=
            HL_TOLOWERASCII(*str2))
        {
            return false;
        }

        ++str1;
        ++str2;
    }
    
    return true;
}

size_t hl_INStringGetReqUTF16CharCountUTF8(const char* str)
{
#ifdef _WIN32
    // Figure out the amount of characters in the UTF-8 string
    int strLen = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
    return static_cast<size_t>(strLen);
#endif

    // TODO: Support for non-Windows platforms
    return 0;
}

HL_RESULT hl_INStringConvertUTF8ToUTF16NoAlloc(
    const char* u8str, uint16_t* u16str, size_t strLen)
{
#ifdef _WIN32
    // Convert the UTF-8 string to UTF-16 and store it in the buffer
    MultiByteToWideChar(CP_UTF8, 0, u8str, -1,
        reinterpret_cast<wchar_t*>(u16str),
        static_cast<int>(strLen)); // TODO: Error checking?
#endif

    // TODO: Support for non-Windows platforms

    return HL_SUCCESS;
}

HL_RESULT hl_INStringConvertUTF8ToUTF16NoAlloc(
    const char* u8str, uint16_t* u16str)
{
#ifdef _WIN32
    // Figure out the amount of characters in the UTF-8 string
    size_t strLen = hl_INStringGetReqUTF16CharCountUTF8(u8str);

    // Convert to UTF-16
    return hl_INStringConvertUTF8ToUTF16NoAlloc(u8str, u16str, strLen);
#endif

    // TODO: Support for non-Windows platforms

    return HL_SUCCESS;
}

HL_RESULT hl_INStringConvertUTF8ToUTF16(
    const char* u8str, uint16_t** u16str, size_t strLen)
{
#ifdef _WIN32
    // Allocate a buffer big enough to hold the UTF-16 string
    *u16str = static_cast<uint16_t*>(std::malloc(
        strLen * sizeof(uint16_t)));

    if (!*u16str) return HL_ERROR_OUT_OF_MEMORY;

    // Convert to UTF-16
    return hl_INStringConvertUTF8ToUTF16NoAlloc(
        u8str, *u16str, strLen);
#endif

    // TODO: Support for non-Windows platforms

    return HL_SUCCESS;
}

HL_RESULT hl_INStringConvertUTF8ToUTF16(const char* u8str, uint16_t** u16str)
{
#ifdef _WIN32
    // Figure out the amount of characters in the UTF-8 string
    size_t strLen = hl_INStringGetReqUTF16CharCountUTF8(u8str);

    // Convert to UTF-16
    return hl_INStringConvertUTF8ToUTF16(u8str, u16str, strLen);
#endif
    
    // TODO: Support for non-Windows platforms

    return HL_SUCCESS;
}

enum HL_RESULT hl_StringConvertUTF8ToUTF16(const char* u8str, uint16_t** u16str)
{
    if (!u8str || !u16str) return HL_ERROR_UNKNOWN;
    return hl_INStringConvertUTF8ToUTF16(u8str, u16str);
}
