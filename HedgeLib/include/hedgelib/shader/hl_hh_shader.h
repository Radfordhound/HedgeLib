#ifndef HL_HH_SHADER_H_INCLUDED
#define HL_HH_SHADER_H_INCLUDED
#include "../hl_internal.h"

namespace hl
{
namespace hh
{
namespace mirage
{
struct raw_shader_param_resource
{
    /** @brief The name of the shader resource this parameter maps to. */
    off32<char> name;
    /** @brief The index of the shader register this resource maps to. */
    u8 regIndex;
    /** @brief Always 0? Padding? */
    u8 unknown1;
    /** @brief Always 0? Padding? */
    u8 unknown2;
    /** @brief Always 0? Padding? */
    u8 unknown3;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
    }
};

HL_STATIC_ASSERT_SIZE(raw_shader_param_resource, 8);

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

struct raw_shader_param_v2
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
    // TODO: Add parse function.
};

HL_STATIC_ASSERT_SIZE(raw_shader_param_v2, 0x28);

enum class raw_vertex_shader_sub_permutation : u32
{
    none = bit_flag<u32>(0U),
    const_tex_coord = bit_flag<u32>(1U),
    all = 3U
};

struct raw_vertex_shader_permutation
{
    /** @brief See hl::hh::mirage::raw_vertex_shader_sub_permutation. */
    u32 subPermutations;
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

struct raw_pixel_shader_permutation
{
    /** @brief See hl::hh::mirage::raw_pixel_shader_sub_permutation. */
    u32 subPermutations;
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
    // TODO: Add parse function.
};

HL_STATIC_ASSERT_SIZE(raw_shader_list_v0, 8);

struct raw_shader_v2
{
    /**
        @brief The name of the file (without its extension) that
        contains the platform-specific compiled shader code.
    */
    off32<char> codeFileName;
    /** @brief The names of the corresponding vsparam/psparam files, without their extensions. */
    arr32<off32<char>> paramFileNames;
    
    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(codeFileName);
        hl::endian_swap<swapOffsets>(paramFileNames);
    }

    HL_API void fix();
    // TODO: Add parse function.
};

HL_STATIC_ASSERT_SIZE(raw_shader_v2, 12);
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
