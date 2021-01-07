#include "rendering.h"
#include "games.h"
#include "parsers.h"
#include "hedgerender/hr_gpu_model.h"
#include "hedgelib/terrain/hl_terrain_group.h"
#include "hedgelib/terrain/hl_terrain_instance_info.h"
#include "hedgelib/terrain/hl_svcol.h"
#include "hedgelib/archives/hl_pacx.h"
#include "hedgelib/io/hl_path.h"
#include "hedgelib/hl_text.h"
#include <stdio.h>

#ifdef HL_IN_WIN32_UNICODE
#define snprintf swprintf
#else
#define snprintf sprintf
#endif

#define LOADER_NAME(gameID) loader_##gameID

static HlResult createDefaultTerrainInstances(
    HlTerrainGroup* HL_RESTRICT terrainGroup,
    HrResMgr* HL_RESTRICT resMgr)
{
    HlTerrainInstanceInfo* defaultInstInfo;
    HlTerrainInstance defaultInst;
    HlTerrainInstanceInfoRef instInfoRef;
    size_t i;
    HlResult result;

    /* Setup default instance. */
    defaultInst.boundingSphere.center.x = 0;
    defaultInst.boundingSphere.center.y = 0;
    defaultInst.boundingSphere.center.z = 0;
    defaultInst.boundingSphere.radius = 0;

    /* Setup default instance info reference. */
    instInfoRef.refType = HL_REF_TYPE_PTR;

    for (i = 0; i < terrainGroup->modelNames.count; ++i)
    {
        size_t i2;
        for (i2 = 0; i2 < terrainGroup->instances.count; ++i2)
        {
            const HlTerrainInstance* inst = &terrainGroup->instances.data[i2];
            size_t i3;

            for (i3 = 0; i3 < inst->instanceInfoRefs.count; ++i3)
            {
                const char* instName =
                    (inst->instanceInfoRefs.data[i3].refType == HL_REF_TYPE_NAME) ?
                    inst->instanceInfoRefs.data[i3].data.name :
                    inst->instanceInfoRefs.data[i3].data.ptr->name;

                if (!strcmp(instName, terrainGroup->modelNames.data[i]))
                {
                    goto has_instance;
                }
            }
        }

        /* No instance was found; add a default instance. */
        defaultInstInfo = hlTerrainInstanceInfoCreateDefault(
            terrainGroup->modelNames.data[i]);

        if (!defaultInstInfo) return HL_ERROR_OUT_OF_MEMORY;

        /* Add default instance info to resource manager. */
        result = hrResMgrAddResource(resMgr, terrainGroup->modelNames.data[i],
            HR_RES_TYPE_TERRAIN_INSTANCE_INFO, defaultInstInfo, NULL);

        if (HL_FAILED(result))
        {
            hlTerrainInstanceInfoDestroy(defaultInstInfo);
            return result;
        }

        /*
           Reset instance info references list.
           NOTE: This is safe as the list data will be freed with the group.
        */
        HL_LIST_INIT(defaultInst.instanceInfoRefs);

        /* Setup instance info reference pointer. */
        instInfoRef.data.ptr = defaultInstInfo;

        /* Add instance info reference to instance. */
        result = HL_LIST_PUSH(defaultInst.instanceInfoRefs, instInfoRef);
        if (HL_FAILED(result)) return result;

        /* Add instance to terrain group. */
        result = HL_LIST_PUSH(terrainGroup->instances, defaultInst);
        if (HL_FAILED(result))
        {
            HL_LIST_FREE(defaultInst.instanceInfoRefs);
            return result;
        }

    has_instance:
        continue;
    }

    return HL_RESULT_SUCCESS;
}

