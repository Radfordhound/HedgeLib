#pragma once
#include "../HedgeLib.h"
#include "../Offsets.h"

namespace hl
{
    struct HHTextureUnit
    {
        StringOffset32 Name;
        std::uint32_t ID; // TODO: Why does this look like it's little endian? Is this actually a byte??

        inline void EndianSwap()
        {
            Swap(ID);
        }
    };

    HL_STATIC_ASSERT_SIZE(HHTextureUnit, 8);

    enum HHVertexFormat : std::uint32_t
    {
        HHVERTEX_FORMAT_UNKNOWN = 0,
        // TODO: Index_Byte might actually just be a uint as BLENDINDICES semantic in HLSL is a uint
        // probably not actually since they seem to be signed.
        HHVERTEX_FORMAT_INDEX_BYTE = 0x001A2286,    // N
        HHVERTEX_FORMAT_INDEX = 0x001A2386,         // N, S
        HHVERTEX_FORMAT_VECTOR2 = 0x002C23A5,       // N, S, 4
        HHVERTEX_FORMAT_VECTOR2_HALF = 0x002C235F,  // N, S
        HHVERTEX_FORMAT_VECTOR3 = 0x002A23B9,       // N, S, 4
        HHVERTEX_FORMAT_VECTOR3_HH1 = 0x002A2190,   // S
        HHVERTEX_FORMAT_VECTOR3_HH2 = 0x002A2187,   // S
        HHVERTEX_FORMAT_VECTOR4 = 0x001A23A6,       // N, S, 4
        HHVERTEX_FORMAT_VECTOR4_BYTE = 0x001A2086,
        HHVERTEX_FORMAT_LAST_ENTRY = 0xFFFFFFFF
    };

    // [Ignore these comments, just trying to figure out if those values above are flags?]
    // UUUU UUUU UUUU UUUU
    // UUUU UUNS UU4U UUUU

    // N = Not normalized
    // S = Signed
    // 4 = 32-bit?

    enum HHVertexType : std::uint16_t
    {
        HHVERTEX_TYPE_POSITION = 0,
        HHVERTEX_TYPE_BONE_WEIGHT = 1,
        HHVERTEX_TYPE_BONE_INDEX = 2,
        HHVERTEX_TYPE_NORMAL = 3,
        HHVERTEX_TYPE_UV = 5,
        HHVERTEX_TYPE_TANGENT = 6,
        HHVERTEX_TYPE_BINORMAL = 7,
        HHVERTEX_TYPE_COLOR = 10
    };

    struct HHVertexElement
    {
        std::uint32_t Offset;
        std::uint32_t Format;
        std::uint16_t Type;
        std::uint8_t Index;
        std::uint8_t Padding;

        inline void EndianSwap()
        {
            Swap(Offset);
            Swap(reinterpret_cast<std::uint32_t&>(Format));
            Swap(reinterpret_cast<std::uint16_t&>(Type));
        }
    };

    HL_STATIC_ASSERT_SIZE(HHVertexElement, 12);

    struct HHSubMesh
    {
        StringOffset32 MaterialName;
        ArrayOffset32<std::uint16_t> Faces;
        std::uint32_t VertexCount;
        std::uint32_t VertexSize;
        DataOffset32<std::uint8_t> Vertices;
        DataOffset32<HHVertexElement> VertexElements;
        ArrayOffset32<std::uint8_t> Bones;
        ArrayOffset32<DataOffset32<HHTextureUnit>> TextureUnits;

        inline void EndianSwap()
        {
            Swap(Faces);
            Swap(VertexCount);
            Swap(VertexSize);
            Swap(Bones);
            Swap(TextureUnits);
        }

        HL_API void EndianSwapRecursive(bool isBigEndian);
    };

    HL_STATIC_ASSERT_SIZE(HHSubMesh, 0x2C);
}
