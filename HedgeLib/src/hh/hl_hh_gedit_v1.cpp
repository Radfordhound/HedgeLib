#include "hedgelib/hh/hl_hh_gedit.h"
#include "hedgelib/sets/hl_hson.h"

namespace hl
{
namespace hh
{
namespace gedit
{
namespace v1
{
guid raw_object_id::as_guid() const noexcept
{
    // Generates a GUID like this:
    // 00000000-AAAA-BBBB-0000-000000000000
    // (where A is object ID and B is group ID)

    // TODO: Will this ever generate a NULL UUID by accident?? It should be fine if Forces also uses 0 as null.

    return guid(
        0, 0, 0, 0,
        // -
        static_cast<unsigned char>((objID >> 8) & 0xFF),
        static_cast<unsigned char>(objID & 0xFF),
        // -
        static_cast<unsigned char>((groupID >> 8) & 0xFF),
        static_cast<unsigned char>(groupID & 0xFF),
        // -
        0, 0,
        // -
        0, 0, 0, 0, 0, 0
    );
}

matrix4x4A raw_transform::as_matrix() const noexcept
{
    return matrix4x4A(pos, rot);
}

raw_transform::raw_transform(const matrix4x4& mtx)
{
    quat rotQuat;
    mtx.decompose(&pos, &rotQuat);

    rot = rotQuat.as_euler();
}

raw_transform::raw_transform(const matrix4x4A& mtx)
{
    quat rotQuat;
    mtx.decompose(&pos, &rotQuat);

    rot = rotQuat.as_euler();
}

hson::parameter raw_tag_data_range_spawning::as_hson_parameter() const
{
    hson::parameter hsonParam(hson::parameter_type::object);

    // Set rangeIn value.
    auto& hsonRangeIn = hsonParam.value_object().insert("rangeIn",
        hson::parameter_type::floating).first->second;

    hsonRangeIn.value_floating() = rangeIn;

    // Set rangeOut value.
    auto& hsonRangeOut = hsonParam.value_object().insert("rangeOut",
        hson::parameter_type::floating).first->second;

    hsonRangeOut.value_floating() = rangeOut;

    return hsonParam;
}

static void in_fix(raw_world& rawHeader)
{
    // Endian-swap header.
    rawHeader.endian_swap<false>();

    // Endian-swap objects.
    for (auto& rawObjOff : rawHeader.objects)
    {
        // Skip this object if its offset is null.
        if (!rawObjOff) continue;

        // Endian-swap this object.
        auto& rawObj = *rawObjOff;
        rawObj.endian_swap<false>();

        // Endian-swap tags.
        for (auto& rawTagOff : rawObj.tags)
        {
            // Skip this tag if its offset is null.
            if (!rawTagOff) continue;

            // Endian-swap this tag.
            auto& rawTag = *rawTagOff;
            rawTag.endian_swap<false>();

            // Endian-swap tag data.
            const auto rawTagType = rawTag.type.get();
            if (text::equal(rawTagType, tag_range_spawning))
            {
                auto& rawTagData = *rawTag.data<raw_tag_data_range_spawning>();
                rawTagData.endian_swap<false>();
            }
        }

        // TODO: Endian-swap parameter data.
    }
}

void raw_world::fix(bina::endian_flag endianFlag)
{
    if (bina::needs_swap(endianFlag))
    {
        in_fix(*this);
    }
}
} // v1
} // gedit
} // hh
} // hl
