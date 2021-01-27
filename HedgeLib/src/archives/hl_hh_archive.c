#include "hl_in_archive.h"
#include "hedgelib/hl_text.h"
#include "hedgelib/hl_endian.h"
#include "hedgelib/io/hl_file.h"
#include "hedgelib/io/hl_path.h"
#include "hedgelib/io/hl_hh.h"
#include "hedgelib/archives/hl_hh_archive.h"
#include <string.h>

const HlNChar HL_HH_ARL_EXT[5] = HL_NTEXT(".arl");
const HlNChar HL_HH_PFD_EXT[5] = HL_NTEXT(".pfd");
const HlNChar HL_HH_PFI_EXT[5] = HL_NTEXT(".pfi");
const HlNChar HL_HH_AR_EXT[4] = HL_NTEXT(".ar");

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

HlResult hlHHArchiveParseInto(const HlHHArchiveHeader* HL_RESTRICT hhArc,
    size_t hhArcSize, HlArchive* HL_RESTRICT hlArc)
{
    /* Get start position and end position. */
    const unsigned char* curPos = HL_ADD_OFFC(hhArc, sizeof(HlHHArchiveHeader));
    const unsigned char* endPos = HL_ADD_OFFC(hhArc, hhArcSize);
    HlResult result = HL_RESULT_SUCCESS;

    /* Setup file entries in this split. */
    while (curPos < endPos)
    {
        /* Get file entry and file name pointers. */
        const HlHHArchiveFileEntry* hhFileEntry = (const HlHHArchiveFileEntry*)curPos;
        const char* fileName = (const char*)(hhFileEntry + 1);
        const void* fileData = HL_ADD_OFFC(hhFileEntry, hhFileEntry->dataOffset);
        HlArchiveEntry hlArcEntry;
        void* dataBuf;

        /* Convert file name to native encoding and copy into buffer. */
        hlArcEntry.path = hlStrConvUTF8ToNative(fileName, 0);
        if (!hlArcEntry.path) return HL_ERROR_OUT_OF_MEMORY;

        /* Allocate new data buffer. */
        hlArcEntry.size = (size_t)hhFileEntry->dataSize;
        dataBuf = hlAlloc(hlArcEntry.size);

        if (!dataBuf)
        {
            hlFree(hlArcEntry.path);
            return HL_ERROR_OUT_OF_MEMORY;
        }

        /* Copy data. */
        memcpy(dataBuf, fileData, hlArcEntry.size);

        /* Finish setting up entry. */
        hlArcEntry.meta = 0;
        hlArcEntry.data = (HlUMax)((HlUPtr)dataBuf);

        /* Add new entry to HlArchive. */
        result = HL_LIST_PUSH(hlArc->entries, hlArcEntry);
        if (HL_FAILED(result))
        {
            hlArchiveEntryDestruct(&hlArcEntry);
            return result;
        }

        /* Go to next file entry within the archive. */
        curPos += hhFileEntry->entrySize;
    }

    return result;
}

HlResult hlHHArchiveReadInto(void* HL_RESTRICT hhArc,
    size_t hhArcSize, HlArchive* HL_RESTRICT hlArc)
{
    /* TODO: Account for endianness on big-endian machines. */

    return hlHHArchiveParseInto(
        (const HlHHArchiveHeader*)hhArc,
        hhArcSize, hlArc);
}

HlResult hlHHArchiveLoadSingleInto(
    const HlNChar* HL_RESTRICT filePath,
    HlBlobList* HL_RESTRICT hhArcs,
    HlArchive* HL_RESTRICT hlArc)
{
    HlBlob* hhArc;
    HlResult result;

    /* Load archive. */
    result = hlBlobLoad(filePath, &hhArc);
    if (HL_FAILED(result)) return result;

    /* Add this archive's blob to the blobs list if necessary. */
    if (hhArcs)
    {
        result = HL_LIST_PUSH(*hhArcs, hhArc);
        if (HL_FAILED(result))
        {
            hlBlobFree(hhArc);
            return result;
        }
    }

    /* Parse blob into HlArchive, free blob if necessary, and return. */
    if (hlArc)
    {
        result = hlHHArchiveReadInto(hhArc->data,
            hhArc->size, hlArc);
    }

    if (!hhArcs) hlBlobFree(hhArc);
    return result;
}

