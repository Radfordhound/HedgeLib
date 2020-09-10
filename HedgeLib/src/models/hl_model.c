#include "hedgelib/hl_math.h"
#include "hedgelib/models/hl_model.h"
#include "hedgelib/io/hl_file.h"
#include "hedgelib/io/hl_path.h"
#include "hedgelib/hl_list.h"
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

        /* Get materials in boolean slot. */
        fixedRefs += hlINMeshesFixMatRefs(meshGroups[i].boolean.meshes,
            meshGroups[i].boolean.meshCount, mat);

        /* TODO: Special slots. */
    }

    return fixedRefs;
}

size_t hlModelFixMatRefs(HlModel** HL_RESTRICT models,
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
    size_t meshCount, HlMaterialNameList* HL_RESTRICT matNameRefs)
{
    size_t i;
    HlResult result;

    for (i = 0; i < meshCount; ++i)
    {
        if (meshes[i].matRef.refType == HL_REF_TYPE_NAME)
        {
            /* Skip duplicates. */
            size_t i2;
            HlBool isDuplicate = HL_FALSE;

            for (i2 = 0; i2 < matNameRefs->count; ++i2)
            {
                if (matNameRefs->data[i2] == meshes[i].matRef.data.name ||
                   !strcmp(meshes[i].matRef.data.name, matNameRefs->data[i2]))
                {
                    isDuplicate = HL_TRUE;
                    break;
                }
            }

            if (isDuplicate) continue;

            /* Add material name reference to list. */
            result = HL_LIST_PUSH(*matNameRefs, meshes[i].matRef.data.name);
            if (HL_FAILED(result)) return result;
        }
    }

    return HL_RESULT_SUCCESS;
}

HlResult hlModelGetMatNameRefs(const HlModel* HL_RESTRICT model,
    HlMaterialNameList* HL_RESTRICT matNameRefs)
{
    size_t i;
    HlResult result;

    for (i = 0; i < model->meshGroupCount; ++i)
    {
        /* Get material name references in solid slot. */
        result = hlINMeshesGetMatNameRefs(model->meshGroups[i].solid.meshes,
            model->meshGroups[i].solid.meshCount, matNameRefs);

        if (HL_FAILED(result)) return result;

        /* Get material name references in transparent slot. */
        result = hlINMeshesGetMatNameRefs(model->meshGroups[i].transparent.meshes,
            model->meshGroups[i].transparent.meshCount, matNameRefs);

        if (HL_FAILED(result)) return result;

        /* Get material name references in boolean slot. */
        result = hlINMeshesGetMatNameRefs(model->meshGroups[i].boolean.meshes,
            model->meshGroups[i].boolean.meshCount, matNameRefs);

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

        /* Get materials in boolean slot. */
        result = hlINMeshesGetMats(model->meshGroups[i].boolean.meshes,
            model->meshGroups[i].boolean.meshCount, mats);

        if (HL_FAILED(result)) return result;

        /* TODO: Special slots. */
    }

    return HL_RESULT_SUCCESS;
}

HlResult hlModelWriteMTL(const HlModel** HL_RESTRICT models,
    size_t modelCount, HlFile* HL_RESTRICT file)
{
    HlMaterialList mats;
    size_t i;
    HlResult result;

    /* Initialize materials list. */
    HL_LIST_INIT(mats);

    /* Generate materials list. */
    for (i = 0; i < modelCount; ++i)
    {
        result = hlModelGetMats(models[i], &mats);
        if (HL_FAILED(result)) return result;
    }

    /* Write MTL, free list, and return result. */
    result = hlMaterialWriteMTL(mats.data, mats.count, file);
    HL_LIST_FREE(mats);
    return result;
}

HlResult hlModelExportMTL(const HlModel** HL_RESTRICT models,
    size_t modelCount, const HlNChar* HL_RESTRICT filePath)
{
    HlFile* file;
    HlResult result;

    /* Open file. */
    result = hlFileOpen(filePath, HL_FILE_MODE_WRITE, &file);
    if (HL_FAILED(result)) return result;

    /* Write MTL to file, close it, and return. */
    result = hlModelWriteMTL(models, modelCount, file);
    if (HL_FAILED(result))
    {
        hlFileClose(file);
        return result;
    }

    return hlFileClose(file);
}

