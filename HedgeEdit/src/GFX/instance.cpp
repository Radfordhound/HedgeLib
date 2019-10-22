#include "instance.h"
#include "dxc_default_vs.h"
#include "dxc_default_ps.h"
#include "HedgeLib/Geometry/HHSubMesh.h"
#include "HedgeLib/Geometry/HHModel.h"
#include "HedgeLib/IO/Path.h"
#include <stdexcept>

namespace HedgeEdit::GFX
{
    Instance MainInstance = Instance();

    void Instance::InitStandard()
    {
#ifdef D3D11
        // Initialize default shaders
        std::size_t indices = 0;
        defaultVS = std::unique_ptr<VertexShaderVariant>(new VertexShaderVariant(
            *this, dxc_default_vs, sizeof(dxc_default_vs),
            dxc_default_vs, sizeof(dxc_default_vs),
            std::make_unique<std::uint32_t[]>(1), 1));

        defaultPS = std::unique_ptr<PixelShaderVariant>(new PixelShaderVariant(
            *this, dxc_default_ps, sizeof(dxc_default_ps),
            std::make_unique<std::uint32_t[]>(1), 1));

        // Initialize cbWorld
        ConstantBuffer* cb = new ConstantBuffer(*this, 64,
        {
            { "viewProj", { 0, 64 } }
        }, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

        constantBuffers.insert({ "cbWorld", std::unique_ptr<ConstantBuffer>(cb) });

        // Initialize cbInstance
        cb = new ConstantBuffer(*this, 64,
        {
            { "world", { 0, 64 } }
        }, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

        constantBuffers.insert({ "cbInstance", std::unique_ptr<ConstantBuffer>(cb) });

        // Initialize samplers
        D3D11_SAMPLER_DESC sdesc =
        {
            D3D11_FILTER_MIN_MAG_MIP_LINEAR,
            D3D11_TEXTURE_ADDRESS_WRAP,
            D3D11_TEXTURE_ADDRESS_WRAP,
            D3D11_TEXTURE_ADDRESS_WRAP,
            0, 0, D3D11_COMPARISON_NEVER,
            { 0, 0, 0, 0 },
            0, D3D11_FLOAT32_MAX
        };

        samplers = winrt::com_array<winrt::com_ptr<ID3D11SamplerState>>(1);
        Device->CreateSamplerState(&sdesc, samplers[0].put()); // TODO: Check result
#endif
    }

    void Instance::InitHH2()
    {
        // TODO
    }

    Instance::Instance(RenderTypes renderType) : renderType(renderType)
    {
#ifdef D3D11
        // Create a D3D Device and Context
        // TODO: Let user specify which video adapter to use
        HRESULT result;
        UINT flags = 0;

#ifdef DEBUG
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE,
            nullptr, flags, nullptr, 0, D3D11_SDK_VERSION,
            Device.put(), nullptr, Context.put());

        if (FAILED(result))
            throw std::runtime_error("Could not create a Direct3D 11 Device!");

        // Create a DXGI Factory
        // TODO: Use DXGI 1.2?
        // TODO: Fallback to DXGI 1.0 if 1.1 is not supported?
        result = CreateDXGIFactory1(__uuidof(IDXGIFactory1),
            (void**)(&Factory));

        if (FAILED(result))
        {
            throw std::runtime_error("Could not create a DXGI 1.1 Factory!");
        }

        // Create Rasterizer State
        D3D11_RASTERIZER_DESC rd = {};
        rd.FillMode = D3D11_FILL_SOLID;
        rd.CullMode = D3D11_CULL_BACK;
        rd.FrontCounterClockwise = false;
        rd.DepthClipEnable = true;
        rd.MultisampleEnable = true;
        rd.AntialiasedLineEnable = true;

        result = Device->CreateRasterizerState(&rd, RasterizerState.put());

        if (FAILED(result))
        {
            throw std::runtime_error(
                "Could not create Direct3D 11 Rasterizer State!");
        }

        Context->RSSetState(RasterizerState.get());
#endif

        // Initialize resources
        switch (renderType)
        {
        case RenderTypes::Standard:
            InitStandard();
            break;

        case RenderTypes::HedgehogEngine2:
            InitHH2();
            break;
        }
    }

    std::size_t Instance::HashVertexElement(const hl_HHVertexElement& e)
    {
        // Slight modification of The Bernstein Hash based on this SO answer:
        // https://stackoverflow.com/a/34006336
        const std::size_t seed = 1009, factor = 9176;
        std::size_t hash = seed;

        hash = (hash * factor) + std::hash<std::uint32_t>()(e.Offset);
        hash = (hash * factor) + std::hash<std::uint32_t>()(
            static_cast<std::uint32_t>(e.Format));

        hash = (hash * factor) + std::hash<std::uint16_t>()(e.Type);
        hash = (hash * factor) + std::hash<std::uint8_t>()(e.Index);

        return hash;
    }

    std::size_t Instance::HashVertexFormat(const hl_HHVertexElement* f)
    {
        // Slight modification of The Bernstein Hash based on this SO answer:
        // https://stackoverflow.com/a/34006336
        const std::size_t seed = 1009, factor = 9176;
        std::size_t hash = seed;

        while (f->Format != HL_HHVERTEX_FORMAT_LAST_ENTRY)
        {
            hash = (hash * factor) + HashVertexElement(*f);
            ++f;
        }

        return hash;
    }

    void Instance::VSUseSampler(std::uint32_t index, unsigned int slot)
    {
        ID3D11SamplerState* s = samplers[index].get();
        Context->VSSetSamplers(slot, 1, &s);
    }

    void Instance::PSUseSampler(std::uint32_t index, unsigned int slot)
    {
        ID3D11SamplerState* s = samplers[index].get();
        Context->PSSetSamplers(slot, 1, &s);
    }

    void Instance::Clear()
    {
        // Release input layouts
        inputLayouts.clear();

        // Clear remaining loaded resources
        // TODO
    }

    InputLayout* Instance::GetInputLayout(std::size_t hash) const
    {
        auto v = inputLayouts.find(hash);
        if (v == inputLayouts.end()) return nullptr;
        return v->second.get();
    }

    ConstantBuffer* Instance::GetConstantBuffer(const std::string& name) const
    {
        auto it = constantBuffers.find(name);
        if (it == constantBuffers.end()) return nullptr;
        return it->second.get();
    }

    std::size_t Instance::GetTextureIndex(const std::string& name) const
    {
        auto it = textureIndices.find(name);
        if (it != textureIndices.end()) return it->second;
        throw std::runtime_error("No texture was present with the given name.");
    }

    Texture* Instance::GetTexture(const std::string& name) const
    {
        auto it = textureIndices.find(name);
        if (it == textureIndices.end()) return nullptr;
        return textures[it->second].get();
    }

    std::size_t Instance::GetMaterialIndex(const std::string& name) const
    {
        auto it = materialIndices.find(name);
        if (it != materialIndices.end()) return it->second;
        throw std::runtime_error("No material was present with the given name.");
    }

    Material* Instance::GetMaterial(const std::string& name) const
    {
        auto it = materialIndices.find(name);
        if (it == materialIndices.end()) return nullptr;
        return materials[it->second].get();
    }

    std::uint16_t Instance::GetVertexShaderIndex(const std::string& name) const
    {
        auto it = vertexShaderIndices.find(name);
        if (it != vertexShaderIndices.end()) return it->second;
        throw std::runtime_error("No vertex shader was present with the given name.");
    }

    VertexShader* Instance::GetVertexShader(const std::string& name) const
    {
        auto it = vertexShaderIndices.find(name);
        if (it == vertexShaderIndices.end()) return nullptr;
        return vertexShaders[it->second].get();
    }

    std::uint16_t Instance::GetPixelShaderIndex(const std::string& name) const
    {
        auto it = pixelShaderIndices.find(name);
        if (it != pixelShaderIndices.end()) return it->second;
        throw std::runtime_error("No pixel shader was present with the given name.");
    }

    PixelShader* Instance::GetPixelShader(const std::string& name) const
    {
        auto it = pixelShaderIndices.find(name);
        if (it == pixelShaderIndices.end()) return nullptr;
        return pixelShaders[it->second].get();
    }

    Geometry* Instance::GetGeometry(const std::uint16_t index) const
    {
        return (index >= geometry.size()) ? nullptr : geometry[index].get();
    }

    std::uint16_t Instance::GetModelIndex(const std::string& name) const
    {
        auto it = modelIndices.find(name);
        if (it != modelIndices.end()) return it->second;
        throw std::runtime_error("No model was present with the given name.");
    }

    Model* Instance::GetModel(const std::string& name) const
    {
        auto it = modelIndices.find(name);
        if (it == modelIndices.end()) return nullptr;
        return models[it->second].get();
    }

    void Instance::AddConstantBuffer(const std::string& name,
        ConstantBuffer* constantBuffer)
    {
        constantBuffers.insert({ name, std::unique_ptr<
            ConstantBuffer>(constantBuffer) });
    }

    std::size_t Instance::AddTexture(const std::string& name, Texture* texture)
    {
        // Add the texture to the vector
        std::size_t index = textures.size();
        textures.push_back(std::unique_ptr<Texture>(texture));
        textureIndices.insert({ name, index });
        return index;
    }

    std::size_t Instance::AddMaterial(const std::string& name, Material* material)
    {
        // Add the material to the vector
        std::size_t index = materials.size();
        materials.push_back(std::unique_ptr<Material>(material));
        materialIndices.insert({ name, index });
        return index;
    }

    std::uint32_t Instance::AddGeometry(std::unique_ptr<Geometry> g)
    {
        // Ensure we haven't reached the geometry limit yet
        std::size_t index = geometry.size();
        if (index > 0xFFFFFFFF)
        {
            throw std::runtime_error(
                "The maximum number of geometries has been reached!");
        }

        // Add the geometry
        geometry.emplace_back(std::move(g));
        return static_cast<std::uint32_t>(index);
    }

    std::uint16_t Instance::AddModel(const std::string& name, Model* model)
    {
        // Ensure we haven't reached the model limit yet
        std::size_t index = models.size();
        if (index > 0xFFFF)
        {
            throw std::runtime_error(
                "The maximum number of models has been reached!");
        }

        // Add the model to the vector
        models.push_back(std::unique_ptr<Model>(model));
        modelIndices.insert({ name, static_cast<std::uint16_t>(index) });
        return static_cast<std::uint16_t>(index);
    }

    std::uint16_t Instance::AddModel(const hl_HHTerrainModel& model)
    {
        return AddModel(model.Name.Get(), new Model(*this, model));
    }

    std::uint16_t Instance::AddModel(const std::string& name,
        const hl_HHTerrainModel& model)
    {
        return AddModel(name, new Model(*this, model));
    }

    std::uint16_t Instance::AddModel(const std::string& name,
        const hl_HHSkeletalModel& model)
    {
        return AddModel(name, new Model(*this, model));
    }

    std::size_t Instance::LoadDDSTexture(const char* filePath)
    {
        return AddTexture(filePath, GFX::LoadDDSTexture(*this, filePath));
    }

    std::size_t Instance::LoadHHSkeletalModel(const char* filePath)
    {
        return AddModel(filePath, GFX::LoadHHSkeletalModel(*this, filePath));
    }

    std::size_t Instance::LoadHHTerrainModel(const char* filePath)
    {
        return AddModel(filePath, GFX::LoadHHTerrainModel(*this, filePath));
    }

    std::size_t Instance::LoadHHModel(const char* filePath)
    {
        const char* ext = hl_PathGetExtPtr(filePath);
        if (!strcmp(ext, ".terrain-model"))
        {
            return LoadHHTerrainModel(filePath);
        }
        else
        {
            return LoadHHSkeletalModel(filePath);
        }
    }
}
