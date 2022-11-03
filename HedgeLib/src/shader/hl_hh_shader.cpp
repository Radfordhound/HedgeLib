#include "hedgelib/hl_blob.h"
#include "hedgelib/io/hl_hh_mirage.h"
#include "hedgelib/io/hl_file.h"
#include "hedgelib/io/hl_path.h"
#include "hedgelib/shader/hl_hh_shader.h"
#include <cstring>

namespace hl
{
namespace hh
{
namespace mirage
{
void raw_shader_params_v2::fix()
{
#ifndef HL_IS_BIG_ENDIAN
    // Swap shader params.
    endian_swap<false>();
#endif
}

void raw_shader_v2::fix()
{
#ifndef HL_IS_BIG_ENDIAN
    // Swap shader.
    endian_swap<false>();
#endif
}

static void in_fix(raw_vertex_shader_permutation& vertexPermutation)
{
    // Swap vertex permutation.
    vertexPermutation.endian_swap<false>();
}

static void in_fix(raw_pixel_shader_permutation& pixelPermutation)
{
    // Swap pixel permutation.
    pixelPermutation.endian_swap<false>();

    // Swap vertex permutations.
    for (auto& vertexPermutation : pixelPermutation.vertexPermutations)
    {
        in_fix(*vertexPermutation);
    }
}

void raw_shader_list_v0::fix()
{
#ifndef HL_IS_BIG_ENDIAN
    // Swap shader list.
    endian_swap<false>();

    // Swap pixel permutations.
    for (auto& pixelPermutation : pixelPermutations)
    {
        in_fix(*pixelPermutation);
    }
#endif
}

shader_param_resource::shader_param_resource(const raw_shader_param_resource& rawShaderRes) :
    name(rawShaderRes.name.get()),
    regAndTexCoordIndex(rawShaderRes.regAndTexCoordIndex) {}

static void in_parse_shader_consts(
    const arr32<off32<raw_shader_param_constant>>& rawParams,
    hl::radix_tree<shader_param_constant>& params)
{
    params.reserve(rawParams.count);
    for (auto& param : rawParams)
    {
        params.insert(param->name.get(), *param);
    }
}

static void in_parse_shader_resources(
    const arr32<off32<raw_shader_param_resource>>& rawParams,
    std::vector<shader_param_resource>& params)
{
    params.reserve(rawParams.count);
    for (auto& param : rawParams)
    {
        params.emplace_back(*param);
    }
}

void shader_params::in_parse(const raw_shader_params_v2& rawShaderParams)
{
    in_parse_shader_consts(rawShaderParams.floats, floats);
    in_parse_shader_consts(rawShaderParams.ints, ints);
    in_parse_shader_consts(rawShaderParams.bools, bools);
    in_parse_shader_resources(rawShaderParams.textures, textures);
    //in_parse_shader_resources(rawShaderParams.unknown2, unknown2);
}

void shader_params::in_parse(const void* rawData)
{
    // Get shader params data and version number.
    u32 version;
    const auto shaderParamsData = get_data(rawData, &version);
    if (!shaderParamsData) return;

    // Parse shader params data based on version number.
    switch (version)
    {
    case 2:
        in_parse(*static_cast<const raw_shader_params_v2*>(shaderParamsData));
        break;

    default:
        throw std::runtime_error("Unsupported HH mirage shader parameters version");
    }
}

void shader_params::in_load(const nchar* filePath)
{
    // Load and parse shader parameters.
    blob rawShaderParams(filePath);
    fix(rawShaderParams);
    in_parse(rawShaderParams);
}

void shader_params::in_clear() noexcept
{
    floats.clear();
    ints.clear();
    bools.clear();
    textures.clear();
    //unknown2.clear();
}

void shader_params::fix(void* rawData)
{
    // Fix mirage data.
    mirage::fix(rawData);

    // Get shader params data and version number.
    u32 version;
    const auto shaderParamsData = get_data(rawData, &version);
    if (!shaderParamsData) return;

    // Fix shader params data based on version number.
    switch (version)
    {
    case 2:
        static_cast<raw_shader_params_v2*>(shaderParamsData)->fix();
        break;

    default:
        throw std::runtime_error("Unsupported HH mirage shader parameters version");
    }
}

std::size_t shader_params::total_constant_count() const noexcept
{
    return (floats.size() + ints.size() + bools.size());
}

std::size_t shader_params::total_resource_count() const noexcept
{
    return (textures.size() /*+ unknown2.size()*/);
}

std::size_t shader_params::total_param_count() const noexcept
{
    return (total_constant_count() + total_resource_count());
}

template<typename T>
static const shader_param_resource* in_get_shader_resource(
    const std::vector<shader_param_resource>& params, T name)
{
    for (auto& param : params)
    {
        if (param.name == name)
        {
            return &param;
        }
    }

    return nullptr;
}

//const shader_param_constant* shader_params::get_float_param(const char* name) const
//{
//    return floats.get(name);
//}
//
//const shader_param_constant* shader_params::get_float_param(const std::string& name) const
//{
//    return in_get_shader_param(floats, name);
//}
//
//const shader_param_constant* shader_params::get_int_param(const char* name) const
//{
//    return in_get_shader_param(ints, name);
//}
//
//const shader_param_constant* shader_params::get_int_param(const std::string& name) const
//{
//    return in_get_shader_param(ints, name);
//}
//
//const shader_param_constant* shader_params::get_bool_param(const char* name) const
//{
//    return in_get_shader_param(bools, name);
//}
//
//const shader_param_constant* shader_params::get_bool_param(const std::string& name) const
//{
//    return in_get_shader_param(bools, name);
//}
//
const shader_param_resource* shader_params::get_texture_param(const char* name) const
{
    return in_get_shader_resource(textures, name);
}

const shader_param_resource* shader_params::get_texture_param(const std::string& name) const
{
    return in_get_shader_resource<const std::string&>(textures, name);
}

void shader_params::parse(const void* rawData, std::string name)
{
    // Clear any existing data.
    in_clear();

    // Set new name.
    this->name = std::move(name);

    // Parse data.
    in_parse(rawData);
}

void shader_params::load(const nchar* filePath)
{
    // Clear any existing data.
    in_clear();

    // Set new name.
    name = get_res_name(filePath);

    // Load shader params.
    in_load(filePath);
}

shader_params::shader_params(const void* rawData, std::string name) :
    res_base(std::move(name))
{
    in_parse(rawData);
}

shader_params::shader_params(const nchar* filePath) :
    res_base(get_res_name(filePath))
{
    in_load(filePath);
}

void shader::in_parse(const raw_shader_v2& rawShader)
{
    // Parse code file name.
    codeName = rawShader.codeFileName.get();

    // Parse parameter list file names.
    paramLists.reserve(rawShader.paramListFileNames.count);
    for (auto& paramListFileName : rawShader.paramListFileNames)
    {
        paramLists.emplace_back(paramListFileName.get());
    }
}

void shader::in_parse(const void* rawData)
{
    // Get shader data and version number.
    u32 version;
    const auto shaderData = get_data(rawData, &version);
    if (!shaderData) return;

    // Parse shader data based on version number.
    switch (version)
    {
    case 2:
        in_parse(*static_cast<const raw_shader_v2*>(shaderData));
        break;

    default:
        throw std::runtime_error("Unsupported HH mirage shader version");
    }
}

void shader::in_load(const nchar* filePath)
{
    // Load and parse shader.
    blob rawShader(filePath);
    fix(rawShader);
    in_parse(rawShader);
}

void shader::in_clear() noexcept
{
    codeDataPtr = nullptr;
    paramLists.clear();
}

void shader::fix(void* rawData)
{
    // Fix mirage data.
    mirage::fix(rawData);

    // Get shader data and version number.
    u32 version;
    const auto shaderData = get_data(rawData, &version);
    if (!shaderData) return;

    // Fix shader data based on version number.
    switch (version)
    {
    case 2:
        static_cast<raw_shader_v2*>(shaderData)->fix();
        break;

    default:
        throw std::runtime_error("Unsupported HH mirage shader version");
    }
}

const shader_param_constant* shader::get_float_param(const char* name) const
{
    for (auto& paramList : paramLists)
    {
        if (!paramList.has_res()) continue;

        const auto param = paramList->floats.get(name);
        if (param) return param;
    }

    return nullptr;
}

//const shader_param_constant* shader::get_float_param(const std::string& name) const
//{
//    for (auto& paramList : paramLists)
//    {
//        if (!paramList.has_res()) continue;
//
//        const auto param = paramList->floats.get(name);
//        if (param) return param;
//    }
//
//    return nullptr;
//}

const shader_param_constant* shader::get_int_param(const char* name) const
{
    for (auto& paramList : paramLists)
    {
        if (!paramList.has_res()) continue;

        const auto param = paramList->ints.get(name);
        if (param) return param;
    }

    return nullptr;
}

//const shader_param_constant* shader::get_int_param(const std::string& name) const
//{
//    for (auto& paramList : paramLists)
//    {
//        if (!paramList.has_res()) continue;
//
//        const auto param = paramList->ints.get(name);
//        if (param) return param;
//    }
//
//    return nullptr;
//}

const shader_param_constant* shader::get_bool_param(const char* name) const
{
    for (auto& paramList : paramLists)
    {
        if (!paramList.has_res()) continue;

        const auto param = paramList->bools.get(name);
        if (param) return param;
    }

    return nullptr;
}

//const shader_param_constant* shader::get_bool_param(const std::string& name) const
//{
//    for (auto& paramList : paramLists)
//    {
//        if (!paramList.has_res()) continue;
//
//        const auto param = paramList->bools.get(name);
//        if (param) return param;
//    }
//
//    return nullptr;
//}

const shader_param_resource* shader::get_texture_param(const char* name) const
{
    for (auto& paramList : paramLists)
    {
        if (!paramList.has_res()) continue;

        const auto param = paramList->get_texture_param(name);
        if (param) return param;
    }

    return nullptr;
}

const shader_param_resource* shader::get_texture_param(const std::string& name) const
{
    for (auto& paramList : paramLists)
    {
        if (!paramList.has_res()) continue;

        const auto param = paramList->get_texture_param(name);
        if (param) return param;
    }

    return nullptr;
}

void shader::parse(const void* rawData, std::string name)
{
    // Clear any existing data.
    in_clear();

    // Set new name.
    this->name = std::move(name);

    // Parse data.
    in_parse(rawData);
}

void shader::load(const nchar* filePath)
{
    // Clear any existing data.
    in_clear();

    // Set new name.
    name = get_res_name(filePath);

    // Load shader.
    in_load(filePath);
}

shader::shader(const void* rawData, std::string name) :
    res_base(std::move(name))
{
    in_parse(rawData);
}

shader::shader(const nchar* filePath) :
    res_base(get_res_name(filePath))
{
    in_load(filePath);
}

vertex_shader_permutation::vertex_shader_permutation(
    const raw_vertex_shader_permutation& rawVertexPermutation) :
    subPermutations(rawVertexPermutation.subPermutations),
    name(rawVertexPermutation.name.get()),
    vertexShaderName(rawVertexPermutation.vertexShaderName.get()) {}

const vertex_shader_permutation* pixel_shader_permutation::get_vertex_permutation(
    const char* name) const
{
    for (auto& vertexPermutation : vertexPermutations)
    {
        if (vertexPermutation.name == name)
        {
            return &vertexPermutation;
        }
    }

    return nullptr;
}

const vertex_shader_permutation* pixel_shader_permutation::get_vertex_permutation(
    const std::string& name) const
{
    for (auto& vertexPermutation : vertexPermutations)
    {
        if (vertexPermutation.name == name)
        {
            return &vertexPermutation;
        }
    }

    return nullptr;
}

pixel_shader_permutation::pixel_shader_permutation(
    const raw_pixel_shader_permutation& rawPixelPermutation) :
    subPermutations(rawPixelPermutation.subPermutations),
    name(rawPixelPermutation.name.get()),
    pixelShaderName(rawPixelPermutation.pixelShaderName.get())
{
    // Parse vertex permutations.
    vertexPermutations.reserve(rawPixelPermutation.vertexPermutations.count);
    for (const auto& vertexPermutation : rawPixelPermutation.vertexPermutations)
    {
        vertexPermutations.emplace_back(*vertexPermutation);
    }
}

void shader_list::in_parse(const raw_shader_list_v0& rawShaderList)
{
    // Parse pixel permutations.
    pixelPermutations.reserve(rawShaderList.pixelPermutations.count);
    for (const auto& pixelPermutation : rawShaderList.pixelPermutations)
    {
        pixelPermutations.emplace_back(*pixelPermutation);
    }
}

void shader_list::in_parse(const void* rawData)
{
    // Get shader list data and version number.
    u32 version;
    const auto shaderListData = get_data(rawData, &version);
    if (!shaderListData) return;

    // Parse shader list data based on version number.
    switch (version)
    {
    case 0:
        in_parse(*static_cast<const raw_shader_list_v0*>(shaderListData));
        break;

    default:
        throw std::runtime_error("Unsupported HH mirage shader list version");
    }
}

void shader_list::in_load(const nchar* filePath)
{
    // Load and parse shader list.
    blob rawShaderList(filePath);
    fix(rawShaderList);
    in_parse(rawShaderList);
}

void shader_list::fix(void* rawData)
{
    // Fix mirage data.
    mirage::fix(rawData);

    // Get shader list data and version number.
    u32 version;
    const auto shaderListData = get_data(rawData, &version);
    if (!shaderListData) return;

    // Fix shader list data based on version number.
    switch (version)
    {
    case 0:
        static_cast<raw_shader_list_v0*>(shaderListData)->fix();
        break;

    default:
        throw std::runtime_error("Unsupported HH mirage shader list version");
    }
}

const pixel_shader_permutation* shader_list::get_pixel_permutation(
    const char* name) const
{
    for (auto& pixelPermutation : pixelPermutations)
    {
        if (pixelPermutation.name == name)
        {
            return &pixelPermutation;
        }
    }

    return nullptr;
}

const pixel_shader_permutation* shader_list::get_pixel_permutation(
    const std::string& name) const
{
    for (auto& pixelPermutation : pixelPermutations)
    {
        if (pixelPermutation.name == name)
        {
            return &pixelPermutation;
        }
    }
    
    return nullptr;
}

void shader_list::parse(const void* rawData, std::string name)
{
    // Clear any existing data.
    pixelPermutations.clear();

    // Set new name.
    this->name = std::move(name);

    // Parse data.
    in_parse(rawData);
}

void shader_list::load(const nchar* filePath)
{
    // Clear any existing data.
    pixelPermutations.clear();

    // Set new name.
    name = get_res_name(filePath);

    // Parse data.
    in_load(filePath);
}

shader_list::shader_list(const void* rawData, std::string name) :
    res_base(std::move(name))
{
    in_parse(rawData);
}

shader_list::shader_list(const nchar* filePath) :
    res_base(get_res_name(filePath))
{
    in_load(filePath);
}
} // mirage

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
