#pragma once
#include "HedgeLib/Blob.h"
#include <cstdint>
#include <cstddef>
#include <cstdlib>

struct hl_Blob
{
    HL_BLOB_TYPE Type;  // The type of data contained within the blob.
    std::uint8_t Data;  // The first byte of data. Do &Data to get data pointer.

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

constexpr static std::size_t hl_INBlobHeaderSize = sizeof(HL_BLOB_TYPE);

inline hl_Blob* hl_INCreateBlob(HL_BLOB_TYPE type, std::size_t size)
{
    // Create blob with extra room for type
    hl_Blob* blob = static_cast<hl_Blob*>(
        std::malloc(hl_INBlobHeaderSize + size));

    // Out of memory check
    if (!blob) return nullptr;

    // Set type and return
    blob->Type = type;
    return blob;
}
