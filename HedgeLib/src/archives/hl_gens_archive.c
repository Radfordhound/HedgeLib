#include "hl_in_archive.h"
#include "hedgelib/hl_endian.h"
#include "hedgelib/io/hl_file.h"
#include "hedgelib/io/hl_path.h"
#include "hedgelib/io/hl_hh.h"
#include "hedgelib/archives/hl_gens_archive.h"

#define HL_INGENS_ARC_BLOB_BUF_LEN 10

const HlNChar HL_GENS_ARL_EXT[5] = HL_NTEXT(".arl");
const HlNChar HL_GENS_AR_EXT[4] = HL_NTEXT(".ar");
const HlNChar HL_GENS_PFD_EXT[5] = HL_NTEXT(".pfd");
const HlNChar HL_GENS_PFI_EXT[5] = HL_NTEXT(".pfi");

void hlHHPackedFileEntrySwap(HlHHPackedFileEntry* entry, HlBool swapOffsets)
{
    if (swapOffsets) hlSwapU32P(&entry->nameOffset);

    hlSwapU32P(&entry->dataPos);
    hlSwapU32P(&entry->dataSize);
}

void hlHHPackedFileIndexV0Swap(HlHHPackedFileIndexV0* pfi, HlBool swapOffsets)
{
    hlSwapU32P(&pfi->entryCount);
    if (swapOffsets) hlSwapU32P(&pfi->entriesOffset);
}

HlResult hlGensArchiveRead(const HlBlob* const HL_RESTRICT * HL_RESTRICT splits,
    size_t splitCount, HlArchive* HL_RESTRICT * HL_RESTRICT archive)
{
    HlArchive* hlArcBuf;
    HlResult result;

    /* Allocate HlArchive buffer. */
    hlArcBuf = HL_ALLOC_OBJ(HlArchive);
    if (!hlArcBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Setup archive. */
    HL_LIST_INIT(hlArcBuf->entries);

    /* TODO: Account for endianness on big-endian machines. */

    /* Setup archive entries. */
    {
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
                /*size_t nameLen;*/
                HlArchiveEntry entry;
                void* dataBuf;

                /* Convert file name to native encoding and copy into buffer. */
                entry.path = hlStrConvUTF8ToNative(fileName, 0);
                
                if (!entry.path)
                {
                    hlArchiveFree(hlArcBuf);
                    return HL_ERROR_OUT_OF_MEMORY;
                }

                /* Allocate new data buffer. */
                entry.size = (size_t)fileEntry->dataSize;
                dataBuf = hlAlloc(entry.size);

                if (!dataBuf)
                {
                    hlFree(entry.path);
                    hlArchiveFree(hlArcBuf);
                    return HL_ERROR_OUT_OF_MEMORY;
                }

                /* Copy data. */
                memcpy(dataBuf, fileData, entry.size);

                /* Finish setting up entry. */
                entry.meta = 0;
                entry.data = (HlUMax)((HlUPtr)dataBuf);
                
                /* Add new entry to archive. */
                {
                    HlArchiveEntry* oldDataPtr = hlArcBuf->entries.data;
                    result = HL_LIST_PUSH(hlArcBuf->entries, entry);

                    if (HL_FAILED(result))
                    {
                        hlArcBuf->entries.data = oldDataPtr;
                        hlArchiveEntryDestruct(&entry);
                        hlArchiveFree(hlArcBuf);
                        return result;
                    }
                }

                /* Go to next file entry within the archive. */
                curPos += fileEntry->entrySize;
            }
        }
    }

    /* Set archive pointer and return success. */
    *archive = hlArcBuf;
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

            /* If this is not the .00 split, check if the .00 split exists. */
            if (ext[1] != HL_NTEXT('0') || ext[2] != HL_NTEXT('0'))
            {
                *(lastCharPtr - 1) = HL_NTEXT('0');
                *lastCharPtr = HL_NTEXT('0');

                /* Fallback to the given file path if .00 split does not exist. */
                if (!hlPathExists(pathBuf))
                {
                    *(lastCharPtr - 1) = ext[1];
                    *lastCharPtr = ext[2];
                }
            }
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

