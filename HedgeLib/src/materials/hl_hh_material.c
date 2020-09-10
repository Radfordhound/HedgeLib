#include "hedgelib/materials/hl_hh_material.h"
#include "hedgelib/textures/hl_hh_texture.h"
#include "hedgelib/io/hl_hh.h"
#include "hedgelib/hl_blob.h"
#include "hedgelib/hl_math.h"
#include "hedgelib/hl_endian.h"
#include "hedgelib/hl_text.h"
#include "../hl_in_assert.h"

void hlHHMaterialParameterSwap(HlHHMaterialParameter* param, HlBool swapOffsets)
{
    if (!swapOffsets) return;

    hlSwapU32P(&param->nameOffset);
    hlSwapU32P(&param->valuesOffset);
}

void hlHHMaterialV1Swap(HlHHMaterialV1* mat, HlBool swapOffsets)
{
    if (!swapOffsets) return;

    hlSwapU32P(&mat->shaderNameOffset);
    hlSwapU32P(&mat->subShaderNameOffset);
    hlSwapU32P(&mat->texsetNameOffset);
    hlSwapU32P(&mat->vec4ParamsOffset);
    hlSwapU32P(&mat->unknown1sOffset);
    hlSwapU32P(&mat->u32ParamsOffset);
}

void hlHHMaterialV3Swap(HlHHMaterialV3* mat, HlBool swapOffsets)
{
    if (!swapOffsets) return;

    hlSwapU32P(&mat->shaderNameOffset);
    hlSwapU32P(&mat->subShaderNameOffset);
    hlSwapU32P(&mat->hhTextureNamesOffset);
    hlSwapU32P(&mat->texturesOffset);
    hlSwapU32P(&mat->vec4ParamsOffset);
    hlSwapU32P(&mat->unknown1sOffset);
    hlSwapU32P(&mat->u32ParamsOffset);
}

#ifndef HL_IS_BIG_ENDIAN
static void hlINHHMaterialParametersFixVec4(HL_OFF32(
    HL_OFF32(HlHHMaterialParameter))* paramOffsets,
    HlU8 paramCount)
{
    HL_OFF32(HlHHMaterialParameter)* params = (HL_OFF32(
        HlHHMaterialParameter)*)hlOff32Get(paramOffsets);

    HlU8 i;

    for (i = 0; i < paramCount; ++i)
    {
        HlHHMaterialParameter* param = (HlHHMaterialParameter*)hlOff32Get(&params[i]);
        HlVector4* values = (HlVector4*)hlOff32Get(&param->valuesOffset);
        HlU8 i2;

        for (i2 = 0; i2 < param->valueCount; ++i2)
        {
            hlVector4Swap(&values[i2]);
        }
    }
}

static void hlINHHMaterialParametersFixU32(HL_OFF32(
    HL_OFF32(HlHHMaterialParameter))* paramOffsets,
    HlU8 paramCount)
{
    HL_OFF32(HlHHMaterialParameter)* params = (HL_OFF32(
        HlHHMaterialParameter)*)hlOff32Get(paramOffsets);

    HlU8 i;

    for (i = 0; i < paramCount; ++i)
    {
        HlHHMaterialParameter* param = (HlHHMaterialParameter*)hlOff32Get(&params[i]);
        HlU32* values = (HlU32*)hlOff32Get(&param->valuesOffset);
        HlU8 i2;

        for (i2 = 0; i2 < param->valueCount; ++i2)
        {
            /*
               Crash if we stumble upon a u32 parameter that isn't 0 so we can see how these work.

               If you encounter this crash, *please* create an issue stating such and including
               the name of the .material you were trying to load on the HedgeLib repository so
               we can figure out what these parameters truly are and fix the crash!
            */
            HL_ASSERT(values[i2] == 0);
            hlSwapU32P(&values[i2]);
        }
    }
}
#endif

