#include "hedgelib/effects/hl_grif.h"
#include "hedgelib/io/hl_bina.h"
#include <cstring>

namespace hl
{
namespace grif
{
template<typename T>
void in_particle_swap_recursive(T& particle)
{
    // Swap particle
    hl::endian_swap<false>(particle);

    // Swap animations.
    // TODO

    // Swap material.
    if (particle.material.get())
    {
        hl::endian_swap<false>(*particle.material);
    }

    // Swap next particle (if any).
    if (particle.nextParticle.get())
    {
        in_particle_swap_recursive(*particle.nextParticle);
    }
}

template<typename T>
void in_emitter_swap_recursive(T& emitter)
{
    // Swap emitter
    hl::endian_swap<false>(emitter);

    // Swap animations.
    // TODO

    // Swap particles recursively.
    if (emitter.particle.get())
    {
        in_particle_swap_recursive(*emitter.particle);
    }

    // Swap next emitter (if any).
    if (emitter.nextEmitter.get())
    {
        in_emitter_swap_recursive(*emitter.nextEmitter);
    }
}

template<typename T>
void in_effect_swap_recursive(T& effect)
{
    // Swap effect.
    hl::endian_swap<false>(effect);

    // Swap animations.
    // TODO

    // Swap emitters recursively.
    if (effect.emitter.get())
    {
        in_emitter_swap_recursive(*effect.emitter);
    }

    // Swap next effect (if any).
    if (effect.nextEffect.get())
    {
        in_effect_swap_recursive(*effect.nextEffect);
    }
}

void effect::fix(bina::endian_flag endianFlag)
{
    // Swap GRIF effect data if necessary.
    if (bina::needs_swap(endianFlag))
    {
        // Swap header.
        endian_swap<false>();

        // Recursively fix parameters.
        if (major_version() == 1)
        {
            switch (minor_version())
            {
            case 1:
                in_effect_swap_recursive(*effects_v1_1());
                break;

            case 2:
                in_effect_swap_recursive(*effects_v1_2());
                break;

            case 3:
                in_effect_swap_recursive(*effects_v1_3());
                break;

            case 4:
                in_effect_swap_recursive(*effects_v1_4());
                break;

            case 5:
                in_effect_swap_recursive(*effects_v1_5());
                break;

            case 6:
                in_effect_swap_recursive(*effects_v1_6());
                break;

            default:
                HL_ERROR(error_type::unsupported);
            }
        }
    }
}
} // grif
} // hl
