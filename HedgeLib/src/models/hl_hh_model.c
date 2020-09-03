#include "hedgelib/models/hl_hh_model.h"
#include "hedgelib/io/hl_hh.h"
#include "hedgelib/hl_blob.h"
#include "hedgelib/hl_endian.h"
#include "hedgelib/hl_math.h"

void hlHHVertexElementSwap(HlHHVertexElement* vertexElement)
{
    hlSwapU32P(&vertexElement->offset);
    hlSwapU32P(&vertexElement->format);
    hlSwapU16P(&vertexElement->type);
}

void hlHHVertexFormatSwap(HlHHMesh* mesh)
{
    HlHHVertexElement* curVertexElement = (HlHHVertexElement*)
        hlOff32Get(&mesh->vertexElementsOffset);

    do
    {
        hlHHVertexElementSwap(curVertexElement);
    }
    while ((curVertexElement++)->format != HL_HH_VERTEX_FORMAT_LAST_ENTRY);
}

void hlHHMeshSwap(HlHHMesh* mesh, HlBool swapOffsets)
{
    if (swapOffsets)
    {
        hlSwapU32P(&mesh->materialNameOffset);
        hlSwapU32P(&mesh->facesOffset);
        hlSwapU32P(&mesh->verticesOffset);
        hlSwapU32P(&mesh->vertexElementsOffset);
        hlSwapU32P(&mesh->bonesOffset);
        hlSwapU32P(&mesh->textureUnitsOffset);
    }

    hlSwapU32P(&mesh->faceCount);
    hlSwapU32P(&mesh->vertexCount);
    hlSwapU32P(&mesh->vertexSize);
    hlSwapU32P(&mesh->boneCount);
    hlSwapU32P(&mesh->textureUnitCount);
}

void hlHHMeshSwapFaces(HlHHMesh* mesh)
{
    /* Get faces pointer. */
    HlU16* faces = (HlU16*)hlOff32Get(&mesh->facesOffset);

    /* Swap faces. */
    HlU32 i;
    for (i = 0; i < mesh->faceCount; ++i)
    {
        hlSwapU16P(&faces[i]);
    }
}

void hlHHMeshSwapVertices(HlHHMesh* mesh)
{
    /* Get pointers. */
    void* hhVertices = hlOff32Get(&mesh->verticesOffset);
    const HlHHVertexElement* hhCurVertexElement = (const HlHHVertexElement*)
        hlOff32Get(&mesh->vertexElementsOffset);

    /* Swap vertices based on vertex format. */
    while (hhCurVertexElement->format != HL_HH_VERTEX_FORMAT_LAST_ENTRY)
    {
        void* curVertex = HL_ADD_OFF(hhVertices, hhCurVertexElement->offset);
        HlU32 i;

        for (i = 0; i < mesh->vertexCount; ++i)
        {
            /* Swap based on HlVertexElement format. */
            switch (hhCurVertexElement->format)
            {
            case HL_HH_VERTEX_FORMAT_INDEX_BYTE:
            case HL_HH_VERTEX_FORMAT_INDEX:
            case HL_HH_VERTEX_FORMAT_VECTOR3_HH1:
            case HL_HH_VERTEX_FORMAT_VECTOR3_HH2:
            case HL_HH_VERTEX_FORMAT_VECTOR4_BYTE:
                /* TODO: Is this correct? */
                hlSwapU32P(((HlU32*)curVertex));
                break;

            case HL_HH_VERTEX_FORMAT_VECTOR2:
                hlVector2Swap(((HlVector2*)curVertex));
                break;

            case HL_HH_VERTEX_FORMAT_VECTOR2_HALF:
                hlVector2HalfSwap(((HlVector2Half*)curVertex));
                break;

            case HL_HH_VERTEX_FORMAT_VECTOR3:
                hlVector3Swap(((HlVector3*)curVertex));
                break;

            /* TODO: Support HL_HH_VERTEX_FORMAT_VECTOR3_HH1. */

            case HL_HH_VERTEX_FORMAT_VECTOR4:
                hlVector4Swap(((HlVector4*)curVertex));
                break;

            /* TODO: Add other formats. */
            default: break;
            }

            /* Increase vertices pointer. */
            curVertex = HL_ADD_OFF(curVertex, mesh->vertexSize);
        }

        /* Increment current vertex element pointer. */
        ++hhCurVertexElement;
    }
}

