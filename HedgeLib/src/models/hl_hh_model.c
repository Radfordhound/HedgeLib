#include "hedgelib/models/hl_hh_model.h"
#include "hedgelib/materials/hl_hh_material.h"
#include "hedgelib/io/hl_hh.h"
#include "hedgelib/io/hl_path.h"
#include "hedgelib/hl_blob.h"
#include "hedgelib/hl_endian.h"
#include "hedgelib/hl_math.h"
#include "../io/hl_in_path.h"

void hlHHVertexElementSwap(HlHHVertexElement* vertexElement)
{
    hlSwapU16P(&vertexElement->stream);
    hlSwapU16P(&vertexElement->offset);
    hlSwapU32P(&vertexElement->format);
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
    hlHHMeshSlotSwap(&meshGroup->punch, swapOffsets);

    /* TODO: Swap special slot. */
}

void hlHHMeshGroupSwapRecursive(HlHHMeshGroup* meshGroup, HlBool swapOffsets)
{
    /* Swap the mesh group. */
    hlHHMeshGroupSwap(meshGroup, swapOffsets);

    /* Recursively swap the slots within the group. */
    hlHHMeshSlotSwapRecursive(&meshGroup->solid, swapOffsets);
    hlHHMeshSlotSwapRecursive(&meshGroup->punch, swapOffsets);
    hlHHMeshSlotSwapRecursive(&meshGroup->transparent, swapOffsets);

    /* TODO: Swap the special slots. */
}

void hlHHTerrainModelV5Swap(HlHHTerrainModelV5* model, HlBool swapOffsets)
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

void hlHHSkeletalModelV5Swap(HlHHSkeletalModelV5* model, HlBool swapOffsets)
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

void hlHHSkeletalModelV5Fix(HlHHSkeletalModelV5* model)
{
    /* TODO: Have this just be a macro if HL_IS_BIG_ENDIAN is set? */

#ifndef HL_IS_BIG_ENDIAN
    HL_OFF32(HlHHMeshGroup)* meshGroupOffsets = (HL_OFF32(HlHHMeshGroup)*)
        hlOff32Get(&model->meshGroupsOffset);

    hlHHSkeletalModelV5Swap(model, HL_FALSE);
    hlINHHMeshGroupsSwapRecursive(meshGroupOffsets, model->meshGroupCount);
#endif
}

void hlHHTerrainModelV5Fix(HlHHTerrainModelV5* model)
{
    /* TODO: Have this just be a macro if HL_IS_BIG_ENDIAN is set? */

#ifndef HL_IS_BIG_ENDIAN
    HL_OFF32(HlHHMeshGroup)* meshGroupOffsets = (HL_OFF32(HlHHMeshGroup)*)
        hlOff32Get(&model->meshGroupsOffset);

    hlHHTerrainModelV5Swap(model, HL_FALSE);
    hlINHHMeshGroupsSwapRecursive(meshGroupOffsets, model->meshGroupCount);
#endif
}

static const HlU8 hlINHHGetIndexType(const HlU32 topologyType)
{
    switch (topologyType)
    {
    default:
    case HL_HH_TOPOLOGY_TYPE_TRIANGLE_STRIP:
        return HL_INDEX_TYPE_TRIANGLE_STRIP;

    case HL_HH_TOPOLOGY_TYPE_TRIANGLE_LIST:
        return HL_INDEX_TYPE_TRIANGLE_LIST;
    }
}

HlU32 hlHHModelGetTopologyType(const HlBlob* blob)
{
    /* Default topology type used in all games before Tokyo 2020 is triangle strip. */
    HlU32 topologyType = HL_HH_TOPOLOGY_TYPE_TRIANGLE_STRIP;

    /* Get topology type if one was specified. */
    if (hlHHHeaderIsMirage(blob->data))
    {
        /* Get "Model" node. */
        const HlHHMirageHeader* mirageHeader = (const HlHHMirageHeader*)blob->data;
        const HlHHMirageNode* node = hlHHMirageGetNode(
            hlHHMirageHeaderGetNodes(mirageHeader),
            "Model   ", HL_FALSE);

        if (!node) return topologyType;

        /* Get "Topology" node. */
        node = hlHHMirageGetNode(hlHHMirageNodeGetChildren(node),
            "Topology", HL_FALSE);

        if (!node) return topologyType;

        /* Get topology type. */
        topologyType = node->value;
    }

    return topologyType;
}

