#include "hr_in_d3d11.h"
#include "hedgerender/hr_instance.h"
#include "hedgelib/hl_memory.h"

//static ID3D11Debug* pDebug;

HlResult hrInstanceCreate(HrAdapter* HL_RESTRICT adapter,
    HrInstance* HL_RESTRICT * HL_RESTRICT instance)
{
    HrInstance* hrInst;
    HlResult result;

    /* Allocate HrInstance. */
    hrInst = HL_ALLOC_OBJ(HrInstance);
    if (!hrInst) return HL_ERROR_OUT_OF_MEMORY;

    /* Setup HrInstance and return. */
    {
        const D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_0
        };

        UINT deviceFlags = 0;

#ifdef DEBUG
        deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        /* Create a D3D11 device. */
        result = hlINWin32GetResult(D3D11CreateDevice((IDXGIAdapter1*)((void*)adapter),
            D3D_DRIVER_TYPE_UNKNOWN, NULL, deviceFlags,featureLevels, 1,
            D3D11_SDK_VERSION, &hrInst->device, NULL, &hrInst->context));

        if (HL_FAILED(result)) goto failed;

        /* Create per-instance buffer. */
        {
            const D3D11_BUFFER_DESC bufDesc =
            {
                (64 * HL_IN_D3D11_MAX_INST_COUNT),  /* ByteWidth */
                D3D11_USAGE_DYNAMIC,                /* Usage */
                D3D11_BIND_VERTEX_BUFFER,           /* BindFlags */
                D3D11_CPU_ACCESS_WRITE,             /* CPUAccessFlags */
                0,                                  /* MiscFlags */
                0                                   /* StructureByteStride */
            };

            result = hlINWin32GetResult(hrInst->device->CreateBuffer(
                &bufDesc, NULL, &hrInst->instBuf));

            if (HL_FAILED(result))
            {
                hrInst->context->Release();
                hrInst->device->Release();
                goto failed;
            }
        }

        ///* Create default cube shader. */
        //{
        //    result = hlINWin32GetResult(hrInst->device->CreateVertexShader(
        //        dxc_DefaultCube_vs, sizeof(dxc_DefaultCube_vs), NULL, &hrInst->cubeVS));

        //    if (HL_FAILED(result))
        //    {
        //        hrInst->instBuf->Release();
        //        hrInst->context->Release();
        //        hrInst->device->Release();
        //        goto failed;
        //    }
        //}

        /*hrInst->device->QueryInterface(IID_PPV_ARGS(&pDebug));*/

        /* Set HrInstance pointer and return success. */
        *instance = hrInst;
        return HL_RESULT_SUCCESS;
    }

failed:
    hlFree(hrInst);
    return result;
}

void hrInstanceDestroy(HrInstance* instance)
{
    /* Return early if instance pointer is null. */
    if (!instance) return;

    /* Release D3D resources. */
    /*instance->cubeVS->Release();*/
    instance->instBuf->Release();
    instance->context->Release();

    /*pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
    pDebug->Release();*/

    instance->device->Release();

    /* Free HrInstance. */
    hlFree(instance);
}