static HlResult setupTerrainGroupGPUInstances(
    const HlTerrainGroup* HL_RESTRICT group,
    HrResMgr* HL_RESTRICT resMgr)
{
    size_t i;
    HlResult result;

    for (i = 0; i < group->instances.count; ++i)
    {
        const HlTerrainInstance* inst = &group->instances.data[i];
        size_t i2;

        for (i2 = 0; i2 < inst->instanceInfoRefs.count; ++i2)
        {
            const HlTerrainInstanceInfo* instInfo;
            HrGPUModel* model;

            /* Get instance info. */
            if (inst->instanceInfoRefs.data[i2].refType == HL_REF_TYPE_PTR)
            {
                instInfo = inst->instanceInfoRefs.data[i2].data.ptr;
            }
            else
            {
                HrResourceID instInfoID;
                result = hrResMgrGetTerrainInstanceInfoID(resMgr,
                    inst->instanceInfoRefs.data[i2].data.name, &instInfoID);

                if (HL_FAILED(result))
                {
                    if (result == HL_ERROR_NOT_FOUND) continue;
                    return result;
                }

                instInfo = hrResMgrGetTerrainInstanceInfo(resMgr, instInfoID);
            }

            /* Get model. */
            {
                HrResourceID mdlID;
                result = hrResMgrGetGPUModelID(resMgr,
                    instInfo->modelName, &mdlID);

                if (HL_FAILED(result))
                {
                    if (result == HL_ERROR_NOT_FOUND) continue;
                    return result;
                }

                model = hrResMgrGetGPUModel(resMgr, mdlID);
            }

            /* Add instance to model. */
            result = HL_LIST_PUSH(model->instances, instInfo->matrix);
            if (HL_FAILED(result)) return result;
        }
    }

    return HL_RESULT_SUCCESS;
}

static HlResult setupGITexturesHH2(
    const HlTerrainGroup* HL_RESTRICT group,
    HrResMgr* HL_RESTRICT resMgr)
{
    size_t i;
    HlResult result;

    for (i = 0; i < group->modelNames.count; ++i)
    {
        HrGPUModel* model;
        HrResourceID giTexResID, modelResID;

        result = hrResMgrGetGPUTextureID(resMgr,
            group->modelNames.data[i], &giTexResID);

        if (HL_FAILED(result))
        {
            if (result == HL_ERROR_NOT_FOUND)
                continue;

            return result;
        }

        result = hrResMgrGetGPUModelID(resMgr,
            group->modelNames.data[i], &modelResID);

        if (HL_FAILED(result)) return result;

        model = hrResMgrGetGPUModel(resMgr, modelResID);

        /* Set GI texture resource ID in model. */
        model->giTex = giTexResID;
    }

    return HL_RESULT_SUCCESS;
}

static HlResult loadPACxV3(HrInstance* HL_RESTRICT instance,
    const HlNChar* HL_RESTRICT pacPath, const char* HL_RESTRICT resName,
    HrResMgr* HL_RESTRICT resMgr)
{
    HlBlobList pacs;
    HlResult result;

    /* Initialize pac blob list. */
    HL_LIST_INIT(pacs);

    /* Load pacs. */
    result = hlPACxV3LoadAllInto(pacPath, &pacs, NULL);
    if (HL_FAILED(result)) return result;

    /* Parse data in pacs. */
    result = parsePACxV3(instance, pacs.data, NULL, resMgr);
    hlBlobListFree(&pacs);
    
    /* TODO: Just ignore failed resources? */
    /* TODO: Should we also parse models here if we find them?? */

    return result;
}

