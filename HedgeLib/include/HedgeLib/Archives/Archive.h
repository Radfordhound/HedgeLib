#pragma once
#include "../String.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @file Archive.h
 *  @brief The header used by generic archives.
*/

/*! @defgroup archives Archives
 *  @brief Functions and types related to archives.
*/

typedef struct hl_Blob hl_Blob;

typedef enum HL_ARCHIVE_TYPE
{
    // These values were picked such that you can safely do
    // (type & HL_ARC_FORMAT_ONE) or (type & HL_ARC_FORMAT_PACX)
    HL_ARC_TYPE_UNKNOWN = 0,
    HL_ARC_FORMAT_ONE = 1,          // .one files
    HL_ARC_FORMAT_PACX = 2,         // .pac files

    HL_ARC_TYPE_HEROES = 17,        // .one files from Heroes/Shadow the Hedgehog
    HL_ARC_TYPE_S06 = 16,           // .arc files from Sonic '06
    HL_ARC_TYPE_STORYBOOK = 33,     // .one files from Secret Rings/Black Knight
    HL_ARC_TYPE_GENS = 32,          // .ar/.pfd files from Unleashed/Generations
    HL_ARC_TYPE_COLORS = 48,        // .arc files from Sonic Colors
    HL_ARC_TYPE_PACX_V2 = 18,       // .pac files from Lost World
    HL_ARC_TYPE_PACX_V3 = 34,       // .pac files from Forces
    HL_ARC_TYPE_TSR = 64,           // .cpu.sp2/.gpu.sp2 files from TSR
    HL_ARC_TYPE_PACX_V4 = 50        // .pac files from Tokyo 2020
}
HL_ARCHIVE_TYPE;

/*! @brief A file in an hl_Archive.
 *  @ingroup archives
*/
typedef struct hl_ArchiveFileEntry
{
    /*! @brief The size of the data, or 0 if Data points to a file path. */
    size_t Size;
    /*! @brief The name of the file + its extension. Does NOT include the path!! */
    const char* Name;
    /*! @brief Points to the file path if Size == 0, or the file's data if Size != 0. */
    const void* Data;
}
hl_ArchiveFileEntry;

/*! @brief An archive.
 *  @ingroup archives
*/
typedef struct hl_Archive
{
    hl_ArchiveFileEntry* Files;
    size_t FileCount;
    // TODO: Add directories too if any archiving formats require that
}
hl_Archive;

HL_API HL_ARCHIVE_TYPE hl_DArchiveBlobGetType(const hl_Blob* blob);

/*! @brief Returns whether the given extension is that of a "split".
 *
 *  This function returns whether the given extension is valid (not null) and is that
 *  of a "split" archive (a file which makes up part of an archive's total data; e.g. "Sonic.pac.00").
 *  E.G. ".00" and ".05" are "split" extensions, whereas ".pac" is not.
 *
 *  @param[in] ext The extension to check.
 *  @return `true` if the given extension is valid (not null) and is that of a "split" archive, otherwise `false`.
 *  @sa @ref hl_ArchiveIsSplitExtNative
 *  @sa @ref hl_ArchiveIsSplit
 *  @ingroup archives
*/
HL_API bool hl_ArchiveIsSplitExt(const char* ext);

/*! @brief Returns whether the given extension is that of a "split".
 *
 *  This function returns whether the given extension is valid (not null) and is that
 *  of a "split" archive (a file which makes up part of an archive's total data; e.g. "Sonic.pac.00").
 *  E.G. ".00" and ".05" are "split" extensions, whereas ".pac" is not.
 *
 *  @remark This is the native variant of the hl_ArchiveIsSplitExt function.
 *
 *  @param[in] ext The extension to check.
 *  @return `true` if the given extension is valid (not null) and is that of a "split" archive, otherwise `false`.
 *  @sa @ref hl_ArchiveIsSplitExt
 *  @sa @ref hl_ArchiveIsSplitNative
 *  @ingroup archives
*/
HL_API bool hl_ArchiveIsSplitExtNative(const hl_NativeChar* ext);

/*! @brief Returns whether the file at the given path is a "split".
 *
 *  This function returns whether the given file path is valid (not null) and has a
 *  "split" extension (e.g. "Sonic.pac.00"). It does not check if the file actually exists.
 *
 *  @remark This is the native variant of the hl_ArchiveIsSplit function.
 *
 *  @param[in] filePath The file path to check.
 *  @return `true` if the given file path is valid (not null) and has a "split" extension, otherwise `false`.
 *  @sa @ref hl_ArchiveIsSplitNative
 *  @sa @ref hl_ArchiveIsSplitExt
 *  @ingroup archives
*/
HL_API bool hl_ArchiveIsSplit(const char* filePath);

/*! @brief Returns whether the file at the given path is a "split".
 *
 *  This function returns whether the given file path is valid (not null) and has a
 *  "split" extension (e.g. "Sonic.pac.00"). It does not check if the file actually exists.
 *
 *  @remark This is the native variant of the hl_ArchiveIsSplit function.
 *
 *  @param[in] filePath The file path to check.
 *  @return `true` if the given file path is valid (not null) and has a "split" extension, otherwise `false`.
 *  @sa @ref hl_ArchiveIsSplit
 *  @sa @ref hl_ArchiveIsSplitExtNative
 *  @ingroup archives
*/
HL_API bool hl_ArchiveIsSplitNative(const hl_NativeChar* filePath);

