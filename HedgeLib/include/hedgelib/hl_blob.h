#ifndef HL_BLOB_H_INCLUDED
#define HL_BLOB_H_INCLUDED
#include "io/hl_file.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HlBlob
{
    /** @brief Pointer to the data this blob contains. */
    void* data;
    /** @brief Size of the data this blob contains, in bytes. */
    size_t size;
}
HlBlob;

HL_API HlResult hlBlobRead(HlFile* HL_RESTRICT file,
    HlBlob* HL_RESTRICT * HL_RESTRICT blob);

HL_API HlResult hlBlobLoad(const HlNChar* HL_RESTRICT filePath,
    HlBlob* HL_RESTRICT * HL_RESTRICT blob);

#ifdef __cplusplus
}
#endif
#endif
