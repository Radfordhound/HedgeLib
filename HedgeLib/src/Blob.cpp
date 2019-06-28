#include "HedgeLib/Blob.h"
#include "HedgeLib/IO/HedgehogEngine.h"
#include "HedgeLib/IO/BINA.h"
#include "INBlob.h"

enum HL_BLOB_TYPE hl_GetType(const struct hl_Blob* blob)
{
    return blob->Type;
}

const void* hl_GetData(const struct hl_Blob* blob)
{
    switch (blob->Type)
    {
    case HL_BLOB_TYPE_HEDGEHOG_ENGINE:
        return hl_HHGetData(blob);

    case HL_BLOB_TYPE_BINA:
        return hl_BINAGetData(blob);

    default:
        return nullptr;
    }
}

void hl_FreeBlob(struct hl_Blob* blob)
{
    switch (blob->Type)
    {
    case HL_BLOB_TYPE_HEDGEHOG_ENGINE:
        hl_HHFreeBlob(blob);
        break;

    case HL_BLOB_TYPE_BINA:
        hl_BINAFreeBlob(blob);
        break;

    default: // This shouldn't normally be the case
        std::free(blob);
        break;
    }
}
