#include "shader.h"
#include "instance.h"
#include <cstdlib>
#include <utility>
#include <stdexcept>

namespace HedgeEdit::GFX
{
    VertexShaderVariant::VertexShaderVariant(const Instance& inst,
        const void* bytecode, std::size_t bytecodeLength, const void* signaturePtr,
        std::size_t signatureLength, std::unique_ptr<std::uint32_t[]> samplerIndices,
        std::uint32_t samplerCount) : signaturePtr(signaturePtr),
        signatureLength(signatureLength), samplerIndices(std::move(samplerIndices)),
        samplerCount(samplerCount)
    {
#ifdef D3D11
        // Create vertex shader
        HRESULT result = inst.Device->CreateVertexShader(bytecode,
            bytecodeLength, nullptr, data.put());

        if (FAILED(result))
            throw std::runtime_error("Could not create vertex shader!");
#endif
    }

    void VertexShaderVariant::Use(Instance& inst) const
    {
        // Use samplers
        for (std::uint32_t i = 0; i < samplerCount; ++i)
        {
            inst.VSUseSampler(samplerIndices[i], i);
        }

        // Use shader
#ifdef D3D11
        inst.Context->VSSetShader(data.get(), nullptr, 0);
#endif
    }

    PixelShaderVariant::PixelShaderVariant(const Instance& inst,
        const void* bytecode, std::size_t bytecodeLength,
        std::unique_ptr<std::uint32_t[]> samplerIndices, std::uint32_t samplerCount) :
        samplerIndices(std::move(samplerIndices)), samplerCount(samplerCount)
    {
#ifdef D3D11
        // Create pixel shader
        HRESULT result = inst.Device->CreatePixelShader(bytecode,
            bytecodeLength, nullptr, data.put());

        if (FAILED(result))
            throw std::runtime_error("Could not create pixel shader!");
#endif
    }

    void PixelShaderVariant::Use(Instance& inst) const
    {
        // Use samplers
        for (std::uint32_t i = 0; i < samplerCount; ++i)
        {
            inst.PSUseSampler(samplerIndices[i], i);
        }

        // Use shader
#ifdef D3D11
        inst.Context->PSSetShader(data.get(), nullptr, 0);
#endif
    }

    VertexShader::~VertexShader()
    {
        delete[] variants;
        std::free(signatures);
    }
}
