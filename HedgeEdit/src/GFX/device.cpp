// Because Microsoft hasn't updated wrl for proper C++17 conformity
#pragma warning(disable : 5043)

#include "device.h"
#include "renderMode.h"
#include "d3d.h"
#include <d3d11.h>
#include <dxgi.h>
#include <wrl.h>
#include <Geometry/VertexFormat.h>
#include <Shaders/IShader.h>
#include <stdexcept>
#include <memory>
#include <cstdint>
#include <string>
#include <utility>
#include <optional>
#include <algorithm>

using namespace HedgeLib::Shaders;
using namespace HedgeLib::Geometry;

namespace HedgeEdit::GFX
{
    const UINT VertexBufferOffset = 0, FallbackBufferStride = Device::FallbackBufferSize;

    std::size_t HashVertexElement(const HedgeLib::Geometry::VertexElement& f) noexcept
    {
        // Slight modification of The Bernstein Hash based on this SO answer:
        // https://stackoverflow.com/a/34006336
        const std::size_t seed = 1009, factor = 9176;
        std::size_t hash = seed;

        hash = (hash * factor) + std::hash<std::uint32_t>()(f.Offset);
        hash = (hash * factor) + std::hash<std::uint32_t>()(
            static_cast<std::uint32_t>(f.Format));

        hash = (hash * factor) + std::hash<std::uint16_t>()(f.Type);
        hash = (hash * factor) + std::hash<std::uint8_t>()(f.Index);

        return hash;
    }

    std::size_t Device::VertexFormatHasher::operator()(const
        std::pair<const HedgeLib::Geometry::VertexElement*,
        const std::size_t>& key) const noexcept
    {
        // Slight modification of The Bernstein Hash based on this SO answer:
        // https://stackoverflow.com/a/34006336
        const std::size_t seed = 1009, factor = 9176;
        std::size_t hash = seed;

        for (std::size_t i = 0; i < key.second; ++i)
        {
            hash = (hash * factor) + HashVertexElement(key.first[i]);
        }

        return hash;
    }

