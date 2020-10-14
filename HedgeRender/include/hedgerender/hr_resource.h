#ifndef HR_RESOURCE_H_INCLUDED
#define HR_RESOURCE_H_INCLUDED
#include "hr_internal.h"
#include "hedgelib/hl_text.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HrInstance HrInstance;
typedef struct HrResMgr HrResMgr;
typedef struct HlArchive HlArchive;
typedef struct HlModel HlModel;
typedef struct HlMaterial HlMaterial;
typedef struct HlTerrainGroup HlTerrainGroup;
typedef struct HlTerrainInstanceInfo HlTerrainInstanceInfo;
typedef struct HlSectorCollision HlSectorCollision;
typedef struct HrShaderProgram HrShaderProgram;
typedef struct HrVertexFormat HrVertexFormat;
typedef struct HrRasterizerState HrRasterizerState;
typedef struct HrBlendState HrBlendState;
typedef struct HrDepthStencilState HrDepthStencilState;
typedef struct HrGPUModel HrGPUModel;
typedef struct HrGPUTexture HrGPUTexture;

typedef unsigned short HrResourceID;

typedef HlResult (*HrResParseFunc)(HrInstance* HL_RESTRICT instance,
    const void* HL_RESTRICT binData, HrResMgr* HL_RESTRICT resMgr,
    HrResourceID* HL_RESTRICT resID);

typedef enum HrResourceType
{
    /* CPU Types */
    HR_RES_TYPE_USER_DATA = 0,
    HR_RES_TYPE_ARCHIVE,
    HR_RES_TYPE_MODEL,
    HR_RES_TYPE_MATERIAL,
    HR_RES_TYPE_TERRAIN_GROUP,
    HR_RES_TYPE_TERRAIN_INSTANCE_INFO,
    HR_RES_TYPE_SV_COL,

    /* GPU Types */
    HR_RES_TYPE_SHADER_PROGRAM,
    HR_RES_TYPE_VERTEX_FORMAT,
    HR_RES_TYPE_RASTERIZER_STATE,
    HR_RES_TYPE_BLEND_STATE,
    HR_RES_TYPE_DEPTH_STENCIL_STATE,
    HR_RES_TYPE_GPU_MODEL,
    HR_RES_TYPE_GPU_TEXTURE,
    HR_RES_TYPE_COUNT
}
HrResourceType;

#define HR_RES_GPU_FIRST_TYPE  HR_RES_TYPE_SHADER_PROGRAM
#define HR_RES_GPU_TYPE_COUNT  (HR_RES_TYPE_COUNT - HR_RES_GPU_FIRST_TYPE)

HR_API HlResult hrResMgrCreate(HrInstance* HL_RESTRICT instance,
    HrResMgr* HL_RESTRICT * HL_RESTRICT resMgr);

HR_API size_t hrResMgrGetTotalResourceCount(const HrResMgr* resMgr);
HR_API size_t hrResMgrGetResourceCount(const HrResMgr* resMgr,
    const HrResourceType resType);

HR_API HlResult hrResMgrGetResourceIDEx(HrResMgr* HL_RESTRICT resMgr,
    const char* HL_RESTRICT relPath, const size_t relPathLen,
    const HrResourceType resType, HrResourceID* HL_RESTRICT resID);

#define hrResMgrGetResourceID(resMgr, relPath, resType, resID)\
    hrResMgrGetResourceIDEx(resMgr, relPath, HL_IN_TEXT_GET_LEN(relPath),\
    resType, resID)

#define hrResMgrGetUserDataID(resMgr, relPath, resID)\
    hrResMgrGetResourceID(resMgr, relPath,\
    HR_RES_TYPE_USER_DATA, resID)

#define hrResMgrGetArchiveID(resMgr, relPath, resID)\
    hrResMgrGetResourceID(resMgr, relPath,\
    HR_RES_TYPE_ARCHIVE, resID)

#define hrResMgrGetModelID(resMgr, relPath, resID)\
    hrResMgrGetResourceID(resMgr, relPath,\
    HR_RES_TYPE_MODEL, resID)

#define hrResMgrGetMaterialID(resMgr, relPath, resID)\
    hrResMgrGetResourceID(resMgr, relPath,\
    HR_RES_TYPE_MATERIAL, resID)

#define hrResMgrGetTerrainGroupID(resMgr, relPath, resID)\
    hrResMgrGetResourceID(resMgr, relPath,\
    HR_RES_TYPE_TERRAIN_GROUP, resID)

#define hrResMgrGetTerrainInstanceInfoID(resMgr, relPath, resID)\
    hrResMgrGetResourceID(resMgr, relPath,\
    HR_RES_TYPE_TERRAIN_INSTANCE_INFO, resID)

#define hrResMgrGetSVColID(resMgr, relPath, resID)\
    hrResMgrGetResourceID(resMgr, relPath,\
    HR_RES_TYPE_SV_COL, resID)

#define hrResMgrGetShaderProgramID(resMgr, relPath, resID)\
    hrResMgrGetResourceID(resMgr, relPath,\
    HR_RES_TYPE_SHADER_PROGRAM, resID)

