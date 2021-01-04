/**
   @file hl_archive.h
   @brief The header which defines the concept of archives within HedgeLib.
*/
#ifndef HL_ARCHIVE_H_INCLUDED
#define HL_ARCHIVE_H_INCLUDED
#include "../hl_list.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
   @defgroup archives Archives
   @brief Functions and types related to archives.
*/

/*
   Apparently in C89 enums are of type int by default, so we can't have
   these as enum values since they won't fit within the size of an int
   on 64-bit x86, for example.
*/
#define HL_ARC_ENTRY_STREAMING_FLAG         HL_BIT_FLAG(HL_BIT_COUNT(size_t) - 1U)
#define HL_ARC_ENTRY_COMPRESSED_SIZE_MASK   ~(HL_ARC_ENTRY_STREAMING_FLAG)

#define HL_ARC_ENTRY_IS_DIR_FLAG            HL_BIT_FLAG(HL_BIT_COUNT(size_t) - 2U)

#define HL_ARC_ENTRY_NOT_OWNS_DATA_FLAG     HL_BIT_FLAG(HL_BIT_COUNT(size_t) - 3U)

/**
   @brief An entry structure representing a file or directory stored within an archive.
   @ingroup archives
*/
typedef struct HlArchiveEntry
{
    /**
       @brief The name of the file or directory represented by this entry, or the
       absolute path to the file if this entry is a file reference.
    */
    HlNChar* path;
    /**
       @brief The uncompressed size of the file if this entry represents a file, or
       the number of entries in the directory if this entry represents a directory.
    */
    size_t size;
    /**
       @brief Various metadata related to the entry, including flags
       specifying what type of entry this is.

       If HL_ARC_ENTRY_STREAMING_FLAG is set, this entry represents a file which has not
       yet been loaded into memory, and data is format-specific information which can be
       used to quickly locate/load the data at a later time. Use HL_ARC_ENTRY_COMPRESSED_SIZE_MASK
       on meta when the HL_ARC_ENTRY_STREAMING_FLAG is set to get the file's compressed
       size, or 0 if the data is not compressed.

       If HL_ARC_ENTRY_STREAMING_FLAG is not set, and HL_ARC_ENTRY_IS_DIR_FLAG is set, this
       entry represents a directory, and data is a pointer to an array of HlArchiveEntry
       structs which represent the contents of the directory.

       If HL_ARC_ENTRY_STREAMING_FLAG is not set, and HL_ARC_ENTRY_IS_DIR_FLAG is not set, this
       entry represents a "regular" file which is not being streamed-in from an archive, and data
       is either a pointer to the file's data, or NULL if this is a file reference, in which case,
       path is the absolute file path to said file on the user's machine.

       If HL_ARC_ENTRY_STREAMING_FLAG is not set, and HL_ARC_ENTRY_NOT_OWNS_DATA_FLAG is set, this
       entry's data pointer is not owned by this entry, and should not be freed with the entry.
    */
    size_t meta;
    /**
       @brief Data pertaining to the actual file or directory this entry represents.
       
       See documentation for the meta parameter for specifics on what this value represents.
    */
    HlUMax data;
}
HlArchiveEntry;

typedef HL_LIST(HlArchiveEntry) HlArchiveEntryList;

typedef struct HlArchive
{
    HlArchiveEntryList entries;
}
HlArchive;

typedef struct HlPackedFileEntry
{
    /** @brief The name this entry represents. */
    char* name;
    /** @brief The absolute position of the file within the packed data (e.g. within the .pfd). */
    HlU32 dataPos;
    /** @brief The size of the file within the packed data (e.g. within the .pfd). */
    HlU32 dataSize;
}
HlPackedFileEntry;

typedef struct HlPackedFileIndex
{
    HL_LIST(HlPackedFileEntry) entries;
}
HlPackedFileIndex;

HL_API size_t hlArchiveExtIsSplit(const HlNChar* ext);

/**
   @brief Creates an HlArchiveEntry which represents a file.
   @param[in] path      The name of the file, or the absolute path to the file if this is a reference.
   @param[in] size      The uncompressed size of the file, in bytes.
   @param[in] data      A pointer to the file's data, or NULL if this is a reference.
   @param[in] dontFree  Just pass in HL_FALSE unless you know what you're doing.
                        If set to HL_TRUE, this entry *won't* create its own copy of data,
                        meaning it will *not* automatically be freed when hlArchiveEntryDestruct
                        is called; you will have to free data yourself manually later on when it
                        will no longer be used. path is not affected by this argument and will be
                        automatically freed regardless.

   @return An HlArchiveEntry representing the given file.
   @ingroup archives
*/
HL_API HlArchiveEntry hlArchiveEntryMakeFile(const HlNChar* HL_RESTRICT path,
    size_t size, const void* HL_RESTRICT data, HlBool dontFree);

/**
   @brief Creates an HlArchiveEntry which represents a directory.
   @param[in] name  The name of the directory.

   @return An HlArchiveEntry representing the given directory.
   @ingroup archives
*/
HL_API HlArchiveEntry hlArchiveEntryMakeDir(const HlNChar* name);

