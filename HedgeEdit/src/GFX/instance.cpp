#include "instance.h"
#include "dxc_default_vs.h"
#include "dxc_default_ps.h"
#include "HedgeLib/Geometry/HHMesh.h"
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

        // Create default terrain group
        terrainGroups.emplace_back();

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

    std::size_t Instance::HashVertexElement(const hl::HHVertexElement& e)
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

    std::size_t Instance::HashVertexFormat(const hl::HHVertexElement* f)
    {
        // Slight modification of The Bernstein Hash based on this SO answer:
        // https://stackoverflow.com/a/34006336
        const std::size_t seed = 1009, factor = 9176;
        std::size_t hash = seed;

        while (f->Format != hl::HHVERTEX_FORMAT_LAST_ENTRY)
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
        textures.clear();
        materials.clear();
        models.clear();

        terrainGroups.clear();
        terrainGroups.emplace_back();
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

    Texture* Instance::GetTexture(const std::string& name) const
    {
        auto it = textures.find(name);
        if (it == textures.end()) return nullptr;
        return it->second.get();
    }

    Material* Instance::GetMaterial(const std::string& name) const
    {
        auto it = materials.find(name);
        if (it == materials.end()) return nullptr;
        return it->second.get();
    }

    VertexShader* Instance::GetVertexShader(const std::string& name) const
    {
        auto it = vertexShaders.find(name);
        if (it == vertexShaders.end()) return nullptr;
        return it->second.get();
    }

    PixelShader* Instance::GetPixelShader(const std::string& name) const
    {
        auto it = pixelShaders.find(name);
        if (it == pixelShaders.end()) return nullptr;
        return it->second.get();
    }

    Model* Instance::GetModel(const std::string& name) const
    {
        auto it = models.find(name);
        if (it == models.end()) return nullptr;
        return it->second.get();
    }

    void Instance::AddConstantBuffer(const std::string& name,
        ConstantBuffer* constantBuffer)
    {
        constantBuffers.insert({ name, std::unique_ptr<
            ConstantBuffer>(constantBuffer) });
    }

    Texture& Instance::AddTexture(const std::string& name, Texture* texture)
    {
        // Add the texture to the hash table
        return *textures.insert({ name, std::unique_ptr<Texture>(texture) }).first->second.get();
    }

    Material& Instance::AddMaterial(const std::string& name, Material* material)
    {
        // Add the material to the hash table
        return *materials.insert({ name, std::unique_ptr<Material>(material) }).first->second.get();
    }

    Model& Instance::AddModel(const std::string& name, Model* model)
    {
        return *models.insert({ name, std::unique_ptr<Model>(model) }).first->second.get();
    }

    Model& Instance::AddModel(const hl::HHTerrainModel& model)
    {
        return AddModel(model.Name.Get(), new Model(*this, model));
    }

    Model& Instance::AddModel(const std::string& name,
        const hl::HHTerrainModel& model)
    {
        return AddModel(name, new Model(*this, model));
    }

    Model& Instance::AddModel(const std::string& name,
        const hl::HHSkeletalModel& model)
    {
        return AddModel(name, new Model(*this, model));
    }

    TerrainGroup* Instance::GetTerrainGroup(const char* name)
    {
        if (!name) return nullptr;
        for (std::size_t i = 0; i < terrainGroups.size(); ++i)
        {
            if (terrainGroups[i].Name == name)
            {
                return &terrainGroups[i];
            }
        }
        
        return nullptr;
    }

    Texture& Instance::LoadDDSTexture(const char* filePath)
    {
        return AddTexture(hl::PathGetNameNoExtPtr(filePath).get(),
            GFX::LoadDDSTexture(*this, filePath));
    }

    Model& Instance::LoadHHSkeletalModel(const char* filePath)
    {
        return AddModel(hl::PathGetNameNoExtPtr(filePath).get(),
            GFX::LoadHHSkeletalModel(*this, filePath));
    }

    Model& Instance::LoadHHTerrainModel(const char* filePath)
    {
        return AddModel(hl::PathGetNameNoExtPtr(filePath).get(),
            GFX::LoadHHTerrainModel(*this, filePath));
    }

    Model& Instance::LoadHHModel(const char* filePath)
    {
        const char* ext = hl::PathGetExtPtr(filePath);
        if (!std::strcmp(ext, ".terrain-model"))
        {
            return LoadHHTerrainModel(filePath);
        }
        else
        {
            return LoadHHSkeletalModel(filePath);
        }
    }

    void Instance::BeginFrameStandard(const Matrix4x4& viewProj) const
    {
        // Update cbWorld
        ConstantBuffer* cb = GetConstantBuffer("cbWorld");
        cb->Map(Context.get(), D3D11_MAP_WRITE_DISCARD);
        cb->SetValue("viewProj", &viewProj);
        cb->Unmap(Context.get());

        // Use cbWorld
        cb->UseVS(*this, 0);
        cb->UsePS(*this, 0);

        // Use cbInstance
        cb = GetConstantBuffer("cbInstance");
        cb->UseVS(*this, 1);
        cb->UsePS(*this, 1);
    }

    void Instance::BeginFrameHH2(const Matrix4x4& viewProj) const
    {
        // TODO
    }

    void Instance::BindTransformStandard(const Transform& transform)
    {
        // Update cbInstance
        ConstantBuffer* cbInstance = GetConstantBuffer("cbInstance");
        cbInstance->Map(Context.get(), D3D11_MAP_WRITE_DISCARD);
        cbInstance->SetValue("world", &transform.GetMatrix());
        cbInstance->Unmap(Context.get());
    }

    void Instance::BindTransformHH2(const Transform& transform)
    {
        // TODO
    }

    void Instance::DrawMeshGroup(const MeshGroup& meshGroup)
    {
        for (std::size_t i = 0; i < meshGroup.MeshCount; ++i)
        {
            const Geometry& geometry = meshGroup.Meshes[i];
            geometry.Bind(*this);
            geometry.Draw(*this);
        }
    }

    void Instance::DrawStage()
    {
        Context->IASetPrimitiveTopology(
            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        // TODO: Bind shaders properly in material bind function
        VertexShaderVariant* vs = GetDefaultVS();
        vs->Use(*this);

        PixelShaderVariant* ps = GetDefaultPS();
        ps->Use(*this);

        // Render first pass (solid)
        Transform defaultTransform = Transform();
        for (const auto& terrainGroup : terrainGroups)
        {
            for (const auto& terrain : terrainGroup.Terrain)
            {
                if (terrain.ModelName.empty()) continue;
                GFX::Model* trrMdl = GetModel(terrain.ModelName);

                if (terrain.Instances.empty())
                {
                    BindTransform(defaultTransform);
                    DrawMeshGroup(trrMdl->Solid);
                    DrawMeshGroup(trrMdl->Boolean);
                }
                else
                {
                    for (const auto& transform : terrain.Instances)
                    {
                        BindTransform(transform);
                        DrawMeshGroup(trrMdl->Solid);
                        DrawMeshGroup(trrMdl->Boolean);
                    }
                }
            }
        }

        // Render second pass (transparent)
        // TODO: Do proper depth-sorting
        for (const auto& terrainGroup : terrainGroups)
        {
            for (const auto& terrain : terrainGroup.Terrain)
            {
                if (terrain.ModelName.empty()) continue;

                GFX::Model* trrMdl = GetModel(terrain.ModelName);
                if (!trrMdl->Transparent.MeshCount) continue;

                if (terrain.Instances.empty())
                {
                    BindTransform(defaultTransform);
                    DrawMeshGroup(trrMdl->Transparent);
                }
                else
                {
                    for (const auto& transform : terrain.Instances)
                    {
                        BindTransform(transform);
                        DrawMeshGroup(trrMdl->Transparent);
                    }
                }
            }
        }

        // TODO: Special passes
    }
}