HlResult hlGensArchiveSave(const HlArchive* HL_RESTRICT arc, HlU32 splitLimit,
    HlU32 dataAlignment, HlCompressType compressType, HlBool generateARL,
    HlPackedFileIndex* HL_RESTRICT pfi, const HlNChar* HL_RESTRICT filePath)
{
    /*
        input           splits          arl             no splits           max reqBufLen
        
        #ghz101.ar      #ghz101.ar.00   #ghz101.arl     #ghz101.ar          +6
        #ghz101.ar.00   #ghz101.ar.00   #ghz101.arl     #ghz101.ar.00       +6
        #ghz101.pfd     #ghz101.pfd.00  #ghz101.arl     #ghz101.pfd         +7
        #ghz101.a.b     #ghz101.a.b.00  #ghz101.arl     #ghz101.a.b         +7
        #ghz101.00      #ghz101.00      #ghz101.arl     #ghz101.00          +4
        #ghz101         #ghz101.00      #ghz101.arl     #ghz101             +4
    */
    HlNChar pathBuf[255];
    HlNChar* pathBufPtr = pathBuf;
    HlFile *arcFile = NULL, *arlFile = NULL;
    const HlNChar* exts = hlPathGetExts(filePath);
    const size_t filePathLen = (size_t)(exts - filePath);
    size_t nonSplitExtsLen, pathBufCount, pathBufCap = 255;
    HlU32 splitCount = 0, arcSize;
    HlResult result = HL_RESULT_SUCCESS;

    /* Setup path buffer. */
    {
        /* Account for the extensions we'll need to place at the end of the path. */
        size_t reqPathBufLen;

        if (splitLimit)
        {
            /* Account for non-split extensions. */
            const HlNChar* finalExt = hlPathGetExt(exts);
            nonSplitExtsLen = (size_t)(finalExt - exts);

            if (*finalExt && !hlArchiveExtIsSplit(finalExt))
            {
                nonSplitExtsLen += hlNStrLen(finalExt);
            }

            /* Account for 2-digit split extension. */
            reqPathBufLen = (nonSplitExtsLen + 3);
        }
        else
        {
            /* We won't be generating splits; account for all extensions in filePath. */
            reqPathBufLen = nonSplitExtsLen = hlNStrLen(exts);
        }

        /* Account for .arl extension. */
        if (generateARL && reqPathBufLen < 4)
        {
            reqPathBufLen = 4;
        }

        /* Account for the rest of the path. */
        reqPathBufLen += filePathLen;

        /* Allocate path buffer if necessary. */
        pathBufCount = (reqPathBufLen + 1);

        if (pathBufCount > pathBufCap)
        {
            pathBufPtr = HL_ALLOC_ARR(HlNChar, pathBufCount);
            if (!pathBufPtr) return HL_ERROR_OUT_OF_MEMORY;
        }

        /* Copy all but the extension and null-terminator into the path buffer. */
        memcpy(pathBufPtr, filePath, filePathLen * sizeof(HlNChar));
    }

    /* Begin writing ARL if requested. */
    if (generateARL)
    {
        /*
           Generate placeholder ARL header (splitCount will be filled-in later).
           
           NOTE: Endianness doesn't matter here; the signature's endianness will
           always be correct, and splitCount is going to filled-in later.
        */
        const HlGensArchiveListHeader arlHeader =
        {
            HL_GENS_ARL_SIG,    /* signature */
            0                   /* splitCount */
        };

        /* Copy ARL extension and null terminator into path buffer. */
        memcpy(&pathBufPtr[filePathLen], HL_GENS_ARL_EXT, sizeof(HL_GENS_ARL_EXT));

        /* Open ARL for writing. */
        result = hlFileOpen(pathBufPtr, HL_FILE_MODE_WRITE, &arlFile);
        if (HL_FAILED(result)) goto failed_open_arl;

        /* Write placeholder ARL header. */
        result = hlFileWrite(arlFile, sizeof(arlHeader), &arlHeader, NULL);
        if (HL_FAILED(result)) goto failed;
    }

    /* Write archive(s) and ARL split sizes as requested. */
    {
        /* Generate the header that will be written to every split. */
        const HlGensArchiveHeader arcHeader =
        {
            0,                              /* unknown1 */
            sizeof(HlGensArchiveHeader),    /* headerSize */
            sizeof(HlGensArchiveFileEntry), /* entrySize */
            dataAlignment                   /* dataAlignment */
        };

        HlNChar* lastCharPtr = &pathBufPtr[filePathLen + nonSplitExtsLen];
        size_t i;
        HlBool wroteEntryToCurArc = HL_FALSE;
        
        /* Account for AR header. */
        arcSize = sizeof(HlGensArchiveHeader);

        /* Copy extension(s) from filePath into path buffer. */
        memcpy(&pathBufPtr[filePathLen], exts, nonSplitExtsLen * sizeof(HlNChar));

        if (splitLimit)
        {
            /* Copy 2-digit split extension and null terminator into buffer. */
            memcpy(lastCharPtr, HL_NTEXT(".00"), sizeof(HL_NTEXT(".00")));

            /* Increase last character pointer so that it points to the last split digit. */
            lastCharPtr += 2;
        }
        else
        {
            /* Copy null-terminator into buffer. */
            *lastCharPtr = HL_NTEXT('\0');
        }

        /* Open first archive for writing. */
        result = hlFileOpen(pathBufPtr, HL_FILE_MODE_WRITE, &arcFile);
        if (HL_FAILED(result)) goto failed_open_arc;

        /* Write AR header. */
        result = hlFileWrite(arcFile, sizeof(arcHeader), &arcHeader, NULL);
        if (HL_FAILED(result)) goto failed;

        /* Write archive(s) and ARL split sizes as requested. */
        for (i = 0; i < arc->entries.count; ++i)
        {
#ifdef HL_IN_WIN32_UNICODE
            char fileNameUTF8Buf[256];
#endif

            HlGensArchiveFileEntry fileEntry;
            const HlArchiveEntry* entry = &arc->entries.data[i];
            const HlNChar* entryName;
            void* entryData;
            const char* fileNameUTF8;
            size_t dataPos, entrySize, fileNameUTF8Size;
            HlBool doFreeEntryData = HL_FALSE;

            /* Skip streaming and directory entries. */
            if (!hlArchiveEntryIsRegularFile(entry))
                continue;

            /* Get entry data pointer. */
            if (entry->data == 0)
            {
                /* This entry is a file reference; load data into memory. */
                result = hlFileLoad(entry->path, &entryData, NULL);
                if (HL_FAILED(result)) goto failed;

                doFreeEntryData = HL_TRUE;
            }
            else
            {
                entryData = (void*)((HlUPtr)entry->data);
            }

            /* Compress data if necessary. */
            if (compressType != HL_COMPRESS_TYPE_NONE)
            {
                void* compressedEntryData;

                /* Compress data. */
                result = hlCompress(compressType,
                    entryData, entry->size,
                    &entrySize, &compressedEntryData);

                if (HL_FAILED(result)) goto failed;

                /* Free uncompressed data if necessary. */
                if (doFreeEntryData) hlFree(entryData);

                /* Set entry data pointer. */
                entryData = compressedEntryData;
                doFreeEntryData = HL_TRUE;
            }
            else
            {
                entrySize = entry->size;
            }

            /* Ensure file size can fit within a 32-bit unsigned integer. */
            if (entrySize > 0xFFFFFFFFU)
            {
                result = HL_ERROR_OUT_OF_RANGE;
                if (doFreeEntryData) hlFree(entryData);
                goto failed;
            }
            
            fileEntry.dataSize = (HlU32)entrySize;

            /* Get entry name and compute required size to convert to UTF-8. */
            entryName = hlArchiveEntryGetName(entry);
            fileNameUTF8Size = hlStrGetReqLenNativeToUTF8(entryName, 0);

            /*
               Ensure required length (size - 1) to convert entry name to UTF-8 fits
               within 255 characters, as this appears to be the limit for how long a
               file name can be within an ARL file (which probably applies to ARs/PFDs too).

               NOTE: That 256 is not a typo. fileNameUTF8Size includes the null-terminator
               which is not included in ARL files, so a fileNameUTF8Size value of 255 is
               okay.
            */
            if (fileNameUTF8Size > 256)
            {
                result = HL_ERROR_OUT_OF_RANGE;
                goto failed;
            }

            /* Account for entry and file name (including null terminator). */
            fileEntry.dataOffset = (HlU32)(sizeof(HlGensArchiveFileEntry) + fileNameUTF8Size);

            /* Account for file data alignment. */
            fileEntry.dataOffset = HL_ALIGN(fileEntry.dataOffset + arcSize, dataAlignment);
            fileEntry.dataOffset -= arcSize;

            /* Account for file size. */
            fileEntry.entrySize = (fileEntry.dataOffset + fileEntry.dataSize);

            /* Increase total archive size. */
            arcSize += fileEntry.entrySize;

            /* Set unknown values. */
            /* TODO: Find out what these are and set them properly. */
            fileEntry.unknown1 = 0;
            fileEntry.unknown2 = 0;

            /* Break off into next split if necessary. */
            if (splitLimit && arcSize > splitLimit && wroteEntryToCurArc)
            {
                /* Close the current split. */
                result = hlFileClose(arcFile);
                if (HL_FAILED(result)) goto failed_close_arc;

                /* Write current split archive size to ARL if necessary. */
                if (generateARL)
                {
                    result = hlFileWrite(arlFile, sizeof(arcSize), &arcSize, NULL);
                    if (HL_FAILED(result)) goto failed_close_arc;
                }

                /* Increase the number in the split extension. */
                if (!hlINArchiveNextSplit2(lastCharPtr))
                {
                    /* Error out if we went over 99 splits. */
                    result = HL_ERROR_OUT_OF_RANGE;
                    goto failed_open_arc;
                }

                /* Open the next split for writing. */
                result = hlFileOpen(pathBufPtr, HL_FILE_MODE_WRITE, &arcFile);
                if (HL_FAILED(result)) goto failed_open_arc;

                /* Write AR header. */
                result = hlFileWrite(arcFile, sizeof(arcHeader), &arcHeader, NULL);
                if (HL_FAILED(result)) goto failed;

                /* Reset arcSize. */
                arcSize = sizeof(HlGensArchiveHeader);

                /* Indicate that we have not yet written an entry to this new split. */
                wroteEntryToCurArc = HL_FALSE;

                /* Increment split count. */
                ++splitCount;

                /* Account for entry and file name (including null terminator). */
                fileEntry.dataOffset = (HlU32)(sizeof(HlGensArchiveFileEntry) + fileNameUTF8Size);

                /* Account for file data alignment. */
                fileEntry.dataOffset = HL_ALIGN(fileEntry.dataOffset + arcSize, dataAlignment);
                fileEntry.dataOffset -= arcSize;

                /* Account for file size. */
                fileEntry.entrySize = (fileEntry.dataOffset + fileEntry.dataSize);

                /* Increase total archive size. */
                arcSize += fileEntry.entrySize;
            }

            /* Convert file name to UTF-8 if necessary. */
#ifdef HL_IN_WIN32_UNICODE
            if (!hlStrConvNativeToUTF8NoAlloc(entryName,
                fileNameUTF8Buf, fileNameUTF8Size, 256))
            {
                result = HL_ERROR_INVALID_DATA;
                goto failed;
            }

            fileNameUTF8 = fileNameUTF8Buf;
#else
            fileNameUTF8 = entryName;
#endif
            
            /* Write file entry to archive. */
            result = hlFileWrite(arcFile, sizeof(fileEntry), &fileEntry, NULL);
            if (HL_FAILED(result))
            {
                if (doFreeEntryData) hlFree(entryData);
                goto failed;
            }

            /* Write file name to archive. */
            result = hlFileWrite(arcFile, fileNameUTF8Size, fileNameUTF8, NULL);
            if (HL_FAILED(result))
            {
                if (doFreeEntryData) hlFree(entryData);
                goto failed;
            }

            /* Write file data padding to archive. */
            result = hlFilePad(arcFile, dataAlignment);
            if (HL_FAILED(result))
            {
                if (doFreeEntryData) hlFree(entryData);
                goto failed;
            }

            /* Store data position. */
            dataPos = hlFileTell(arcFile);

            /* Write file data to archive. */
            result = hlFileWrite(arcFile, entrySize, entryData, NULL);

            /* Free entry data if necessary. */
            if (doFreeEntryData) hlFree(entryData);
            if (HL_FAILED(result)) goto failed; /* NOTE: From the hlFileWrite above. */

            /* Indicate that we have written at least one entry to this archive. */
            wroteEntryToCurArc = HL_TRUE;

            /* Add packed file entry to packed file index if necessary. */
            if (pfi && !splitLimit)
            {
                /* Generate packed file entry. */
                HlPackedFileEntry packedEntry =
                {
                    HL_ALLOC_ARR(char, fileNameUTF8Size),   /* name */
                    (HlU32)dataPos,                         /* dataPos */
                    fileEntry.dataSize                      /* dataSize */
                };

                if (!packedEntry.name)
                {
                    result = HL_ERROR_OUT_OF_MEMORY;
                    goto failed;
                }

                /* Copy UTF-8 name into packed file entry name buffer. */
                memcpy(packedEntry.name, fileNameUTF8, fileNameUTF8Size);

                /* Add packed file entry to packed file index. */
                result = HL_LIST_PUSH(pfi->entries, packedEntry);
                if (HL_FAILED(result))
                {
                    hlPackedFileEntryDestruct(&packedEntry);
                    goto failed;
                }
            }
        }
        
        /* Close archive. */
        result = hlFileClose(arcFile);
        if (HL_FAILED(result)) goto failed_close_arc;
    }

    /* Finish writing ARL if necessary. */
    if (generateARL)
    {
        size_t i, arlEOF;

        /* Write final split archive size to ARL. */
        result = hlFileWrite(arlFile, sizeof(arcSize), &arcSize, NULL);
        if (HL_FAILED(result)) goto failed_finishing_arl;

        /* Increment split count. */
        ++splitCount;

        /* Get ARL EOF position. */
        arlEOF = hlFileTell(arlFile);

        /* Jump to ARL split count position. */
        result = hlFileJumpTo(arlFile, 4);
        if (HL_FAILED(result)) goto failed_finishing_arl;

        /* Fill-in ARL split count. */
        result = hlFileWrite(arlFile, sizeof(splitCount), &splitCount, NULL);
        if (HL_FAILED(result)) goto failed_finishing_arl;

        /* Jump to end of ARL. */
        result = hlFileJumpTo(arlFile, arlEOF);
        if (HL_FAILED(result)) goto failed_finishing_arl;

        /* Write file names to ARL. */
        for (i = 0; i < arc->entries.count; ++i)
        {
#ifdef HL_IN_WIN32_UNICODE
            char fileNameUTF8Buf[255];
#endif

            const HlArchiveEntry* entry = &arc->entries.data[i];
            const HlNChar* entryName;
            const char* fileNameUTF8;
            size_t fileNameUTF8Size;
            HlU8 fileNameUTF8Len;

            /* Skip streaming and directory entries. */
            if (!hlArchiveEntryIsRegularFile(entry))
                continue;

            /* Get entry name and compute required size to convert to UTF-8. */
            entryName = hlArchiveEntryGetName(entry);
            fileNameUTF8Size = hlStrGetReqLenNativeToUTF8(entryName, 0);

            /*
               Get entry name length (size - 1) as a single byte we can write to the ARL.

               NOTE: We already verified the names could all fit within 255 characters
               when we wrote the archives, so this should be safe.
            */
            fileNameUTF8Len = (HlU8)(fileNameUTF8Size - 1);

            /* Convert file name to UTF-8 if necessary. */
#ifdef HL_IN_WIN32_UNICODE
            if (!hlStrConvNativeToUTF8NoAlloc(entryName,
                fileNameUTF8Buf, fileNameUTF8Size, 256))
            {
                result = HL_ERROR_INVALID_DATA;
                goto failed_finishing_arl;
            }

            fileNameUTF8 = fileNameUTF8Buf;
#else
            fileNameUTF8 = entryName;
#endif

            /* Write file name length (size - 1) to ARL. */
            result = hlFileWrite(arlFile, 1, &fileNameUTF8Len, NULL);
            if (HL_FAILED(result)) goto failed_finishing_arl;

            /* Write file name to ARL without null terminator. */
            result = hlFileWrite(arlFile, fileNameUTF8Size - 1, fileNameUTF8, NULL);
            if (HL_FAILED(result)) goto failed_finishing_arl;
        }
        
        /* Close ARL. */
        result = hlFileClose(arlFile);
    }

    /* Close/free everything and return result. */
    goto end;

failed:
    hlFileClose(arcFile);

failed_open_arc:
failed_close_arc:
failed_finishing_arl:
    hlFileClose(arlFile);

failed_open_arl:
end:
    if (pathBufPtr != pathBuf)
    {
        hlFree(pathBufPtr);
    }

    return result;
}

