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

HL_API enum HL_BLOB_TYPE hl_GetType(const struct hl_Blob* blob);
HL_API const void* hl_GetData(const struct hl_Blob* blob);
HL_API void hl_FreeBlob(struct hl_Blob* blob);

#ifdef __cplusplus
}

template<typename T>
inline T* hl_GetData(struct hl_Blob* blob)
{
    return const_cast<T*>(static_cast<const T*>(
        hl_GetData(blob)));
}

template<typename T>
inline const T* hl_GetData(const struct hl_Blob* blob)
{
    return static_cast<const T*>(hl_GetData(blob));
}
#endif
