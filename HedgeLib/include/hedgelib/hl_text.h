#ifndef HL_TEXT_H_INCLUDED
#define HL_TEXT_H_INCLUDED
#include "hl_memory.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Helper macros */
#define HL_IS_DIGIT(c) ((c) <= '9' && (c) >= '0')

/** @brief Auto-compute text length. Supports both pointers and string literals. */
#define HL_IN_TEXT_GET_LEN(text)\
    ((sizeof(text) == sizeof(char*)) ? strlen(text) :\
    ((sizeof(text) / sizeof(char)) - 1))

/** @brief Auto-compute text length. Supports both pointers and string literals. */
#define HL_IN_NTEXT_GET_LEN(text)\
    ((sizeof(text) == sizeof(HlNChar*)) ? hlNStrLen(text) :\
    ((sizeof(text) / sizeof(HlNChar)) - 1))

/* String helper functions */
HL_API size_t hlStrCopyAndLen(const char* HL_RESTRICT src, char* HL_RESTRICT dst);
HL_API HlBool hlStrCopyLimit(const char* HL_RESTRICT src,
    char* HL_RESTRICT dst, size_t dstBufLen,
    size_t* HL_RESTRICT copiedCount);

/* Native string helper functions */
HL_API size_t hlNStrLen(const HlNChar* str);
HL_API HlNChar* hlNStrCopy(const HlNChar* HL_RESTRICT src, HlNChar* HL_RESTRICT dst);
HL_API int hlNStrCmp(const HlNChar* HL_RESTRICT str1, const HlNChar* HL_RESTRICT str2);
HL_API int hlNStrNCmp(const HlNChar* HL_RESTRICT str1,
    const HlNChar* HL_RESTRICT str2, size_t maxCount);

HL_API int hlNStrICmp(const HlNChar* HL_RESTRICT str1,
    const HlNChar* HL_RESTRICT str2);

HL_API int hlNStrNICmp(const HlNChar* HL_RESTRICT str1,
    const HlNChar* HL_RESTRICT str2, size_t maxCount);

HL_API const HlNChar* hlNStrStr(const HlNChar* HL_RESTRICT str1,
    const HlNChar* HL_RESTRICT str2);

#define hlNStrsEqual(str1, str2) (HlBool)(hlNStrCmp(str1, str2) == 0)

HL_API size_t hlNStrCopyAndLen(const HlNChar* HL_RESTRICT src, HlNChar* HL_RESTRICT dst);
HL_API HlBool hlNStrCopyLimit(const HlNChar* HL_RESTRICT src,
    HlNChar* HL_RESTRICT dst, size_t dstBufLen,
    size_t* HL_RESTRICT copiedCount);

/* String conversion functions */
HL_API size_t hlStrGetReqLenUTF8ToUTF16(const char* src, size_t srcLen);
HL_API size_t hlStrGetReqLenUTF8ToUTF32(const char* src, size_t srcLen);
HL_API size_t hlStrGetReqLenUTF8ToNative(const char* src, size_t srcLen);

HL_API size_t hlStrGetReqLenUTF16ToUTF8(const HlChar16* src, size_t srcLen);
HL_API size_t hlStrGetReqLenUTF16ToUTF32(const HlChar16* src, size_t srcLen);
HL_API size_t hlStrGetReqLenUTF16ToNative(const HlChar16* src, size_t srcLen);

HL_API size_t hlStrGetReqLenUTF32ToUTF8(const HlChar32* src, size_t srcLen);
HL_API size_t hlStrGetReqLenUTF32ToUTF16(const HlChar32* src, size_t srcLen);
HL_API size_t hlStrGetReqLenUTF32ToNative(const HlChar32* src, size_t srcLen);

HL_API size_t hlStrGetReqLenNativeToUTF8(const HlNChar* src, size_t srcLen);
HL_API size_t hlStrGetReqLenNativeToUTF16(const HlNChar* src, size_t srcLen);
HL_API size_t hlStrGetReqLenNativeToUTF32(const HlNChar* src, size_t srcLen);

HL_API size_t hlStrConvUTF8ToUTF16NoAlloc(const char* HL_RESTRICT src,
    HlChar16* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen);

HL_API size_t hlStrConvUTF8ToUTF32NoAlloc(const char* HL_RESTRICT src,
    HlChar32* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen);

HL_API size_t hlStrConvUTF8ToNativeNoAlloc(const char* HL_RESTRICT src,
    HlNChar* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen);

HL_API size_t hlStrConvUTF16ToUTF8NoAlloc(const HlChar16* HL_RESTRICT src,
    char* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen);

HL_API size_t hlStrConvUTF16ToUTF32NoAlloc(const HlChar16* HL_RESTRICT src,
    HlChar32* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen);

HL_API size_t hlStrConvUTF16ToNativeNoAlloc(const HlChar16* HL_RESTRICT src,
    HlNChar* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen);

HL_API size_t hlStrConvUTF32ToUTF8NoAlloc(const HlChar32* HL_RESTRICT src,
    char* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen);

HL_API size_t hlStrConvUTF32ToUTF16NoAlloc(const HlChar32* HL_RESTRICT src,
    HlChar16* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen);

HL_API size_t hlStrConvUTF32ToNativeNoAlloc(const HlChar32* HL_RESTRICT src,
    HlNChar* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen);

HL_API size_t hlStrConvNativeToUTF8NoAlloc(const HlNChar* HL_RESTRICT src,
    char* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen);

HL_API size_t hlStrConvNativeToUTF16NoAlloc(const HlNChar* HL_RESTRICT src,
    HlChar16* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen);

HL_API size_t hlStrConvNativeToUTF32NoAlloc(const HlNChar* HL_RESTRICT src,
    HlChar32* HL_RESTRICT dst, size_t srcLen, size_t dstBufLen);

HL_API HlChar16* hlStrConvUTF8ToUTF16(const char* src, size_t srcLen);
HL_API HlChar32* hlStrConvUTF8ToUTF32(const char* src, size_t srcLen);
HL_API HlNChar* hlStrConvUTF8ToNative(const char* src, size_t srcLen);

HL_API char* hlStrConvUTF16ToUTF8(const HlChar16* src, size_t srcLen);
HL_API HlChar32* hlStrConvUTF16ToUTF32(const HlChar16* src, size_t srcLen);
HL_API HlNChar* hlStrConvUTF16ToNative(const HlChar16* src, size_t srcLen);

HL_API char* hlStrConvUTF32ToUTF8(const HlChar32* src, size_t srcLen);
HL_API HlChar16* hlStrConvUTF32ToUTF16(const HlChar32* src, size_t srcLen);
HL_API HlNChar* hlStrConvUTF32ToNative(const HlChar32* src, size_t srcLen);

HL_API char* hlStrConvNativeToUTF8(const HlNChar* src, size_t srcLen);
HL_API HlChar16* hlStrConvNativeToUTF16(const HlNChar* src, size_t srcLen);
HL_API HlChar32* hlStrConvNativeToUTF32(const HlNChar* src, size_t srcLen);

#ifndef HL_NO_EXTERNAL_WRAPPERS
HL_API HlBool hlNStrsEqualExt(const HlNChar* HL_RESTRICT str1,
    const HlNChar* HL_RESTRICT str2);
#endif

#ifdef __cplusplus
}

/* C++-specific stuff */
#include <string>

typedef std::basic_string<char, std::char_traits<char>, HlAllocator<char>> HlString;
typedef std::basic_string<HlNChar, std::char_traits<HlNChar>, HlAllocator<HlNChar>> HlNString;

#endif
#endif
