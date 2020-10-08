#include "hl_in_archive.h"
#include "hedgelib/archives/hl_gens_archive.h"
#include "hedgelib/io/hl_file.h"
#include "hedgelib/io/hl_path.h"
#include "hedgelib/hl_blob.h"

#define HL_INGENS_ARC_BLOB_BUF_LEN 10

const HlNChar HL_GENS_ARL_EXT[5] = HL_NTEXT(".arl");
const HlNChar HL_GENS_AR_EXT[4] = HL_NTEXT(".ar");
const HlNChar HL_GENS_PFD_EXT[5] = HL_NTEXT(".pfd");

HlResult hlGensArchiveStreamPFI(const HlNChar* HL_RESTRICT filePath,
    HlArchive* HL_RESTRICT * HL_RESTRICT archive)
{
    /* TODO */
    return HL_ERROR_UNKNOWN;
}

HlResult hlGensArchiveRead(const HlBlob* HL_RESTRICT * HL_RESTRICT splits,
    size_t splitCount, HlArchive* HL_RESTRICT * HL_RESTRICT archive)
{
    void* hlArcBuf;
    HlArchiveEntry* curEntry;
    size_t entryCount = 0;

    /* Allocate HlArchive buffer. */
    {
        /* Calculate required size for HlArchive buffer. */
        size_t i, reqBufSize = sizeof(HlArchive);
        for (i = 0; i < splitCount; ++i)
        {
            /* Get start position and end position. */
            const HlGensArchiveHeader* header = (const HlGensArchiveHeader*)splits[i]->data;
            const unsigned char* curPos = HL_ADD_OFFC(header, sizeof(*header));
            const unsigned char* endPos = HL_ADD_OFFC(header, splits[i]->size);

            /* Account for entries in this split. */
            while (curPos < endPos)
            {
                /* Get file entry and file name pointers. */
                const HlGensArchiveFileEntry* fileEntry = (const HlGensArchiveFileEntry*)curPos;
                const char* fileName = (const char*)(fileEntry + 1);

                /* Increment entry count. */
                ++entryCount;

                /* Account for data. */
                reqBufSize += fileEntry->dataSize;

                /* Account for text. */
#ifdef HL_IN_WIN32_UNICODE
                reqBufSize += (hlStrGetReqLenUTF8ToUTF16(fileName, 0) *
                    sizeof(HlNChar));
#else
                reqBufSize += (strlen(fileName) + 1);
#endif

                /* Go to next file entry within the archive. */
                curPos += fileEntry->entrySize;
            }
        }

        /* Account for archive entries. */
        reqBufSize += (sizeof(HlArchiveEntry) * entryCount);

        /* Allocate archive buffer. */
        hlArcBuf = hlAlloc(reqBufSize);
        if (!hlArcBuf) return HL_ERROR_OUT_OF_MEMORY;
    }

    /* Setup archive. */
    {
        /* Get pointers. */
        HlArchive* arc = (HlArchive*)hlArcBuf;
        curEntry = (HlArchiveEntry*)(arc + 1);

        /* Setup archive. */
        arc->entries = curEntry;
        arc->entryCount = entryCount;
    }

    /* Setup archive entries. */
    {
        char* curDataPtr = (char*)(&curEntry[entryCount]);
        size_t i;

        for (i = 0; i < splitCount; ++i)
        {
            /* Get start position and end position. */
            const HlGensArchiveHeader* header = (const HlGensArchiveHeader*)splits[i]->data;
            const unsigned char* curPos = HL_ADD_OFFC(header, sizeof(*header));
            const unsigned char* endPos = HL_ADD_OFFC(header, splits[i]->size);

            /* Setup file entries in this split. */
            while (curPos < endPos)
            {
                /* Get file entry and file name pointers. */
                const HlGensArchiveFileEntry* fileEntry = (const HlGensArchiveFileEntry*)curPos;
                const char* fileName = (const char*)(fileEntry + 1);
                const void* fileData = HL_ADD_OFFC(fileEntry, fileEntry->dataOffset);

                /* Setup file entry. */
                curEntry->path = (const HlNChar*)curDataPtr;
                curEntry->size = (size_t)fileEntry->dataSize;
                curEntry->meta = 0;
                
                /* Copy file name. */
#ifdef HL_IN_WIN32_UNICODE
                {
                    /* Convert file name to UTF-16 and copy into buffer. */
                    size_t utf16StrLen = hlStrConvUTF8ToUTF16NoAlloc(fileName,
                        (HlChar16*)curDataPtr, 0, 0);

                    if (!utf16StrLen)
                    {
                        hlFree(hlArcBuf);
                        return HL_ERROR_UNKNOWN;
                    }

                    /* Increase pointer. */
                    curDataPtr += (utf16StrLen * sizeof(HlNChar));
                }
#else
                /* Copy file name into buffer and increase pointer. */
                curDataPtr += (hlStrCopyAndLen(fileName, curDataPtr) + 1);
#endif

                /* Set data pointer. */
                curEntry->data = (HlUMax)((HlUPtr)curDataPtr);

                /* Copy data. */
                memcpy(curDataPtr, fileData, curEntry->size);
                curDataPtr += curEntry->size;

                /* Go to next file entry within the archive. */
                ++curEntry;
                curPos += fileEntry->entrySize;
            }
        }
    }

    /* Set archive pointer and return success. */
    *archive = (HlArchive*)hlArcBuf;
    return HL_RESULT_SUCCESS;
}