void hlHHMeshSlotSwap(HlHHMeshSlot* meshSlot, HlBool swapOffsets)
{
    hlSwapU32P(&meshSlot->meshCount);
    if (swapOffsets) hlSwapU32P(&meshSlot->meshesOffset);
}

void hlHHMeshSlotSwapRecursive(HlHHMeshSlot* meshSlot, HlBool swapOffsets)
{
    /* Get pointers. */
    HL_OFF32(HlHHMesh)* meshOffsets = (HL_OFF32(HlHHMesh)*)
        hlOff32Get(&meshSlot->meshesOffset);

    /* Swap the meshes in the mesh slot. */
    HlU32 i;
    for (i = 0; i < meshSlot->meshCount; ++i)
    {
        /* Swap the mesh. */
        HlHHMesh* mesh = (HlHHMesh*)hlOff32Get(&meshOffsets[i]);
        hlHHMeshSwap(mesh, swapOffsets);

        /* Swap the vertex format. */
        hlHHVertexFormatSwap(mesh);

        /* Swap the faces. */
        hlHHMeshSwapFaces(mesh);

        /* Swap the vertices. */
        hlHHMeshSwapVertices(mesh);
    }
}

void hlHHMeshGroupSwap(HlHHMeshGroup* meshGroup, HlBool swapOffsets)
{
    hlHHMeshSlotSwap(&meshGroup->solid, swapOffsets);
    hlHHMeshSlotSwap(&meshGroup->transparent, swapOffsets);
    hlHHMeshSlotSwap(&meshGroup->boolean, swapOffsets);

    /* TODO: Swap special slot. */
}

void hlHHMeshGroupSwapRecursive(HlHHMeshGroup* meshGroup, HlBool swapOffsets)
{
    /* Swap the mesh group. */
    hlHHMeshGroupSwap(meshGroup, swapOffsets);

    /* Recursively swap the slots within the group. */
    hlHHMeshSlotSwapRecursive(&meshGroup->solid, swapOffsets);
    hlHHMeshSlotSwapRecursive(&meshGroup->boolean, swapOffsets);
    hlHHMeshSlotSwapRecursive(&meshGroup->transparent, swapOffsets);

    /* TODO: Swap the special slots. */
}

void hlHHTerrainModelSwap(HlHHTerrainModel* model, HlBool swapOffsets)
{
    hlSwapU32P(&model->meshGroupCount);

    if (swapOffsets)
    {
        hlSwapU32P(&model->meshGroupsOffset);
        hlSwapU32P(&model->nameOffset);
    }

    hlSwapU32P(&model->flags);
}

void hlHHSkeletonSwap(HlHHSkeleton* skeleton)
{
    /* TODO */
}

void hlHHSkeletalModelSwap(HlHHSkeletalModel* model, HlBool swapOffsets)
{
    hlSwapU32P(&model->meshGroupCount);
    if (swapOffsets) hlSwapU32P(&model->meshGroupsOffset);
    hlHHSkeletonSwap(&model->skeleton);
}

static void hlINHHMeshGroupsSwapRecursive(
    HL_OFF32(HlHHMeshGroup)* meshGroups, HlU32 meshGroupCount)
{
    HlU32 i;
    for (i = 0; i < meshGroupCount; ++i)
    {
        /* Get the mesh group. */
        HlHHMeshGroup* meshGroup = (HlHHMeshGroup*)hlOff32Get(&meshGroups[i]);

        /* Swap the mesh group recursively. */
        hlHHMeshGroupSwapRecursive(meshGroup, HL_FALSE);
    }
}

