#include "HedgeLib/Blob.h"
#include "HedgeLib/IO/HedgehogEngine.h"
#include "HedgeLib/IO/BINA.h"
#include "HedgeLib/Archives/PACx.h"

namespace hl
{
    const void* Blob::Data() const
    {
        switch (format)
        {
        case BlobFormat::HedgehogEngine:
            return DHHGetData(data.get());

        case BlobFormat::BINA:
            return DBINAGetData(*this);

        case BlobFormat::PACx:
            return DPACxGetData(*this);

        default:
            return data.get();
        }
    }
}
