#pragma once
#include "../Endian.h"

namespace hl
{
    struct AABB
    {
        float StartX;
        float EndX;
        float StartY;
        float EndY;
        float StartZ;
        float EndZ;

        inline AABB() = default;
        constexpr AABB(float startX, float endX, float startY,
            float endY, float startZ, float endZ) : StartX(startX),
            EndX(endX), StartY(startY), EndY(endY), StartZ(startZ),
            EndZ(endZ) {}

        // TODO: Add additonal operators and such

        inline void EndianSwap()
        {
            Swap(StartX);
            Swap(EndX);
            Swap(StartY);
            Swap(EndY);
            Swap(StartZ);
            Swap(EndZ);
        }
    };
}
