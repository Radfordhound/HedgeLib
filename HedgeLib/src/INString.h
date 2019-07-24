#pragma once
#include "HedgeLib/Errors.h"
#include <stdint.h>
#include <stddef.h>
#include <cstdlib>

#ifdef _WIN32
typedef wchar_t hl_INNativeChar;
#define HL_INNATIVE_TEXT(str) L##str
#else
typedef char hl_INNativeChar;
#define HL_INNATIVE_TEXT(str) str
#endif

#define hl_INNativeStr hl_INNativeChar*
#define HL_INCREATE_NATIVE_STR(len) static_cast<hl_INNativeStr>(\
    std::malloc(len * sizeof(hl_INNativeChar)))

size_t hl_INStringGetReqUTF16CharCountUTF8(const char* str);

HL_RESULT hl_INStringConvertUTF8ToUTF16NoAlloc(
    const char* u8str, uint16_t* u16str, size_t strLen);

HL_RESULT hl_INStringConvertUTF8ToUTF16NoAlloc(
    const char* u8str, uint16_t* u16str);

HL_RESULT hl_INStringConvertUTF8ToUTF16(
    const char* u8str, uint16_t** u16str, size_t strLen);

HL_RESULT hl_INStringConvertUTF8ToUTF16(
    const char* u8str, uint16_t** u16str);

#ifdef _WIN32
inline HL_RESULT hl_INWin32StringConvertToNative(
    const char* str, hl_INNativeStr* nativeStr)
{
    return hl_INStringConvertUTF8ToUTF16(str,
        reinterpret_cast<uint16_t**>(nativeStr));
}
#endif
