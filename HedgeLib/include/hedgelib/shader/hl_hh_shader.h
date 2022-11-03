#ifndef HL_HH_SHADER_H_INCLUDED
#define HL_HH_SHADER_H_INCLUDED
#include "../hl_resource.h"
#include "../hl_math.h"
#include "../hl_radix_tree.h"

namespace hl
{
namespace hh
{
namespace mirage
{
struct raw_shader_param_constant
{
    /** @brief The name of the shader constant this parameter maps to. */
    off32<char> name;
    /** @brief The index of the first shader register this constant maps to. */
    u8 regFirstIndex;
    /** @brief The number of shader registers this constant maps to. */
    u8 regCount;
    /** @brief Always 0? Padding? */
    u8 unknown1;
    /** @brief Always 0? Padding? */
    u8 unknown2;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
    }
};

HL_STATIC_ASSERT_SIZE(raw_shader_param_constant, 8);

struct raw_shader_param_resource
{
    /** @brief The name of the shader resource this parameter maps to. */
    off32<char> name;
    /** @brief The index of the shader register and texcoord this resource maps to. */
    u8 regAndTexCoordIndex;
    /** @brief Always 0? Padding? */
    u8 unknown1;
    /** @brief Always 0? Padding? */
    u8 unknown2;
    /** @brief Always 0? Padding? */
    u8 unknown3;

    inline u8 reg_index() const noexcept
    {
        return (regAndTexCoordIndex & 0xF);
    }

    inline u8 tex_coord_index() const noexcept
    {
        return ((regAndTexCoordIndex & 0xF0) >> 4);
    }

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
    }
};

HL_STATIC_ASSERT_SIZE(raw_shader_param_resource, 8);

struct raw_shader_params_v2
{
    arr32<off32<raw_shader_param_constant>> floats;
    arr32<off32<raw_shader_param_constant>> ints;
    arr32<off32<raw_shader_param_constant>> bools;
    arr32<off32<raw_shader_param_resource>> textures; // TODO: Are these actually samplers?
    arr32<off32<raw_shader_param_resource>> unknown2; // TODO: Probably another type of shader resource?
    
    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(floats);
        hl::endian_swap<swapOffsets>(ints);
        hl::endian_swap<swapOffsets>(bools);
        hl::endian_swap<swapOffsets>(textures);
        hl::endian_swap<swapOffsets>(unknown2);
    }

    HL_API void fix();
};

HL_STATIC_ASSERT_SIZE(raw_shader_params_v2, 0x28);

struct raw_shader_v2
{
    /**
        @brief The name of the file (without its extension) that
        contains the platform-specific compiled shader code.
    */
    off32<char> codeFileName;
    /** @brief The names of the corresponding vsparam/psparam files, without their extensions. */
    arr32<off32<char>> paramListFileNames;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(codeFileName);
        hl::endian_swap<swapOffsets>(paramListFileNames);
    }

    HL_API void fix();
};

HL_STATIC_ASSERT_SIZE(raw_shader_v2, 12);

enum class raw_vertex_shader_sub_permutation : u32
{
    none = bit_flag<u32>(0U),
    const_tex_coord = bit_flag<u32>(1U),
    all = 3U
};

HL_ENUM_CLASS_DEF_BITWISE_OPS(raw_vertex_shader_sub_permutation)

struct raw_vertex_shader_permutation
{
    raw_vertex_shader_sub_permutation subPermutations;
    off32<char> name;
    off32<char> vertexShaderName;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(subPermutations);
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap<swapOffsets>(vertexShaderName);
    }
};

HL_STATIC_ASSERT_SIZE(raw_vertex_shader_permutation, 12);

enum class raw_pixel_shader_sub_permutation : u32
{
    none = bit_flag<u32>(0U),
    const_tex_coord = bit_flag<u32>(1U),
    no_gi = bit_flag<u32>(2U),
    no_gi_const_tex_coord = bit_flag<u32>(3U),
    no_light = bit_flag<u32>(4U),
    no_light_const_tex_coord = bit_flag<u32>(5U),
    no_light_no_gi = bit_flag<u32>(6U),
    no_light_no_gi_const_tex_coord = bit_flag<u32>(7U),
    all = 0xFFU
};

HL_ENUM_CLASS_DEF_BITWISE_OPS(raw_pixel_shader_sub_permutation)

