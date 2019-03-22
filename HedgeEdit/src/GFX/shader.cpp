#include "shader.h"
#include "d3d.h"
#include <d3d11.h>
#include <Shaders/IShaderVariant.h>
#include <cstddef>
#include <utility>

using namespace HedgeLib::Shaders;

namespace HedgeEdit::GFX
{
    Shader::~Shader() {}

    VertexShader::VertexShader(ID3D11Device* device,
        std::unique_ptr<HedgeLib::Shaders::IShader> shader) :
        Shader(std::move(shader))
    {
        const std::size_t variantCount = this->shader->VariantCount();
        if (variantCount == 0)
            throw std::logic_error("Cannot create shader with no variants!");

        const std::unique_ptr<IShaderVariant>* origVariants =
            this->shader->Variants();

        // Create VertexShaderVariants
        variants.resize(variantCount);
        for (std::size_t i = 0; i < variantCount; ++i)
        {
            variants[i] = std::unique_ptr<VertexShaderVariant>(
                new VertexShaderVariant(device, origVariants[i].get()));
        }
    };

    void VertexShader::Use(ID3D11DeviceContext* context,
        const std::size_t flags) const noexcept
    {
        variants[VariantIndex(flags)]->Use(context);
    }

    PixelShader::PixelShader(ID3D11Device* device,
        std::unique_ptr<HedgeLib::Shaders::IShader> shader) :
        Shader(std::move(shader))
    {
        const std::size_t variantCount = this->shader->VariantCount();
        if (variantCount == 0)
            throw std::logic_error("Cannot create shader with no variants!");

        const std::unique_ptr<IShaderVariant>* origVariants =
            this->shader->Variants();

        // Create PixelShaderVariants
        variants.resize(variantCount);
        for (std::size_t i = 0; i < variantCount; ++i)
        {
            variants[i] = std::unique_ptr<PixelShaderVariant>(
                new PixelShaderVariant(device, origVariants[i].get()));
        }
    };

    void PixelShader::Use(ID3D11DeviceContext* context,
        const std::size_t flags) const noexcept
    {
        variants[VariantIndex(flags)]->Use(context);
    }
}
