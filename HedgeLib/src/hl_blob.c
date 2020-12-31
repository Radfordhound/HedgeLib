#include "hedgelib/hl_blob.h"
#include "hedgelib/io/hl_file.h"

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

HlResult hlBlobRead(HlFile* HL_RESTRICT file,
    HlBlob* HL_RESTRICT * HL_RESTRICT blob)
{
    HlBlob* blobBuf;
    size_t fileSize;
    HlResult result;

    /* Get file size. */
    result = hlFileGetSize(file, &fileSize);
    if (HL_FAILED(result)) return result;

    /* Allocate blob buffer. */
    blobBuf = (HlBlob*)hlAlloc(HL_BLOB_ALIGNED_SIZE + fileSize);
    if (!blobBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Construct blob. */
    blobBuf->data = HL_ADD_OFF(blobBuf, HL_BLOB_ALIGNED_SIZE);
    blobBuf->size = fileSize;

    /* Read all data into blob. */
    result = hlFileRead(file, fileSize, blobBuf->data, NULL);
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
    HlFile* file;
    HlResult result;

    /* Open the file at the given path. */
    result = hlFileOpen(filePath, HL_FILE_MODE_READ, &file);
    if (HL_FAILED(result)) return result;

    /* Read the blob, close the file, and return. */
    result = hlBlobRead(file, blob);
    hlFileClose(file);

    return result;
}

void hlBlobFree(HlBlob* blob)
{
    hlFree(blob);
}
