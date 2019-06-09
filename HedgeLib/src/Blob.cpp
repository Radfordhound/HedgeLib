#include "HedgeLib/Blob.h"
#include "HedgeLib/IO/HedgehogEngine.h"
#include "INBlob.h"

enum HL_BLOB_TYPE hl_GetType(struct hl_Blob* blob)
{
    return blob->Type;
}

void* hl_GetData(struct hl_Blob* blob)
{
    switch (blob->Type)
    {
    case HL_BLOB_TYPE_HEDGEHOG_ENGINE:
        return hl_HHGetData(blob);

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

    default: // This shouldn't normally be the case
        std::free(blob);
        break;
    }
}
