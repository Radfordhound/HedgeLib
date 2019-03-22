#pragma once
#include "Reflect.h"

namespace HedgeLib::Math
{
    struct AABB
    {
        float StartX = 0;
        float EndX = 0;
        float StartY = 0;
        float EndY = 0;
        float StartZ = 0;
        float EndZ = 0;

        constexpr AABB() = default;

        // TODO: More constructors
        // TODO: AABB-Math functions

        ENDIAN_SWAP(StartX, EndX, StartY, EndY, StartZ, EndZ);
    };
}
