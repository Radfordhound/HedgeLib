/**
   @file hl_archive.h
   @brief The header which defines the concept of archives within HedgeLib.
*/
#ifndef HL_ARCHIVE_H_INCLUDED
#define HL_ARCHIVE_H_INCLUDED
#include "../hl_internal.h"

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

/**
   @brief Creates an HlArchiveEntry in-place which represents a file.
   @param[in] path  The name of the file, or the absolute path to the file if this is a reference.
   @param[in] size  The uncompressed size of the file.
   @param[in] data  Pointer to the file's data, or NULL if this is a reference.
   @return An HlArchiveEntry representing the given file.
   @ingroup archives
*/
#define HL_ARC_ENTRY_MAKE_FILE(path, size, data)\
    { path, size, 0, (HlUMax)(data) }

/**
   @brief Creates an HlArchiveEntry in-place which represents a directory.
   @param[in] name          The name of the directory.
   @param[in] childCount    The number of "child" entries contained within the directory.
   @param[in] children      Pointer to an array of HlArchiveEntry structs representing
                            the "child" entries contained within the directory.

   @return An HlArchiveEntry representing the given file.
   @ingroup archives
*/
#define HL_ARC_ENTRY_MAKE_DIR(name, childCount, children)\
    { name, childCount, HL_ARC_ENTRY_IS_DIR_FLAG, (HlUMax)(children) }

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
    const HlNChar* path;
    /**
       @brief The uncompressed size of the file if this entry represents a file, or
       the number of entries in the directory if this entry represents a directory.
    */
    size_t size;
    /**
       @brief Various metadata related to the entry, including flags
       specifying what type of entry this is.

       If HL_ARC_ENTRY_STREAMING_FLAG is set, this entry represents a file which has not
       yet been loaded into memory, and data is a size_t which represents the location of
       the file's (compressed) data within the archive. Use HL_ARC_ENTRY_COMPRESSED_SIZE_MASK
       on meta when the HL_ARC_ENTRY_STREAMING_FLAG is set to get the file's compressed
       size, or 0 if the data is not compressed.

       If HL_ARC_ENTRY_STREAMING_FLAG is not set, and HL_ARC_ENTRY_IS_DIR_FLAG is set, this
       entry represents a directory, and data is a pointer to an array of HlArchiveEntry
       structs which represent the contents of the directory.

       If HL_ARC_ENTRY_STREAMING_FLAG is not set, and HL_ARC_ENTRY_IS_DIR_FLAG is not set, this
       entry represents a "normal" file which is not being streamed-in from an archive, and data
       is either a pointer to the file's data, or NULL if this is a file reference, in which case,
       path is the absolute file path to said file on the user's machine.
    */
    size_t meta;
    /**
       @brief Data pertaining to the actual file or directory this entry represents.
       
       See documentation for the meta parameter for specifics on what this value represents.
    */
    HlUMax data;
}
HlArchiveEntry;

typedef struct HlArchive
{
    const HlArchiveEntry* entries;
    size_t entryCount;
}
HlArchive;

HL_API size_t hlArchiveExtIsSplit(const HlNChar* ext);

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

HL_API HlResult hlArchiveCreateFromDir(const HlNChar* HL_RESTRICT dirPath,
    HlArchive** HL_RESTRICT arc);

#define hlArchiveEntryIsStreaming(entry) (HlBool)((entry)->meta & HL_ARC_ENTRY_STREAMING_FLAG)

#define hlArchiveEntryIsDir(entry) (HlBool)(!hlArchiveEntryIsStreaming(entry) &&\
    (entry)->meta & HL_ARC_ENTRY_IS_DIR_FLAG)

#define hlArchiveEntryIsFile(entry)\
    (HlBool)(hlArchiveEntryIsStreaming(entry) ||\
    ((entry)->meta & HL_ARC_ENTRY_IS_DIR_FLAG) == 0)

#define hlArchiveEntryGetCompressedSize(entry)\
    (size_t)(hlArchiveEntryIsStreaming(entry) ?\
    ((entry)->meta & HL_ARC_ENTRY_COMPRESSED_SIZE_MASK) : 0)

#define hlArchiveEntryIsReference(entry)\
    (HlBool)(!hlArchiveEntryIsStreaming(entry) &&\
    ((entry)->meta & HL_ARC_ENTRY_IS_DIR_FLAG) == 0 &&\
    !(entry)->data)

#define hlArchiveExtract(arc, dirPath, recursive) hlArchiveEntriesExtract(\
    (arc)->entries, (arc)->entryCount, dirPath, recursive)

#ifndef HL_NO_EXTERNAL_WRAPPERS
HL_API HlBool hlArchiveEntryIsStreamingExt(const HlArchiveEntry* entry);
HL_API HlBool hlArchiveEntryIsDirExt(const HlArchiveEntry* entry);
HL_API HlBool hlArchiveEntryIsFileExt(const HlArchiveEntry* entry);
HL_API size_t hlArchiveEntryGetCompressedSizeExt(const HlArchiveEntry* entry);
HL_API HlBool hlArchiveEntryIsReferenceExt(const HlArchiveEntry* entry);

HL_API HlResult hlArchiveExtractExt(const HlArchive* HL_RESTRICT arc,
    const HlNChar* HL_RESTRICT dirPath, HlBool recursive);
#endif

#ifdef __cplusplus
}
#endif
#endif
