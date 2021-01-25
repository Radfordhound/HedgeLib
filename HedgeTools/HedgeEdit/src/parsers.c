#include "parsers.h"
#include "hedgerender/hr_gpu_model.h"
#include "hedgerender/hr_gpu_texture.h"
#include "hedgelib/archives/hl_pacx.h"
#include "hedgelib/materials/hl_hh_material.h"
#include "hedgelib/models/hl_hh_model.h"
#include "hedgelib/terrain/hl_hh_terrain_instance_info.h"
#include "hedgelib/terrain/hl_terrain_group.h"
#include "hedgelib/terrain/hl_svcol.h"
#include "hedgelib/hl_list.h"

typedef struct ParseTypeInfo
{
    const char* const type;
    const ParserFunc parser;
}
ParseTypeInfo;

static const ParserFunc getParser(const ParseTypeInfo* HL_RESTRICT types,
    size_t typeCount, const char* HL_RESTRICT const typeStr)
{
    size_t i;
    for (i = 0; i < typeCount; ++i)
    {
        if (!strcmp(types[i].type, typeStr))
        {
            return types[i].parser;
        }
    }

    return NULL;
}

static HlResult ddsTextureParser(HrInstance* HL_RESTRICT instance,
    const char* HL_RESTRICT relPath, void* HL_RESTRICT data,
    size_t dataSize, HrResMgr* HL_RESTRICT resMgr,
    HrResourceID* HL_RESTRICT resID)
{
    HrGPUTexture* gpuTex;
    HlResult result;

    /* Create GPU texture from DDS file. */
    result = hrGPUTextureCreateFromDDS(instance, data, dataSize, &gpuTex);
    if (HL_FAILED(result)) return result;

    /* Add GPU texture to resource manager and return result. */
    result = hrResMgrAddResource(resMgr, relPath,
        HR_RES_TYPE_GPU_TEXTURE, gpuTex, resID);

    if (HL_FAILED(result))
    {
        hrGPUTextureDestroy(gpuTex);
        return result;
    }

    return result;
}

static HlResult hhTextureParser(HrInstance* HL_RESTRICT instance,
    const char* HL_RESTRICT relPath, void* HL_RESTRICT data,
    size_t dataSize, HrResMgr* HL_RESTRICT resMgr,
    HrResourceID* HL_RESTRICT resID)
{
    /* TODO: Support GFX2 textures for Lost World Wii U. */
    
    /* Parse texture as DDS. */
    return ddsTextureParser(instance, relPath,
        data, dataSize, resMgr, resID);
}

static HlResult hhMaterialParser(HrInstance* HL_RESTRICT instance,
    const char* HL_RESTRICT relPath, void* HL_RESTRICT data,
    size_t dataSize, HrResMgr* HL_RESTRICT resMgr,
    HrResourceID* HL_RESTRICT resID)
{
    HlMaterial* hlMaterial;
    HlResult result;

    result = hlHHMaterialRead(data, relPath, &hlMaterial);
    if (HL_FAILED(result)) return result;

    result = hrResMgrAddResource(resMgr, relPath,
        HR_RES_TYPE_MATERIAL, hlMaterial, resID);

    if (HL_FAILED(result))
    {
        hlFree(hlMaterial);
        return result;
    }

    return result;
}

static HlResult hhTerrainModelParser(HrInstance* HL_RESTRICT instance,
    const char* HL_RESTRICT relPath, void* HL_RESTRICT data,
    size_t dataSize, HrResMgr* HL_RESTRICT resMgr,
    HrResourceID* HL_RESTRICT resID)
{
    HlModel* hlModel;
    HrGPUModel* hrModel;
    HlResult result;

    result = hlHHTerrainModelRead(data, &hlModel);
    if (HL_FAILED(result)) return result;

    result = hrGPUModelCreate(instance, hlModel,
        HL_TRUE, resMgr, &hrModel);

    hlFree(hlModel);
    if (HL_FAILED(result)) return result;

    result = hrResMgrAddResource(resMgr, relPath,
        HR_RES_TYPE_GPU_MODEL, hrModel, resID);

    if (HL_FAILED(result))
    {
        hrGPUModelDestroy(hrModel);
        return result;
    }

    return result;
}

