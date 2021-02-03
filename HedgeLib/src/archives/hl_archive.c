#include "../hl_in_assert.h"
#include "../io/hl_in_path.h"
#include "hedgelib/hl_text.h"
#include "hedgelib/io/hl_path.h"
#include "hedgelib/io/hl_file.h"
#include "hedgelib/archives/hl_archive.h"

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

#define HL_IN_ARC_PATH_BUF_LEN 255

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

HlBool hlINArchiveNextSplit3(HlNChar* lastCharPtr)
{
    /*
       Increment the last character in the path and check if it's > 9.
       
       Before           After           > 9?
       Sonic.pac.000    Sonic.pac.001   No
       Sonic.pac.009    Sonic.pac.00:   Yes
       Sonic.pac.090    Sonic.pac.091   No
       Sonic.pac.199    Sonic.pac.19:   Yes
       Sonic.pac.999    Sonic.pac.99:   Yes
    */
    if (++(*lastCharPtr) > HL_NTEXT('9'))
    {
        /*
           Increment the second-to-last character in the path and check if it's > 9.

           Before           After           > 9?
           Sonic.pac.00:    Sonic.pac.01:   No
           Sonic.pac.19:    Sonic.pac.1::   Yes
           Sonic.pac.99:    Sonic.pac.9::   Yes
        */
        if (++(*(lastCharPtr - 1)) > HL_NTEXT('9'))
        {
            /* 
               Increment the third-to-last character in the path and check if it's > 9.

               Before           After           > 9?
               Sonic.pac.1::    Sonic.pac.2::   No
               Sonic.pac.9::    Sonic.pac.:::   Yes
            */
            if (++(*(lastCharPtr - 2)) > HL_NTEXT('9'))
            {
                /* We've gone over split .999 - there is no next split; return false. */
                return HL_FALSE;
            }
            else
            {
                /*
                   We haven't gone over split 999 yet; reset second-to-last character in path.

                   Before           After
                   Sonic.pac.2::    Sonic.pac.20:
                */
                *(lastCharPtr - 1) = HL_NTEXT('0');
            }
        }

        /*
           We haven't gone over split 99 yet; reset last character in path.

           Before           After
           Sonic.pac.01:    Sonic.pac.010
           Sonic.pac.20:    Sonic.pac.200
        */
        *lastCharPtr = HL_NTEXT('0');
    }

    return HL_TRUE;
}

static HlNChar* hlINArchiveEnlargePathBuffer(size_t minAmount,
    HlBool* HL_RESTRICT pathBufOnHeap, HlNChar* HL_RESTRICT pathBuf,
    size_t* HL_RESTRICT pathBufCap)
{
    size_t pathBufEnlargeAmount;

    /* Ensure path buffer can still be enlargened. */
    if ((sizeof(HlNChar) * (*pathBufCap)) > (HL_SIZE_MAX -
        (sizeof(HlNChar) * minAmount)))
    {
        return NULL;
    }

    /*
       Get amount to enlarge the path buffer by.

       We want to enlarge the path buffer by a decent amount to lower
       the amount of necessary subsequent re-allocations, therefore we
       enlarge it by whichever is larger: HL_IN_ARC_PATH_BUF_LEN or minAmount.

       If HL_IN_ARC_PATH_BUF_LEN is larger than minAmount, we also have to
       ensure, once again, that the path buffer can still be enlargened - this
       time by HL_IN_ARC_PATH_BUF_LEN.
    */
    pathBufEnlargeAmount = HL_IN_ARC_PATH_BUF_LEN;

    if (pathBufEnlargeAmount < minAmount ||
        (sizeof(HlNChar) * (*pathBufCap)) > (HL_SIZE_MAX -
        (sizeof(HlNChar) * pathBufEnlargeAmount)))
    {
        pathBufEnlargeAmount = minAmount;
    }

    /* Enlarge path buffer. */
    *pathBufCap += pathBufEnlargeAmount;

    if (*pathBufOnHeap)
    {
        return HL_RESIZE_ARR(HlNChar, *pathBufCap, pathBuf);
    }
    else
    {
        HlNChar* newPathBuf = HL_ALLOC_ARR(HlNChar, *pathBufCap);
        if (!newPathBuf) return NULL;

        memcpy(newPathBuf, pathBuf, *pathBufCap * sizeof(HlNChar));
        *pathBufOnHeap = HL_TRUE;
        return newPathBuf;
    }
}