HlResult hlHHPackedFileIndexV0Write(
    const HlPackedFileIndex* HL_RESTRICT pfi,
    size_t dataPos, HlOffTable* HL_RESTRICT offTable,
    HlFile* HL_RESTRICT file)
{
    size_t i, curOffPos, eofPos;
    HlResult result;

    /* Write PFI header. */
    {
        /* Generate PFI header. */
        HlHHPackedFileIndexV0 hhPfiHeader =
        {
            (HlU32)pfi->entries.count,      /* entryCount */
            sizeof(HlHHPackedFileIndexV0)   /* entriesOffset */
        };

        /* Endian-swap if necessary. */
#ifndef HL_IS_BIG_ENDIAN
        hlHHPackedFileIndexV0Swap(&hhPfiHeader, HL_TRUE);
#endif

        /* Write PFI header. */
        result = hlFileWrite(file, sizeof(hhPfiHeader), &hhPfiHeader, NULL);
        if (HL_FAILED(result)) return result;

        /* Add entries offset to offset table. */
        result = HL_LIST_PUSH(*offTable, dataPos +
            offsetof(HlHHPackedFileIndexV0, entriesOffset));

        if (HL_FAILED(result)) return result;
    }

    /* Get current offset position. */
    curOffPos = hlFileTell(file);

    /* Write placeholder entry offsets. */
    result = hlFileWriteNulls(file, sizeof(HlU32) *
        pfi->entries.count, NULL);

    if (HL_FAILED(result)) return result;

    /* Get EOF position. */
    eofPos = hlFileTell(file);

    /* Write entries and fill-in placeholder offsets. */
    for (i = 0; i < pfi->entries.count; ++i)
    {
        const HlPackedFileEntry* pfiEntry = &pfi->entries.data[i];
        HlU32 curEntryRelPos = (HlU32)(eofPos - dataPos);

        /* Generate HH packed file entry. */
        HlHHPackedFileEntry entry =
        {
            (curEntryRelPos + sizeof(HlHHPackedFileEntry)), /* nameOffset */
            pfiEntry->dataPos,                              /* dataPos */
            pfiEntry->dataSize                              /* dataSize */
        };

        /* Jump to entry offset. */
        result = hlFileJumpTo(file, curOffPos);
        if (HL_FAILED(result)) return result;

        /* Endian-swap entry offset if necessary. */
#ifndef HL_IS_BIG_ENDIAN
        hlSwapU32P(&curEntryRelPos);
#endif

        /* Fill-in entry offset. */
        result = hlFileWrite(file, sizeof(curEntryRelPos),
            &curEntryRelPos, NULL);

        if (HL_FAILED(result)) return result;

        /* Add current entry offset to offset table. */
        result = HL_LIST_PUSH(*offTable, curOffPos);
        if (HL_FAILED(result)) return result;

        /* Increase current offset position. */
        curOffPos += sizeof(HlU32);

        /* Jump to EOF. */
        result = hlFileJumpTo(file, eofPos);
        if (HL_FAILED(result)) return result;

        /* Endian-swap entry if necessary. */
#ifndef HL_IS_BIG_ENDIAN
        hlHHPackedFileEntrySwap(&entry, HL_TRUE);
#endif

        /* Write entry. */
        result = hlFileWrite(file, sizeof(entry), &entry, NULL);
        if (HL_FAILED(result)) return result;

        /* Write entry name. */
        result = hlFileWriteString(file, pfiEntry->name, NULL);
        if (HL_FAILED(result)) return result;

        /* Add name offset to offset table. */
        result = HL_LIST_PUSH(*offTable, eofPos);
        if (HL_FAILED(result)) return result;

        /* Write padding. */
        result = hlFilePad(file, 4);
        if (HL_FAILED(result)) return result;

        /* Get EOF position. */
        eofPos = hlFileTell(file);
    }

    return result;
}

