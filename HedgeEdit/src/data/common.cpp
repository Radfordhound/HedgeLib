#include "common.h"
#include "../gfx/texture.h"
#include "../gfx/instance.h"

namespace HedgeEdit::Data::Parsers
{
    DEFINE_PARSER(ResTexture)
    {
        inst.AddTexture(name, new GFX::Texture(inst,
            *static_cast<hl::DDSTexture*>(data), size));
    }
}