void hlHHMaterialV1Fix(HlHHMaterialV1* mat)
{
    /* TODO: Have this just be a macro if HL_IS_BIG_ENDIAN is set? */

#ifndef HL_IS_BIG_ENDIAN
    /*
       Crash if we stumble upon a V1 material that has reserved1 set since this shouldn't happen.

       If you encounter this crash, *please* create an issue stating such and including
       the name of the .material you were trying to load on the HedgeLib repository so
       we can figure out what's going on and fix the crash!
    */
    HL_ASSERT(mat->reserved1 == 0);

    /* Fix vector4 parameters. */
    hlINHHMaterialParametersFixVec4(&mat->vec4ParamsOffset, mat->vec4ParamCount);

    /*
       Crash if we stumble upon a material that has unknown1s so we can see how these work.

       If you encounter this crash, *please* create an issue stating such and including
       the name of the .material you were trying to load on the HedgeLib repository so
       we can figure out what unknown1s are and fix the crash!
    */
    HL_ASSERT(mat->unknown1Count == 0 && mat->unknown1sOffset == 0);

    /* Fix u32 parameters. */
    hlINHHMaterialParametersFixU32(&mat->u32ParamsOffset, mat->u32ParamCount);
#endif
}

void hlHHMaterialV3Fix(HlHHMaterialV3* mat)
{
#ifndef HL_IS_BIG_ENDIAN
    HL_OFF32(HlHHTexture)* textures;
    HlU8 i;

    /* Fix vector4 parameters. */
    hlINHHMaterialParametersFixVec4(&mat->vec4ParamsOffset, mat->vec4ParamCount);

    /*
       Crash if we stumble upon a material that has unknown1s so we can see how these work.

       If you encounter this crash, *please* create an issue stating such and including
       the name of the .material you were trying to load on the HedgeLib repository so
       we can figure out what unknown1s are and fix the crash!
    */
    HL_ASSERT(mat->unknown1Count == 0 && mat->unknown1sOffset == 0);

    /* Fix u32 parameters. */
    hlINHHMaterialParametersFixU32(&mat->u32ParamsOffset, mat->u32ParamCount);

    /* Swap HHTextures if necessary. */
    textures = (HL_OFF32(HlHHTexture)*)hlOff32Get(&mat->texturesOffset);
    for (i = 0; i < mat->textureCount; ++i)
    {
        HlHHTexture* tex = (HlHHTexture*)hlOff32Get(&textures[i]);
        hlHHTextureSwap(tex, HL_FALSE);
    }
#endif
}

static size_t hlINHHMaterialParametersGetReqSize(const HL_OFF32(
    HL_OFF32(HlHHMaterialParameter))* paramOffsets,
    HlU8 paramCount, size_t valueSize)
{
    const HL_OFF32(HlHHMaterialParameter)* params = (const
        HL_OFF32(HlHHMaterialParameter)*)hlOff32Get(paramOffsets);

    size_t reqBufSize = 0;
    HlU8 i;

    for (i = 0; i < paramCount; ++i)
    {
        const HlHHMaterialParameter* param =
            (const HlHHMaterialParameter*)hlOff32Get(&params[i]);

        const char* name = (const char*)hlOff32Get(&param->nameOffset);

        /* Account for parameter. */
        reqBufSize += sizeof(HlMaterialParam);

        /* Account for name and null terminator. */
        reqBufSize += (strlen(name) + 1);

        /* Account for values. */
        reqBufSize += (sizeof(HlVector4) * param->valueCount);
    }
    
    return reqBufSize;
}

static size_t hlINHHMaterialV1GetReqSize(const HlHHMaterialV1* mat)
{
    size_t reqBufSize = sizeof(HlMaterial);

    /* Account for shader name. */
    reqBufSize += (strlen((const char*)hlOff32Get(&mat->shaderNameOffset)) + 1);

    /* Ensure sub-shader name is the same as shader name. */
    HL_ASSERT(!strcmp(
        (const char*)hlOff32Get(&mat->shaderNameOffset),
        (const char*)hlOff32Get(&mat->subShaderNameOffset)));

    /* Account for texset name. */
    reqBufSize += (strlen((const char*)hlOff32Get(&mat->texsetNameOffset)) + 1);

    /* Account for vector4 parameters. */
    reqBufSize += hlINHHMaterialParametersGetReqSize(
        &mat->vec4ParamsOffset, mat->vec4ParamCount,
        sizeof(HlVector4));

    /* Account for u32 parameters. */
    reqBufSize += hlINHHMaterialParametersGetReqSize(
        &mat->u32ParamsOffset, mat->u32ParamCount,
        sizeof(HlU32));

    return reqBufSize;
}