static size_t hlINHHMeshSlotGetReqSize(
    const HlHHMeshSlot* HL_RESTRICT slot,
    size_t* HL_RESTRICT totalMeshCount)
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

        /* Account for material name. */
        reqSize += (strlen((const char*)hlOff32Get(
            &mesh->materialNameOffset)) + 1);

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

    /* Increase total mesh count and return required size. */
    *totalMeshCount += slot->meshCount;
    return reqSize;
}

static size_t hlINHHMeshGroupsGetReqSize(
    const HL_OFF32(HlHHMeshGroup)* HL_RESTRICT meshGroups,
    HlU32 meshGroupCount, size_t* HL_RESTRICT totalMeshCount)
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
        reqSize += hlINHHMeshSlotGetReqSize(&meshGroup->solid, totalMeshCount);
        reqSize += hlINHHMeshSlotGetReqSize(&meshGroup->transparent, totalMeshCount);
        reqSize += hlINHHMeshSlotGetReqSize(&meshGroup->punch, totalMeshCount);

        /* TODO: Account for special slots. */
    }

    return reqSize;
}

static HlResult hlINHHVertexFormatRead(const HlHHMeshSlot* HL_RESTRICT hhMeshSlot,
    HlVertexFormat* HL_RESTRICT * HL_RESTRICT curHlVertexFormat,
    void* HL_RESTRICT * HL_RESTRICT endPtr)
{
    /* Get pointers. */
    const HL_OFF32(HlHHMesh)* meshOffsets = (const HL_OFF32(HlHHMesh)*)
        hlOff32Get(&hhMeshSlot->meshesOffset);

    HlVertexElement* hlCurVertexElement = (HlVertexElement*)(*endPtr);

    /* Read mesh slot. */
    HlU32 i;
    for (i = 0; i < hhMeshSlot->meshCount; ++i)
    {
        /* Get pointers. */
        const HlHHMesh* mesh = (const HlHHMesh*)hlOff32Get(&meshOffsets[i]);

        /* Setup HlVertexFormat. */
        (*curHlVertexFormat)[i].vertexFormatSize = (size_t)mesh->vertexSize;
        (*curHlVertexFormat)[i].vertexElementCount = 0;
        (*curHlVertexFormat)[i].vertexElements = hlCurVertexElement;

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
                ++((*curHlVertexFormat)[i].vertexElementCount);

                /* Increment pointers. */
                ++hlCurVertexElement;
                ++hhCurVertexElement;
            }
        }
    }

    /* Inrease current vertex format pointer. */
    *curHlVertexFormat += hhMeshSlot->meshCount;

    /* Set end pointer and return success. */
    *endPtr = hlCurVertexElement;
    return HL_RESULT_SUCCESS;
}

static HlResult hlINHHMeshSlotRead(const HlHHMeshSlot* HL_RESTRICT hhMeshSlot,
    HlU8 indexType, HlMeshSlot* HL_RESTRICT hlMeshSlot,
    size_t* HL_RESTRICT globalMeshIndex, void* HL_RESTRICT * HL_RESTRICT endPtr)
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
            hlMesh->matRef.refType = HL_REF_TYPE_NAME;
            hlMesh->vertexFormatIndex = (*globalMeshIndex)++;
            hlMesh->vertexCount = (size_t)hhMesh->vertexCount;
            hlMesh->vertices = hlVertices;
            hlMesh->faceCount = (size_t)hhMesh->faceCount;
            hlMesh->faces = hlFaces;
            hlMesh->clockwise = HL_FALSE;
            hlMesh->indexType = indexType;

            /* Copy faces. */
            {
                const HlU16* hhFaces = (const HlU16*)hlOff32Get(&hhMesh->facesOffset);
                size_t i2;

                for (i2 = 0; i2 < hlMesh->faceCount; ++i2)
                {
                    hlFaces[i2] = (unsigned short)hhFaces[i2];
                }
            }

            /* Copy vertices/name and set end pointer. */
            {
                /* Copy vertices. */
                const void* hhVertices = (const void*)hlOff32Get(&hhMesh->verticesOffset);
                const size_t vertexBufferSize = ((size_t)hhMesh->vertexSize * hhMesh->vertexCount);
                char* matNamePtr = (char*)HL_ADD_OFF(hlVertices, vertexBufferSize);
                
                memcpy(hlVertices, hhVertices, vertexBufferSize);

                /* Copy material name. */
                hlMesh->matRef.data.name = matNamePtr;
                matNamePtr += (hlStrCopyAndLen((const char*)hlOff32Get(
                    &hhMesh->materialNameOffset), matNamePtr) + 1);

                /* Set end pointer. */
                *endPtr = matNamePtr;
            }
        }

        return HL_RESULT_SUCCESS;
    }
}

