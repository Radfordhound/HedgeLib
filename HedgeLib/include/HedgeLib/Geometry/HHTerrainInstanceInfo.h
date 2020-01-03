#pragma once
#include "../HedgeLib.h"
#include "../Offsets.h"
#include "../Math/Matrix.h"

namespace hl
{
    struct HHTerrainInstanceInfoV0
    {
        StringOffset32 ModelName;
        DataOffset32<Matrix4x4> Matrix;
        StringOffset32 Filename;

        inline void EndianSwapRecursive(bool isBigEndian)
        {
            Matrix->EndianSwap();
        }
    };

    // TODO: Other versions
}
