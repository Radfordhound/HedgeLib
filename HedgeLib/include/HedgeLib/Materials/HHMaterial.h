#pragma once
#include "HedgeLib/Textures/HHTexture.h"
#include "HedgeLib/HedgeLib.h"

namespace hl
{
    struct Vector4;

    struct HHMaterialParameter
    {
        std::uint16_t Flag1;
        std::uint16_t Flag2;
        StringOffset32 Name;
        DataOffset32<Vector4> Value;

        inline void EndianSwap()
        {
            Swap(Flag1);
            Swap(Flag2);
        }

        HL_API void EndianSwapRecursive(bool isBigEndian);
    };

    struct HHMaterial
    {
        StringOffset32 Shader;

        // I thought the first one was VertexShader and this was PixelShader, but from
        // testing it appears that this one is just unused, at least in LW and Forces.
        // Not sure if it's used in Unleashed/Generations.
        StringOffset32 UnknownName;

        DataOffset32<StringOffset32> Texset;
        DataOffset32<DataOffset32<HHTexture>> Textures;

        std::uint8_t MaterialFlag;
        std::uint8_t NoBackFaceCulling;
        std::uint8_t AdditiveBlending;
        std::uint8_t UnknownFlag1;

        std::uint8_t ParameterCount;
        std::uint8_t Padding1; // Always 0?
        std::uint8_t UnknownFlag2;
        std::uint8_t TextureCount;

        DataOffset32<DataOffset32<HHMaterialParameter>> Parameters;
        std::uint32_t Padding2;
        std::uint32_t Unknown1;

        inline void EndianSwap()
        {
            Swap(Padding2);
            Swap(Unknown1);
        }

        HL_API void EndianSwapRecursive(bool isBigEndian);
    };
}
