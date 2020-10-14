#ifndef HR_STATE_GROUP_H_INCLUDED
#define HR_STATE_GROUP_H_INCLUDED
#include "hr_shader.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum HrStateType
{
    HR_STATE_TYPE_SHADER_PROGRAM = 1,
    HR_STATE_TYPE_BLEND = 2,
    HR_STATE_TYPE_RASTER = 4,
    HR_STATE_TYPE_DEPTH = 8,
    HR_STATE_TYPE_INDEX_TYPE = 16,
    HR_STATE_TYPE_VERTEX_FORMAT = 32
}
HrStateType;

typedef enum HrStateResourceType
{
    HR_STATE_RES_TYPE_MESH = 1
}
HrStateResourceType;

typedef struct HrStateGroup
{
    unsigned int statesMask;
    unsigned int resourcesMask;

    HrResourceID shaderProgramID;
    unsigned char blendID;
    unsigned char rasterID;
    unsigned char depthID;

    unsigned char indexType;
    HrResourceID vertexFormatID;

    /*HrShaderResources resVS;*/
    HrPSShaderResources resPS;
    /*HrShaderResources resDS;
    HrShaderResources resHS;
    HrShaderResources resGS;*/
    HrResourceID modelID;
    unsigned short meshIndex;
}
HrStateGroup;

HR_API const HrStateGroup* hrStateGroupGetFirstWithState(
    const HrStateGroup** stateGroups,
    size_t stateGroupCount, HrStateType type);

HR_API const HrStateGroup* hrStateGroupGetFirstWithResource(
    const HrStateGroup** stateGroups,
    size_t stateGroupCount, HrStateResourceType type);

#define hrStateGroupSetShaderProgram(stateGroup, newShaderProgramID)\
    ((stateGroup)->statesMask |= HR_STATE_TYPE_SHADER_PROGRAM,\
    (stateGroup)->shaderProgramID = (newShaderProgramID))

#define hrStateGroupSetBlendState(stateGroup, newBlendStateID)\
    ((stateGroup)->statesMask |= HR_STATE_TYPE_BLEND,\
    (stateGroup)->blendID = (unsigned char)(newBlendStateID))

#define hrStateGroupSetRasterizerState(stateGroup, newRasterizerStateID)\
    ((stateGroup)->statesMask |= HR_STATE_TYPE_RASTER,\
    (stateGroup)->rasterID = (unsigned char)(newRasterizerStateID))

#define hrStateGroupSetDepthStencilState(stateGroup, newDepthStateID)\
    ((stateGroup)->statesMask |= HR_STATE_TYPE_DEPTH,\
    (stateGroup)->depthID = (unsigned char)(newDepthStateID))

#define hrStateGroupSetIndexType(stateGroup, newIndexType)\
    ((stateGroup)->statesMask |= HR_STATE_TYPE_INDEX_TYPE,\
    (stateGroup)->indexType = (unsigned char)(newIndexType))

#define hrStateGroupSetVertexFormat(stateGroup, newVertexFormatID)\
    ((stateGroup)->statesMask |= HR_STATE_TYPE_VERTEX_FORMAT,\
    (stateGroup)->vertexFormatID = (newVertexFormatID))

/* TODO: Functions for setting resources. */

#define hrStateGroupSetModel(stateGroup, newModelID)\
    ((stateGroup)->resourcesMask |= HR_STATE_RES_TYPE_MESH,\
    (stateGroup)->modelID = (newModelID))

#define hrStateGroupSetMeshIndex(stateGroup, newMeshIndex)\
    ((stateGroup)->meshIndex = (newMeshIndex))

#define hrStateGroupSetMesh(stateGroup, newModelID, newMeshIndex)\
    (hrStateGroupSetModel(stateGroup, newModelID),\
    hrStateGroupSetMeshIndex(stateGroup, newMeshIndex))

#ifdef __cplusplus
}
#endif
#endif