static HlResult loadHH2TerrainSectors(HrInstance* HL_RESTRICT instance,
    size_t pathBufLen, size_t fileNameIndex, HlU8 sectorCount,
    HlNChar* HL_RESTRICT pathBufPtr, HrResMgr* HL_RESTRICT resMgr)
{
    HlBlobList pacs;
    HlResult result;
    HlU8 i;

    for (i = 0; i < sectorCount; ++i)
    {
        HlTerrainGroup* terrainGroup;
        HL_LIST_INIT(pacs);

        /* Load pacs. */
        snprintf(&pathBufPtr[fileNameIndex], pathBufLen - fileNameIndex,
            HL_NTEXT("trr_s%02u.pac"), i);

        result = hlPACxLoadInto(pathBufPtr, 0, HL_TRUE, &pacs, NULL);
        if (HL_FAILED(result))
        {
            /* Just skip this sector if the pac wasn't found. */
            if (result == HL_ERROR_NOT_FOUND)
                continue;

            /* If we encountered another error, fail. */
            return result;
        }

        /* Create terrain group. */
        terrainGroup = HL_ALLOC_OBJ(HlTerrainGroup);
        if (!terrainGroup) return HL_ERROR_OUT_OF_MEMORY;

        /* Initialize terrain group. */
        terrainGroup->idType = HL_TERRAIN_GROUP_ID_TYPE_INDEX;
        terrainGroup->id.index = (size_t)i;
        HL_LIST_INIT(terrainGroup->instances);
        HL_LIST_INIT(terrainGroup->modelNames);

        /* Parse data in pacs. */
        result = parsePACxV3(instance, pacs.data, terrainGroup, resMgr);
        hlBlobListFree(&pacs);

        if (HL_FAILED(result)) /* TODO: Just ignore failed resources? */
        {
            hlTerrainGroupDestroy(terrainGroup);
            return result;
        }

        /* Add terrain group to resource manager. */
        result = hrResMgrAddResourceEx(resMgr, NULL, 0,
            HR_RES_TYPE_TERRAIN_GROUP, terrainGroup, NULL);

        if (HL_FAILED(result))
        {
            hlTerrainGroupDestroy(terrainGroup);
            return result;
        }

        /* Setup GI textures for terrain-models. */
        result = setupGITexturesHH2(terrainGroup, resMgr);
        if (HL_FAILED(result)) return result;
    }

    return HL_RESULT_SUCCESS;
}

static HlResult LOADER_NAME(FORCES)(HrInstance* HL_RESTRICT instance,
    const HlNChar* HL_RESTRICT dir, const HlNChar* HL_RESTRICT stageID,
    HrResMgr* HL_RESTRICT resMgr)
{
    HlNChar pathBuf[255];
    HlNChar* pathBufPtr = pathBuf;
    size_t pathBufLen = 255, fileNameIndex;
    HlResult result = HL_RESULT_SUCCESS;

    /* Get stage path. */
    {
        const size_t dirLen = hlNStrLen(dir);
        const size_t stageIDLen = hlNStrLen(stageID);

        /* _trr_cmn.pac + path separator. */
        const size_t maxFileNameLen = (stageIDLen + 13);

        /* Compute required path buffer length. */
        size_t reqPathBufLen = (dirLen + stageIDLen);
        if (hlPathCombineNeedsSep(dir, stageID, dirLen)) ++reqPathBufLen;
        
        fileNameIndex = reqPathBufLen;
        reqPathBufLen += maxFileNameLen;

        /* If necessary, allocate path buffer on heap. */
        if (reqPathBufLen >= pathBufLen)
        {
            pathBufPtr = HL_ALLOC_ARR(HlNChar, reqPathBufLen + 1);
            if (!pathBufPtr) return HL_ERROR_OUT_OF_MEMORY;
        }

        /* Combine paths. */
        hlPathCombineNoAlloc(dir, stageID, dirLen, stageIDLen, pathBufPtr);
        pathBufPtr[fileNameIndex++] = HL_PATH_SEP;

        /* Append stageID filename prefix. */
        memcpy(&pathBufPtr[fileNameIndex], stageID, stageIDLen * sizeof(HlNChar));
        fileNameIndex += stageIDLen;
        pathBufPtr[fileNameIndex++] = HL_NTEXT('_');
    }

    /* TODO: Have PACxV3 load blob functions!! */

    /* Load terrain common. */
    hlNStrCopy(HL_NTEXT("trr_cmn.pac"), &pathBufPtr[fileNameIndex]);
    result = loadPACxV3(instance, pathBufPtr, "trr_cmn", resMgr);
    if (HL_FAILED(result)) goto end;

    /* Load terrain sectors. */
    /* TODO: Parse actstgmission.lua!! */
    result = loadHH2TerrainSectors(instance, pathBufLen,
        fileNameIndex, 100, pathBufPtr, resMgr);

    if (HL_FAILED(result)) goto end;

    /* TODO */

    /* Setup GPU instances. */
    {
        const size_t terrainGroupCount = hrResMgrGetResourceCount(
            resMgr, HR_RES_TYPE_TERRAIN_GROUP);

        unsigned short i;

        for (i = 1; i < terrainGroupCount; ++i)
        {
            HlTerrainGroup* terrainGroup = hrResMgrGetTerrainGroup(resMgr, i);

            /* Create default instances for any models that don't have them. */
            result = createDefaultTerrainInstances(terrainGroup, resMgr);
            if (HL_FAILED(result)) goto end;

            /* Create GPU instances. */
            result = setupTerrainGroupGPUInstances(terrainGroup, resMgr);
            if (HL_FAILED(result)) goto end;
        }
    }

    /* Setup SVCOL instances. */
    {
        const size_t svcolCount = hrResMgrGetResourceCount(
            resMgr, HR_RES_TYPE_SV_COL);

        unsigned short i;
        
        /* TODO: Move this line elsewhere? */
        HL_LIST_CLEAR(hrResMgrGetGPUModel(resMgr, 0)->instances);

        for (i = 1; i < svcolCount; ++i)
        {
            result = updateSVColInstances(
                hrResMgrGetSVCol(resMgr, i),
                0.1f);

            if (HL_FAILED(result)) goto end;
        }
    }
    
    /* Set result to success so we don't fail just because sector 99 didn't exist. */
    result = HL_RESULT_SUCCESS;

end:
    if (pathBufPtr != pathBuf) hlFree(pathBufPtr);
    return result;
}

