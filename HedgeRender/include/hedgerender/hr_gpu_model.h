#ifndef HR_GPU_MODEL_H_INCLUDED
#define HR_GPU_MODEL_H_INCLUDED
#include "hr_resource.h"
#include "hedgelib/hl_list.h"
#include "hedgelib/hl_math.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HlModel HlModel;
typedef struct HrInstance HrInstance;
typedef struct HrResMgr HrResMgr;
typedef struct HrShaderCode HrShaderCode;

/* TODO: Make HrSlotType part of HedgeLib. */

typedef enum HrSlotType
{
    HR_SLOT_TYPE_SOLID,
    HR_SLOT_TYPE_TRANSPARENT,
    HR_SLOT_TYPE_PUNCH,
    HR_SLOT_TYPE_SPECIAL
}
HrSlotType;

typedef struct HrGPUMesh
{
    void* vtxBuf;
    void* idxBuf;
    unsigned int indexCount;
    HrResourceID vtxFmtID;
    unsigned char indexType;
    /** @brief See HrSlotType. */
    unsigned char slotType;
    /** @brief Unused unless slotType == HR_SLOT_TYPE_SPECIAL. */
    const char* specialSlotType;
    const char* matName;
}
HrGPUMesh;

typedef struct HrGPUModel
{
    HrGPUMesh* meshes;
    HL_LIST(HlMatrix4x4) instances;
    unsigned short meshCount;
    HrResourceID giTex, giShadowTex; /* TODO: Remove these; store this info elsewhere! */
    HlBool isTerrain;
}
HrGPUModel;

HR_BACK_FUNC(HlResult, hrGPUModelCreate)(HrInstance* HL_RESTRICT instance,
    const HlModel* HL_RESTRICT hlModel, HlBool isTerrain,
    HrResMgr* HL_RESTRICT resMgr, HrGPUModel* HL_RESTRICT * HL_RESTRICT hrModel);

HR_BACK_FUNC(void, hrGPUModelDestroy)(HrGPUModel* hrModel);

#ifdef __cplusplus
}
#endif
#endif
