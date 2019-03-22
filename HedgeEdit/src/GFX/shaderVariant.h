#pragma once
#include "d3d.h"
#include <d3d11.h>
#include <Shaders/IShaderVariant.h>
#include <cstddef>
#include <array>
#include <memory>
#include <filesystem>

namespace HedgeEdit::GFX
{
	class ShaderVariant
	{
    protected:
        HedgeLib::Shaders::IShaderVariant* variant;

    public:
        inline ShaderVariant(HedgeLib::Shaders::IShaderVariant* variant)
            noexcept : variant(variant) {};

		virtual ~ShaderVariant() = 0;
		virtual void Use(ID3D11DeviceContext* context) const noexcept = 0;

        inline std::uint8_t* Bytecode() const noexcept
        {
            return variant->Bytecode();
        }

        inline std::size_t BytecodeSize() const noexcept
        {
            return variant->BytecodeSize();
        }
	};
	
	class VertexShaderVariant : public ShaderVariant
	{
		ID3D11VertexShader* vertexShader;

        void Init(ID3D11Device* device);

	public:
		inline VertexShaderVariant(ID3D11Device* device,
            HedgeLib::Shaders::IShaderVariant* variant) :
            ShaderVariant(variant)
		{
			Init(device);
		}

		inline VertexShaderVariant(HedgeLib::Shaders::IShaderVariant* variant,
            ID3D11VertexShader* vertexShader) :
			ShaderVariant(variant), vertexShader(vertexShader) {}

		inline ~VertexShaderVariant() override
		{
			//SAFE_RELEASE(inputLayout);
			SAFE_RELEASE(vertexShader);
		}

		inline void Use(ID3D11DeviceContext* context)
			const noexcept override
		{
			//context->IASetInputLayout(inputLayout);
			context->VSSetShader(vertexShader, nullptr, 0);
		}
	};

	class PixelShaderVariant : ShaderVariant
	{
		ID3D11PixelShader* pixelShader;

		void Init(ID3D11Device* device);

	public:
		inline PixelShaderVariant(ID3D11Device* device,
            HedgeLib::Shaders::IShaderVariant* variant) :
            ShaderVariant(variant)
		{
			Init(device);
		}

		inline PixelShaderVariant(HedgeLib::Shaders::IShaderVariant* variant,
            ID3D11PixelShader* pixelShader) :
			ShaderVariant(variant), pixelShader(pixelShader) {}

		inline ~PixelShaderVariant() override
		{
			SAFE_RELEASE(pixelShader);
		}

		inline void Use(ID3D11DeviceContext* context)
			const noexcept override
		{
			context->PSSetShader(pixelShader, nullptr, 0);
		}
	};
}