struct raw_pixel_shader_permutation
{
    raw_pixel_shader_sub_permutation subPermutations;
    off32<char> name;
    off32<char> pixelShaderName;
    arr32<off32<raw_vertex_shader_permutation>> vertexPermutations;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(subPermutations);
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap<swapOffsets>(pixelShaderName);
        hl::endian_swap<swapOffsets>(vertexPermutations);
    }
};

HL_STATIC_ASSERT_SIZE(raw_pixel_shader_permutation, 0x14);

struct raw_shader_list_v0
{
    arr32<off32<raw_pixel_shader_permutation>> pixelPermutations;
    
    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(pixelPermutations);
    }

    HL_API void fix();
};

HL_STATIC_ASSERT_SIZE(raw_shader_list_v0, 8);

struct shader_param_constant
{
    /** @brief The index of the first shader register this constant maps to. */
    u8 regFirstIndex = 0;
    /** @brief The number of shader registers this constant maps to. */
    u8 regCount = 0;

    shader_param_constant() noexcept = default;

    inline shader_param_constant(u8 regFirstIndex, u8 regCount) noexcept :
        regFirstIndex(regFirstIndex),
        regCount(regCount) {}

    inline shader_param_constant(const raw_shader_param_constant& rawShaderConst) noexcept :
        regFirstIndex(rawShaderConst.regFirstIndex),
        regCount(rawShaderConst.regCount) {}
};

struct shader_param_resource
{
    /** @brief The name of the shader resource this parameter maps to. */
    std::string name;
    /** @brief The index of the shader register and texcoord this resource maps to. */
    u8 regAndTexCoordIndex = 0;

    inline u8 reg_index() const noexcept
    {
        return (regAndTexCoordIndex & 0xF);
    }

    inline u8 tex_coord_index() const noexcept
    {
        return ((regAndTexCoordIndex & 0xF0) >> 4);
    }

    inline shader_param_resource(std::string name, u8 regAndTexCoordIndex) noexcept :
        name(std::move(name)),
        regAndTexCoordIndex(regAndTexCoordIndex) {}

    HL_API shader_param_resource(const raw_shader_param_resource& rawShaderRes);
};

class shader_params : public res_base
{
protected:
    HL_API void in_parse(const raw_shader_params_v2& rawShaderParams);

    HL_API void in_parse(const void* rawData);

    HL_API void in_load(const nchar* filePath);

    HL_API void in_clear() noexcept;

public:
    hl::radix_tree<shader_param_constant> floats;
    hl::radix_tree<shader_param_constant> ints;
    hl::radix_tree<shader_param_constant> bools;
    std::vector<shader_param_resource> textures;
    //hl::radix_tree<shader_param_resource> unknown2;

    HL_API static void fix(void* rawData);

    HL_API std::size_t total_constant_count() const noexcept;

    HL_API std::size_t total_resource_count() const noexcept;

    HL_API std::size_t total_param_count() const noexcept;

    /*HL_API const shader_param_constant* get_float_param(const char* name) const;

    HL_API const shader_param_constant* get_float_param(const std::string& name) const;

    inline shader_param_constant* get_float_param(const char* name)
    {
        return const_cast<shader_param_constant*>(const_cast<
            const shader_params*>(this)->get_float_param(name));
    }

    inline shader_param_constant* get_float_param(const std::string& name)
    {
        return const_cast<shader_param_constant*>(const_cast<
            const shader_params*>(this)->get_float_param(name));
    }

    HL_API const shader_param_constant* get_int_param(const char* name) const;

    HL_API const shader_param_constant* get_int_param(const std::string& name) const;

    inline shader_param_constant* get_int_param(const char* name)
    {
        return const_cast<shader_param_constant*>(const_cast<
            const shader_params*>(this)->get_int_param(name));
    }

    inline shader_param_constant* get_int_param(const std::string& name)
    {
        return const_cast<shader_param_constant*>(const_cast<
            const shader_params*>(this)->get_int_param(name));
    }

    HL_API const shader_param_constant* get_bool_param(const char* name) const;

    HL_API const shader_param_constant* get_bool_param(const std::string& name) const;

    inline shader_param_constant* get_bool_param(const char* name)
    {
        return const_cast<shader_param_constant*>(const_cast<
            const shader_params*>(this)->get_bool_param(name));
    }

    inline shader_param_constant* get_bool_param(const std::string& name)
    {
        return const_cast<shader_param_constant*>(const_cast<
            const shader_params*>(this)->get_bool_param(name));
    }*/

    HL_API const shader_param_resource* get_texture_param(const char* name) const;

    HL_API const shader_param_resource* get_texture_param(const std::string& name) const;

