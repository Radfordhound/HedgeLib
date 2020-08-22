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

static HlResult hlINMeshesWriteOBJ(const HlMesh* HL_RESTRICT meshes,
    size_t meshCount, const HlVertexFormat* HL_RESTRICT vertexFormats,
    char* HL_RESTRICT buf, size_t* HL_RESTRICT globalIndex,
    size_t* HL_RESTRICT globalVertexCount, HlFile* HL_RESTRICT file)
{
    /* TODO: Make this function not literal trash. */

    size_t i;
    HlResult result;

    for (i = 0; i < meshCount; ++i)
    {
        const HlVertexFormat* vtxFmt = &vertexFormats[meshes[i].vertexFormatIndex];
        size_t bufLen, globalVertexOffset, i2;
        int len;

        /* Get global vertex offset. */
        globalVertexOffset = (*globalVertexCount + 1);

        /* Store group name data in buffer. */
        len = sprintf(buf, "g group_%lu\n",
            (unsigned long)(*globalIndex)++);

        if (len < 0) return HL_ERROR_UNKNOWN;

        /* Get buffer length. */
        bufLen = (size_t)len;

        /* Write group name to file. */
        result = hlFileWrite(file, bufLen, buf, 0);
        if (HL_FAILED(result)) return result;

        /* Write data. */
        for (i2 = 0; i2 < vtxFmt->vertexElementCount; ++i2)
        {
            const HlVertexElement* vtxElem = &vtxFmt->vertexElements[i2];

            /* Skip this vertex element if it's of an index that is greater than 0. */
            if ((vtxElem->type & HL_VERTEX_ELEM_INDEX_MASK) !=
                HL_VERTEX_ELEM_INDEX0) continue;

            /* Write vertex buffer contents. */
            switch (vtxElem->type & HL_VERTEX_ELEM_TYPE_MASK)
            {
            case HL_VERTEX_ELEM_TYPE_POSITION:
            {
                const void* verticesBufPtr;
                size_t i3;

                /* Skip this vertex element if it's not at least 3 dimensions. */
                if ((vtxElem->type & HL_VERTEX_ELEM_DIMENSION_MASK) <
                    HL_VERTEX_ELEM_DIMENSION_3D)
                    continue;

                /* Store type in buffer. */
                buf[0] = 'v';
                buf[1] = ' ';
                bufLen = 2;

                /* Store data in buffer. */
                for (i3 = 0; i3 < meshes[i].vertexCount; ++i3)
                {
                    /* Get pointer to data within vertices buffer. */
                    verticesBufPtr = HL_ADD_OFFC(meshes[i].vertices,
                         (i3 * vtxFmt->vertexFormatSize) + (size_t)vtxElem->offset);

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

                        /* Store data in buffer. */
                        len = sprintf(&buf[2], "%f %f %f\n",
                            x, y, z);

                        if (len < 0) return HL_ERROR_UNKNOWN;

                        /* Increase buffer length and global vertex count. */
                        bufLen = (2 + (size_t)len);
                        ++(*globalVertexCount);
                        break;
                    }

                    /* TODO: Support other formats. */
                    default: continue;
                    }

                    /* Write buffer contents to file. */
                    result = hlFileWrite(file, bufLen * sizeof(char), buf, 0);
                    if (HL_FAILED(result)) return result;
                }
                break;
            }

            /* TODO */

            /*case HL_VERTEX_ELEM_TYPE_NORMAL:
                buf[0] = 'v';
                buf[1] = 'n';
                buf[2] = ' ';
                bufLen = 3;
                break;

            case HL_VERTEX_ELEM_TYPE_TEXCOORD:
                buf[0] = 'v';
                buf[1] = 't';
                buf[2] = ' ';
                bufLen = 3;
                break;*/

            /* Skip unknown/unsupported vertex element types. */
            default: continue;
            }
        }

        /* Write faces. */
        /* TODO: Handle quads?? */
        if (meshes[i].faceCount > 2)
        {
            unsigned short f1, f2, f3;
            HlBool reverse = HL_FALSE;

            /* Get initial f1 and f2 indices. */
            f1 = meshes[i].faces[0];
            f2 = meshes[i].faces[1];

            for (i2 = 2; i2 < meshes[i].faceCount; ++i2)
            {
                /* Get f3 index. */
                f3 = meshes[i].faces[i2];

                if (f3 == (unsigned short)(-1))
                {
                    f1 = meshes[i].faces[i2 + 1];
                    f2 = meshes[i].faces[i2 + 2];

                    reverse = HL_FALSE;
                    i2 += 2;
                }
                else
                {
                    /* Get f1 and f2 indices. */
                    if (f1 != f2 && f2 != f3 && f3 != f1)
                    {

/* C89 doesn't support the printf z modifier, so fallback to l. */
#if (defined(__cplusplus) || defined(_MSC_VER)) ||\
    (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#define HL_IN_OBJ_FACES_PRINTF_FORMAT   "f %zu %zu %zu\n"
#define HL_IN_OBJ_FACES_CAST_T          size_t
#else
#define HL_IN_OBJ_FACES_PRINTF_FORMAT   "f %lu %lu %lu\n"
#define HL_IN_OBJ_FACES_CAST_T          unsigned long
#endif

                        /* Store data in buffer. */
                        /* TODO: Write normal/texcoord indices. */
                        if (reverse)
                        {
                            len = sprintf(buf, HL_IN_OBJ_FACES_PRINTF_FORMAT,
                                (HL_IN_OBJ_FACES_CAST_T)(f1 + globalVertexOffset),
                                (HL_IN_OBJ_FACES_CAST_T)(f3 + globalVertexOffset),
                                (HL_IN_OBJ_FACES_CAST_T)(f2 + globalVertexOffset));
                        }
                        else
                        {
                            len = sprintf(buf, HL_IN_OBJ_FACES_PRINTF_FORMAT,
                                (HL_IN_OBJ_FACES_CAST_T)(f1 + globalVertexOffset),
                                (HL_IN_OBJ_FACES_CAST_T)(f2 + globalVertexOffset),
                                (HL_IN_OBJ_FACES_CAST_T)(f3 + globalVertexOffset));
                        }

                        if (len < 0) return HL_ERROR_UNKNOWN;

                        /* Write data to file. */
                        result = hlFileWrite(file, (size_t)len * sizeof(char), buf, 0);
                        if (HL_FAILED(result)) return result;
                    }

                    f1 = f2;
                    f2 = f3;
                    reverse = !reverse;
                }
            }
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
        char buf[255];
        size_t globalIndex = 0, globalVertexCount = 0, i;

        for (i = 0; i < model->meshGroupCount; ++i)
        {
            /* Write solid slot. */
            result = hlINMeshesWriteOBJ(model->meshGroups[i].solid.meshes,
                model->meshGroups[i].solid.meshCount, model->vertexFormats,
                buf, &globalIndex, &globalVertexCount, file);

            if (HL_FAILED(result)) return result;

            /* Write transparent slot. */
            result = hlINMeshesWriteOBJ(model->meshGroups[i].transparent.meshes,
                model->meshGroups[i].transparent.meshCount, model->vertexFormats,
                buf, &globalIndex, &globalVertexCount, file);

            if (HL_FAILED(result)) return result;

            /* Write boolean slot. */
            result = hlINMeshesWriteOBJ(model->meshGroups[i].boolean.meshes,
                model->meshGroups[i].boolean.meshCount, model->vertexFormats,
                buf, &globalIndex, &globalVertexCount, file);

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
