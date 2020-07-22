#include "hedgelib/hl_text.h"
#include "hl_in_assert.h"
#include <string.h>

#ifdef _MSC_VER
/*
  Temporarily disable annoying Microsoft-specific level 3 warnings.
  "please use wcscpy_s even though it's not in the C++ standard we're begging you"
*/
#pragma warning(disable: 4996)
#endif

#ifdef _WIN32
/* Include windows.h and limits.h if target is Windows */
#include <windows.h>
#include <limits.h>

#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
/* Include unistd.h if target is a POSIX-compliant system */
#include <unistd.h>
#endif

#ifdef HL_IN_WIN32_UNICODE
#include <wchar.h>

HL_STATIC_ASSERT(sizeof(wchar_t) == sizeof(HlChar16),
    "The Windows implementations of HedgeLib's Unicode conversion functions"
    "require sizeof(wchar_t) to be equal to sizeof(HlChar16).");
#endif

size_t hlStrCopyAndLen(const char* HL_RESTRICT src, char* HL_RESTRICT dst)
{
#if defined(_POSIX_VERSION) && _POSIX_VERSION >= 200809L
    /*
       Copy the string from src into dst using POSIX's stpcpy which returns
       a pointer to the null terminator in dst, then use that pointer to
       compute the length of the string and return it.
    */
    return ((size_t)(stpcpy(dst, src) - dst));
#else
    /*
       Copy the string from src into dst.

       (While condition is wrapped in double parenthesis so
       clang/gcc/etc. know that the single = was intentional.)
    */
    const char* ptr = src;
    while ((*dst++ = *ptr++));

    /*
       Get the length of the string we just copied.
       (We subtract 1 so the null terminator isn't factored in.)
    */
    return ((size_t)(ptr - src) - 1);
#endif
}

HlBool hlStrCopyLimit(const char* HL_RESTRICT src,
    char* HL_RESTRICT dst, size_t dstBufLen, size_t* HL_RESTRICT len)
{
    /* Copy up to dstBufLen characters from src into dst. */
    size_t copied = 0;
    while (copied++ < dstBufLen)
    {
        if (!(*dst++ = *src++))
        {
            if (len) *len = copied;
            return 1;
        }
    }

    if (len) *len = copied;
    return 0;
}

size_t hlNStrLen(const HlNChar* str)
{
#ifdef HL_IN_WIN32_UNICODE
    return wcslen(str);
#else
    return strlen(str);
#endif
}

HlNChar* hlNStrCopy(const HlNChar* HL_RESTRICT src, HlNChar* HL_RESTRICT dst)
{
#ifdef HL_IN_WIN32_UNICODE
    return wcscpy(dst, src);
#else
    return strcpy(dst, src);
#endif
}

size_t hlNStrCopyAndLen(const HlNChar* HL_RESTRICT src, HlNChar* HL_RESTRICT dst)
{
#if !defined(HL_IN_WIN32_UNICODE) && (defined(_POSIX_VERSION) && _POSIX_VERSION >= 200809L)
    /* 
       Copy the string from src into dst using POSIX's stpcpy which returns
       a pointer to the null terminator in dst, then use that pointer to
       compute the length of the string and return it.
    */
    return ((size_t)(stpcpy(dst, src) - dst));
#else
    /*
       Copy the string from src into dst.

       (While condition is wrapped in double parenthesis so
       clang/gcc/etc. know that the single = was intentional.)
    */
    const HlNChar* ptr = src;
    while ((*dst++ = *ptr++));

    /*
       Get the length of the string we just copied.
       (We subtract 1 so the null terminator isn't factored in.)
    */
    return ((size_t)(ptr - src) - 1);
#endif
}

HlBool hlNStrCopyLimit(const HlNChar* HL_RESTRICT src,
    HlNChar* HL_RESTRICT dst, size_t dstBufLen, size_t* HL_RESTRICT len)
{
    /* Copy up to dstBufLen characters from src into dst. */
    size_t copied = 0;
    while (copied++ < dstBufLen)
    {
        if (!(*dst++ = *src++))
        {
            if (len) *len = copied;
            return 1;
        }
    }

    if (len) *len = copied;
    return 0;
}

