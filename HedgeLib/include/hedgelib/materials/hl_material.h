#ifndef HL_MATERIAL_H_INCLUDED
#define HL_MATERIAL_H_INCLUDED
#include "../hl_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HlFile HlFile;

typedef enum HlMaterialParamFormat
{
    HL_MATERIAL_PARAM_FORMAT_BOOL4,
    HL_MATERIAL_PARAM_FORMAT_INT4,
    HL_MATERIAL_PARAM_FORMAT_FLOAT4
}
HlMaterialParamFormat;

typedef struct HlMaterialParam
{
    const char* name;
    HlMaterialParamFormat format;
    unsigned int valueCount;
    void* values;
}
HlMaterialParam;

typedef struct HlTextureEntry
{
    const char* entryName;
    const char* texFileName;
    const char* type;
}
HlTextureEntry;

typedef struct HlTexset
{
    size_t textureCount;
    HlTextureEntry* textures;
}
HlTexset;

typedef struct HlMaterial
{
    const char* name;
    const char* shaderName;
    const char* techniqueName;
    float mipMapBias;
    size_t paramCount;
    HlMaterialParam* params;
    HL_REF(HlTexset) texset;
}
HlMaterial;

HL_API HlResult hlMaterialWriteMTL(const HlMaterial* HL_RESTRICT * HL_RESTRICT materials,
    size_t materialCount, HlFile* HL_RESTRICT file);

HL_API HlResult hlMaterialExportMTL(const HlMaterial* HL_RESTRICT * HL_RESTRICT materials,
    size_t materialCount, const HlNChar* HL_RESTRICT filePath);

#ifdef __cplusplus
}
#endif
#endif