HlModel** hlModelReadOBJ(const HlBlob* blob, size_t* modelCount)
{
    /* TODO */
    return NULL;
}

static HlResult hlINMeshWriteVerticesOBJVec1(const HlMesh* HL_RESTRICT mesh,
    const HlVertexFormat* HL_RESTRICT vtxFmt, const HlVertexElement* HL_RESTRICT vtxElem,
    size_t bufStartIndex, char* HL_RESTRICT buf, size_t* HL_RESTRICT globalVertexCount,
    HlFile* HL_RESTRICT file)
{
    const void* verticesBufPtr;
    size_t i, bufLen;
    int len;
    HlResult result;

    /* Write vertices. */
    for (i = 0; i < mesh->vertexCount; ++i)
    {
        /* Get pointer to data within vertices buffer. */
        verticesBufPtr = HL_ADD_OFFC(mesh->vertices,
            (i * vtxFmt->vertexFormatSize) + (size_t)vtxElem->offset);

        switch (vtxElem->type & HL_VERTEX_ELEM_FORMAT_MASK)
        {
        case HL_VERTEX_ELEM_FORMAT_FLOAT:
        {
            const float* data = (const float*)verticesBufPtr;
            double x;

            /* Store X value. */
            x = (double)data[0];

            /* Check for NAN and infinity values. */
            if (HL_IS_NAN(x)) x = 0;

            /* Store data in buffer. */
            len = sprintf(&buf[bufStartIndex], "%f\n", x);
            if (len < 0) return HL_ERROR_UNKNOWN;

            /* Increase buffer length and global vertex count. */
            bufLen = (bufStartIndex + (size_t)len);
            ++(*globalVertexCount);
            break;
        }

        /* TODO: Support half floats. */

        default: return HL_ERROR_UNSUPPORTED;
        }

        /* Write buffer contents to file. */
        result = hlFileWrite(file, bufLen * sizeof(char), buf, 0);
        if (HL_FAILED(result)) return result;
    }

    return HL_RESULT_SUCCESS;
}

static HlResult hlINMeshWriteVerticesOBJVec2(const HlMesh* HL_RESTRICT mesh,
    const HlVertexFormat* HL_RESTRICT vtxFmt, const HlVertexElement* HL_RESTRICT vtxElem,
    size_t bufStartIndex, char* HL_RESTRICT buf, size_t* HL_RESTRICT globalVertexCount,
    HlFile* HL_RESTRICT file)
{
    const void* verticesBufPtr;
    size_t i, bufLen;
    int len;
    HlResult result;

    /* Write vertices. */
    for (i = 0; i < mesh->vertexCount; ++i)
    {
        /* Get pointer to data within vertices buffer. */
        verticesBufPtr = HL_ADD_OFFC(mesh->vertices,
            (i * vtxFmt->vertexFormatSize) + (size_t)vtxElem->offset);

        switch (vtxElem->type & HL_VERTEX_ELEM_FORMAT_MASK)
        {
        case HL_VERTEX_ELEM_FORMAT_FLOAT:
        {
            const float* data = (const float*)verticesBufPtr;
            double x, y;

            /* Store XY values. */
            x = (double)data[0];
            y = (double)data[1];

            /* Check for NAN and infinity values. */
            if (HL_IS_NAN(x)) x = 0;
            if (HL_IS_NAN(y)) y = 0;

            /* Flip Y if this is a texture coordinate. */
            if ((vtxElem->type & HL_VERTEX_ELEM_TYPE_MASK) ==
                HL_VERTEX_ELEM_TYPE_TEXCOORD)
            {
                y = -y;
            }

            /* Store data in buffer. */
            len = sprintf(&buf[bufStartIndex], "%f %f\n", x, y);
            if (len < 0) return HL_ERROR_UNKNOWN;

            /* Increase buffer length and global vertex count. */
            bufLen = (bufStartIndex + (size_t)len);
            ++(*globalVertexCount);
            break;
        }

        /* TODO: Support half floats. */

        default: return HL_ERROR_UNSUPPORTED;
        }

        /* Write buffer contents to file. */
        result = hlFileWrite(file, bufLen * sizeof(char), buf, 0);
        if (HL_FAILED(result)) return result;
    }

    return HL_RESULT_SUCCESS;
}