static HlResult hlINArchiveEntriesExtract(const HlArchiveEntry* HL_RESTRICT entries,
    size_t entryCount, HlBool recursive, HlBool* HL_RESTRICT pathBufOnHeap,
    HlNChar* HL_RESTRICT * HL_RESTRICT pathBuf,
    size_t* HL_RESTRICT pathBufCap, size_t pathEndPos)
{
    /* Extract entries. */
    size_t i;
    HlResult result;

    for (i = 0; i < entryCount; ++i)
    {
        const HlNChar* entryName;
        size_t entryNameSize;

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
            const size_t pathBufFreeSpace = (*pathBufCap - pathEndPos);
            
            if (!hlNStrCopyLimit(entryName, pathBufEnd,
                pathBufFreeSpace, &entryNameSize))
            {
                /*
                   The archive entry name won't fit within the path buffer...
                   We need to resize the path buffer to make it fit.
                */

                /*
                   Get entry name length.
                   (hlNStrCopyLimit doesn't set this for us if it fails.)
                */
                entryNameSize = (hlNStrLen(entryName) + 1);

                /* Enlarge path buffer. */
                *pathBuf = hlINArchiveEnlargePathBuffer(
                    entryNameSize, pathBufOnHeap,
                    *pathBuf, pathBufCap);

                if (!(*pathBuf)) return HL_ERROR_OUT_OF_MEMORY;

                /* Get pathBufEnd again since pathBuf may have changed. */
                pathBufEnd = (*pathBuf + pathEndPos);

                /* Append archive entry name to end of path buffer. */
                memcpy(pathBufEnd, entryName, entryNameSize * sizeof(HlNChar));
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
                return HL_ERROR_UNSUPPORTED;
            }

            /* Extract file. */
            else
            {
                result = hlFileSave((void*)((HlUPtr)entries[i].data),
                    entries[i].size, *pathBuf);

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
            fullPathLen = (pathEndPos + (entryNameSize - 1));
            if (hlINPathCombineNeedsSep1(*pathBuf, fullPathLen))
            {
                /* Enlarge path buffer if necessary to append path combine separator. */
                if ((fullPathLen + 2) > *pathBufCap)
                {
                    /* Enlarge path buffer. */
                    *pathBuf = hlINArchiveEnlargePathBuffer(1,
                        pathBufOnHeap, *pathBuf, pathBufCap);

                    if (!(*pathBuf)) return HL_ERROR_OUT_OF_MEMORY;
                }

                /* Append path combine separator (and new null terminator). */
                *pathBuf[fullPathLen++] = HL_PATH_SEP;
                *pathBuf[fullPathLen] = HL_NTEXT('\0');
            }

            /* Recursively extract archive entries within this directory. */
            result = hlINArchiveEntriesExtract(
                (HlArchiveEntry*)((HlUPtr)entries[i].data), entries[i].size,
                recursive, pathBufOnHeap, pathBuf, pathBufCap, fullPathLen);

            if (HL_FAILED(result)) return result;
        }
    }

    return HL_RESULT_SUCCESS;
}

HlResult hlArchiveEntriesExtract(const HlArchiveEntry* HL_RESTRICT entries,
    size_t entryCount, const HlNChar* HL_RESTRICT dirPath, HlBool recursive)
{
    HlNChar pathBuf[HL_IN_ARC_PATH_BUF_LEN];
    HlNChar* pathBufPtr = pathBuf;
    size_t pathBufCap = HL_IN_ARC_PATH_BUF_LEN, dirPathLen;
    HlResult result;
    HlBool pathBufOnHeap;

    /* Ensure extraction directory exists. */
    result = hlPathCreateDirectory(dirPath, HL_TRUE);
    if (HL_FAILED(result)) return result;

    /* Return early if entryCount == 0 to avoid unnecessary work. */
    if (!entryCount) return result;

    /* Get dirPath length. */
    dirPathLen = hlNStrLen(dirPath);

    /* Allocate path buffer if necessary. */
    if ((dirPathLen + 2) > pathBufCap)
    {
        pathBufCap = (dirPathLen + HL_IN_ARC_PATH_BUF_LEN);
        pathBufPtr = HL_ALLOC_ARR(HlNChar, pathBufCap);
        pathBufOnHeap = HL_TRUE;

        if (!pathBufPtr) return HL_ERROR_OUT_OF_MEMORY;
    }
    else
    {
        pathBufOnHeap = HL_FALSE;
    }

    /* Copy directory into path buffer. */
    memcpy(pathBufPtr, dirPath, dirPathLen * sizeof(HlNChar));

    /* Append path combine separator if necessary. */
    if (hlINPathCombineNeedsSep1(pathBufPtr, dirPathLen))
    {
        pathBufPtr[dirPathLen++] = HL_PATH_SEP;
    }

    /* Extract entries, free path buffer if necessary, and return result. */
    result = hlINArchiveEntriesExtract(entries, entryCount,
        recursive, &pathBufOnHeap, &pathBufPtr,
        &pathBufCap, dirPathLen);

    if (pathBufOnHeap) hlFree(pathBufPtr);

    return result;
}

