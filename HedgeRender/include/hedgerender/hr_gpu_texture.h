#ifndef HR_GPU_TEXTURE_H_INCLUDED
#define HR_GPU_TEXTURE_H_INCLUDED
#include "hr_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HrInstance HrInstance;

typedef enum HrGPUTextureType
{
    HR_GPU_TEX_TYPE_1D,
    HR_GPU_TEX_TYPE_2D,
    HR_GPU_TEX_TYPE_3D
}
HrGPUTextureType;

typedef struct HrGPUTexture
{
    HrGPUTextureType type;
    void* handle;
    void* shaderViewHandle;
}
HrGPUTexture;

HR_BACK_FUNC(HlResult, hrGPUTextureCreateFromR8G8B8A8)(HrInstance* HL_RESTRICT instance,
    const void* HL_RESTRICT texData, size_t width, size_t height, size_t depth,
    size_t mipLevels, size_t arrayCount, HrGPUTexture* HL_RESTRICT * HL_RESTRICT gpuTex);

#define hrGPUTexture1DCreateFromR8G8B8A8(instance, texData,\
    width, mipLevels, arrayCount, gpuTex) hrGPUTextureCreateFromR8G8B8A8(\
    instance, texData, width, 0, 0, mipLevels, arrayCount, gpuTex)

#define hrGPUTexture2DCreateFromR8G8B8A8(instance, texData,\
    width, height, mipLevels, arrayCount, gpuTex) hrGPUTextureCreateFromR8G8B8A8(\
    instance, texData, width, height, 0, mipLevels, arrayCount, gpuTex)

#define hrGPUTexture3DCreateFromR8G8B8A8 hrGPUTextureCreateFromR8G8B8A8

HR_BACK_FUNC(HlResult, hrGPUTextureCreateFromDDS)(HrInstance* HL_RESTRICT instance,
    const void* HL_RESTRICT texData, size_t texDataSize,
    HrGPUTexture* HL_RESTRICT * HL_RESTRICT gpuTex);

HR_BACK_FUNC(void, hrGPUTextureDestroy)(HrGPUTexture* gpuTex);

#ifdef __cplusplus
}
#endif
#endif
