#pragma once
#include <cstddef>
#include <vector>
#include <string>

namespace hl
{
    struct HHMaterialV1;
    struct HHMaterialV3;
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
        std::string VertexShaderName;
        std::string PixelShaderName;
        std::vector<TextureMeta> Textures;
        
        // TODO: Add constructor for V1 materials too
        Material(Instance& inst, const hl::HHMaterialV3& mat);

        void Bind(Instance& inst) const;
        // TODO
    };

    Material* LoadHHMaterial(Instance& inst, const char* filePath);
}