static HlResult LOADER_NAME(TOKYO_2020)(HrInstance* HL_RESTRICT instance,
    const HlNChar* HL_RESTRICT dir, const HlNChar* HL_RESTRICT stageID,
    HrResMgr* HL_RESTRICT resMgr)
{
    HlNChar pathBuf[255];
    HlNChar* pathBufPtr = pathBuf;
    size_t pathBufLen = 255, fileNameIndex;
    HlResult result = HL_RESULT_SUCCESS;

    /* Get stage path. */
    {
        const size_t dirLen = hlNStrLen(dir);
        const size_t stageIDLen = hlNStrLen(stageID);

        /* _trr_cmn.pac + path separator. */
        const size_t maxFileNameLen = (stageIDLen + 13);

        /* Compute required path buffer length. */
        size_t reqPathBufLen = (dirLen + stageIDLen + 7);
        if (hlPathCombineNeedsSep(dir, HL_NTEXT("terrain"), dirLen))
        {
            ++reqPathBufLen;
        }

        if (hlPathCombineNeedsSep(HL_NTEXT("terrain"), stageID, dirLen))
        {
            ++reqPathBufLen;
        }
        
        fileNameIndex = reqPathBufLen;
        reqPathBufLen += maxFileNameLen;

        /* If necessary, allocate path buffer on heap. */
        if (reqPathBufLen >= pathBufLen)
        {
            pathBufPtr = HL_ALLOC_ARR(HlNChar, reqPathBufLen + 1);
            if (!pathBufPtr) return HL_ERROR_OUT_OF_MEMORY;
        }

        /* Combine paths. */
        {
            /* Combine dir and "terrain". */
            size_t pathEndIndex = hlPathCombineNoAlloc(
                dir, HL_NTEXT("terrain"), dirLen, 7, pathBufPtr);

            /* Append path separator if necessary. */
            if (hlPathCombineNeedsSep(HL_NTEXT("terrain"), stageID, dirLen))
            {
                pathBufPtr[pathEndIndex++] = HL_PATH_SEP;
            }

            /* Append stageID. */
            memcpy(&pathBufPtr[pathEndIndex], stageID, stageIDLen * sizeof(HlNChar));
            pathBufPtr[fileNameIndex++] = HL_PATH_SEP;
        }

        /* Append stageID filename prefix. */
        memcpy(&pathBufPtr[fileNameIndex], stageID, stageIDLen * sizeof(HlNChar));
        fileNameIndex += stageIDLen;
        pathBufPtr[fileNameIndex++] = HL_NTEXT('_');
    }

    /* TODO: Have PACxV4 load blob functions!! */

    /* Load terrain miscellaneous. */
    {
        HlBlobList pacs;
        HL_LIST_INIT(pacs);

        /* Load pacs. */
        hlNStrCopy(HL_NTEXT("trr_misc.pac"), &pathBufPtr[fileNameIndex]);
        result = hlPACxV4LoadInto(pathBufPtr, HL_TRUE, &pacs, NULL);
        if (HL_FAILED(result)) goto end;

        /* Parse data in pacs. */
        result = parsePACxV3(instance, pacs.data, NULL, resMgr);
        hlBlobListFree(&pacs);

        if (HL_FAILED(result)) goto end; /* TODO: Just ignore failed resources? */
    }

    /* Load terrain common. */
    {
        HlBlobList pacs;
        HL_LIST_INIT(pacs);

        /* Load pacs. */
        hlNStrCopy(HL_NTEXT("trr_cmn.pac"), &pathBufPtr[fileNameIndex]);
        result = hlPACxV4LoadInto(pathBufPtr, HL_TRUE, &pacs, NULL);
        if (HL_FAILED(result)) goto end;

        /* Parse data in pacs. */
        result = parsePACxV3(instance, pacs.data, NULL, resMgr);
        hlBlobListFree(&pacs);

        if (HL_FAILED(result)) goto end; /* TODO: Just ignore failed resources? */
        /* TODO: Should we also parse models here if we find them?? */
    }

    /* Load terrain sectors. */
    /* TODO: Load sectors properly; how does the game do it??? */
    result = loadHH2TerrainSectors(instance, pathBufLen,
        fileNameIndex, 100, pathBufPtr, resMgr);

    if (HL_FAILED(result)) goto end;

    /* TODO */

    /* Setup GPU instances. */
    {
        const size_t terrainGroupCount = hrResMgrGetResourceCount(
            resMgr, HR_RES_TYPE_TERRAIN_GROUP);

        unsigned short i;

        for (i = 1; i < terrainGroupCount; ++i)
        {
            HlTerrainGroup* terrainGroup = hrResMgrGetTerrainGroup(resMgr, i);

            /* Create default instances for any models that don't have them. */
            result = createDefaultTerrainInstances(terrainGroup, resMgr);
            if (HL_FAILED(result)) goto end;

            /* Create GPU instances. */
            result = setupTerrainGroupGPUInstances(terrainGroup, resMgr);
            if (HL_FAILED(result)) goto end;
        }
    }

    /* Setup SVCOL instances. */
    {
        const size_t svcolCount = hrResMgrGetResourceCount(
            resMgr, HR_RES_TYPE_SV_COL);

        unsigned short i;
        
        /* TODO: Move this line elsewhere? */
        HL_LIST_CLEAR(hrResMgrGetGPUModel(resMgr, 0)->instances);

        for (i = 1; i < svcolCount; ++i)
        {
            result = updateSVColInstances(
                hrResMgrGetSVCol(resMgr, i),
                1.0f);

            if (HL_FAILED(result)) goto end;
        }
    }
    
    /* Set result to success so we don't fail just because sector 99 didn't exist. */
    result = HL_RESULT_SUCCESS;

end:
    if (pathBufPtr != pathBuf) hlFree(pathBufPtr);
    return result;
}

