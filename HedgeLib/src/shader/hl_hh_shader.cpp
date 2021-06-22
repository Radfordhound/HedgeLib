#include "hedgelib/shader/hl_hh_shader.h"
#include <cstring>

namespace hl
{
namespace hh
{
namespace needle
{
const raw_code_container* raw_shader_variant::input_bytecode() const noexcept
{
    const raw_code_container* container = bytecode();
    return ptradd<raw_code_container>(container->data(), container->size);
}

raw_code_container* raw_shader_variant::input_bytecode() noexcept
{
    raw_code_container* container = bytecode();
    return ptradd<raw_code_container>(container->data(), container->size);
}

const raw_typed_params_container* raw_shader_variant::typed_params() const noexcept
{
    const raw_code_container* inputBytecode = input_bytecode();
    return ptradd<raw_typed_params_container>(inputBytecode->data(), inputBytecode->size);
}

raw_typed_params_container* raw_shader_variant::typed_params() noexcept
{
    raw_code_container* inputBytecode = input_bytecode();
    return ptradd<raw_typed_params_container>(inputBytecode->data(), inputBytecode->size);
}

const raw_permutation* raw_permutation::next() const noexcept
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
    return reinterpret_cast<const raw_permutation*>(namePtr);
}

void raw_shader_v2::fix()
{
#ifndef HL_IS_BIG_ENDIAN
    // Endian-swap header.
    endian_swap();

    // Endian-swap permutations if necessary.
    raw_permutation* permutationPtr = first_permutation();
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
    raw_shader_variant* variantPtr = reinterpret_cast<raw_shader_variant*>(variantIndexPtr);
    for (u32 i = 0; i < variantCount; ++i)
    {
        // Endian-swap shader variant.
        variantPtr->endian_swap();

        // Get next shader variant pointer.
        raw_shader_variant* nextVariantPtr = variantPtr->next();

        // Endian-swap bytecode if necessary.
        raw_code_container* codeContainerPtr = variantPtr->bytecode();
        if (codeContainerPtr < reinterpret_cast<raw_code_container*>(nextVariantPtr))
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
        if (codeContainerPtr < reinterpret_cast<raw_code_container*>(nextVariantPtr))
        {
            codeContainerPtr->endian_swap();
        }
        else
        {
            variantPtr = nextVariantPtr;
            continue;
        }

        // Endian-swap typed params containers.
        raw_typed_params_container* typedParamsContPtr = variantPtr->typed_params();
        while (typedParamsContPtr < reinterpret_cast<raw_typed_params_container*>(nextVariantPtr))
        {
            // Endian-swap typed params container.
            typedParamsContPtr->endian_swap();

            // Get next typed params container pointer.
            raw_typed_params_container* nextTypedParamsContPtr = typedParamsContPtr->next();

            // Endian-swap typed params.
            raw_typed_params* typedParamsPtr = typedParamsContPtr->first_type();
            while (typedParamsPtr < reinterpret_cast<raw_typed_params*>(nextTypedParamsContPtr))
            {
                // Endian-swap typed params.
                typedParamsPtr->endian_swap();

                // Endian-swap params container.
                raw_params_container& params = typedParamsPtr->params();
                void* curParam = params.first_param();
                void* paramsEnd = params.end();

                switch (static_cast<raw_param_type>(typedParamsPtr->type))
                {
                case raw_param_type::res_texture:
                case raw_param_type::res_sampler:
                {
                    while (curParam < paramsEnd)
                    {
                        raw_resource* curRes = static_cast<raw_resource*>(curParam);
                        curRes->endian_swap();
                    }
                    break;
                }

                case raw_param_type::constant_buffer:
                    while (curParam < paramsEnd)
                    {
                        raw_constant_buffer* curBuf = static_cast<raw_constant_buffer*>(curParam);
                        curBuf->endian_swap();
                    }
                    break;

                case raw_param_type::const_bool:
                case raw_param_type::const_int:
                case raw_param_type::const_float:
                    while (curParam < paramsEnd)
                    {
                        raw_constant* curConst = static_cast<raw_constant*>(curParam);
                        curConst->endian_swap();
                    }
                    break;
                }
            }

            // TODO?

            typedParamsContPtr = nextTypedParamsContPtr;
        }
    }
#endif
}
} // needle
} // hh
} // hl