static HlResult hlINMeshWriteVerticesOBJVec3(const HlMesh* HL_RESTRICT mesh,
    const HlVertexFormat* HL_RESTRICT vtxFmt, const HlVertexElement* HL_RESTRICT vtxElem,
    size_t bufStartIndex, char* HL_RESTRICT buf, size_t* HL_RESTRICT globalVertexCount,
    HlFile* HL_RESTRICT file)
{
    const void* verticesBufPtr;
    size_t i, bufLen;
    int len;
    HlResult result;

    /* Write vertices. */
    for (i = 0; i < mesh->vertexCount; ++i)
    {
        /* Get pointer to data within vertices buffer. */
        verticesBufPtr = HL_ADD_OFFC(mesh->vertices,
            (i * vtxFmt->vertexFormatSize) + (size_t)vtxElem->offset);

        switch (vtxElem->type & HL_VERTEX_ELEM_FORMAT_MASK)
        {
        case HL_VERTEX_ELEM_FORMAT_FLOAT:
        {
            const float* data = (const float*)verticesBufPtr;
            double x, y, z;

            /* Store XYZ values. */
            x = (double)data[0];
            y = (double)data[1];
            z = (double)data[2];

            /* Check for NAN and infinity values. */
            if (HL_IS_NAN(x)) x = 0;
            if (HL_IS_NAN(y)) y = 0;
            if (HL_IS_NAN(z)) z = 0;

            /* Flip Y if this is a texture coordinate. */
            if ((vtxElem->type & HL_VERTEX_ELEM_TYPE_MASK) ==
                HL_VERTEX_ELEM_TYPE_TEXCOORD)
            {
                y = -y;
            }

            /* Store data in buffer. */
            len = sprintf(&buf[bufStartIndex], "%f %f %f\n", x, y, z);
            if (len < 0) return HL_ERROR_UNKNOWN;

            /* Increase buffer length and global vertex count. */
            bufLen = (bufStartIndex + (size_t)len);
            ++(*globalVertexCount);
            break;
        }

        /* TODO: Support half floats. */

        default: return HL_ERROR_UNSUPPORTED;
        }

        /* Write buffer contents to file. */
        result = hlFileWrite(file, bufLen * sizeof(char), buf, 0);
        if (HL_FAILED(result)) return result;
    }

    return HL_RESULT_SUCCESS;
}

static HlResult hlINMeshWriteVerticesOBJVec4(const HlMesh* HL_RESTRICT mesh,
    const HlVertexFormat* HL_RESTRICT vtxFmt, const HlVertexElement* HL_RESTRICT vtxElem,
    size_t bufStartIndex, char* HL_RESTRICT buf, size_t* HL_RESTRICT globalVertexCount,
    HlFile* HL_RESTRICT file)
{
    const void* verticesBufPtr;
    size_t i, bufLen;
    int len;
    HlResult result;

    /* Write vertices. */
    for (i = 0; i < mesh->vertexCount; ++i)
    {
        /* Get pointer to data within vertices buffer. */
        verticesBufPtr = HL_ADD_OFFC(mesh->vertices,
            (i * vtxFmt->vertexFormatSize) + (size_t)vtxElem->offset);

        switch (vtxElem->type & HL_VERTEX_ELEM_FORMAT_MASK)
        {
        case HL_VERTEX_ELEM_FORMAT_FLOAT:
        {
            const float* data = (const float*)verticesBufPtr;
            double x, y, z, w;

            /* Store XYZW values. */
            x = (double)data[0];
            y = (double)data[1];
            z = (double)data[2];
            w = (double)data[3];

            /* Check for NAN and infinity values. */
            if (HL_IS_NAN(x)) x = 0;
            if (HL_IS_NAN(y)) y = 0;
            if (HL_IS_NAN(z)) z = 0;
            if (HL_IS_NAN(w)) w = 1.0;

            /* Store data in buffer. */
            len = sprintf(&buf[bufStartIndex], "%f %f %f %f\n", x, y, z, w);
            if (len < 0) return HL_ERROR_UNKNOWN;

            /* Increase buffer length and global vertex count. */
            bufLen = (bufStartIndex + (size_t)len);
            ++(*globalVertexCount);
            break;
        }

        /* TODO: Support half floats. */

        default: return HL_ERROR_UNSUPPORTED;
        }

        /* Write buffer contents to file. */
        result = hlFileWrite(file, bufLen * sizeof(char), buf, 0);
        if (HL_FAILED(result)) return result;
    }

    return HL_RESULT_SUCCESS;
}

