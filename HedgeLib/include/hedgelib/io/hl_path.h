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

#define HlDirHandle void*

typedef enum HlDirEntryType
{
    HL_DIR_ENTRY_TYPE_UNKNOWN = 0,
    HL_DIR_ENTRY_TYPE_PIPE = 1,
    HL_DIR_ENTRY_TYPE_CHAR_DEV = 2,
    HL_DIR_ENTRY_TYPE_DIRECTORY = 4,
    HL_DIR_ENTRY_TYPE_BLOCK_DEV = 6,
    HL_DIR_ENTRY_TYPE_FILE = 8,
    HL_DIR_ENTRY_TYPE_SYMBOLIC_LINK = 10,
    HL_DIR_ENTRY_TYPE_SOCKET = 12
}
HlDirEntryType;

typedef struct HlDirEntry
{
    /**
        @brief Pointer to this entry's name (e.g. the file or directory name).
    
        This pointer is only guaranteed to be valid until the next
        call to hlPathDirGetNextEntry. Please do not attempt to free
        this pointer; it will be automatically cleaned up with the
        directory handle.
    */
    const HlNChar* name;
    /** @brief See HlDirEntryType. */
    unsigned char type;
}
HlDirEntry;

HL_API const HlNChar* hlPathGetName(const HlNChar* path);
HL_API const HlNChar* hlPathGetExt(const HlNChar* path);
HL_API const HlNChar* hlPathGetExts(const HlNChar* path);

HL_API size_t hlPathRemoveExtNoAlloc(const HlNChar* HL_RESTRICT path,
    HlNChar* HL_RESTRICT result);

HL_API size_t hlPathRemoveExtsNoAlloc(const HlNChar* HL_RESTRICT path,
    HlNChar* HL_RESTRICT result);

HL_API HlNChar* hlPathRemoveExt(const HlNChar* path);
HL_API HlNChar* hlPathRemoveExts(const HlNChar* path);

HL_API size_t hlPathGetParentNoAlloc(const HlNChar* HL_RESTRICT path,
    HlNChar* HL_RESTRICT result);

HL_API HlNChar* hlPathGetParent(const HlNChar* path);

HL_API HlBool hlPathCombineNeedsSep(const HlNChar* HL_RESTRICT path1,
    const HlNChar* HL_RESTRICT path2, size_t path1Len);

HL_API size_t hlPathCombineNoAlloc(const HlNChar* HL_RESTRICT path1,
    const HlNChar* HL_RESTRICT path2, size_t path1Len, size_t path2Len,
    HlNChar* HL_RESTRICT result);

HL_API HlNChar* hlPathCombine(const HlNChar* HL_RESTRICT path1,
    const HlNChar* HL_RESTRICT path2, size_t path1Len, size_t path2Len);

HL_API size_t hlPathGetSize(const HlNChar* filePath);
HL_API HlBool hlPathIsDirectory(const HlNChar* path);
HL_API HlBool hlPathExists(const HlNChar* path);
HL_API HlResult hlPathCreateDirectory(const HlNChar* dirPath, HlBool overwrite);

HL_API HlResult hlPathDirOpen(const HlNChar* HL_RESTRICT dirPath,
    HlDirHandle HL_RESTRICT * HL_RESTRICT dir);

HL_API HlResult hlPathDirGetNextEntry(HlDirHandle HL_RESTRICT dir,
    HlDirEntry* HL_RESTRICT entry);

HL_API HlResult hlPathDirClose(HlDirHandle dir);

#ifdef __cplusplus
}
#endif
#endif
