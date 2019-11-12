#pragma once
#ifdef D3D11
#include <winrt/base.h>
#include <d3d11.h>
#endif

#include <cstdint>
#include <cstddef>

namespace hl
{
    struct HHSubMesh;
}

namespace HedgeEdit::GFX
{
    class Instance;

    // Geometry:
    // - VertexBuffer* VertexBuffer;
    // - IndexBuffer* IndexBuffer;
    // - size_t InputLayoutIndex;
    // - ushort MaterialIndex;
    // - bool SeeThrough; // For Solid/Boolean this is false
    class Geometry
    {
#ifdef D3D11
        winrt::com_ptr<ID3D11Buffer> vertexBuffer;
        winrt::com_ptr<ID3D11Buffer> indexBuffer;
#endif

        std::size_t vertexFormatHash;
        unsigned int stride, faceCount;

    public:
        std::size_t MaterialIndex;
        bool SeeThrough; // For Solid/Boolean this is false

        inline Geometry() = default;
        Geometry(Instance& inst, const hl::HHSubMesh& subMesh,
            bool seeThrough = false);
        
        inline std::size_t VertexFormatHash() const noexcept
        {
            return vertexFormatHash;
        }

        void Bind(const Instance& inst) const;
        void Draw(const Instance& inst) const;
    };
}