static HlResult LOADER_NAME(SAKURA)(HrInstance* HL_RESTRICT instance,
    const HlNChar* HL_RESTRICT dir, const HlNChar* HL_RESTRICT stageID,
    HrResMgr* HL_RESTRICT resMgr)
{
    HlNChar pathBuf[255];
    HlNChar* pathBufPtr = pathBuf;
    size_t pathBufLen = 255, fileNameIndex;
    HlResult result = HL_RESULT_SUCCESS;

    /* Get stage path. */
    {
        const size_t dirLen = hlNStrLen(dir);
        const size_t stageIDLen = hlNStrLen(stageID);

        /* _trr_cmn.pac + path separator. */
        const size_t maxFileNameLen = (stageIDLen + 13);

        /* Compute required path buffer length. */
        size_t reqPathBufLen = (dirLen + stageIDLen + 5);
        if (hlPathCombineNeedsSep(dir, HL_NTEXT("stage"), dirLen))
        {
            ++reqPathBufLen;
        }

        if (hlPathCombineNeedsSep(HL_NTEXT("stage"), stageID, dirLen))
        {
            ++reqPathBufLen;
        }
        
        fileNameIndex = reqPathBufLen;
        reqPathBufLen += maxFileNameLen;

        /* If necessary, allocate path buffer on heap. */
        if (reqPathBufLen >= pathBufLen)
        {
            pathBufPtr = HL_ALLOC_ARR(HlNChar, reqPathBufLen + 1);
            if (!pathBufPtr) return HL_ERROR_OUT_OF_MEMORY;
        }

        /* Combine paths. */
        {
            /* Combine dir and "terrain". */
            size_t pathEndIndex = hlPathCombineNoAlloc(
                dir, HL_NTEXT("stage"), dirLen, 5, pathBufPtr);

            /* Append path separator if necessary. */
            if (hlPathCombineNeedsSep(HL_NTEXT("stage"), stageID, dirLen))
            {
                pathBufPtr[pathEndIndex++] = HL_PATH_SEP;
            }

            /* Append stageID. */
            memcpy(&pathBufPtr[pathEndIndex], stageID, stageIDLen * sizeof(HlNChar));
            pathBufPtr[fileNameIndex++] = HL_PATH_SEP;
        }

        /* Append stageID filename prefix. */
        memcpy(&pathBufPtr[fileNameIndex], stageID, stageIDLen * sizeof(HlNChar));
        fileNameIndex += stageIDLen;
        pathBufPtr[fileNameIndex++] = HL_NTEXT('_');
    }

    /* TODO: Have PACxV4 load blob functions!! */

    /* Load terrain miscellaneous. */
    {
        HlBlobList pacs;
        HL_LIST_INIT(pacs);

        /* Load pacs. */
        hlNStrCopy(HL_NTEXT("trr_misc.pac"), &pathBufPtr[fileNameIndex]);
        result = hlPACxV4LoadInto(pathBufPtr, HL_TRUE, &pacs, NULL);
        if (HL_FAILED(result)) goto end;

        /* Parse data in pacs. */
        result = parsePACxV3(instance, pacs.data, NULL, resMgr);
        hlBlobListFree(&pacs);

        if (HL_FAILED(result)) goto end; /* TODO: Just ignore failed resources? */
    }

    /* Load terrain common. */
    {
        HlBlobList pacs;
        HL_LIST_INIT(pacs);

        /* Load pacs. */
        hlNStrCopy(HL_NTEXT("trr_cmn.pac"), &pathBufPtr[fileNameIndex]);
        result = hlPACxV4LoadInto(pathBufPtr, HL_TRUE, &pacs, NULL);
        if (HL_FAILED(result)) goto end;

        /* Parse data in pacs. */
        result = parsePACxV3(instance, pacs.data, NULL, resMgr);
        hlBlobListFree(&pacs);

        if (HL_FAILED(result)) goto end; /* TODO: Just ignore failed resources? */
        /* TODO: Should we also parse models here if we find them?? */
    }

    /* Load terrain sectors. */
    /* TODO: Load sectors properly; how does the game do it??? */
    result = loadHH2TerrainSectors(instance, pathBufLen,
        fileNameIndex, 100, pathBufPtr, resMgr);

    if (HL_FAILED(result)) goto end;

    /* TODO */

    /* Setup GPU instances. */
    {
        const size_t terrainGroupCount = hrResMgrGetResourceCount(
            resMgr, HR_RES_TYPE_TERRAIN_GROUP);

        unsigned short i;

        for (i = 1; i < terrainGroupCount; ++i)
        {
            HlTerrainGroup* terrainGroup = hrResMgrGetTerrainGroup(resMgr, i);

            /* Create default instances for any models that don't have them. */
            result = createDefaultTerrainInstances(terrainGroup, resMgr);
            if (HL_FAILED(result)) goto end;

            /* Create GPU instances. */
            result = setupTerrainGroupGPUInstances(terrainGroup, resMgr);
            if (HL_FAILED(result)) goto end;
        }
    }

    /* Setup SVCOL instances. */
    {
        const size_t svcolCount = hrResMgrGetResourceCount(
            resMgr, HR_RES_TYPE_SV_COL);

        unsigned short i;
        
        /* TODO: Move this line elsewhere? */
        HL_LIST_CLEAR(hrResMgrGetGPUModel(resMgr, 0)->instances);

        for (i = 1; i < svcolCount; ++i)
        {
            result = updateSVColInstances(
                hrResMgrGetSVCol(resMgr, i),
                1.0f);

            if (HL_FAILED(result)) goto end;
        }
    }
    
    /* Set result to success so we don't fail just because sector 99 didn't exist. */
    result = HL_RESULT_SUCCESS;

end:
    if (pathBufPtr != pathBuf) hlFree(pathBufPtr);
    return result;
}

