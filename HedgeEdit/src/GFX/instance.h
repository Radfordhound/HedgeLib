#pragma once
#include "renderTypes.h"
#include "inputLayout.h"
#include "constantBuffer.h"
#include "texture.h"
#include "material.h"
#include "shader.h"
#include "geometry.h"
#include "model.h"
#include "../flat_hash_map.h"

#include "HedgeLib/String.h"

#ifdef D3D11
#include <winrt/base.h>
#include <d3d11.h>
#endif

#include <vector>
#include <string>
#include <cstddef>
#include <cstdint>

namespace hl
{
    struct HHVertexElement;
    struct HHTerrainModel;
    struct HHSkeletalModel;
    struct HHTerrainInstanceInfoV0;
}

namespace HedgeEdit::GFX
{
    extern class Instance MainInstance;

    class Instance
    {
        RenderTypes renderType;
        std::unique_ptr<VertexShaderVariant> defaultVS;
        std::unique_ptr<PixelShaderVariant> defaultPS;

#ifdef D3D11
        winrt::com_array<winrt::com_ptr<ID3D11SamplerState>> samplers;
#endif

        ska::flat_hash_map<std::size_t, std::unique_ptr<InputLayout>> inputLayouts;
        ska::flat_hash_map<std::string, std::unique_ptr<ConstantBuffer>> constantBuffers;
        ska::flat_hash_map<std::string, std::unique_ptr<Texture>> textures;
        ska::flat_hash_map<std::string, std::unique_ptr<Material>> materials;
        ska::flat_hash_map<std::string, std::unique_ptr<VertexShader>> vertexShaders;
        ska::flat_hash_map<std::string, std::unique_ptr<PixelShader>> pixelShaders;
        ska::flat_hash_map<std::string, std::unique_ptr<Model>> models;

        void InitStandard();
        void InitHH2();

    public:
#ifdef D3D11
        winrt::com_ptr<ID3D11Device> Device;
        winrt::com_ptr<ID3D11DeviceContext> Context;
        winrt::com_ptr<IDXGIFactory1> Factory;
        winrt::com_ptr<ID3D11RasterizerState> RasterizerState;
#endif

        Instance(RenderTypes renderType = RenderTypes::Standard);

        static std::size_t HashVertexElement(const hl::HHVertexElement& e);
        static std::size_t HashVertexFormat(const hl::HHVertexElement* f);

        inline RenderTypes RenderType() const
        {
            return renderType;
        }

        inline VertexShaderVariant* GetDefaultVS() const
        {
            return defaultVS.get();
        }

        inline PixelShaderVariant* GetDefaultPS() const
        {
            return defaultPS.get();
        }

        void VSUseSampler(std::uint32_t index, unsigned int slot);
        void PSUseSampler(std::uint32_t index, unsigned int slot);

        void Clear();

        inline std::size_t GetModelCount()
        {
            return models.size();
        }

        InputLayout* GetInputLayout(std::size_t hash) const;
        ConstantBuffer* GetConstantBuffer(const std::string& name) const;

        Texture* GetTexture(const std::string& name) const;
        Material* GetMaterial(const std::string& name) const;
        VertexShader* GetVertexShader(const std::string& name) const;
        PixelShader* GetPixelShader(const std::string& name) const;
        Model* GetModel(const std::string& name) const;

        inline void AddInputLayout(std::size_t hash, InputLayout* inputLayout)
        {
            inputLayouts.insert({ hash, std::unique_ptr<InputLayout>(inputLayout) });
        }

        void AddConstantBuffer(const std::string& name,
            ConstantBuffer* constantBuffer);

        Texture& AddTexture(const std::string& name, Texture* texture);
        Material& AddMaterial(const std::string& name, Material* material);
        Model& AddModel(const std::string& name, Model* model);
        Model& AddModel(const hl::HHTerrainModel& model);
        Model& AddModel(const std::string& name, const hl::HHTerrainModel& model);
        Model& AddModel(const std::string& name, const hl::HHSkeletalModel& model);

        Texture& LoadDDSTexture(const char* filePath);
        Model& LoadHHSkeletalModel(const char* filePath);
        Model& LoadHHTerrainModel(const char* filePath);
        Model& LoadHHModel(const char* filePath);

        void BeginFrameStandard(const Matrix4x4& viewProj) const;
        void BeginFrameHH2(const Matrix4x4& viewProj) const;

        inline void BeginFrame(const Matrix4x4& viewProj)
        {
            switch (renderType)
            {
            case RenderTypes::HedgehogEngine2:
                BeginFrameHH2(viewProj);
                break;

            default:
                BeginFrameStandard(viewProj);
                break;
            }
        }

        void BindTransformStandard(const Transform& transform);
        void BindTransformHH2(const Transform& transform);

        inline void BindTransform(const Transform& transform)
        {
            switch (renderType)
            {
            case RenderTypes::HedgehogEngine2:
                BindTransformHH2(transform);
                break;

            default:
                BindTransformStandard(transform);
                break;
            }
        }

        void DrawMeshGroup(const MeshGroup& meshGroup);
        void DrawStage();
    };
}