    inline shader_param_resource* get_texture_param(const char* name)
    {
        return const_cast<shader_param_resource*>(const_cast<
            const shader_params*>(this)->get_texture_param(name));
    }

    inline shader_param_resource* get_texture_param(const std::string& name)
    {
        return const_cast<shader_param_resource*>(const_cast<
            const shader_params*>(this)->get_texture_param(name));
    }

    HL_API void parse(const void* rawData, std::string name);

    HL_API void load(const nchar* filePath);

    inline void load(const nstring& filePath)
    {
        load(filePath.c_str());
    }

    shader_params() = default;

    HL_API shader_params(const void* rawData, std::string name);

    HL_API shader_params(const nchar* filePath);

    inline shader_params(const nstring& filePath) :
        shader_params(filePath.c_str()) {}
};

struct vertex_shader_params : public shader_params
{
    inline constexpr static nchar ext[] = HL_NTEXT(".vsparam");

    using shader_params::shader_params;
};

struct pixel_shader_params : public shader_params
{
    inline constexpr static nchar ext[] = HL_NTEXT(".psparam");

    using shader_params::shader_params;
};

class shader : public res_base
{
    HL_API void in_parse(const raw_shader_v2& rawShader);

    HL_API void in_parse(const void* rawData);

    HL_API void in_load(const nchar* filePath);

    HL_API void in_clear() noexcept;

public:
    std::string codeName;
    void* codeDataPtr = nullptr;
    std::vector<res_ref<shader_params>> paramLists;

    HL_API static void fix(void* rawData);

    HL_API const shader_param_constant* get_float_param(const char* name) const;

    inline const shader_param_constant* get_float_param(const std::string& name) const
    {
        return get_float_param(name.c_str());
    }

    inline shader_param_constant* get_float_param(const char* name)
    {
        return const_cast<shader_param_constant*>(const_cast<
            const shader*>(this)->get_float_param(name));
    }

    inline shader_param_constant* get_float_param(const std::string& name)
    {
        return get_float_param(name.c_str());
    }

    HL_API const shader_param_constant* get_int_param(const char* name) const;

    inline const shader_param_constant* get_int_param(const std::string& name) const
    {
        return get_int_param(name.c_str());
    }

    inline shader_param_constant* get_int_param(const char* name)
    {
        return const_cast<shader_param_constant*>(const_cast<
            const shader*>(this)->get_int_param(name));
    }

    inline shader_param_constant* get_int_param(const std::string& name)
    {
        return get_int_param(name.c_str());
    }

    HL_API const shader_param_constant* get_bool_param(const char* name) const;

    inline const shader_param_constant* get_bool_param(const std::string& name) const
    {
        return get_bool_param(name.c_str());
    }

    inline shader_param_constant* get_bool_param(const char* name)
    {
        return const_cast<shader_param_constant*>(const_cast<
            const shader*>(this)->get_bool_param(name));
    }

    inline shader_param_constant* get_bool_param(const std::string& name)
    {
        return get_bool_param(name.c_str());
    }

    HL_API const shader_param_resource* get_texture_param(const char* name) const;

    HL_API const shader_param_resource* get_texture_param(const std::string& name) const;

    inline shader_param_resource* get_texture_param(const char* name)
    {
        return const_cast<shader_param_resource*>(const_cast<
            const shader*>(this)->get_texture_param(name));
    }

    inline shader_param_resource* get_texture_param(const std::string& name)
    {
        return const_cast<shader_param_resource*>(const_cast<
            const shader*>(this)->get_texture_param(name));
    }

    inline vec4* get_float(void* paramData, const char* name) const
    {
        const auto param = get_float_param(name);
        return (param) ? (static_cast<vec4*>(paramData) +
            param->regFirstIndex) : nullptr;
    }

    inline vec4* get_float(void* paramData, const std::string& name) const
    {
        return get_float(paramData, name.c_str());
    }

    inline ivec4* get_int(void* paramData, const char* name) const
    {
        const auto param = get_int_param(name);
        return (param) ? (static_cast<ivec4*>(paramData) +
            param->regFirstIndex) : nullptr;
    }

    inline ivec4* get_int(void* paramData, const std::string& name) const
    {
        return get_int(paramData, name.c_str());
    }

    inline bvec4* get_bool(void* paramData, const char* name) const
    {
        const auto param = get_bool_param(name);
        return (param) ? (static_cast<bvec4*>(paramData) +
            param->regFirstIndex) : nullptr;
    }

