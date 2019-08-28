#pragma once
#include "HedgeLib/String.h"

#ifdef _WIN32
// Converts the given path to UTF-16, calls a function, frees, and returns the HL_RESULT
#define HL_INSTRING_NATIVE_CALL(str, funcCall) {\
hl_NativeStr nativeStr;\
HL_RESULT err = hl_INStringConvertUTF8ToNative(str, &nativeStr);\
if (HL_FAILED(err)) return err;\
err = funcCall;\
free(nativeStr);\
return err;\
}
#else
// Simply calls the function and returns the HL_RESULT
#define HL_INSTRING_NATIVE_CALL(str, funcCall) \
const hl_NativeStr nativeStr = str; return funcCall;
#endif

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

HL_RESULT hl_INStringConvertUTF8ToUTF16NoAlloc(const char* u8str,
    uint16_t* u16str, size_t u16bufLen, size_t u8bufLen);

HL_RESULT hl_INStringConvertUTF8ToUTF16NoAlloc(const char* u8str,
    uint16_t* u16str, size_t u8bufLen);

HL_RESULT hl_INStringConvertUTF8ToUTF16(const char* u8str,
    uint16_t** u16str, size_t u16bufLen, size_t u8bufLen);

HL_RESULT hl_INStringConvertUTF8ToUTF16(const char* u8str,
    uint16_t** u16str, size_t u8bufLen);

HL_RESULT hl_INStringConvertUTF8ToNative(const char* u8str,
    hl_NativeStr* nativeStr, size_t u8bufLen = 0);
