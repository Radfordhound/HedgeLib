#ifndef HL_PATH_H_INCLUDED
#define HL_PATH_H_INCLUDED
#include "../hl_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Path separator macros. */
#ifdef _WIN32
#define HL_PATH_SEP     HL_NTEXT('\\')
#else
#define HL_PATH_SEP     HL_NTEXT('/')
#endif

#ifdef _WIN32
#define HL_IS_PATH_SEP(c) ((c) == HL_NTEXT('\\') || (c) == HL_NTEXT('/'))
#else
/*
   (Don't include backslashes in this check, as paths on
   POSIX systems allow backslashes in file names.)
*/
#define HL_IS_PATH_SEP(c) ((c) == HL_NTEXT('/'))
#endif

HL_API const HlNChar* hlPathGetName(const HlNChar* path);
HL_API const HlNChar* hlPathGetExt(const HlNChar* path, size_t pathLen);
HL_API const HlNChar* hlPathGetExts(const HlNChar* path, size_t pathLen);

HL_API size_t hlPathRemoveExtNoAlloc(const HlNChar* HL_RESTRICT path,
    HlNChar* HL_RESTRICT result, size_t pathLen);

HL_API size_t hlPathRemoveExtsNoAlloc(const HlNChar* HL_RESTRICT path,
    HlNChar* HL_RESTRICT result, size_t pathLen);

HL_API HlNChar* hlPathRemoveExt(const HlNChar* path, size_t pathLen);
HL_API HlNChar* hlPathRemoveExts(const HlNChar* path, size_t pathLen);

HL_API HlBool hlPathCombineNeedsSep(const HlNChar* HL_RESTRICT path1,
    const HlNChar* HL_RESTRICT path2, size_t path1Len);

HL_API void hlPathCombineNoAlloc(const HlNChar* HL_RESTRICT path1,
    const HlNChar* HL_RESTRICT path2, HlNChar* HL_RESTRICT result,
    size_t path1Len, size_t path2Len);

HL_API HlNChar* hlPathCombine(const HlNChar* HL_RESTRICT path1,
    const HlNChar* HL_RESTRICT path2, size_t path1Len, size_t path2Len);

HL_API size_t hlPathGetSize(const HlNChar* filePath);
HL_API HlBool hlPathExists(const HlNChar* path);
HL_API HlResult hlPathCreateDirectory(const HlNChar* dirPath, HlBool overwrite);

#ifdef __cplusplus
}
#endif
#endif
