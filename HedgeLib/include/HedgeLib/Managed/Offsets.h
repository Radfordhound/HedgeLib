#pragma once
#include "HedgeLib/Direct/Offsets.h"
#include <vector>

struct hl_OffsetTable : public std::vector<long>
{
    using std::vector<long>::vector;
};

namespace HedgeLib
{
    template<typename T, typename OffsetType>
    inline T* GetAbs(void* baseAddress, OffsetType offset)
    {
        return reinterpret_cast<T*>(static_cast<uint8_t*>(
            baseAddress) + offset);
    }
}