/*! @brief Returns the type of archive that corresponds to the given extension.
 *
 *  This function returns the type of archive that corresponds to the given extension.
 *
 *  @param[in] ext The extension to check.
 *  @return The [archive type](@ref HL_ARCHIVE_TYPE that corresponds to the given extension, or
 *  HL_ARC_TYPE_UNKNOWN if the given extension is null or not known.
 *  @sa @ref hl_ArchiveGetTypeExtNative
 *  @sa @ref hl_ArchiveGetType
 *  @ingroup archives
*/
HL_API HL_ARCHIVE_TYPE hl_ArchiveGetTypeExt(const char* ext);

/*! @brief Returns the type of archive that corresponds to the given extension.
 *
 *  This function returns the type of archive that corresponds to the given extension.
 *
 *  @remark This is the native variant of the hl_ArchiveGetTypeExt function.
 *
 *  @param[in] ext The extension to check.
 *  @return The [archive type](@ref HL_ARCHIVE_TYPE that corresponds to the given extension, or
 *  HL_ARC_TYPE_UNKNOWN if the given extension is null or not known.
 *  @sa @ref hl_ArchiveGetTypeExt
 *  @sa @ref hl_ArchiveGetTypeNative
 *  @ingroup archives
*/
HL_API HL_ARCHIVE_TYPE hl_ArchiveGetTypeExtNative(const hl_NativeChar* ext);

/*! @brief Gets the type of archive at the given path.
 *
 *  This function retrives the type of archive at the given path based on its extension, and
 *  whether the given archive is a "split" or not. It does not check the contents of the file
 *  or if the file actually exists.
 *
 * @param[in] filePath The file path to check.
 * @param[out] type The [archive type](@ref HL_ARCHIVE_TYPE) of the given file, or
 * HL_ARC_TYPE_UNKNOWN if the given file path is null or its extension is invalid/not known.
 * @return Whether the given file path and type pointer are valid (not null) and
 * the given file path is that of a "split" archive or not.
 * @sa @ref hl_ArchiveGetTypeNative
 * @sa @ref hl_ArchiveGetTypeExt
 * @ingroup archives
*/
HL_API bool hl_ArchiveGetType(const char* filePath, HL_ARCHIVE_TYPE* type);

/*! @brief Gets the type of archive at the given path.
 *
 *  This function retrives the type of archive at the given path based on its extension, and
 *  whether the given archive is a "split" or not. It does not check the contents of the file
 *  or if the file actually exists.
 *
 *  @remark This is the native variant of the hl_ArchiveGetType function.
 *
 * @param[in] filePath The file path to check.
 * @param[out] type The [archive type](@ref HL_ARCHIVE_TYPE) of the given file, or
 * HL_ARC_TYPE_UNKNOWN if the given file path is null or its extension is invalid/not known.
 * @return Whether the given file path and type pointer are valid (not null) and
 * the given file path is that of a "split" archive or not.
 * @sa @ref hl_ArchiveGetType
 * @sa @ref hl_ArchiveGetTypeExtNative
 * @ingroup archives
*/
HL_API bool hl_ArchiveGetTypeNative(const hl_NativeChar* filePath, HL_ARCHIVE_TYPE* type);

HL_API HL_RESULT hl_ArchiveGetRootPath(const char* splitPath, char** rootPath);
HL_API HL_RESULT hl_ArchiveGetRootPathNative(
    const hl_NativeChar* splitPath, hl_NativeChar** rootPath);

HL_API size_t hl_DArchiveGetFileCount(const hl_Blob* blob,
    bool HL_DEFARG(includeProxies, true));

HL_API HL_RESULT hl_CreateArchive(const char* dir, hl_Archive** arc);
HL_API HL_RESULT hl_CreateArchiveNative(const hl_NativeChar* dir, hl_Archive** arc);

HL_API HL_RESULT hl_DCreateArchive(const hl_Blob** blobs,
    size_t blobCount, hl_Archive** arc);

HL_API HL_RESULT hl_DLoadArchiveOfType(const char* filePath,
    HL_ARCHIVE_TYPE type, hl_Blob** blob);

HL_API HL_RESULT hl_DLoadArchiveOfTypeNative(const hl_NativeChar* filePath,
    HL_ARCHIVE_TYPE type, hl_Blob** blob);

HL_API HL_RESULT hl_DLoadArchive(const char* filePath, hl_Blob** blob);
HL_API HL_RESULT hl_DLoadArchiveNative(const hl_NativeChar* filePath, hl_Blob** blob);
HL_API HL_RESULT hl_DLoadRootArchive(const char* filePath, hl_Blob** blob);
HL_API HL_RESULT hl_DLoadRootArchiveNative(const hl_NativeChar* filePath, hl_Blob** blob);

