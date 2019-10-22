#pragma once
#include <cstddef>

#ifdef D3D11
#include <winrt/base.h>
#include <d3d11.h>
#endif

namespace HedgeEdit::GFX
{
    class Instance;

    class VertexShaderVariant
    {
#ifdef D3D11
        winrt::com_ptr<ID3D11VertexShader> data;
#endif

        const void* signaturePtr;
        std::size_t signatureLength;
        std::unique_ptr<std::uint32_t[]> samplerIndices;
        std::uint32_t samplerCount;

    public:
        VertexShaderVariant(const Instance& inst, const void* bytecode,
            std::size_t bytecodeLength, const void* signaturePtr,
            std::size_t signatureLength, std::unique_ptr<std::uint32_t[]> samplerIndices,
            std::uint32_t samplerCount);

        inline const void* GetSignature() const
        {
            return signaturePtr;
        }

        inline std::size_t GetSignatureLength() const
        {
            return signatureLength;
        }

        void Use(Instance& inst) const;
    };

    class PixelShaderVariant
    {
#ifdef D3D11
        winrt::com_ptr<ID3D11PixelShader> data;
#endif

        std::unique_ptr<std::uint32_t[]> samplerIndices;
        std::uint32_t samplerCount;

    public:
        PixelShaderVariant(const Instance& inst, const void* bytecode,
            std::size_t bytecodeLength, std::unique_ptr<std::uint32_t[]> samplerIndices,
            std::uint32_t samplerCount);

        void Use(Instance& inst) const;
    };

    class VertexShader
    {
    protected:
        VertexShaderVariant* variants;
        void* signatures;
        std::size_t variantCount;

    public:
        inline VertexShader(VertexShaderVariant* variants, void* signatures,
            std::size_t variantCount) : variants(variants),
            variantCount(variantCount), signatures(signatures) {}

        ~VertexShader();

        inline VertexShaderVariant& GetVariant(std::size_t index)
        {
            return variants[index];
        }

        inline const VertexShaderVariant& GetVariant(std::size_t index) const
        {
            return variants[index];
        }

        inline std::size_t GetVariantCount() const
        {
            return variantCount;
        }
    };

    class PixelShader
    {
    protected:
        std::unique_ptr<PixelShaderVariant[]> variants;
        std::size_t variantCount;

    public:
        inline PixelShader(PixelShaderVariant* variants, std::size_t variantCount) :
            variants(variants), variantCount(variantCount) {}

        inline PixelShaderVariant& GetVariant(std::size_t index)
        {
            return variants[index];
        }

        inline const PixelShaderVariant& GetVariant(std::size_t index) const
        {
            return variants[index];
        }

        inline std::size_t GetVariantCount() const
        {
            return variantCount;
        }
    };
}