const GameInfo GameInfos[GAME_TYPE_COUNT + 1] =
{
/* Auto-generate GameInfos. */
#define GAME(gameID, name) { name, LOADER_NAME(gameID) },
#include "games_autogen.h"

    /* Append a dummy struct to avoid trailing-comma issues. */
    { "DUMMY", NULL }
};

GameType CurGameType = GAME_TYPE_COUNT;
HlResult StageLoadResult = HL_RESULT_SUCCESS;
HlBool IsLoadingStage = HL_FALSE;

void startLoadingStage(HrInstance* HL_RESTRICT instance, GameType gameType,
    const HlNChar* HL_RESTRICT dir, const HlNChar* HL_RESTRICT stageID,
    HrResMgr* HL_RESTRICT resMgr)
{
    /* TODO: Make this multi-threaded!! */

    /* Clear resources. */
    /* TODO */

    /* Set current game type. */
    CurGameType = gameType;

    /* Begin loading stage. */
    IsLoadingStage = HL_TRUE;
    /*wasLoadingStage = HL_TRUE;*/
    StageLoadResult = GameInfos[gameType].loader(instance, dir, stageID, resMgr);
    IsLoadingStage = HL_FALSE;
    queueDrawListRebuild();
}

void startLoadingStageUTF8(HrInstance* HL_RESTRICT instance, GameType gameType,
    const char* HL_RESTRICT dir, const char* HL_RESTRICT stageID,
    HrResMgr* HL_RESTRICT resMgr)
{
#ifdef HL_IN_WIN32_UNICODE
    HlNChar* const nativeDir = hlStrConvUTF8ToNative(dir, 0);
    HlNChar* const nativeStageID = hlStrConvUTF8ToNative(stageID, 0);

    startLoadingStage(instance, gameType,
        nativeDir, nativeStageID, resMgr);

    hlFree(nativeDir);
    hlFree(nativeStageID);
#else
    startLoadingStage(gameType, dir, stageID, resMgr);
#endif
}
