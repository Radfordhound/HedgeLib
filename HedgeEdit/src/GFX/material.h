#pragma once
#include <cstddef>

namespace HedgeEdit::GFX
{
    class Instance;

    class Material
    {
    public:
        std::size_t VertexShaderIndex;
        std::size_t PixelShaderIndex;
        std::size_t TextureIndex;

        void Bind(const Instance& inst) const;
        // TODO
    };
}
