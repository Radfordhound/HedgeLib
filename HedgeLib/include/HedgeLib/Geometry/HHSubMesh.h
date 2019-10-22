#pragma once
#include "../Endian.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct hl_HHTextureUnit
{
    HL_STR32 Name;
    uint32_t ID; // TODO: Why does this look like it's little endian? Is this actually a byte??

    HL_DECL_ENDIAN_SWAP_CPP();
}
hl_HHTextureUnit;

HL_STATIC_ASSERT_SIZE(hl_HHTextureUnit, 8);
HL_DECL_ENDIAN_SWAP(hl_HHTextureUnit);

typedef enum HL_HHVERTEX_DATA_FORMAT
{
    HL_HHVERTEX_FORMAT_UNKNOWN = 0,
    // TODO: Index_Byte might actually just be a uint as BLENDINDICES semantic in HLSL is a uint
    // probably not actually since they seem to be signed.
    HL_HHVERTEX_FORMAT_INDEX_BYTE = 0x001A2286,     // N
    HL_HHVERTEX_FORMAT_INDEX = 0x001A2386,          // N, S
    HL_HHVERTEX_FORMAT_VECTOR2 = 0x002C23A5,        // N, S, 4
    HL_HHVERTEX_FORMAT_VECTOR2_HALF = 0x002C235F,   // N, S
    HL_HHVERTEX_FORMAT_VECTOR3 = 0x002A23B9,        // N, S, 4
    HL_HHVERTEX_FORMAT_VECTOR3_HH1 = 0x002A2190,    // S
    HL_HHVERTEX_FORMAT_VECTOR3_HH2 = 0x002A2187,    // S
    HL_HHVERTEX_FORMAT_VECTOR4 = 0x001A23A6,        // N, S, 4
    HL_HHVERTEX_FORMAT_VECTOR4_BYTE = 0x001A2086,
    HL_HHVERTEX_FORMAT_LAST_ENTRY = 0xFFFFFFFF
}
HL_HHVERTEX_DATA_FORMAT;

// [Ignore these comments, just trying to figure out if those values above are flags?]
// UUUU UUUU UUUU UUUU
// UUUU UUNS UU4U UUUU

// N = Not normalized
// S = Signed
// 4 = 32-bit?

typedef enum HL_HHVERTEX_DATA_TYPE
{
    HL_HHVERTEX_TYPE_POSITION = 0,
    HL_HHVERTEX_TYPE_BONE_WEIGHT = 1,
    HL_HHVERTEX_TYPE_BONE_INDEX = 2,
    HL_HHVERTEX_TYPE_NORMAL = 3,
    HL_HHVERTEX_TYPE_UV = 5,
    HL_HHVERTEX_TYPE_TANGENT = 6,
    HL_HHVERTEX_TYPE_BINORMAL = 7,
    HL_HHVERTEX_TYPE_COLOR = 10
}
HL_HHVERTEX_DATA_TYPE;

typedef struct hl_HHVertexElement
{
    uint32_t Offset;
    uint32_t Format;
    uint16_t Type;
    uint8_t Index;
    uint8_t Padding;

    HL_DECL_ENDIAN_SWAP_CPP();
}
hl_HHVertexElement;

HL_STATIC_ASSERT_SIZE(hl_HHVertexElement, 12);
HL_DECL_ENDIAN_SWAP(hl_HHVertexElement);

typedef struct hl_HHSubMesh
{
    HL_STR32 MaterialName;
    HL_ARR32(uint16_t) Faces;
    uint32_t VertexCount;
    uint32_t VertexSize;
    HL_OFF32(uint8_t) Vertices;
    HL_OFF32(hl_HHVertexElement) VertexElements;
    HL_ARR32(uint8_t) Bones;
    HL_ARR32(HL_OFF32(hl_HHTextureUnit)) TextureUnits;

    HL_DECL_ENDIAN_SWAP_CPP();
    HL_DECL_ENDIAN_SWAP_RECURSIVE_CPP();
}
hl_HHSubMesh;

HL_STATIC_ASSERT_SIZE(hl_HHSubMesh, 0x2C);
HL_DECL_ENDIAN_SWAP(hl_HHSubMesh);
HL_DECL_ENDIAN_SWAP_RECURSIVE(hl_HHSubMesh);

#ifdef __cplusplus
}
#endif
