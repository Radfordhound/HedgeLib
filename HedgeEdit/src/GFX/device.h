#pragma once
#include "renderMode.h"
#include "shader.h"
#include "geometry.h"
#include <d3d11.h>
#include <dxgi.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <optional>

// Microsoft hasn't yet updated wrl for proper C++17 conformity, so when we
// #include wrl.h one of the headers that gets included (corewrappers.h) logs
// warning C5043. Here we forward-declare ComPtr rather than including wrl.h
// (we'll do that in device.cpp) to avoid this warning showing several times.
namespace Microsoft::WRL
{
    template <typename T>
    class ComPtr;
}

namespace HedgeLib::Geometry
{
    struct VertexElement;
}

namespace HedgeEdit::GFX
{
    extern const UINT VertexBufferOffset, FallbackBufferStride;

    class Device
    {
        ID3D11Device* device;
        ID3D11DeviceContext* context;
        IDXGIFactory1* factory;
        ID3D11RasterizerState* rs;
        ID3D11Buffer* fallbackBuffer;

        RenderMode renderMode;

        struct VertexFormatHasher
        {
            std::size_t operator()(const std::pair<
                const HedgeLib::Geometry::VertexElement*,
                const std::size_t>& key) const noexcept;
        };

        std::unordered_map<std::pair<
            const HedgeLib::Geometry::VertexElement*,
            const std::size_t>, std::size_t,
            VertexFormatHasher> vertexFormatIndices;

        std::vector<std::unique_ptr<std::size_t[]>> inputLayoutIndices;

        std::unordered_map<std::string, std::uint16_t> vertexShaderIndices;
        std::unordered_map<std::string, std::uint16_t> pixelShaderIndices;

        std::vector<Microsoft::WRL::ComPtr<ID3D11InputLayout>> inputLayouts;
        std::vector<std::unique_ptr<VertexShader>> vertexShaders;
        std::vector<std::unique_ptr<PixelShader>> pixelShaders;
        std::vector<std::unique_ptr<Geometry>> geometry;

        std::size_t CreateInputLayoutHH2(const HedgeLib::Geometry::VertexElement* elements,
            const std::size_t elementCount, const VertexShader& shader,
            const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputElements,
            const std::size_t flags, const std::size_t shaderFlags);

    public:
        // 16 nulls for data required by the shader that's not in the vertex buffer
        constexpr static UINT FallbackBufferSlot = 0;
        constexpr static UINT FallbackBufferSize = (sizeof(float) * 4);

        // Vertex buffer
        constexpr static UINT VertexBufferSlot = 1;

        // VSVs
        constexpr static std::size_t StandardVSVCount = 1;
        constexpr static std::size_t HH2VSVCount = 4;

        enum VertexShaderVariantFlag : std::uint32_t
        {
            VSV_FLAG_DEFAULT                    = 0,
            VSV_FLAG_HH2_MAX_BONE_INFLUENCES    = 1,
            VSV_FLAG_HH2_MULTI_TANGENT_SPACE    = 2
        };

        Device(const GFX::RenderMode renderMode = RENDER_MODE_STANDARD);
        ~Device() noexcept;

        inline ID3D11Device* D3DDevice() const noexcept
        {
            return device;
        }

        inline ID3D11DeviceContext* D3DContext() const noexcept
        {
            return context;
        }

        inline IDXGIFactory1* D3DFactory() const noexcept
        {
            return factory;
        }

        inline ID3D11RasterizerState* D3DRasterizerState() const noexcept
        {
            return rs;
        }

        inline GFX::RenderMode RenderMode() const noexcept
        {
            return renderMode;
        }

        inline std::size_t VSVCount() const noexcept
        {
            switch (renderMode)
            {
            case RENDER_MODE_HH2:
                return HH2VSVCount;

            default:
                return StandardVSVCount;
            }
        }

        inline void BindFallbackBuffer() const noexcept
        {
            context->IASetVertexBuffers(FallbackBufferSlot, 1,
                &fallbackBuffer, &FallbackBufferStride, &VertexBufferOffset);
        }

        void Reset(const GFX::RenderMode renderMode = RENDER_MODE_STANDARD) noexcept;

        ID3D11InputLayout* GetInputLayout(const std::size_t index) const noexcept;
        std::optional<std::size_t> GetVertexFormatIndex(
            const HedgeLib::Geometry::VertexElement* elements,
            const std::size_t elementCount) const noexcept;

        const std::size_t* GetInputLayoutIndices(
            const HedgeLib::Geometry::VertexElement* elements,
            const std::size_t elementCount) const noexcept;

        const std::size_t* GetInputLayoutIndices(
            const std::size_t vertexFormatIndex) const noexcept;

        std::size_t CreateInputLayouts(
            const HedgeLib::Geometry::VertexElement* elements,
            const std::size_t elementCount, const VertexShader& shader);

        inline VertexShader* GetVertexShader(
            const std::uint16_t index) const noexcept
        {
            return (index >= vertexShaders.size()) ?
                nullptr : vertexShaders[index].get();
        }

        VertexShader* GetVertexShader(const std::string name) const noexcept; // TODO: Use string_view instead?

        inline PixelShader* GetPixelShader(
            const std::uint16_t index) const noexcept
        {
            return (index >= pixelShaders.size()) ?
                nullptr : pixelShaders[index].get();
        }

        PixelShader* GetPixelShader(const std::string name) const noexcept; // TODO: Use string_view instead?

        Shader* CreateVertexShader(const std::string name,
            std::unique_ptr<HedgeLib::Shaders::IShader> shader);

        Shader* CreatePixelShader(const std::string name,
            std::unique_ptr<HedgeLib::Shaders::IShader> shader);

        Geometry* GetGeometry(const std::uint16_t index) const noexcept; // TODO: inline?
        Geometry* AddGeometry(std::unique_ptr<Geometry> geom) noexcept;
    };
}
