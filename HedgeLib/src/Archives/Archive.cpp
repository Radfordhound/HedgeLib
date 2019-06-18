#include "HedgeLib/Archives/Archive.h"
#include "HedgeLib/Archives/PACx.h"
#include "../INBlob.h"

void hl_ExtractArchive(const struct hl_Blob* blob, const char* dir)
{
    switch (blob->Type)
    {
    // .ar/.pfd (Unleashed/Generations)
    case HL_BLOB_TYPE_HEDGEHOG_ENGINE:
        // TODO: AR Support
        break;

    // .pac (LW/Forces)
    case HL_BLOB_TYPE_BINA:
        hl_ExtractPACxArchive(blob, dir);
        break;

    default:
        // TODO: Should we return an error or something??
        break;
    }
}