static HlResult hlINHHMeshGroupsRead(
    const HL_OFF32(HlHHMeshGroup)* HL_RESTRICT hhMeshGroups,
    HlU32 meshGroupCount, size_t totalMeshCount, HlU8 indexType,
    HlMeshGroup* HL_RESTRICT hlMeshGroups)
{
    HlVertexFormat* curHlVertexFormat = (HlVertexFormat*)&hlMeshGroups[meshGroupCount];
    void* endPtr = &curHlVertexFormat[totalMeshCount];
    size_t globalMeshIndex = 0;
    HlU32 i;
    HlResult result;

    /* Parse vertex formats. */
    for (i = 0; i < meshGroupCount; ++i)
    {
        /* Get mesh group pointer. */
        const HlHHMeshGroup* hhMeshGroup = (const HlHHMeshGroup*)
            hlOff32Get(&hhMeshGroups[i]);

        /* Read slots. */
        result = hlINHHVertexFormatRead(&hhMeshGroup->solid,
            &curHlVertexFormat, &endPtr);

        if (HL_FAILED(result)) return result;

        result = hlINHHVertexFormatRead(&hhMeshGroup->transparent,
            &curHlVertexFormat, &endPtr);

        if (HL_FAILED(result)) return result;

        result = hlINHHVertexFormatRead(&hhMeshGroup->punch,
            &curHlVertexFormat, &endPtr);

        if (HL_FAILED(result)) return result;

        /* TODO: Read special slots. */
    }

    /* Parse meshes. */
    for (i = 0; i < meshGroupCount; ++i)
    {
        /* Get mesh group pointer. */
        const HlHHMeshGroup* hhMeshGroup = (const HlHHMeshGroup*)
            hlOff32Get(&hhMeshGroups[i]);

        /* Read slots. */
        result = hlINHHMeshSlotRead(&hhMeshGroup->solid, indexType,
            &hlMeshGroups[i].solid, &globalMeshIndex, &endPtr);

        if (HL_FAILED(result)) return result;

        result = hlINHHMeshSlotRead(&hhMeshGroup->transparent, indexType,
            &hlMeshGroups[i].transparent, &globalMeshIndex, &endPtr);

        if (HL_FAILED(result)) return result;

        result = hlINHHMeshSlotRead(&hhMeshGroup->punch, indexType,
            &hlMeshGroups[i].punch, &globalMeshIndex, &endPtr);

        if (HL_FAILED(result)) return result;

        /* TODO: Read special slots. */
        hlMeshGroups[i].specialSlotCount = 0;
        hlMeshGroups[i].specialSlots = NULL;
    }

    return HL_RESULT_SUCCESS;
}

HlResult hlHHSkeletalModelV5Parse(
    const HlHHSkeletalModelV5* HL_RESTRICT hhModel,
    HlU32 hhTopologyType, HlModel* HL_RESTRICT * HL_RESTRICT hlModel)
{
    const HL_OFF32(HlHHMeshGroup)* hhMeshGroups = (const HL_OFF32(HlHHMeshGroup)*)
        hlOff32Get(&hhModel->meshGroupsOffset);

    HlModel* hlModelBuf;
    size_t totalMeshCount = 0;
    HlResult result;

    /* Allocate HlModel buffer. */
    {
        /* Compute total required buffer size. */
        size_t reqBufSize = sizeof(HlModel);
        reqBufSize += hlINHHMeshGroupsGetReqSize(hhMeshGroups,
            hhModel->meshGroupCount, &totalMeshCount);

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

        hlModelBuf->vertexFormatCount = totalMeshCount;

        result = hlINHHMeshGroupsRead(hhMeshGroups,
            hhModel->meshGroupCount, totalMeshCount,
            hlINHHGetIndexType(hhTopologyType), hlModelBuf->meshGroups);

        if (HL_FAILED(result)) return result;

        /* Set model pointer and return success. */
        *hlModel = hlModelBuf;
        return HL_RESULT_SUCCESS;
    }
}