    std::size_t Device::CreateInputLayoutHH2(
        const HedgeLib::Geometry::VertexElement* elements,
        const std::size_t elementCount, const VertexShader& shader,
        const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputElements,
        const std::size_t flags, const std::size_t shaderFlags)
    {
        // Create copy of elements data to operate on
        std::unique_ptr<D3D11_INPUT_ELEMENT_DESC[]> inputElementsCopy =
            std::make_unique<D3D11_INPUT_ELEMENT_DESC[]>(inputElements.size());

        std::copy(inputElements.begin(), inputElements.end(),
            inputElementsCopy.get());

        // Format input elements to align with vertex format
        D3D11_INPUT_ELEMENT_DESC* element;
        for (std::size_t i = 0; i < elementCount; ++i)
        {
            // Skip vertex formats with indices that don't
            // correspond to any semantic in the shader
            if (elements[i].Index != 0)
            {
                switch (elements[i].Type)
                {
                // Used by default HH2 shaders
                case VERTEX_TYPE_TEXCOORD:
                    if (elements[i].Index > 3)
                        continue;
                    break;

                // Used by enable_max_bone_influences_8
                case VERTEX_TYPE_BLEND_WEIGHT:
                case VERTEX_TYPE_BLEND_INDICES:
                    if (elements[i].Index != 1 ||
                        !(flags & VSV_FLAG_HH2_MAX_BONE_INFLUENCES))
                        continue;
                    break;

                // Used by enable_multi_tangent_space
                case VERTEX_TYPE_TANGENT:
                case VERTEX_TYPE_BINORMAL:
                    if (elements[i].Index != 1 ||
                        !(flags & VSV_FLAG_HH2_MULTI_TANGENT_SPACE))
                        continue;
                    break;

                default:
                    continue;
                }
            }

            // Get input element of type which corresponds with vertex format
            switch (elements[i].Type)
            {
            case VERTEX_TYPE_POSITION:
                element = inputElementsCopy.get();
                break;

            case VERTEX_TYPE_BLEND_WEIGHT:
                element = (inputElementsCopy.get() +
                    ((elements[i].Index == 1) ? 11 : 9));
                break;

            case VERTEX_TYPE_BLEND_INDICES:
                element = (inputElementsCopy.get() +
                    ((elements[i].Index == 1) ? 12 : 10));
                break;

            case VERTEX_TYPE_NORMAL:
                element = (inputElementsCopy.get() + 1);
                break;

            case VERTEX_TYPE_TEXCOORD:
                if (elements[i].Index > 3)
                    continue;

                element = (inputElementsCopy.get() + 5) +
                    elements[i].Index;
                break;

            case VERTEX_TYPE_TANGENT:
                element = (inputElementsCopy.get() + ((elements[i].Index == 1) ?
                    (flags & VSV_FLAG_HH2_MAX_BONE_INFLUENCES) ? 13 :
                    11 : 2));
                break;

            case VERTEX_TYPE_BINORMAL:
                element = (inputElementsCopy.get() + ((elements[i].Index == 1) ?
                    (flags & VSV_FLAG_HH2_MAX_BONE_INFLUENCES) ? 14 :
                    12 : 3));
                break;

            case VERTEX_TYPE_COLOR:
                element = (inputElementsCopy.get() + 4);
                break;

            default:
                // Just skip this vertex format if no corresponding type could be found
                continue;
            }

            // Set Offset and Input Slot
            element->AlignedByteOffset = static_cast<UINT>(elements[i].Offset);
            element->InputSlot = VertexBufferSlot;
        }

        // Get shader variant
        std::size_t variantIndex = shader.VariantIndex(shaderFlags);
        VertexShaderVariant* variant = shader.Variants()
            [variantIndex].get();

        // Create input layout
        std::size_t layoutIndex = inputLayouts.size();
        inputLayouts.push_back(nullptr);

        HRESULT result = device->CreateInputLayout(
            inputElementsCopy.get(),
            static_cast<UINT>(inputElements.size()),
            variant->Bytecode(),
            static_cast<SIZE_T>(variant->BytecodeSize()),
            inputLayouts[layoutIndex].GetAddressOf());

        if (FAILED(result))
        {
            throw std::logic_error(
                "Could not create a DirectX input layout!");
        }

        return layoutIndex;
    }

    Device::Device(const GFX::RenderMode renderMode) : renderMode(renderMode)
    {
        // Create a D3D Device and Context
        // TODO: Let user specify which video adapter to use
        // TODO: Maybe let user specify which feature levels to use?
        HRESULT result;
        UINT flags = 0;

#ifdef DEBUG
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        result = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE,
            NULL, flags, NULL, NULL, D3D11_SDK_VERSION,
            &device, NULL, &context);

        if (FAILED(result))
            throw std::runtime_error("Could not create a Direct3D 11 Device!");

        // Create a DXGI Factory
        // TODO: Use DXGI 1.2?
        // TODO: Fallback to DXGI 1.0 if 1.1 is not supported?
        result = CreateDXGIFactory1(__uuidof(IDXGIFactory1),
            (void**)(&factory));

        if (FAILED(result))
        {
            SAFE_RELEASE(context);
            SAFE_RELEASE(device);

            throw std::runtime_error("Could not create a DXGI 1.1 Factory!");
        }

        // Create Rasterizer State
        D3D11_RASTERIZER_DESC rd = {};
        rd.FillMode = D3D11_FILL_SOLID;
        rd.CullMode = D3D11_CULL_NONE;
        rd.FrontCounterClockwise = true;

        result = device->CreateRasterizerState(&rd, &rs);

        if (FAILED(result))
        {
            SAFE_RELEASE(factory);
            SAFE_RELEASE(context);
            SAFE_RELEASE(device);

            throw std::runtime_error(
                "Could not create Direct3D 11 Rasterizer State!");
        }

        context->RSSetState(rs);

        // Create fallback buffer
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.ByteWidth = FallbackBufferSize;
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        std::unique_ptr<std::uint8_t[]> data = std::make_unique<
            std::uint8_t[]>(FallbackBufferSize);

        D3D11_SUBRESOURCE_DATA bufferData = {};
        bufferData.pSysMem = data.get();