/**
   @brief Modifies the data within an HlArchiveEntry which represents a file.
   @param[in] entry     The file entry to be modified.
   @param[in] size      The new uncompressed size of the file, in bytes.
   @param[in] data      A pointer to the file's data, or NULL if this is a reference.
   @param[in] dontFree  Just pass in HL_FALSE unless you know what you're doing.
                        If set to HL_TRUE, this entry *won't* create its own copy of data,
                        meaning it will *not* automatically be freed when hlArchiveEntryDestruct
                        is called; you will have to free data yourself manually later on when it
                        will no longer be used. path is not affected by this argument and will be
                        automatically freed regardless.

   @return An HlResult indicating whether data buffer allocation succeeded or not.
   @ingroup archives
*/
HL_API HlResult hlArchiveEntryFileSetData(HlArchiveEntry* HL_RESTRICT entry,
    size_t size, const void* HL_RESTRICT data, HlBool dontFree);

/**
   @brief Extracts all the files, and optionally, the directories in the given array of entries.
   @param[in] entries       The array of HlArchiveEntry structs to extract.
   @param[in] entryCount    The number of HlArchiveEntry structs in the entries array to extract.
   @param[in] dirPath       Path to the directory the given files/directories should be extracted to.
   @param[in] recursive     Whether or not the entries should be extracted recursively.
                            Set this to HL_FALSE to only extract files from the root directory.
   
   @return An HlResult indicating whether extraction succeeded or not.
   @ingroup archives
*/
HL_API HlResult hlArchiveEntriesExtract(const HlArchiveEntry* HL_RESTRICT entries,
    size_t entryCount, const HlNChar* HL_RESTRICT dirPath, HlBool recursive);

HL_API HlResult hlArchiveConstruct(HlArchive* arc);
HL_API HlResult hlArchiveCreate(HlArchive** arc);

HL_API HlResult hlArchiveAddDir(const HlNChar* HL_RESTRICT dirPath,
    HlBool loadData, HlBool recursive, HlArchive* HL_RESTRICT arc);

HL_API HlResult hlArchiveCreateFromDir(const HlNChar* HL_RESTRICT dirPath,
    HlBool loadData, HlBool recursive, HlArchive* HL_RESTRICT * HL_RESTRICT arc);

#define hlArchiveEntryIsStreaming(entry) (HlBool)(\
    ((entry)->meta & HL_ARC_ENTRY_STREAMING_FLAG) != 0)

#define hlArchiveEntryIsDir(entry)\
    (HlBool)(!hlArchiveEntryIsStreaming(entry) &&\
    ((entry)->meta & HL_ARC_ENTRY_IS_DIR_FLAG) != 0)

#define hlArchiveEntryIsFile(entry)\
    (HlBool)(hlArchiveEntryIsStreaming(entry) ||\
    ((entry)->meta & HL_ARC_ENTRY_IS_DIR_FLAG) == 0)

#define hlArchiveEntryIsRegularFile(entry)\
    (HlBool)(!hlArchiveEntryIsStreaming(entry) &&\
    ((entry)->meta & HL_ARC_ENTRY_IS_DIR_FLAG) == 0)

#define hlArchiveEntryGetCompressedSize(entry)\
    (size_t)(hlArchiveEntryIsStreaming(entry) ?\
    ((entry)->meta & HL_ARC_ENTRY_COMPRESSED_SIZE_MASK) : 0)

#define hlArchiveEntryIsReference(entry)\
    (HlBool)(!hlArchiveEntryIsStreaming(entry) &&\
    ((entry)->meta & HL_ARC_ENTRY_IS_DIR_FLAG) == 0 &&\
    (entry)->data == 0)

HL_API const HlNChar* hlArchiveEntryGetName(const HlArchiveEntry* entry);

#define hlArchiveExtract(arc, dirPath, recursive) hlArchiveEntriesExtract(\
    (arc)->entries.data, (arc)->entries.count, dirPath, recursive)

HL_API void hlArchiveEntryDestruct(HlArchiveEntry* entry);
HL_API void hlArchiveDestruct(HlArchive* arc);
HL_API void hlArchiveFree(HlArchive* arc);

HL_API HlResult hlPackedFileIndexConstruct(HlPackedFileIndex* pfi);
HL_API HlResult hlPackedFileIndexCreate(HlPackedFileIndex** pfi);
HL_API void hlPackedFileEntryDestruct(HlPackedFileEntry* entry);
HL_API void hlPackedFileIndexDestruct(HlPackedFileIndex* pfi);
HL_API void hlPackedFileIndexDestroy(HlPackedFileIndex* pfi);

#ifndef HL_NO_EXTERNAL_WRAPPERS
HL_API HlBool hlArchiveEntryIsStreamingExt(const HlArchiveEntry* entry);
HL_API HlBool hlArchiveEntryIsDirExt(const HlArchiveEntry* entry);
HL_API HlBool hlArchiveEntryIsFileExt(const HlArchiveEntry* entry);
HL_API HlBool hlArchiveEntryIsRegularFileExt(const HlArchiveEntry* entry);
HL_API size_t hlArchiveEntryGetCompressedSizeExt(const HlArchiveEntry* entry);
HL_API HlBool hlArchiveEntryIsReferenceExt(const HlArchiveEntry* entry);

HL_API HlResult hlArchiveExtractExt(const HlArchive* HL_RESTRICT arc,
    const HlNChar* HL_RESTRICT dirPath, HlBool recursive);
#endif

#ifdef __cplusplus
}
#endif
#endif
