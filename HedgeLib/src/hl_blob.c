#include "hedgelib/hl_blob.h"
#include "hedgelib/hl_memory.h"
#include "hedgelib/io/hl_file.h"

/** @brief The size of an HlBlob, aligned to a 16-byte offset. */
static const size_t hlINBlobAlignedSize = (sizeof(HlBlob) + (sizeof(HlBlob) % 16));

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
    blobBuf = (HlBlob*)hlAlloc(hlINBlobAlignedSize + fileSize);
    if (!blobBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Construct blob. */
    blobBuf->data = HL_ADD_OFF(blobBuf, hlINBlobAlignedSize);
    blobBuf->size = fileSize;

    /* Read all data into blob. */
    result = hlFileRead(file, fileSize, blobBuf->data, NULL);
    if (HL_FAILED(result))
    {
        hlFree(blobBuf);
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