HlResult hlArchiveConstruct(HlArchive* arc)
{
    HL_LIST_INIT(arc->entries);
    return HL_RESULT_SUCCESS;
}

HlResult hlArchiveCreate(HlArchive** arc)
{
    HlArchive* hlArcBuf;
    HlResult result;
    
    /* Allocate HlArchive. */
    hlArcBuf = HL_ALLOC_OBJ(HlArchive);
    if (!hlArcBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Construct HlArchive. */
    result = hlArchiveConstruct(hlArcBuf);
    if (HL_FAILED(result))
    {
        hlFree(hlArcBuf);
        return result;
    }

    /* Set HlArchive pointer and return. */
    *arc = hlArcBuf;
    return result;
}

static HlResult hlINArchiveAddDir(
    HlNChar* HL_RESTRICT * HL_RESTRICT pathBuf,
    size_t* HL_RESTRICT pathBufCap, size_t pathBufLen,
    HlBool* HL_RESTRICT pathBufOnHeap, HlBool loadData,
    HlBool recursive, HlArchiveEntryList* HL_RESTRICT arcEntries)
{
    HlDirEntry dirEntry;
    HlDirHandle dir;
    HlResult result;

    /* Open the given directory. */
    (*pathBuf)[pathBufLen] = HL_NTEXT('\0');
    result = hlPathDirOpen(*pathBuf, &dir);
    if (HL_FAILED(result)) return result;

    /* Iterate through the files in the given directory. */
    while (result = hlPathDirGetNextEntry(dir, &dirEntry),
        HL_OK(result))
    {
        HlArchiveEntry arcEntry;
        const HlNChar* name = dirEntry.name;
        size_t nameSize;

        /* Ensure name doesn't begin with a path separator. */
        if (!hlINPathCombineNeedsSep2(name)) ++name;

        /* Append name to the end of the path buffer. */
        {
            HlNChar* pathBufEnd = (*pathBuf + pathBufLen);
            const size_t pathBufFreeSpace = (*pathBufCap - pathBufLen);
            
            if (!hlNStrCopyLimit(name, pathBufEnd,
                pathBufFreeSpace, &nameSize))
            {
                /*
                   The archive entry name won't fit within the path buffer...
                   We need to resize the path buffer to make it fit.
                */

                /*
                   Get entry name length.
                   (hlNStrCopyLimit doesn't set this for us if it fails.)
                */
                nameSize = (hlNStrLen(name) + 1);

                /* Enlarge path buffer. */
                *pathBuf = hlINArchiveEnlargePathBuffer(
                    nameSize, pathBufOnHeap,
                    *pathBuf, pathBufCap);

                if (!(*pathBuf))
                {
                    result = HL_ERROR_OUT_OF_MEMORY;
                    break;
                }

                /* Get pathBufEnd again since pathBuf may have changed. */
                pathBufEnd = (*pathBuf + pathBufLen);

                /* Append archive entry name to end of path buffer. */
                memcpy(pathBufEnd, name, nameSize * sizeof(HlNChar));
            }
        }

        if (dirEntry.type == HL_DIR_ENTRY_TYPE_FILE)
        {
            arcEntry.size = hlPathGetSize(*pathBuf);

            if (loadData)
            {
                void* fileData;
                HlFileStream* file;

                /* Open file. */
                result = hlFileStreamOpen(*pathBuf, HL_FILE_MODE_READ, &file);
                if (HL_FAILED(result)) break;

                /* Allocate name buffer. */
                arcEntry.path = HL_ALLOC_ARR(HlNChar, nameSize);
                if (!arcEntry.path)
                {
                    hlFileStreamClose(file);
                    result = HL_ERROR_OUT_OF_MEMORY;
                    break;
                }

                /* Copy name and null terminator into name buffer. */
                memcpy(arcEntry.path, name, nameSize * sizeof(HlNChar));

                /* Allocate data buffer. */
                fileData = hlAlloc(arcEntry.size);
                if (!fileData)
                {
                    hlFree(arcEntry.path);
                    hlFileStreamClose(file);
                    result = HL_ERROR_OUT_OF_MEMORY;
                    break;
                }

                /* Read data into buffer. */
                result = hlStreamRead(file, arcEntry.size, fileData, NULL);
                if (HL_FAILED(result))
                {
                    hlFree(fileData);
                    hlFree(arcEntry.path);
                    hlFileStreamClose(file);
                    break;
                }

                /* Close file. */
                result = hlFileStreamClose(file);
                if (HL_FAILED(result))
                {
                    hlFree(fileData);
                    hlFree(arcEntry.path);
                    break;
                }
                
                /* Finish setting up entry. */
                arcEntry.meta = 0;
                arcEntry.data = (HlUMax)((HlUPtr)fileData);
            }
            else
            {
                /* Allocate path buffer. */
                arcEntry.path = HL_ALLOC_ARR(HlNChar, pathBufLen + nameSize);
                if (!arcEntry.path)
                {
                    result = HL_ERROR_OUT_OF_MEMORY;
                    break;
                }

                /* Copy path and null terminator into path buffer. */
                memcpy(arcEntry.path, *pathBuf,
                    (pathBufLen + nameSize) * sizeof(HlNChar));

                /* Finish setting up entry. */
                arcEntry.meta = 0;
                arcEntry.data = (HlUMax)((HlUPtr)NULL);
            }

            /* Add entry to entries list. */
            result = HL_LIST_PUSH(*arcEntries, arcEntry);
            if (HL_FAILED(result))
            {
                hlArchiveEntryDestruct(&arcEntry);
                break;
            }
        }
        else if (recursive && dirEntry.type == HL_DIR_ENTRY_TYPE_DIRECTORY)
        {
            HlArchiveEntryList subDirEntries;
            size_t fullPathLen;

            /* Append path combine separator if necessary. */
            fullPathLen = (pathBufLen + (nameSize - 1));
            if (hlINPathCombineNeedsSep1(*pathBuf, fullPathLen))
            {
                /* Enlarge path buffer if necessary to append path combine separator. */
                if ((fullPathLen + 2) > *pathBufCap)
                {
                    /* Enlarge path buffer. */
                    *pathBuf = hlINArchiveEnlargePathBuffer(1,
                        pathBufOnHeap, *pathBuf, pathBufCap);

                    if (!(*pathBuf))
                    {
                        result = HL_ERROR_OUT_OF_MEMORY;
                        break;
                    }
                }

                /* Append path combine separator (and new null terminator). */
                (*pathBuf)[fullPathLen++] = HL_PATH_SEP;
                (*pathBuf)[fullPathLen] = HL_NTEXT('\0');
            }

            /* Initialize sub-directory entries list. */
            HL_LIST_INIT(subDirEntries);

            /* Recursively add archive entries within this sub-directory. */
            result = hlINArchiveAddDir(pathBuf,
                pathBufCap, fullPathLen, pathBufOnHeap,
                loadData, recursive, &subDirEntries);

            if (HL_FAILED(result))
            {
                HL_LIST_FREE(subDirEntries);
                break;
            }

            /* Allocate name buffer. */
            arcEntry.path = HL_ALLOC_ARR(HlNChar, nameSize);
            if (!arcEntry.path)
            {
                HL_LIST_FREE(subDirEntries);
                result = HL_ERROR_OUT_OF_MEMORY;
                break;
            }

            /* Copy name and null terminator into name buffer. */
            memcpy(arcEntry.path, name, nameSize * sizeof(HlNChar));

            /* Finish setting up entry. */
            arcEntry.size = subDirEntries.count;
            arcEntry.meta = HL_ARC_ENTRY_IS_DIR_FLAG;
            arcEntry.data = (HlUMax)((HlUPtr)subDirEntries.data);

            /*
                NOTE: From here on out we do NOT need to free the subDirEntries list
                manually; its data is now contained within arcEntry, so it can be
                freed just by freeing arcEntry.
            */

            /* Add directory entry to entries list. */
            result = HL_LIST_PUSH(*arcEntries, arcEntry);
            if (HL_FAILED(result))
            {
                hlArchiveEntryDestruct(&arcEntry);
                break;
            }
        }
    }

    /* Free resources and return result. */
    if (result != HL_ERROR_NO_MORE_ENTRIES)
    {
        hlPathDirClose(dir);
        return result;
    }

    return hlPathDirClose(dir);
}

HlResult hlArchiveAddDir(const HlNChar* HL_RESTRICT dirPath,
    HlBool loadData, HlBool recursive, HlArchive* HL_RESTRICT arc)
{
    HlNChar pathBuf[HL_IN_ARC_PATH_BUF_LEN];
    HlNChar* pathBufPtr = pathBuf;
    size_t pathBufLen, pathBufCap = HL_IN_ARC_PATH_BUF_LEN;
    HlResult result;
    HlBool pathBufOnHeap;

    /* Get directory path length. */
    pathBufLen = hlNStrLen(dirPath);

    /* Allocate path buffer if necessary. */
    if ((pathBufLen + 2) > pathBufCap)
    {
        pathBufCap = (pathBufLen + HL_IN_ARC_PATH_BUF_LEN);
        pathBufPtr = HL_ALLOC_ARR(HlNChar, pathBufCap);
        pathBufOnHeap = HL_TRUE;

        if (!pathBufPtr) return HL_ERROR_OUT_OF_MEMORY;
    }
    else
    {
        pathBufOnHeap = HL_FALSE;
    }

    /* Copy directory into path buffer. */
    memcpy(pathBufPtr, dirPath, pathBufLen * sizeof(HlNChar));

    /* Append path combine separator if necessary. */
    if (hlINPathCombineNeedsSep1(pathBufPtr, pathBufLen))
    {
        pathBufPtr[pathBufLen++] = HL_PATH_SEP;
    }

    /* Add files/directories within the given directory. */
    result = hlINArchiveAddDir(&pathBufPtr,
        &pathBufCap, pathBufLen, &pathBufOnHeap,
        loadData, recursive, &arc->entries);

    /* Free path buffer if necessary and return result. */
    if (pathBufOnHeap) hlFree(pathBufPtr);

    return result;
}

HlResult hlArchiveCreateFromDir(const HlNChar* HL_RESTRICT dirPath,
    HlBool loadData, HlBool recursive, HlArchive* HL_RESTRICT * HL_RESTRICT arc)
{
    HlArchive* hlArcBuf;
    HlResult result;
    
    /* Create an HlArchive. */
    result = hlArchiveCreate(&hlArcBuf);
    if (HL_FAILED(result)) return result;

    /* Add all the entries in the given directory. */
    result = hlArchiveAddDir(dirPath, loadData, recursive, hlArcBuf);
    if (HL_FAILED(result))
    {
        hlArchiveFree(hlArcBuf);
        return result;
    }

    /* Set pointer and return result. */
    *arc = hlArcBuf;
    return result;
}

const HlNChar* hlArchiveEntryGetName(const HlArchiveEntry* entry)
{
    return (hlArchiveEntryIsReference(entry)) ?
        hlPathGetName(entry->path) : entry->path;
}

void hlArchiveEntryDestruct(HlArchiveEntry* entry)
{
    if (!entry) return;

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
    if (!arc) return;

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

HlResult hlPackedFileEntryConstruct(const HlNChar* HL_RESTRICT name,
    HlU32 dataPos, HlU32 dataSize, HlPackedFileEntry* HL_RESTRICT entry)
{
    /* Convert name to UTF-8. */
    char* nameUTF8 = hlStrConvNativeToUTF8(name, 0);
    if (!nameUTF8) return HL_ERROR_OUT_OF_MEMORY;

    /* Construct packed file entry and return success. */
    entry->name = nameUTF8;
    entry->dataPos = dataPos;
    entry->dataSize = dataSize;

    return HL_RESULT_SUCCESS;
}

void hlPackedFileEntryDestruct(HlPackedFileEntry* entry)
{
    if (!entry) return;
    hlFree(entry->name);
}

HlResult hlPackedFileIndexConstruct(HlPackedFileIndex* pfi)
{
    HL_LIST_INIT(pfi->entries);
    return HL_RESULT_SUCCESS;
}

void hlPackedFileIndexDestruct(HlPackedFileIndex* pfi)
{
    size_t i;
    for (i = 0; i < pfi->entries.count; ++i)
    {
        hlPackedFileEntryDestruct(&pfi->entries.data[i]);
    }

    HL_LIST_FREE(pfi->entries);
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

HlBool hlArchiveEntryIsRegularFileExt(const HlArchiveEntry* entry)
{
    return hlArchiveEntryIsRegularFile(entry);
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
