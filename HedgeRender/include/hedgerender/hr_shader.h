#ifndef HR_SHADER_H_INCLUDED
#define HR_SHADER_H_INCLUDED
#include "hr_resource.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HR_SHADER_RESOURCE_SLOT_COUNT 128
#define HR_SHADER_INPUT_SLOT_COUNT 16

typedef struct HrInstance HrInstance;

/*
   TODO:
   D3D12: typedef of HrShaderCode
   D3D11: union containing pointers of all D3D11 shader types
*/
typedef struct HrShaderData HrShaderData;

typedef struct HrShaderProgram HrShaderProgram;

typedef struct HrShaderCode
{
    /** @brief The length of the data pointed to by code. */
    size_t codeLen;
    const void* code;
}
HrShaderCode;

typedef enum HrShaderType
{
    HR_SHADER_TYPE_VERTEX = 1,
    HR_SHADER_TYPE_PIXEL = 2,
    HR_SHADER_TYPE_DOMAIN = 4,
    HR_SHADER_TYPE_HULL = 8,
    HR_SHADER_TYPE_GEOMETRY = 16
}
HrShaderType;

/*typedef struct HrShaderResourceList
{
    HrResourceID resIDs[HR_SHADER_RESOURCE_SLOT_COUNT];
}
HrShaderResourceList;*/

typedef struct HrPSShaderResources
{
    HrResourceID difTexID;
    HrResourceID spcTexID;
    HrResourceID nrmTexID;
    HrResourceID giTexID;
    HrResourceID giShadowTexID;
    /* TODO: Add more texture IDs */

    /*HrResourceID constBufID;
    HrResourceID resListID;
    HrResourceID samplerID;
    HrResourceID uavID;*/
}
HrPSShaderResources;

HR_BACK_FUNC(const HrShaderCode*, hrShaderGetDefaultVSCode)(void);
HR_BACK_FUNC(const HrShaderCode*, hrShaderGetDefaultPSCode)(void);

HR_BACK_FUNC(HlResult, hrShaderProgramCreate)(HrInstance* HL_RESTRICT instance,
    const HrShaderCode* HL_RESTRICT vsCode, const HrShaderCode* HL_RESTRICT psCode,
    const HrShaderCode* HL_RESTRICT dsCode, const HrShaderCode* HL_RESTRICT hsCode,
    const HrShaderCode* HL_RESTRICT gsCode, HrShaderProgram* HL_RESTRICT * HL_RESTRICT shaderProgram);

HR_BACK_FUNC(void, hrShaderProgramDestroy)(HrShaderProgram* shaderProgram);

HR_BACK_FUNC(HlResult, hrShaderCompile)(const void* HL_RESTRICT text,
    size_t textSize, HrShaderType type,
    HrShaderCode* HL_RESTRICT * HL_RESTRICT bytecode);

#ifdef __cplusplus
}
#endif
#endif
