#pragma once
#include "shaderVariant.h"
#include <d3d11.h>
#include <Shaders/IShader.h>
#include <Shaders/IShaderVariant.h>
#include <memory>
#include <utility>
#include <vector>
#include <cstddef>
#include <cstdint>

namespace HedgeEdit::GFX
{
    class Shader
    {
    protected:
        std::unique_ptr<HedgeLib::Shaders::IShader> shader;

    public:
        inline Shader(std::unique_ptr<HedgeLib::Shaders::IShader> shader)
            noexcept : shader(std::move(shader)) {};

        virtual ~Shader() = 0;

        inline std::size_t Flag(const char* variantType) const noexcept
        {
            return shader->Flag(variantType);
        }

        inline std::size_t VariantIndex(const std::size_t flags) const noexcept
        {
            return shader->VariantIndex(flags);
        }

        virtual void Use(ID3D11DeviceContext* context,
            const std::size_t flags) const noexcept = 0;
    };

    class VertexShader : public Shader
    {
        std::vector<std::unique_ptr<VertexShaderVariant>> variants;

    public:
        VertexShader(ID3D11Device* device,
            std::unique_ptr<HedgeLib::Shaders::IShader> shader);

        inline ~VertexShader() override = default;

        inline const std::vector<std::unique_ptr
            <VertexShaderVariant>>& Variants() const noexcept
        {
            return variants;
        }

        void Use(ID3D11DeviceContext* context, const std::size_t flags)
            const noexcept override;
    };

    class PixelShader : public Shader
    {
        std::vector<std::unique_ptr<PixelShaderVariant>> variants;

    public:
        PixelShader(ID3D11Device* device, std::unique_ptr
            <HedgeLib::Shaders::IShader> shader);

        inline ~PixelShader() override = default;

        inline const std::vector<std::unique_ptr
            <PixelShaderVariant>>& Variants() const noexcept
        {
            return variants;
        }

        void Use(ID3D11DeviceContext* context, const std::size_t flags)
            const noexcept override;
    };
}
