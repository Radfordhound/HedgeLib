#include "../hl_in_assert.h"
#include "../io/hl_in_path.h"
#include "hedgelib/hl_text.h"
#include "hedgelib/io/hl_path.h"
#include "hedgelib/io/hl_file.h"
#include "hedgelib/archives/hl_archive.h"
#include <string.h>

size_t hlArchiveExtIsSplit(const HlNChar* ext)
{
    size_t numSplitChars = 0;

    /* Account for dot at beginning of extension. */
    if (*ext == HL_NTEXT('.')) ++ext;

    /* Ensure there is at least one digit in the extension. */
    if (!HL_IS_DIGIT(*ext)) return 0;

    /* Loop through the remaining characters in the extension. */
    while (1)
    {
        /* Increment numSplitChars. */
        ++numSplitChars;

        /* Return if we encounter a character which isn't a valid ASCII digit. */
        if (!HL_IS_DIGIT(ext[numSplitChars]))
        {
            /*
               If this non-digit character is the null-terminator, this
               is a valid split extension. Otherwise, it isn't.
            */
            return (ext[numSplitChars] == HL_NTEXT('\0')) ?
                numSplitChars : 0;
        }
    }
}

HlArchiveEntry hlArchiveEntryMakeFile(const HlNChar* HL_RESTRICT path,
    size_t size, const void* HL_RESTRICT data, HlBool dontFree)
{
    /* Get path buffer length. */
    const size_t pathBufLen = (hlNStrLen(path) + 1);
    HlArchiveEntry entry;
    HlResult result;

    /* Allocate new path buffer. */
    entry.path = HL_ALLOC_ARR(HlNChar, pathBufLen);
    
    /* Ensure path buffer allocation didn't fail. */
    HL_ASSERT(entry.path != NULL);

    /* Copy the given name into our new path buffer. */
    memcpy(entry.path, path, pathBufLen);

    /* Setup data and return the entry. */
    result = hlArchiveEntryFileSetData(&entry, size, data, dontFree);
    HL_ASSERT(HL_OK(result));

    return entry;
}

HlArchiveEntry hlArchiveEntryMakeDir(const HlNChar* name)
{
    /* Get path buffer length. */
    const size_t pathBufLen = (hlNStrLen(name) + 1);

    /* Setup entry. */
    HlArchiveEntry entry =
    {
        HL_ALLOC_ARR(HlNChar, pathBufLen),      /* path */
        0,                                      /* size */
        HL_ARC_ENTRY_IS_DIR_FLAG,               /* meta */
        (HlUMax)((HlUPtr)NULL)                  /* data */
    };
    
    /* Ensure path buffer allocation didn't fail. */
    HL_ASSERT(entry.path != NULL);

    /* Copy the given name into our new path buffer and return the entry. */
    memcpy(entry.path, name, pathBufLen);
    return entry;
}

HlResult hlArchiveEntryFileSetData(HlArchiveEntry* HL_RESTRICT entry,
    size_t size, const void* HL_RESTRICT data, HlBool dontFree)
{
    /* Set new entry data size. */
    entry->size = size;

    if (dontFree)
    {
        /* Setup reference to data that is not owned by this entry. */
        entry->meta = HL_ARC_ENTRY_NOT_OWNS_DATA_FLAG;
        entry->data = (HlUMax)((HlUPtr)data);
    }
    else
    {
        /* Allocate new buffer that is owned by this entry. */
        void* newDataBuf = hlAlloc(size);
        if (!newDataBuf) return HL_ERROR_OUT_OF_MEMORY;

        /* Copy the given data into newly-allocated data buffer. */
        memcpy(newDataBuf, data, size);

        /* Setup entry. */
        entry->meta = 0;
        entry->data = (HlUMax)((HlUPtr)newDataBuf);
    }

    return HL_RESULT_SUCCESS;
}

#define HL_INARC_PATH_BUF_LEN 255

