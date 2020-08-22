#ifndef HL_MODEL_H_INCLUDED
#define HL_MODEL_H_INCLUDED
#include "../hl_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HlBlob HlBlob;
typedef struct HlFile HlFile;
typedef struct HlMaterial HlMaterial;

typedef struct HlMesh
{
    HL_REF(HlMaterial) matRef;
    size_t vertexFormatIndex;
    size_t vertexCount;
    void* vertices;
    size_t faceCount;
    unsigned short* faces;

    /* TODO: Texture stuff */
    /*size_t textureUnitCount;
    HlTextureUnit* textureUnits;*/

    /* TODO: Bones */
}
HlMesh;

typedef struct HlMeshSlot
{
    size_t meshCount;
    HlMesh* meshes;
}
HlMeshSlot;

typedef struct HlSpecialMeshSlot
{
    const char* type;
    size_t meshCount;
    HlMesh* meshes;
}
HlSpecialMeshSlot;

typedef struct HlMeshGroup
{
    HlMeshSlot solid;
    HlMeshSlot transparent;
    HlMeshSlot boolean;
    size_t specialSlotCount;
    HlSpecialMeshSlot* specialSlots;
}
HlMeshGroup;

typedef enum HlVertexElementType
{
    /* Masks */
    HL_VERTEX_ELEM_FORMAT_MASK = 0x3FU,
    HL_VERTEX_ELEM_DIMENSION_MASK = 0xC0U,
    HL_VERTEX_ELEM_TYPE_MASK = 0x3FFFFF00U,
    HL_VERTEX_ELEM_INDEX_MASK = 0xC0000000U,

    /* Formats */
    HL_VERTEX_ELEM_FORMAT_IS_UNSIGNED = 1,
    HL_VERTEX_ELEM_FORMAT_IS_NORM_OR_HALF = 2,

    HL_VERTEX_ELEM_FORMAT_FLOAT = 0,
    HL_VERTEX_ELEM_FORMAT_INDEX = HL_VERTEX_ELEM_FORMAT_IS_UNSIGNED,
    HL_VERTEX_ELEM_FORMAT_HALF = 2,
    HL_VERTEX_ELEM_FORMAT_SBYTE = 4,
    HL_VERTEX_ELEM_FORMAT_BYTE = (HL_VERTEX_ELEM_FORMAT_SBYTE |
    HL_VERTEX_ELEM_FORMAT_IS_UNSIGNED),

    HL_VERTEX_ELEM_FORMAT_SBYTE_NORM = (HL_VERTEX_ELEM_FORMAT_SBYTE |
        HL_VERTEX_ELEM_FORMAT_IS_NORM_OR_HALF),

    HL_VERTEX_ELEM_FORMAT_BYTE_NORM = (HL_VERTEX_ELEM_FORMAT_BYTE |
        HL_VERTEX_ELEM_FORMAT_IS_NORM_OR_HALF),

    HL_VERTEX_ELEM_FORMAT_SHORT = 8,
    HL_VERTEX_ELEM_FORMAT_USHORT = (HL_VERTEX_ELEM_FORMAT_SHORT |
        HL_VERTEX_ELEM_FORMAT_IS_UNSIGNED),

    HL_VERTEX_ELEM_FORMAT_SHORT_NORM = (HL_VERTEX_ELEM_FORMAT_SHORT |
        HL_VERTEX_ELEM_FORMAT_IS_NORM_OR_HALF),

    HL_VERTEX_ELEM_FORMAT_USHORT_NORM = (HL_VERTEX_ELEM_FORMAT_USHORT |
        HL_VERTEX_ELEM_FORMAT_IS_NORM_OR_HALF),

    HL_VERTEX_ELEM_FORMAT_INT = 12,
    HL_VERTEX_ELEM_FORMAT_UINT = (HL_VERTEX_ELEM_FORMAT_INT |
        HL_VERTEX_ELEM_FORMAT_IS_UNSIGNED),

    HL_VERTEX_ELEM_FORMAT_INT_NORM = (HL_VERTEX_ELEM_FORMAT_INT |
        HL_VERTEX_ELEM_FORMAT_IS_NORM_OR_HALF),

    HL_VERTEX_ELEM_FORMAT_UINT_NORM = (HL_VERTEX_ELEM_FORMAT_UINT |
        HL_VERTEX_ELEM_FORMAT_IS_NORM_OR_HALF),

    /* Dimensions */
    HL_VERTEX_ELEM_DIMENSION_1D = 0,
    HL_VERTEX_ELEM_DIMENSION_2D = 0x40U,
    HL_VERTEX_ELEM_DIMENSION_3D = 0x80U,
    HL_VERTEX_ELEM_DIMENSION_4D = 0xC0U,

    /* Types */
    HL_VERTEX_ELEM_TYPE_POSITION = 0x100U,
    HL_VERTEX_ELEM_TYPE_BONE_WEIGHT = 0x200U,
    HL_VERTEX_ELEM_TYPE_BONE_INDEX = 0x300U,
    HL_VERTEX_ELEM_TYPE_NORMAL = 0x400U,
    HL_VERTEX_ELEM_TYPE_TEXCOORD = 0x500U,
    HL_VERTEX_ELEM_TYPE_TANGENT = 0x600U,
    HL_VERTEX_ELEM_TYPE_BINORMAL = 0x700U,
    HL_VERTEX_ELEM_TYPE_COLOR = 0x800U,

    /* Index types */
    HL_VERTEX_ELEM_INDEX0 = 0,
    HL_VERTEX_ELEM_INDEX1 = 0x40000000U,
    HL_VERTEX_ELEM_INDEX2 = 0x80000000U,
    HL_VERTEX_ELEM_INDEX3 = 0xC0000000U
}
HlVertexElementType;

typedef struct HlVertexElement
{
    HlVertexElementType type;
    unsigned int offset;
}
HlVertexElement;

typedef struct HlVertexFormat
{
    size_t vertexFormatSize;
    size_t vertexElementCount;
    HlVertexElement* vertexElements;
}
HlVertexFormat;

typedef struct HlModel
{
    const char* name;
    size_t meshGroupCount;
    HlMeshGroup* meshGroups;
    size_t vertexFormatCount;
    HlVertexFormat* vertexFormats;
    /* TODO: Bones */
}
HlModel;

HL_API HlModel** hlModelReadOBJ(const HlBlob* blob, size_t* modelCount);

HL_API HlResult hlModelWriteOBJ(const HlModel** HL_RESTRICT models,
    size_t modelCount, HlFile* HL_RESTRICT file);

HL_API HlModel** hlModelImportOBJ(const HlNChar* filePath, size_t* modelCount);

HL_API HlResult hlModelExportOBJ(const HlModel** HL_RESTRICT models,
    size_t modelCount, const HlNChar* HL_RESTRICT filePath);

#ifdef __cplusplus
}
#endif
#endif
