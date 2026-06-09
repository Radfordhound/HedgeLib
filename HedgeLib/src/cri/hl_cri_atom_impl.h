#ifndef HL_CRI_ATOM_IMPL_H_INCLUDED
#define HL_CRI_ATOM_IMPL_H_INCLUDED

#include <rad/rad_span.h>
#include <rad/rad_vector.h>

#include "hedgelib/cri/hl_cri_utf.h"
#include "hedgelib/cri/hl_cri_atom.h"

namespace hl::cri::atom
{
struct revision_info_
{
    packed_version version;
    utf::column_info_group columnGroup;

    constexpr revision_info_(
        packed_version version,
        rad::span<const utf::column_info_range> columnRanges) noexcept
        : version(version)
        , columnGroup{columnRanges}
    {
    }

    constexpr u16 get_column_count() const noexcept
    {
        return columnGroup.get_column_count();
    }
};

const revision_info_* get_revision_info_for_version_(
    rad::span<const revision_info_> revisionInfos,
    packed_version version
) noexcept;

ref_type parse_ref_type_(u8 type);

ref_type parse_ref_type_(u16 type);

void deserialize_ref_items_array_(
    utf::deserializer& dr,
    utf::raw_buffer rawBuf,
    rad::vector<ref_item>& refItems
);

void deserialize_u16_array_(
    utf::deserializer& dr,
    utf::raw_buffer rawBuf,
    rad::vector<u16>& values
);

void deserialize_f32_array_(
    utf::deserializer& dr,
    utf::raw_buffer rawBuf,
    rad::vector<float>& values
);

void serialize_ref_items_array_(
    utf::buffers_resolver& br,
    const rad::vector<ref_item>& refItems
);

void serialize_u16_array_(
    utf::buffers_resolver& br,
    const rad::vector<u16>& values
);

void serialize_f32_array_(
    utf::buffers_resolver& br,
    const rad::vector<float>& values
);
}

#endif