void hlHHSkeletalModelFix(HlHHSkeletalModel* model)
{
    /* TODO: Have this just be a macro if HL_IS_BIG_ENDIAN is set? */

#ifndef HL_IS_BIG_ENDIAN
    HL_OFF32(HlHHMeshGroup)* meshGroupOffsets = (HL_OFF32(HlHHMeshGroup)*)
        hlOff32Get(&model->meshGroupsOffset);

    hlHHSkeletalModelSwap(model, HL_FALSE);
    hlINHHMeshGroupsSwapRecursive(meshGroupOffsets, model->meshGroupCount);
#endif
}

void hlHHTerrainModelFix(HlHHTerrainModel* model)
{
    /* TODO: Have this just be a macro if HL_IS_BIG_ENDIAN is set? */

#ifndef HL_IS_BIG_ENDIAN
    HL_OFF32(HlHHMeshGroup)* meshGroupOffsets = (HL_OFF32(HlHHMeshGroup)*)
        hlOff32Get(&model->meshGroupsOffset);

    hlHHTerrainModelSwap(model, HL_FALSE);
    hlINHHMeshGroupsSwapRecursive(meshGroupOffsets, model->meshGroupCount);
#endif
}

static size_t hlINHHMeshSlotGetReqSize(const HlHHMeshSlot* slot)
{
    /* Get mesh offsets pointer. */
    const HL_OFF32(HlHHMesh)* meshOffsets = (const HL_OFF32(HlHHMesh)*)
        hlOff32Get(&slot->meshesOffset);

    /* Account for mesh slot. */
    size_t reqSize = sizeof(HlMeshSlot);
    HlU32 i;

    /* Account for meshes. */
    for (i = 0; i < slot->meshCount; ++i)
    {
        /* Get mesh pointer. */
        const HlHHMesh* mesh = (const HlHHMesh*)hlOff32Get(&meshOffsets[i]);

        /* Account for mesh. */
        reqSize += sizeof(HlMesh);

        /* Account for faces. */
        reqSize += (sizeof(unsigned short) * mesh->faceCount);

        /* Account for vertices. */
        reqSize += ((size_t)mesh->vertexSize * mesh->vertexCount);

        /* Account for vertex format. */
        reqSize += sizeof(HlVertexFormat);

        /* Account for vertex elements. */
        {
            const HlHHVertexElement* curVertexElement = (const HlHHVertexElement*)
                hlOff32Get(&mesh->vertexElementsOffset);
            
            while ((curVertexElement++)->format != HL_HH_VERTEX_FORMAT_LAST_ENTRY)
            {
                reqSize += sizeof(HlVertexElement);
            }
        }

        /* TODO: Account for other stuff like bones and texture units. */
    }

    return reqSize;
}

static size_t hlINHHMeshGroupsGetReqSize(
    const HL_OFF32(HlHHMeshGroup)* meshGroups,
    HlU32 meshGroupCount)
{
    size_t reqSize = 0;
    HlU32 i;

    for (i = 0; i < meshGroupCount; ++i)
    {
        /* Get mesh group pointer. */
        HlHHMeshGroup* meshGroup = (HlHHMeshGroup*)hlOff32Get(&meshGroups[i]);

        /* Account for mesh group. */
        reqSize += sizeof(HlMeshGroup);

        /* Account for mesh slots. */
        reqSize += hlINHHMeshSlotGetReqSize(&meshGroup->solid);
        reqSize += hlINHHMeshSlotGetReqSize(&meshGroup->transparent);
        reqSize += hlINHHMeshSlotGetReqSize(&meshGroup->boolean);

        /* TODO: Account for special slots. */
    }

    return reqSize;
}