        result = device->CreateBuffer(&bufferDesc,
            &bufferData, &fallbackBuffer);

        if (FAILED(result))
        {
            SAFE_RELEASE(factory);
            SAFE_RELEASE(context);
            SAFE_RELEASE(device);
            SAFE_RELEASE(rs);

            throw std::runtime_error(
                "Could not create fallback buffer!");
        }

        // Bind fallback buffer
        BindFallbackBuffer();
    }

    Device::~Device() noexcept
    {
        // Release in reverse-order
        SAFE_RELEASE(fallbackBuffer);
        SAFE_RELEASE(rs);
        SAFE_RELEASE(factory);
        SAFE_RELEASE(context);
        SAFE_RELEASE(device);
    }

    void Device::Reset(const GFX::RenderMode renderMode) noexcept
    {
        // Clear all loaded resources
        // TODO

        // Set new render mode
        this->renderMode = renderMode;
    }

    ID3D11InputLayout* Device::GetInputLayout(
        const std::size_t index) const noexcept
    {
        return (index >= inputLayouts.size()) ?
            nullptr : inputLayouts[index].Get();
    }

    std::optional<std::size_t> Device::GetVertexFormatIndex(
        const HedgeLib::Geometry::VertexElement* elements,
        const std::size_t elementCount) const noexcept
    {
        auto it = vertexFormatIndices.find({ elements, elementCount });
        if (it != vertexFormatIndices.end())
            return it->second;

        return std::nullopt;
    }

    const std::size_t* Device::GetInputLayoutIndices(
        const HedgeLib::Geometry::VertexElement* elements,
        const std::size_t elementCount) const noexcept
    {
        std::optional<std::size_t> index = GetVertexFormatIndex(
            elements, elementCount);

        if (index.has_value())
            return inputLayoutIndices[index.value()].get();

        return nullptr;
    }

    const std::size_t* Device::GetInputLayoutIndices(
        const std::size_t vertexFormatIndex) const noexcept
    {
        return (vertexFormatIndex >= inputLayoutIndices.size()) ?
            nullptr : inputLayoutIndices[vertexFormatIndex].get();
    }

