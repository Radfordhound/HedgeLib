#include "hr_in_d3d11.h"
#include "dxc_Default_vs.h"
#include "dxc_Default_ps.h"
#include "hedgerender/hr_shader.h"
#include "hedgelib/hl_memory.h"
#include <d3dcompiler.h>

static const HrShaderCode HrIND3D11DefaultVSCode =
{
    sizeof(dxc_Default_vs), dxc_Default_vs
};

const HrShaderCode* hrShaderGetDefaultVSCode(void)
{
    return &HrIND3D11DefaultVSCode;
}

static const HrShaderCode HrIND3D11DefaultPSCode =
{
    sizeof(dxc_Default_ps), dxc_Default_ps
};

const HrShaderCode* hrShaderGetDefaultPSCode(void)
{
    return &HrIND3D11DefaultPSCode;
}

HlResult hrShaderProgramCreate(HrInstance* HL_RESTRICT instance,
    const HrShaderCode* HL_RESTRICT vsCode, const HrShaderCode* HL_RESTRICT psCode,
    const HrShaderCode* HL_RESTRICT dsCode, const HrShaderCode* HL_RESTRICT hsCode,
    const HrShaderCode* HL_RESTRICT gsCode, HrShaderProgram* HL_RESTRICT * HL_RESTRICT shaderProgram)
{
    HrShaderProgram* shaderProgramBuf;
    HRESULT hresult;

    /* Allocate buffer for shader program. */
    shaderProgramBuf = HL_ALLOC_OBJ(HrShaderProgram);
    if (!shaderProgramBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Set default value for shaderTypesMask. */
    shaderProgramBuf->shaderTypesMask = 0;

    /* Setup vertex shader if necessary. */
    if (vsCode)
    {
        /* Create vertex shader. */
        hresult = instance->device->CreateVertexShader(vsCode->code,
            vsCode->codeLen, NULL, &shaderProgramBuf->vs);

        if (FAILED(hresult)) goto failed_vs;

        /* Set flags. */
        shaderProgramBuf->shaderTypesMask |= HR_SHADER_TYPE_VERTEX;
    }
    else
    {
        shaderProgramBuf->vs = NULL;
    }

    /* Setup pixel shader if necessary. */
    if (psCode)
    {
        /* Create pixel shader. */
        hresult = instance->device->CreatePixelShader(psCode->code,
            psCode->codeLen, NULL, &shaderProgramBuf->ps);

        if (FAILED(hresult)) goto failed_ps;

        /* Set flags. */
        shaderProgramBuf->shaderTypesMask |= HR_SHADER_TYPE_PIXEL;
    }
    else
    {
        shaderProgramBuf->ps = NULL;
    }

    /* Setup domain shader if necessary. */
    if (dsCode)
    {
        /* Create domain shader. */
        hresult = instance->device->CreateDomainShader(dsCode->code,
            dsCode->codeLen, NULL, &shaderProgramBuf->ds);

        if (FAILED(hresult)) goto failed_ds;

        /* Set flags. */
        shaderProgramBuf->shaderTypesMask |= HR_SHADER_TYPE_DOMAIN;
    }
    else
    {
        shaderProgramBuf->ds = NULL;
    }

    /* Setup hull shader if necessary. */
    if (hsCode)
    {
        /* Create hull shader. */
        hresult = instance->device->CreateHullShader(hsCode->code,
            hsCode->codeLen, NULL, &shaderProgramBuf->hs);

        if (FAILED(hresult)) goto failed_hs;

        /* Set flags. */
        shaderProgramBuf->shaderTypesMask |= HR_SHADER_TYPE_HULL;
    }
    else
    {
        shaderProgramBuf->hs = NULL;
    }

    /* Setup geometry shader if necessary. */
    if (gsCode)
    {
        /* Create geometry shader. */
        hresult = instance->device->CreateGeometryShader(gsCode->code,
            gsCode->codeLen, NULL, &shaderProgramBuf->gs);

        if (FAILED(hresult)) goto failed_gs;

        /* Set flags. */
        shaderProgramBuf->shaderTypesMask |= HR_SHADER_TYPE_GEOMETRY;
    }
    else
    {
        shaderProgramBuf->gs = NULL;
    }

    /* Set pointer and return success. */
    *shaderProgram = shaderProgramBuf;
    return HL_RESULT_SUCCESS;

failed_gs:
    shaderProgramBuf->hs->Release();

failed_hs:
    shaderProgramBuf->ds->Release();

failed_ds:
    shaderProgramBuf->ps->Release();

failed_ps:
    shaderProgramBuf->vs->Release();

failed_vs:
    hlFree(shaderProgramBuf);
    return hlINWin32GetResult(hresult);
}

void hrShaderProgramDestroy(HrShaderProgram* shaderProgram)
{
    if (!shaderProgram) return;

    if (shaderProgram->shaderTypesMask & HR_SHADER_TYPE_VERTEX)
        shaderProgram->vs->Release();

    if (shaderProgram->shaderTypesMask & HR_SHADER_TYPE_PIXEL)
        shaderProgram->ps->Release();

    if (shaderProgram->shaderTypesMask & HR_SHADER_TYPE_DOMAIN)
        shaderProgram->ds->Release();

    if (shaderProgram->shaderTypesMask & HR_SHADER_TYPE_HULL)
        shaderProgram->hs->Release();

    if (shaderProgram->shaderTypesMask & HR_SHADER_TYPE_GEOMETRY)
        shaderProgram->gs->Release();

    hlFree(shaderProgram);
}

static const char* hrIND3D11ShaderGetTarget(HrShaderType type)
{
    /* TODO: Let user specify feature level somehow. */
    switch (type)
    {
    default:                        return NULL;
    case HR_SHADER_TYPE_VERTEX:     return "vs_5_0";
    case HR_SHADER_TYPE_PIXEL:      return "ps_5_0";
    case HR_SHADER_TYPE_DOMAIN:     return "ds_5_0";
    case HR_SHADER_TYPE_HULL:       return "hs_5_0";
    case HR_SHADER_TYPE_GEOMETRY:   return "gs_5_0";
    }
}

HlResult hrShaderCompile(const void* HL_RESTRICT text,
    size_t textSize, HrShaderType type,
    HrShaderCode* HL_RESTRICT * HL_RESTRICT bytecode)
{
    ID3DBlob* shaderBlobHandle = NULL;

    /* Compile shader and handle any errors as necessary. */
    {
        ID3DBlob* errorBlobHandle = NULL;
        HRESULT hresult;

        /* TODO: Let user specify flags. */
        /* TODO: Let user specify entry point. */

        /* Compile shader. */
        hresult = D3DCompile2(text, textSize, NULL,
            NULL, NULL, "main", hrIND3D11ShaderGetTarget(type),
            0, 0, 0, NULL, 0, &shaderBlobHandle, &errorBlobHandle);

        /* Handle any errors as necessary. */
        if (FAILED(hresult))
        {
            /* If error blob was allocated, print errors, and release it. */
            if (errorBlobHandle)
            {
                /* TODO: Print errors. */
                const char* errors = (const char*)errorBlobHandle->GetBufferPointer();
                errorBlobHandle->Release();
            }

            /* If shader blob was allocated, release it. */
            if (shaderBlobHandle) shaderBlobHandle->Release();

            /* Return result. */
            return hlINWin32GetResult(hresult);
        }
    }

    /* Setup HrShaderCode and return. */
    {
        HrShaderCode* shaderCode;
        void* shaderBytecode;

        /* Allocate memory for shader bytecode. */
        shaderCode = (HrShaderCode*)hlAlloc(sizeof(HrShaderCode) +
            shaderBlobHandle->GetBufferSize());

        if (!shaderCode)
        {
            shaderBlobHandle->Release();
            return HL_ERROR_OUT_OF_MEMORY;
        }

        /* Set shaderBytecode pointer. */
        shaderBytecode = HL_ADD_OFF(shaderCode, sizeof(HrShaderCode));

        /* Copy shader bytecode. */
        memcpy(shaderBytecode, shaderBlobHandle->GetBufferPointer(),
            shaderBlobHandle->GetBufferSize());

        /* Setup HrShaderCode. */
        shaderCode->code = shaderBytecode;
        shaderCode->codeLen = shaderBlobHandle->GetBufferSize();
        *bytecode = shaderCode;

        /* Release shader blob and return success. */
        shaderBlobHandle->Release();
        return HL_RESULT_SUCCESS;
    }
}
