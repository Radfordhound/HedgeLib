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
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
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
    char* HL_RESTRICT dst, size_t dstBufLen,
    size_t* HL_RESTRICT copiedCount)
{
    /* Copy up to dstBufLen characters from src into dst. */
    size_t copied = 0;
    while (copied < dstBufLen)
    {
        ++copied;
        if (!(*dst++ = *src++))
        {
            if (copiedCount) *copiedCount = copied;
            return HL_TRUE;
        }
    }

    if (copiedCount) *copiedCount = copied;
    return HL_FALSE;
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

int hlNStrCmp(const HlNChar* HL_RESTRICT str1, const HlNChar* HL_RESTRICT str2)
{
#ifdef HL_IN_WIN32_UNICODE
    return wcscmp(str1, str2);
#else
    return strcmp(str1, str2);
#endif
}

int hlNStrNCmp(const HlNChar* HL_RESTRICT str1,
    const HlNChar* HL_RESTRICT str2, size_t maxCount)
{
#ifdef HL_IN_WIN32_UNICODE
    return wcsncmp(str1, str2, maxCount);
#else
    return strncmp(str1, str2, maxCount);
#endif
}

int hlNStrICmp(const HlNChar* HL_RESTRICT str1,
    const HlNChar* HL_RESTRICT str2)
{
    HlNChar c1 = 0, c2 = 0;
    while ((c2 = *str2, c1 = *str1))
    {
        if (c1 != c2)
        {
            if (c1 >= HL_NTEXT('A') && c2 >= HL_NTEXT('A'))
            {
                /* If c1 is lower-cased, upper-case it. */
                if (c1 >= HL_NTEXT('a') && c1 <= HL_NTEXT('z'))
                {
                    c1 -= 32;
                }

                /* If c2 is lower-cased, upper-case it. */
                if (c2 >= HL_NTEXT('a') && c2 <= HL_NTEXT('z'))
                {
                    c2 -= 32;
                }

                if (c1 != c2) goto end;
            }
            else goto end;
        }

        ++str1;
        ++str2;
    }

end:
    return (((int)c1) - ((int)c2));
}

int hlNStrNICmp(const HlNChar* HL_RESTRICT str1,
    const HlNChar* HL_RESTRICT str2, size_t maxCount)
{
    HlNChar c1 = 0, c2 = 0;
    while (maxCount && (c2 = *str2, c1 = *str1))
    {
        if (c1 != c2)
        {
            if (c1 >= HL_NTEXT('A') && c2 >= HL_NTEXT('A'))
            {
                /* If c1 is lower-cased, upper-case it. */
                if (c1 >= HL_NTEXT('a') && c1 <= HL_NTEXT('z'))
                {
                    c1 -= 32;
                }

                /* If c2 is lower-cased, upper-case it. */
                if (c2 >= HL_NTEXT('a') && c2 <= HL_NTEXT('z'))
                {
                    c2 -= 32;
                }

                if (c1 != c2) goto end;
            }
            else goto end;
        }

        ++str1;
        ++str2;
        --maxCount;
    }

end:
    if (maxCount == 0) return 0;

    return (((int)c1) - ((int)c2));
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
    HlNChar* HL_RESTRICT dst, size_t dstBufLen,
    size_t* HL_RESTRICT copiedCount)
{
    /* Copy up to dstBufLen characters from src into dst. */
    size_t copied = 0;
    while (copied < dstBufLen)
    {
        ++copied;
        if (!(*dst++ = *src++))
        {
            if (copiedCount) *copiedCount = copied;
            return HL_TRUE;
        }
    }

    if (copiedCount) *copiedCount = copied;
    return HL_FALSE;
}

size_t hlStrGetReqLenUTF8ToUTF16(const char* src, size_t srcLen)
{
#ifdef _WIN32
    /* Ensure casting srcLen to a signed integer will be valid. */
    HL_ASSERT(srcLen <= INT_MAX);

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

size_t hlStrGetReqLenUTF8ToNative(const char* src, size_t srcLen)
{
#ifdef HL_IN_WIN32_UNICODE
    return hlStrGetReqLenUTF8ToUTF16(src, srcLen);
#else
    return (srcLen) ? srcLen : (strlen(src) + 1);
#endif
}

size_t hlStrGetReqLenUTF16ToUTF8(const HlChar16* src, size_t srcLen)
{
#ifdef _WIN32
    /* Ensure casting srcLen to a signed integer will be valid. */
    HL_ASSERT(srcLen <= INT_MAX);

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

size_t hlStrGetReqLenUTF16ToNative(const HlChar16* src, size_t srcLen)
{
#ifdef HL_IN_WIN32_UNICODE
    return (srcLen) ? srcLen : (wcslen((const wchar_t*)src) + 1);
#else
    return hlStrGetReqLenUTF16ToUTF8(src, srcLen);
#endif
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

size_t hlStrGetReqLenUTF32ToNative(const HlChar32* src, size_t srcLen)
{
#ifdef HL_IN_WIN32_UNICODE
    return hlStrGetReqLenUTF32ToUTF16(src, srcLen);
#else
    return hlStrGetReqLenUTF32ToUTF8(src, srcLen);
#endif
}

size_t hlStrGetReqLenNativeToUTF8(const HlNChar* src, size_t srcLen)
{
#ifdef HL_IN_WIN32_UNICODE
    return hlStrGetReqLenUTF16ToUTF8((const HlChar16*)src, srcLen);
#else
    return (srcLen) ? srcLen : (strlen(src) + 1);
#endif
}

size_t hlStrGetReqLenNativeToUTF16(const HlNChar* src, size_t srcLen)
{
#ifdef HL_IN_WIN32_UNICODE
    return (srcLen) ? srcLen : (wcslen((const wchar_t*)src) + 1);
#else
    return hlStrGetReqLenUTF8ToUTF16(src, srcLen);
#endif
}

size_t hlStrGetReqLenNativeToUTF32(const HlNChar* src, size_t srcLen)
{
#ifdef HL_IN_WIN32_UNICODE
    return hlStrGetReqLenUTF16ToUTF32((const HlChar16*)src, srcLen);
#else
    return hlStrGetReqLenUTF8ToUTF32(src, srcLen);
#endif
}

size_t hlStrConvUTF8ToUTF16NoAlloc(const char* HL_RESTRICT src,
    HlChar16* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen)
{
#ifdef _WIN32
    /* Ensure casting srcLen and dstBufLen to signed integers will be valid. */
    HL_ASSERT((srcLen <= INT_MAX) && (dstBufLen <= INT_MAX));

    /*
       If the destination buffer size was not provided by the
       user, compute the required destination buffer size.
    */
    if (!dstBufLen)
    {
        dstBufLen = hlStrGetReqLenUTF8ToUTF16(src, srcLen);
    }

    /* Convert the UTF-8 string to UTF-16. */
    return (size_t)MultiByteToWideChar(
        CP_UTF8,                                    /* CodePage */
        0,                                          /* dwFlags */
        (const char*)src,                           /* lpMultiByteStr */
        (srcLen) ? (int)srcLen : -1,                /* cbMultiByte */
        (wchar_t*)dst,                              /* lpWideCharStr */
        (int)dstBufLen);                            /* cchWideChar */
#else
    /* NOTE: We assume dst is NULL-checked in this function throughout HedgeLib. */
    if (!dst) return 0;

    /* TODO: Support for non-Windows platforms. */
    HL_ASSERT(0);
    return 0; /* So compiler doesn't complain. */
#endif
}

size_t hlStrConvUTF8ToUTF32NoAlloc(const char* HL_RESTRICT src,
    HlChar32* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen)
{
    /* NOTE: We assume dst is NULL-checked in this function throughout HedgeLib. */
    if (!dst) return 0;

    /* TODO: Implement this function. */
    HL_ASSERT(0);
    return 0; /* So compiler doesn't complain. */
}

size_t hlStrConvUTF8ToNativeNoAlloc(const char* HL_RESTRICT src,
    HlNChar* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen)
{
#ifdef HL_IN_WIN32_UNICODE
    return hlStrConvUTF8ToUTF16NoAlloc(src,
        (HlChar16*)dst, srcLen, dstBufLen);
#else
    /* Ensure dst is not NULL. */
    if (!dst) return 0;

    /* Copy src into dst as necessary with the given arguments. */
    if (srcLen)
    {
        /* Ensure src can fit within dst. */
        if (dstBufLen && srcLen > dstBufLen) return 0;

        /* Copy src into dst and return copied length. */
        memcpy(dst, src, sizeof(char) * srcLen);
        return srcLen;
    }
    else if (dstBufLen)
    {
        /* Copy src into dst, stopping if we exceed dstBufLen, and return copied length. */
        size_t copiedLen;
        return ((hlStrCopyLimit(src, dst, dstBufLen,
            &copiedLen)) ? copiedLen : 0);
    }
    else
    {
        /* Copy src into dst and return src length + 1 for null terminator. */
        return (hlStrCopyAndLen(src, dst) + 1);
    }
#endif
}

size_t hlStrConvUTF16ToUTF8NoAlloc(const HlChar16* HL_RESTRICT src,
    char* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen)
{
#ifdef _WIN32
    /* Ensure casting srcLen and dstBufLen to signed integers will be valid. */
    HL_ASSERT((srcLen <= INT_MAX) && (dstBufLen <= INT_MAX));

    /*
       If the destination buffer size was not provided by the
       user, compute the required destination buffer size.
    */
    if (!dstBufLen)
    {
        dstBufLen = hlStrGetReqLenUTF16ToUTF8(src, srcLen);
    }

    /* Convert the UTF-16 string to UTF-8. */
    return (size_t)WideCharToMultiByte(
        CP_UTF8,                        /* CodePage */
        0,                              /* dwFlags */
        (const wchar_t*)src,            /* lpWideCharStr */
        (srcLen) ? (int)srcLen : -1,    /* cchWideChar */
        (char*)dst,                     /* lpMultiByteStr */
        (int)dstBufLen,                 /* cbMultiByte */
        0,                              /* lpDefaultChar */
        0);                             /* lpUsedDefaultChar */
#else
    /* NOTE: We assume dst is NULL-checked in this function throughout HedgeLib. */
    if (!dst) return 0;

    /* TODO: Support for non-Windows platforms. */
    HL_ASSERT(0);
    return 0; /* So compiler doesn't complain. */
#endif
}

size_t hlStrConvUTF16ToUTF32NoAlloc(const HlChar16* HL_RESTRICT src,
    HlChar32* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen)
{
    /* NOTE: We assume dst is NULL-checked in this function throughout HedgeLib. */
    if (!dst) return 0;

    /* TODO: Implement this function. */
    HL_ASSERT(0);
    return 0; /* So compiler doesn't complain. */
}

size_t hlStrConvUTF16ToNativeNoAlloc(const HlChar16* HL_RESTRICT src,
    HlNChar* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen)
{
#ifdef HL_IN_WIN32_UNICODE
    /* Ensure dst is not NULL. */
    if (!dst) return 0;

    /* Copy src into dst as necessary with the given arguments. */
    if (srcLen)
    {
        /* Ensure src can fit within dst. */
        if (dstBufLen && srcLen > dstBufLen) return 0;

        /* Copy src into dst and return copied length. */
        memcpy(dst, src, srcLen * sizeof(HlChar16));
        return srcLen;
    }
    else if (dstBufLen)
    {
        /* Copy src into dst, stopping if we exceed dstBufLen, and return copied length. */
        size_t copiedLen;
        return ((hlNStrCopyLimit((const HlNChar*)src,
            dst, dstBufLen, &copiedLen)) ? copiedLen : 0);
    }
    else
    {
        /* Copy src into dst and return src length + 1 for null terminator. */
        return (hlNStrCopyAndLen((const HlNChar*)src, dst) + 1);
    }
#else
    return hlStrConvUTF16ToUTF8NoAlloc(src, dst, srcLen, dstBufLen);
#endif
}

size_t hlStrConvUTF32ToUTF8NoAlloc(const HlChar32* HL_RESTRICT src,
    char* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen)
{
    /* NOTE: We assume dst is NULL-checked in this function throughout HedgeLib. */
    if (!dst) return 0;

    /* TODO: Implement this function. */
    HL_ASSERT(0);
    return 0; /* So compiler doesn't complain. */
}

size_t hlStrConvUTF32ToUTF16NoAlloc(const HlChar32* HL_RESTRICT src,
    HlChar16* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen)
{
    /* NOTE: We assume dst is NULL-checked in this function throughout HedgeLib. */
    if (!dst) return 0;

    /* TODO: Implement this function. */
    HL_ASSERT(0);
    return 0; /* So compiler doesn't complain. */
}

size_t hlStrConvUTF32ToNativeNoAlloc(const HlChar32* HL_RESTRICT src,
    HlNChar* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen)
{
#ifdef HL_IN_WIN32_UNICODE
    return hlStrConvUTF32ToUTF16NoAlloc(src,
        (HlChar16*)dst, srcLen, dstBufLen);
#else
    return hlStrConvUTF32ToUTF8NoAlloc(src,
        dst, srcLen, dstBufLen);
#endif
}

size_t hlStrConvNativeToUTF8NoAlloc(const HlNChar* HL_RESTRICT src,
    char* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen)
{
#ifdef HL_IN_WIN32_UNICODE
    return hlStrConvUTF16ToUTF8NoAlloc((const HlChar16*)src,
        dst, srcLen, dstBufLen);
#else
    /* NOTE: We assume dst is NULL-checked in this function throughout HedgeLib. */
    if (!dst) return 0;

    /* Copy src into dst as necessary with the given arguments. */
    if (srcLen)
    {
        /* Ensure src can fit within dst. */
        if (dstBufLen && srcLen > dstBufLen) return 0;

        /* Copy src into dst and return copied length. */
        memcpy(dst, src, sizeof(HlNChar) * srcLen);
        return srcLen;
    }
    else if (dstBufLen)
    {
        /* Copy src into dst, stopping if we exceed dstBufLen, and return copied length. */
        size_t copiedLen;
        return ((hlNStrCopyLimit(src, dst, dstBufLen,
            &copiedLen)) ? copiedLen : 0);
    }
    else
    {
        /* Copy src into dst and return src length + 1 for null terminator. */
        return (hlNStrCopyAndLen(src, dst) + 1);
    }
#endif
}

size_t hlStrConvNativeToUTF16NoAlloc(const HlNChar* HL_RESTRICT src,
    HlChar16* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen)
{
#ifdef HL_IN_WIN32_UNICODE
    /* NOTE: We assume dst is NULL-checked in this function throughout HedgeLib. */
    if (!dst) return 0;

    /* Copy src into dst as necessary with the given arguments. */
    if (srcLen)
    {
        /* Ensure src can fit within dst. */
        if (dstBufLen && srcLen > dstBufLen) return 0;

        /* Copy src into dst and return copied length. */
        memcpy(dst, src, sizeof(HlNChar) * srcLen);
        return srcLen;
    }
    else if (dstBufLen)
    {
        /* Copy src into dst, stopping if we exceed dstBufLen, and return copied length. */
        size_t copiedLen;
        return ((hlNStrCopyLimit(src, (HlNChar*)dst, dstBufLen,
            &copiedLen)) ? copiedLen : 0);
    }
    else
    {
        /* Copy src into dst and return src length + 1 for null terminator. */
        return (hlNStrCopyAndLen(src, (HlNChar*)dst) + 1);
    }
#else
    return hlStrConvUTF8ToUTF16NoAlloc(src, dst, srcLen, dstBufLen);
#endif
}

size_t hlStrConvNativeToUTF32NoAlloc(const HlNChar* HL_RESTRICT src,
    HlChar32* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen)
{
#ifdef HL_IN_WIN32_UNICODE
    return hlStrConvUTF16ToUTF32NoAlloc((const HlChar16*)src,
        dst, srcLen, dstBufLen);
#else
    return hlStrConvUTF8ToUTF32NoAlloc(src,
        dst, srcLen, dstBufLen);
#endif
}

HlChar16* hlStrConvUTF8ToUTF16(const char* src, size_t srcLen)
{
    HlChar16* dst;
    size_t dstBufLen;

    /* Get required length for UTF-16 buffer. */
    dstBufLen = hlStrGetReqLenUTF8ToUTF16(src, srcLen);
    if (!dstBufLen) return NULL;

    /* Allocate UTF-16 buffer, convert src string, and store the result in that buffer. */
    dst = HL_ALLOC_ARR(HlChar16, dstBufLen);
    if (!hlStrConvUTF8ToUTF16NoAlloc(src, dst, srcLen, dstBufLen))
    {
        hlFree(dst);
        return NULL;
    }

    return dst;
}

HlChar32* hlStrConvUTF8ToUTF32(const char* src, size_t srcLen)
{
    HlChar32* dst;
    size_t dstBufLen;

    /* Get required length for UTF-32 buffer. */
    dstBufLen = hlStrGetReqLenUTF8ToUTF32(src, srcLen);
    if (!dstBufLen) return NULL;

    /* Allocate UTF-32 buffer, convert src string, and store the result in that buffer. */
    dst = HL_ALLOC_ARR(HlChar32, dstBufLen);
    if (!hlStrConvUTF8ToUTF32NoAlloc(src, dst, srcLen, dstBufLen))
    {
        hlFree(dst);
        return NULL;
    }

    return dst;
}

HlNChar* hlStrConvUTF8ToNative(const char* src, size_t srcLen)
{
#ifdef HL_IN_WIN32_UNICODE
    return (HlNChar*)hlStrConvUTF8ToUTF16(src, srcLen);
#else
    HlNChar* dst;

    /* Get src length if necessary. */
    if (!srcLen) srcLen = (strlen(src) + 1);

    /* Allocate native buffer, copy src string into buffer, and return buffer. */
    dst = HL_ALLOC_ARR(HlNChar, srcLen);
    if (!dst) return NULL;

    memcpy(dst, src, sizeof(char) * srcLen);
    return dst;
#endif
}

char* hlStrConvUTF16ToUTF8(const HlChar16* src, size_t srcLen)
{
    char* dst;
    size_t dstBufLen;

    /* Get required length for UTF-8 buffer. */
    dstBufLen = hlStrGetReqLenUTF16ToUTF8(src, srcLen);
    if (!dstBufLen) return NULL;

    /* Allocate UTF-8 buffer, convert src string, and store the result in that buffer. */
    dst = HL_ALLOC_ARR(char, dstBufLen);
    if (!hlStrConvUTF16ToUTF8NoAlloc(src, dst, srcLen, dstBufLen))
    {
        hlFree(dst);
        return NULL;
    }

    return dst;
}

HlChar32* hlStrConvUTF16ToUTF32(const HlChar16* src, size_t srcLen)
{
    HlChar32* dst;
    size_t dstBufLen;

    /* Get required length for UTF-32 buffer. */
    dstBufLen = hlStrGetReqLenUTF16ToUTF32(src, srcLen);
    if (!dstBufLen) return NULL;

    /* Allocate UTF-32 buffer, convert src string, and store the result in that buffer. */
    dst = HL_ALLOC_ARR(HlChar32, dstBufLen);
    if (!hlStrConvUTF16ToUTF32NoAlloc(src, dst, srcLen, dstBufLen))
    {
        hlFree(dst);
        return NULL;
    }

    return dst;
}

HlNChar* hlStrConvUTF16ToNative(const HlChar16* src, size_t srcLen)
{
#ifdef HL_IN_WIN32_UNICODE
    HlNChar* dst;

    /* Get src length if necessary. */
    if (!srcLen) srcLen = (wcslen((const wchar_t*)src) + 1);

    /* Allocate native buffer, copy src string into buffer, and return buffer. */
    dst = HL_ALLOC_ARR(HlNChar, srcLen);
    if (!dst) return NULL;

    memcpy(dst, src, sizeof(HlChar16) * srcLen);
    return dst;
#else
    return hlStrConvUTF16ToUTF8(src, srcLen);
#endif
}

char* hlStrConvUTF32ToUTF8(const HlChar32* src, size_t srcLen)
{
    char* dst;
    size_t dstBufLen;

    /* Get required length for UTF-8 buffer. */
    dstBufLen = hlStrGetReqLenUTF32ToUTF8(src, srcLen);
    if (!dstBufLen) return NULL;

    /* Allocate UTF-8 buffer, convert src string, and store the result in that buffer. */
    dst = HL_ALLOC_ARR(char, dstBufLen);
    if (!hlStrConvUTF32ToUTF8NoAlloc(src, dst, srcLen, dstBufLen))
    {
        hlFree(dst);
        return NULL;
    }

    return dst;
}

HlChar16* hlStrConvUTF32ToUTF16(const HlChar32* src, size_t srcLen)
{
    HlChar16* dst;
    size_t dstBufLen;

    /* Get required length for UTF-16 buffer. */
    dstBufLen = hlStrGetReqLenUTF32ToUTF16(src, srcLen);
    if (!dstBufLen) return NULL;

    /* Allocate UTF-16 buffer, convert src string, and store the result in that buffer. */
    dst = HL_ALLOC_ARR(HlChar16, dstBufLen);
    if (!hlStrConvUTF32ToUTF16NoAlloc(src, dst, srcLen, dstBufLen))
    {
        hlFree(dst);
        return NULL;
    }

    return dst;
}

HlNChar* hlStrConvUTF32ToNative(const HlChar32* src, size_t srcLen)
{
#ifdef HL_IN_WIN32_UNICODE
    return (HlNChar*)hlStrConvUTF32ToUTF16(src, srcLen);
#else
    return hlStrConvUTF32ToUTF8(src, srcLen);
#endif
}

char* hlStrConvNativeToUTF8(const HlNChar* src, size_t srcLen)
{
#ifdef HL_IN_WIN32_UNICODE
    return hlStrConvUTF16ToUTF8((const HlChar16*)src, srcLen);
#else
    HlNChar* dst;

    /* Get src length if necessary. */
    if (!srcLen) srcLen = (strlen(src) + 1);

    /* Allocate native buffer, copy src string into buffer, and return buffer. */
    dst = HL_ALLOC_ARR(HlNChar, srcLen);
    if (!dst) return NULL;

    memcpy(dst, src, sizeof(HlNChar) * srcLen);
    return dst;
#endif
}

HlChar16* hlStrConvNativeToUTF16(const HlNChar* src, size_t srcLen)
{
#ifdef HL_IN_WIN32_UNICODE
    HlChar16* dst;

    /* Get src length if necessary. */
    if (!srcLen) srcLen = (wcslen((const wchar_t*)src) + 1);

    /* Allocate native buffer, copy src string into buffer, and return buffer. */
    dst = HL_ALLOC_ARR(HlChar16, srcLen);
    if (!dst) return NULL;

    memcpy(dst, src, sizeof(HlNChar) * srcLen);
    return dst;
#else
    return hlStrConvUTF8ToUTF16(src, srcLen);
#endif
}

HlChar32* hlStrConvNativeToUTF32(const HlNChar* src, size_t srcLen)
{
#ifdef HL_IN_WIN32_UNICODE
    return hlStrConvUTF16ToUTF32((const HlChar16*)src, srcLen);
#else
    return hlStrConvUTF8ToUTF32(src, srcLen);
#endif
}

#ifndef HL_NO_EXTERNAL_WRAPPERS
HlBool hlNStrsEqualExt(const HlNChar* HL_RESTRICT str1,
    const HlNChar* HL_RESTRICT str2)
{
    return hlNStrsEqual(str1, str2);
}
#endif
