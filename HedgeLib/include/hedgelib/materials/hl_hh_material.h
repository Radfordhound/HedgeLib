#ifndef HL_HH_MATERIAL_H_INCLUDED
#define HL_HH_MATERIAL_H_INCLUDED
#include "../hl_math.h"
#include "../textures/hl_hh_texture.h"

namespace hl
{
class material;
class scene;

namespace hh
{
namespace mirage
{
template<typename T>
struct raw_material_param
{
    /** @brief Always 2 or 0? */
    u8 flag1;
    /** @brief Always 0? */
    u8 flag2;
    /** @brief How many values are in the array pointed to by values. */
    u8 valueCount;
    /** @brief Always 0? */
    u8 flag3;
    off32<char> name;
    off32<T> values;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap<swapOffsets>(values);
    }
};

HL_STATIC_ASSERT_SIZE(raw_material_param<vec4>, 12);

struct raw_material_v1
{
    off32<char> shaderName;
    off32<char> subShaderName;
    off32<char> texsetName;
    u32 reserved1;
    u8 alphaThreshold;
    u8 noBackfaceCulling;
    u8 useAdditiveBlending;
    u8 unknownFlag1;
    u8 float4ParamCount;
    u8 int4ParamCount;
    u8 bool4ParamCount;
    u8 reserved2;
    off32<off32<raw_material_param<vec4>>> float4Params;
    off32<off32<raw_material_param<ivec4>>> int4Params;
    off32<off32<raw_material_param<bvec4>>> bool4Params;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(shaderName);
        hl::endian_swap<swapOffsets>(subShaderName);
        hl::endian_swap<swapOffsets>(texsetName);
        hl::endian_swap(reserved1);
        hl::endian_swap<swapOffsets>(float4Params);
        hl::endian_swap<swapOffsets>(int4Params);
        hl::endian_swap<swapOffsets>(bool4Params);
    }

    HL_API void fix();
};

HL_STATIC_ASSERT_SIZE(raw_material_v1, 0x24);

struct raw_material_v3
{
    off32<char> shaderName;
    off32<char> subShaderName;
    off32<off32<char>> textureEntryNames;
    off32<off32<raw_texture_entry_v1>> textureEntries;
    u8 alphaThreshold;
    u8 noBackfaceCulling;
    u8 useAdditiveBlending;
    u8 unknownFlag1;
    u8 float4ParamCount;
    u8 int4ParamCount;
    u8 bool4ParamCount;
    u8 textureEntryCount;
    off32<off32<raw_material_param<vec4>>> float4Params;
    off32<off32<raw_material_param<ivec4>>> int4Params;
    off32<off32<raw_material_param<bvec4>>> bool4Params;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(shaderName);
        hl::endian_swap<swapOffsets>(subShaderName);
        hl::endian_swap<swapOffsets>(textureEntryNames);
        hl::endian_swap<swapOffsets>(textureEntries);
        hl::endian_swap<swapOffsets>(float4Params);
        hl::endian_swap<swapOffsets>(int4Params);
        hl::endian_swap<swapOffsets>(bool4Params);
    }

    HL_API void fix();
};

HL_STATIC_ASSERT_SIZE(raw_material_v3, 0x24);

template<typename T>
struct material_param
{
    u8 flag1 = 0;
    u8 flag2 = 0;
    u8 flag3 = 0;
    std::string name;
    std::vector<T> values;

    material_param(const char* name) : name(name) {}
    material_param(const std::string& name) : name(name) {}
    material_param(std::string&& name) noexcept : name(std::move(name)) {}

    material_param(const raw_material_param<T>& rawParam) :
        flag1(rawParam.flag1),
        flag2(rawParam.flag2),
        flag3(rawParam.flag3),
        name(rawParam.name.get()),
        values(rawParam.values.get(), rawParam.values.get() + rawParam.valueCount) {}
};

class material
{
    HL_API void in_add_to_material(std::string& utf8TexFilePath,
        hl::material& mat, bool includeLibGensTags = true) const;

public:
    std::string name;
    std::string shaderName = "Common_d";
    std::string subShaderName = "Common_d";
    float alphaThreshold = 0.5f;
    bool noBackfaceCulling = false;
    bool useAdditiveBlending = false;
    u8 unknownFlag1 = 0;
    std::vector<material_param<vec4>> float4Params;
    std::vector<material_param<ivec4>> int4Params;
    std::vector<material_param<bvec4>> bool4Params;
    mirage::texset texset;

    constexpr static const nchar* const ext = HL_NTEXT(".material");

    HL_API static void fix(void* rawData);

    HL_API void add_to_scene(const nchar* texDir, scene& scene,
        bool merge = true, bool includeLibGensTags = true) const;

    void add_to_scene(const nstring& texDir, scene& scene,
        bool merge = true, bool includeLibGensTags = true) const
    {
        add_to_scene(texDir.c_str(), scene, merge, includeLibGensTags);
    }

    HL_API void parse(const raw_material_v1& rawMat);

    HL_API void parse(const raw_material_v1& rawMat, const nchar* texsetDir);
    HL_API void parse(const raw_material_v1& rawMat, const nstring& texsetDir);

    HL_API void parse(const raw_material_v3& rawMat);

    HL_API void parse(const void* rawData, const nchar* texsetDir = nullptr);

    inline void parse(const void* rawData, const nstring& texsetDir)
    {
        parse(rawData, texsetDir.c_str());
    }

    HL_API void load(const nchar* filePath);

    inline void load(const nstring& filePath)
    {
        load(filePath.c_str());
    }

    material() = default;

    HL_API material(const void* rawData, const char* name);
    HL_API material(const void* rawData, const std::string& name);
    HL_API material(const void* rawData, std::string&& name);

    HL_API material(const nchar* filePath);

    inline material(const nstring& filePath) :
        material(filePath.c_str()) {}
};
} // mirage
} // hh
} // hl
#endif