HlResult hlHHArchiveLoadAllInto(
    const HlNChar* HL_RESTRICT filePath,
    HlBlobList* HL_RESTRICT hhArcs,
    HlArchive* HL_RESTRICT hlArc)
{
    HlNChar pathBuf[255];
    HlNChar* pathBufPtr = pathBuf;
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
            /* Allocate a new buffer if necessary. */
            const size_t filePathSize = (filePathLen + 1);
            if (filePathSize > 255)
            {
                pathBufPtr = HL_ALLOC_ARR(HlNChar, filePathSize);
                if (!pathBufPtr) return HL_ERROR_OUT_OF_MEMORY;
            }

            /* Copy filePath into buffer. */
            memcpy(pathBufPtr, filePath, filePathSize * sizeof(HlNChar));

            /* Set lastCharPtr. */
            lastCharPtr = &pathBufPtr[filePathLen - 1];

            /* If this is not the .00 split, check if the .00 split exists. */
            if (ext[1] != HL_NTEXT('0') || ext[2] != HL_NTEXT('0'))
            {
                *(lastCharPtr - 1) = HL_NTEXT('0');
                *lastCharPtr = HL_NTEXT('0');

                /* Fallback to the given file path if .00 split does not exist. */
                if (!hlPathExists(pathBufPtr))
                {
                    *(lastCharPtr - 1) = ext[1];
                    *lastCharPtr = ext[2];
                }
            }
        }
        
        /* If the given file path is an arl, get the path to the corresponding .ar or .ar.00 */
        else if (hlNStrsEqual(ext, HL_HH_ARL_EXT))
        {
            /* Allocate a new buffer if necessary. */
            const size_t reqPathBufSize = (filePathLen + 3);
            if (reqPathBufSize > 255)
            {
                pathBufPtr = HL_ALLOC_ARR(HlNChar, reqPathBufSize);
                if (!pathBufPtr) return HL_ERROR_OUT_OF_MEMORY;
            }

            /* Copy filePath into buffer. */
            memcpy(pathBufPtr, filePath, filePathLen * sizeof(HlNChar));

            /* Change extension from .arl$ to .ar.00$ */
            pathBufPtr[filePathLen - 1] = HL_NTEXT('.');
            pathBufPtr[filePathLen]     = HL_NTEXT('0');
            pathBufPtr[filePathLen + 1] = HL_NTEXT('0');
            pathBufPtr[filePathLen + 2] = HL_NTEXT('\0');

            /* See if .ar.00 archive exists... */
            if (!hlPathExists(pathBufPtr))
            {
                /* ...If it doesn't, change extension from .ar.00$ to .ar$ */
                pathBufPtr[filePathLen - 1] = HL_NTEXT('\0');

                /* Check if the .ar$ exists, and if not, return with an error. */
                if (!hlPathExists(pathBufPtr))
                {
                    result = HL_ERROR_NOT_FOUND;
                    goto end;
                }

                /* Set filePath. */
                filePath = pathBufPtr;
            }
            else
            {
                /* Set lastCharPtr. */
                lastCharPtr = &pathBufPtr[filePathLen + 1];
            }
        }

        /*
            If the given file path has another non-split extension and it
            exists, just load it directly.
        */
        else if (hlPathExists(filePath))
        {
            result = hlHHArchiveLoadSingleInto(filePath, hhArcs, hlArc);
            goto end;
        }

        /*
            If the given file path has another non-split extension and it
            doesn't exist, see if it has any splits.
        */
        else
        {
            /* Allocate a new buffer if necessary. */
            const size_t reqPathBufSize = (filePathLen + 4);
            if (reqPathBufSize > 255)
            {
                pathBufPtr = HL_ALLOC_ARR(HlNChar, reqPathBufSize);
                if (!pathBufPtr) return HL_ERROR_OUT_OF_MEMORY;
            }

            /* Copy filePath into buffer. */
            memcpy(pathBufPtr, filePath, filePathLen * sizeof(HlNChar));

            /* Add split extension .00$ */
            pathBufPtr[filePathLen]     = HL_NTEXT('.');
            pathBufPtr[filePathLen + 1] = HL_NTEXT('0');
            pathBufPtr[filePathLen + 2] = HL_NTEXT('0');
            pathBufPtr[filePathLen + 3] = HL_NTEXT('\0');

            /* See if .00 archive exists, and if not, return with an error. */
            if (!hlPathExists(pathBufPtr))
            {
                result = HL_ERROR_NOT_FOUND;
                goto end;
            }

            /* Set lastCharPtr. */
            lastCharPtr = &pathBufPtr[filePathLen + 2];
        }
    }

    /* Load splits. */
    do
    {
        result = hlHHArchiveLoadSingleInto(pathBufPtr, hhArcs, hlArc);
    }
    while (lastCharPtr && hlINArchiveNextSplit2(
        lastCharPtr) && hlPathExists(pathBufPtr));