HL_API HL_RESULT hl_LoadArchive(const char* filePath, hl_Archive** arc);
HL_API HL_RESULT hl_LoadArchiveNative(const hl_NativeChar* filePath, hl_Archive** arc);

HL_API size_t hl_ArchiveGetSplitCountRoot(const char* rootPath);
HL_API size_t hl_ArchiveGetSplitCountRootNative(const hl_NativeChar* rootPath);
HL_API size_t hl_ArchiveGetSplitCount(const char* filePath);
HL_API size_t hl_ArchiveGetSplitCountNative(const hl_NativeChar* filePath);

HL_API HL_RESULT hl_DExtractArchive(const hl_Blob* blob, const char* dir);
HL_API HL_RESULT hl_DExtractArchiveNative(
    const hl_Blob* blob, const hl_NativeChar* dir);

HL_API HL_RESULT hl_ExtractArchive(const hl_Archive* arc, const char* dir);
HL_API HL_RESULT hl_ExtractArchiveNative(
    const hl_Archive* arc, const hl_NativeChar* dir);

HL_API HL_RESULT hl_ExtractArchivesOfType(const char* filePath,
    const char* dir, HL_ARCHIVE_TYPE type);

HL_API HL_RESULT hl_ExtractArchivesOfTypeNative(const hl_NativeChar* filePath,
    const hl_NativeChar* dir, HL_ARCHIVE_TYPE type);

HL_API HL_RESULT hl_ExtractArchives(const char* filePath, const char* dir);
HL_API HL_RESULT hl_ExtractArchivesNative(
    const hl_NativeChar* filePath, const hl_NativeChar* dir);

HL_API hl_ArchiveFileEntry hl_CreateArchiveFileEntry(const char* filePath);

inline hl_ArchiveFileEntry hl_CreateArchiveFileEntryData(size_t dataSize,
    const char* name, const void* data)
{
    return
    {
        dataSize, name, data
    };
}

HL_API hl_ArchiveFileEntry* hl_CreateArchiveFileEntries(
    const char** files, size_t fileCount);

#ifdef __cplusplus
}

// Windows-specific overloads
#ifdef _WIN32

inline bool hl_ArchiveIsSplitExt(const hl_NativeChar* ext)
{
    return hl_ArchiveIsSplitExtNative(ext);
}

inline bool hl_ArchiveIsSplit(const hl_NativeChar* filePath)
{
    return hl_ArchiveIsSplitNative(filePath);
}

inline HL_ARCHIVE_TYPE hl_ArchiveGetTypeExt(const hl_NativeChar* ext)
{
    return hl_ArchiveGetTypeExtNative(ext);
}

inline bool hl_ArchiveGetType(const hl_NativeChar* filePath, HL_ARCHIVE_TYPE* type)
{
    return hl_ArchiveGetTypeNative(filePath, type);
}

inline HL_RESULT hl_ArchiveGetRootPath(
    const hl_NativeChar* splitPath, hl_NativeChar** rootPath)
{
    return hl_ArchiveGetRootPathNative(splitPath, rootPath);
}

inline HL_RESULT hl_CreateArchive(const hl_NativeChar* dir, hl_Archive** arc)
{
    return hl_CreateArchiveNative(dir, arc);
}

inline HL_RESULT hl_DLoadArchiveOfType(const hl_NativeChar* filePath,
    HL_ARCHIVE_TYPE type, hl_Blob** blob)
{
    return hl_DLoadArchiveOfTypeNative(filePath, type, blob);
}

inline HL_RESULT hl_DLoadArchive(const hl_NativeChar* filePath, hl_Blob** blob)
{
    return hl_DLoadArchiveNative(filePath, blob);
}

inline HL_RESULT hl_DLoadRootArchive(const hl_NativeChar* filePath, hl_Blob** blob)
{
    return hl_DLoadRootArchiveNative(filePath, blob);
}

inline HL_RESULT hl_LoadArchive(const hl_NativeChar* filePath, hl_Archive** arc)
{
    return hl_LoadArchiveNative(filePath, arc);
}

inline size_t hl_ArchiveGetSplitCountRoot(const hl_NativeChar* rootPath)
{
    return hl_ArchiveGetSplitCountRootNative(rootPath);
}

inline size_t hl_ArchiveGetSplitCount(const hl_NativeChar* filePath)
{
    return hl_ArchiveGetSplitCountNative(filePath);
}

inline HL_RESULT hl_DExtractArchive(
    const hl_Blob* blob, const hl_NativeChar* dir)
{
    return hl_DExtractArchiveNative(blob, dir);
}

inline HL_RESULT hl_ExtractArchive(
    const hl_Archive* arc, const hl_NativeChar* dir)
{
    return hl_ExtractArchiveNative(arc, dir);
}

inline HL_RESULT hl_ExtractArchivesOfType(const hl_NativeChar* filePath,
    const hl_NativeChar* dir, HL_ARCHIVE_TYPE type)
{
    return hl_ExtractArchivesOfTypeNative(filePath, dir, type);
}

inline HL_RESULT hl_ExtractArchives(
    const hl_NativeChar* filePath, const hl_NativeChar* dir)
{
    return hl_ExtractArchivesNative(filePath, dir);
}
#endif
#endif
