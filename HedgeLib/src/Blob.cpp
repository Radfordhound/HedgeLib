#include "HedgeLib/Blob.h"
#include "HedgeLib/IO/HedgehogEngine.h"
#include "HedgeLib/IO/BINA.h"
#include "INBlob.h"

enum HL_BLOB_FORMAT hl_BlobGetFormat(const struct hl_Blob* blob)
{
    return blob->Format;
}

const void* hl_BlobGetData(const struct hl_Blob* blob)
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

void hl_FreeBlob(struct hl_Blob* blob)
{
    switch (blob->Format)
    {
    case HL_BLOB_FORMAT_HEDGEHOG_ENGINE:
        hl_HHFreeBlob(blob);
        break;

    case HL_BLOB_FORMAT_BINA:
        hl_BINAFreeBlob(blob);
        break;

    default:
        std::free(blob);
        break;
    }
}
