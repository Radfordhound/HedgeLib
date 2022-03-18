#include "hedgelib/models/hl_hh_model.h"
#include "hedgelib/materials/hl_hh_material.h"
#include "hedgelib/io/hl_file.h"
#include "hedgelib/io/hl_path.h"
#include "hedgelib/hl_blob.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <cstring>

namespace hl
{
namespace hh
{
namespace mirage
{
void raw_vertex_element::convert_to_vec4(const void* vtx, vec4& vec) const
{
    switch (static_cast<raw_vertex_format>(format))
    {
    case raw_vertex_format::float1:
    {
        const float* f = static_cast<const float*>(vtx);
        vec = vec4(f[0], 0.0f, 0.0f, 0.0f);
        break;
    }

    case raw_vertex_format::float2:
    {
        const float* f = static_cast<const float*>(vtx);
        vec = vec4(f[0], f[1], 0.0f, 0.0f);
        break;
    }

    case raw_vertex_format::float3:
    {
        const float* f = static_cast<const float*>(vtx);
        vec = vec4(f[0], f[1], f[2], 0.0f);
        break;
    }

    case raw_vertex_format::float4:
    {
        const float* f = static_cast<const float*>(vtx);
        vec = vec4(f[0], f[1], f[2], f[3]);
        break;
    }

    case raw_vertex_format::int1:
    {
        const s32* v = static_cast<const s32*>(vtx);
        vec = vec4(static_cast<float>(v[0]), 0.0f, 0.0f, 0.0f);
        break;
    }

    case raw_vertex_format::int2:
    {
        const s32* v = static_cast<const s32*>(vtx);
        vec = vec4(static_cast<float>(v[0]),
            static_cast<float>(v[1]), 0.0f, 0.0f);
        break;
    }

    case raw_vertex_format::int4:
    {
        const s32* v = static_cast<const s32*>(vtx);
        vec = vec4(static_cast<float>(v[0]),
            static_cast<float>(v[1]),
            static_cast<float>(v[2]),
            static_cast<float>(v[3]));
        break;
    }

    case raw_vertex_format::uint1:
    {
        const u32* v = static_cast<const u32*>(vtx);
        vec = vec4(static_cast<float>(v[0]), 0.0f, 0.0f, 0.0f);
        break;
    }

    case raw_vertex_format::uint2:
    {
        const u32* v = static_cast<const u32*>(vtx);
        vec = vec4(static_cast<float>(v[0]),
            static_cast<float>(v[1]), 0.0f, 0.0f);
        break;
    }

    case raw_vertex_format::uint4:
    {
        const u32* v = static_cast<const u32*>(vtx);
        vec = vec4(static_cast<float>(v[0]),
            static_cast<float>(v[1]),
            static_cast<float>(v[2]),
            static_cast<float>(v[3]));
        break;
    }

    case raw_vertex_format::int1_norm:
    {
        const u32* v = static_cast<const u32*>(vtx);
        vec = vec4(math::snorm_to_float(v[0]), 0.0f, 0.0f, 0.0f);
        break;
    }

    case raw_vertex_format::int2_norm:
    {
        const u32* v = static_cast<const u32*>(vtx);
        vec = vec4(math::snorm_to_float(v[0]),
            math::snorm_to_float(v[1]), 0.0f, 0.0f);
        break;
    }

    case raw_vertex_format::int4_norm:
    {
        const u32* v = static_cast<const u32*>(vtx);
        vec = vec4(math::snorm_to_float(v[0]),
            math::snorm_to_float(v[1]),
            math::snorm_to_float(v[2]),
            math::snorm_to_float(v[3]));
        break;
    }

    case raw_vertex_format::uint1_norm:
    {
        const u32* v = static_cast<const u32*>(vtx);
        vec = vec4(math::unorm_to_float(v[0]), 0.0f, 0.0f, 0.0f);
        break;
    }

    case raw_vertex_format::uint2_norm:
    {
        const u32* v = static_cast<const u32*>(vtx);
        vec = vec4(math::unorm_to_float(v[0]),
            math::unorm_to_float(v[1]), 0.0f, 0.0f);
        break;
    }

    case raw_vertex_format::uint4_norm:
    {
        const u32* v = static_cast<const u32*>(vtx);
        vec = vec4(math::unorm_to_float(v[0]),
            math::unorm_to_float(v[1]),
            math::unorm_to_float(v[2]),
            math::unorm_to_float(v[3]));
        break;
    }

    case raw_vertex_format::d3d_color:
    {
        // TODO: Is this correct?
        const u8* v = static_cast<const u8*>(vtx);
        vec = vec4(math::unorm_to_float(v[0]),
            math::unorm_to_float(v[1]),
            math::unorm_to_float(v[2]),
            math::unorm_to_float(v[3]));
        break;
    }

    case raw_vertex_format::ubyte4:
    {
        const u8* v = static_cast<const u8*>(vtx);
        vec = vec4(v[0], v[1], v[2], v[3]);
        break;
    }

    case raw_vertex_format::byte4:
    {
        const s8* v = static_cast<const s8*>(vtx);
        vec = vec4(v[0], v[1], v[2], v[3]);
        break;
    }

    case raw_vertex_format::ubyte4_norm:
    {
        const u8* v = static_cast<const u8*>(vtx);
        vec = vec4(math::unorm_to_float(v[0]),
            math::unorm_to_float(v[1]),
            math::unorm_to_float(v[2]),
            math::unorm_to_float(v[3]));
        break;
    }

    case raw_vertex_format::byte4_norm:
    {
        const u8* v = static_cast<const u8*>(vtx);
        vec = vec4(math::snorm_to_float(v[0]),
            math::snorm_to_float(v[1]),
            math::snorm_to_float(v[2]),
            math::snorm_to_float(v[3]));
        break;
    }

    case raw_vertex_format::short2:
    {
        const s16* v = static_cast<const s16*>(vtx);
        vec = vec4(v[0], v[1], 0.0f, 0.0f);
        break;
    }

    case raw_vertex_format::short4:
    {
        const s16* v = static_cast<const s16*>(vtx);
        vec = vec4(v[0], v[1], v[2], v[3]);
        break;
    }

    case raw_vertex_format::ushort2:
    {
        const u16* v = static_cast<const u16*>(vtx);
        vec = vec4(v[0], v[1], 0.0f, 0.0f);
        break;
    }

    case raw_vertex_format::ushort4:
    {
        const u16* v = static_cast<const u16*>(vtx);
        vec = vec4(v[0], v[1], v[2], v[3]);
        break;
    }

    case raw_vertex_format::short2_norm:
    {
        const u16* v = static_cast<const u16*>(vtx);
        vec = vec4(math::snorm_to_float(v[0]),
            math::snorm_to_float(v[1]), 0.0f, 0.0f);
        break;
    }

    case raw_vertex_format::short4_norm:
    {
        const u16* v = static_cast<const u16*>(vtx);
        vec = vec4(math::snorm_to_float(v[0]),
            math::snorm_to_float(v[1]),
            math::snorm_to_float(v[2]),
            math::snorm_to_float(v[3]));
        break;
    }

    case raw_vertex_format::ushort2_norm:
    {
        const u16* v = static_cast<const u16*>(vtx);
        vec = vec4(math::unorm_to_float(v[0]),
            math::unorm_to_float(v[1]), 0.0f, 0.0f);
        break;
    }

    case raw_vertex_format::ushort4_norm:
    {
        const u16* v = static_cast<const u16*>(vtx);
        vec = vec4(math::unorm_to_float(v[0]),
            math::unorm_to_float(v[1]),
            math::unorm_to_float(v[2]),
            math::unorm_to_float(v[3]));
        break;
    }

    case raw_vertex_format::udec3:
    {
        // TODO: Is this correct??
        const u32 v = *static_cast<const u32*>(vtx);
        vec = vec4(static_cast<float>(v & 0x3FF),
            static_cast<float>((v >> 10) & 0x3FF),
            static_cast<float>((v >> 20) & 0x3FF),
            0.0f);
        break;
    }

    //case raw_vertex_format::dec3: // TODO

    case raw_vertex_format::udec3_norm:
    {
        // TODO: Is this correct??
        const u32 v = *static_cast<const u32*>(vtx);
        vec = vec4(math::unorm_to_float<10>(v),
            math::unorm_to_float<10>(v >> 10),
            math::unorm_to_float<10>(v >> 20),
            0.0f);
        break;
    }

    case raw_vertex_format::dec3_norm:
    {
        // TODO: Is this correct??
        const u32 v = *static_cast<const u32*>(vtx);
        vec = vec4(math::snorm_to_float<10>(v),
            math::snorm_to_float<10>(v >> 10),
            math::snorm_to_float<10>(v >> 20),
            0.0f);
        break;
    }

    // TODO
    //case raw_vertex_format::udec4:
    //case raw_vertex_format::dec4:

    case raw_vertex_format::udec4_norm:
    {
        // TODO: Is this correct??
        const u32 v = *static_cast<const u32*>(vtx);
        vec = vec4(math::unorm_to_float<10>(v),
            math::unorm_to_float<10>(v >> 10),
            math::unorm_to_float<10>(v >> 20),
            math::unorm_to_float<2>(v >> 30));
        break;
    }

    case raw_vertex_format::dec4_norm:
    {
        // TODO: Is this correct??
        const u32 v = *static_cast<const u32*>(vtx);
        vec = vec4(math::snorm_to_float<10>(v),
            math::snorm_to_float<10>(v >> 10),
            math::snorm_to_float<10>(v >> 20),
            math::snorm_to_float<2>(v >> 30));
        break;
    }
    // TODO
    //case raw_vertex_format::uhend3:
    //case raw_vertex_format::hend3:

    case raw_vertex_format::uhend3_norm:
    {
        // TODO: Is this correct??
        const u32 v = *static_cast<const u32*>(vtx);
        vec = vec4(math::unorm_to_float<11>(v),
            math::unorm_to_float<11>(v >> 11),
            math::unorm_to_float<10>(v >> 22),
            0.0f);
        break;
    }

    case raw_vertex_format::hend3_norm:
    {
        // TODO: Is this correct??
        const u32 v = *static_cast<const u32*>(vtx);
        vec = vec4(math::snorm_to_float<11>(v),
            math::snorm_to_float<11>(v >> 11),
            math::snorm_to_float<10>(v >> 22),
            0.0f);
        break;
    }

    // TODO
    /*case raw_vertex_format::udhen3:
    case raw_vertex_format::dhen3:*/

    case raw_vertex_format::udhen3_norm:
    {
        // TODO: Is this correct??
        const u32 v = *static_cast<const u32*>(vtx);
        vec = vec4(math::unorm_to_float<10>(v),
            math::unorm_to_float<11>(v >> 10),
            math::unorm_to_float<11>(v >> 21),
            0.0f);
        break;
    }

    case raw_vertex_format::dhen3_norm:
    {
        // TODO: Is this correct??
        const u32 v = *static_cast<const u32*>(vtx);
        vec = vec4(math::snorm_to_float<10>(v),
            math::snorm_to_float<11>(v >> 10),
            math::snorm_to_float<11>(v >> 21),
            0.0f);
        break;
    }

    case raw_vertex_format::float16_2:
    {
        const glm::uint v = *static_cast<const glm::uint*>(vtx);
        const glm::vec2 unpackedV = glm::unpackHalf2x16(v);

        vec = vec4(unpackedV[0], unpackedV[1], 0.0f, 0.0f);
        break;
    }

    case raw_vertex_format::float16_4:
    {
        const glm::uint v = *static_cast<const glm::uint*>(vtx);
        const glm::vec2 unpackedV = glm::unpackHalf2x16(v);

        vec = vec4(unpackedV[0], unpackedV[1], unpackedV[2], unpackedV[3]);
        break;
    }

    default:
        throw std::runtime_error("Unknown or unsupported vertex format.");
    }
}

void raw_vertex_element::convert_to_ivec4(const void* vtx, ivec4& ivec) const
{
    switch (static_cast<raw_vertex_format>(format))
    {
    case raw_vertex_format::float1:
    {
        const float* f = static_cast<const float*>(vtx);
        ivec = ivec4(static_cast<int>(f[0]), 0, 0, 0);
        break;
    }

    case raw_vertex_format::float2:
    {
        const float* f = static_cast<const float*>(vtx);
        ivec = ivec4(static_cast<int>(f[0]),
            static_cast<int>(f[1]), 0, 0);
        break;
    }

    case raw_vertex_format::float3:
    {
        const float* f = static_cast<const float*>(vtx);
        ivec = ivec4(static_cast<int>(f[0]),
            static_cast<int>(f[1]),
            static_cast<int>(f[2]), 0);
        break;
    }

    case raw_vertex_format::float4:
    {
        const float* f = static_cast<const float*>(vtx);
        ivec = ivec4(static_cast<int>(f[0]),
            static_cast<int>(f[1]),
            static_cast<int>(f[2]),
            static_cast<int>(f[3]));
        break;
    }

    case raw_vertex_format::int1:
    {
        const s32* v = static_cast<const s32*>(vtx);
        ivec = ivec4(static_cast<int>(v[0]), 0, 0, 0);
        break;
    }

    case raw_vertex_format::int2:
    {
        const s32* v = static_cast<const s32*>(vtx);
        ivec = ivec4(static_cast<int>(v[0]),
            static_cast<int>(v[1]), 0, 0);
        break;
    }

    case raw_vertex_format::int4:
    {
        const s32* v = static_cast<const s32*>(vtx);
        ivec = ivec4(v[0], v[1], v[2], v[3]);
        break;
    }

    case raw_vertex_format::uint1:
    {
        const u32* v = static_cast<const u32*>(vtx);
        ivec = ivec4(static_cast<int>(v[0]), 0, 0, 0);
        break;
    }

    case raw_vertex_format::uint2:
    {
        const u32* v = static_cast<const u32*>(vtx);
        ivec = ivec4(static_cast<int>(v[0]),
            static_cast<int>(v[1]), 0, 0);
        break;
    }

    case raw_vertex_format::uint4:
    {
        const u32* v = static_cast<const u32*>(vtx);
        ivec = ivec4(v[0], v[1], v[2], v[3]);
        break;
    }

    case raw_vertex_format::int1_norm:
    {
        const u32* v = static_cast<const u32*>(vtx);
        ivec = ivec4(static_cast<int>(math::snorm_to_float(v[0])),
            0, 0, 0);
        break;
    }

    case raw_vertex_format::int2_norm:
    {
        const u32* v = static_cast<const u32*>(vtx);
        ivec = ivec4(static_cast<int>(math::snorm_to_float(v[0])),
            static_cast<int>(math::snorm_to_float(v[1])), 0, 0);
        break;
    }

    case raw_vertex_format::int4_norm:
    {
        const u32* v = static_cast<const u32*>(vtx);
        ivec = ivec4(static_cast<int>(math::snorm_to_float(v[0])),
            static_cast<int>(math::snorm_to_float(v[1])),
            static_cast<int>(math::snorm_to_float(v[2])),
            static_cast<int>(math::snorm_to_float(v[3])));
        break;
    }

    case raw_vertex_format::uint1_norm:
    {
        const u32* v = static_cast<const u32*>(vtx);
        ivec = ivec4(static_cast<int>(math::unorm_to_float(v[0])),
            0, 0, 0);
        break;
    }

    case raw_vertex_format::uint2_norm:
    {
        const u32* v = static_cast<const u32*>(vtx);
        ivec = ivec4(static_cast<int>(math::unorm_to_float(v[0])),
            static_cast<int>(math::unorm_to_float(v[1])), 0, 0);
        break;
    }

    case raw_vertex_format::uint4_norm:
    {
        const u32* v = static_cast<const u32*>(vtx);
        ivec = ivec4(static_cast<int>(math::unorm_to_float(v[0])),
            static_cast<int>(math::unorm_to_float(v[1])),
            static_cast<int>(math::unorm_to_float(v[2])),
            static_cast<int>(math::unorm_to_float(v[3])));
        break;
    }

    case raw_vertex_format::d3d_color:
    {
        // TODO: Is this correct?
        const u8* v = static_cast<const u8*>(vtx);
        ivec = ivec4(static_cast<int>(math::unorm_to_float(v[0])),
            static_cast<int>(math::unorm_to_float(v[1])),
            static_cast<int>(math::unorm_to_float(v[2])),
            static_cast<int>(math::unorm_to_float(v[3])));
        break;
    }

    case raw_vertex_format::ubyte4:
    {
        const u8* v = static_cast<const u8*>(vtx);
        ivec = ivec4(v[0], v[1], v[2], v[3]);
        break;
    }

    case raw_vertex_format::byte4:
    {
        const s8* v = static_cast<const s8*>(vtx);
        ivec = ivec4(v[0], v[1], v[2], v[3]);
        break;
    }

    case raw_vertex_format::ubyte4_norm:
    {
        const u8* v = static_cast<const u8*>(vtx);
        ivec = ivec4(static_cast<int>(math::unorm_to_float(v[0])),
            static_cast<int>(math::unorm_to_float(v[1])),
            static_cast<int>(math::unorm_to_float(v[2])),
            static_cast<int>(math::unorm_to_float(v[3])));
        break;
    }

    case raw_vertex_format::byte4_norm:
    {
        const u8* v = static_cast<const u8*>(vtx);
        ivec = ivec4(static_cast<int>(math::snorm_to_float(v[0])),
            static_cast<int>(math::snorm_to_float(v[1])),
            static_cast<int>(math::snorm_to_float(v[2])),
            static_cast<int>(math::snorm_to_float(v[3])));
        break;
    }

    case raw_vertex_format::short2:
    {
        const s16* v = static_cast<const s16*>(vtx);
        ivec = ivec4(v[0], v[1], 0, 0);
        break;
    }

    case raw_vertex_format::short4:
    {
        const s16* v = static_cast<const s16*>(vtx);
        ivec = ivec4(v[0], v[1], v[2], v[3]);
        break;
    }

    case raw_vertex_format::ushort2:
    {
        const u16* v = static_cast<const u16*>(vtx);
        ivec = ivec4(v[0], v[1], 0, 0);
        break;
    }

    case raw_vertex_format::ushort4:
    {
        const u16* v = static_cast<const u16*>(vtx);
        ivec = ivec4(v[0], v[1], v[2], v[3]);
        break;
    }

    case raw_vertex_format::short2_norm:
    {
        const u16* v = static_cast<const u16*>(vtx);
        ivec = ivec4(static_cast<int>(math::snorm_to_float(v[0])),
            static_cast<int>(math::snorm_to_float(v[1])), 0, 0);
        break;
    }

    case raw_vertex_format::short4_norm:
    {
        const u16* v = static_cast<const u16*>(vtx);
        ivec = ivec4(static_cast<int>(math::snorm_to_float(v[0])),
            static_cast<int>(math::snorm_to_float(v[1])),
            static_cast<int>(math::snorm_to_float(v[2])),
            static_cast<int>(math::snorm_to_float(v[3])));
        break;
    }

    case raw_vertex_format::ushort2_norm:
    {
        const u16* v = static_cast<const u16*>(vtx);
        ivec = ivec4(static_cast<int>(math::unorm_to_float(v[0])),
            static_cast<int>(math::unorm_to_float(v[1])), 0, 0);
        break;
    }

    case raw_vertex_format::ushort4_norm:
    {
        const u16* v = static_cast<const u16*>(vtx);
        ivec = ivec4(static_cast<int>(math::unorm_to_float(v[0])),
            static_cast<int>(math::unorm_to_float(v[1])),
            static_cast<int>(math::unorm_to_float(v[2])),
            static_cast<int>(math::unorm_to_float(v[3])));
        break;
    }

    case raw_vertex_format::udec3:
    {
        // TODO: Is this correct??
        const u32 v = *static_cast<const u32*>(vtx);
        ivec = ivec4(v & 0x3FF,
            (v >> 10) & 0x3FF,
            (v >> 20) & 0x3FF,
            0);
        break;
    }

    //case raw_vertex_format::dec3: // TODO

    case raw_vertex_format::udec3_norm:
    {
        // TODO: Is this correct??
        const u32 v = *static_cast<const u32*>(vtx);
        ivec = ivec4(static_cast<int>(math::unorm_to_float<10>(v)),
            static_cast<int>(math::unorm_to_float<10>(v >> 10)),
            static_cast<int>(math::unorm_to_float<10>(v >> 20)),
            0);
        break;
    }

    case raw_vertex_format::dec3_norm:
    {
        // TODO: Is this correct??
        const u32 v = *static_cast<const u32*>(vtx);
        ivec = ivec4(static_cast<int>(math::snorm_to_float<10>(v)),
            static_cast<int>(math::snorm_to_float<10>(v >> 10)),
            static_cast<int>(math::snorm_to_float<10>(v >> 20)),
            0);
        break;
    }

    // TODO
    //case raw_vertex_format::udec4:
    //case raw_vertex_format::dec4:

    case raw_vertex_format::udec4_norm:
    {
        // TODO: Is this correct??
        const u32 v = *static_cast<const u32*>(vtx);
        ivec = ivec4(static_cast<int>(math::unorm_to_float<10>(v)),
            static_cast<int>(math::unorm_to_float<10>(v >> 10)),
            static_cast<int>(math::unorm_to_float<10>(v >> 20)),
            static_cast<int>(math::unorm_to_float<2>(v >> 30)));
        break;
    }

    case raw_vertex_format::dec4_norm:
    {
        // TODO: Is this correct??
        const u32 v = *static_cast<const u32*>(vtx);
        ivec = ivec4(static_cast<int>(math::snorm_to_float<10>(v)),
            static_cast<int>(math::snorm_to_float<10>(v >> 10)),
            static_cast<int>(math::snorm_to_float<10>(v >> 20)),
            static_cast<int>(math::snorm_to_float<2>(v >> 30)));
        break;
    }
    // TODO
    //case raw_vertex_format::uhend3:
    //case raw_vertex_format::hend3:

    case raw_vertex_format::uhend3_norm:
    {
        // TODO: Is this correct??
        const u32 v = *static_cast<const u32*>(vtx);
        ivec = ivec4(static_cast<int>(math::unorm_to_float<11>(v)),
            static_cast<int>(math::unorm_to_float<11>(v >> 11)),
            static_cast<int>(math::unorm_to_float<10>(v >> 22)),
            0);
        break;
    }

    case raw_vertex_format::hend3_norm:
    {
        // TODO: Is this correct??
        const u32 v = *static_cast<const u32*>(vtx);
        ivec = ivec4(static_cast<int>(math::snorm_to_float<11>(v)),
            static_cast<int>(math::snorm_to_float<11>(v >> 11)),
            static_cast<int>(math::snorm_to_float<10>(v >> 22)),
            0);
        break;
    }

    // TODO
    /*case raw_vertex_format::udhen3:
    case raw_vertex_format::dhen3:*/

    case raw_vertex_format::udhen3_norm:
    {
        // TODO: Is this correct??
        const u32 v = *static_cast<const u32*>(vtx);
        ivec = ivec4(static_cast<int>(math::unorm_to_float<10>(v)),
            static_cast<int>(math::unorm_to_float<11>(v >> 10)),
            static_cast<int>(math::unorm_to_float<11>(v >> 21)),
            0);
        break;
    }

    case raw_vertex_format::dhen3_norm:
    {
        // TODO: Is this correct??
        const u32 v = *static_cast<const u32*>(vtx);
        ivec = ivec4(static_cast<int>(math::snorm_to_float<10>(v)),
            static_cast<int>(math::snorm_to_float<11>(v >> 10)),
            static_cast<int>(math::snorm_to_float<11>(v >> 21)),
            0);
        break;
    }

    case raw_vertex_format::float16_2:
    {
        const glm::uint v = *static_cast<const glm::uint*>(vtx);
        const glm::vec2 unpackedV = glm::unpackHalf2x16(v);

        ivec = ivec4(static_cast<int>(unpackedV[0]),
            static_cast<int>(unpackedV[1]), 0, 0);
        break;
    }

    case raw_vertex_format::float16_4:
    {
        const glm::uint v = *static_cast<const glm::uint*>(vtx);
        const glm::vec2 unpackedV = glm::unpackHalf2x16(v);

        ivec = ivec4(static_cast<int>(unpackedV[0]),
            static_cast<int>(unpackedV[1]),
            static_cast<int>(unpackedV[2]),
            static_cast<int>(unpackedV[3]));
        break;
    }

    default:
        throw std::runtime_error("Unknown or unsupported vertex format.");
    }
}

bool raw_terrain_model_v5::is_revision2() const
{
    // HACK: Since Sonic Team didn't bother upping the version number, there's no
    // "clean" way to check which revision of the format this is. So, we check to
    // see if any of the pointers in the struct point *before* where the revision 2
    // struct is supposed to end. If so, it must be a revision 1 struct.
    const std::uintptr_t rev2_endAddr = reinterpret_cast<
        std::uintptr_t>(ptradd(this, sizeof(*this)));

    const std::uintptr_t meshGroupsAddr = reinterpret_cast<
        std::uintptr_t>(meshGroups.get());

    const std::uintptr_t nameAddr = reinterpret_cast<
        std::uintptr_t>(name.get());

    return (meshGroupsAddr >= rev2_endAddr && nameAddr >= rev2_endAddr);
}

static void in_swap_vertex(const raw_vertex_element& rawVtxElem, void* rawVtx)
{
    // Swap vertex based on vertex element.
    switch (static_cast<raw_vertex_format>(rawVtxElem.format))
    {
    case raw_vertex_format::float1:
        endian_swap(*static_cast<float*>(rawVtx));
        break;

    case raw_vertex_format::float2:
        endian_swap(*static_cast<vec2*>(rawVtx));
        break;

    case raw_vertex_format::float3:
        endian_swap(*static_cast<vec3*>(rawVtx));
        break;

    case raw_vertex_format::float4:
        endian_swap(*static_cast<vec4*>(rawVtx));
        break;

    case raw_vertex_format::int1:
    case raw_vertex_format::int1_norm:
    case raw_vertex_format::uint1:
    case raw_vertex_format::uint1_norm:
    case raw_vertex_format::d3d_color: // TODO: Is this correct? Or do we not swap these?
    case raw_vertex_format::udec3:
    case raw_vertex_format::dec3:
    case raw_vertex_format::udec3_norm:
    case raw_vertex_format::dec3_norm:
    case raw_vertex_format::udec4:
    case raw_vertex_format::dec4:
    case raw_vertex_format::udec4_norm:
    case raw_vertex_format::dec4_norm:
    case raw_vertex_format::uhend3:
    case raw_vertex_format::hend3:
    case raw_vertex_format::uhend3_norm:
    case raw_vertex_format::hend3_norm:
    case raw_vertex_format::udhen3:
    case raw_vertex_format::dhen3:
    case raw_vertex_format::udhen3_norm:
    case raw_vertex_format::dhen3_norm:
        endian_swap(*static_cast<u32*>(rawVtx));
        break;

    case raw_vertex_format::int2:
    case raw_vertex_format::int2_norm:
    case raw_vertex_format::uint2:
    case raw_vertex_format::uint2_norm:
        endian_swap(*(static_cast<u32*>(rawVtx)));
        endian_swap(*(static_cast<u32*>(rawVtx) + 1));
        break;

    case raw_vertex_format::int4:
    case raw_vertex_format::int4_norm:
    case raw_vertex_format::uint4:
    case raw_vertex_format::uint4_norm:
        endian_swap(*(static_cast<u32*>(rawVtx)));
        endian_swap(*(static_cast<u32*>(rawVtx) + 1));
        endian_swap(*(static_cast<u32*>(rawVtx) + 2));
        endian_swap(*(static_cast<u32*>(rawVtx) + 3));
        break;

    case raw_vertex_format::ubyte4:
    case raw_vertex_format::byte4:
    case raw_vertex_format::ubyte4_norm:
    case raw_vertex_format::byte4_norm:
        endian_swap(*static_cast<u32*>(rawVtx));
        break;

    case raw_vertex_format::short2:
    case raw_vertex_format::short2_norm:
    case raw_vertex_format::ushort2:
    case raw_vertex_format::ushort2_norm:
    case raw_vertex_format::float16_2:
        endian_swap(*(static_cast<u16*>(rawVtx)));
        endian_swap(*(static_cast<u16*>(rawVtx) + 1));
        break;

    case raw_vertex_format::short4:
    case raw_vertex_format::short4_norm:
    case raw_vertex_format::ushort4:
    case raw_vertex_format::ushort4_norm:
    case raw_vertex_format::float16_4:
        endian_swap(*(static_cast<u16*>(rawVtx)));
        endian_swap(*(static_cast<u16*>(rawVtx) + 1));
        endian_swap(*(static_cast<u16*>(rawVtx) + 2));
        endian_swap(*(static_cast<u16*>(rawVtx) + 3));
        break;

    default:
        throw std::runtime_error("Unsupported HH vertex format");
    }
}

static void in_swap_recursive(raw_mesh& mesh)
{
    // Swap mesh.
    mesh.endian_swap<false>();

    // Swap faces.
    for (auto& face : mesh.faces)
    {
        endian_swap(face);
    }

    // Swap vertex format (array of vertex elements).
    raw_vertex_element* curVtxElem = mesh.vertexElements.get();
    do
    {
        curVtxElem->endian_swap<false>();
    }
    while ((curVtxElem++)->format != static_cast<u32>(raw_vertex_format::last_entry));

    // Swap vertices based on vertex format.
    curVtxElem = mesh.vertexElements.get();
    while (curVtxElem->format != static_cast<u32>(raw_vertex_format::last_entry))
    {
        // Swap vertices based on vertex element.
        void* curVtx = ptradd(mesh.vertices.get(), curVtxElem->offset);
        for (u32 i = 0; i < mesh.vertexCount; ++i)
        {
            // Swap vertex.
            in_swap_vertex(*curVtxElem, curVtx);

            // Increase vertices pointer.
            curVtx = ptradd(curVtx, mesh.vertexSize);
        }

        // Increase current vertex element pointer.
        ++curVtxElem;
    }
}

static void in_swap_recursive(raw_mesh_slot& slot)
{
    for (auto& meshOff : slot)
    {
        in_swap_recursive(*meshOff);
    }
}

static void in_swap_recursive(raw_special_meshes& special)
{
    for (u32 i = 0; i < special.count; ++i)
    {
        // Swap mesh count.
        endian_swap(*special.meshCounts[i]);

        // Swap meshes.
        const u32 meshCount = *special.meshCounts[i];
        for (u32 i2 = 0; i2 < meshCount; ++i2)
        {
            in_swap_recursive(*special.meshes[i][i2]);
        }
    }
}

static void in_swap_recursive(raw_mesh_group& group)
{
    // Swap the mesh group.
    group.endian_swap<false>();

    // Recursively swap mesh slots.
    in_swap_recursive(group.opaq);
    in_swap_recursive(group.punch);
    in_swap_recursive(group.trans);
    in_swap_recursive(group.special);
}

static void in_swap_recursive(arr32<off32<raw_mesh_group>>& groups)
{
    for (auto& groupOff : groups)
    {
        in_swap_recursive(*groupOff);
    }
}

void raw_terrain_model_v5::fix()
{
#ifndef HL_IS_BIG_ENDIAN
    // Swap terrain model header.
    endian_swap<false>();

    // Swap mesh groups.
    in_swap_recursive(meshGroups);
#endif
}

void raw_skeletal_model_v2::fix()
{
#ifndef HL_IS_BIG_ENDIAN
    // Swap skeletal model header.
    endian_swap<false>();

    // Swap mesh groups.
    in_swap_recursive(meshes);

    // TODO: Swap unknown1
    // TODO: Swap unknown2

    // Swap nodes.
    for (u32 i = 0; i < nodeCount; ++i)
    {
        nodes[i]->endian_swap<false>();
    }

    // Swap node matrices.
    for (u32 i = 0; i < nodeCount; ++i)
    {
        nodeMatrices[i].endian_swap<false>();
    }

    // Swap bounds.
    bounds->endian_swap<false>();

    // TODO: Swap unknown3
#endif
}

void raw_skeletal_model_v5::fix()
{
#ifndef HL_IS_BIG_ENDIAN
    // Swap skeletal model header.
    endian_swap<false>();

    // Swap mesh groups.
    in_swap_recursive(meshGroups);

    // TODO: Swap unknown1?

    // Swap nodes.
    for (u32 i = 0; i < nodeCount; ++i)
    {
        nodes[i]->endian_swap<false>();
    }

    // Swap node matrices.
    for (u32 i = 0; i < nodeCount; ++i)
    {
        nodeMatrices[i].endian_swap<false>();
    }

    // Swap bounds.
    bounds->endian_swap<false>();
#endif
}

void texture_unit::write(writer& writer) const
{
    // Generate raw texture unit.
    const auto nameOff = writer.tell();
    raw_texture_unit rawTexUnit =
    {
        nullptr,                                                        // name
        index                                                           // index
    };

    // Endian-swap texture unit if necessary.
#ifndef HL_IS_BIG_ENDIAN
    rawTexUnit.endian_swap();
#endif

    // Write texture unit to stream.
    writer.write_obj(rawTexUnit);

    // Write texture unit name.
    writer.fix_offset(nameOff);
    writer.write_str(name);
    writer.pad(4);
}

texture_unit::texture_unit(const raw_texture_unit& rawTexUnit) :
    name(rawTexUnit.name.get()), index(rawTexUnit.index) {}

static void in_mesh_add_faces_strips(const mesh& hhMesh, hl::mesh& hlMesh)
{
    // Ensure there are enough faces to properly convert.
    if (hhMesh.faces.size() < 3)
    {
        throw std::runtime_error("Invalid data; mesh uses triangle "
            "strips but has less than 3 face indices");
    }

    // Get initial indices.
    u16 f1 = hhMesh.faces[0];
    u16 f2 = hhMesh.faces[1];
    u16 f3;

    // Convert triangle strips to triangle indices and write indices to stream.
    bool reverse = false;
    for (std::size_t i = 2; i < hhMesh.faces.size(); ++i)
    {
        f3 = hhMesh.faces[i];

        if (f3 == UINT16_MAX)
        {
            f1 = hhMesh.faces[i + 1];
            f2 = hhMesh.faces[i + 2];

            reverse = false;
            i += 2;
        }
        else
        {
            if (f1 != f2 && f2 != f3 && f3 != f1)
            {
                if (reverse)
                {
                    hlMesh.faces.push_back(f1);
                    hlMesh.faces.push_back(f3);
                    hlMesh.faces.push_back(f2);
                }
                else
                {
                    hlMesh.faces.push_back(f1);
                    hlMesh.faces.push_back(f2);
                    hlMesh.faces.push_back(f3);
                }
            }

            f1 = f2;
            f2 = f3;
            reverse = !reverse;
        }
    }
}

hl::mesh& mesh::add_to_node(hl::node& node, raw_topology_type topType,
    const std::vector<mirage::node>* hhNodes, bool includeLibGensTags,
    const char* libGensLayerName) const
{
    // TODO: Handle multi-channel stuff!!!

    // Create mesh.
    std::unique_ptr<hl::mesh> mesh(new hl::mesh(node.name));

    // Determine what types of vertex elements we have.
    bool hasVertices = false;
    bool hasUVs = false;
    bool hasUVChannel[4] = { false, false, false, false };
    bool hasNormals = false;
    bool hasTangents = false;
    bool hasBinormals = false;
    bool hasColors = false;
    bool hasBoneWeights = false;
    bool hasBones = false;

    for (auto& vtxElem : vertexElements)
    {
        switch (vtxElem.type)
        {
        case raw_vertex_type::position:
            hasVertices = true;
            break;

        case raw_vertex_type::blend_weight:
            hasBoneWeights = true;
            break;

        case raw_vertex_type::blend_indices:
            hasBones = true;
            break;

        case raw_vertex_type::psize:
        case raw_vertex_type::tess_factor:
        case raw_vertex_type::position_t:
        case raw_vertex_type::fog:
        case raw_vertex_type::depth:
        case raw_vertex_type::sample:
            // TODO
            continue;

        case raw_vertex_type::normal:
            hasNormals = true;
            break;

        case raw_vertex_type::texcoord:
            if (vtxElem.index > 3)
            {
                // TODO: Log warning about how only 4 UV channels are supported.
            }
            else
            {
                hasUVChannel[vtxElem.index] = true;
            }

            hasUVs = true;
            break;

        case raw_vertex_type::tangent:
            hasTangents = true;
            break;

        case raw_vertex_type::binormal:
            hasBinormals = true;
            break;

        case raw_vertex_type::color:
            hasColors = true;
            break;
        }
    }

    // Reserve space for required vertex elements.
    if (hasVertices)
    {
        mesh->vertices.reserve(vertexCount);
    }

    if (hasUVs)
    {
        for (u8 i = 0; i < hl::mesh::max_uv_channel_count; ++i)
        {
            if (hasUVChannel[i])
            {
                mesh->uvs[i].reserve(vertexCount);
            }
        }
    }

    if (hasNormals)
    {
        mesh->normals.reserve(vertexCount);
    }

    if (hasTangents)
    {
        mesh->tangents.reserve(vertexCount);
    }

    if (hasBinormals)
    {
        mesh->binormals.reserve(vertexCount);
    }

    if (hasColors)
    {
        mesh->colors.reserve(vertexCount);
    }

    if (hasBoneWeights)
    {
        mesh->boneWeights.reserve(vertexCount);
    }

    if (hasBones)
    {
        mesh->boneRefs.reserve(vertexCount);
    }

    // Convert vertex elements and store them in mesh.
    hl::scene& scene = node.scene();
    for (auto& vtxElem : vertexElements)
    {
        // Get a pointer to the vec4 vector within the mesh
        // that we need to add the vertex elements to.
        std::vector<vec4>* meshVtxVector;
        switch (vtxElem.type)
        {
        case raw_vertex_type::position:
            meshVtxVector = &mesh->vertices;
            break;

        case raw_vertex_type::blend_weight:
            meshVtxVector = &mesh->boneWeights;
            break;

        case raw_vertex_type::blend_indices:
        {
            if (!hhNodes) continue;

            const u8* curVtx = (vertices.get() + vtxElem.offset);
            for (u32 i = 0; i < vertexCount; ++i)
            {
                // Convert vertex to ivector4.
                ivec4 val;
                vtxElem.convert_to_ivec4(curVtx, val);

                // Setup bone references from indices in ivector4.
                mesh->boneRefs.emplace_back();
                bone_ref& bone = mesh->boneRefs.back();

                if (val.x >= 0)
                {
                    bone[0] = scene.find_node<hl::bone>(
                        (*hhNodes)[boneNodeIndices[val.x]].name);
                }

                if (val.y >= 0)
                {
                    bone[1] = scene.find_node<hl::bone>(
                        (*hhNodes)[boneNodeIndices[val.y]].name);
                }

                if (val.z >= 0)
                {
                    bone[2] = scene.find_node<hl::bone>(
                        (*hhNodes)[boneNodeIndices[val.z]].name);
                }

                if (val.w >= 0)
                {
                    bone[3] = scene.find_node<hl::bone>(
                        (*hhNodes)[boneNodeIndices[val.w]].name);
                }

                // Get next vertex.
                curVtx += vertexSize;
            }

            continue;
        }

        case raw_vertex_type::normal:
            meshVtxVector = &mesh->normals;
            break;

        // TODO
        //case raw_vertex_type::psize = 4,

        case raw_vertex_type::texcoord:
        {
            if (vtxElem.index > 3) continue;

            const u8* curVtx = (vertices.get() + vtxElem.offset);
            for (u32 i = 0; i < vertexCount; ++i)
            {
                // Convert vertex to vector4.
                vec4 val;
                vtxElem.convert_to_vec4(curVtx, val);

                // Create vector2 from vector4 and store it in mesh.
                mesh->uvs[vtxElem.index].emplace_back(val.x, val.y);

                // Get next vertex.
                curVtx += vertexSize;
            }

            continue;
        }

        case raw_vertex_type::tangent:
            meshVtxVector = &mesh->tangents;
            break;

        case raw_vertex_type::binormal:
            meshVtxVector = &mesh->binormals;
            break;

        // TODO
        //case raw_vertex_type::tess_factor = 8,
        //case raw_vertex_type::position_t = 9,

        case raw_vertex_type::color:
            meshVtxVector = &mesh->colors;
            break;

        // TODO?
        //case raw_vertex_type::fog = 11,
        //case raw_vertex_type::depth = 12,
        //case raw_vertex_type::sample = 13:

        default:
            continue;
        }

        // Convert vertex elements to vector4s and add them to mesh.
        const u8* curVtx = (vertices.get() + vtxElem.offset);
        for (u32 i = 0; i < vertexCount; ++i)
        {
            // Convert vertex to vector4.
            vec4 vec;
            vtxElem.convert_to_vec4(curVtx, vec);

            // Store vector4 in mesh.
            meshVtxVector->push_back(vec);

            // Get next vertex.
            curVtx += vertexSize;
        }
    }

    // Convert faces as necessary and store them in mesh.
    switch (topType)
    {
    case raw_topology_type::triangle_list:
        mesh->faces.assign(faces.begin(), faces.end());
        break;

    case raw_topology_type::triangle_strip:
        in_mesh_add_faces_strips(*this, *mesh);
        break;

    default:
        throw std::runtime_error("Unknown topology type");
    }

    // Find the referenced material within the scene and link it to the mesh.
    std::string matName = materialName;
    if (includeLibGensTags && libGensLayerName)
    {
        matName += "@LYR(";
        matName += libGensLayerName;
        matName += ')';
    }

    mesh->material = node.scene().find_material(matName);

    // Add a new placeholder material if no matching material could be found within the scene.
    if (!mesh->material)
    {
        mesh->material = &node.scene().add_material(std::move(matName));
    }

    // Add mesh to node.
    node.attributes.emplace_back(std::move(mesh));
    return static_cast<hl::mesh&>(*node.attributes.back());
}

void mesh::write(writer& writer) const
{
    // Generate raw mesh.
    const auto matNameOff = writer.tell(offsetof(raw_mesh, materialName));
    const auto facesOff = writer.tell(offsetof(raw_mesh, faces.data));
    const auto verticesOff = writer.tell(offsetof(raw_mesh, vertices));
    const auto vtxElemsOff = writer.tell(offsetof(raw_mesh, vertexElements));
    const auto boneNodeIndicesOff = writer.tell(offsetof(raw_mesh, boneNodeIndices.data));
    const auto textureUnitsOff = writer.tell(offsetof(raw_mesh, textureUnits.data));

    raw_mesh rawMesh =
    {
        nullptr,                                                        // materialName
        { static_cast<u32>(faces.size()), nullptr },                    // faces
        vertexCount,                                                    // vertexCount
        vertexSize,                                                     // vertexSize
        nullptr,                                                        // vertices
        nullptr,                                                        // vertexElements
        { static_cast<u32>(boneNodeIndices.size()), nullptr },          // boneNodeIndices
        { static_cast<u32>(textureUnits.size()), nullptr }              // textureUnits
    };

    // Endian-swap raw mesh if necessary.
#ifndef HL_IS_BIG_ENDIAN
    rawMesh.endian_swap();
#endif

    // Write raw mesh to stream.
    writer.write_obj(rawMesh);

    // Write faces.
    writer.fix_offset(facesOff);

#ifndef HL_IS_BIG_ENDIAN
    for (auto face : faces)
    {
        hl::endian_swap(face);
        writer.write_obj(face);
    }
#else
    writer.write_arr(faces.size(), faces.data());
#endif

    writer.pad(4);

    // Write vertices.
    writer.fix_offset(verticesOff);

#ifndef HL_IS_BIG_ENDIAN
    std::unique_ptr<u8[]> tmpVertexBuf(new u8[vertexSize]);
    const u8* curVtx = vertices.get();

    for (u32 i = 0; i < vertexCount; ++i)
    {
        // Copy current vertex into temporary vertex buffer.
        std::memcpy(tmpVertexBuf.get(), curVtx, vertexSize);

        // Swap vertex in temporary vertex buffer.
        for (auto& curVtxElem : vertexElements)
        {
            in_swap_vertex(curVtxElem, tmpVertexBuf.get() + curVtxElem.offset);
        }

        // Write swapped vertex to stream.
        writer.write_all(vertexSize, tmpVertexBuf.get());

        // Increase vertices pointer.
        curVtx += vertexSize;
    }
#else
    writer.write_all(static_cast<std::size_t>(vertexSize) *
        vertexCount, vertices.get());
#endif

    // Write vertex elements.
    writer.fix_offset(vtxElemsOff);

#ifndef HL_IS_BIG_ENDIAN
    for (auto& vtxElem : vertexElements)
    {
        raw_vertex_element tmpVtxElem = vtxElem;
        tmpVtxElem.endian_swap();
        writer.write_obj(tmpVtxElem);
    }
#else
    writer.write_arr(vertexElements.size(), vertexElements.data());
#endif

    // Write last vertex element.
    raw_vertex_element lastRawVtxElem =
    {
        255,                                                            // stream
        0,                                                              // offset
        static_cast<u32>(raw_vertex_format::last_entry),                // format
        raw_vertex_method::normal,                                      // method
        raw_vertex_type::position,                                      // type
        0                                                               // padding
    };

#ifndef HL_IS_BIG_ENDIAN
    lastRawVtxElem.endian_swap();
#endif

    writer.write_obj(lastRawVtxElem);

    // Write node indices.
    writer.fix_offset(boneNodeIndicesOff);
    writer.write_arr(boneNodeIndices.size(), boneNodeIndices.data());
    writer.pad(4);

    // Write placeholder texture unit offsets.
    writer.fix_offset(textureUnitsOff);
    std::size_t curTexUnitOffPos = writer.tell();
    writer.write_nulls(sizeof(off32<raw_texture_unit>) * textureUnits.size());

    // Write texture units and fill-in placeholder texture unit offsets.
    for (auto& textureUnit : textureUnits)
    {
        // Fill-in texture unit placeholder offset.
        writer.fix_offset(curTexUnitOffPos);

        // Write texture unit.
        textureUnit.write(writer);

        // Increase texture unit offset position.
        curTexUnitOffPos += sizeof(off32<raw_texture_unit>);
    }

    // Write material name.
    writer.fix_offset(matNameOff);
    writer.write_str(materialName);
    writer.pad(4);
}

static const raw_vertex_element* in_get_last_vtx_elem(const raw_vertex_element* rawVtxElem)
{
    while (rawVtxElem->format != static_cast<u32>(raw_vertex_format::last_entry))
    {
        ++rawVtxElem;
    }

    return rawVtxElem;
}

mesh::mesh(const raw_mesh& rawMesh) :
    materialName(rawMesh.materialName.get()),
    faces(rawMesh.faces.begin(), rawMesh.faces.end()),