static HlResult hlINGensArchiveLoadSingle(const HlNChar* HL_RESTRICT filePath,
    HlArchive* HL_RESTRICT * HL_RESTRICT archive)
{
    HlBlob* blob;
    HlResult result;

    /* Load archive. */
    result = hlBlobLoad(filePath, &blob);
    if (HL_FAILED(result)) return result;

    /* Parse blob into HlArchive, free blob, and return. */
    result = hlGensArchiveRead((const HlBlob**)&blob, 1, archive);
    hlFree(blob);
    return result;
}

static HlResult hlINGensArchiveLoadSplits(const HlNChar* HL_RESTRICT filePath,
    HlArchive* HL_RESTRICT * HL_RESTRICT archive)
{
    HlNChar* pathBuf = NULL;
    HlNChar* lastCharPtr = NULL;
    HlResult result = HL_RESULT_SUCCESS;

    /* Get valid file path. */
    {
        const size_t filePathLen = hlNStrLen(filePath);
        const HlNChar* ext = hlPathGetExt(filePath);

        /* If the given file path is a split, create a copy of it we can iterate on. */
        if (ext[0] == HL_NTEXT('.') && HL_IS_DIGIT(ext[1]) &&
            HL_IS_DIGIT(ext[2]) && ext[3] == HL_NTEXT('\0'))
        {
            /* Create a copy of filePath. */
            pathBuf = HL_ALLOC_ARR(HlNChar, filePathLen + 1);
            if (!pathBuf) return HL_ERROR_OUT_OF_MEMORY;

            memcpy(pathBuf, filePath, (filePathLen + 1) * sizeof(HlNChar));

            /* Set lastCharPtr. */
            lastCharPtr = &pathBuf[filePathLen - 1];
        }
        
        /* If the given file path is an arl, get the path to the corresponding .ar or .ar.00 */
        else if (hlNStrsEqual(ext, HL_GENS_ARL_EXT))
        {
            /* Create a bigger copy of filePath. */
            pathBuf = HL_ALLOC_ARR(HlNChar, filePathLen + 3);
            if (!pathBuf) return HL_ERROR_OUT_OF_MEMORY;

            memcpy(pathBuf, filePath, filePathLen * sizeof(HlNChar));

            /* Change extension from .arl$ to .ar.00$ */
            pathBuf[filePathLen - 1] = HL_NTEXT('.');
            pathBuf[filePathLen]     = HL_NTEXT('0');
            pathBuf[filePathLen + 1] = HL_NTEXT('0');
            pathBuf[filePathLen + 2] = HL_NTEXT('\0');

            /* See if .ar.00 archive exists... */
            if (!hlPathExists(pathBuf))
            {
                /* ...If it doesn't, change extension from .ar.00$ to .ar$ */
                pathBuf[filePathLen - 1] = HL_NTEXT('\0');

                /* Check if the .ar$ exists, and if not, return with an error. */
                if (!hlPathExists(pathBuf))
                {
                    result = HL_ERROR_NOT_FOUND;
                    goto end;
                }

                /* Set filePath. */
                filePath = pathBuf;
            }
            else
            {
                /* Set lastCharPtr. */
                lastCharPtr = &pathBuf[filePathLen + 1];
            }
        }

        /*
            If the given file path has another non-split extension and it
            doesn't exist, see if it has any splits.
        */
        else if (!hlPathExists(filePath))
        {
            /* Create a bigger copy of filePath. */
            pathBuf = HL_ALLOC_ARR(HlNChar, filePathLen + 4);
            if (!pathBuf) return HL_ERROR_OUT_OF_MEMORY;

            memcpy(pathBuf, filePath, filePathLen * sizeof(HlNChar));

            /* Add split extension .00$ */
            pathBuf[filePathLen]     = HL_NTEXT('.');
            pathBuf[filePathLen + 1] = HL_NTEXT('0');
            pathBuf[filePathLen + 2] = HL_NTEXT('0');
            pathBuf[filePathLen + 3] = HL_NTEXT('\0');

            /* See if .00 archive exists, and if not, return with an error. */
            if (!hlPathExists(pathBuf))
            {
                result = HL_ERROR_NOT_FOUND;
                goto end;
            }

            /* Set lastCharPtr. */
            lastCharPtr = &pathBuf[filePathLen + 2];
        }
    }

    /* Load splits. */
    if (lastCharPtr)
    {
        HlBlob* blobs[HL_INGENS_ARC_BLOB_BUF_LEN];
        HlBlob** blobsPtr = blobs;
        size_t blobCount = 0, blobCapacity = HL_INGENS_ARC_BLOB_BUF_LEN;

        /* Load splits. */
        do
        {
            /* Load this split. */
            result = hlBlobLoad(pathBuf, &blobsPtr[blobCount]);
            if (HL_FAILED(result)) goto free_blobs_and_end;

            /* Increase blob count, and size of blobs array if necessary. */
            if (++blobCount >= blobCapacity)
            {
                /* Increase blobs array capacity. */
                HlBlob** newBlobsPtr;
                blobCapacity += HL_INGENS_ARC_BLOB_BUF_LEN;

                /* Make sure we're already using a dynamically-allocated array, and resize it. */
                if (blobsPtr != blobs)
                {
                    /* Realloc the existing dynamically-allocated blobs array. */
                    newBlobsPtr = HL_RESIZE_ARR(HlBlob*,
                        blobCapacity, blobsPtr);

                    if (!newBlobsPtr)
                    {
                        result = HL_ERROR_OUT_OF_MEMORY;
                        goto free_blobs_and_end;
                    }
                }

                /* Switch from the static "blobs" array to a dynamically-allocated array. */
                else
                {
                    /* Dynamically allocate a new blobs array. */
                    newBlobsPtr = HL_ALLOC_ARR(HlBlob*, blobCapacity);

                    if (!newBlobsPtr)
                    {
                        result = HL_ERROR_OUT_OF_MEMORY;
                        goto free_blobs_and_end;
                    }

                    /* Copy blob pointers into new array. */
                    memcpy(newBlobsPtr, blobs, sizeof(HlBlob*) *
                        HL_INGENS_ARC_BLOB_BUF_LEN);
                }

                /* Set blobsPtr. */
                blobsPtr = newBlobsPtr;
            }
        }
        while (hlINArchiveNextSplit2(lastCharPtr) && hlPathExists(pathBuf));

        /* Parse blobs into HlArchive. */
        result = hlGensArchiveRead((const HlBlob**)blobsPtr, blobCount, archive);

    free_blobs_and_end:
        {
            /* Free blobs. */
            size_t i;
            for (i = 0; i < blobCount; ++i)
            {
                hlFree(blobsPtr[i]);
            }

            /* Free blobs array if necessary. */
            if (blobsPtr != blobs) hlFree(blobsPtr);

            goto end;
        }
    }

    /* Load single archive. */
    else
    {
        result = hlINGensArchiveLoadSingle(filePath, archive);
        goto end;
    }

end:
    /* Free path buffer if we allocated it. */
    if (pathBuf) hlFree(pathBuf);

    /* Return result. */
    return result;
}

HlResult hlGensArchiveLoad(const HlNChar* HL_RESTRICT filePath,
    HlBool loadSplits, HlArchive* HL_RESTRICT * HL_RESTRICT archive)
{
    /* Load all splits or simply a single archive based on user request. */
    return (loadSplits) ?
        hlINGensArchiveLoadSplits(filePath, archive) :
        hlINGensArchiveLoadSingle(filePath, archive);
}

HlResult hlGensArchiveSave(const HlArchive* arc, HlU32 splitLimit,
    HlU32 padAmount, HlCompressType compressType, HlBool generateARL,
    const HlNChar* filePath)
{
    /* TODO */
    return HL_ERROR_UNKNOWN;
}
