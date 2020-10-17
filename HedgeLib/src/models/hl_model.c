#include "hedgelib/models/hl_model.h"
#include "hedgelib/materials/hl_material.h"
#include <string.h>
#include <stdio.h>

static size_t hlINMeshesFixMatRefs(HlMesh* HL_RESTRICT meshes,
    size_t meshCount, HlMaterial* HL_RESTRICT mat)
{
    size_t fixedRefs = 0, i;
    for (i = 0; i < meshCount; ++i)
    {
        if (meshes[i].matRef.refType == HL_REF_TYPE_NAME &&
            !strcmp(mat->name, meshes[i].matRef.data.name))
        {
            /* We found a match! Fix it. */
            meshes[i].matRef.refType = HL_REF_TYPE_PTR;
            meshes[i].matRef.data.ptr = mat;
            ++fixedRefs;
        }
    }

    return fixedRefs;
}

static size_t hlINMeshGroupsFixMatRefs(HlMeshGroup* HL_RESTRICT meshGroups,
    size_t meshGroupCount, HlMaterial* HL_RESTRICT mat)
{
    size_t fixedRefs = 0, i;
    for (i = 0; i < meshGroupCount; ++i)
    {
        /* Get materials in solid slot. */
        fixedRefs += hlINMeshesFixMatRefs(meshGroups[i].solid.meshes,
            meshGroups[i].solid.meshCount, mat);

        /* Get materials in transparent slot. */
        fixedRefs += hlINMeshesFixMatRefs(meshGroups[i].transparent.meshes,
            meshGroups[i].transparent.meshCount, mat);

        /* Get materials in punch slot. */
        fixedRefs += hlINMeshesFixMatRefs(meshGroups[i].punch.meshes,
            meshGroups[i].punch.meshCount, mat);

        /* TODO: Special slots. */
    }

    return fixedRefs;
}

size_t hlModelFixMatRefs(HlModel* HL_RESTRICT * HL_RESTRICT models,
    size_t modelCount, HlMaterial* HL_RESTRICT mat)
{
    size_t fixedRefs = 0, i;

    /* We can't fix references if we don't know the material's name. */
    if (!mat->name) return 0;

    /* Fix references. */
    for (i = 0; i < modelCount; ++i)
    {
        fixedRefs += hlINMeshGroupsFixMatRefs(models[i]->meshGroups,
            models[i]->meshGroupCount, mat);
    }
    
    return fixedRefs;
}

static HlResult hlINMeshesGetMatNameRefs(const HlMesh* HL_RESTRICT meshes,
    size_t meshCount, HlBool skipPtrRefs, HlMaterialNameList* HL_RESTRICT matNames)
{
    size_t i;
    HlResult result;

    for (i = 0; i < meshCount; ++i)
    {
        if (meshes[i].matRef.refType == HL_REF_TYPE_NAME)
        {
            /* Skip duplicates. */
            size_t i2;
            for (i2 = 0; i2 < matNames->count; ++i2)
            {
                if (matNames->data[i2] == meshes[i].matRef.data.name ||
                   !strcmp(meshes[i].matRef.data.name, matNames->data[i2]))
                {
                    goto next_loop_iteration;
                }
            }

            /* Add material name to list. */
            result = HL_LIST_PUSH(*matNames, meshes[i].matRef.data.name);
            if (HL_FAILED(result)) return result;
        }
        else if (!skipPtrRefs && meshes[i].matRef.data.ptr)
        {
            /* Skip duplicates. */
            size_t i2;
            for (i2 = 0; i2 < matNames->count; ++i2)
            {
                if (meshes[i].matRef.data.ptr->name &&
                    (matNames->data[i2] == meshes[i].matRef.data.ptr->name ||
                    !strcmp(meshes[i].matRef.data.ptr->name, matNames->data[i2])))
                {
                    goto next_loop_iteration;
                }
            }

            /* Add material name to list. */
            result = HL_LIST_PUSH(*matNames, meshes[i].matRef.data.ptr->name);
            if (HL_FAILED(result)) return result;
        }

    next_loop_iteration:
        continue;
    }

    return HL_RESULT_SUCCESS;
}

HlResult hlModelGetMatNameRefs(const HlModel* HL_RESTRICT model,
    HlBool skipPtrRefs, HlMaterialNameList* HL_RESTRICT matNameRefs)
{
    size_t i;
    HlResult result;

    for (i = 0; i < model->meshGroupCount; ++i)
    {
        /* Get material names in solid slot. */
        result = hlINMeshesGetMatNameRefs(model->meshGroups[i].solid.meshes,
            model->meshGroups[i].solid.meshCount, skipPtrRefs, matNameRefs);

        if (HL_FAILED(result)) return result;

        /* Get material names in transparent slot. */
        result = hlINMeshesGetMatNameRefs(model->meshGroups[i].transparent.meshes,
            model->meshGroups[i].transparent.meshCount, skipPtrRefs, matNameRefs);

        if (HL_FAILED(result)) return result;

        /* Get material names in punch slot. */
        result = hlINMeshesGetMatNameRefs(model->meshGroups[i].punch.meshes,
            model->meshGroups[i].punch.meshCount, skipPtrRefs, matNameRefs);

        if (HL_FAILED(result)) return result;

        /* TODO: Special slots. */
    }

    return HL_RESULT_SUCCESS;
}

static HlResult hlINMeshesGetMats(const HlMesh* HL_RESTRICT meshes,
    size_t meshCount, HlMaterialList* HL_RESTRICT mats)
{
    size_t i;
    HlResult result;

    for (i = 0; i < meshCount; ++i)
    {
        if (meshes[i].matRef.refType == HL_REF_TYPE_PTR)
        {
            /* Skip duplicates. */
            size_t i2;
            HlBool isDuplicate = HL_FALSE;

            for (i2 = 0; i2 < mats->count; ++i2)
            {
                if (mats->data[i2] == meshes[i].matRef.data.ptr ||
                   (meshes[i].matRef.data.ptr->name && mats->data[i2]->name &&
                   !strcmp(meshes[i].matRef.data.ptr->name, mats->data[i2]->name)))
                {
                    isDuplicate = HL_TRUE;
                    break;
                }
            }

            if (isDuplicate) continue;

            /* Add material pointer to materials list. */
            result = HL_LIST_PUSH(*mats, meshes[i].matRef.data.ptr);
            if (HL_FAILED(result)) return result;
        }
    }

    return HL_RESULT_SUCCESS;
}

HlResult hlModelGetMats(const HlModel* HL_RESTRICT model,
    HlMaterialList* HL_RESTRICT mats)
{
    size_t i;
    HlResult result;

    for (i = 0; i < model->meshGroupCount; ++i)
    {
        /* Get materials in solid slot. */
        result = hlINMeshesGetMats(model->meshGroups[i].solid.meshes,
            model->meshGroups[i].solid.meshCount, mats);

        if (HL_FAILED(result)) return result;

        /* Get materials in transparent slot. */
        result = hlINMeshesGetMats(model->meshGroups[i].transparent.meshes,
            model->meshGroups[i].transparent.meshCount, mats);

        if (HL_FAILED(result)) return result;

        /* Get materials in punch slot. */
        result = hlINMeshesGetMats(model->meshGroups[i].punch.meshes,
            model->meshGroups[i].punch.meshCount, mats);

        if (HL_FAILED(result)) return result;

        /* TODO: Special slots. */
    }

    return HL_RESULT_SUCCESS;
}
