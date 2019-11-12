#pragma once
#include "renderTypes.h"
#include "inputLayout.h"
#include "constantBuffer.h"
#include "texture.h"
#include "material.h"
#include "shader.h"
#include "geometry.h"
#include "model.h"

#include "HedgeLib/String.h"

#ifdef D3D11
#include <winrt/base.h>
#include <d3d11.h>
#endif

#include <vector>
#include <unordered_map>
#include <string>
#include <cstddef>
#include <cstdint>

namespace hl
{
    struct HHVertexElement;
    struct HHTerrainModel;
    struct HHSkeletalModel;
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

        std::unordered_map<std::size_t, std::unique_ptr<InputLayout>> inputLayouts;
        std::unordered_map<std::string, std::unique_ptr<ConstantBuffer>> constantBuffers;
        std::unordered_map<std::string, std::size_t> textureIndices;
        std::unordered_map<std::string, std::size_t> materialIndices;
        std::unordered_map<std::string, std::uint16_t> vertexShaderIndices;
        std::unordered_map<std::string, std::uint16_t> pixelShaderIndices;
        std::unordered_map<std::string, std::uint16_t> modelIndices;

        std::vector<std::unique_ptr<Texture>> textures;
        std::vector<std::unique_ptr<Material>> materials;
        std::vector<std::unique_ptr<VertexShader>> vertexShaders;
        std::vector<std::unique_ptr<PixelShader>> pixelShaders;
        std::vector<std::unique_ptr<Geometry>> geometry;
        std::vector<std::unique_ptr<Model>> models;

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

        InputLayout* GetInputLayout(std::size_t hash) const;
        ConstantBuffer* GetConstantBuffer(const std::string& name) const;

        std::size_t GetTextureIndex(const std::string& name) const;
        Texture* GetTexture(const std::string& name) const;

        inline Texture* GetTexture(std::size_t index) const
        {
            return textures[index].get();
        }

        std::size_t GetMaterialIndex(const std::string& name) const;
        Material* GetMaterial(const std::string& name) const;

        inline Material* GetMaterial(std::size_t index) const
        {
            return materials[index].get();
        }

        std::uint16_t GetVertexShaderIndex(const std::string& name) const;
        VertexShader* GetVertexShader(const std::string& name) const;

        inline VertexShader* GetVertexShader(std::uint16_t index) const
        {
            return vertexShaders[index].get();
        }

        std::uint16_t GetPixelShaderIndex(const std::string& name) const;
        PixelShader* GetPixelShader(const std::string& name) const;

        inline PixelShader* GetPixelShader(std::uint16_t index) const
        {
            return pixelShaders[index].get();
        }

        Geometry* GetGeometry(const std::uint16_t index) const; // TODO: inline?

        std::uint16_t GetModelIndex(const std::string& name) const;
        Model* GetModel(const std::string& name) const;

        inline Model* GetModel(std::uint16_t index) const
        {
            return models[index].get();
        }

        inline void AddInputLayout(std::size_t hash, InputLayout* inputLayout)
        {
            inputLayouts.insert({ hash, std::unique_ptr<InputLayout>(inputLayout) });
        }

        void AddConstantBuffer(const std::string& name,
            ConstantBuffer* constantBuffer);

        std::size_t AddTexture(const std::string& name, Texture* texture);
        std::size_t AddMaterial(const std::string& name, Material* material);
        std::uint32_t AddGeometry(std::unique_ptr<Geometry> g);
        std::uint16_t AddModel(const std::string& name, Model* model);
        std::uint16_t AddModel(const hl::HHTerrainModel& model);
        std::uint16_t AddModel(const std::string& name, const hl::HHTerrainModel& model);
        std::uint16_t AddModel(const std::string& name, const hl::HHSkeletalModel& model);

        std::size_t LoadDDSTexture(const char* filePath);
        std::size_t LoadHHSkeletalModel(const char* filePath);
        std::size_t LoadHHTerrainModel(const char* filePath);
        std::size_t LoadHHModel(const char* filePath);
    };
}
