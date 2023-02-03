#include "hedgelib/materials/hl_hh_material.h"
#include "hedgelib/io/hl_path.h"
#include "hedgelib/hl_scene.h"
#include "hedgelib/hl_blob.h"

namespace hl
{
namespace hh
{
namespace mirage
{
template<typename T>
void in_material_fix_param(raw_material_param<T>& rawParam)
{
    // Swap parameter entry.
    endian_swap<false>(rawParam);

    // Swap parameter values.
    for (u8 i = 0; i < rawParam.valueCount; ++i)
    {
        endian_swap<false>(rawParam.values[i]);
    }
}

template<typename T>
void in_material_fix_params(off32<raw_material_param<T>>* rawParams, u8 rawParamCount)
{
    for (u8 i = 0; i < rawParamCount; ++i)
    {
        in_material_fix_param(*rawParams[i]);
    }
}

void raw_material_v1::fix()
{
#ifndef HL_IS_BIG_ENDIAN
    // Swap material header.
    endian_swap<false>();

    // Swap parameters.
    in_material_fix_params(float4Params.get(), float4ParamCount);
    in_material_fix_params(int4Params.get(), int4ParamCount);
    in_material_fix_params(bool4Params.get(), bool4ParamCount);
#endif
}

void raw_material_v3::fix()
{
#ifndef HL_IS_BIG_ENDIAN
    // Swap material header.
    endian_swap<false>();

    // Swap texture entries.
    for (u8 i = 0; i < textureEntryCount; ++i)
    {
        textureEntries[i]->endian_swap<false>();
    }

    // Swap parameters.
    in_material_fix_params(float4Params.get(), float4ParamCount);
    in_material_fix_params(int4Params.get(), int4ParamCount);
    in_material_fix_params(bool4Params.get(), bool4ParamCount);
#endif
}

template<typename T>
static void in_material_parse_params(const off32<raw_material_param<T>>* rawParams,
    u8 rawParamCount, std::vector<material_param<T>>& params)
{
    params.reserve(rawParamCount);
    for (u8 i = 0; i < rawParamCount; ++i)
    {
        params.emplace_back(*rawParams[i]);
    }
}

void material::in_parse(const raw_material_v1& rawMat, const nchar* texsetDir)
{
    // Parse shader names.
    shader = rawMat.shaderName.get();
    subShader = rawMat.subShaderName.get();

    // Parse flags.
    alphaThreshold = math::unorm_to_float<u8>(rawMat.alphaThreshold);
    noBackfaceCulling = rawMat.noBackfaceCulling;
    useAdditiveBlending = rawMat.useAdditiveBlending;
    unknownFlag1 = rawMat.unknownFlag1;

    // Parse parameters.
    in_material_parse_params(rawMat.float4Params.get(), rawMat.float4ParamCount, float4Params);
    in_material_parse_params(rawMat.int4Params.get(), rawMat.int4ParamCount, int4Params);
    in_material_parse_params(rawMat.bool4Params.get(), rawMat.bool4ParamCount, bool4Params);

    // Attempt to load and parse texset if possible.
    const auto texsetName = rawMat.texsetName.get();
    if (texsetDir && texsetName)
    {
        // Get texset path.
        auto texsetPath = path::combine(texsetDir,
#ifdef HL_IN_WIN32_UNICODE
            text::conv<text::utf8_to_native>(texsetName).c_str());
#else
            texsetName);
#endif

        texsetPath += mirage::texset::ext;

        // Load the texset if it actually exists.
        if (path::exists(texsetPath))
        {
            texset.load(texsetPath);
        }
    }