    inline bvec4* get_bool(void* paramData, const std::string& name) const
    {
        return get_bool(paramData, name.c_str());
    }

    inline void set_float(void* paramData, const char* name, const vec4& v) const
    {
        const auto param = get_float(paramData, name);
        if (param) *param = v;
    }

    inline void set_float(void* paramData, const std::string& name, const vec4& v) const
    {
        set_float(paramData, name.c_str(), v);
    }

    inline void set_float(void* paramData, const char* name, const matrix4x4& v) const
    {
        const auto param = reinterpret_cast<matrix4x4*>(get_float(paramData, name));
        if (param) *param = v;
    }

    inline void set_float(void* paramData, const std::string& name, const matrix4x4& v) const
    {
        set_float(paramData, name.c_str(), v);
    }

    inline void set_int(void* paramData, const char* name, const ivec4& v) const
    {
        const auto param = get_int(paramData, name);
        if (param) *param = v;
    }

    inline void set_int(void* paramData, const std::string& name, const ivec4& v) const
    {
        set_int(paramData, name.c_str(), v);
    }

    inline void set_bool(void* paramData, const char* name, const bvec4& v) const
    {
        const auto param = get_bool(paramData, name);
        if (param) *param = v;
    }

    inline void set_bool(void* paramData, const std::string& name, const bvec4& v) const
    {
        set_bool(paramData, name.c_str(), v);
    }

    HL_API void parse(const void* rawData, std::string name);

    HL_API void load(const nchar* filePath);

    inline void load(const nstring& filePath)
    {
        load(filePath.c_str());
    }

    shader() = default;

    HL_API shader(const void* rawData, std::string name);

    HL_API shader(const nchar* filePath);

    inline shader(const nstring& filePath) :
        shader(filePath.c_str()) {}
};

struct vertex_shader : public shader
{
    inline constexpr static nchar ext[] = HL_NTEXT(".vertexshader");

    using shader::shader;
};

struct pixel_shader : public shader
{
    inline constexpr static nchar ext[] = HL_NTEXT(".pixelshader");

    using shader::shader;
};

using vertex_shader_sub_permutation = raw_vertex_shader_sub_permutation;

inline unsigned int get_vertex_shader_index(
    vertex_shader_sub_permutation subPermutation) noexcept
{
    return bit_ctz(static_cast<unsigned int>(subPermutation));
}

struct vertex_shader_permutation
{
    vertex_shader_sub_permutation subPermutations;
    std::string name;
    std::string vertexShaderName;
    vertex_shader* vertexShaders[2] = {};

    inline bool has_sub_permutation(vertex_shader_sub_permutation subPermutation) const noexcept
    {
        return ((subPermutations & subPermutation) !=
            static_cast<vertex_shader_sub_permutation>(0));
    }

    HL_API vertex_shader_permutation(const raw_vertex_shader_permutation& rawVertexPermutation);
};

using pixel_shader_sub_permutation = raw_pixel_shader_sub_permutation;

inline unsigned int get_pixel_shader_index(
    pixel_shader_sub_permutation subPermutation) noexcept
{
    return bit_ctz(static_cast<unsigned int>(subPermutation));
}

struct pixel_shader_permutation
{
    pixel_shader_sub_permutation subPermutations;
    std::string name;
    std::string pixelShaderName;
    pixel_shader* pixelShaders[8] = {};
    std::vector<vertex_shader_permutation> vertexPermutations;

    inline bool has_sub_permutation(pixel_shader_sub_permutation subPermutation) const noexcept
    {
        return ((subPermutations & subPermutation) !=
            static_cast<pixel_shader_sub_permutation>(0));
    }

    HL_API const vertex_shader_permutation* get_vertex_permutation(const char* name) const;

    HL_API const vertex_shader_permutation* get_vertex_permutation(const std::string& name) const;

    inline vertex_shader_permutation* get_vertex_permutation(const char* name)
    {
        return const_cast<vertex_shader_permutation*>(const_cast<
            const pixel_shader_permutation*>(this)->get_vertex_permutation(name));
    }

    inline vertex_shader_permutation* get_vertex_permutation(const std::string& name)
    {
        return const_cast<vertex_shader_permutation*>(const_cast<
            const pixel_shader_permutation*>(this)->get_vertex_permutation(name));
    }

    pixel_shader_permutation() = default;

    HL_API pixel_shader_permutation(const raw_pixel_shader_permutation& rawPixelPermutation);
};

class shader_list : public res_base
{
    HL_API void in_parse(const raw_shader_list_v0& rawShaderList);

