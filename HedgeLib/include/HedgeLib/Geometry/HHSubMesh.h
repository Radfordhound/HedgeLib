#pragma once
#include "../Offsets.h"
#include "../Endian.h"
#include "../IO/IO.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hl_DHHTextureUnit
{
    HL_STR32 Name;
    uint32_t ID; // TODO: Why does this look like it's little endian? Is this actually a byte??

    HL_DECL_ENDIAN_SWAP_CPP();
    HL_DECL_WRITEO_CPP();
};

HL_DECL_ENDIAN_SWAP(hl_DHHTextureUnit);
HL_DECL_WRITEO(hl_DHHTextureUnit);

enum HL_HHVERTEX_DATA_FORMAT : uint32_t
{
    HL_HHVERTEX_FORMAT_UNKNOWN = 0,
    // TODO: Index_Byte might actually just be a uint as BLENDINDICES semantic in HLSL is a uint
    // probably not actually since they seem to be signed.
    HL_HHVERTEX_FORMAT_INDEX_BYTE = 0x001A2286,
    HL_HHVERTEX_FORMAT_INDEX = 0x001A2386,
    HL_HHVERTEX_FORMAT_VECTOR2 = 0x002C23A5,
    HL_HHVERTEX_FORMAT_VECTOR2_HALF = 0x002C235F,
    HL_HHVERTEX_FORMAT_VECTOR3 = 0x002A23B9,
    HL_HHVERTEX_FORMAT_VECTOR3_HH1 = 0x002A2190,
    HL_HHVERTEX_FORMAT_VECTOR3_HH2 = 0x002A2187,
    HL_HHVERTEX_FORMAT_VECTOR4 = 0x001A23A6,
    HL_HHVERTEX_FORMAT_VECTOR4_BYTE = 0x001A2086,
    HL_HHVERTEX_FORMAT_LAST_ENTRY = 0xFFFFFFFF
};

enum HL_HHVERTEX_DATA_TYPE : uint16_t
{
    HL_HHVERTEX_TYPE_POSITION = 0,
    HL_HHVERTEX_TYPE_BONE_WEIGHT = 1,
    HL_HHVERTEX_TYPE_BONE_INDEX = 2,
    HL_HHVERTEX_TYPE_NORMAL = 3,
    HL_HHVERTEX_TYPE_UV = 5,
    HL_HHVERTEX_TYPE_TANGENT = 6,
    HL_HHVERTEX_TYPE_BINORMAL = 7,
    HL_HHVERTEX_TYPE_COLOR = 10
};

struct hl_DHHVertexElement
{
    uint32_t Offset;
    enum HL_HHVERTEX_DATA_FORMAT Format;
    enum HL_HHVERTEX_DATA_TYPE Type;
    uint8_t Index;
    uint8_t Padding;

    HL_DECL_ENDIAN_SWAP_CPP();
    HL_DECL_WRITE_CPP();
};

HL_DECL_ENDIAN_SWAP(hl_DHHVertexElement);
HL_DECL_WRITE(hl_DHHVertexElement);

struct hl_DHHSubMesh
{
    HL_STR32 MaterialName;
    HL_ARR32(uint16_t) Faces;
    uint32_t VertexCount;
    uint32_t VertexSize;
    HL_OFF32(uint8_t) Vertices;
    HL_OFF32(hl_DHHVertexElement) VertexElements;
    HL_ARR32(uint8_t) Bones;
    HL_ARR32(HL_OFF32(hl_DHHTextureUnit)) TextureUnits;

    HL_INLN_X64_OFFSETS_CPP(MaterialName, Faces.Offset, Vertices,
        VertexElements, Bones.Offset, TextureUnits.Offset);

    HL_DECL_ENDIAN_SWAP_CPP();
    HL_DECL_ENDIAN_SWAP_RECURSIVE_CPP();
    HL_DECL_WRITEO_CPP();
};

HL_DECL_X64_OFFSETS(hl_DHHSubMesh);
HL_DECL_ENDIAN_SWAP(hl_DHHSubMesh);
HL_DECL_ENDIAN_SWAP_RECURSIVE(hl_DHHSubMesh);
HL_DECL_WRITEO(hl_DHHSubMesh);

#ifdef __cplusplus
}
#endif
