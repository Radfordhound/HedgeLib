#include "hedgelib/cri/hl_cri_atom_cue_sheet.h"
#include <rad/rad_stack_or_heap_array.h>

namespace hl::cri_new::atom
{
static const utf::column_info cue_columns_[] =
{
    // r0 columns

    // group: 0, index: 0, count: 8
    { utf::cell_type::u32, "CueId" },
    { utf::cell_type::u8, "ReferenceType" },
    { utf::cell_type::u16, "ReferenceIndex" },
    { utf::cell_type::string, "UserData" },
    { utf::cell_type::u16, "Worksize" },
    { utf::cell_type::buffer, "AisacControlMap" },
    { utf::cell_type::u32, "Length" },
    { utf::cell_type::u8, "NumAisacControlMaps" },

    // r1 columns

    // group: 1, index: 8, count: 1
    { utf::cell_type::u8, "HeaderVisibility" },

    // r2 columns

    // group: 2, index: 9, count: 1
    { utf::cell_type::u16, "NumRelatedWaveforms" },
};

static const utf::column_info_range cue_columns_r0_[] =
{
    { 0, 8 },   // CueId - NumAisacControlMaps
};

static const utf::column_info_range cue_columns_r1_[] =
{
    { 0, 9 },   // CueId - HeaderVisibility
};

static const utf::column_info_range cue_columns_r2_[] =
{
    { 0, 10 },  // CueId - NumRelatedWaveforms
};

static const revision_info cue_revisions_[] =
{
    { packed_version(0), cue_columns_r0_ }, // r0
    { packed_version(1, 20, 03), cue_columns_r1_ }, // r1
    { packed_version(1, 37, 00), cue_columns_r2_ }, // r2
};

static const utf::column_info cue_name_columns_[] =
{
    // r0 columns
    { utf::cell_type::string, "CueName" },
    { utf::cell_type::u16, "CueIndex" },
};

void cue_sheet::read_cue_table_(
    rad::stream& stream,
    packed_version version)
{
    // Read raw table header.
    utf::table_deserializer td(stream);

    const auto revisionInfo = get_revision_info(
        cue_revisions_,
        version
    );

    if (!revisionInfo->columns.validate(cue_columns_, td))
    {
        throw std::runtime_error("Invalid or unsupported ACB Cue layout");
    }

    // Read rows.
    cues.reserve(td.row_count());

    for (u32 i = 0; i < td.row_count(); td.next_row(), ++i)
    {
        auto& cue = cues.emplace_back();

        // r0
        cue.id = td.read_cell_as_u32();

        const auto refType = td.read_cell_as_u8();
        detail_::validate_reference_type(refType);

        cue.refItem.type = static_cast<reference_type>(refType);
        cue.refItem.index = td.read_cell_as_u16();

        const auto rawUserData = td.read_cell_as_string();
        cue.userData = td.get_string_data(rawUserData);

        const auto worksize = td.read_cell_as_u16(); // TODO
        const auto rawAisacControlMap = td.read_cell_as_buffer();
        cue.playDuration = td.read_cell_as_u32();
        const auto aisacControlMapCount = td.read_cell_as_u8();

        if (aisacControlMapCount)
        {
            if (aisacControlMapCount > rawAisacControlMap.size)
            {
                throw std::runtime_error("Invalid AISAC Control Map Count");
            }

            // Read AISAC Control Map.
            const auto curPos = stream.tell();
            stream.jump_to(td.get_buffer_data_position(rawAisacControlMap));

            cue.aisacControlMap.assign(rad::no_value_init, aisacControlMapCount);
            stream.read(cue.aisacControlMap.data(), aisacControlMapCount);
            stream.jump_to(curPos);
        }

        // r1
        if (version < cue_revisions_[1].version) continue;

        const auto headerVisibility = td.read_cell_as_u8(); // TODO

        // r2
        if (version < cue_revisions_[2].version) continue;

        td.read_cell_as_u16(); // NumRelatedWaveforms
    }
}

void cue_sheet::write_cue_name_table_(detail_::write_params& wp) const
{
    utf::table_serializer ts(*wp.stream, *wp.allocator);

    ts.start("CueName", cue_name_columns_, wp.encoding);

    // Sort cue names.
    rad::stack_or_heap_array<unsigned short, 256> cueIndices(
        rad::no_value_init, cues.size()
    );

    auto cueIndicesEnd = cueIndices.begin();
    const auto cueCount = static_cast<unsigned short>(cues.size());

    for (unsigned short i = 0; i < cueCount; ++i)
    {
        if (!cues[i].name.empty())
        {
            *cueIndicesEnd = i;
            ++cueIndicesEnd;
        }
    }

    std::sort(cueIndices.begin(), cueIndicesEnd,
        [&cues = this->cues](unsigned short a, unsigned short b)
        {
            return std::strcmp(cues[a].name.data(), cues[b].name.data()) < 0;
        }
    );

    // Write rows.
    for (auto it = cueIndices.begin(); it != cueIndicesEnd; ++it)
    {
        const auto cueIndex = *it;

        ts.write_cell_as_string(cues[cueIndex].name); // CueName
        ts.write_cell_as_u16(cueIndex); // CueIndex

        ts.next_row();
    }

    // Finish writing rows.
    ts.finish_rows();

    // Finish writing table.
    ts.finish();
    wp.stream->pad(32);
}

void cue_sheet::write_cue_table_(detail_::write_params& wp) const
{
    utf::table_serializer ts(*wp.stream, *wp.allocator);

    const auto revisionInfo = get_revision_info(
        cue_revisions_,
        wp.version
    );

    ts.start(
        "Cue",
        { cue_columns_, revisionInfo->columns.get_total_count() },
        wp.encoding
    );

    // Write rows.
    for (std::size_t i = 0; i < cues.size(); ts.next_row(), ++i)
    {
        const auto& cue = cues[i];

        // r0
        ts.write_cell_as_u32(cue.id); // CueId
        ts.write_cell_as_u8(static_cast<u8>(cue.refItem.type)); // ReferenceType
        ts.write_cell_as_u16(cue.refItem.index); // ReferenceIndex
        ts.write_cell_as_string(cue.userData); // UserData
        
        ts.write_cell_as_u16(0); // Worksize
        ts.write_cell_as_buffer(); // AisacControlMap
        ts.write_cell_as_u32(cue.playDuration); // Length

        if (cue.aisacControlMap.size() > 255)
        {
            throw std::runtime_error("Can only write AISAC Control Maps up to 255 bytes in size");
        }

        ts.write_cell_as_u8(static_cast<u8>(cue.aisacControlMap.size())); // NumAisacControlMaps

        // r1
        if (wp.version < cue_revisions_[1].version) continue;

        // TODO: The following value
        ts.write_cell_as_u8(1); // HeaderVisibility

        // r2
        if (wp.version < cue_revisions_[2].version) continue;

        ts.write_cell_as_u16(
            cue.compute_related_waveform_count(*this)
        ); // NumRelatedWaveforms
    }

    // Finish writing rows.
    ts.finish_rows();

    // Write buffers.
    for (std::size_t cellIndex = 0, i = 0;
        i < cues.size();
        cellIndex += ts.column_count(), ++i)
    {
        const auto& cue = cues[i];

        // Write AisacControlMap.
        if (!cue.aisacControlMap.empty())
        {
            const auto aisacControlMapDataPos = wp.stream->tell();
            ts.stream().write(cue.aisacControlMap.data(), cue.aisacControlMap.size());

            ts.fill_buffer_cell(cellIndex + 5, aisacControlMapDataPos);
        }
    }

    // Finish writing table.
    ts.finish();
    wp.stream->pad(32);
}
}