#define AddElement(name, index, format) inputElements.push_back({ name, index, format,\
    FallbackBufferSlot, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 })

    std::size_t Device::CreateInputLayouts(
        const HedgeLib::Geometry::VertexElement* elements,
        const std::size_t elementCount, const VertexShader& shader)
    {
        // Return if input layouts in this format already exist
        std::optional<std::size_t> index = GetVertexFormatIndex(
            elements, elementCount);

        if (index.has_value())
            return index.value();

        // Create the needed input layouts for each VSV
        std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;
        switch (renderMode)
        {
        case RENDER_MODE_HH2:
        {
            // Create the "default" HH2 Shader Variant's input layout
            AddElement("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT);
            AddElement("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT);
            AddElement("TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT);
            AddElement("BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT);
            AddElement("COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT);
            AddElement("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT);
            AddElement("TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT);
            AddElement("TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT);
            AddElement("TEXCOORD", 3, DXGI_FORMAT_R32G32_FLOAT);
            AddElement("BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT);
            AddElement("BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_SINT);

            std::size_t inputLayoutIndex = CreateInputLayoutHH2(elements,
                elementCount, shader, inputElements, VSV_FLAG_DEFAULT, 0);

            // Create an array of indices to input layouts - one for each shader variant
            std::size_t vertexFormatIndex = inputLayoutIndices.size();
            inputLayoutIndices.push_back(std::make_unique<
                std::size_t[]>(HH2VSVCount));

            std::unique_ptr<std::size_t[]>& indices = inputLayoutIndices.back();
            indices[VSV_FLAG_DEFAULT] = inputLayoutIndex;

            // Create the max bone influences variant's input layout
            AddElement("BLENDWEIGHT", 1, DXGI_FORMAT_R32G32B32A32_FLOAT);
            AddElement("BLENDINDICES", 1, DXGI_FORMAT_R32G32B32A32_SINT);

            std::size_t shaderFlags = shader.Flag("enable_max_bone_influences_8");
            if (shaderFlags != 0)
            {
                inputLayoutIndex = CreateInputLayoutHH2(elements, elementCount, shader,
                    inputElements, VSV_FLAG_HH2_MAX_BONE_INFLUENCES, shaderFlags);

                indices[VSV_FLAG_HH2_MAX_BONE_INFLUENCES] = inputLayoutIndex;
            }

            // Create the max bone influences + multi tangent space variant's input layout
            AddElement("TANGENT", 1, DXGI_FORMAT_R32G32B32_FLOAT);
            AddElement("BINORMAL", 1, DXGI_FORMAT_R32G32B32_FLOAT);

            shaderFlags = shader.Flag("enable_max_bone_influences_8") |
                shader.Flag("enable_multi_tangent_space");

            if (shaderFlags != 0)
            {
                inputLayoutIndex = CreateInputLayoutHH2(elements, elementCount, shader,
                    inputElements, VSV_FLAG_HH2_MAX_BONE_INFLUENCES |
                    VSV_FLAG_HH2_MULTI_TANGENT_SPACE, shaderFlags);

                indices[VSV_FLAG_HH2_MAX_BONE_INFLUENCES |
                    VSV_FLAG_HH2_MULTI_TANGENT_SPACE] = inputLayoutIndex;
            }

            // Create the multi tangent space variant's input layout
            shaderFlags = shader.Flag("enable_multi_tangent_space");

            if (shaderFlags != 0)
            {
                // Get rid of the BLENDWEIGHT1/BLENDINDICES1 elements
                inputElements.erase(inputElements.end() - 4, inputElements.end() - 2);

                inputLayoutIndex = CreateInputLayoutHH2(elements, elementCount, shader,
                    inputElements, VSV_FLAG_HH2_MULTI_TANGENT_SPACE, shaderFlags);

                indices[VSV_FLAG_HH2_MULTI_TANGENT_SPACE] = inputLayoutIndex;
            }

            return vertexFormatIndex;
        }

        default:
        {
            // TODO
            //CreateInputLayout(shader, VSV_FLAG_DEFAULT, inputElements);
            throw std::logic_error("Not yet implemented!");
        }
        }
    }

    VertexShader* Device::GetVertexShader(const std::string name) const noexcept
    {
        auto it = vertexShaderIndices.find(name);
        if (it != vertexShaderIndices.end())
            return vertexShaders[it->second].get();
        
        return nullptr;
    }

    PixelShader* Device::GetPixelShader(const std::string name) const noexcept
    {
        auto it = pixelShaderIndices.find(name);
        if (it != pixelShaderIndices.end())
            return pixelShaders[it->second].get();

        return nullptr;
    }

    #define AddShaderEnsures(shaders, shaderIndices, name)\
        std::size_t shaderIndex = shaders.size();\
        if (shaderIndex > UINT16_MAX)\
            throw std::logic_error("Could not add shader; Shader limit reached!");\
\
        auto p = shaderIndices.insert({ name, static_cast\
            <std::uint16_t>(shaderIndex) });\
        if (!p.second) return nullptr;

    Shader* Device::CreateVertexShader(const std::string name,
        std::unique_ptr<IShader> shader)
    {
        AddShaderEnsures(vertexShaders, vertexShaderIndices, name);
        vertexShaders.push_back(std::unique_ptr<VertexShader>(
            new VertexShader(device, std::move(shader))));

        return vertexShaders.back().get();
    }

    Shader* Device::CreatePixelShader(const std::string name,
        std::unique_ptr<IShader> shader)
    {
        AddShaderEnsures(pixelShaders, pixelShaderIndices, name);
        pixelShaders.push_back(std::unique_ptr<PixelShader>(
            new PixelShader(device, std::move(shader))));

        return pixelShaders.back().get();
    }

    Geometry* Device::GetGeometry(const std::uint16_t index) const noexcept
    {
        return (index >= geometry.size()) ? nullptr : geometry[index].get();
    }

    Geometry* Device::AddGeometry(std::unique_ptr<Geometry> geom) noexcept
    {
        // TODO: Make this function better or remove it??
        geometry.push_back(std::move(geom));
        return geometry.back().get();
    }
}
