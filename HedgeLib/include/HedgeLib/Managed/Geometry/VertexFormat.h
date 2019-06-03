#pragma once
#include <cstdint>

namespace HedgeLib::Geometry
{
    enum VertexDataFormat : std::uint32_t
    {
        HL_VERTEX_FORMAT_UNKNOWN = 0,
        HL_VERTEX_FORMAT_BYTE4,            // byte[4]
        HL_VERTEX_FORMAT_VECTOR2,          // float X, float Y
        HL_VERTEX_FORMAT_VECTOR2_HALF,     // half X, half Y
        HL_VERTEX_FORMAT_VECTOR3,          // float X, float Y, float Z
        HL_VERTEX_FORMAT_VECTOR3_HH1,      // crazy thing held in a uint
        HL_VERTEX_FORMAT_VECTOR3_HH2,      // crazy thing held in a uint
        HL_VERTEX_FORMAT_VECTOR4,          // float X, float Y, float Z, float W
        HL_VERTEX_FORMAT_VECTOR4_BYTE,     // byte X, byte Y, byte Z, byte W
    };

    // Setup to closely match HLSL Semantics
    enum VertexDataType : std::uint16_t
    {
        HL_VERTEX_TYPE_UNKNOWN,
        HL_VERTEX_TYPE_POSITION,
        HL_VERTEX_TYPE_POSITIONT,
        HL_VERTEX_TYPE_BLEND_WEIGHT,
        HL_VERTEX_TYPE_BLEND_INDICES,
        HL_VERTEX_TYPE_NORMAL,
        HL_VERTEX_TYPE_TEXCOORD,
        HL_VERTEX_TYPE_TESS_FACTOR,
        HL_VERTEX_TYPE_PSIZE,
        HL_VERTEX_TYPE_FOG,
        HL_VERTEX_TYPE_TANGENT,
        HL_VERTEX_TYPE_BINORMAL,
        HL_VERTEX_TYPE_COLOR
    };

    struct VertexElement
    {
        // TODO: Should we initialize these values?
        std::uint32_t Offset = 0;
        VertexDataFormat Format = HL_VERTEX_FORMAT_UNKNOWN;
        VertexDataType Type = HL_VERTEX_TYPE_UNKNOWN;
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
