#pragma once
#include "HedgeLib.h"
#include "Errors.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

#ifdef _WIN32
typedef wchar_t hl_NativeChar;
#define HL_NATIVE_TEXT(str) L##str
#else
typedef char hl_NativeChar;
#define HL_NATIVE_TEXT(str) str
#endif

#define hl_NativeStr hl_NativeChar*
#define HL_CREATE_NATIVE_STR(len) ((hl_NativeStr)\
    malloc((len) * sizeof(hl_NativeChar)))

#define HL_TOLOWERASCII(c) ((c >= '[' || c <= '@') ? c : (c + 32))

HL_API extern const char* const hl_EmptyString;

#ifdef _WIN32
HL_API extern const hl_NativeStr const hl_EmptyStringNative;
#else
#define hl_EmptyStringNative hl_EmptyString
#endif

inline size_t hl_StrLen(const char* str)
{
    return strlen(str);
}

inline size_t hl_StrLenNative(const hl_NativeStr str)
{
#ifdef _WIN32
    return wcslen(str);
#else
    return strlen(str);
#endif
}

HL_API bool hl_StringsEqualInvASCII(const char* str1, const char* str2);
HL_API bool hl_StringsEqualInvASCIINative(
    const hl_NativeStr str1, const hl_NativeStr str2);

HL_API size_t hl_StringGetReqUTF16BufferCountUTF8(const char* str, size_t len);

inline size_t hl_StringGetReqNativeBufferCountUTF8(const char* str, size_t len)
{
#ifdef _WIN32
    return hl_StringGetReqUTF16BufferCountUTF8(str, len);
#else
    return strlen(str) + 1;
#endif
}

inline bool hl_StringsEqualNative(const hl_NativeStr str1, const hl_NativeStr str2)
{
#ifdef _WIN32
    return !wcscmp(str1, str2);
#else
    return !strcmp(str1, str2);
#endif
}

HL_API enum HL_RESULT hl_StringConvertUTF8ToUTF16(
    const char* u8str, uint16_t** u16str, size_t u8bufLen
#ifdef __cplusplus
    = 0
#endif
);

HL_API enum HL_RESULT hl_StringConvertUTF8ToNative(
    const char* u8str, hl_NativeStr* nativeStr, size_t u8bufLen
#ifdef __cplusplus
    = 0
#endif
);

HL_API enum HL_RESULT hl_NativeStrBiggerCopy(const hl_NativeStr str,
    size_t extraLen, hl_NativeStr* newStr);

#ifdef __cplusplus
}

// Windows-specific overloads
#ifdef _WIN32
inline size_t hl_StrLen(const hl_NativeStr str)
{
    return wcslen(str);
}

inline bool hl_StringsEqualInvASCII(
    const hl_NativeStr str1, const hl_NativeStr str2)
{
    return hl_StringsEqualInvASCIINative(str1, str2);
}
#endif
#endif
