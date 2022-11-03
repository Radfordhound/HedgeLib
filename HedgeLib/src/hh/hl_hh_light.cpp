#include "hedgelib/hh/hl_hh_light.h"
#include "hedgelib/io/hl_hh_mirage.h"
#include "hedgelib/hl_blob.h"

namespace hl::hh::mirage
{
void light::in_parse(const raw_light_v1& rawLight) noexcept
{
    type = rawLight.type;
    switch (type)
    {
    case light_type::directional:
        pos = rawLight.directionalData.dir;
        color = rawLight.directionalData.color;
        break;

    case light_type::point:
        pos = rawLight.pointData.pos;
        color = rawLight.pointData.color;
        attribute = rawLight.pointData.attribute;
        range = rawLight.pointData.range;
        break;
    }
}

void light::in_parse(const void* rawData)
{
    // Get light data and version number.
    u32 version;
    const auto lightData = get_data(rawData, &version);
    if (!lightData) return;

    // Parse light data based on version number.
    switch (version)
    {
    case 1:
        in_parse(*static_cast<const raw_light_v1*>(lightData));
        break;

    default:
        throw std::runtime_error("Unsupported HH mirage light version");
    }
}

void light::in_load(const nchar* filePath)
{
    // Load and parse light.
    blob rawLight(filePath);
    fix(rawLight);
    in_parse(rawLight);
}

void light::fix(void* rawData)
{
    // Fix mirage data.
    mirage::fix(rawData);

    // Get light data and version number.
    u32 version;
    const auto lightData = get_data(rawData, &version);
    if (!lightData) return;

    // Fix light data based on version number.
    switch (version)
    {
    case 1:
        static_cast<raw_light_v1*>(lightData)->fix();
        break;

    default:
        throw std::runtime_error("Unsupported HH mirage light version");
    }
}

void light::parse(const void* rawData, std::string name)
{
    // Set new name.
    this->name = std::move(name);

    // Parse data.
    in_parse(rawData);
}

void light::load(const nchar* filePath)
{
    // Set new name.
    name = std::move(get_res_name(filePath));

    // Load data.
    in_load(filePath);
}

light::light(const void* rawData, std::string name) :
    res_base(std::move(name))
{
    in_parse(rawData);
}

light::light(const nchar* filePath) :
    res_base(std::move(get_res_name(filePath)))
{
    in_load(filePath);
}
} // hl::hh::mirage