static HlResult hlINHHVertexFormatRead(const HlHHMeshSlot* HL_RESTRICT hhMeshSlot,
    void** HL_RESTRICT endPtr)
{
    /* Get pointers. */
    const HL_OFF32(HlHHMesh)* meshOffsets = (const HL_OFF32(HlHHMesh)*)
        hlOff32Get(&hhMeshSlot->meshesOffset);

    HlVertexFormat* hlVertexFormats = (HlVertexFormat*)(*endPtr);
    HlVertexElement* hlCurVertexElement = (HlVertexElement*)
        &hlVertexFormats[hhMeshSlot->meshCount];

    /* Read mesh slot. */
    HlU32 i;
    for (i = 0; i < hhMeshSlot->meshCount; ++i)
    {
        /* Get pointers. */
        const HlHHMesh* mesh = (const HlHHMesh*)hlOff32Get(&meshOffsets[i]);

        /* Setup HlVertexFormat. */
        hlVertexFormats[i].vertexFormatSize = (size_t)mesh->vertexSize;
        hlVertexFormats[i].vertexElements = hlCurVertexElement;

        /* Setup HlVertexElements. */
        {
            const HlHHVertexElement* hhCurVertexElement = (const HlHHVertexElement*)
                hlOff32Get(&mesh->vertexElementsOffset);

            while (hhCurVertexElement->format != HL_HH_VERTEX_FORMAT_LAST_ENTRY)
            {
                /* Get HlVertexElement format. */
                switch (hhCurVertexElement->format)
                {
                case HL_HH_VERTEX_FORMAT_INDEX_BYTE:
                    hlCurVertexElement->type = (HL_VERTEX_ELEM_FORMAT_BYTE |
                        HL_VERTEX_ELEM_DIMENSION_4D);
                    break;

                case HL_HH_VERTEX_FORMAT_INDEX:
                    /* TODO: Is this correct? */
                    hlCurVertexElement->type = (HL_VERTEX_ELEM_FORMAT_SBYTE |
                        HL_VERTEX_ELEM_DIMENSION_4D);
                    break;

                case HL_HH_VERTEX_FORMAT_VECTOR2:
                    hlCurVertexElement->type = (HL_VERTEX_ELEM_FORMAT_FLOAT |
                        HL_VERTEX_ELEM_DIMENSION_2D);
                    break;

                case HL_HH_VERTEX_FORMAT_VECTOR2_HALF:
                    hlCurVertexElement->type = (HL_VERTEX_ELEM_FORMAT_HALF |
                        HL_VERTEX_ELEM_DIMENSION_2D);
                    break;

                case HL_HH_VERTEX_FORMAT_VECTOR3:
                    hlCurVertexElement->type = (HL_VERTEX_ELEM_FORMAT_FLOAT |
                        HL_VERTEX_ELEM_DIMENSION_3D);
                    break;

                /* TODO: Support HL_HH_VERTEX_FORMAT_VECTOR3_HH1. */

                case HL_HH_VERTEX_FORMAT_VECTOR3_HH2:
                    hlCurVertexElement->type = (HL_VERTEX_ELEM_FORMAT_SBYTE_NORM |
                        HL_VERTEX_ELEM_DIMENSION_4D);
                    break;

                case HL_HH_VERTEX_FORMAT_VECTOR4:
                    hlCurVertexElement->type = (HL_VERTEX_ELEM_FORMAT_FLOAT |
                        HL_VERTEX_ELEM_DIMENSION_4D);
                    break;

                case HL_HH_VERTEX_FORMAT_VECTOR4_BYTE:
                    hlCurVertexElement->type = (HL_VERTEX_ELEM_FORMAT_BYTE_NORM |
                        HL_VERTEX_ELEM_DIMENSION_4D);
                    break;

                /* TODO: Add other formats. */
                default: return HL_ERROR_UNSUPPORTED;
                }

                /* Get HlVertexFormat type. */
                switch (hhCurVertexElement->type)
                {
                case HL_HH_VERTEX_TYPE_POSITION:
                    hlCurVertexElement->type |= HL_VERTEX_ELEM_TYPE_POSITION;
                    break;

                case HL_HH_VERTEX_TYPE_BONE_WEIGHT:
                    hlCurVertexElement->type |= HL_VERTEX_ELEM_TYPE_BONE_WEIGHT;
                    break;

                case HL_HH_VERTEX_TYPE_BONE_INDEX:
                    hlCurVertexElement->type |= HL_VERTEX_ELEM_TYPE_BONE_INDEX;
                    break;

                case HL_HH_VERTEX_TYPE_NORMAL:
                    hlCurVertexElement->type |= HL_VERTEX_ELEM_TYPE_NORMAL;
                    break;

                case HL_HH_VERTEX_TYPE_TEXCOORD:
                    hlCurVertexElement->type |= HL_VERTEX_ELEM_TYPE_TEXCOORD;
                    break;

                case HL_HH_VERTEX_TYPE_TANGENT:
                    hlCurVertexElement->type |= HL_VERTEX_ELEM_TYPE_TANGENT;
                    break;

                case HL_HH_VERTEX_TYPE_BINORMAL:
                    hlCurVertexElement->type |= HL_VERTEX_ELEM_TYPE_BINORMAL;
                    break;

                case HL_HH_VERTEX_TYPE_COLOR:
                    hlCurVertexElement->type |= HL_VERTEX_ELEM_TYPE_COLOR;
                    break;

                default: return HL_ERROR_UNSUPPORTED;
                }

                /* Get HlVertexFormat index. */
                switch (hhCurVertexElement->index)
                {
                case 0:
                    hlCurVertexElement->type |= HL_VERTEX_ELEM_INDEX0;
                    break;

                case 1:
                    hlCurVertexElement->type |= HL_VERTEX_ELEM_INDEX1;
                    break;

                case 2:
                    hlCurVertexElement->type |= HL_VERTEX_ELEM_INDEX2;
                    break;

                case 3:
                    hlCurVertexElement->type |= HL_VERTEX_ELEM_INDEX3;
                    break;

                default: return HL_ERROR_UNSUPPORTED;
                }

                /* Get HlVertexFormat offset. */
                hlCurVertexElement->offset = (unsigned int)hhCurVertexElement->offset;

                /* Increment vertex element count. */
                ++hlVertexFormats[i].vertexElementCount;

                /* Increment pointers. */
                ++hlCurVertexElement;
                ++hhCurVertexElement;
            }
        }
    }

    /* Set end pointer and return success. */
    *endPtr = hlCurVertexElement;
    return HL_RESULT_SUCCESS;
}

