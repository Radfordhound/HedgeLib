#include "hedgelib/materials/hl_hh_material.h"
#include "hedgelib/io/hl_path.h"
#include "hedgelib/hl_scene.h"
#include "hedgelib/hl_blob.h"
#include <new>

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
    rawParam.template endian_swap<false>();

    // Swap parameter values.
    for (u8 i = 0; i < rawParam.valueCount; ++i)
    {
        rawParam.values[i].template endian_swap<false>();
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

void material::in_add_to_material(std::string& utf8TexFilePath,
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
                mat.diffuseColor = float4Param.values[0].to_vec3();
            }
            else if (float4Param.name == "specular")
            {
                mat.specularColor = float4Param.values[0].to_vec3();
            }
            else if (float4Param.name == "ambient")
            {
                mat.ambientColor = float4Param.values[0].to_vec3();
            }
            else if (float4Param.name == "emissive")
            {
                mat.emissiveColor = float4Param.values[0].to_vec3();
            }

            // TODO
        }
    }

    // TODO

    // Parse texset.
    const std::size_t utf8TexFileNamePos = utf8TexFilePath.size();
    for (auto& texEntry : texset)
    {
        hl::texture* tex = mat.scene().find_texture(texEntry.name);
        if (!tex)
        {
            utf8TexFilePath += texEntry.texName;
            utf8TexFilePath += ".dds";

            tex = &mat.scene().add_texture(texEntry.name, utf8TexFilePath);

            utf8TexFilePath.erase(utf8TexFileNamePos);
        }

        mat.textures.emplace_back(texEntry.get_map_slot_type(), *tex);
    }

    // TODO
}

template<typename T>
void in_material_parse_params(const off32<raw_material_param<T>>* rawParams,
    u8 rawParamCount, std::vector<material_param<T>>& params)
{
    params.reserve(rawParamCount);
    for (u8 i = 0; i < rawParamCount; ++i)
    {
        params.emplace_back(*rawParams[i]);
    }
}

void material::fix(void* rawData)
{
    // Fix mirage data.
    mirage::fix(rawData);

    // Get material data and version number.
    u32 version;
    void* matData = get_data(rawData, &version);
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
        throw std::runtime_error("Unsupported HH material version");
    }
}

void material::add_to_scene(const nchar* texDir, scene& scene,
    bool merge, bool includeLibGensTags) const
{
    // Get absolute texture UTF-8 directory.
    // TODO: Make texDir absolute first.
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
                in_add_to_material(utf8TexFilePath, *materialPtr, includeLibGensTags);
                didMerge = true;

                // NOTE: We purposely don't break here as we want to merge with *ALL*
                // materials in the scene with a matching name. We might have, for example,
                // "my_material@LYR(trans)" and also "my_material&LYR(punch)".
            }
        }

        if (didMerge) return;
    }
    
    // Add new material to scene.
    hl::material& mat = scene.add_material(name);
    in_add_to_material(utf8TexFilePath, mat, includeLibGensTags);
}

void material::parse(const raw_material_v1& rawMat)
{
    // Parse shader names.
    shaderName = rawMat.shaderName.get();
    subShaderName = rawMat.subShaderName.get();

    // Parse texset name.
    texset.name = rawMat.texsetName.get();

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

void material::parse(const raw_material_v1& rawMat, const nchar* texsetDir)
{
    // Parse material.
    parse(rawMat);

    // Attempt to load and parse texset.
    if (!texset.name.empty() && texsetDir)
    {
        // Get texset path.
        nstring texsetPath = path::combine(texsetDir,
#ifdef HL_IN_WIN32_UNICODE
            text::conv<text::utf8_to_native>(texset.name).c_str());
#else
            texset.name.c_str());
#endif

        texsetPath += texset::ext;

        // Load the texset if it actually exists.
        if (path::exists(texsetPath))
        {
            texset.load(texsetPath);
        }
    }
}

void material::parse(const raw_material_v1& rawMat, const nstring& texsetDir)
{
    // Parse material.
    parse(rawMat);

    // Attempt to load and parse texset.
    if (!texset.name.empty())
    {
        // Get texset path.
        nstring texsetPath = path::combine(texsetDir,
#ifdef HL_IN_WIN32_UNICODE
            text::conv<text::utf8_to_native>(texset.name));
#else
            texset.name);
#endif

        texsetPath += texset::ext;

        // Load the texset if it actually exists.
        if (path::exists(texsetPath))
        {
            texset.load(texsetPath);
        }
    }
}

void material::parse(const raw_material_v3& rawMat)
{
    // Parse shader names.
    shaderName = rawMat.shaderName.get();
    subShaderName = rawMat.subShaderName.get();

    // Parse texset data.
    texset.name = name;
    texset.reserve(rawMat.textureEntryCount);

    for (u8 i = 0; i < rawMat.textureEntryCount; ++i)
    {
        texset.emplace_back(
            rawMat.textureEntryNames[i].get(),
            *rawMat.textureEntries[i]);
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

void material::parse(const void* rawData, const nchar* texsetDir)
{
    // Get material data and version number.
    u32 version;
    const void* matData = get_data(rawData, &version);
    if (!matData) return; // TODO: Should this be an error?

    // Parse material data based on version number.
    switch (version)
    {
    case 1:
        parse(*static_cast<const raw_material_v1*>(matData), texsetDir);
        break;

    case 3:
        parse(*static_cast<const raw_material_v3*>(matData));
        break;

    default:
        throw std::runtime_error("Unsupported HH material version");
    }

    // Parse sample chunk nodes if necessary.
    if (has_sample_chunk_header_fixed(rawData))
    {
        // Ensure that we have Material node.
        const sample_chunk::raw_header* rawHeader = static_cast<
            const sample_chunk::raw_header*>(rawData);

        const sample_chunk::raw_node* rawMaterialNode =
            rawHeader->get_node("Material", false);

        if (!rawMaterialNode) return;

        // TODO: Parse sample chunk nodes.
    }
}

void material::load(const nchar* filePath)
{
    // Get material name from file path.
#ifdef HL_IN_WIN32_UNICODE
    name = text::conv<text::native_to_utf8>(
        path::remove_ext(path::get_name(filePath)));
#else
    name = path::remove_ext(path::get_name(filePath));
#endif

    // Load and parse material.
    blob rawMat(filePath);
    fix(rawMat);
    parse(rawMat, path::get_parent(filePath));
}

material::material(const void* rawData, const char* name) :
    name(name),
    shaderName(),
    subShaderName()
{
    parse(rawData);
}

material::material(const void* rawData, const std::string& name) :
    name(name),
    shaderName(),
    subShaderName()
{
    parse(rawData);
}

material::material(const void* rawData, std::string&& name) :
    name(std::move(name)),
    shaderName(),
    subShaderName()
{
    parse(rawData);
}

material::material(const nchar* filePath) :
    shaderName(),
    subShaderName()
{
    load(filePath);
}
} // mirage
} // hh
} // hl