HlBool hlINArchiveNextSplit2(HlNChar* lastCharPtr)
{
    /*
       Increment the last character in the path and check if it's > 9.
       
       Before           After           > 9?
       Sonic.ar.00      Sonic.ar.01     No
       Sonic.ar.09      Sonic.ar.0:     Yes
       Sonic.ar.99      Sonic.ar.9:     Yes
    */
    if (++(*lastCharPtr) > HL_NTEXT('9'))
    {
        /*
           Increment the second-to-last character in the path and check if it's > 9.

           Before           After           > 9?
           Sonic.ar.0:      Sonic.ar.1:     No
           Sonic.ar.9:      Sonic.ar.::     Yes
        */
        if (++(*(lastCharPtr - 1)) > HL_NTEXT('9'))
        {
            /* We've gone over split .99 - there is no next split; return false. */
            return HL_FALSE;
        }
        else
        {
            /*
               We haven't gone over split 99 yet; reset last character in path.

               Before           After
               Sonic.ar.1:      Sonic.ar.10
            */
            *lastCharPtr = HL_NTEXT('0');
        }
    }

    return HL_TRUE;
}

static HlNChar* hlINArchiveEnlargePathBuffer(size_t minAmount,
    HlNChar* HL_RESTRICT pathBuf, size_t* HL_RESTRICT pathBufLen)
{
    size_t pathBufEnlargeAmount;

    /* Ensure path buffer can still be enlargened. */
    if ((sizeof(HlNChar) * (*pathBufLen)) > (HL_SIZE_MAX -
        (sizeof(HlNChar) * minAmount)))
    {
        return NULL;
    }

    /*
       Get amount to enlarge the path buffer by.

       We want to enlarge the path buffer by a decent amount to lower
       the amount of necessary subsequent re-allocations, therefore we
       enlarge it by whichever is larger: 255 or minAmount.

       If 255 is larger than minAmount, we also have to ensure, once again,
       that the path buffer can still be enlargened - this time by 255.
    */

    pathBufEnlargeAmount = 255;
    if (pathBufEnlargeAmount < minAmount ||
        (sizeof(HlNChar) * (*pathBufLen)) > (HL_SIZE_MAX -
        (sizeof(HlNChar) * pathBufEnlargeAmount)))
    {
        pathBufEnlargeAmount = minAmount;
    }

    /* Enlarge path buffer. */
    *pathBufLen += pathBufEnlargeAmount;
    pathBuf = HL_RESIZE_ARR(HlNChar, *pathBufLen, pathBuf);
    if (!pathBuf) return NULL;
    
    return pathBuf;
}