static HlResult hlINHHMeshSlotRead(const HlHHMeshSlot* HL_RESTRICT hhMeshSlot,
    HlMeshSlot* HL_RESTRICT hlMeshSlot, void** HL_RESTRICT endPtr)
{
    /* Get mesh offsets pointer. */
    const HL_OFF32(HlHHMesh)* meshOffsets = (const HL_OFF32(HlHHMesh)*)
        hlOff32Get(&hhMeshSlot->meshesOffset);

    /* Setup HlMeshSlot. */
    hlMeshSlot->meshCount = (size_t)hhMeshSlot->meshCount;
    hlMeshSlot->meshes = (HlMesh*)(*endPtr);

    /* Set end pointer. */
    *endPtr = &hlMeshSlot->meshes[hlMeshSlot->meshCount];

    /* Read meshes. */
    {
        HlU32 i;
        for (i = 0; i < hhMeshSlot->meshCount; ++i)
        {
            /* Get pointers. */
            const HlHHMesh* hhMesh = (const HlHHMesh*)hlOff32Get(&meshOffsets[i]);
            HlMesh* hlMesh = &hlMeshSlot->meshes[i];
            unsigned short* hlFaces = (unsigned short*)(*endPtr);
            void* hlVertices = &hlFaces[hhMesh->faceCount];

            /* Setup HlMesh. */
            /* TODO: Setup matRef. */
            hlMesh->vertexFormatIndex = (size_t)i;
            hlMesh->vertexCount = (size_t)hhMesh->vertexCount;
            hlMesh->vertices = hlVertices;
            hlMesh->faceCount = (size_t)hhMesh->faceCount;
            hlMesh->faces = hlFaces;

            /* Copy faces. */
            {
                const HlU16* hhFaces = (const HlU16*)hlOff32Get(&hhMesh->facesOffset);
                size_t i2;

                for (i2 = 0; i2 < hlMesh->faceCount; ++i2)
                {
                    hlFaces[i2] = (unsigned short)hhFaces[i2];
                }
            }

            /* Copy vertices and set end pointer. */
            {
                /* Copy vertices. */
                const void* hhVertices = (const void*)hlOff32Get(&hhMesh->verticesOffset);
                const size_t vertexBufferSize = ((size_t)hhMesh->vertexSize * hhMesh->vertexCount);
                memcpy(hlVertices, hhVertices, vertexBufferSize);

                /* Set end pointer. */
                *endPtr = HL_ADD_OFF(hlVertices, vertexBufferSize);
            }
        }

        return HL_RESULT_SUCCESS;
    }
}