    HL_API void in_parse(const void* rawData);

    HL_API void in_load(const nchar* filePath);

public:
    std::vector<pixel_shader_permutation> pixelPermutations;

    inline constexpr static nchar ext[] = HL_NTEXT(".shader-list");

    HL_API static void fix(void* rawData);

    HL_API const pixel_shader_permutation* get_pixel_permutation(const char* name) const;

    HL_API const pixel_shader_permutation* get_pixel_permutation(const std::string& name) const;

    inline pixel_shader_permutation* get_pixel_permutation(const char* name)
    {
        return const_cast<pixel_shader_permutation*>(const_cast<
            const shader_list*>(this)->get_pixel_permutation(name));
    }

    inline pixel_shader_permutation* get_pixel_permutation(const std::string& name)
    {
        return const_cast<pixel_shader_permutation*>(const_cast<
            const shader_list*>(this)->get_pixel_permutation(name));
    }

    HL_API void parse(const void* rawData, std::string name);

    HL_API void load(const nchar* filePath);

    inline void load(const nstring& filePath)
    {
        load(filePath.c_str());
    }

    shader_list() = default;

    HL_API shader_list(const void* rawData, std::string name);

    HL_API shader_list(const nchar* filePath);

    inline shader_list(const nstring& filePath) :
        shader_list(filePath.c_str()) {}
};
} // mirage

namespace needle
{
constexpr u8 sig[8] = { 'H', 'H', 'N', 'E', 'E', 'D', 'L', 'E' };

struct raw_header
{
    /** @brief "HHNEEDLE" signature. */
    u8 signature[8];
    /** @brief The complete size of this data, including this "size" value. */
    u32 size;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(size);
    }

    inline const char* build_path() const noexcept
    {
        return reinterpret_cast<const char*>(this + 1);
    }

    inline char* build_path() noexcept
    {
        return reinterpret_cast<char*>(this + 1);
    }
};

HL_STATIC_ASSERT_SIZE(raw_header, 12);

//constexpr u8 sig[8] = { 'H', 'N', 'S', 'H', 'V', '0', '0', '2' };

struct raw_resource
{
    /** @brief Array count?? */
    u32 unknown1; // TODO: Figure this out.
    u32 registerIndex;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(unknown1);
        hl::endian_swap(registerIndex);
    }

    inline const char* name() const noexcept
    {
        return reinterpret_cast<const char*>(this + 1);
    }

    inline char* name() noexcept
    {
        return reinterpret_cast<char*>(this + 1);
    }
};

HL_STATIC_ASSERT_SIZE(raw_resource, 8);

struct raw_constant_buffer
{
    /** @brief Array count?? */
    u32 unknown1; // TODO: Figure this out.
    u32 constBufSize;
    u32 registerIndex;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(unknown1);
        hl::endian_swap(constBufSize);
        hl::endian_swap(registerIndex);
    }

    inline const char* name() const noexcept
    {
        return reinterpret_cast<const char*>(this + 1);
    }

    inline char* name() noexcept
    {
        return reinterpret_cast<char*>(this + 1);
    }
};

HL_STATIC_ASSERT_SIZE(raw_constant_buffer, 12);

struct raw_constant
{
    /** @brief Array count?? */
    u32 unknown1; // TODO: Figure this out.
    u32 constBufIndex;
    u32 dataOffset;
    u32 dataSize;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(unknown1);
        hl::endian_swap(constBufIndex);
        hl::endian_swap(dataOffset);
        hl::endian_swap(dataSize);
    }

    inline const char* name() const noexcept
    {
        return reinterpret_cast<const char*>(this + 1);
    }

    inline char* name() noexcept
    {
        return reinterpret_cast<char*>(this + 1);
    }
};

HL_STATIC_ASSERT_SIZE(raw_constant, 16);

struct raw_params_container
{
    /** @brief The complete size of this data, including this "size" value. */
    u32 size;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(size);
    }

    inline const void* first_param() const noexcept
    {
        return (this + 1);
    }

    inline void* first_param() noexcept
    {
        return (this + 1);
    }

    inline const void* end() const noexcept
    {
        return ptradd(this, size);
    }

    inline void* end() noexcept
    {
        return ptradd(this, size);
    }
};

HL_STATIC_ASSERT_SIZE(raw_params_container, 4);

enum class raw_param_type : u32
{
    res_texture = 3U,
    res_sampler = 4U, // TODO: Is this correct?
    constant_buffer = 5U,
    const_bool = 6U,
    const_int = 7U,
    const_float = 8U
};