static HlResult hlINArchiveEntriesExtract(const HlArchiveEntry* HL_RESTRICT entries,
    size_t entryCount, HlBool recursive, HlNChar* HL_RESTRICT * HL_RESTRICT pathBuf,
    size_t* HL_RESTRICT pathBufLen, size_t pathEndPos)
{
    /* Extract entries. */
    size_t i;
    HlResult result;

    for (i = 0; i < entryCount; ++i)
    {
        const HlNChar* entryName;
        size_t entryNameLen;

        /* Skip streaming file entries. */
        if (hlArchiveEntryIsStreaming(&entries[i]))
            continue;

        /* Get archive entry name. */
        if (hlArchiveEntryIsReference(&entries[i]))
        {
            /*
               This entry is a "reference" to a file on the user's
               machine, and path is the absolute path to the file;
               get the filename from the path.
            */
            entryName = hlPathGetName(entries[i].path);
        }
        else
        {
            /* path is just the entry's name. */
            entryName = entries[i].path;

            /* Ensure entryName doesn't begin with a path separator. */
            if (!hlINPathCombineNeedsSep2(entryName)) ++entryName;
        }
        
        /* Append archive entry name to end of path buffer. */
        {
            HlNChar* pathBufEnd = (*pathBuf + pathEndPos);
            size_t pathBufFreeLen = (*pathBufLen - pathEndPos);
            
            if (!hlNStrCopyLimit(entryName, pathBufEnd,
                pathBufFreeLen, &entryNameLen))
            {
                /*
                   The archive entry name won't fit within the path buffer...
                   We need to resize the path buffer to make it fit.
                */

                /*
                   Get entry name length.
                   (hlNStrCopyLimit doesn't set this for us if it fails.)
                */
                entryNameLen = hlNStrLen(entryName);

                /* Enlarge path buffer. */
                *pathBuf = hlINArchiveEnlargePathBuffer(entryNameLen + 1, *pathBuf, pathBufLen);
                if (!(*pathBuf)) return HL_ERROR_OUT_OF_MEMORY;

                /* Get pathBufEnd and pathBufFreeLen again since pathBuf may have changed. */
                pathBufEnd = (*pathBuf + pathEndPos);
                pathBufFreeLen = (*pathBufLen - pathEndPos);

                /* Append archive entry name to end of path buffer. */
                memcpy(pathBufEnd, entryName, entryNameLen + 1);
            }
        }

        /* Extract entry. */
        if (hlArchiveEntryIsFile(&entries[i])) /* File */
        {
            /* Copy referenced file. */
            if (hlArchiveEntryIsReference(&entries[i]))
            {
                /* TODO: Copy referenced file? Or should we just skip file references? */
                HL_ASSERT(0);
            }

            /* Extract file. */
            else
            {
                /* Open the file. */
                HlFile* file;
                result = hlFileOpen(*pathBuf, HL_FILE_MODE_WRITE, &file);
                if (HL_FAILED(result)) return result;

                /* Write data to the file. */
                result = hlFileWrite(file, entries[i].size,
                    (void*)((HlUPtr)entries[i].data), NULL);

                if (HL_FAILED(result))
                {
                    hlFileClose(file);
                    return result;
                }

                /* Close the file, then return if any errors were encountered. */
                result = hlFileClose(file);
                if (HL_FAILED(result)) return result;
            }
        }
        else if (recursive) /* Directory - only parse if we're allowed to recurse. */
        {
            size_t fullPathLen;

            /* Create directory - overwriting if directory already exists. */
            result = hlPathCreateDirectory(*pathBuf, HL_TRUE);
            if (HL_FAILED(result)) return result;

            /* Append path combine separator if necessary. */
            fullPathLen = (pathEndPos + entryNameLen);
            if (hlINPathCombineNeedsSep1(*pathBuf, fullPathLen))
            {
                /* Enlarge path buffer if necessary to append path combine separator. */
                if ((fullPathLen + 2) > *pathBufLen)
                {
                    /* Enlarge path buffer. */
                    *pathBuf = hlINArchiveEnlargePathBuffer(1, *pathBuf, pathBufLen);
                    if (!(*pathBuf)) return HL_ERROR_OUT_OF_MEMORY;
                }

                /* Append path combine separator (and new null terminator). */
                *pathBuf[fullPathLen++] = HL_PATH_SEP;
                *pathBuf[fullPathLen] = HL_NTEXT('\0');
            }

            /* Recursively extract archive entries within this directory. */
            result = hlINArchiveEntriesExtract(
                (HlArchiveEntry*)((HlUPtr)entries[i].data), entries[i].size,
                recursive, pathBuf, pathBufLen, fullPathLen);

            if (HL_FAILED(result)) return result;
        }
    }

    return HL_RESULT_SUCCESS;
}

