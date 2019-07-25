#pragma once
#include "HedgeLib/Blob.h"
#include <cstddef>
#include <cstdlib>

struct hl_Blob
{
    HL_BLOB_FORMAT Format;  // The general format of the data contained within the blob. (e.g. BINA)
    uint16_t Type;          // The specific type of the data contained within the blob. (e.g. LW .pac)
    uint8_t Data;           // The first byte of data. Do &Data to get data pointer.

    template<typename T>
    inline T* GetData()
    {
        return reinterpret_cast<T*>(&Data);
    }

    template<typename T>
    inline const T* GetData() const
    {
        return reinterpret_cast<const T*>(&Data);
    }
};

constexpr static std::size_t hl_INBlobHeaderSize = 4;

inline hl_Blob* hl_INCreateBlob(std::size_t size,
    HL_BLOB_FORMAT format, uint16_t type = 0)
{
    // Create blob with extra room for type
    hl_Blob* blob = static_cast<hl_Blob*>(
        std::malloc(hl_INBlobHeaderSize + size));

    // Out of memory check
    if (!blob) return nullptr;

    // Set values and return
    blob->Format = format;
    blob->Type = type;
    return blob;
}