static size_t hlINHHMaterialV3GetReqSize(const HlHHMaterialV3* mat)
{
    size_t reqBufSize = sizeof(HlMaterial);
    const HL_OFF32_STR* hhTextureNames = (const HL_OFF32_STR*)
        hlOff32Get(&mat->hhTextureNamesOffset);

    const HL_OFF32(HlHHTexture)* hhTextureEntries =
        (const HL_OFF32(HlHHTexture)*)hlOff32Get(&mat->texturesOffset);

    HlU8 i;
    
    /* Account for shader name. */
    reqBufSize += (strlen((const char*)hlOff32Get(&mat->shaderNameOffset)) + 1);

    /* Ensure sub-shader name is the same as shader name. */
    HL_ASSERT(!strcmp(
        (const char*)hlOff32Get(&mat->shaderNameOffset),
        (const char*)hlOff32Get(&mat->subShaderNameOffset)));

    /* Account for texset. */
    reqBufSize += sizeof(HlTexset);

    /* Account for texture entries. */
    for (i = 0; i < mat->textureCount; ++i)
    {
        const char* texEntryName = (const char*)
            hlOff32Get(&hhTextureNames[i]);

        const HlHHTexture* texEntry = (const HlHHTexture*)
            hlOff32Get(&hhTextureEntries[i]);

        /* Account for texture entry. */
        reqBufSize += sizeof(HlTextureEntry);

        /* Account for texture entry name. */
        reqBufSize += (strlen(texEntryName) + 1);

        /* Account for texture file name. */
        reqBufSize += (strlen((const char*)hlOff32Get(
            &texEntry->fileNameOffset)) + 1);

        /* Account for texture type. */
        reqBufSize += (strlen((const char*)hlOff32Get(
            &texEntry->typeOffset)) + 1);
    }

    /* Account for vector4 parameters. */
    reqBufSize += hlINHHMaterialParametersGetReqSize(
        &mat->vec4ParamsOffset, mat->vec4ParamCount,
        sizeof(HlVector4));

    /* Account for u32 parameters. */
    reqBufSize += hlINHHMaterialParametersGetReqSize(
        &mat->u32ParamsOffset, mat->u32ParamCount,
        sizeof(HlU32));

    return reqBufSize;
}

static void hlINHHMaterialParametersParse(const HL_OFF32(HL_OFF32(
    HlHHMaterialParameter))* HL_RESTRICT paramOffsets,
    HlU8 paramCount, const HlMaterialParamFormat format,
    const size_t valueSize, HlMaterialParam* HL_RESTRICT hlParams,
    void** HL_RESTRICT curDataPtr)
{
    const HL_OFF32(HlHHMaterialParameter)* params = (const
        HL_OFF32(HlHHMaterialParameter)*)hlOff32Get(paramOffsets);

    HlU8 i;

    for (i = 0; i < paramCount; ++i)
    {
        const HlHHMaterialParameter* hhParam = (const
            HlHHMaterialParameter*)hlOff32Get(&params[i]);

        char* curStrPtr = (char*)(*curDataPtr);

        /* Set name pointer and format. */
        hlParams->name = curStrPtr;
        hlParams->format = format;

        /* Copy name. */
        curStrPtr += (hlStrCopyAndLen((const char*)hlOff32Get(
            &hhParam->nameOffset), curStrPtr) + 1);

        /* Copy values. */
        {
            const size_t valuesTotalSize = (valueSize *
                hhParam->valueCount);

            hlParams->valueCount = (unsigned int)hhParam->valueCount;
            hlParams->values = curStrPtr;

            memcpy(curStrPtr, hlOff32Get(&hhParam->valuesOffset),
                valuesTotalSize);

            /* Increase curDataPtr and hlParams pointer. */
            ++hlParams;
            *curDataPtr = HL_ADD_OFF(curStrPtr, valuesTotalSize);
        }
    }
}