/* C89 doesn't support the printf z modifier, so fallback to l. */
#if (defined(__cplusplus) || defined(_MSC_VER)) ||\
    (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#define HL_IN_OBJ_FACES_PRINTF_FORMAT   "%zu"
#define HL_IN_OBJ_FACES_CAST_T          size_t
#else
#define HL_IN_OBJ_FACES_PRINTF_FORMAT   "%lu"
#define HL_IN_OBJ_FACES_CAST_T          unsigned long
#endif

typedef enum HlINOBJFaceTypes
{
    HL_IN_OBJ_FACE_TYPE_POSITION = 1,
    HL_IN_OBJ_FACE_TYPE_TEXCOORD = 2,
    HL_IN_OBJ_FACE_TYPE_NORMAL = 4
}
HlINOBJFaceTypes;

typedef struct HlINOBJGlobalCounts
{
    size_t vertexCount;
    size_t texcoordCount;
    size_t normalCount;
}
HlINOBJGlobalCounts;

static int hlINMeshStoreFaceIndexOBJ(char types, unsigned short f,
    const HlINOBJGlobalCounts* HL_RESTRICT globalOffset,
    char* HL_RESTRICT buf)
{
    /* f 1 */
    int idx, len;
    len = sprintf(buf, HL_IN_OBJ_FACES_PRINTF_FORMAT,
        (HL_IN_OBJ_FACES_CAST_T)f + globalOffset->vertexCount);

    if (len < 0) return len;

    idx = len;

    if (types & HL_IN_OBJ_FACE_TYPE_TEXCOORD)
    {
        /* f 1/1 */
        buf[idx++] = '/';
        len = sprintf(&buf[idx], HL_IN_OBJ_FACES_PRINTF_FORMAT,
            (HL_IN_OBJ_FACES_CAST_T)f + globalOffset->texcoordCount);

        if (len < 0) return len;
        
        idx += len;

        if (types & HL_IN_OBJ_FACE_TYPE_NORMAL)
        {
            /* f 1/1/1 */
            buf[idx++] = '/';
            len = sprintf(&buf[idx], HL_IN_OBJ_FACES_PRINTF_FORMAT,
                (HL_IN_OBJ_FACES_CAST_T)f + globalOffset->normalCount);

            if (len < 0) return len;

            idx += len;
        }
    }
    else if (types & HL_IN_OBJ_FACE_TYPE_NORMAL)
    {
        /* f 1//1 */
        buf[idx++] = '/';
        buf[idx++] = '/';

        len = sprintf(&buf[idx], HL_IN_OBJ_FACES_PRINTF_FORMAT,
            (HL_IN_OBJ_FACES_CAST_T)f + globalOffset->normalCount);

        if (len < 0) return len;

        idx += len;
    }

    return idx;
}

static int hlINMeshStoreFaceOBJ(char types, HlBool clockwise,
    unsigned short f1, unsigned short f2, unsigned short f3,
    const HlINOBJGlobalCounts* HL_RESTRICT globalOffset,
    char* HL_RESTRICT buf)
{
    int tmpLen, len = 2;

    /* Store face #1 */
    tmpLen = hlINMeshStoreFaceIndexOBJ(types,
        (clockwise) ? f1 : f3,
        globalOffset, &buf[len]);

    if (tmpLen < 0) return 0;

    len += tmpLen;
    buf[len++] = ' ';

    /* Store face #2 */
    tmpLen = hlINMeshStoreFaceIndexOBJ(types, f2,
        globalOffset, &buf[len]);

    if (tmpLen < 0) return 0;

    len += tmpLen;
    buf[len++] = ' ';

    /* Store face #3 */
    tmpLen = hlINMeshStoreFaceIndexOBJ(types,
        (clockwise) ? f3 : f1,
        globalOffset, &buf[len]);

    if (tmpLen < 0) return 0;

    len += tmpLen;
    buf[len++] = '\n';

    return len;
}

static HlResult hlINMeshWriteFacesOBJStrips(const HlMesh* HL_RESTRICT mesh,
    char types, const HlINOBJGlobalCounts* HL_RESTRICT globalOffset,
    char* HL_RESTRICT buf, HlFile* HL_RESTRICT file)
{
    size_t i, bufLen;
    HlResult result;
    unsigned short f1, f2, f3;
    HlBool reverse = HL_FALSE;

    /* Return early if there aren't enough faces to properly convert. */
    /* TODO: Should this be an error instead? */
    if (mesh->faceCount <= 2) return HL_RESULT_SUCCESS;

    /* Get initial f1 and f2 indices. */
    f1 = mesh->faces[0];
    f2 = mesh->faces[1];

    /* Store type in buffer. */
    buf[0] = 'f';
    buf[1] = ' ';

    /* Convert triangle strips to triangle indices and write. */
    for (i = 2; i < mesh->faceCount; ++i)
    {
        /* Get f3 index. */
        f3 = mesh->faces[i];

        if (f3 == (unsigned short)(-1))
        {
            f1 = mesh->faces[i + 1];
            f2 = mesh->faces[i + 2];

            reverse = HL_FALSE;
            i += 2;
        }
        else
        {
            /* Get f1 and f2 indices. */
            if (f1 != f2 && f2 != f3 && f3 != f1)
            {
                /* Store data in buffer. */
                if (reverse)
                {
                    bufLen = (size_t)hlINMeshStoreFaceOBJ(types,
                        mesh->clockwise, f1, f3, f2, globalOffset, buf);
                }
                else
                {
                    bufLen = (size_t)hlINMeshStoreFaceOBJ(types,
                        mesh->clockwise, f1, f2, f3, globalOffset, buf);
                }

                /* Multiply by sizeof(char). This should almost always be optimized-out. */
                bufLen *= sizeof(char);

                /* Write data to file. */
                result = hlFileWrite(file, bufLen, buf, 0);
                if (HL_FAILED(result)) return result;
            }

            f1 = f2;
            f2 = f3;
            reverse = !reverse;
        }
    }

    return HL_RESULT_SUCCESS;
}

static HlResult hlINMeshesWriteOBJ(const HlMesh* HL_RESTRICT meshes,
    size_t meshCount, const HlVertexFormat* HL_RESTRICT vertexFormats,
    HlBool writeMats, char* HL_RESTRICT buf, size_t* HL_RESTRICT globalIndex,
    HlINOBJGlobalCounts* HL_RESTRICT globalCounts, HlFile* HL_RESTRICT file)
{
    size_t i;
    HlResult result;

    for (i = 0; i < meshCount; ++i)
    {
        const HlINOBJGlobalCounts globalOffsets =
        {
            (globalCounts->vertexCount + 1),
            (globalCounts->texcoordCount + 1),
            (globalCounts->normalCount + 1)
        };

        const HlVertexFormat* vtxFmt = &vertexFormats[meshes[i].vertexFormatIndex];
        size_t i2;
        char types = 0;
        int len;

        /* Store group name data in buffer. */
        len = sprintf(buf, "g group_%lu\n",
            (unsigned long)(*globalIndex));

        if (len < 0) return HL_ERROR_UNKNOWN;

        /* Increment global index. */
        ++(*globalIndex);

        /* Write group name to file. */
        result = hlFileWrite(file, len, buf, 0);
        if (HL_FAILED(result)) return result;

        /* Store material name data in buffer if necessary. */
        if (writeMats)
        {
            const char* matName;
            matName = (meshes[i].matRef.refType == HL_REF_TYPE_NAME) ?
                meshes[i].matRef.data.name : ((meshes[i].matRef.data.ptr) ?
                meshes[i].matRef.data.ptr->name : NULL);

            if (matName)
            {
                len = sprintf(buf, "usemtl %s\n", matName);
                if (len < 0) return HL_ERROR_UNKNOWN;

                /* Write material name to file. */
                result = hlFileWrite(file, len, buf, 0);
                if (HL_FAILED(result)) return result;
            }
        }
        
        /* Write data. */
        for (i2 = 0; i2 < vtxFmt->vertexElementCount; ++i2)
        {
            const HlVertexElement* vtxElem = &vtxFmt->vertexElements[i2];
            const HlVertexElementType vtxElemDimension = (vtxElem->type &
                HL_VERTEX_ELEM_DIMENSION_MASK);

            /* Skip this vertex element if it's of an index that is greater than 0. */
            if ((vtxElem->type & HL_VERTEX_ELEM_INDEX_MASK) !=
                HL_VERTEX_ELEM_INDEX0) continue;

            /* Write vertex buffer contents. */
            switch (vtxElem->type & HL_VERTEX_ELEM_TYPE_MASK)
            {
            case HL_VERTEX_ELEM_TYPE_POSITION:
                /* Skip this vertex element if it's not at least 3 dimensions. */
                if (vtxElemDimension < HL_VERTEX_ELEM_DIMENSION_3D) continue;

                /* Store type in buffer. */
                buf[0] = 'v';
                buf[1] = ' ';

                /* Store data in buffer. */
                if (vtxElemDimension == HL_VERTEX_ELEM_DIMENSION_3D)
                {
                    result = hlINMeshWriteVerticesOBJVec3(&meshes[i], vtxFmt,
                        vtxElem, 2, buf, &globalCounts->vertexCount, file);
                }
                else
                {
                    result = hlINMeshWriteVerticesOBJVec4(&meshes[i], vtxFmt,
                        vtxElem, 2, buf, &globalCounts->vertexCount, file);
                }

                if (HL_FAILED(result)) return result;
                
                /* Store type. */
                types |= HL_IN_OBJ_FACE_TYPE_POSITION;
                break;

            case HL_VERTEX_ELEM_TYPE_NORMAL:
                /* Skip this vertex element if it's not at least 3 dimensions. */
                if (vtxElemDimension < HL_VERTEX_ELEM_DIMENSION_3D) continue;

                /* Store type in buffer. */
                buf[0] = 'v';
                buf[1] = 'n';
                buf[2] = ' ';

                /* Store data in buffer. */
                result = hlINMeshWriteVerticesOBJVec3(&meshes[i], vtxFmt,
                    vtxElem, 3, buf, &globalCounts->normalCount, file);

                if (HL_FAILED(result)) return result;

                /* Store type. */
                types |= HL_IN_OBJ_FACE_TYPE_NORMAL;
                break;

            case HL_VERTEX_ELEM_TYPE_TEXCOORD:
                /* Store type in buffer. */
                buf[0] = 'v';
                buf[1] = 't';
                buf[2] = ' ';

                /* Store data in buffer. */
                if (vtxElemDimension == HL_VERTEX_ELEM_DIMENSION_2D)
                {
                    result = hlINMeshWriteVerticesOBJVec2(&meshes[i], vtxFmt,
                        vtxElem, 3, buf, &globalCounts->texcoordCount, file);
                }
                else if (vtxElemDimension == HL_VERTEX_ELEM_DIMENSION_1D)
                {
                    result = hlINMeshWriteVerticesOBJVec1(&meshes[i], vtxFmt,
                        vtxElem, 3, buf, &globalCounts->texcoordCount, file);
                }
                else
                {
                    result = hlINMeshWriteVerticesOBJVec3(&meshes[i], vtxFmt,
                        vtxElem, 3, buf, &globalCounts->texcoordCount, file);
                }

                if (HL_FAILED(result)) return result;

                /* Store type. */
                types |= HL_IN_OBJ_FACE_TYPE_TEXCOORD;
                break;

            /* Skip unknown/unsupported vertex element types. */
            default: continue;
            }
        }

        /* Write faces. */
        if (types & HL_IN_OBJ_FACE_TYPE_POSITION)
        {
            /* TODO: Handle quads?? */
            /* TODO: Triangle lists. */
            result = hlINMeshWriteFacesOBJStrips(&meshes[i],
                types, &globalOffsets, buf, file);

            if (HL_FAILED(result)) return result;
        }
    }

    return HL_RESULT_SUCCESS;
}

static HlResult hlINModelWriteOBJ(const HlModel* HL_RESTRICT model,
    HlBool writeMats, HlFile* HL_RESTRICT file)
{
    HlResult result;

    /* Write object. */
    result = HL_FILE_WRITE_TEXT(file, "o ", NULL);
    if (HL_FAILED(result)) return result;

    /* Write object name. */
    if (model->name)
    {
        result = hlFileWrite(file, strlen(model->name) *
            sizeof(char), model->name, NULL);
    }
    else
    {
        result = HL_FILE_WRITE_TEXT(file, "default", NULL);
    }

    if (HL_FAILED(result)) return result;

    /* Write newline. */
    result = HL_FILE_WRITE_TEXT(file, "\n", NULL);
    if (HL_FAILED(result)) return result;

    /* Write meshes. */
    {
        HlINOBJGlobalCounts globalCounts = { 0, 0, 0 };
        char buf[255];
        size_t globalIndex = 1, i;

        for (i = 0; i < model->meshGroupCount; ++i)
        {
            /* Write solid slot. */
            result = hlINMeshesWriteOBJ(model->meshGroups[i].solid.meshes,
                model->meshGroups[i].solid.meshCount, model->vertexFormats,
                writeMats, buf, &globalIndex, &globalCounts, file);

            if (HL_FAILED(result)) return result;

            /* Write transparent slot. */
            result = hlINMeshesWriteOBJ(model->meshGroups[i].transparent.meshes,
                model->meshGroups[i].transparent.meshCount, model->vertexFormats,
                writeMats, buf, &globalIndex, &globalCounts, file);

            if (HL_FAILED(result)) return result;

            /* Write boolean slot. */
            result = hlINMeshesWriteOBJ(model->meshGroups[i].boolean.meshes,
                model->meshGroups[i].boolean.meshCount, model->vertexFormats,
                writeMats, buf, &globalIndex, &globalCounts, file);

            if (HL_FAILED(result)) return result;

            /* TODO: Write special slots. */
        }

        return HL_RESULT_SUCCESS;
    }
}

HlResult hlModelWriteOBJ(const HlModel** HL_RESTRICT models,
    size_t modelCount, const char* HL_RESTRICT mtlName,
    HlFile* HL_RESTRICT file)
{
    size_t i;
    HlResult result;

    /* Write "Generated by HedgeLib" comment. */
    result = HL_FILE_WRITE_TEXT(file, "# Generated by HedgeLib\n", NULL);
    if (HL_FAILED(result)) return result;
    
    /* Write mtl name if necessary. */
    if (mtlName)
    {
        /* Write mtllib. */
        result = HL_FILE_WRITE_TEXT(file, "mtllib ", NULL);
        if (HL_FAILED(result)) return result;

        /* Write name of mtl. */
        result = hlFileWrite(file, strlen(mtlName), mtlName, NULL);
        if (HL_FAILED(result)) return result;

        /* Write newline. */
        result = HL_FILE_WRITE_TEXT(file, "\n", NULL);
        if (HL_FAILED(result)) return result;
    }

    /* Write objects. */
    for (i = 0; i < modelCount; ++i)
    {
        result = hlINModelWriteOBJ(models[i], (mtlName != NULL), file);
        if (HL_FAILED(result)) return result;
    }

    return HL_RESULT_SUCCESS;
}

HlModel** hlModelImportOBJ(const HlNChar* filePath, size_t* modelCount)
{
    /* TODO */
    return NULL;
}

HlResult hlModelExportOBJ(const HlModel** HL_RESTRICT models,
    size_t modelCount, HlBool writeMTL, const HlNChar* HL_RESTRICT filePath)
{
    HlFile* file;
    const HlNChar* ext;
    HlResult result;

    /* Write OBJ. */
    {
        char buf[64];
        char* mtlName = NULL;

        if (writeMTL)
        {
            const HlNChar* fileName = hlPathGetName(filePath);
            size_t mtlNameLen, extPos = (size_t)((ext =
                hlPathGetExt(fileName)) - fileName);

#ifdef HL_IN_WIN32_UNICODE
            size_t extPosU8 = extPos;
            extPos = hlStrGetReqLenUTF16ToUTF8(fileName, extPosU8);
#endif

            mtlNameLen = (extPos + 5);

            /* Allocate buffer if necessary. */
            if (mtlNameLen > 64)
            {
                mtlName = HL_ALLOC_ARR(char, mtlNameLen);
                if (!mtlName) return HL_ERROR_OUT_OF_MEMORY;
            }
            else
            {
                mtlName = buf;
            }

            /* Copy file name without extension into buffer. */
#ifdef HL_IN_WIN32_UNICODE
            if (!hlStrConvUTF16ToUTF8NoAlloc(fileName, mtlName, extPosU8, 0))
            {
                if (mtlName != buf) hlFree(mtlName);
                return HL_ERROR_UNKNOWN;
            }
#else
            memcpy(mtlName, fileName, extPos * sizeof(char));
#endif

            /* Copy new extension into buffer. */
            strcpy(&mtlName[extPos], ".mtl");
        }

        /* Open OBJ file. */
        result = hlFileOpen(filePath, HL_FILE_MODE_WRITE, &file);
        if (HL_FAILED(result)) return result;

        /* Write OBJ to file. */
        result = hlModelWriteOBJ(models, modelCount, mtlName, file);
        if (HL_FAILED(result))
        {
            hlFileClose(file);
            return result;
        }

        /* Free MTL name buffer if necessary. */
        if (writeMTL && mtlName != buf)
            hlFree(mtlName);

        /* Close OBJ file and return result if failed, or if we're not writing an MTL. */
        result = hlFileClose(file);
        if (!writeMTL || HL_FAILED(result))
            return result;
    }

    /* Write MTL. */
    {
        HlNChar buf[255];
        HlNChar* mtlPath = buf;
        size_t extPos = (size_t)(ext - filePath),
            mtlPathLen = (extPos + 5);

        /* Allocate buffer if necessary. */
        if (mtlPathLen > 255)
        {
            mtlPath = HL_ALLOC_ARR(HlNChar, mtlPathLen);
            if (!mtlPath) return HL_ERROR_OUT_OF_MEMORY;
        }

        /* Copy file path without extension into buffer. */
        memcpy(mtlPath, filePath, extPos * sizeof(HlNChar));

        /* Copy new extension into buffer. */
        hlNStrCopy(HL_NTEXT(".mtl"), &mtlPath[extPos]);

        /* Open MTL file. */
        result = hlFileOpen(mtlPath, HL_FILE_MODE_WRITE, &file);
        if (HL_FAILED(result)) return result;

        /* Write MTL to file. */
        result = hlModelWriteMTL(models, modelCount, file);
        if (HL_FAILED(result))
        {
            if (mtlPath != buf) hlFree(mtlPath);
            hlFileClose(file);
            return result;
        }

        /* Free MTL name buffer if necessary. */
        if (mtlPath != buf) hlFree(mtlPath);

        /* Close MTL file and return result. */
        return hlFileClose(file);
    }
}
