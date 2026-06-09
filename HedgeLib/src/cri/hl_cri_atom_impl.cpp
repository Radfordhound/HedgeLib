#include "hl_cri_atom_impl.h"
#include "hedgelib/hl_internal.h"

namespace hl::cri::atom
{
// TODO: Move these functions to hl_cri_atom.cpp !!! ==========
command_table::const_iterator& command_table::const_iterator::operator++() noexcept
{
    const auto& cmd = *(*this);
    ptr_ += (sizeof(u16) + 1 + cmd.argument_count());
    return *this;
}

command_table::const_iterator command_table::const_iterator::operator++(int) noexcept
{
    const_iterator it;
    ++(*this);
    return it;
}

void command_table::write(rad::stream& stream) const
{
    stream.write_as(rawData.data(), rawData.size());
}

command_table::command_table(
    rad::allocator& allocator) noexcept
    : rawData(allocator)
{
}

command_table::command_table(rad::vector<unsigned char> data) noexcept
    : rawData(std::move(data))
{
}
// ============================================================

const revision_info_* get_revision_info_for_version_(
    rad::span<const revision_info_> revisionInfos,
    packed_version version) noexcept
{
    const revision_info_* result = nullptr;

    for (const auto& revisionInfo : revisionInfos)
    {
        if (version < revisionInfo.version) break;

        result = &revisionInfo;
    }
    
    return result;
}

ref_type parse_ref_type_(u8 type)
{
    if (type != static_cast<u8>(ref_type::waveform) &&
        type != static_cast<u8>(ref_type::synth) &&
        type != static_cast<u8>(ref_type::sequence) &&
        type != static_cast<u8>(ref_type::block_sequence))
    {
        throw std::runtime_error("Unsupported CriAtom ref type");
    }

    return static_cast<ref_type>(type);
}

ref_type parse_ref_type_(u16 type)
{
    if (type > UINT8_MAX)
    {
        throw std::runtime_error("Unsupported CriAtom ref type");
    }

    return parse_ref_type_(static_cast<u8>(type));
}

void deserialize_ref_items_array_(
    utf::deserializer& dr,
    utf::raw_buffer rawBuf,
    rad::vector<ref_item>& refItems)
{
    assert(refItems.empty() &&
        "The given output vector must be empty"
    );

    if (!rawBuf.size) return;

    const u32 refItemCount = rawBuf.size / 4;
    refItems.reserve(refItemCount);

    dr.reader().stream().jump_to(dr.get_buffer_data_position(rawBuf));

    for (u32 i = 0; i < refItemCount; ++i)
    {
        const auto refType = parse_ref_type_(dr.reader().read_u16());
        const auto index = dr.reader().read_u16();

        refItems.emplace_back_unchecked(refType, index);
    }
}

void deserialize_u16_array_(
    utf::deserializer& dr,
    utf::raw_buffer rawBuf,
    rad::vector<u16>& values)
{
    assert(values.empty() &&
        "The given output vector must be empty"
    );

    if (!rawBuf.size) return;

    const u32 valueCount = rawBuf.size / 2;
    values.reserve(valueCount);

    dr.reader().stream().jump_to(dr.get_buffer_data_position(rawBuf));
    
    for (u32 i = 0; i < valueCount; ++i)
    {
        values.emplace_back_unchecked(dr.reader().read_u16());
    }
}

void deserialize_f32_array_(
    utf::deserializer& dr,
    utf::raw_buffer rawBuf,
    rad::vector<float>& values)
{
    assert(values.empty() &&
        "The given output vector must be empty"
    );

    if (!rawBuf.size) return;

    const u32 valueCount = rawBuf.size / 4;
    values.reserve(valueCount);

    dr.reader().stream().jump_to(dr.get_buffer_data_position(rawBuf));
    
    for (u32 i = 0; i < valueCount; ++i)
    {
        values.emplace_back_unchecked(dr.reader().read_f32());
    }
}

void serialize_ref_items_array_(
    utf::buffers_resolver& br,
    const rad::vector<ref_item>& refItems)
{
    if (!refItems.empty())
    {
        br.start();

        for (const auto refItem : refItems)
        {
            br.writer().write_u16(static_cast<u16>(refItem.type));
            br.writer().write_u16(refItem.index);
        }
    }

    br.next();
}

void serialize_u16_array_(
    utf::buffers_resolver& br,
    const rad::vector<u16>& values)
{
    if (!values.empty())
    {
        br.start();

        for (const auto value : values)
        {
            br.writer().write_u16(value);
        }
    }

    br.next();
}

void serialize_f32_array_(
    utf::buffers_resolver& br,
    const rad::vector<float>& values)
{
    if (!values.empty())
    {
        br.start();

        for (const auto value : values)
        {
            br.writer().write_f32(value);
        }
    }

    br.next();
}
}