HlResult hlArchiveEntriesExtract(const HlArchiveEntry* HL_RESTRICT entries,
    size_t entryCount, const HlNChar* HL_RESTRICT dirPath, HlBool recursive)
{
    HlNChar* pathBuf;
    size_t dirPathLen, pathBufLen;
    HlResult result;

    /* Return early if entryCount == 0 to avoid unnecessary work. */
    if (!entryCount) return HL_RESULT_SUCCESS;

    /* Ensure extraction directory exists. */
    result = hlPathCreateDirectory(dirPath, HL_TRUE);
    if (HL_FAILED(result)) return result;

    /* Get dirPath length. */
    dirPathLen = hlNStrLen(dirPath);

    /* Ensure we can safely create a path buffer without integral overflow. */
    HL_ASSERT((HL_SIZE_MAX - (sizeof(HlNChar) * HL_INARC_PATH_BUF_LEN)) >=
        (sizeof(HlNChar) * dirPathLen));

    /* Compute path buffer length. */
    pathBufLen = (dirPathLen + 255);

    /* Create path buffer and copy directory into it. */
    pathBuf = HL_ALLOC_ARR(HlNChar, pathBufLen);
    if (!pathBuf) return HL_ERROR_OUT_OF_MEMORY;

    memcpy(pathBuf, dirPath, dirPathLen * sizeof(HlNChar));

    /* Append path combine separator if necessary. */
    if (hlINPathCombineNeedsSep1(pathBuf, dirPathLen))
    {
        pathBuf[dirPathLen++] = HL_PATH_SEP;
    }

    /* Append null terminator. */
    pathBuf[dirPathLen] = HL_NTEXT('\0');

    /* Extract entries, free path buffer, and return. */
    result = hlINArchiveEntriesExtract(entries, entryCount,
        recursive, &pathBuf, &pathBufLen, dirPathLen);

    hlFree(pathBuf);
    return result;
}

HlResult hlArchiveCreateFromDir(const HlNChar* HL_RESTRICT dirPath,
    HlArchive* HL_RESTRICT * HL_RESTRICT arc)
{
    /* TODO: Implement this function. */
    HL_ASSERT(0);
    return HL_ERROR_UNKNOWN; /* So compiler doesn't complain. */
}

void hlArchiveEntryDestruct(HlArchiveEntry* entry)
{
    /* Free path buffer. */
    hlFree(entry->path);

    /* Skip streaming entries. */
    if (hlArchiveEntryIsStreaming(entry))
        return;

    /* Recursively destruct directory sub-entries. */
    if ((entry->meta & HL_ARC_ENTRY_IS_DIR_FLAG) != 0)
    {
        HlArchiveEntry* subEntries;
        size_t i;

        /* Get sub-entry list pointer. */
        subEntries = (HlArchiveEntry*)((HlUPtr)entry->data);

        /* Recursively destruct sub-entries. */
        for (i = 0; i < entry->size; ++i)
        {
            hlArchiveEntryDestruct(&(subEntries[i]));
        }
    }

    /* Free entry data that is owned by the entry. */
    if ((entry->meta & HL_ARC_ENTRY_NOT_OWNS_DATA_FLAG) == 0)
    {
        hlFree((void*)((HlUPtr)entry->data));
    }
}

void hlArchiveDestruct(HlArchive* arc)
{
    size_t i;

    for (i = 0; i < arc->entries.count; ++i)
    {
        hlArchiveEntryDestruct(&(arc->entries.data[i]));
    }

    HL_LIST_FREE(arc->entries);
}

void hlArchiveFree(HlArchive* arc)
{
    hlArchiveDestruct(arc);
    hlFree(arc);
}

#ifndef HL_NO_EXTERNAL_WRAPPERS
HlBool hlArchiveEntryIsStreamingExt(const HlArchiveEntry* entry)
{
    return hlArchiveEntryIsStreaming(entry);
}

HlBool hlArchiveEntryIsDirExt(const HlArchiveEntry* entry)
{
    return hlArchiveEntryIsDir(entry);
}

HlBool hlArchiveEntryIsFileExt(const HlArchiveEntry* entry)
{
    return hlArchiveEntryIsFile(entry);
}

size_t hlArchiveEntryGetCompressedSizeExt(const HlArchiveEntry* entry)
{
    return hlArchiveEntryGetCompressedSize(entry);
}

HlBool hlArchiveEntryIsReferenceExt(const HlArchiveEntry* entry)
{
    return hlArchiveEntryIsReference(entry);
}

HlResult hlArchiveExtractExt(const HlArchive* HL_RESTRICT arc,
    const HlNChar* HL_RESTRICT dirPath, HlBool recursive)
{
    return hlArchiveExtract(arc, dirPath, recursive);
}
#endif
