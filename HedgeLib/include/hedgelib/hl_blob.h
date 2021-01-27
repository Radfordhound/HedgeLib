#ifndef HL_BLOB_H_INCLUDED
#define HL_BLOB_H_INCLUDED
#include "hl_list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HlStream HlStream;

typedef struct HlBlob
{
    /**
        @brief Pointer to the data this blob contains.

        IMPORTANT: If you manually set this value, you will have
        to manually free it yourself later; hlBlobFree will
        NOT free the data pointer for you in that case.
        
        This does not apply to hlBlobCreate, hlBlobRead,
        hlBlobLoad, or any other HedgeLib function that
        returns an HlBlob; unless otherwise stated, the data
        allocated by those functions will also be freed
        automatically when hlBlobFree is called.
    */
    void* data;
    /** @brief Size of the data this blob contains, in bytes. */
    size_t size;
}
HlBlob;

typedef HL_LIST(HlBlob*) HlBlobList;

/** @brief The size of an HlBlob, aligned to a 16-byte offset. */
#define HL_BLOB_ALIGNED_SIZE    HL_ALIGN(sizeof(HlBlob), 16)

HL_API HlResult hlBlobCreate(const void* HL_RESTRICT initialData,
    size_t size, HlBlob* HL_RESTRICT * HL_RESTRICT blob);

HL_API HlResult hlBlobRead(HlStream* HL_RESTRICT stream,
    HlBlob* HL_RESTRICT * HL_RESTRICT blob);

HL_API HlResult hlBlobLoad(const HlNChar* HL_RESTRICT filePath,
    HlBlob* HL_RESTRICT * HL_RESTRICT blob);

HL_API void hlBlobFree(HlBlob* blob);
HL_API void hlBlobListFree(HlBlobList* blobList);

#ifdef __cplusplus
}
#endif
#endif
