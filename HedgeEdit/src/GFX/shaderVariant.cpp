#include "shaderVariant.h"
#include <d3d11.h>
//#include <d3dcompiler.h>
#include <stdexcept>
#include <utility>

namespace HedgeEdit::GFX
{
    ShaderVariant::~ShaderVariant() {}

    /*static HRESULT CompileD3DShader(const std::filesystem::path filePath, 
        const char* entryPoint, const char* target, ID3DBlob** byteCode)
    {
        return D3DCompileFromFile(filePath.wstring().c_str(),
            nullptr, nullptr, entryPoint, target, 0, 0,
            byteCode, nullptr);
    }*/

    void VertexShaderVariant::Init(ID3D11Device* device)
    {
        // Get shader bytecode
        std::uint8_t* bytecode = variant->Bytecode();
        SIZE_T bytecodeSize = static_cast<SIZE_T>(
            variant->BytecodeSize());

        // Create D3D Shader from compiled HLSL
        HRESULT result = device->CreateVertexShader(bytecode,
            bytecodeSize, nullptr, &vertexShader);

        if (FAILED(result))
            throw std::runtime_error("Could not create D3D Vertex Shader!");
    }

    //void VShader::Load(ID3D11Device* device, const std::filesystem::path filePath,
    //	const D3D11_INPUT_ELEMENT_DESC* elements, const UINT elementCount,
    //	const char* entryPoint)
    //{
    //	// Load HLSL code and compile it
    //	ID3DBlob* vsByteCode;
    //	HRESULT result = CompileD3DShader(filePath,
    //		entryPoint, Target, &vsByteCode);

    //	if (FAILED(result))
    //		throw std::runtime_error("Could not compile D3D Vertex Shader!");

    //	// Create vertex shader
    //	try
    //	{
    //		Init(device, vsByteCode->GetBufferPointer(),
    //			vsByteCode->GetBufferSize(), elements, elementCount);
    //	}
    //	catch (std::runtime_error)
    //	{
    //		SAFE_RELEASE(vsByteCode);
    //		throw;
    //	}

    //	// Release compiled HLSL code
    //	SAFE_RELEASE(vsByteCode);
    //}

    void PixelShaderVariant::Init(ID3D11Device* device)
    {
        // Get shader bytecode
        std::uint8_t* bytecode = variant->Bytecode();
        SIZE_T bytecodeSize = static_cast<SIZE_T>(
            variant->BytecodeSize());

        // Create D3D Shader from compiled HLSL
        HRESULT result = device->CreatePixelShader(bytecode,
            bytecodeSize, nullptr, &pixelShader);

        if (FAILED(result))
            throw std::runtime_error("Could not create D3D Pixel Shader!");
    }

    //void PShader::Load(ID3D11Device* device, const
    //	std::filesystem::path filePath, const char* entryPoint)
    //{
    //	// Load HLSL code and compile it
    //	ID3DBlob* psByteCode;
    //	HRESULT result = CompileD3DShader(filePath,
    //		entryPoint, Target, &psByteCode);

    //	if (FAILED(result))
    //		throw std::runtime_error("Could not compile D3D Pixel Shader!");

    //	// Create pixel shader
    //	try
    //	{
    //		Init(device, psByteCode->GetBufferPointer(),
    //			psByteCode->GetBufferSize());
    //	}
    //	catch (std::runtime_error)
    //	{
    //		SAFE_RELEASE(psByteCode);
    //		throw;
    //	}

    //	// Release compiled HLSL code
    //	SAFE_RELEASE(psByteCode);
    //}
}
