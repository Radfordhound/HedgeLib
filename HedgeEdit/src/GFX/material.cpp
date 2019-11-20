#include "material.h"
#include "texture.h"
#include "instance.h"
#include "HedgeLib/Blob.h"
#include "HedgeLib/IO/HedgehogEngine.h"
#include "HedgeLib/Materials/HHMaterial.h"

namespace HedgeEdit::GFX
{
    Material::Material(Instance& inst, const hl::HHMaterial& mat)
    {
        // Get shaders
        VertexShader = inst.GetVertexShader(mat.Shader.Get());
        PixelShader = inst.GetPixelShader(mat.Shader.Get());
        
        // Generate texture metadata
        const hl::DataOffset32<hl::HHTexture>* textures = mat.Textures.Get();
        Textures.resize(static_cast<std::size_t>(mat.TextureCount));

        for (std::uint8_t i = 0; i < mat.TextureCount; ++i)
        {
            Textures[i].TextureName = textures[i]->TextureName;
            Textures[i].Type = textures[i]->Type;
        }
    }

    void Material::Bind(const Instance& inst) const
    {
        // Get texture
        // TODO: Do this correctly
        const Texture* tex = inst.GetTexture(Textures[0].TextureName);
        // TODO: nullptr check
        tex->Bind(inst, 0);

        // TODO: Set material parameters
    }

    Material* LoadHHMaterial(Instance& inst, const char* filePath)
    {
        // Open the file
        hl::Blob blob = hl::DHHLoad(filePath);

        // Load and endian-swap the data
        hl::HHMaterial* mat = hl::DHHGetData<hl::HHMaterial>(blob.RawData());
        mat->EndianSwapRecursive(true);

        // Generate a HedgeEdit material from it and return
        return new Material(inst, *mat);
    }
}
