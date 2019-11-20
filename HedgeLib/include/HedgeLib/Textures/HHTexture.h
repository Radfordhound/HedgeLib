#pragma once
#include "HedgeLib/Offsets.h"

namespace hl
{
    using HHTexset = ArrayOffset32<StringOffset32>;

    struct HHTexture
    {
        StringOffset32 TextureName;
        std::uint32_t Flags;
        StringOffset32 Type;

        inline void EndianSwap()
        {
            Swap(Flags);
        }
    };
}