size_t hlStrGetReqLenUTF8ToUTF16(const char* src, size_t srcLen)
{
#ifdef _WIN32
    /* Ensure casting srcLen to a signed integer will be valid. */
    HL_ASSERT((srcLen == 0) || (srcLen <= INT_MAX));

    /*
       Return the minimum required length of a UTF-16 buffer large
       enough to hold the given UTF-8 string once converted to UTF-16.
    */
    return (size_t)MultiByteToWideChar(
        CP_UTF8,                                    /* CodePage */
        0,                                          /* dwFlags */
        (const char*)src,                           /* lpMultiByteStr */
        (srcLen) ? (int)srcLen : -1,                /* cbMultiByte */
        0,                                          /* lpWideCharStr */
        0);                                         /* cchWideChar */
#else
    /* TODO: Support for non-Windows platforms. */
    HL_ASSERT(0);
    return 0; /* So compiler doesn't complain. */
#endif
}

size_t hlStrGetReqLenUTF8ToUTF32(const char* src, size_t srcLen)
{
    /* TODO: Implement this function. */
    HL_ASSERT(0);
    return 0; /* So compiler doesn't complain. */
}

size_t hlStrGetReqLenUTF16ToUTF8(const HlChar16* src, size_t srcLen)
{
#ifdef _WIN32
    /* Ensure casting srcLen to a signed integer will be valid. */
    HL_ASSERT((srcLen == 0) || (srcLen <= INT_MAX));

    /*
       Return the minimum required length of a UTF-8 buffer large
       enough to hold the given UTF-16 string once converted to UTF-8.
    */
    return (size_t)WideCharToMultiByte(
        CP_UTF8,                                    /* CodePage */
        0,                                          /* dwFlags */
        (const wchar_t*)src,                        /* lpWideCharStr */
        (srcLen) ? (int)srcLen : -1,                /* cchWideChar */
        0,                                          /* lpMultiByteStr */
        0,                                          /* cbMultiByte */
        0,                                          /* lpDefaultChar */
        0);                                         /* lpUsedDefaultChar */
#else
    /* TODO: Support for non-Windows platforms. */
    HL_ASSERT(0);
    return 0; /* So compiler doesn't complain. */
#endif
}

size_t hlStrGetReqLenUTF16ToUTF32(const HlChar16* src, size_t srcLen)
{
    /* TODO: Implement this function. */
    HL_ASSERT(0);
    return 0; /* So compiler doesn't complain. */
}

size_t hlStrGetReqLenUTF32ToUTF8(const HlChar32* src, size_t srcLen)
{
    /* TODO: Implement this function. */
    HL_ASSERT(0);
    return 0; /* So compiler doesn't complain. */
}

size_t hlStrGetReqLenUTF32ToUTF16(const HlChar32* src, size_t srcLen)
{
    /* TODO: Implement this function. */
    HL_ASSERT(0);
    return 0; /* So compiler doesn't complain. */
}

size_t hlStrConvUTF8ToUTF16NoAlloc(const char* HL_RESTRICT src,
    HlChar16* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen)
{
#ifdef _WIN32
    /* Ensure casting srcLen and dstBufLen to signed integers will be valid. */
    HL_ASSERT(((srcLen == 0) || (srcLen <= INT_MAX)) &&
        ((dstBufLen == 0) || (dstBufLen <= INT_MAX)));

    /* Convert the UTF-8 string to UTF-16. */
    return (size_t)MultiByteToWideChar(
        CP_UTF8,                                    /* CodePage */
        0,                                          /* dwFlags */
        (const char*)src,                           /* lpMultiByteStr */
        (srcLen) ? (int)srcLen : -1,                /* cbMultiByte */
        (wchar_t*)dst,                              /* lpWideCharStr */
        (dstBufLen) ? (int)dstBufLen : INT_MAX);    /* cchWideChar */
#else
    /* TODO: Support for non-Windows platforms. */
    HL_ASSERT(0);
    return 0; /* So compiler doesn't complain. */
#endif
}

size_t hlStrConvUTF8ToUTF32NoAlloc(const char* HL_RESTRICT src,
    HlChar32* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen)
{
    /* TODO: Implement this function. */
    HL_ASSERT(0);
    return 0; /* So compiler doesn't complain. */
}