static void hlINHHTexturesParse(
    const HL_OFF32(HL_OFF32_STR)* HL_RESTRICT textureNamesOffset,
    const HL_OFF32(HL_OFF32(HlHHTexture))* HL_RESTRICT textureOffsets,
    HlU8 textureCount, void** HL_RESTRICT curDataPtr)
{
    const HL_OFF32_STR* textureNames = (const HL_OFF32_STR*)
        hlOff32Get(textureNamesOffset);

    const HL_OFF32(HlHHTexture)* textures = (const
        HL_OFF32(HlHHTexture)*)hlOff32Get(textureOffsets);

    HlTextureEntry* hlTexEntries = (HlTextureEntry*)HL_ADD_OFF(
        *curDataPtr, sizeof(HlTexset));

    char* curStrPtr = (char*)&hlTexEntries[textureCount];
    HlU8 i;

    /* Setup texset. */
    {
        HlTexset* hlTexset = (HlTexset*)(*curDataPtr);
        hlTexset->textureCount = (size_t)textureCount;
        hlTexset->textures = hlTexEntries;
    }

    /* Setup texture entries. */
    for (i = 0; i < textureCount; ++i)
    {
        const HlHHTexture* hhTexEntry = (const HlHHTexture*)hlOff32Get(&textures[i]);
        HlTextureEntry* hlTexEntry = &hlTexEntries[i];

        /* Set entryName pointer, copy entry name, and increase curStrPtr. */
        hlTexEntry->entryName = curStrPtr;
        curStrPtr += (hlStrCopyAndLen((const char*)hlOff32Get(
            &textureNames[i]), curStrPtr) + 1);

        /* Set texFileName pointer, copy file name, and increase curStrPtr. */
        hlTexEntry->texFileName = curStrPtr;
        curStrPtr += (hlStrCopyAndLen((const char*)hlOff32Get(
            &hhTexEntry->fileNameOffset), curStrPtr) + 1);

        /* Set type pointer, copy type, and increase curStrPtr. */
        hlTexEntry->type = curStrPtr;
        curStrPtr += (hlStrCopyAndLen((const char*)hlOff32Get(
            &hhTexEntry->typeOffset), curStrPtr) + 1);
    }

    /* Increase curDataPtr. */
    *curDataPtr = curStrPtr;
}

HlResult hlHHMaterialV1Parse(const HlHHMaterialV1* HL_RESTRICT hhMat,
    const char* HL_RESTRICT name, HlMaterial** HL_RESTRICT hlMat)
{
    HlMaterial* hlMatBuf;

    /* Allocate HlMaterial buffer. */
    {
        size_t reqBufSize = hlINHHMaterialV1GetReqSize(hhMat);
        if (name) reqBufSize += (strlen(name) + 1);

        hlMatBuf = (HlMaterial*)hlAlloc(reqBufSize);
        if (!hlMatBuf) return HL_ERROR_OUT_OF_MEMORY;
    }

    /* Setup HlMaterial, set pointer, and return. */
    {
        void* curDataPtr = (hlMatBuf + 1);
        char* curStrPtr = (char*)curDataPtr;

        /* Set material name pointer. */
        if (name)
        {
            /* Copy material name and set pointer. */
            hlMatBuf->name = curStrPtr;
            curStrPtr += (hlStrCopyAndLen(name, curStrPtr) + 1);
        }
        else
        {
            /* We don't know what the material's name is; just set name to NULL. */
            hlMatBuf->name = NULL;
        }

        /* Copy shader name and set pointer. */
        hlMatBuf->shaderName = curStrPtr;
        curStrPtr += (hlStrCopyAndLen((const char*)hlOff32Get(
            &hhMat->shaderNameOffset), curStrPtr) + 1);

        curDataPtr = curStrPtr;

        /* Set defaults for technique name and mipMapBias. */
        hlMatBuf->techniqueName = NULL;
        hlMatBuf->mipMapBias = 0;

        /* Setup parameters. */
        hlMatBuf->params = (HlMaterialParam*)curDataPtr;
        hlMatBuf->paramCount = (size_t)hhMat->vec4ParamCount + hhMat->u32ParamCount;
        curDataPtr = &hlMatBuf->params[hlMatBuf->paramCount];

        hlINHHMaterialParametersParse(&hhMat->vec4ParamsOffset,
            hhMat->vec4ParamCount, HL_MATERIAL_PARAM_FORMAT_FLOAT4,
            sizeof(HlVector4), hlMatBuf->params, &curDataPtr);

        hlINHHMaterialParametersParse(&hhMat->u32ParamsOffset,
            hhMat->u32ParamCount, HL_MATERIAL_PARAM_FORMAT_U32,
            sizeof(HlU32), &hlMatBuf->params[hhMat->vec4ParamCount],
            &curDataPtr);

        /* Setup texset. */
        hlMatBuf->texset.refType = HL_REF_TYPE_NAME;
        hlMatBuf->texset.data.name = (const char*)curDataPtr;

        /* Copy texset name. */
        strcpy(curDataPtr, (const char*)hlOff32Get(
            &hhMat->texsetNameOffset));

        /* Set pointer and return success. */
        *hlMat = hlMatBuf;
        return HL_RESULT_SUCCESS;
    }
}

