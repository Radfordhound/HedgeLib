#include "hedgelib/shader/hl_hh_shader.h"
#include <cstring>

namespace hl
{
namespace hh
{
namespace needle
{
namespace shader
{
namespace v2
{
const code_container* variant::input_bytecode() const noexcept
{
    const code_container* container = bytecode();
    return ptradd<code_container>(container->data(), container->size);
}

code_container* variant::input_bytecode() noexcept
{
    code_container* container = bytecode();
    return ptradd<code_container>(container->data(), container->size);
}

const typed_params_container* variant::typed_params() const noexcept
{
    const code_container* inputBytecode = input_bytecode();
    return ptradd<typed_params_container>(inputBytecode->data(), inputBytecode->size);
}

typed_params_container* variant::typed_params() noexcept
{
    code_container* inputBytecode = input_bytecode();
    return ptradd<typed_params_container>(inputBytecode->data(), inputBytecode->size);
}

const permutation* permutation::next() const noexcept
{
    // Skip to the end of the name.
    const char* namePtr = name();
    while (*namePtr != '\0')
    {
        ++namePtr;
    }

    // Skip past any padding at the end of the name.
    while (*(++namePtr) == '\0') {}
    
    // Return the pointer.
    return reinterpret_cast<const permutation*>(namePtr);
}

void header::fix()
{
#ifndef HL_IS_BIG_ENDIAN
    // Endian-swap header.
    endian_swap();

    // Endian-swap permutations if necessary.
    permutation* permutationPtr = first_permutation();
    for (u32 i = 0; i < permutationCount; ++i)
    {
        permutationPtr->endian_swap();
        permutationPtr = permutationPtr->next();
    }

    // Endian-swap shader variant indices.
    u32 variantIndicesCount = variant_count();
    u32* variantIndexPtr = reinterpret_cast<u32*>(permutationPtr);

    for (u32 i = 0; i < variantIndicesCount; ++i)
    {
        hl::endian_swap(*(variantIndexPtr++));
    }

    // Endian-swap shader variant count.
    hl::endian_swap(*variantIndexPtr);
    u32 variantCount = *(variantIndexPtr++);

    // Endian-swap shader variants.
    variant* variantPtr = reinterpret_cast<variant*>(variantIndexPtr);
    for (u32 i = 0; i < variantCount; ++i)
    {
        // Endian-swap shader variant.
        variantPtr->endian_swap();

        // Get next shader variant pointer.
        variant* nextVariantPtr = variantPtr->next();

        // Endian-swap bytecode if necessary.
        code_container* codeContainerPtr = variantPtr->bytecode();
        if (codeContainerPtr < reinterpret_cast<code_container*>(nextVariantPtr))
        {
            codeContainerPtr->endian_swap();
        }
        else
        {
            variantPtr = nextVariantPtr;
            continue;
        }

        // Endian-swap input bytecode if necessary.
        codeContainerPtr = variantPtr->input_bytecode();
        if (codeContainerPtr < reinterpret_cast<code_container*>(nextVariantPtr))
        {
            codeContainerPtr->endian_swap();
        }
        else
        {
            variantPtr = nextVariantPtr;
            continue;
        }

        // Endian-swap typed params containers.
        typed_params_container* typedParamsContPtr = variantPtr->typed_params();
        while (typedParamsContPtr < reinterpret_cast<typed_params_container*>(nextVariantPtr))
        {
            // Endian-swap typed params container.
            typedParamsContPtr->endian_swap();

            // Get next typed params container pointer.
            typed_params_container* nextTypedParamsContPtr = typedParamsContPtr->next();

            // Endian-swap typed params.
            typed_params* typedParamsPtr = typedParamsContPtr->first_type();
            while (typedParamsPtr < reinterpret_cast<typed_params*>(nextTypedParamsContPtr))
            {
                // Endian-swap typed params.
                typedParamsPtr->endian_swap();

                // Endian-swap params container.
                params_container& params = typedParamsPtr->params();
                void* curParam = params.first_param();
                void* paramsEnd = params.end();

                switch (static_cast<param_type>(typedParamsPtr->type))
                {
                case param_type::res_texture:
                case param_type::res_sampler:
                {
                    while (curParam < paramsEnd)
                    {
                        resource* curRes = static_cast<resource*>(curParam);
                        curRes->endian_swap();
                    }
                    break;
                }

                case param_type::constant_buffer:
                    // TODO
                    break;

                case param_type::const_bool:
                case param_type::const_int:
                case param_type::const_float:
                    // TODO
                    break;
                }
            }

            // TODO

            typedParamsContPtr = nextTypedParamsContPtr;
        }
    }
#endif
}
} // v2
} // shader
} // needle
} // hh
} // hl