HlResult hlHHSkeletalModelRead(HlBlob* HL_RESTRICT blob,
    HlModel* HL_RESTRICT * HL_RESTRICT hlModel)
{
    HlHHSkeletalModelV5* hhModel;
    HlU32 version, hhTopologyType;

    /* Fix HH general data. */
    hlHHFix(blob);

    /* Get HH skeletal model pointer and version number. */
    hhModel = (HlHHSkeletalModelV5*)hlHHGetData(blob, &version);
    if (!hhModel) return HL_ERROR_INVALID_DATA;

    /* Get HH topology type. */
    hhTopologyType = hlHHModelGetTopologyType(blob);

    /* TODO: Take version number into account. */

    /* Fix HH skeletal model data. */
    hlHHSkeletalModelV5Fix(hhModel);

    /* Parse HH skeletal model data into HlModel and return result. */
    return hlHHSkeletalModelV5Parse(hhModel,
        hhTopologyType, hlModel);
}

HlResult hlHHTerrainModelV5Parse(
    const HlHHTerrainModelV5* HL_RESTRICT hhModel,
    HlU32 hhTopologyType, HlModel* HL_RESTRICT * HL_RESTRICT hlModel)
{
    const HL_OFF32(HlHHMeshGroup)* hhMeshGroups = (const HL_OFF32(HlHHMeshGroup)*)
        hlOff32Get(&hhModel->meshGroupsOffset);

    const char* hhModelName = (const char*)hlOff32Get(&hhModel->nameOffset);
    HlModel* hlModelBuf;
    size_t modelNameLen, totalMeshCount = 0;
    HlResult result;

    /* Allocate HlModel buffer. */
    {
        /* Compute total required buffer size. */
        size_t reqBufSize = sizeof(HlModel);
        reqBufSize += hlINHHMeshGroupsGetReqSize(hhMeshGroups,
            hhModel->meshGroupCount, &totalMeshCount);

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

        hlModelBuf->vertexFormatCount = totalMeshCount;

        result = hlINHHMeshGroupsRead(hhMeshGroups,
            hhModel->meshGroupCount, totalMeshCount,
            hlINHHGetIndexType(hhTopologyType), hlModelBuf->meshGroups);

        if (HL_FAILED(result)) return result;

        /* Copy name. */
        memcpy(hlModelBuf + 1, hhModelName, modelNameLen);

        /* Set model pointer and return success. */
        *hlModel = hlModelBuf;
        return HL_RESULT_SUCCESS;
    }
}

HlResult hlHHTerrainModelRead(HlBlob* HL_RESTRICT blob,
    HlModel* HL_RESTRICT * HL_RESTRICT hlModel)
{
    HlHHTerrainModelV5* hhModel;
    HlU32 version, hhTopologyType;

    /* Fix HH general data. */
    hlHHFix(blob);

    /* Get HH terrain model pointer and version number. */
    hhModel = (HlHHTerrainModelV5*)hlHHGetData(blob, &version);
    if (!hhModel) return HL_ERROR_INVALID_DATA;

    /* Get HH topology type. */
    hhTopologyType = hlHHModelGetTopologyType(blob);

    /* TODO: Take version number into account. */

    /* Fix HH terrain model data. */
    hlHHTerrainModelV5Fix(hhModel);

    /* Parse HH terrain model data into HlModel and return result. */
    return hlHHTerrainModelV5Parse(hhModel,
        hhTopologyType, hlModel);
}