end:
    /* Free path buffer if we allocated it. */
    if (pathBufPtr != pathBuf) hlFree(pathBufPtr);

    /* Return result. */
    return result;
}

HlResult hlHHArchiveLoad(const HlNChar* HL_RESTRICT filePath,
    HlBool loadSplits, HlBlobList* HL_RESTRICT hhArcs,
    HlArchive* HL_RESTRICT * HL_RESTRICT hlArc)
{
    HlArchive* hlArcBuf;
    HlResult result;

    /* Allocate HlArchive buffer. */
    hlArcBuf = HL_ALLOC_OBJ(HlArchive);
    if (!hlArcBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Setup archive. */
    HL_LIST_INIT(hlArcBuf->entries);

    /*
       Add all the files from all the splits or simply a
       single archive based on user request.
    */
    result = (loadSplits) ?
        hlHHArchiveLoadAllInto(filePath, hhArcs, hlArcBuf) :
        hlHHArchiveLoadSingleInto(filePath, hhArcs, hlArcBuf);

    if (HL_FAILED(result)) return result;

    /* Set pointer and return result. */
    *hlArc = hlArcBuf;
    return result;
}

HlResult hlHHArchiveSave(const HlArchive* HL_RESTRICT arc, HlU32 splitLimit,
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
    HlFileStream *arcFile = NULL, *arlFile = NULL;
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
        const HlHHArchiveListHeader arlHeader =
        {
            HL_HH_ARL_SIG,      /* signature */
            0                   /* splitCount */
        };

        /* Copy ARL extension and null terminator into path buffer. */
        memcpy(&pathBufPtr[filePathLen], HL_HH_ARL_EXT, sizeof(HL_HH_ARL_EXT));

        /* Open ARL for writing. */
        result = hlFileStreamOpen(pathBufPtr, HL_FILE_MODE_WRITE, &arlFile);
        if (HL_FAILED(result)) goto failed_open_arl;

        /* Write placeholder ARL header. */
        result = hlStreamWrite(arlFile, sizeof(arlHeader), &arlHeader, NULL);
        if (HL_FAILED(result)) goto failed;
    }

    /* Write archive(s) and ARL split sizes as requested. */
    {
        /* Generate the header that will be written to every split. */
        const HlHHArchiveHeader arcHeader =
        {
            0,                              /* unknown1 */
            sizeof(HlHHArchiveHeader),      /* headerSize */
            sizeof(HlHHArchiveFileEntry),   /* entrySize */
            dataAlignment                   /* dataAlignment */
        };

        HlNChar* lastCharPtr = &pathBufPtr[filePathLen + nonSplitExtsLen];
        size_t i;
        HlBool wroteEntryToCurArc = HL_FALSE;
        
        /* Account for AR header. */
        arcSize = sizeof(HlHHArchiveHeader);

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
        result = hlFileStreamOpen(pathBufPtr, HL_FILE_MODE_WRITE, &arcFile);
        if (HL_FAILED(result)) goto failed_open_arc;

        /* Write AR header. */
        result = hlStreamWrite(arcFile, sizeof(arcHeader), &arcHeader, NULL);
        if (HL_FAILED(result)) goto failed;

        /* Write archive(s) and ARL split sizes as requested. */
        for (i = 0; i < arc->entries.count; ++i)
        {
#ifdef HL_IN_WIN32_UNICODE
            char fileNameUTF8Buf[256];
#endif

            HlHHArchiveFileEntry hhFileEntry;
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
            
            hhFileEntry.dataSize = (HlU32)entrySize;

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
            hhFileEntry.dataOffset = (HlU32)(sizeof(HlHHArchiveFileEntry) + fileNameUTF8Size);

            /* Account for file data alignment. */
            hhFileEntry.dataOffset = HL_ALIGN(hhFileEntry.dataOffset + arcSize, dataAlignment);
            hhFileEntry.dataOffset -= arcSize;

            /* Account for file size. */
            hhFileEntry.entrySize = (hhFileEntry.dataOffset + hhFileEntry.dataSize);

            /* Increase total archive size. */
            arcSize += hhFileEntry.entrySize;

            /* Set unknown values. */
            /* TODO: Find out what these are and set them properly. */
            hhFileEntry.unknown1 = 0;
            hhFileEntry.unknown2 = 0;

            /* Break off into next split if necessary. */
            if (splitLimit && arcSize > splitLimit && wroteEntryToCurArc)
            {
                /* Close the current split. */
                result = hlFileStreamClose(arcFile);
                if (HL_FAILED(result)) goto failed_close_arc;

                /* Write current split archive size to ARL if necessary. */
                if (generateARL)
                {
                    result = hlStreamWrite(arlFile, sizeof(arcSize), &arcSize, NULL);
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
                result = hlFileStreamOpen(pathBufPtr, HL_FILE_MODE_WRITE, &arcFile);
                if (HL_FAILED(result)) goto failed_open_arc;

                /* Write AR header. */
                result = hlStreamWrite(arcFile, sizeof(arcHeader), &arcHeader, NULL);
                if (HL_FAILED(result)) goto failed;

                /* Reset arcSize. */
                arcSize = sizeof(HlHHArchiveHeader);

                /* Indicate that we have not yet written an entry to this new split. */
                wroteEntryToCurArc = HL_FALSE;

                /* Increment split count. */
                ++splitCount;

                /* Account for entry and file name (including null terminator). */
                hhFileEntry.dataOffset = (HlU32)(sizeof(HlHHArchiveFileEntry) + fileNameUTF8Size);

                /* Account for file data alignment. */
                hhFileEntry.dataOffset = HL_ALIGN(hhFileEntry.dataOffset + arcSize, dataAlignment);
                hhFileEntry.dataOffset -= arcSize;

                /* Account for file size. */
                hhFileEntry.entrySize = (hhFileEntry.dataOffset + hhFileEntry.dataSize);

                /* Increase total archive size. */
                arcSize += hhFileEntry.entrySize;
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
            result = hlStreamWrite(arcFile, sizeof(hhFileEntry), &hhFileEntry, NULL);
            if (HL_FAILED(result))
            {
                if (doFreeEntryData) hlFree(entryData);
                goto failed;
            }

            /* Write file name to archive. */
            result = hlStreamWrite(arcFile, fileNameUTF8Size, fileNameUTF8, NULL);
            if (HL_FAILED(result))
            {
                if (doFreeEntryData) hlFree(entryData);
                goto failed;
            }

            /* Write file data padding to archive. */
            result = hlStreamPad(arcFile, dataAlignment);
            if (HL_FAILED(result))
            {
                if (doFreeEntryData) hlFree(entryData);
                goto failed;
            }

            /* Store data position. */
            dataPos = hlStreamTell(arcFile);

            /* Write file data to archive. */
            result = hlStreamWrite(arcFile, entrySize, entryData, NULL);

            /* Free entry data if necessary. */
            if (doFreeEntryData) hlFree(entryData);
            if (HL_FAILED(result)) goto failed; /* NOTE: From the hlStreamWrite above. */

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
                    hhFileEntry.dataSize                    /* dataSize */
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
        result = hlFileStreamClose(arcFile);
        if (HL_FAILED(result)) goto failed_close_arc;
    }

    /* Finish writing ARL if necessary. */
    if (generateARL)
    {
        size_t i, arlEOF;

        /* Write final split archive size to ARL. */
        result = hlStreamWrite(arlFile, sizeof(arcSize), &arcSize, NULL);
        if (HL_FAILED(result)) goto failed_finishing_arl;

        /* Increment split count. */
        ++splitCount;

        /* Get ARL EOF position. */
        arlEOF = hlStreamTell(arlFile);

        /* Jump to ARL split count position. */
        result = hlStreamJumpTo(arlFile, 4);
        if (HL_FAILED(result)) goto failed_finishing_arl;

        /* Fill-in ARL split count. */
        result = hlStreamWrite(arlFile, sizeof(splitCount), &splitCount, NULL);
        if (HL_FAILED(result)) goto failed_finishing_arl;

        /* Jump to end of ARL. */
        result = hlStreamJumpTo(arlFile, arlEOF);
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
            result = hlStreamWrite(arlFile, 1, &fileNameUTF8Len, NULL);
            if (HL_FAILED(result)) goto failed_finishing_arl;

            /* Write file name to ARL without null terminator. */
            result = hlStreamWrite(arlFile, fileNameUTF8Size - 1, fileNameUTF8, NULL);
            if (HL_FAILED(result)) goto failed_finishing_arl;
        }
        
        /* Close ARL. */
        result = hlFileStreamClose(arlFile);
    }

    /* Close/free everything and return result. */
    goto end;

failed:
    hlFileStreamClose(arcFile);

failed_open_arc:
failed_close_arc:
failed_finishing_arl:
    hlFileStreamClose(arlFile);

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
    HlStream* HL_RESTRICT stream)
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
        result = hlStreamWrite(stream, sizeof(hhPfiHeader), &hhPfiHeader, NULL);
        if (HL_FAILED(result)) return result;

        /* Add entries offset to offset table. */
        result = HL_LIST_PUSH(*offTable, dataPos +
            offsetof(HlHHPackedFileIndexV0, entriesOffset));

        if (HL_FAILED(result)) return result;
    }

    /* Get current offset position. */
    curOffPos = hlStreamTell(stream);

    /* Write placeholder entry offsets. */
    result = hlStreamWriteNulls(stream, sizeof(HlU32) *
        pfi->entries.count, NULL);

    if (HL_FAILED(result)) return result;

    /* Get EOF position. */
    eofPos = hlStreamTell(stream);

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
        result = hlStreamJumpTo(stream, curOffPos);
        if (HL_FAILED(result)) return result;

        /* Endian-swap entry offset if necessary. */
#ifndef HL_IS_BIG_ENDIAN
        hlSwapU32P(&curEntryRelPos);
#endif

        /* Fill-in entry offset. */
        result = hlStreamWrite(stream, sizeof(curEntryRelPos),
            &curEntryRelPos, NULL);

        if (HL_FAILED(result)) return result;

        /* Add current entry offset to offset table. */
        result = HL_LIST_PUSH(*offTable, curOffPos);
        if (HL_FAILED(result)) return result;

        /* Increase current offset position. */
        curOffPos += sizeof(HlU32);

        /* Jump to EOF. */
        result = hlStreamJumpTo(stream, eofPos);
        if (HL_FAILED(result)) return result;

        /* Endian-swap entry if necessary. */
#ifndef HL_IS_BIG_ENDIAN
        hlHHPackedFileEntrySwap(&entry, HL_TRUE);
#endif

        /* Write entry. */
        result = hlStreamWrite(stream, sizeof(entry), &entry, NULL);
        if (HL_FAILED(result)) return result;

        /* Write entry name. */
        result = hlStreamWriteStringUTF8(stream, pfiEntry->name, NULL);
        if (HL_FAILED(result)) return result;

        /* Add name offset to offset table. */
        result = HL_LIST_PUSH(*offTable, eofPos);
        if (HL_FAILED(result)) return result;

        /* Write padding. */
        result = hlStreamPad(stream, 4);
        if (HL_FAILED(result)) return result;

        /* Get EOF position. */
        eofPos = hlStreamTell(stream);
    }

    return result;
}

HlResult hlHHPackedFileIndexWrite(
    const HlPackedFileIndex* HL_RESTRICT pfi,
    HlU32 version, size_t dataPos,
    HlOffTable* HL_RESTRICT offTable,
    HlStream* HL_RESTRICT stream)
{
    /* Ensure version is supported. */
    if (version != 0) return HL_ERROR_UNSUPPORTED;

    /* Write PFI data. */
    return hlHHPackedFileIndexV0Write(pfi,
        dataPos, offTable, stream);
}

HlResult hlHHPackedFileIndexSave(
    const HlPackedFileIndex* HL_RESTRICT pfi,
    HlU32 version, const HlNChar* HL_RESTRICT filePath)
{
    HlOffTable offTable;
    HlFileStream* file;
    HlResult result;

    /* Initialize offset table. */
    HL_LIST_INIT(offTable);

    /* Open file. */
    result = hlFileStreamOpen(filePath, HL_FILE_MODE_WRITE, &file);
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
    return hlFileStreamClose(file);

failed:
    HL_LIST_FREE(offTable);
    hlFileStreamClose(file);
    return result;
}