static HlResult hhTerrainInstanceInfoParser(HrInstance* HL_RESTRICT instance,
    const char* HL_RESTRICT relPath, void* HL_RESTRICT data,
    size_t dataSize, HrResMgr* HL_RESTRICT resMgr,
    HrResourceID* HL_RESTRICT resID)
{
    HlTerrainInstanceInfo* hlInstInfo;
    HlResult result;

    result = hlHHTerrainInstanceInfoRead(data, &hlInstInfo);
    if (HL_FAILED(result)) return result;

    result = hrResMgrAddResource(resMgr, relPath,
        HR_RES_TYPE_TERRAIN_INSTANCE_INFO, hlInstInfo, resID);

    if (HL_FAILED(result))
    {
        hlFree(hlInstInfo);
        return result;
    }

    return result;
}

static HlResult hhSvColParser(HrInstance* HL_RESTRICT instance,
    const char* HL_RESTRICT relPath, void* HL_RESTRICT data,
    size_t dataSize, HrResMgr* HL_RESTRICT resMgr,
    HrResourceID* HL_RESTRICT resID)
{
    HlSectorCollision* hlSvCol;
    HlResult result;

    result = hlSVColRead(data, dataSize, relPath, &hlSvCol);
    if (HL_FAILED(result)) return result;

    result = hrResMgrAddResource(resMgr, relPath,
        HR_RES_TYPE_SV_COL, hlSvCol, resID);

    if (HL_FAILED(result))
    {
        hlFree(hlSvCol);
        return result;
    }

    return HL_RESULT_SUCCESS;
}

/* Organized based on frequency information determined via a custom analyzation program */
static const ParseTypeInfo typesPACxV3[] =
{
    /* High Frequency */
    { "ResTexture", hhTextureParser },
    { "ResMirageMaterial", hhMaterialParser },
    { "ResMirageTerrainModel", hhTerrainModelParser },
    { "ResMirageTerrainInstanceInfo", hhTerrainInstanceInfoParser },

    /* Low Frequency */
    { "ResSvCol", hhSvColParser }
};

static const size_t typesPACxV3Count = (sizeof(typesPACxV3) / sizeof(ParseTypeInfo));

static HlResult INPACxV3ParseFileEntries(HrInstance* HL_RESTRICT instance,
    const HlPACxV3Node* fileNodes, const HlPACxV3Node* fileNode,
    const HlPACxV3Node* typeNodes, const HlPACxV3Node* instanceInfoNode,
    const ParserFunc parser, char* HL_RESTRICT pathBuf,
    HrResMgr* HL_RESTRICT resMgr, HlTerrainGroup* HL_RESTRICT terrainGroup)
{
    const HlS32* childIndices = (const HlS32*)hlOff64Get(&fileNode->childIndices);
    HlResult result;
    HlU16 i;

    if (fileNode->hasData)
    {
        const HlPACxV3DataEntry* dataEntry = (const HlPACxV3DataEntry*)
            hlOff64Get(&fileNode->data);

        /* Skip proxies if requested. */
        if (dataEntry->dataType != HL_PACXV3_DATA_TYPE_NOT_HERE)
        {
            HrResourceID resID;

            /* Parse data. */
            /* TODO: Account for BINA data!! */
            result = parser(instance, pathBuf, hlOff64Get(
                &dataEntry->data), dataEntry->dataSize,
                resMgr, &resID);

            if (HL_FAILED(result)) return result;

            if (!terrainGroup) goto recurse_through_children;

            if (parser == hhTerrainModelParser)
            {
                /* Create a copy of the model name. */
                const size_t mdlNameSize = (strlen(pathBuf) + 1);
                char* mdlName = hlAlloc(mdlNameSize);
                if (!mdlName) return HL_ERROR_OUT_OF_MEMORY;

                memcpy(mdlName, pathBuf, mdlNameSize);

                /* TODO: FREE THIS LATER!!!! */

                /* Add model name to terrain group. */
                result = HL_LIST_PUSH(terrainGroup->modelNames,
                    mdlName);

                if (HL_FAILED(result))
                {
                    hlFree(mdlName);
                    return result;
                }
            }
            else if (parser == hhTerrainInstanceInfoParser)
            {
                /* Create instance and add it to terrain group. */
                HlTerrainInstance terrainInstance;
                HlTerrainInstanceInfoRef instInfoRef;

                HL_LIST_INIT(terrainInstance.instanceInfoRefs);

                /* TODO: Compute this? */
                terrainInstance.boundingSphere.center.x = 0;
                terrainInstance.boundingSphere.center.y = 0;
                terrainInstance.boundingSphere.center.z = 0;
                terrainInstance.boundingSphere.radius = 0;

                instInfoRef.refType = HL_REF_TYPE_PTR;
                instInfoRef.data.ptr = hrResMgrGetTerrainInstanceInfo(
                    resMgr, resID);

                result = HL_LIST_PUSH(terrainInstance.instanceInfoRefs,
                    instInfoRef);

                if (HL_FAILED(result)) return result;

                result = HL_LIST_PUSH(terrainGroup->instances,
                    terrainInstance);

                if (HL_FAILED(result))
                {
                    HL_LIST_FREE(terrainInstance.instanceInfoRefs);
                    return result;
                }
            }
        }
    }
    else if (fileNode->name)
    {
        /* Copy name into path buffer. */
        strcpy(&pathBuf[fileNode->bufStartIndex],
            (const char*)hlOff64Get(&fileNode->name));
    }

    /* Recurse through children. */
recurse_through_children:
    for (i = 0; i < fileNode->childCount; ++i)
    {
        result = INPACxV3ParseFileEntries(instance,
            fileNodes, &fileNodes[childIndices[i]], typeNodes,
            instanceInfoNode, parser, pathBuf, resMgr,
            terrainGroup);

        if (HL_FAILED(result)) return result;
    }

    return HL_RESULT_SUCCESS;
}

