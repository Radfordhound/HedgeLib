#pragma once
#include <cstddef>
#include <vector>
#include <string>

namespace hl
{
    struct HHMaterial;
}

namespace HedgeEdit::GFX
{
    class Instance;
    class VertexShader;
    class PixelShader;

    struct TextureMeta
    {
        std::string TextureName;
        std::string Type;
    };

    class Material
    {
    public:
        VertexShader* VertexShader;
        PixelShader* PixelShader;
        std::vector<TextureMeta> Textures;
        
        Material(Instance& inst, const hl::HHMaterial& mat);

        void Bind(const Instance& inst) const;
        // TODO
    };

    Material* LoadHHMaterial(Instance& inst, const char* filePath);
}
