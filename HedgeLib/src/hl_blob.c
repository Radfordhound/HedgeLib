#include "hedgelib/hl_blob.h"
#include "hedgelib/io/hl_file.h"
#include <string.h>

HlResult hlBlobCreate(const void* HL_RESTRICT initialData,
    size_t size, HlBlob* HL_RESTRICT* HL_RESTRICT blob)
{
    HlBlob* blobBuf;

    /* Allocate blob buffer. */
    blobBuf = (HlBlob*)hlAlloc(HL_BLOB_ALIGNED_SIZE + size);
    if (!blobBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Construct blob. */
    blobBuf->data = HL_ADD_OFF(blobBuf, HL_BLOB_ALIGNED_SIZE);
    blobBuf->size = size;

    /* Copy initial data into blob, if any. */
    if (initialData)
    {
        memcpy(blobBuf->data, initialData, size);
    }

    /* Set blob pointer, and return success. */
    *blob = blobBuf;
    return HL_RESULT_SUCCESS;
}

HlResult hlBlobRead(HlStream* HL_RESTRICT stream,
    HlBlob* HL_RESTRICT * HL_RESTRICT blob)
{
    HlBlob* blobBuf;
    size_t size;
    HlResult result;

    /* Get stream size. */
    result = hlStreamGetSize(stream, &size);
    if (HL_FAILED(result)) return result;

    /* Allocate blob buffer. */
    blobBuf = (HlBlob*)hlAlloc(HL_BLOB_ALIGNED_SIZE + size);
    if (!blobBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Construct blob. */
    blobBuf->data = HL_ADD_OFF(blobBuf, HL_BLOB_ALIGNED_SIZE);
    blobBuf->size = size;

    /* Read all data into blob. */
    result = hlStreamRead(stream, size, blobBuf->data, NULL);
    if (HL_FAILED(result))
    {
        hlBlobFree(blobBuf);
        return result;
    }

    /* Set blob pointer, and return success. */
    *blob = blobBuf;
    return HL_RESULT_SUCCESS;
}

HlResult hlBlobLoad(const HlNChar* HL_RESTRICT filePath,
    HlBlob* HL_RESTRICT * HL_RESTRICT blob)
{
    HlFileStream* file;
    HlResult result;

    /* Open the file at the given path. */
    result = hlFileStreamOpen(filePath, HL_FILE_MODE_READ, &file);
    if (HL_FAILED(result)) return result;

    /* Read the blob. */
    result = hlBlobRead(file, blob);
    if (HL_FAILED(result))
    {
        hlFileStreamClose(file);
        return result;
    }

    /* Close the stream and return result. */
    return hlFileStreamClose(file);
}

void hlBlobFree(HlBlob* blob)
{
    hlFree(blob);
}

void hlBlobListFree(HlBlobList* blobList)
{
    size_t i;
    if (!blobList) return;

    for (i = 0; i < blobList->count; ++i)
    {
        hlBlobFree(blobList->data[i]);
    }

    HL_LIST_FREE(*blobList);
}