HlResult hlHHMaterialV3Parse(const HlHHMaterialV3* HL_RESTRICT hhMat,
    const char* HL_RESTRICT name, HlMaterial** HL_RESTRICT hlMat)
{
    HlMaterial* hlMatBuf;

    /* Allocate HlMaterial buffer. */
    {
        size_t reqBufSize = hlINHHMaterialV3GetReqSize(hhMat);
        if (name) reqBufSize += (strlen(name) + 1);

        hlMatBuf = (HlMaterial*)hlAlloc(reqBufSize);
        if (!hlMatBuf) return HL_ERROR_OUT_OF_MEMORY;
    }

    /* Setup HlMaterial, set pointer, and return. */
    {
        void* curDataPtr = (hlMatBuf + 1);
        char* curStrPtr = (char*)curDataPtr;

        /* Set material name pointer. */
        if (name)
        {
            /* Copy material name and set pointer. */
            hlMatBuf->name = curStrPtr;
            curStrPtr += (hlStrCopyAndLen(name, curStrPtr) + 1);
        }
        else
        {
            /* We don't know what the material's name is; just set name to NULL. */
            hlMatBuf->name = NULL;
        }

        /* Copy shader name and set pointer. */
        hlMatBuf->shaderName = curStrPtr;
        curStrPtr += (hlStrCopyAndLen((const char*)hlOff32Get(
            &hhMat->shaderNameOffset), curStrPtr) + 1);

        curDataPtr = curStrPtr;

        /* Set defaults for technique name and mipMapBias. */
        hlMatBuf->techniqueName = NULL;
        hlMatBuf->mipMapBias = 0;

        /* Setup parameters. */
        hlMatBuf->params = (HlMaterialParam*)curDataPtr;
        hlMatBuf->paramCount = (size_t)hhMat->vec4ParamCount + hhMat->u32ParamCount;
        curDataPtr = &hlMatBuf->params[hlMatBuf->paramCount];

        hlINHHMaterialParametersParse(&hhMat->vec4ParamsOffset,
            hhMat->vec4ParamCount, HL_MATERIAL_PARAM_FORMAT_FLOAT4,
            sizeof(HlVector4), hlMatBuf->params, &curDataPtr);

        hlINHHMaterialParametersParse(&hhMat->u32ParamsOffset,
            hhMat->u32ParamCount, HL_MATERIAL_PARAM_FORMAT_U32,
            sizeof(HlU32), &hlMatBuf->params[hhMat->vec4ParamCount],
            &curDataPtr);

        /* Setup texset. */
        hlMatBuf->texset.refType = HL_REF_TYPE_PTR;
        hlMatBuf->texset.data.ptr = (HlTexset*)curDataPtr;

        hlINHHTexturesParse(&hhMat->hhTextureNamesOffset,
            &hhMat->texturesOffset, hhMat->textureCount, &curDataPtr);

        /* Set pointer and return success. */
        *hlMat = hlMatBuf;
        return HL_RESULT_SUCCESS;
    }
}

HlResult hlHHMaterialRead(HlBlob* HL_RESTRICT blob,
    const char* HL_RESTRICT name, HlMaterial** HL_RESTRICT hlMat)
{
    void* hhMat;
    HlU32 version;

    /* Fix HH general data. */
    hlHHFix(blob);

    /* Get HH material pointer and version number. */
    hhMat = (void*)hlHHGetData(blob, &version);
    if (!hhMat) return HL_ERROR_INVALID_DATA;

    /* Fix and parse HH material data based on version number. */
    switch (version)
    {
    case 1:
        /* Fix and parse HH material V1 data. */
        hlHHMaterialV1Fix((HlHHMaterialV1*)hhMat);
        return hlHHMaterialV1Parse((const HlHHMaterialV1*)hhMat,
            name, hlMat);

    case 3:
        /* Fix and parse HH material V3 data. */
        hlHHMaterialV3Fix((HlHHMaterialV3*)hhMat);
        return hlHHMaterialV3Parse((const HlHHMaterialV3*)hhMat,
            name, hlMat);

    /* Unknown or unsupported HH material version. */
    default: return HL_ERROR_UNSUPPORTED;
    }
}