static HlResult hlINHHMeshGroupsRead(
    const HL_OFF32(HlHHMeshGroup)* HL_RESTRICT hhMeshGroups,
    HlU32 meshGroupCount, HlMeshGroup* HL_RESTRICT hlMeshGroups)
{
    void* endPtr = &hlMeshGroups[meshGroupCount];
    HlU32 i;
    HlResult result;

    for (i = 0; i < meshGroupCount; ++i)
    {
        /* Get mesh group pointer. */
        const HlHHMeshGroup* hhMeshGroup = (const HlHHMeshGroup*)
            hlOff32Get(&hhMeshGroups[i]);

        /* Read slots. */
        result = hlINHHVertexFormatRead(&hhMeshGroup->solid, &endPtr);
        if (HL_FAILED(result)) return result;

        result = hlINHHVertexFormatRead(&hhMeshGroup->transparent,  &endPtr);
        if (HL_FAILED(result)) return result;

        result = hlINHHVertexFormatRead(&hhMeshGroup->boolean, &endPtr);
        if (HL_FAILED(result)) return result;

        /* TODO: Read special slots. */
    }

    for (i = 0; i < meshGroupCount; ++i)
    {
        /* Get mesh group pointer. */
        const HlHHMeshGroup* hhMeshGroup = (const HlHHMeshGroup*)
            hlOff32Get(&hhMeshGroups[i]);

        /* Read slots. */
        result = hlINHHMeshSlotRead(&hhMeshGroup->solid,
            &hlMeshGroups[i].solid, &endPtr);

        if (HL_FAILED(result)) return result;

        result = hlINHHMeshSlotRead(&hhMeshGroup->transparent,
            &hlMeshGroups[i].transparent, &endPtr);

        if (HL_FAILED(result)) return result;

        result = hlINHHMeshSlotRead(&hhMeshGroup->boolean,
            &hlMeshGroups[i].boolean, &endPtr);

        if (HL_FAILED(result)) return result;

        /* TODO: Read special slots. */
    }

    return HL_RESULT_SUCCESS;
}

HlResult hlHHSkeletalModelParse(
    const HlHHSkeletalModel* HL_RESTRICT hhModel,
    HlModel** HL_RESTRICT hlModel)
{
    const HL_OFF32(HlHHMeshGroup)* hhMeshGroups = (const HL_OFF32(HlHHMeshGroup)*)
        hlOff32Get(&hhModel->meshGroupsOffset);

    HlModel* hlModelBuf;
    HlResult result;

    /* Allocate HlModel buffer. */
    {
        /* Compute total required buffer size. */
        size_t reqBufSize = sizeof(HlModel);
        reqBufSize += hlINHHMeshGroupsGetReqSize(
            hhMeshGroups, hhModel->meshGroupCount);

        /* Allocate HlModel buffer. */
        hlModelBuf = (HlModel*)hlAlloc(reqBufSize);
        if (!hlModelBuf) return HL_ERROR_OUT_OF_MEMORY;
    }

    /* Read HlModel. */
    {
        /* Setup HlModel. */
        hlModelBuf->name = NULL;
        hlModelBuf->meshGroupCount = (size_t)hhModel->meshGroupCount;
        hlModelBuf->meshGroups = (HlMeshGroup*)HL_ADD_OFF(
            hlModelBuf, sizeof(HlModel));

        /* Setup HlMeshGroups. */
        hlModelBuf->vertexFormats = (HlVertexFormat*)
            &hlModelBuf->meshGroups[hlModelBuf->meshGroupCount];

        hlModelBuf->vertexFormatCount = hlModelBuf->meshGroupCount;

        result = hlINHHMeshGroupsRead(hhMeshGroups,
            hhModel->meshGroupCount, hlModelBuf->meshGroups);

        if (HL_FAILED(result)) return result;

        /* Set model pointer and return success. */
        *hlModel = hlModelBuf;
        return HL_RESULT_SUCCESS;
    }
}

