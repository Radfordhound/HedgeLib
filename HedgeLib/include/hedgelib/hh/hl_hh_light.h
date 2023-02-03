#ifndef HL_HH_LIGHT_H_INCLUDED
#define HL_HH_LIGHT_H_INCLUDED
#include "../hl_math.h"
#include "../hl_resource.h"

namespace hl::hh::mirage
{
enum class raw_light_type : u32
{
    directional = 0,
    point = 1
};

struct raw_light_v1
{
    raw_light_type type;
    union
    {
        struct
        {
            vec3 dir;
            vec3 color;
        }
        directionalData;

        struct
        {
            vec3 pos;
            vec3 color;
            u32 attribute;
            vec4 range;
        }
        pointData;
    };

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(type);

        // NOTE: This check is always safe, since type will be equal to 0
        // regardless of whether the data is in big endian or little endian.
        if (type == raw_light_type::directional)
        {
            hl::endian_swap<swapOffsets>(directionalData.dir);
            hl::endian_swap<swapOffsets>(directionalData.color);
        }
        else
        {
            hl::endian_swap<swapOffsets>(pointData.pos);
            hl::endian_swap<swapOffsets>(pointData.color);
            hl::endian_swap<swapOffsets>(pointData.attribute);
            hl::endian_swap<swapOffsets>(pointData.range);
        }
    }

    inline void fix()
    {
#ifndef HL_IS_BIG_ENDIAN
        endian_swap<false>();
#endif
    }
};

HL_STATIC_ASSERT_SIZE(raw_light_v1, 0x30);

using light_type = raw_light_type;

class light : public res_base
{
    HL_API void in_parse(const raw_light_v1& rawLight) noexcept;

    HL_API void in_parse(const void* rawData);

    HL_API void in_load(const nchar* filePath);

public:
    light_type type = light_type::directional;
    vec3 pos = vec3::zero();
    vec3 color = vec3::one();
    u32 attribute = 0;
    vec4 range = vec4::one();

    inline constexpr static nchar ext[] = HL_NTEXT(".light");

    HL_API static void fix(void* rawData);

    HL_API void parse(const void* rawData, std::string name);

    HL_API void load(const nchar* filePath);

    inline void load(const nstring& filePath)
    {
        load(filePath.c_str());
    }

    light() noexcept :
        res_base("default_light") {}

    HL_API light(const void* rawData, std::string name);

    HL_API light(const nchar* filePath);

    inline light(const nstring& filePath) :
        light(filePath.c_str()) {}
};
} // hl::hh::mirage
#endif
