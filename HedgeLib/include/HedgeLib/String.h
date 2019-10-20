#pragma once
#include "HedgeLib.h"
#include "Result.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#ifdef _WIN32
#include <wchar.h>
#endif
#endif

#ifdef _WIN32
typedef wchar_t hl_NativeChar;
#define HL_NATIVE_TEXT(str) L##str
#else
typedef char hl_NativeChar;
#define HL_NATIVE_TEXT(str) str
#endif

#define HL_CREATE_NATIVE_STR(len) ((hl_NativeChar*)\
    malloc((len) * sizeof(hl_NativeChar)))

#define HL_TOLOWERASCII(c) ((c >= '[' || c <= '@') ? c : (c + 32))

HL_API extern const char* const hl_EmptyString;

#ifdef _WIN32
HL_API extern const hl_NativeChar* const hl_EmptyStringNative;
#else
#define hl_EmptyStringNative hl_EmptyString
#endif

HL_API size_t hl_StringEncodeCodepointUTF8(
    char* u8str, uint32_t codepoint);

HL_API size_t hl_StringEncodeCodepointUTF16(
    uint16_t* u16str, uint32_t codepoint);

HL_API size_t hl_StringDecodeCodepointUTF8(
    const char* u8str, uint32_t* codepoint);

HL_API size_t hl_StringDecodeCodepointUTF16(
    const uint16_t* u16str, uint32_t* codepoint);

inline size_t hl_StrLen(const char* str)
{
    return strlen(str);
}

inline size_t hl_StrLenNative(const hl_NativeChar* str)
{
#ifdef _WIN32
    return wcslen(str);
#else
    return strlen(str);
#endif
}

inline char* hl_StringCopy(const char* src, char* dst)
{
    return strcpy(dst, src);
}

inline hl_NativeChar* hl_StringCopyNative(
    const hl_NativeChar* src, hl_NativeChar* dst)
{
#ifdef _WIN32
    return wcscpy(dst, src);
#else
    return strcpy(dst, src);
#endif
}

HL_API bool hl_StringsEqualInvASCII(const char* str1, const char* str2);
HL_API bool hl_StringsEqualInvASCIINative(
    const hl_NativeChar* str1, const hl_NativeChar* str2);

inline size_t hl_StringGetReqUTF8UnitCount(uint32_t cp)
{
    // Return the amount of code units required to store the given
    // codepoint, or 3 if the codepoint is invalid. (Invalid characters
    // are replaced with 0xFFFD which requires 3 units to encode)
    return (cp < 0x80) ? 1 : (cp < 0x800) ? 2 :
        (cp < 0x10000) ? 3 : (cp < 0x110000) ? 4 : 3;
}

inline size_t hl_StringGetReqUTF16UnitCount(uint32_t cp)
{
    // Return 2 if the given codepoint is not in the BMP, otherwise 1.
    // (Invalid characters are replaced with 0xFFFD which is in the BMP)
    return (cp >= 0x10000 && cp < 0x110000) ? 2 : 1;
}

HL_API size_t hl_StringGetReqUTF16BufferCountUTF8(
    const char* str, size_t HL_DEFARG(len, 0));

inline size_t hl_StringGetReqNativeBufferCountUTF8(
    const char* str, size_t HL_DEFARG(len, 0))
{
#ifdef _WIN32
    return hl_StringGetReqUTF16BufferCountUTF8(str, len);
#else
    return (len) ? len : (strlen(str) + 1);
#endif
}

HL_API size_t hl_StringGetReqUTF8BufferCountUTF16(
    const uint16_t* str, size_t HL_DEFARG(len, 0));

HL_API size_t hl_StringGetReqUTF8BufferCountCP932(
    const char* str, size_t HL_DEFARG(len, 0));

HL_API size_t hl_StringGetReqUTF16BufferCountCP932(
    const char* str, size_t HL_DEFARG(len, 0));

HL_API size_t hl_StringGetReqCP932BufferCountUTF8(
    const char* str, size_t HL_DEFARG(len, 0));

HL_API size_t hl_StringGetReqCP932BufferCountUTF16(
    const uint16_t* str, size_t HL_DEFARG(len, 0));

inline size_t hl_StringGetReqNativeBufferCountUTF16(
    const uint16_t* str, size_t HL_DEFARG(len, 0))
{
#ifdef _WIN32
    return (len) ? (wcslen(reinterpret_cast<const wchar_t*>(str)) + 1) : len;
#else
    return hl_StringGetReqUTF8BufferCountUTF16(str, len);
#endif
}

inline bool hl_StringsEqualNative(const hl_NativeChar* str1, const hl_NativeChar* str2)
{
#ifdef _WIN32
    return !wcscmp(str1, str2);
#else
    return !strcmp(str1, str2);
#endif
}

HL_API enum HL_RESULT hl_StringConvertUTF8ToUTF16(
    const char* u8str, uint16_t** u16str, size_t HL_DEFARG(u8bufLen, 0));

HL_API enum HL_RESULT hl_StringConvertUTF8ToCP932(
    const char* u8str, char** cp932str, size_t HL_DEFARG(u8bufLen, 0));

HL_API enum HL_RESULT hl_StringConvertUTF16ToCP932(
    const uint16_t* u16str, char** cp932str, size_t HL_DEFARG(u16bufLen, 0));

HL_API enum HL_RESULT hl_StringConvertUTF8ToNative(
    const char* u8str, hl_NativeChar** nativeStr, size_t HL_DEFARG(u8bufLen, 0));

HL_API enum HL_RESULT hl_StringConvertUTF16ToUTF8(
    const uint16_t* u16str, char** u8str, size_t HL_DEFARG(u16bufLen, 0));

HL_API enum HL_RESULT hl_StringConvertUTF16ToNative(
    const uint16_t* u16str, hl_NativeChar** nativeStr, size_t HL_DEFARG(u16bufLen, 0));

HL_API enum HL_RESULT hl_StringConvertCP932ToUTF8(
    const char* cp932str, char** u8str, size_t HL_DEFARG(cp932bufLen, 0));

HL_API enum HL_RESULT hl_StringConvertCP932ToUTF16(
    const char* cp932str, uint16_t** u16str, size_t HL_DEFARG(cp932bufLen, 0));

HL_API enum HL_RESULT hl_StringConvertCP932ToNative(
    const char* cp932str, hl_NativeChar** nativeStr, size_t HL_DEFARG(cp932bufLen, 0));

HL_API HL_RESULT hl_StringJoin(const char* str1, const char* str2, char** result);
HL_API HL_RESULT hl_StringJoinNative(const hl_NativeChar* str1,
    const hl_NativeChar* str2, hl_NativeChar** result);

#ifdef __cplusplus
}

#include "Memory.h"

using hl_CStrPtr = hl_CPtr<char>;
using hl_NStrPtr = hl_CPtr<hl_NativeChar>;

// Windows-specific overloads
#ifdef _WIN32
inline size_t hl_StrLen(const hl_NativeChar* str)
{
    return wcslen(str);
}

inline hl_NativeChar* hl_StringCopy(const hl_NativeChar* src, hl_NativeChar* dst)
{
    return wcscpy(dst, src);
}

inline bool hl_StringsEqualInvASCII(
    const hl_NativeChar* str1, const hl_NativeChar* str2)
{
    return hl_StringsEqualInvASCIINative(str1, str2);
}
#endif
#endif