struct raw_typed_params
{
    /** @brief See hl::hh::needle::raw_param_type. */
    u32 type;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(type);
    }

    inline const raw_params_container& params() const noexcept
    {
        return *reinterpret_cast<const raw_params_container*>(this + 1);
    }

    inline raw_params_container& params() noexcept
    {
        return *reinterpret_cast<raw_params_container*>(this + 1);
    }
};

HL_STATIC_ASSERT_SIZE(raw_typed_params, 4);

struct raw_typed_params_container
{
    /** @brief The complete size of this data, including this "size" value. */
    u32 size;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(size);
    }

    inline const raw_typed_params* first_type() const noexcept
    {
        return reinterpret_cast<const raw_typed_params*>(this + 1);
    }

    inline raw_typed_params* first_type() noexcept
    {
        return reinterpret_cast<raw_typed_params*>(this + 1);
    }

    const raw_typed_params_container* next() const noexcept
    {
        return ptradd<raw_typed_params_container>(this, size);
    }

    raw_typed_params_container* next() noexcept
    {
        return const_cast<raw_typed_params_container*>(const_cast<
            const raw_typed_params_container*>(this)->next());
    }
};

HL_STATIC_ASSERT_SIZE(raw_typed_params_container, 4);

struct raw_code_container
{
    /** @brief The size of the shader data, *NOT* including this "size" value. */
    u32 size;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(size);
    }

    inline const void* data() const noexcept
    {
        return (this + 1);
    }

    inline void* data() noexcept
    {
        return (this + 1);
    }
};

HL_STATIC_ASSERT_SIZE(raw_code_container, 4);

struct raw_shader_variant
{
    /** @brief The complete size of this data, including this "size" value. */
    u32 size;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(size);
    }

    inline const raw_code_container* bytecode() const noexcept
    {
        return reinterpret_cast<const raw_code_container*>(this + 1);
    }

    inline raw_code_container* bytecode() noexcept
    {
        return reinterpret_cast<raw_code_container*>(this + 1);
    }

    HL_API const raw_code_container* input_bytecode() const noexcept;
    HL_API raw_code_container* input_bytecode() noexcept;
    HL_API const raw_typed_params_container* typed_params() const noexcept;
    HL_API raw_typed_params_container* typed_params() noexcept;

    const raw_shader_variant* next() const noexcept
    {
        return ptradd<raw_shader_variant>(this, size);
    }

    raw_shader_variant* next() noexcept
    {
        return const_cast<raw_shader_variant*>(const_cast<
            const raw_shader_variant*>(this)->next());
    }
};

HL_STATIC_ASSERT_SIZE(raw_shader_variant, 4);

enum class raw_permutation_type
{
    permutation = 0U, // TODO: This needs a better name. Is this actually a pixel permutation?
    texture = 1U // TODO: Is this correct? Is this actually a vertex permutation?
};

struct raw_permutation
{
    /** @brief See hl::hh::needle::raw_permutation_type. */
    u32 type;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(type);
    }

    inline const char* name() const noexcept
    {
        return reinterpret_cast<const char*>(this + 1);
    }

    inline char* name() noexcept
    {
        return reinterpret_cast<char*>(this + 1);
    }

    HL_API const raw_permutation* next() const noexcept;

    raw_permutation* next() noexcept
    {
        return const_cast<raw_permutation*>(const_cast<
            const raw_permutation*>(this)->next());
    }
};

HL_STATIC_ASSERT_SIZE(raw_permutation, 4);

struct raw_shader_v2
{
    /** @brief "HNSH" signature. */
    u8 signature[4];
    /** @brief "V002" string. */
    u8 version[4];
    /** @brief The complete size of this data, including this "size" value. */
    u32 size;
    /** @brief Always 0? */
    u32 unknown1; // TODO: Is permutationCount just a u64? If so, why is there not padding to 8??
    u32 permutationCount;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(size);
        hl::endian_swap(unknown1);
        hl::endian_swap(permutationCount);
    }

    u32 variant_count() const noexcept
    {
        return (1U << permutationCount);
    }

    inline const raw_permutation* first_permutation() const noexcept
    {
        return reinterpret_cast<const raw_permutation*>(this + 1);
    }

    inline raw_permutation* first_permutation() noexcept
    {
        return reinterpret_cast<raw_permutation*>(this + 1);
    }

    HL_API void fix();
};

HL_STATIC_ASSERT_SIZE(raw_shader_v2, 0x14);
} // needle
} // hh
} // hl
#endif
