#include "HedgeLib/Blob.h"
#include "HedgeLib/IO/HedgehogEngine.h"
#include "HedgeLib/IO/BINA.h"
#include "INBlob.h"

hl_Blob* hl_CreateBlob(size_t size, HL_BLOB_FORMAT format)
{
    return hl_INCreateBlob(size, format);
}

HL_BLOB_FORMAT hl_BlobGetFormat(const hl_Blob* blob)
{
    return static_cast<HL_BLOB_FORMAT>(blob->Format);
}

const void* hl_BlobGetRawData(const hl_Blob* blob)
{
    return &blob->Data;
}

const void* hl_BlobGetData(const hl_Blob* blob)
{
    switch (blob->Format)
    {
    case HL_BLOB_FORMAT_HEDGEHOG_ENGINE:
        return hl_HHGetData(blob);

    case HL_BLOB_FORMAT_BINA:
        return hl_BINAGetData(blob);

    default:
        return nullptr;
    }
}