static HlResult hlINHHModelLoadMaterials(HlModel* HL_RESTRICT hlModel,
    HlNChar* HL_RESTRICT * HL_RESTRICT bufPtr, const size_t dirLen,
    HlBool* HL_RESTRICT bufOnHeap, size_t* HL_RESTRICT bufLen,
    HlMaterialList* HL_RESTRICT mats)
{
    HlMaterialNameList matNameRefs;
    size_t i;
    HlResult result = HL_RESULT_SUCCESS;

    /* Get material name references. */
    HL_LIST_INIT(matNameRefs);
    result = hlModelGetMatNameRefs(hlModel, &matNameRefs);
    if (HL_FAILED(result)) return result;

    /* Load materials and fix references. */
    for (i = 0; i < matNameRefs.count; ++i)
    {
        HlBlob* blob;
        HlMaterial* mat;
        size_t nameLen, extPos, totalLen;

        /* Compute lengths. */
#ifdef HL_IN_WIN32_UNICODE
        nameLen = hlStrGetReqLenUTF8ToUTF16(matNameRefs.data[i], 0);
        if (!nameLen)
        {
            result = HL_ERROR_UNKNOWN;
            goto failed_loop;
        }

        --nameLen; /* We don't want the null terminator in nameLen. */
#else
        nameLen = strlen(matNameRefs.data[i]);
#endif

        extPos = (dirLen + nameLen);
        totalLen = (extPos + 10); /* 9 characters in ".material" + null terminator. */

        /* Resize path buffer if necessary. */
        if (totalLen > *bufLen)
        {
            if (*bufOnHeap)
            {
                *bufPtr = HL_RESIZE_ARR(HlNChar, totalLen, *bufPtr);
            }
            else
            {
                *bufPtr = HL_ALLOC_ARR(HlNChar, totalLen);
                *bufOnHeap = HL_TRUE;
            }

            if (!(*bufPtr))
            {
                result = HL_ERROR_OUT_OF_MEMORY;
                goto failed_loop;
            }

            *bufLen = totalLen;
        }

        /* Copy name into buffer. */
#ifdef HL_IN_WIN32_UNICODE
        if (!hlStrConvUTF8ToUTF16NoAlloc(matNameRefs.data[i],
            (HlChar16*)(*bufPtr + dirLen), nameLen, *bufLen))
        {
            result = HL_ERROR_UNKNOWN;
            goto failed_loop;
        }
#else
        memcpy(*bufPtr + dirLen, matNameRefs.data[i],
            nameLen * sizeof(char));
#endif

        /* Copy extension into buffer. */
        memcpy(*bufPtr + extPos, HL_NTEXT(".material"), sizeof(HlNChar) * 10);

        /* Load material from file. */
        result = hlBlobLoad(*bufPtr, &blob);
        /* TODO: Make it possible to ignore missing material files? */
        if (HL_FAILED(result)) goto failed_loop;

        /* Parse material data. */
        result = hlHHMaterialRead(blob, matNameRefs.data[i], &mat);
        hlFree(blob);

        if (HL_FAILED(result)) goto failed_loop;

        /* Add HlMaterial to list. */
        result = HL_LIST_PUSH(*mats, mat);
        if (HL_FAILED(result))
        {
            hlFree(mat);
            goto failed_loop;
        }

        /* Fix material references within model. */
        hlModelFixMatRefs(&hlModel, 1, mat);
        continue;

    failed_loop:
        /* Free loaded materials, remove them from the mats list, and goto end. */
        while (i-- > 0)
        {
            hlFree(mats->data[--mats->count]);
        }

        goto end;
    }

end:
    /* Free list and return result. */
    HL_LIST_FREE(matNameRefs);
    return result;
}

HlResult hlHHModelLoadMaterials(HlModel* HL_RESTRICT hlModel,
    const HlNChar* HL_RESTRICT dir, HlMaterialList* HL_RESTRICT mats)
{
    HlNChar buf[255];
    HlNChar* bufPtr = buf;
    size_t bufLen = 255, dirLen = hlNStrLen(dir);

    /* Setup buffer and copy directory into it. */
    {
        /* Determine whether directory needs a path separator appended to it. */
        const HlBool needsSep = hlINPathCombineNeedsSep1(dir, dirLen);
        if (needsSep) ++dirLen;

        /* Allocate heap buffer if necessary. */
        if (dirLen > bufLen)
        {
            bufLen = (dirLen * 2);
            bufPtr = HL_ALLOC_ARR(HlNChar, bufLen);
            if (!bufPtr) return HL_ERROR_OUT_OF_MEMORY;
        }

        /* Copy directory into buffer. */
        memcpy(bufPtr, dir, dirLen * sizeof(HlNChar));

        /* Copy path separator into buffer if necessary. */
        if (needsSep) bufPtr[dirLen - 1] = HL_PATH_SEP;
    }

    /*
       Load materials, fix model references, free buffer
       if necessary, and return result.
    */
    {
        HlResult result;
        HlBool bufOnHeap = (bufPtr != buf);

        result = hlINHHModelLoadMaterials(hlModel, &bufPtr,
            dirLen, &bufOnHeap, &bufLen, mats);

        if (bufOnHeap) hlFree(bufPtr);
        return result;
    }
}