    // Or just store texset name if we can't load the texset.
    else
    {
        texset = mirage::texset();
        texset.name = texsetName;
    }
}

void material::in_parse(const raw_material_v3& rawMat)
{
    // Parse shader names.
    shader = rawMat.shaderName.get();
    subShader = rawMat.subShaderName.get();

    // Parse texset data.
    texset = mirage::texset();
    texset.name = name;
    texset.reserve(rawMat.textureEntryCount);

    for (u8 i = 0; i < rawMat.textureEntryCount; ++i)
    {
        texset.emplace_back(*rawMat.textureEntries[i],
            rawMat.textureEntryNames[i].get());
    }

    // Parse flags.
    alphaThreshold = math::unorm_to_float<u8>(rawMat.alphaThreshold);
    noBackfaceCulling = rawMat.noBackfaceCulling;
    useAdditiveBlending = rawMat.useAdditiveBlending;
    unknownFlag1 = rawMat.unknownFlag1;

    // Parse parameters.
    in_material_parse_params(rawMat.float4Params.get(), rawMat.float4ParamCount, float4Params);
    in_material_parse_params(rawMat.int4Params.get(), rawMat.int4ParamCount, int4Params);
    in_material_parse_params(rawMat.bool4Params.get(), rawMat.bool4ParamCount, bool4Params);
}

void material::in_parse_sample_chunk_nodes(const void* rawData)
{
    // Ensure that we have Material node.
    const auto& rawHeader = *static_cast<const sample_chunk::raw_header*>(rawData);
    const auto rawMaterialNode = rawHeader.get_node("Material", false);

    if (!rawMaterialNode) return;

    // TODO: Parse sample chunk nodes.
}

void material::in_parse(const void* rawData, const nchar* texsetDir)
{
    // Get material data and version number.
    u32 version;
    const auto matData = get_data(rawData, &version);
    if (!matData) return; // TODO: Should this be an error?

    // Parse material data based on version number.
    switch (version)
    {
    case 1:
        in_parse(*static_cast<const raw_material_v1*>(matData), texsetDir);
        break;

    case 3:
        in_parse(*static_cast<const raw_material_v3*>(matData));
        break;

    default:
        throw std::runtime_error("Unsupported HH mirage material version");
    }

    // Parse sample chunk nodes if necessary.
    if (has_sample_chunk_header_fixed(rawData))
    {
        in_parse_sample_chunk_nodes(rawData);
    }
}

void material::in_load(const nchar* filePath)
{
    // Load and parse material.
    const auto texsetDir = path::get_parent(filePath);
    blob rawMat(filePath);
    fix(rawMat);
    in_parse(rawMat, texsetDir.c_str());
}

void material::in_clear() noexcept
{
    float4Params.clear();
    int4Params.clear();
    bool4Params.clear();
}

void material::in_add_to_hl_material(std::string& utf8TexFilePath,
    hl::material& mat, bool includeLibGensTags) const
{
    if (includeLibGensTags)
    {
        // TODO: Add @TXTR tags to material name as needed?
        // TODO: Add @SHDR tag to material name as needed.
        // TODO: Add @PMTR tags to material name as needed.
        // TODO: Add @CULL tag to material name as needed.
        // TODO: Add @ADD tag to material name as needed.
    }

    // Parse float4 parameters.
    for (auto& float4Param : float4Params)
    {
        if (!float4Param.values.empty())
        {
            if (float4Param.name == "diffuse")
            {
                mat.diffuseColor = float4Param.values[0].as_vec3();
            }
            else if (float4Param.name == "specular")
            {
                mat.specularColor = float4Param.values[0].as_vec3();
            }
            else if (float4Param.name == "ambient")
            {
                mat.ambientColor = float4Param.values[0].as_vec3();
            }
            else if (float4Param.name == "emissive")
            {
                mat.emissiveColor = float4Param.values[0].as_vec3();
            }

            // TODO
        }
    }

    // TODO

    // Parse texset.
    auto& scene = mat.scene();
    const auto utf8TexFileNamePos = utf8TexFilePath.size();
    
    for (auto& texEntry : texset)
    {
        auto tex = scene.find_texture(texEntry.name);
        if (!tex)
        {
            utf8TexFilePath += texEntry.texName;
            utf8TexFilePath += ".dds";

            tex = &scene.add_texture(texEntry.name, utf8TexFilePath);

            utf8TexFilePath.erase(utf8TexFileNamePos);
        }

        mat.textures.emplace_back(texEntry.get_hl_map_slot_type(), *tex);
    }

    // TODO
}

void material::fix(void* rawData)
{
    // Fix mirage data.
    mirage::fix(rawData);

    // Get material data and version number.
    u32 version;
    const auto matData = get_data(rawData, &version);
    if (!matData) return;

    // Fix material data based on version number.
    switch (version)
    {
    case 1:
        static_cast<raw_material_v1*>(matData)->fix();
        break;

    case 3:
        static_cast<raw_material_v3*>(matData)->fix();
        break;

    default:
        throw std::runtime_error("Unsupported HH mirage material version");
    }
}

void material::parse(const void* rawData, std::string name, const nchar* texsetDir)
{
    // Clear any existing data.
    in_clear();

    // Set new name.
    this->name = std::move(name);

    // Parse raw material data.
    in_parse(rawData, texsetDir);
}

void material::load(const nchar* filePath)
{
    // Clear any existing data.
    in_clear();

    // Set new name.
    name = std::move(get_res_name(filePath));

    // Load material.
    in_load(filePath);
}

void material::add_to_hl_scene(const nchar* texDir, scene& scene,
    bool merge, bool includeLibGensTags) const
{
    // Get absolute texture UTF-8 directory.
    // TODO: Actually make texDir absolute.
    std::string utf8TexFilePath =
#ifdef HL_IN_WIN32_UNICODE
        text::conv<text::native_to_utf8>(texDir);
#else
        texDir;
#endif

    if (path::combine_needs_sep1(utf8TexFilePath))
    {
        utf8TexFilePath += path::separator;
    }

    // Attempt to merge with existing materials if requested.
    if (merge)
    {
        bool didMerge = false;
        for (auto& materialPtr : scene.materials())
        {
            if (materialPtr->name.name_equals(name))
            {
                in_add_to_hl_material(utf8TexFilePath, *materialPtr, includeLibGensTags);
                didMerge = true;

                // NOTE: We purposely don't break here as we want to merge with *ALL*
                // materials in the scene with a matching name. We might have, for example,
                // "my_material@LYR(trans)" and also "my_material&LYR(punch)".
            }
        }

        if (didMerge) return;
    }
    
    // Add new material to scene.
    auto& mat = scene.add_material(name);
    in_add_to_hl_material(utf8TexFilePath, mat, includeLibGensTags);
}

material::material(const void* rawData, std::string name, const nchar* texsetDir) :
    res_base(std::move(name)),
    shader(),
    subShader()
{
    in_parse(rawData, texsetDir);
}

material::material(const void* rawData, std::string name, const nstring& texsetDir) :
    res_base(std::move(name)),
    shader(),
    subShader()
{
    in_parse(rawData, texsetDir.c_str());
}

material::material(const nchar* filePath) :
    res_base(std::move(get_res_name(filePath))),
    shader(),
    subShader()
{
    in_load(filePath);
}
} // mirage
} // hh
} // hl
