#pragma once
#include "d3d.h"
#include <Geometry/ISubMesh.h>
#include <d3d11.h>
#include <cstdint>
#include <cstddef>

namespace HedgeEdit::GFX
{
    // Forward-declaration for Device
    class Device;

    class Geometry
    {
        ID3D11Buffer* vertexBuffer;
        ID3D11Buffer* indexBuffer;
        std::size_t vertexFormatIndex;
        UINT stride, faceCount;

        void CreateVertexBufferHH2(Device& device,
            const HedgeLib::Geometry::ISubMesh* subMesh);

    public:
        std::uint16_t MaterialIndex;
        bool SeeThrough; // For Solid/Boolean this is false

        void Create(Device& device,
            const HedgeLib::Geometry::ISubMesh* subMesh);

        inline Geometry() = default;
        inline Geometry(Device& device,
            const HedgeLib::Geometry::ISubMesh* subMesh)
        {
            Create(device, subMesh);
        }

        inline ~Geometry() noexcept
        {
            SAFE_RELEASE(vertexBuffer);
            SAFE_RELEASE(indexBuffer);
        }
        
        inline std::size_t VertexFormatIndex() const noexcept
        {
            return vertexFormatIndex;
        }

        void Bind(ID3D11DeviceContext* context) const;
        void Draw(ID3D11DeviceContext* context) const;
    };
}