static HlResult INPACxV3ParseTypeEntries(HrInstance* HL_RESTRICT instance,
    const HlPACxV3Node* typeNodes, const HlPACxV3Node* typeNode,
    const HlPACxV3Node* instanceInfoNode, char* HL_RESTRICT typeBuf,
    char* HL_RESTRICT pathBuf, HrResMgr* HL_RESTRICT resMgr,
    HlTerrainGroup* HL_RESTRICT terrainGroup)
{
    const HlS32* childIndices = (const HlS32*)hlOff64Get(&typeNode->childIndices);
    HlResult result;
    HlU16 i;

    if (typeNode->hasData)
    {
        const HlPACxV3NodeTree* fileTree = (const HlPACxV3NodeTree*)
            hlOff64Get(&typeNode->data);

        const HlPACxV3Node* fileNodes = (const HlPACxV3Node*)
            hlOff64Get(&fileTree->nodes);

        /* Get parser. */
        const ParserFunc parser = getParser(typesPACxV3,
            typesPACxV3Count, typeBuf);

        /* Skip unknown or unsupported types. */
        if (parser)
        {
            /* Parse file entries. */
            result = INPACxV3ParseFileEntries(instance, fileNodes,
                fileNodes, typeNodes, instanceInfoNode, parser,
                pathBuf, resMgr, terrainGroup);

            if (HL_FAILED(result)) return result;
        }
    }
    else if (typeNode->name)
    {
        /* Copy name into type buffer. */
        strcpy(&typeBuf[typeNode->bufStartIndex],
            (const char*)hlOff64Get(&typeNode->name));
    }

    /* Recurse through children. */
    for (i = 0; i < typeNode->childCount; ++i)
    {
        result = INPACxV3ParseTypeEntries(instance,
            typeNodes, &typeNodes[childIndices[i]], instanceInfoNode,
            typeBuf, pathBuf, resMgr, terrainGroup);

        if (HL_FAILED(result)) return result;
    }

    return HL_RESULT_SUCCESS;
}

HlResult parsePACxV3(HrInstance* HL_RESTRICT instance,
    HlBlob* HL_RESTRICT * HL_RESTRICT pacs,
    HlTerrainGroup* HL_RESTRICT terrainGroup,
    HrResMgr* HL_RESTRICT resMgr)
{
    const HlPACxV3Header* rootPac = (const HlPACxV3Header*)pacs[0]->data;

    /* PACxV3 names are hard-limited to 255, not including null terminator. */
    char typeBuf[256];
    char pathBuf[256];
    
    HlU32 i;
    HlResult result;

    /* Parse pacs. */
    for (i = 0; i < (rootPac->splitCount + 1); ++i)
    {
        const HlPACxV3Header* header = (const HlPACxV3Header*)pacs[i]->data;
        const HlPACxV3NodeTree* typeTree = hlPACxV3GetTypeTree(header);
        const HlPACxV3Node* typeNodes = (const HlPACxV3Node*)hlOff64Get(&typeTree->nodes);
        const HlPACxV3Node* instanceInfoNode = hlPACxV3GetNode(
            typeTree, "ResMirageTerrainInstanceInfo");

        result = INPACxV3ParseTypeEntries(instance, typeNodes,
            typeNodes, instanceInfoNode, typeBuf, pathBuf, resMgr,
            terrainGroup);

        if (HL_FAILED(result)) goto failed;
    }

    return HL_RESULT_SUCCESS;

failed:
    return result;
}
