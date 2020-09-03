#include "hedgelib/hl_math.h"
#include "hedgelib/models/hl_model.h"
#include "hedgelib/io/hl_file.h"
#include <string.h>
#include <stdio.h>

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

static int hlINMeshStoreFaceOBJ(char types,
    unsigned short f1, unsigned short f2, unsigned short f3,
    const HlINOBJGlobalCounts* HL_RESTRICT globalOffset,
    char* HL_RESTRICT buf)
{
    int tmpLen, len = 2;

    /* Store face #1 */
    tmpLen = hlINMeshStoreFaceIndexOBJ(types, f1,
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
    tmpLen = hlINMeshStoreFaceIndexOBJ(types, f3,
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
                        f1, f3, f2, globalOffset, buf);
                }
                else
                {
                    bufLen = (size_t)hlINMeshStoreFaceOBJ(types,
                        f1, f2, f3, globalOffset, buf);
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
    char* HL_RESTRICT buf, size_t* HL_RESTRICT globalIndex,
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
        result = hlFileWrite(file, (size_t)len, buf, 0);
        if (HL_FAILED(result)) return result;

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
    HlFile* HL_RESTRICT file)
{
    HlResult result;

    /* Write object. */
    result = HL_FILE_WRITE_TEXT(file, "o ", 0);
    if (HL_FAILED(result)) return result;

    /* Write object name. */
    if (model->name)
    {
        result = hlFileWrite(file, strlen(model->name) *
            sizeof(char), model->name, 0);
    }
    else
    {
        result = HL_FILE_WRITE_TEXT(file, "default", 0);
    }

    if (HL_FAILED(result)) return result;

    /* Write newline. */
    result = HL_FILE_WRITE_TEXT(file, "\n", 0);
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
                buf, &globalIndex, &globalCounts, file);

            if (HL_FAILED(result)) return result;

            /* Write transparent slot. */
            result = hlINMeshesWriteOBJ(model->meshGroups[i].transparent.meshes,
                model->meshGroups[i].transparent.meshCount, model->vertexFormats,
                buf, &globalIndex, &globalCounts, file);

            if (HL_FAILED(result)) return result;

            /* Write boolean slot. */
            result = hlINMeshesWriteOBJ(model->meshGroups[i].boolean.meshes,
                model->meshGroups[i].boolean.meshCount, model->vertexFormats,
                buf, &globalIndex, &globalCounts, file);

            if (HL_FAILED(result)) return result;

            /* TODO: Write special slots. */
        }

        return HL_RESULT_SUCCESS;
    }
}

HlResult hlModelWriteOBJ(const HlModel** HL_RESTRICT models,
    size_t modelCount, HlFile* HL_RESTRICT file)
{
    size_t i;
    HlResult result;

    /* Write "Generated by HedgeLib" comment. */
    result = HL_FILE_WRITE_TEXT(file, "# Generated by HedgeLib\n", 0);
    if (HL_FAILED(result)) return result;

    /* Write objects. */
    for (i = 0; i < modelCount; ++i)
    {
        result = hlINModelWriteOBJ(models[i], file);
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
    size_t modelCount, const HlNChar* HL_RESTRICT filePath)
{
    HlFile* file;
    HlResult result;

    /* Open file. */
    result = hlFileOpen(filePath, HL_FILE_MODE_WRITE, &file);
    if (HL_FAILED(result)) return result;

    /* Write OBJ to file, close it, and return. */
    result = hlModelWriteOBJ(models, modelCount, file);
    if (HL_FAILED(result))
    {
        hlFileClose(file);
        return result;
    }

    return hlFileClose(file);
}
