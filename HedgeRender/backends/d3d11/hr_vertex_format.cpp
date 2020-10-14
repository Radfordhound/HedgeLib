#include "hr_in_d3d11.h"
#include "hedgerender/hr_vertex_format.h"
#include "hedgerender/hr_shader.h"
#include "hedgelib/models/hl_model.h"

static const D3D11_INPUT_ELEMENT_DESC HrIND3D11PerInstanceInputElements[] =
{
    { "INST_MTX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "INST_MTX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "INST_MTX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "INST_MTX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
};

HlResult hrVertexFormatCreate(HrInstance* HL_RESTRICT instance,
    const HlVertexFormat* HL_RESTRICT hlVtxFmt,
    const HrShaderCode* HL_RESTRICT code,
    HrVertexFormat* HL_RESTRICT * HL_RESTRICT hrVtxFmt)
{
    D3D11_INPUT_ELEMENT_DESC inputElemsBuf[16];
    D3D11_INPUT_ELEMENT_DESC* inputElems = inputElemsBuf;
    HrVertexFormat* hrVtxFmtBuf;
    size_t i;
    HlResult result;

    /* Allocate input element description buffer on heap if necessary. */
    if ((hlVtxFmt->vertexElementCount + 5) > 16)
    {
        inputElems = HL_ALLOC_ARR(D3D11_INPUT_ELEMENT_DESC,
            hlVtxFmt->vertexElementCount + 5);

        if (!inputElems) return HL_ERROR_OUT_OF_MEMORY;
    }
    
    /* Generate input element descriptions. */
    for (i = 0; i < hlVtxFmt->vertexElementCount; ++i)
    {
        /* Get semantic name. */
        switch (hlVtxFmt->vertexElements[i].type & HL_VERTEX_ELEM_TYPE_MASK)
        {
        case HL_VERTEX_ELEM_TYPE_POSITION:
            inputElems[i].SemanticName = "POSITION";
            break;

        case HL_VERTEX_ELEM_TYPE_BONE_WEIGHT:
            inputElems[i].SemanticName = "BLENDWEIGHT";
            break;

        case HL_VERTEX_ELEM_TYPE_BONE_INDEX:
            inputElems[i].SemanticName = "BLENDINDICES";
            break;

        case HL_VERTEX_ELEM_TYPE_NORMAL:
            inputElems[i].SemanticName = "NORMAL";
            break;

        case HL_VERTEX_ELEM_TYPE_TEXCOORD:
            inputElems[i].SemanticName = "TEXCOORD";
            break;

        case HL_VERTEX_ELEM_TYPE_TANGENT:
            inputElems[i].SemanticName = "TANGENT";
            break;

        case HL_VERTEX_ELEM_TYPE_BINORMAL:
            inputElems[i].SemanticName = "BINORMAL";
            break;

        case HL_VERTEX_ELEM_TYPE_COLOR:
            inputElems[i].SemanticName = "COLOR";
            break;

        default:
            result = HL_ERROR_UNSUPPORTED;
            goto failed;
        }

        /* Get semantic index. */
        switch (hlVtxFmt->vertexElements[i].type & HL_VERTEX_ELEM_INDEX_MASK)
        {
        default:
        case HL_VERTEX_ELEM_INDEX0:
            inputElems[i].SemanticIndex = 0;
            break;

        case HL_VERTEX_ELEM_INDEX1:
            inputElems[i].SemanticIndex = 1;
            break;

        case HL_VERTEX_ELEM_INDEX2:
            inputElems[i].SemanticIndex = 2;
            break;

        case HL_VERTEX_ELEM_INDEX3:
            inputElems[i].SemanticIndex = 3;
            break;
        }

        /* Get DXGI format. */
        switch (hlVtxFmt->vertexElements[i].type & HL_VERTEX_ELEM_FORMAT_MASK)
        {
        default:
            result = HL_ERROR_UNSUPPORTED;
            goto failed;

            /* Auto-generate the rest of this switch's body. */
#define HR_IN_DXGI_FORMAT_AUTOGEN_DEFAULT() DXGI_FORMAT_UNKNOWN;\
            result = HL_ERROR_UNSUPPORTED;\
            goto failed;

#define HR_IN_DXGI_FORMAT_AUTOGEN(hlFormat, dxgiFormat1D,\
    dxgiFormat2D, dxgiFormat3D, dxgiFormat4D)\
            case hlFormat:\
                switch (hlVtxFmt->vertexElements[i].type & HL_VERTEX_ELEM_DIMENSION_MASK)\
                {\
                case HL_VERTEX_ELEM_DIMENSION_1D:\
                    inputElems[i].Format = dxgiFormat1D;\
                    break;\
                case HL_VERTEX_ELEM_DIMENSION_2D:\
                    inputElems[i].Format = dxgiFormat2D;\
                    break;\
                case HL_VERTEX_ELEM_DIMENSION_3D:\
                    inputElems[i].Format = dxgiFormat3D;\
                    break;\
                case HL_VERTEX_ELEM_DIMENSION_4D:\
                    inputElems[i].Format = dxgiFormat4D;\
                    break;\
                }\
                break;

#include "../hr_in_dxgi_autogen.h"
        }
        
        /* Get remaining values. */
        inputElems[i].InputSlot = 0;
        inputElems[i].AlignedByteOffset = (UINT)hlVtxFmt->vertexElements[i].offset;
        /* TODO: Also support D3D11_INPUT_PER_INSTANCE_DATA somehow. */
        inputElems[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        inputElems[i].InstanceDataStepRate = 0;
    }

    /* Create per-instance-data input element descriptions. */
    inputElems[i++] = HrIND3D11PerInstanceInputElements[0];
    inputElems[i++] = HrIND3D11PerInstanceInputElements[1];
    inputElems[i++] = HrIND3D11PerInstanceInputElements[2];
    inputElems[i] = HrIND3D11PerInstanceInputElements[3];

    /* Create HrVertexFormat buffer. */
    hrVtxFmtBuf = HL_ALLOC_OBJ(HrVertexFormat);
    if (!hrVtxFmtBuf)
    {
        result = HL_ERROR_OUT_OF_MEMORY;
        goto failed;
    }

    /* Create input layout. */
    result = hlINWin32GetResult(instance->device->CreateInputLayout(inputElems,
        (UINT)(hlVtxFmt->vertexElementCount + 4), code->code, code->codeLen,
        &hrVtxFmtBuf->inputLayout));

    if (HL_FAILED(result))
    {
        hlFree(hrVtxFmtBuf);
        goto failed;
    }

    /* Set stride. */
    hrVtxFmtBuf->stride = (UINT)hlVtxFmt->vertexFormatSize;

    /* Set pointers and return result. */
    *hrVtxFmt = hrVtxFmtBuf;

failed:
    if (inputElems != inputElemsBuf)
        hlFree(inputElems);

    return result;
}

void hrVertexFormatDestroy(HrVertexFormat* hrVtxFmt)
{
    if (!hrVtxFmt) return;
    hrVtxFmt->inputLayout->Release();
    hlFree(hrVtxFmt);
}