    vertexElements(rawMesh.vertexElements.get(),
        in_get_last_vtx_elem(rawMesh.vertexElements.get())),

    vertices(new u8[static_cast<std::size_t>(
        rawMesh.vertexSize) * rawMesh.vertexCount]),

    vertexCount(rawMesh.vertexCount),
    vertexSize(rawMesh.vertexSize),
    boneNodeIndices(rawMesh.boneNodeIndices.begin(), rawMesh.boneNodeIndices.end())
{
    // Copy vertices.
    std::size_t verticesSize = (static_cast<std::size_t>(
        rawMesh.vertexSize) * rawMesh.vertexCount);

    std::memcpy(vertices.get(), rawMesh.vertices.get(), verticesSize);

    // Emplace texture units.
    textureUnits.reserve(rawMesh.textureUnits.count);
    for (auto& rawTexUnitOff : rawMesh.textureUnits)
    {
        textureUnits.emplace_back(*rawTexUnitOff.get());
    }
}

void mesh_slot::get_unique_material_names(std::unordered_set<std::string>& uniqueMatNames) const
{
    for (auto& mesh : *this)
    {
        uniqueMatNames.emplace(mesh.materialName);
    }
}

void mesh_slot::add_to_node(hl::node& node, raw_topology_type topType,
    const std::vector<mirage::node>* hhNodes, bool includeLibGensTags,
    const char* libGensLayerName) const
{
    for (auto& hhMesh : *this)
    {
        hhMesh.add_to_node(node, topType, hhNodes,
            includeLibGensTags, libGensLayerName);
    }
}

void mesh_slot::write(writer& writer) const
{
    // Write placeholder mesh offsets.
    std::size_t curMeshOffPos = writer.tell();
    writer.write_nulls(sizeof(off32<raw_mesh>) * size());

    // Write meshes and fill-in placeholder mesh offsets.
    for (auto& mesh : *this)
    {
        // Fill-in placeholder mesh offset.
        writer.fix_offset(curMeshOffPos);

        // Write mesh.
        mesh.write(writer);

        // Increase current mesh offset position.
        curMeshOffPos += sizeof(off32<raw_mesh>);
    }
}

mesh_slot::mesh_slot(const raw_mesh_slot& rawSlot)
{
    reserve(rawSlot.count);

    for (const auto& rawMeshOff : rawSlot)
    {
        emplace_back(*rawMeshOff);
    }
}

special_mesh_slot::special_mesh_slot(const raw_special_meshes::const_wrapper& rawSpecialSlot) :
    type(rawSpecialSlot.type)
{
    reserve(rawSpecialSlot.meshCount);

    for (const auto& rawMesh : rawSpecialSlot)
    {
        emplace_back(rawMesh);
    }
}

special_meshes::special_meshes(const raw_special_meshes& rawSpecial)
{
    reserve(rawSpecial.count);

    for (const auto& rawSlot : rawSpecial)
    {
        emplace_back(rawSlot);
    }
}

void mesh_group::get_unique_material_names(std::unordered_set<std::string>& uniqueMatNames) const
{
    opaq.get_unique_material_names(uniqueMatNames);
    trans.get_unique_material_names(uniqueMatNames);
    punch.get_unique_material_names(uniqueMatNames);

    for (auto& specialSlot : special)
    {
        specialSlot.get_unique_material_names(uniqueMatNames);
    }
}

void mesh_group::add_to_node(hl::node& node, raw_topology_type topType,
    const std::vector<mirage::node>* hhNodes, bool includeLibGensTags) const
{
    // Add LibGens NAME tag to node if necessary.
    if (includeLibGensTags && !name.empty())
    {
        node.name += "@NAME(";
        node.name += name;
        node.name += ')';
    }

    // Add normal mesh slots to node.
    opaq.add_to_node(node, topType, hhNodes, includeLibGensTags);
    trans.add_to_node(node, topType, hhNodes, includeLibGensTags, "trans");
    punch.add_to_node(node, topType, hhNodes, includeLibGensTags, "punch");

    // Add special mesh slots to node.
    for (auto& specialSlot : special)
    {
        specialSlot.add_to_node(node, topType, hhNodes, includeLibGensTags);
    }
}

void mesh_group::write(writer& writer, u32 revision) const
{
    // Generate raw mesh group.
    const auto opaqOff = writer.tell(offsetof(raw_mesh_group, opaq.data));
    const auto transOff = writer.tell(offsetof(raw_mesh_group, trans.data));
    const auto punchOff = writer.tell(offsetof(raw_mesh_group, punch.data));
    const auto specialTypesOff = writer.tell(offsetof(raw_mesh_group, special.types));
    const auto specialCountsOff = writer.tell(offsetof(raw_mesh_group, special.meshCounts));
    const auto specialMeshesOff = writer.tell(offsetof(raw_mesh_group, special.meshes));

    raw_mesh_group rawMeshGroup =
    {
        { static_cast<u32>(opaq.size()), nullptr },                     // opaq
        { static_cast<u32>(trans.size()), nullptr },                    // trans
        { static_cast<u32>(punch.size()), nullptr },                    // punch
        {                                                               // special
            static_cast<u32>(special.size()),                           //   count
            UINT32_MAX,                                                 //   types
            UINT32_MAX,                                                 //   meshCounts
            UINT32_MAX                                                  //   meshes
        }
    };

    // Endian-swap raw mesh group if necessary.
#ifndef HL_IS_BIG_ENDIAN
    rawMeshGroup.endian_swap();
#endif

    // Write raw mesh group to stream.
    writer.write_obj(rawMeshGroup);

    // Write mesh group name.
    writer.write_str(name);
    writer.pad(4);

    // Write normal mesh slots.
    writer.fix_offset(opaqOff);
    opaq.write(writer);

    writer.fix_offset(transOff);
    trans.write(writer);

    writer.fix_offset(punchOff);
    punch.write(writer);

    // Early out if there are no special meshes.
    if (special.empty())
    {
        // Revision 1 fills in offsets anyway even if there
        // are no special meshes; replicate that here.
        if (revision < 2)
        {
            writer.fix_offset(specialTypesOff);
            writer.fix_offset(specialCountsOff);
            writer.fix_offset(specialMeshesOff);
        }

        return;
    }

    // Write placeholder special mesh group type offsets.
    std::size_t curOffPos = writer.tell();
    writer.fix_offset(specialTypesOff);
    writer.write_nulls(sizeof(off32<char>) * special.size());

    // Write special mesh group types.
    for (std::size_t i = 0; i < special.size(); i++)
    {
        // Fix special mesh group type offset.
        writer.fix_offset(curOffPos);

        // Write type.
        writer.write_str(special[i].type);
        writer.pad(4);

        // Increase current offset position.
        curOffPos += sizeof(off32<char>);
    }

    // Write placeholder special mesh count offsets.
    curOffPos = writer.tell();
    writer.fix_offset(specialCountsOff);
    writer.write_nulls(sizeof(off32<u32>) * special.size());

    // Write special mesh counts.
    for (std::size_t i = 0; i < special.size(); i++)
    {
        // Fix special mesh count offset.
        writer.fix_offset(curOffPos);

        // Write special mesh count.
        u32 specialMeshCount = static_cast<u32>(special[i].size());

        // Endian swap if necessary.
#ifndef HL_IS_BIG_ENDIAN
        endian_swap(specialMeshCount);
#endif

        writer.write_obj(specialMeshCount);

        // Increase current offset position.
        curOffPos += sizeof(off32<u32>);
    }

    // Write placeholder special mesh group offsets.
    curOffPos = writer.tell();
    writer.fix_offset(specialMeshesOff);
    writer.write_nulls(sizeof(off32<off32<raw_mesh>>) * special.size());

    // Write special mesh groups.
    for (std::size_t i = 0; i < special.size(); i++)
    {
        // Write placeholder special mesh offsets.
        std::size_t curMeshOffPos = writer.tell();
        writer.fix_offset(curOffPos);
        writer.write_nulls(sizeof(off32<raw_mesh>) * special[i].size());

        // Write special meshes.
        for (std::size_t i2 = 0; i2 < special[i].size(); i2++)
        {
            writer.fix_offset(curMeshOffPos);
            special[i][i2].write(writer);

            curMeshOffPos += sizeof(off32<raw_mesh>);
        }

        // Increase current offset position.
        curOffPos += sizeof(off32<off32<raw_mesh>>);
    }
}

mesh_group::mesh_group(const raw_mesh_group& rawGroup) :
    name(rawGroup.name()),
    opaq(rawGroup.opaq),
    trans(rawGroup.trans),
    punch(rawGroup.punch),
    special(rawGroup.special) {}

mesh_group::mesh_group(const raw_mesh_slot& rawSlot) :
    opaq(rawSlot) {}

hl::node& node::add_to_node(hl::node& parentNode,
    const std::vector<node>* nodes, bool addAsBone) const
{
    // Create new child node.
    hl::node& newNode = (addAsBone) ?
        parentNode.add_child<hl::bone>(name) :
        parentNode.add_child<hl::node>(name);

    // Decompose node matrix.
    glm::quat glmRot;
    glm::vec3 glmPos, glmScale;

    {
        glm::mat4x4 glmMatrix(
            matrix.m11, matrix.m12, matrix.m13, matrix.m14,
            matrix.m21, matrix.m22, matrix.m23, matrix.m24,
            matrix.m31, matrix.m32, matrix.m33, matrix.m34,
            matrix.m41, matrix.m42, matrix.m43, matrix.m44
        );

        glmMatrix = glm::inverse(glmMatrix);

        if (nodes)
        {
            for (long i = parentIndex; i >= 0; i = (*nodes)[i].parentIndex)
            {
                const matrix4x4& hhParentMatrix = (*nodes)[i].matrix;
                glm::mat4x4 glmParentMatrix(
                    hhParentMatrix.m11, hhParentMatrix.m12, hhParentMatrix.m13, hhParentMatrix.m14,
                    hhParentMatrix.m21, hhParentMatrix.m22, hhParentMatrix.m23, hhParentMatrix.m24,
                    hhParentMatrix.m31, hhParentMatrix.m32, hhParentMatrix.m33, hhParentMatrix.m34,
                    hhParentMatrix.m41, hhParentMatrix.m42, hhParentMatrix.m43, hhParentMatrix.m44
                );

                glmMatrix *= glmParentMatrix;
                //glmMatrix = glmParentMatrix * glmMatrix;
                break; // TODO: Do we really only do this once?
            }
        }

        {
            glm::vec3 glmSkew;
            glm::vec4 glmPerspective;

            glmMatrix = glm::transpose(glmMatrix);
            glm::decompose(glmMatrix, glmScale, glmRot, glmPos, glmSkew, glmPerspective);
        }
    }
    
    // Set object transform.
    newNode.pos = vec3(glmPos.x, glmPos.y, glmPos.z);
    newNode.rot = quat(glmRot.x, glmRot.y, glmRot.z, glmRot.w);
    newNode.scale = vec3(glmScale.x, glmScale.y, glmScale.z);

    return newNode;
}

void node::parse_sample_chunk_params(
    const sample_chunk::raw_node& rawNodePrmsNode)
{
    // Parse SCA parameters.
    const sample_chunk::raw_node* rawScaParamNode =
        rawNodePrmsNode.get_child("SCAParam", false);

    if (rawScaParamNode)
    {
        const sample_chunk::raw_node* rawNodeParamNode =
            rawScaParamNode->children();

        while (rawNodeParamNode)
        {
            scaParams.emplace_back(*rawNodeParamNode);
            rawNodeParamNode = rawNodeParamNode->next();
        }
    }
}

void node::write_sample_chunk_params(u32 nodeIndex, writer& writer) const
{
    // Return early if this node has no parameters.
    if (scaParams.empty()) return;

    // Start writing NodePrms and SCAParam sample chunk nodes.
    writer.start_node("NodePrms", nodeIndex);
    writer.start_node("SCAParam");

    // Write SCA parameters.
    for (auto& curSCAParam : scaParams)
    {
        writer.write_node(curSCAParam);
    }

    // Finish writing SCAParam and NodePrms sample chunk nodes.
    writer.finish_node();
    writer.finish_node();
}

void node::write(writer& writer) const
{
    // Generate raw node.
    const auto nameOff = writer.tell(offsetof(raw_node, name));
    raw_node rawNode =
    {
        static_cast<s32>(parentIndex),                                  // parentIndex
        nullptr                                                         // name
    };

    // Endian-swap raw node if necessary.
#ifndef HL_IS_BIG_ENDIAN
    rawNode.endian_swap();
#endif

    // Write raw node to stream.
    writer.write_obj(rawNode);

    // Write raw node name.
    writer.fix_offset(nameOff);
    writer.write_str(name);
    writer.pad(4);
}

bool model::in_has_per_node_params(std::size_t nodeCount,
    const node* nodes) const noexcept
{
    for (std::size_t i = 0; i < nodeCount; ++i)
    {
        if (!nodes[i].scaParams.empty())
        {
            return true;
        }
    }

    return false;
}

void model::in_parse_mesh_groups(const arr32<off32<raw_mesh_group>>& rawMeshGroups)
{
    // Reserve space for mesh groups.
    meshGroups.reserve(rawMeshGroups.count);

    // Parse mesh groups.
    for (const auto& rawMeshGroupOff : rawMeshGroups)
    {
        meshGroups.emplace_back(*rawMeshGroupOff);
    }
}

void model::in_parse_sample_chunk_nodes(const void* rawData,
    std::size_t nodeCount, node* nodes)
{
    // Parse sample chunk nodes if necessary.
    if (has_sample_chunk_header_fixed(rawData))
    {
        // Ensure that we have Model node.
        const sample_chunk::raw_header* rawHeader = static_cast<
            const sample_chunk::raw_header*>(rawData);

        const sample_chunk::raw_node* rawModelNode =
            rawHeader->get_node("Model", false);

        if (!rawModelNode) return;

        // If we have NodesExt node, parse per-node parameters.
        const sample_chunk::raw_node* rawNodesExtNode =
            rawModelNode->get_child("NodesExt", false);

        const sample_chunk::raw_node* curRawMdlProperty;

        if (rawNodesExtNode)
        {
            const sample_chunk::raw_node* rawNodePrmsNode =
                rawNodesExtNode->get_child("NodePrms", false);

            while (rawNodePrmsNode)
            {
                // Parse NodePrms if node index is valid.
                if (rawNodePrmsNode->value < nodeCount)
                {
                    nodes[rawNodePrmsNode->value].parse_sample_chunk_params(*rawNodePrmsNode);
                }
                else
                {
                    // TODO: Log warning about invalid node index.
                }

                rawNodePrmsNode = rawNodePrmsNode->next_of_name("NodePrms");
            }

            curRawMdlProperty = rawNodesExtNode->next();
        }
        else
        {
            curRawMdlProperty = rawModelNode->children();
        }

        // Parse model parameters.
        while (curRawMdlProperty)
        {
            // Stop if we've reached the Contexts node.
            if (std::memcmp(curRawMdlProperty->name, "Contexts", 8) == 0)
            {
                break;
            }

            // Parse model parameter.
            params.emplace_back(*curRawMdlProperty);
            curRawMdlProperty = curRawMdlProperty->next();
        }
    }
}

void model::in_write_sample_chunk_nodes(std::size_t nodeCount,
    const node* nodes, writer& writer) const
{
    // Write per-node sample chunk nodes if necessary.
    if (in_has_per_node_params(nodeCount, nodes))
    {
        // Start writing NodesExt sample chunk node.
        writer.start_node("NodesExt");

        // Write per-node sample chunk nodes.
        for (std::size_t i = 0; i < nodeCount; ++i)
        {
            nodes[i].write_sample_chunk_params(
                static_cast<u32>(i), writer);
        }

        // Finish writing NodesExt sample chunk node.
        writer.finish_node();
    }

    // Write per-model sample chunk nodes.
    for (auto& mdlParam : params)
    {
        writer.write_node(mdlParam);
    }
}

const sample_chunk::property* model::get_param(const char* name) const
{
    for (auto& param : params)
    {
        if (std::strcmp(param.get_name(), name) == 0)
        {
            return &param;
        }
    }

    return nullptr;
}

raw_topology_type model::get_topology_type() const
{
    // Get toplogy parameter, if any.
    const auto topologyProp = get_param("Topology");

    // Return value, defaulting to triangle strips if there was no topology property.
    return (!topologyProp) ? raw_topology_type::triangle_strip :
        static_cast<raw_topology_type>(topologyProp->value);
}

void model::get_unique_material_names(std::unordered_set<std::string>& uniqueMatNames) const
{
    for (auto& meshGroup : meshGroups)
    {
        meshGroup.get_unique_material_names(uniqueMatNames);
    }
}

std::unordered_set<std::string> model::get_unique_material_names() const
{
    std::unordered_set<std::string> uniqueMatNames;
    get_unique_material_names(uniqueMatNames);
    return uniqueMatNames;
}

void model::import_materials(const nchar* materialDir, scene& scene,
    bool merge, bool includeLibGensTags)
{
    nstring matPath = materialDir;
    if (path::combine_needs_sep1(matPath))
    {
        matPath += path::separator;
    }

    const std::size_t matNamePos = matPath.size();
    std::unordered_set<std::string> uniqueMatNames = get_unique_material_names();

    for (auto& matName : uniqueMatNames)
    {
        matPath +=
#ifdef HL_IN_WIN32_UNICODE
            text::conv<text::utf8_to_native>(matName);
#else
            matName;
#endif

        matPath += material::ext;

        if (path::exists(matPath))
        {
            material mat(matPath);
            mat.add_to_scene(materialDir, scene, merge, includeLibGensTags);
            matPath.erase(matNamePos);
        }
        else
        {
            // TODO: Log warning about missing material
        }
    }
}

header_type terrain_model::in_get_default_header_type() const
{
    if (has_params())
    {
        // Default to sample chunk V2 if we have a Topology parameter, or V1 otherwise.
        const auto topologyProp = get_param("Topology");
        return (topologyProp) ?
            header_type::sample_chunk_v2 :
            header_type::sample_chunk_v1;
    }
    else
    {
        // Default to standard if we have no parameters.
        return header_type::standard;
    }
}

void terrain_model::fix(void* rawData)
{
    // Fix mirage data.
    mirage::fix(rawData);

    // Get terrain model data and version number.
    u32 version;
    void* mdlData = get_data(rawData, &version);
    if (!mdlData) return;

    // Fix terrain model data based on version number.
    switch (version)
    {
    case 5:
        static_cast<raw_terrain_model_v5*>(mdlData)->fix();
        break;

    default:
        throw std::runtime_error("Unsupported HH terrain model version");
    }
}

void terrain_model::add_to_node(hl::node& parentNode, bool includeLibGensTags) const
{
    // Get model topology type.
    const raw_topology_type topType = get_topology_type();

    // Add node for model.
    hl::node& modelNode = parentNode.add_child(rootNode.name);

    // Add mesh groups to model.
    std::size_t unnamedMeshGroupCount = 0;
    for (std::size_t i = 0; i < meshGroups.size(); ++i)
    {
        std::string meshGroupName((!meshGroups[i].name.empty()) ?
            meshGroups[i].name : modelNode.name);

        if (meshGroups[i].name.empty() && unnamedMeshGroupCount++)
        {
            meshGroupName += " (";
            meshGroupName += std::to_string(unnamedMeshGroupCount);
            meshGroupName += ')';
        }

        hl::node& meshGroupNode = modelNode.add_child(std::move(meshGroupName));
        meshGroups[i].add_to_node(meshGroupNode, topType, nullptr, includeLibGensTags);
    }
}

void terrain_model::parse(const raw_terrain_model_v5& rawMdl)
{
    // Parse mesh groups.
    in_parse_mesh_groups(rawMdl.meshGroups);

    // Parse root node.
    rootNode = node(rawMdl.name.get());
}

void terrain_model::parse(const void* rawData)
{
    // Get terrain model data and version number.
    u32 version;
    const void* mdlData = get_data(rawData, &version);
    if (!mdlData) return; // TODO: Should this be an error?

    // Parse terrain model data based on version number.
    switch (version)
    {
    case 5:
        parse(*static_cast<const raw_terrain_model_v5*>(mdlData));
        break;

    default:
        throw std::runtime_error("Unsupported HH terrain model version");
    }

    // Parse sample chunk nodes.
    in_parse_sample_chunk_nodes(rawData, 1, &rootNode);
}

void terrain_model::load(const nchar* filePath)
{
    blob rawMdl(filePath);
    fix(rawMdl);
    parse(rawMdl);
}

void terrain_model::write(writer& writer, header_type headerType,
    u32 version, u32 revision, const char* fileName) const
{
    // Start writing model.
    writer.start(headerType);
    if (headerType != header_type::standard)
    {
        writer.start_node("Model");
        in_write_sample_chunk_nodes(1, &rootNode, writer);
    }

    writer.start_data(version);

    // Write terrain model header based on version.
    std::size_t meshGroupsOff, nameOff;
    switch (version)
    {
    case 5:
    {
        // Generate raw V5 terrain model header.
        const u32 flags = static_cast<u32>((isInstanced) ?
            raw_terrain_model_flags::is_instanced :
            raw_terrain_model_flags::none);

        meshGroupsOff = writer.tell(offsetof(raw_terrain_model_v5, meshGroups.data));
        nameOff = writer.tell(offsetof(raw_terrain_model_v5, name));

        raw_terrain_model_v5 rawMdl =
        {
            { static_cast<u32>(meshGroups.size()), nullptr },           // meshGroups
            nullptr,                                                    // name
            flags                                                       // flags
        };

        // Endian-swap header if necessary.
#ifndef HL_IS_BIG_ENDIAN
        rawMdl.endian_swap(revision >= 2);
#endif

        // Write header to stream.
        writer.write_all((revision >= 2) ? 16 : 12, &rawMdl);
        break;
    }

    default:
        throw std::runtime_error("Unsupported HH terrain model version");
    }

    // Write meshes/mesh groups as necessary.
    writer.fix_offset(meshGroupsOff);
    switch (version)
    {
    case 5:
        // Write placeholder offsets for mesh groups.
        std::size_t curMeshGroupOffPos = writer.tell();
        writer.write_nulls(sizeof(off32<raw_mesh_group>) * meshGroups.size());

        // Write mesh groups and fill-in placeholder offsets.
        for (auto& meshGroup : meshGroups)
        {
            // Fill-in placeholder mesh group offset.
            writer.fix_offset(curMeshGroupOffPos);

            // Write mesh group.
            meshGroup.write(writer, revision);

            // Increase current mesh group offset position.
            curMeshGroupOffPos += sizeof(off32<raw_mesh_group>);
        }
        break;
    }

    // Write root node name if necessary.
    switch (version)
    {
    case 5:
        writer.fix_offset(nameOff);
        writer.write_str(rootNode.name);
        writer.pad(4);
        break;
    }

    // Finish writing.
    writer.finish_data();
    writer.finish(fileName);
}

void terrain_model::save(stream& stream, header_type headerType,
    u32 version, u32 revision, const char* fileName) const
{
    writer writer(stream);
    write(writer, headerType, version, revision, fileName);
}

void terrain_model::save(const nchar* filePath,
    header_type headerType, u32 version, u32 revision) const
{
    file_stream stream(filePath, file::mode::write);
    save(stream, headerType, version, revision,
        (revision >= 2) ? nullptr : "");
}

void terrain_model::save(const nchar* filePath,
    header_type headerType, u32 version) const
{
    file_stream stream(filePath, file::mode::write);
    save(stream, headerType, version);
}

void terrain_model::save(stream& stream) const
{
    save(stream, in_get_default_header_type());
}

void terrain_model::save(const nchar* filePath) const
{
    file_stream stream(filePath, file::mode::write);
    save(stream);
}

header_type skeletal_model::in_get_default_header_type() const
{
    if (has_params())
    {
        // Default to sample chunk V2 if we have a Topology parameter, or V1 otherwise.
        const auto topologyProp = get_param("Topology");
        return (topologyProp) ?
            header_type::sample_chunk_v2 :
            header_type::sample_chunk_v1;
    }
    else
    {
        // Default to standard if we have no parameters.
        return header_type::standard;
    }
}

void skeletal_model::fix(void* rawData)
{
    // Fix mirage data.
    mirage::fix(rawData);

    // Get skeletal model data and version number.
    u32 version;
    void* mdlData = get_data(rawData, &version);
    if (!mdlData) return;

    // Fix skeletal model data based on version number.
    switch (version)
    {
    case 2:
        static_cast<raw_skeletal_model_v2*>(mdlData)->fix();
        break;

    // TODO: V4 support.
        
    case 5:
        static_cast<raw_skeletal_model_v5*>(mdlData)->fix();
        break;

    default:
        throw std::runtime_error("Unsupported HH skeletal model version");
    }
}

bool skeletal_model::has_per_node_params() const noexcept
{
    return in_has_per_node_params(nodes.size(), nodes.data());
}

void skeletal_model::add_to_node(hl::node& parentNode, bool includeLibGensTags) const
{
    // Get model topology type.
    const raw_topology_type topType = get_topology_type();

    // Add nodes to the scene.
    std::unique_ptr<hl::node*[]> nodePtrs(new hl::node*[nodes.size()]);
    hl::node* fallbackParentNode = nullptr;

    for (std::size_t i = 0; i < nodes.size(); ++i)
    {
        // Get current parent node.
        const node& hhCurNode = nodes[i];
        hl::node& curParentNode = (hhCurNode.parentIndex < 0) ?
            parentNode : *nodePtrs[hhCurNode.parentIndex];

        // HACK (Tokyo2020+): If we have no fallback parent node and this node is a
        // child of the node named "mesh", use this node as the fallback parent node.
        const bool isFirstMeshNode = (!fallbackParentNode && curParentNode.name == "mesh");

        // Add the node to the scene.
        nodePtrs[i] = &hhCurNode.add_to_node(curParentNode, &nodes,
            hhCurNode.parentIndex != -1 && !isFirstMeshNode);

        if (isFirstMeshNode)
        {
            fallbackParentNode = nodePtrs[i];
        }
    }

    // Use the first child node (or parent if there are no children) as the fallback
    // parent node if we weren't able to find a better candidate earlier.
    if (!fallbackParentNode)
    {
        fallbackParentNode = (nodes.empty()) ?
            &parentNode : nodePtrs[0];
    }

    // Add mesh groups to model.
    std::size_t unnamedMeshGroupCount = 0;
    for (std::size_t i = 0; i < meshGroups.size(); ++i)
    {
        std::string meshGroupName((!meshGroups[i].name.empty()) ?
            meshGroups[i].name : fallbackParentNode->name);

        if (meshGroups[i].name.empty() && unnamedMeshGroupCount++)
        {
            meshGroupName += " (";
            meshGroupName += std::to_string(unnamedMeshGroupCount);
            meshGroupName += ')';
        }

        hl::node& meshGroupNode = fallbackParentNode->add_child(std::move(meshGroupName));
        meshGroups[i].add_to_node(meshGroupNode, topType, &nodes, includeLibGensTags);
    }
}

void skeletal_model::parse(const raw_skeletal_model_v2& rawMdl)
{
    // Parse mesh groups.
    meshGroups.emplace_back(rawMdl.meshes);

    // TODO: Parse unknown1
    // TODO: Parse unknown2

    // Reserve space for nodes.
    nodes.reserve(rawMdl.nodeCount);

    // Parse nodes.
    for (u32 i = 0; i < rawMdl.nodeCount; ++i)
    {
        nodes.emplace_back(*rawMdl.nodes[i].get(), rawMdl.nodeMatrices[i]);
    }

    // TODO: Parse bounds?

    // TODO: Parse unknown3
}

void skeletal_model::parse(const raw_skeletal_model_v4& rawMdl)
{
    // TODO
}

void skeletal_model::parse(const raw_skeletal_model_v5& rawMdl)
{
    // Parse mesh groups.
    in_parse_mesh_groups(rawMdl.meshGroups);

    // TODO: Parse unknown1

    // Reserve space for nodes.
    nodes.reserve(rawMdl.nodeCount);

    // Parse nodes.
    for (u32 i = 0; i < rawMdl.nodeCount; ++i)
    {
        nodes.emplace_back(*rawMdl.nodes[i].get(), rawMdl.nodeMatrices[i]);
    }

    // TODO: Parse bounds?
}

void skeletal_model::parse(const void* rawData)
{
    // Get skeletal model data and version number.
    u32 version;
    const void* mdlData = get_data(rawData, &version);
    if (!mdlData) return; // TODO: Should this be an error?

    // Parse skeletal model data based on version number.
    switch (version)
    {
    case 2:
        parse(*static_cast<const raw_skeletal_model_v2*>(mdlData));
        break;

    // TODO: V4 support.

    case 5:
        parse(*static_cast<const raw_skeletal_model_v5*>(mdlData));
        break;

    default:
        throw std::runtime_error("Unsupported HH skeletal model version");
    }

    // Parse sample chunk nodes.
    in_parse_sample_chunk_nodes(rawData, nodes.size(), nodes.data());
}

void skeletal_model::load(const nchar* filePath)
{
    blob rawMdl(filePath);
    fix(rawMdl);
    parse(rawMdl);
}

void skeletal_model::write(writer& writer, header_type headerType,
    u32 version, const char* fileName) const
{
    // Start writing model.
    writer.start(headerType);
    if (headerType != header_type::standard)
    {
        writer.start_node("Model");
        in_write_sample_chunk_nodes(nodes.size(),
            nodes.data(), writer);
    }

    writer.start_data(version);

    // Write skeletal model header based on version.
    std::size_t meshesOff, unknown1Off, unknown2Off, unknown3Off,
        nodesOff, nodeMatricesOff, boundsOff;

    switch (version)
    {
    case 2:
    {
        // Generate raw V2 skeletal model header.
        const std::size_t meshCount = (meshGroups.empty()) ?
            0 : meshGroups[0].opaq.size();

        meshesOff = writer.tell(offsetof(raw_skeletal_model_v2, meshes.data));
        unknown1Off = writer.tell(offsetof(raw_skeletal_model_v2, unknown1.data));
        unknown2Off = writer.tell(offsetof(raw_skeletal_model_v2, unknown2.data));
        nodesOff = writer.tell(offsetof(raw_skeletal_model_v2, nodes));
        nodeMatricesOff = writer.tell(offsetof(raw_skeletal_model_v2, nodeMatrices));
        boundsOff = writer.tell(offsetof(raw_skeletal_model_v2, bounds));
        unknown3Off = writer.tell(offsetof(raw_skeletal_model_v2, unknown3));

        raw_skeletal_model_v2 rawMdl =
        {
            { static_cast<u32>(meshCount), nullptr },                   // meshes
            { 0, nullptr },                                             // unknown1
            { 0, nullptr },                                             // unknown2
            static_cast<u32>(nodes.size()),                             // nodeCount
            nullptr,                                                    // nodes
            nullptr,                                                    // nodeMatrices
            nullptr,                                                    // bounds
            nullptr                                                     // unknown3 // TODO: This points to the offset table??
        };

        // Endian-swap header if necessary.
#ifndef HL_IS_BIG_ENDIAN
        rawMdl.endian_swap();
#endif

        // Write header to stream.
        writer.write_obj(rawMdl);
        break;
    }

    case 4:
    {
        // Generate raw V4 skeletal model header.
        meshesOff = writer.tell(offsetof(raw_skeletal_model_v4, meshGroups.data));
        unknown1Off = writer.tell(offsetof(raw_skeletal_model_v4, unknown1.data));
        unknown2Off = writer.tell(offsetof(raw_skeletal_model_v4, unknown2.data));
        unknown3Off = writer.tell(offsetof(raw_skeletal_model_v4, unknown3.data));
        nodesOff = writer.tell(offsetof(raw_skeletal_model_v4, nodes));
        nodeMatricesOff = writer.tell(offsetof(raw_skeletal_model_v4, nodeMatrices));
        boundsOff = writer.tell(offsetof(raw_skeletal_model_v4, bounds));

        raw_skeletal_model_v4 rawMdl =
        {
            { static_cast<u32>(meshGroups.size()), nullptr },           // meshGroups
            { 0, nullptr },                                             // unknown1
            { 0, nullptr },                                             // unknown2
            { 0, nullptr },                                             // unknown3
            static_cast<u32>(nodes.size()),                             // nodeCount
            nullptr,                                                    // nodes
            nullptr,                                                    // nodeMatrices
            nullptr                                                     // bounds
        };

        // Endian-swap header if necessary.
#ifndef HL_IS_BIG_ENDIAN
        rawMdl.endian_swap();
#endif

        // Write header to stream.
        writer.write_obj(rawMdl);
        break;
    }

    case 5:
    {
        // Generate raw V5 skeletal model header.
        meshesOff = writer.tell(offsetof(raw_skeletal_model_v5, meshGroups.data));
        unknown1Off = writer.tell(offsetof(raw_skeletal_model_v5, unknown1.data));
        nodesOff = writer.tell(offsetof(raw_skeletal_model_v5, nodes));
        nodeMatricesOff = writer.tell(offsetof(raw_skeletal_model_v5, nodeMatrices));
        boundsOff = writer.tell(offsetof(raw_skeletal_model_v5, bounds));

        raw_skeletal_model_v5 rawMdl =
        {
            { static_cast<u32>(meshGroups.size()), nullptr },           // meshGroups
            { 0, nullptr },                                             // unknown1
            static_cast<u32>(nodes.size()),                             // nodeCount
            nullptr,                                                    // nodes
            nullptr,                                                    // nodeMatrices
            nullptr                                                     // bounds
        };

        // Endian-swap header if necessary.
#ifndef HL_IS_BIG_ENDIAN
        rawMdl.endian_swap();
#endif

        // Write header to stream.
        writer.write_obj(rawMdl);
        break;
    }

    default:
        throw std::runtime_error("Unsupported HH skeletal model version");
    }

    // Write meshes/mesh groups as necessary.
    writer.fix_offset(meshesOff);
    switch (version)
    {
    case 2:
        if (!meshGroups.empty())
        {
            meshGroups[0].opaq.write(writer);
        }
        break;

    case 4:
    case 5:
        // Write placeholder offsets for mesh groups.
        std::size_t curMeshGroupOffPos = writer.tell();
        writer.write_nulls(sizeof(off32<raw_mesh_group>) * meshGroups.size());

        // Write mesh groups and fill-in placeholder offsets.
        for (auto& meshGroup : meshGroups)
        {
            // Fill-in placeholder mesh group offset.
            writer.fix_offset(curMeshGroupOffPos);

            // Write mesh group.
            meshGroup.write(writer);

            // Increase current mesh group offset position.
            curMeshGroupOffPos += sizeof(off32<raw_mesh_group>);
        }
        break;
    }

    // Write unknown1s if necessary.
    switch (version)
    {
    case 2:
    case 4:
    case 5:
        writer.fix_offset(unknown1Off);
        // TODO: Write unknown1s if necessary.
        break;
    }

    // Write unknown2s if necessary.
    switch (version)
    {
    case 2:
    case 4:
        writer.fix_offset(unknown2Off);
        // TODO: Write unknown2s if necessary.
        break;
    }

    // Write unknown3s if necessary.
    switch (version)
    {
    case 4:
        writer.fix_offset(unknown3Off);
        // TODO: Write unknown3s if necessary.
        break;
    }

    // Write node placeholder offsets.
    writer.fix_offset(nodesOff);
    std::size_t curNodeOffPos = writer.tell();
    writer.write_nulls(sizeof(off32<raw_node>) * nodes.size());

    // Write nodes and fill-in node placeholder offsets.
    for (auto& node : nodes)
    {
        // Fill-in node placeholder offset.
        writer.fix_offset(curNodeOffPos);
        
        // Write node.
        node.write(writer);

        // Increase current node offset position.
        curNodeOffPos += sizeof(off32<raw_node>);
    }

    // Write node matrices.
    writer.fix_offset(nodeMatricesOff);
    for (auto& node : nodes)
    {
#ifndef HL_IS_BIG_ENDIAN
        matrix4x4 nodeMatrix = node.matrix;
        nodeMatrix.endian_swap();
        writer.write_obj(nodeMatrix);
#else
        writer.write_obj(node.matrix);
#endif
    }

    // Write bounds.
    writer.fix_offset(boundsOff);
    writer.write_nulls(sizeof(aabb)); // TODO: GET RID OF THIS LINE AND DO THIS CORRECTLY!!!

    // Write unknown3s if necessary.
    switch (version)
    {
    case 2:
        writer.fix_offset(unknown3Off);
        // TODO: Write unknown3s if necessary.
        break;
    }

    // Finish writing.
    writer.finish_data();
    writer.finish(fileName);
}

void skeletal_model::save(stream& stream, header_type headerType,
    u32 version, const char* fileName) const
{
    writer writer(stream);
    write(writer, headerType, version, fileName);
}

void skeletal_model::save(const nchar* filePath,
    header_type headerType, u32 version) const
{
#ifdef HL_IN_WIN32_UNICODE
    std::string fileName;
#endif

    const char* fileNamePtr;
    if (version == 2)
    {
#ifdef HL_IN_WIN32_UNICODE
        fileName = text::conv<text::native_to_utf8>(path::get_name(filePath));
        fileNamePtr = fileName.c_str();
#else
        fileNamePtr = path::get_name(filePath);
#endif
    }
    else
    {
        fileNamePtr = nullptr;
    }

    file_stream stream(filePath, file::mode::write);
    save(stream, headerType, version, fileNamePtr);
}

void skeletal_model::save(stream& stream) const
{
    // TODO: Use version 2 or 4 instead if necessary.
    save(stream, in_get_default_header_type());
}

void skeletal_model::save(const nchar* filePath) const
{
    file_stream stream(filePath, file::mode::write);
    save(stream);
}
} // mirage
} // hh
} // hl
