#pragma once
#include "HedgeLib.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum HL_BLOB_FORMAT : uint16_t
{
    HL_BLOB_FORMAT_UNKNOWN = 0,
    HL_BLOB_FORMAT_HEDGEHOG_ENGINE,
    HL_BLOB_FORMAT_BINA
};

struct hl_Blob;

HL_API enum HL_BLOB_FORMAT hl_BlobGetFormat(const struct hl_Blob* blob);
HL_API const void* hl_BlobGetData(const struct hl_Blob* blob);
HL_API void hl_FreeBlob(struct hl_Blob* blob);

#ifdef __cplusplus
}

template<typename T>
inline T* hl_BlobGetData(struct hl_Blob* blob)
{
    return const_cast<T*>(static_cast<const T*>(
        hl_BlobGetData(blob)));
}

template<typename T>
inline const T* hl_BlobGetData(const struct hl_Blob* blob)
{
    return static_cast<const T*>(hl_BlobGetData(blob));
}
#endif
