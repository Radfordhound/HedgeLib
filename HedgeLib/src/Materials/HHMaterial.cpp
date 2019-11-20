#include "HedgeLib/Materials/HHMaterial.h"
#include "HedgeLib/Math/Vector.h"

namespace hl
{
    void HHMaterialParameter::EndianSwapRecursive(bool isBigEndian)
    {
        Swap(Flag1);
        Swap(Flag2);
        Value->EndianSwap();
    }

    void HHMaterial::EndianSwapRecursive(bool isBigEndian)
    {
        // Header
        EndianSwap();

        // Textures
        for (std::uint8_t i = 0; i < TextureCount; ++i)
        {
            Textures[i]->EndianSwap();
        }

        // Parameters
        for (std::uint8_t i = 0; i < ParameterCount; ++i)
        {
            Parameters[i]->EndianSwapRecursive(isBigEndian);
        }
    }
}
