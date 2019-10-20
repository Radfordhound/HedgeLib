#pragma once
#include "HedgeLib.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum HL_BLOB_FORMAT
{
    HL_BLOB_FORMAT_GENERIC = 0,
    HL_BLOB_FORMAT_HEDGEHOG_ENGINE,
    HL_BLOB_FORMAT_BINA,
    HL_BLOB_FORMAT_PACX
}
HL_BLOB_FORMAT;

typedef struct hl_Blob hl_Blob;

HL_API hl_Blob* hl_CreateBlob(size_t size, HL_BLOB_FORMAT format);
HL_API HL_BLOB_FORMAT hl_BlobGetFormat(const hl_Blob* blob);
HL_API const void* hl_BlobGetRawData(const hl_Blob* blob);
HL_API const void* hl_BlobGetData(const hl_Blob* blob);

#ifdef __cplusplus
}

#include "Memory.h"

using hl_BlobPtr = hl_CPtr<hl_Blob>;

// Non-const overloads
inline void* hl_BlobGetRawData(hl_Blob* blob)
{
    return const_cast<void*>(hl_BlobGetRawData(
        const_cast<const hl_Blob*>(blob)));
}

inline void* hl_BlobGetData(hl_Blob* blob)
{
    return const_cast<void*>(hl_BlobGetData(
        const_cast<const hl_Blob*>(blob)));
}

// Helper functions
template<typename T>
inline T* hl_BlobGetData(hl_Blob* blob)
{
    return static_cast<T*>(hl_BlobGetData(blob));
}

template<typename T>
inline const T* hl_BlobGetData(const hl_Blob* blob)
{
    return static_cast<const T*>(hl_BlobGetData(blob));
}
#endif
