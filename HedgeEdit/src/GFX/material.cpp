#include "material.h"
#include "texture.h"
#include "instance.h"

namespace HedgeEdit::GFX
{
    void Material::Bind(const Instance& inst) const
    {
        // Get texture
        // TODO: Do this correctly
        const Texture* tex = inst.GetTexture(TextureIndex);
        // TODO: nullptr check
        tex->Bind(inst, 0);

        // TODO: Set material parameters
    }
}
