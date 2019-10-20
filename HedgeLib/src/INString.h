#pragma once
#include "HedgeLib/String.h"

#ifdef _WIN32
// Converts the given path to UTF-16, calls a function, frees, and returns the HL_RESULT
#define HL_INSTRING_NATIVE_CALL(str, funcCall) {\
hl_NativeChar* nativeStr;\
HL_RESULT err = hl_INStringConvertUTF8ToNative(str, &nativeStr);\
if (HL_FAILED(err)) return err;\
err = funcCall;\
free(nativeStr);\
return err;\
}
#else
// Simply calls the function and returns the HL_RESULT
#define HL_INSTRING_NATIVE_CALL(str, funcCall) \
const hl_NativeChar* nativeStr = str; return funcCall;
#endif

void hl_INStringEncodeCodepointUTF8(uint32_t cp, char*& u8str);
void hl_INStringEncodeCodepointUTF16(uint32_t cp, uint16_t*& u16str);
uint32_t hl_INStringDecodeCodepointUTF8(const char*& u8str);
uint32_t hl_INStringDecodeCodepointUTF16(const uint16_t*& u16str);

template<typename str1_t, typename str2_t>
bool hl_INStringsEqualInvASCII(const str1_t* str1, const str2_t* str2);

size_t hl_INStringGetReqUTF16BufferCountUTF8(const char* str, size_t len = 0);

inline size_t hl_INStringGetReqNativeBufferCountUTF8(const char* str, size_t len = 0)
{
#ifdef _WIN32
    return hl_INStringGetReqUTF16BufferCountUTF8(str, len);
#else
    return strlen(str) + 1;
#endif
}

size_t hl_INStringGetReqUTF8BufferCountUTF16(const uint16_t* str, size_t len = 0);

template<typename char_t>
size_t hl_INStringGetReqUnicodeBufferCountCP932(
    const char* str, size_t len);

template<typename char_t>
size_t hl_INStringGetReqCP932BufferCountUnicode(
    const char_t* str, size_t len);

HL_RESULT hl_INStringConvertUTF8ToUTF16NoAlloc(const char* u8str,
    uint16_t* u16str, size_t u16bufLen, size_t u8bufLen);

HL_RESULT hl_INStringConvertUTF8ToUTF16NoAlloc(const char* u8str,
    uint16_t* u16str, size_t u8bufLen);

HL_RESULT hl_INStringConvertUTF8ToUTF16(const char* u8str,
    uint16_t** u16str, size_t u16bufLen, size_t u8bufLen);

HL_RESULT hl_INStringConvertUTF8ToUTF16(const char* u8str,
    uint16_t** u16str, size_t u8bufLen);

template<typename char_t>
HL_RESULT hl_INStringConvertUnicodeToCP932NoAlloc(
    const char_t* ustr, char* cp932str, size_t ubufLen);

HL_RESULT hl_INStringConvertUTF8ToCP932(const char* u8str,
    char** cp932str, size_t cp932bufLen, size_t u8bufLen);

HL_RESULT hl_INStringConvertUTF8ToCP932(const char* u8str,
    char** cp932str, size_t u8bufLen);

HL_RESULT hl_INStringConvertUTF16ToCP932(const uint16_t* u16str,
    char** cp932str, size_t cp932bufLen, size_t u16bufLen);

HL_RESULT hl_INStringConvertUTF16ToCP932(const uint16_t* u16str,
    char** cp932str, size_t u16bufLen);

HL_RESULT hl_INStringConvertUTF8ToNative(const char* u8str,
    hl_NativeChar** nativeStr, size_t u8bufLen = 0);

HL_RESULT hl_INStringConvertUTF16ToUTF8NoAlloc(const uint16_t* u16str,
    char* u8str, size_t u8bufLen, size_t u16bufLen);

HL_RESULT hl_INStringConvertUTF16ToUTF8NoAlloc(
    const uint16_t* u16str, char* u8str, size_t u16bufLen);

HL_RESULT hl_INStringConvertUTF16ToUTF8(const uint16_t* u16str,
    char** u8str, size_t u8bufLen, size_t u16bufLen);

HL_RESULT hl_INStringConvertUTF16ToUTF8(const uint16_t* u16str,
    char** u8str, size_t u16bufLen);

HL_RESULT hl_INStringConvertUTF16ToNative(const uint16_t* u16str,
    hl_NativeChar** nativeStr, size_t u16bufLen = 0);

template<typename char_t>
HL_RESULT hl_INStringConvertCP932ToUnicodeNoAlloc(
    const char* cp932str, char_t* ustr, size_t cp932bufLen);

HL_RESULT hl_INStringConvertCP932ToUTF8(const char* cp932str,
    char** u8str, size_t u8bufLen, size_t cp932bufLen);

HL_RESULT hl_INStringConvertCP932ToUTF8(const char* cp932str,
    char** u8str, size_t cp932bufLen);

HL_RESULT hl_INStringConvertCP932ToUTF16(const char* cp932str,
    uint16_t** u16str, size_t u16bufLen, size_t cp932bufLen);

HL_RESULT hl_INStringConvertCP932ToUTF16(const char* cp932str,
    uint16_t** u16str, size_t cp932bufLen);

HL_RESULT hl_INStringConvertCP932ToNative(
    const char* cp932str, hl_NativeChar** nativeStr, size_t cp932bufLen);
