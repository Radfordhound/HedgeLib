#include "hr_in_d3d11.h"
#include "hedgerender/hr_gpu_texture.h"
#include "depends/DDSTextureLoader11.h"

HlResult hrGPUTextureCreateFromR8G8B8A8(HrInstance* HL_RESTRICT instance,
    const void* HL_RESTRICT texData, size_t width, size_t height, size_t depth,
    size_t mipLevels, size_t arrayCount, HrGPUTexture* HL_RESTRICT * HL_RESTRICT gpuTex)
{
    HrGPUTexture* gpuTexBuf;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    HRESULT hresult;

    /* Allocate HrGPUTexture. */
    gpuTexBuf = HL_ALLOC_OBJ(HrGPUTexture);
    if (!gpuTexBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Create 3D texture. */
    if (depth > 0)
    {
        /* Create descriptions. */
        const D3D11_TEXTURE3D_DESC desc =
        {
            (UINT)width,                /* Width */
            (UINT)height,               /* Height */
            (UINT)depth,                /* Depth */
            (UINT)mipLevels,            /* MipLevels */
            DXGI_FORMAT_R8G8B8A8_UNORM, /* Format */
            D3D11_USAGE_IMMUTABLE,      /* Usage */ /* TODO: LET USER SET THIS! */
            D3D11_BIND_SHADER_RESOURCE, /* BindFlags */
            0,                          /* CPUAccessFlags */ /* TODO: LET USER SET THIS! */
            0                           /* MiscFlags */ /* TODO: LET USER SET THIS! */
        };

        const D3D11_SUBRESOURCE_DATA subResource =
        {
            texData,                            /* pSysMem */
            (desc.Width * 4),                   /* SysMemPitch */
            ((desc.Width * desc.Height) * 4)    /* SysMemSlicePitch */ /* TODO: IS THIS RIGHT? */
        };

        hresult = instance->device->CreateTexture3D(&desc,
            &subResource, (ID3D11Texture3D**)&gpuTexBuf->handle);

        /* Set type. */
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
        gpuTexBuf->type = HR_GPU_TEX_TYPE_3D;
    }

    /* Create 2D texture. */
    else if (height > 0)
    {
        /* Create descriptions. */
        const D3D11_TEXTURE2D_DESC desc =
        {
            (UINT)width,                /* Width */
            (UINT)height,               /* Height */
            (UINT)mipLevels,            /* MipLevels */
            (UINT)arrayCount,           /* ArraySize */
            DXGI_FORMAT_R8G8B8A8_UNORM, /* Format */
            { 1, 0 },                   /* SampleDesc */
            D3D11_USAGE_IMMUTABLE,      /* Usage */ /* TODO: LET USER SET THIS! */
            D3D11_BIND_SHADER_RESOURCE, /* BindFlags */
            0,                          /* CPUAccessFlags */ /* TODO: LET USER SET THIS! */
            0                           /* MiscFlags */ /* TODO: LET USER SET THIS! */
        };

        const D3D11_SUBRESOURCE_DATA subResource =
        {
            texData,                    /* pSysMem */
            (desc.Width * 4),           /* SysMemPitch */
            0                           /* SysMemSlicePitch */
        };
        
        hresult = instance->device->CreateTexture2D(&desc,
            &subResource, (ID3D11Texture2D**)&gpuTexBuf->handle);

        /* Set type. */
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        gpuTexBuf->type = HR_GPU_TEX_TYPE_2D;
    }

    /* Create 1D texture. */
    else
    {
        /* Create descriptions. */
        const D3D11_TEXTURE1D_DESC desc =
        {
            (UINT)width,                /* Width */
            (UINT)mipLevels,            /* MipLevels */
            (UINT)arrayCount,           /* ArraySize */
            DXGI_FORMAT_R8G8B8A8_UNORM, /* Format */
            D3D11_USAGE_IMMUTABLE,      /* Usage */ /* TODO: LET USER SET THIS! */
            D3D11_BIND_SHADER_RESOURCE, /* BindFlags */
            0,                          /* CPUAccessFlags */ /* TODO: LET USER SET THIS! */
            0                           /* MiscFlags */ /* TODO: LET USER SET THIS! */
        };

        const D3D11_SUBRESOURCE_DATA subResource =
        {
            texData,                    /* pSysMem */
            0,                          /* SysMemPitch */
            0                           /* SysMemSlicePitch */
        };

        hresult = instance->device->CreateTexture1D(&desc,
            &subResource, (ID3D11Texture1D**)&gpuTexBuf->handle);

        /* Set type. */
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
        gpuTexBuf->type = HR_GPU_TEX_TYPE_1D;
    }

    /* Return error if texture creation failed. */
    if (FAILED(hresult))
    {
        hlFree(gpuTexBuf);
        return hlINWin32GetResult(hresult);
    }

    /* Create shader resource view. */
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    switch (srvDesc.ViewDimension)
    {
    case D3D11_SRV_DIMENSION_TEXTURE3D:
        srvDesc.Texture3D.MostDetailedMip = 0;
        srvDesc.Texture3D.MipLevels = (UINT)mipLevels;
        break;

    case D3D11_SRV_DIMENSION_TEXTURE2D:
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = (UINT)mipLevels;
        break;

    case D3D11_SRV_DIMENSION_TEXTURE1D:
        srvDesc.Texture1D.MostDetailedMip = 0;
        srvDesc.Texture1D.MipLevels = (UINT)mipLevels;
        break;
    }

    hresult = instance->device->CreateShaderResourceView(
        (ID3D11Resource*)gpuTexBuf->handle, &srvDesc,
        (ID3D11ShaderResourceView**)&gpuTexBuf->shaderViewHandle);

    if (FAILED(hresult))
    {
        ((ID3D11Resource*)gpuTexBuf->handle)->Release();
        hlFree(gpuTexBuf);
        return hlINWin32GetResult(hresult);
    }

    /* Set pointer and return success. */
    *gpuTex = gpuTexBuf;
    return HL_RESULT_SUCCESS;
}

HlResult hrGPUTextureCreateFromDDS(HrInstance* HL_RESTRICT instance,
    const void* HL_RESTRICT texData, size_t texDataSize,
    HrGPUTexture* HL_RESTRICT * HL_RESTRICT gpuTex)
{
    HrGPUTexture* gpuTexBuf;
    HRESULT hresult;

    /* Allocate HrGPUTexture. */
    gpuTexBuf = HL_ALLOC_OBJ(HrGPUTexture);
    if (!gpuTexBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Create DDS texture from memory. */
    hresult = DirectX::CreateDDSTextureFromMemory(instance->device,
        instance->context, (const uint8_t*)texData,
        texDataSize, (ID3D11Resource**)&gpuTexBuf->handle,
        (ID3D11ShaderResourceView**)&gpuTexBuf->shaderViewHandle);

    if (FAILED(hresult))
    {
        hlFree(gpuTexBuf);
        return hlINWin32GetResult(hresult);
    }

    /* Set pointer and return success. */
    *gpuTex = gpuTexBuf;
    return HL_RESULT_SUCCESS;
}

void hrGPUTextureDestroy(HrGPUTexture* gpuTex)
{
    if (!gpuTex) return;

    /* Release texture data. */
    ((ID3D11Resource*)gpuTex->handle)->Release();

    /* Release shader view. */
    ((ID3D11ShaderResourceView*)gpuTex->shaderViewHandle)->Release();

    /* Release buffer. */
    hlFree(gpuTex);
}
