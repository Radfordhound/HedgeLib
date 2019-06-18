#pragma once
#include "HedgeLib.h"

#ifdef __cplusplus
extern "C" {
#endif

enum HL_BLOB_TYPE
{
    HL_BLOB_TYPE_UNKNOWN = 0,
    HL_BLOB_TYPE_HEDGEHOG_ENGINE,
    HL_BLOB_TYPE_BINA
};

struct hl_Blob;

HL_API enum HL_BLOB_TYPE hl_GetType(struct hl_Blob* blob);
HL_API void* hl_GetData(struct hl_Blob* blob);
HL_API void hl_FreeBlob(struct hl_Blob* blob);

#ifdef __cplusplus
}

template<typename T>
inline T* hl_GetData(struct hl_Blob* blob)
{
    return static_cast<T*>(hl_GetData(blob));
}
#endif