#define hrResMgrGetVertexFormatID(resMgr, relPath, resID)\
    hrResMgrGetResourceID(resMgr, relPath,\
    HR_RES_TYPE_VERTEX_FORMAT, resID)

#define hrResMgrGetRasterizerStateID(resMgr, relPath, resID)\
    hrResMgrGetResourceID(resMgr, relPath,\
    HR_RES_TYPE_RASTERIZER_STATE, resID)

#define hrResMgrGetBlendStateID(resMgr, relPath, resID)\
    hrResMgrGetResourceID(resMgr, relPath,\
    HR_RES_TYPE_BLEND_STATE, resID)

#define hrResMgrGetDepthStencilStateID(resMgr, relPath, resID)\
    hrResMgrGetResourceID(resMgr, relPath,\
    HR_RES_TYPE_DEPTH_STENCIL_STATE, resID)

#define hrResMgrGetGPUModelID(resMgr, relPath, resID)\
    hrResMgrGetResourceID(resMgr, relPath,\
    HR_RES_TYPE_GPU_MODEL, resID)

#define hrResMgrGetGPUTextureID(resMgr, relPath, resID)\
    hrResMgrGetResourceID(resMgr, relPath,\
    HR_RES_TYPE_GPU_TEXTURE, resID)

HR_API void* hrResMgrGetResource(HrResMgr* HL_RESTRICT resMgr,
    const HrResourceType resType, const HrResourceID resID);

#define hrResMgrGetUserData(resMgr, resID) hrResMgrGetResource(\
    resMgr, HR_RES_TYPE_USER_DATA, resID)

#define hrResMgrGetArchive(resMgr, resID) ((HlArchive*)\
    hrResMgrGetResource(resMgr, HR_RES_TYPE_ARCHIVE, resID))

#define hrResMgrGetModel(resMgr, resID) ((HlModel*)\
    hrResMgrGetResource(resMgr, HR_RES_TYPE_MODEL, resID))

#define hrResMgrGetMaterial(resMgr, resID) ((HlMaterial*)\
    hrResMgrGetResource(resMgr, HR_RES_TYPE_MATERIAL, resID))

#define hrResMgrGetTerrainGroup(resMgr, resID) ((HlTerrainGroup*)\
    hrResMgrGetResource(resMgr, HR_RES_TYPE_TERRAIN_GROUP, resID))

#define hrResMgrGetTerrainInstanceInfo(resMgr, resID) ((HlTerrainInstanceInfo*)\
    hrResMgrGetResource(resMgr, HR_RES_TYPE_TERRAIN_INSTANCE_INFO, resID))

#define hrResMgrGetSVCol(resMgr, resID) ((HlSectorCollision*)\
    hrResMgrGetResource(resMgr, HR_RES_TYPE_SV_COL, resID))

#define hrResMgrGetShaderProgram(resMgr, resID) ((HrShaderProgram*)\
    hrResMgrGetResource(resMgr, HR_RES_TYPE_SHADER_PROGRAM, resID))

#define hrResMgrGetVertexFormat(resMgr, resID) ((HrVertexFormat*)\
    hrResMgrGetResource(resMgr, HR_RES_TYPE_VERTEX_FORMAT, resID))

#define hrResMgrGetRasterizerState(resMgr, resID) ((HrRasterizerState*)\
    hrResMgrGetResource(resMgr, HR_RES_TYPE_RASTERIZER_STATE, resID))

#define hrResMgrGetBlendState(resMgr, resID) ((HrBlendState*)\
    hrResMgrGetResource(resMgr, HR_RES_TYPE_BLEND_STATE, resID))

#define hrResMgrGetDepthStencilState(resMgr, resID) ((HrDepthStencilState*)\
    hrResMgrGetResource(resMgr, HR_RES_TYPE_DEPTH_STENCIL_STATE, resID))

#define hrResMgrGetGPUModel(resMgr, resID) ((HrGPUModel*)\
    hrResMgrGetResource(resMgr, HR_RES_TYPE_GPU_MODEL, resID))

#define hrResMgrGetGPUTexture(resMgr, resID) ((HrGPUTexture*)\
    hrResMgrGetResource(resMgr, HR_RES_TYPE_GPU_TEXTURE, resID))

HR_API HlResult hrResMgrAddResourceEx(HrResMgr* HL_RESTRICT resMgr,
    const char* HL_RESTRICT relPath, const size_t relPathLen,
    const HrResourceType resType, void* HL_RESTRICT resData,
    HrResourceID* HL_RESTRICT resID);

#define hrResMgrAddResource(resMgr, relPath, resType, resData, resID)\
    hrResMgrAddResourceEx(resMgr, relPath, HL_IN_TEXT_GET_LEN(relPath),\
        resType, resData, resID)

HR_API void hrResMgrClearGroup(HrResMgr* resMgr, const HrResourceType resType);
HR_API void hrResMgrDestroy(HrResMgr* resMgr);

#ifdef __cplusplus
}
#endif
#endif
