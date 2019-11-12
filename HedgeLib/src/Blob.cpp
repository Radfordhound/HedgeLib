#include "HedgeLib/Blob.h"
#include "HedgeLib/IO/HedgehogEngine.h"
#include "HedgeLib/IO/BINA.h"

namespace hl
{
    const void* Blob::Data() const
    {
        switch (format)
        {
        case BlobFormat::HedgehogEngine:
            return DHHGetData(*this);

        case BlobFormat::BINA:
            return DBINAGetData(*this);

        default:
            return data.get();
        }
    }
}
