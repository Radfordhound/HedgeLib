#pragma once
#include <cstdint>

namespace HedgeLib::Geometry
{
    enum VertexDataFormat : std::uint32_t
    {
        VERTEX_FORMAT_UNKNOWN = 0,
        VERTEX_FORMAT_BYTE4,            // byte[4]
        VERTEX_FORMAT_VECTOR2,          // float X, float Y
        VERTEX_FORMAT_VECTOR2_HALF,     // half X, half Y
        VERTEX_FORMAT_VECTOR3,          // float X, float Y, float Z
        VERTEX_FORMAT_VECTOR3_HH1,      // crazy thing held in a uint
        VERTEX_FORMAT_VECTOR3_HH2,      // crazy thing held in a uint
        VERTEX_FORMAT_VECTOR4,          // float X, float Y, float Z, float W
        VERTEX_FORMAT_VECTOR4_BYTE,     // byte X, byte Y, byte Z, byte W
    };

    // Setup to closely match HLSL Semantics
    enum VertexDataType : std::uint16_t
    {
        VERTEX_TYPE_UNKNOWN,
        VERTEX_TYPE_POSITION,
        VERTEX_TYPE_POSITIONT,
        VERTEX_TYPE_BLEND_WEIGHT,
        VERTEX_TYPE_BLEND_INDICES,
        VERTEX_TYPE_NORMAL,
        VERTEX_TYPE_TEXCOORD,
        VERTEX_TYPE_TESS_FACTOR,
        VERTEX_TYPE_PSIZE,
        VERTEX_TYPE_FOG,
        VERTEX_TYPE_TANGENT,
        VERTEX_TYPE_BINORMAL,
        VERTEX_TYPE_COLOR
    };

    struct VertexElement
    {
        std::uint32_t Offset = 0;
        VertexDataFormat Format = VERTEX_FORMAT_UNKNOWN;
        VertexDataType Type = VERTEX_TYPE_UNKNOWN;
        std::uint8_t Index = 0;

        // TODO: Doesn't having this operator make this non-POD?
        // TODO: Does that even matter in this case?? This isn't a direct type you bimbo
        /*bool operator==(const VertexElement& b) const noexcept
        {
            return (Offset == b.Offset && Format == b.Format &&
                Type == b.Type && Index == b.Index);
        }*/
    };
}
