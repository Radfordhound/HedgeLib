#ifndef HL_FXCOL_H_INCLUDED
#define HL_FXCOL_H_INCLUDED
#include "../hl_math.h"

namespace hl
{
namespace fxcol
{
namespace v1
{
struct shape_group
{
    u32 bspNodeCount;
    u32 bspFirstNodeIndex;
    aabb bounds;

    template<bool swapOffsets = true>
    inline void endian_swap() noexcept
    {
        hl::endian_swap(bspNodeCount);
        hl::endian_swap(bspFirstNodeIndex);
        hl::endian_swap<swapOffsets>(bounds);
    }
};

HL_STATIC_ASSERT_SIZE(shape_group, 0x20);

struct bsp_node
{
    u32 unknown1;
    float unknown2;

    template<bool swapOffsets = true>
    inline void endian_swap() noexcept
    {
        hl::endian_swap(unknown1);
        hl::endian_swap(unknown2);
    }
};

HL_STATIC_ASSERT_SIZE(bsp_node, 8);

union shape_data32
{
    struct
    {
        float radius; // TODO: Is this correct?
        float unknown2;
    }
    sphere;

    struct
    {
        float unknown1;
        float unknown2;
        float unknown3;
    }
    capsule;

    struct
    {
        float sizeZ;
        float sizeX;
        float sizeY;
        float unknown1;
        float unknown2;
    }
    box;

    struct
    {
        float unknown1;
        float unknown2;
        float unknown3;
        float unknown4;
    }
    omni_box;
};

HL_STATIC_ASSERT_SIZE(shape_data32, 0x14);

union shape_data64
{
    struct
    {
        float radius; // TODO: Is this correct?
        float unknown2;
    }
    sphere;

    struct
    {
        float unknown1;
        float unknown2;
        float unknown3;
    }
    capsule;

    struct
    {
        float sizeZ;
        float sizeX;
        float sizeY;
        float unknown1;
        float unknown2;
        float unknown3;
    }
    box;

    struct
    {
        float unknown1;
        float unknown2;
        float unknown3;
        float unknown4;
    }
    omni_box;
};

HL_STATIC_ASSERT_SIZE(shape_data64, 0x18);

enum class shape_type : u8
{
    sphere = 0,
    capsule = 1,
    box = 2,
    omni_box = 3
};

struct shape32
{
    off32<char> name;
    /** @brief See hl::fxcol::v1::shape_type. */
    u8 type;
    /** @brief hasLight? */
    u8 unknown1;
    u8 padding1[2];
    shape_data32 data;
    u32 groupIndex;
    u32 unknown6;
    u32 unknown7;
    u32 unknown8;
    u32 unknown9;
    off32<char> lightName;
    vec3 pos;
    quat rot;

    // TODO: Endian-swap function.
};

HL_STATIC_ASSERT_SIZE(shape32, 0x50);

struct shape64
{
    off64<char> name;
    /** @brief See hl::fxcol::v1::shape_type. */
    u8 type;
    /** @brief hasLight? */
    u8 unknown1;
    u8 padding1[2];
    shape_data64 data;
    u32 groupIndex;
    u64 unknown6;
    u64 unknown7;
    u64 unknown8;
    u64 unknown9;
    off64<char> lightName;
    vec3 pos;
    quat rot;
    u32 padding2;

    template<bool swapOffsets = true>
    inline void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);

        switch (static_cast<shape_type>(type))
        {
        case shape_type::sphere:
            // TODO
            break;

        case shape_type::capsule:
            // TODO
            break;

        case shape_type::box:
            // TODO
            break;

        case shape_type::omni_box:
            // TODO
            break;
        }

        hl::endian_swap(groupIndex);
        hl::endian_swap(unknown6);
        hl::endian_swap(unknown7);
        hl::endian_swap(unknown8);
        hl::endian_swap(unknown9);
        hl::endian_swap<swapOffsets>(lightName);
        hl::endian_swap<swapOffsets>(pos);
        hl::endian_swap<swapOffsets>(rot);
    }
};

HL_STATIC_ASSERT_SIZE(shape64, 0x70);

struct header32
{
    u32 signature;
    u32 version;
    arr32<shape> shapes;
    arr32<shape_group> groups;
    arr32<bsp_node> bspNodes;

    template<bool swapOffsets = true>
    inline void endian_swap() noexcept
    {
        hl::endian_swap(version);
        hl::endian_swap<swapOffsets>(shapes);
        hl::endian_swap<swapOffsets>(groups);
        hl::endian_swap<swapOffsets>(bspNodes);
    }
};

HL_STATIC_ASSERT_SIZE(header32, 0x20);

struct header64
{
    u32 signature;
    u32 version;
    arr64<shape> shapes;
    arr64<shape_group> groups;
    arr64<bsp_node> bspNodes;

    template<bool swapOffsets = true>
    inline void endian_swap() noexcept
    {
        hl::endian_swap(version);
        hl::endian_swap<swapOffsets>(shapes);
        hl::endian_swap<swapOffsets>(groups);
        hl::endian_swap<swapOffsets>(bspNodes);
    }
};

HL_STATIC_ASSERT_SIZE(header64, 0x38);
} // v1
} // fxcol
} // hl
#endif
