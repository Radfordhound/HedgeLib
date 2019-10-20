#pragma once
#include "HedgeLib/Blob.h"
#include <cstdint>
#include <cstdlib>

struct hl_Blob
{
    std::uint16_t Format;   // The general format of the data contained within the blob. (e.g. BINA)
    std::uint16_t Type;     // The specific type of the data contained within the blob. (e.g. LW .pac)
    std::uint8_t Data;      // The first byte of data. Do &Data to get data pointer.

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

inline hl_Blob* hl_INCreateBlob(size_t size,
    HL_BLOB_FORMAT format, std::uint16_t type = 0)
{
    // Create blob with extra room for type
    hl_Blob* blob = static_cast<hl_Blob*>(
        std::malloc(offsetof(hl_Blob, Data) + size));

    // Out of memory check
    if (!blob) return nullptr;

    // Set values and return
    blob->Format = static_cast<std::uint16_t>(format);
    blob->Type = type;
    return blob;
}