HlResult hlHHSkeletalModelRead(HlBlob* HL_RESTRICT blob,
    HlModel** HL_RESTRICT hlModel)
{
    /* Fix HH general data. */
    HlHHSkeletalModel* hhModel;
    hlHHFix(blob);

    /* Get HH skeletal model pointer. */
    hhModel = (HlHHSkeletalModel*)hlHHGetData(blob);
    if (!hhModel) return HL_ERROR_INVALID_DATA;

    /* Fix HH skeletal model data. */
    hlHHSkeletalModelFix(hhModel);

    /* Parse HH skeletal model data into HlModel and return result. */
    return hlHHSkeletalModelParse(hhModel, hlModel);
}

HlResult hlHHTerrainModelParse(
    const HlHHTerrainModel* HL_RESTRICT hhModel,
    HlModel** HL_RESTRICT hlModel)
{
    const HL_OFF32(HlHHMeshGroup)* hhMeshGroups = (const HL_OFF32(HlHHMeshGroup)*)
        hlOff32Get(&hhModel->meshGroupsOffset);

    const char* hhModelName = (const char*)hlOff32Get(&hhModel->nameOffset);
    HlModel* hlModelBuf;
    size_t modelNameLen;
    HlResult result;

    /* Allocate HlModel buffer. */
    {
        /* Compute total required buffer size. */
        size_t reqBufSize = sizeof(HlModel);
        reqBufSize += hlINHHMeshGroupsGetReqSize(
            hhMeshGroups, hhModel->meshGroupCount);

        /* Account for model name. */
        modelNameLen = (strlen(hhModelName) + 1);
        reqBufSize += modelNameLen;

        /* Allocate HlModel buffer. */
        hlModelBuf = (HlModel*)hlAlloc(reqBufSize);
        if (!hlModelBuf) return HL_ERROR_OUT_OF_MEMORY;
    }

    /* Read HlModel. */
    {
        /* Setup HlModel. */
        hlModelBuf->name = (const char*)HL_ADD_OFFC(hlModelBuf, sizeof(HlModel));
        hlModelBuf->meshGroupCount = (size_t)hhModel->meshGroupCount;
        hlModelBuf->meshGroups = (HlMeshGroup*)HL_ADD_OFF(hlModelBuf,
            sizeof(HlModel) + modelNameLen);

        /* Setup HlMeshGroups. */
        hlModelBuf->vertexFormats = (HlVertexFormat*)
            &hlModelBuf->meshGroups[hlModelBuf->meshGroupCount];

        hlModelBuf->vertexFormatCount = hlModelBuf->meshGroupCount;

        result = hlINHHMeshGroupsRead(hhMeshGroups,
            hhModel->meshGroupCount, hlModelBuf->meshGroups);

        if (HL_FAILED(result)) return result;

        /* Copy name. */
        memcpy(hlModelBuf + 1, hhModelName, modelNameLen);

        /* Set model pointer and return success. */
        *hlModel = hlModelBuf;
        return HL_RESULT_SUCCESS;
    }
}

HlResult hlHHTerrainModelRead(HlBlob* HL_RESTRICT blob,
    HlModel** HL_RESTRICT hlModel)
{
    /* Fix HH general data. */
    HlHHTerrainModel* hhModel;
    hlHHFix(blob);

    /* Get HH terrain model pointer. */
    hhModel = (HlHHTerrainModel*)hlHHGetData(blob);
    if (!hhModel) return HL_ERROR_INVALID_DATA;

    /* Fix HH terrain model data. */
    hlHHTerrainModelFix(hhModel);

    /* Parse HH terrain model data into HlModel and return result. */
    return hlHHTerrainModelParse(hhModel, hlModel);
}