HlResult hlHHPackedFileIndexWrite(
    const HlPackedFileIndex* HL_RESTRICT pfi,
    HlU32 version, size_t dataPos,
    HlOffTable* HL_RESTRICT offTable,
    HlFile* HL_RESTRICT file)
{
    /* Ensure version is supported. */
    if (version != 0) return HL_ERROR_UNSUPPORTED;

    /* Write PFI data. */
    return hlHHPackedFileIndexV0Write(pfi,
        dataPos, offTable, file);
}

HlResult hlHHPackedFileIndexSave(
    const HlPackedFileIndex* HL_RESTRICT pfi,
    HlU32 version, const HlNChar* HL_RESTRICT filePath)
{
    HlOffTable offTable;
    HlFile* file;
    HlResult result;

    /* Initialize offset table. */
    HL_LIST_INIT(offTable);

    /* Open file. */
    result = hlFileOpen(filePath, HL_FILE_MODE_WRITE, &file);
    if (HL_FAILED(result)) return result;

    /* Start writing HH standard header. */
    result = hlHHStandardStartWrite(file, version);
    if (HL_FAILED(result)) goto failed;

    /* Write HH PFI data. */
    result = hlHHPackedFileIndexWrite(pfi, version,
        sizeof(HlHHStandardHeader), &offTable, file);

    if (HL_FAILED(result)) goto failed;

    /* Finish writing HH standard header. */
    result = hlHHStandardFinishWrite(0,
        HL_TRUE, &offTable, file);

    if (HL_FAILED(result)) goto failed;

    /* Free lists, close file, and return result. */
    HL_LIST_FREE(offTable);
    return hlFileClose(file);

failed:
    HL_LIST_FREE(offTable);
    hlFileClose(file);
    return result;
}