size_t hlStrConvUTF16ToUTF8NoAlloc(const HlChar16* HL_RESTRICT src,
    char* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen)
{
#ifdef _WIN32
    /* Ensure casting srcLen and dstBufLen to signed integers will be valid. */
    HL_ASSERT(((srcLen == 0) || (srcLen <= INT_MAX)) &&
        ((dstBufLen == 0) || (dstBufLen <= INT_MAX)));

    /* Convert the UTF-16 string to UTF-8. */
    return (size_t)WideCharToMultiByte(
        CP_UTF8,                                    /* CodePage */
        0,                                          /* dwFlags */
        (const wchar_t*)src,                        /* lpWideCharStr */
        (srcLen) ? (int)srcLen : -1,                /* cchWideChar */
        (char*)dst,                                 /* lpMultiByteStr */
        (dstBufLen) ? (int)dstBufLen : INT_MAX,     /* cbMultiByte */
        0,                                          /* lpDefaultChar */
        0);                                         /* lpUsedDefaultChar */
#else
    /* TODO: Support for non-Windows platforms. */
    HL_ASSERT(0);
    return 0; /* So compiler doesn't complain. */
#endif
}

size_t hlStrConvUTF16ToUTF32NoAlloc(const HlChar16* HL_RESTRICT src,
    HlChar32* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen)
{
    /* TODO: Implement this function. */
    HL_ASSERT(0);
    return 0; /* So compiler doesn't complain. */
}

size_t hlStrConvUTF32ToUTF8NoAlloc(const HlChar32* HL_RESTRICT src,
    char* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen)
{
    /* TODO: Implement this function. */
    HL_ASSERT(0);
    return 0; /* So compiler doesn't complain. */
}

size_t hlStrConvUTF32ToUTF16NoAlloc(const HlChar32* HL_RESTRICT src,
    HlChar16* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen)
{
    /* TODO: Implement this function. */
    HL_ASSERT(0);
    return 0; /* So compiler doesn't complain. */
}

HlChar16* hlStrConvUTF8ToUTF16(const char* src, size_t srcLen)
{
    HlChar16* dst;
    size_t dstBufLen;

    /* Get required length for UTF-16 buffer. */
    dstBufLen = hlStrGetReqLenUTF8ToUTF16(src, srcLen);
    if (!dstBufLen) return 0;

    /* Allocate UTF-16 buffer, convert src string, and store the result in that buffer. */
    dst = HL_ALLOC_ARR(HlChar16, dstBufLen);
    if (!hlStrConvUTF8ToUTF16NoAlloc(src, dst, srcLen, 0))
    {
        hlFree(dst);
        return 0;
    }

    return dst;
}

HlChar32* hlStrConvUTF8ToUTF32(const char* src, size_t srcLen)
{
    /* TODO: Implement this function. */
    HL_ASSERT(0);
    return 0; /* So compiler doesn't complain. */
}

char* hlStrConvUTF16ToUTF8(const HlChar16* src, size_t srcLen)
{
    char* dst;
    size_t dstBufLen;

    /* Get required length for UTF-8 buffer. */
    dstBufLen = hlStrGetReqLenUTF16ToUTF8(src, srcLen);
    if (!dstBufLen) return 0;

    /* Allocate UTF-8 buffer, convert src string, and store the result in that buffer. */
    dst = HL_ALLOC_ARR(char, dstBufLen);
    if (!hlStrConvUTF16ToUTF8NoAlloc(src, dst, srcLen, 0))
    {
        hlFree(dst);
        return 0;
    }

    return dst;
}

HlChar32* hlStrConvUTF16ToUTF32(const HlChar16* src, size_t srcLen)
{
    /* TODO: Implement this function. */
    HL_ASSERT(0);
    return 0; /* So compiler doesn't complain. */
}

char* hlStrConvUTF32ToUTF8(const HlChar32* src, size_t srcLen)
{
    /* TODO: Implement this function. */
    HL_ASSERT(0);
    return 0; /* So compiler doesn't complain. */
}

HlChar16* hlStrConvUTF32ToUTF16(const HlChar32* src, size_t srcLen)
{
    /* TODO: Implement this function. */
    HL_ASSERT(0);
    return 0; /* So compiler doesn't complain. */
}
