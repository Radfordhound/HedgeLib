#pragma once
#ifdef D3D11
#include <winrt/base.h>
#include <d3d11.h>
#endif

#include <cstdint>
#include <cstddef>
#include <string>

namespace hl
{
    struct HHMesh;
}

namespace HedgeEdit::GFX
{
    class Instance;
    class Material;

    // Geometry:
    // - VertexBuffer* VertexBuffer;
    // - IndexBuffer* IndexBuffer;
    // - size_t InputLayoutIndex;
    // - ushort MaterialIndex;
    // - bool SeeThrough; // For Solid/Boolean this is false
    class Geometry
    {
        std::size_t vertexFormatHash;

#ifdef D3D11
        winrt::com_ptr<ID3D11Buffer> vertexBuffer;
        winrt::com_ptr<ID3D11Buffer> indexBuffer;
#endif

        unsigned int stride, faceCount;

    public:
        std::string MaterialName;

        inline Geometry() = default;
        Geometry(Instance& inst, const hl::HHMesh& mesh);
        
        inline std::size_t VertexFormatHash() const noexcept
        {
            return vertexFormatHash;
        }

        void Bind(Instance& inst) const;
        void Draw(const Instance& inst) const;
    };
}
