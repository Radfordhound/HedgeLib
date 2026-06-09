#include "hedgelib/cri/hl_cri_atom_cue_sheet.h"
#include "hl_cri_atom_impl.h"

namespace hl::cri::atom
{
static const utf::column_info track_columns_[] =
{
    // r0 columns

    // group: 0, index: 0, count: 5
    { utf::cell_type::u16, "EventIndex" },
    { utf::cell_type::u16, "CommandIndex" },
    { utf::cell_type::buffer, "LocalAisacs" },
    { utf::cell_type::u16, "GlobalAisacStartIndex" },
    { utf::cell_type::u16, "GlobalAisacNumRefs" },

    // r1 columns

    // group: 1, index: 5, count: 1
    { utf::cell_type::u16, "ParameterPallet" },

    // r2 columns

    // group: 2, index: 6, count: 6
    { utf::cell_type::u8, "TargetType" },
    { utf::cell_type::string, "TargetName" },
    { utf::cell_type::u32, "TargetId" },
    { utf::cell_type::string, "TargetAcbName" },
    { utf::cell_type::u8, "Scope" },
    { utf::cell_type::u16, "TargetTrackNo" },

    // r3 columns

    // group: 3, index: 12, count: 1
    { utf::cell_type::u16, "BeatSyncLabel" },

    // r4 columns
    { utf::cell_type::u8, "TargetIdentificationCondition" },
};

static const utf::column_info_range track_columns_r0_[] =
{
    { 0, 5 },   // EventIndex - GlobalAisacNumRefs
};

static const utf::column_info_range track_columns_r1_[] =
{
    { 0, 6 },   // EventIndex - ParameterPallet
};

static const utf::column_info_range track_columns_r2_[] =
{
    { 0, 12 },  // EventIndex - TargetTrackNo
};

static const utf::column_info_range track_columns_r3_[] =
{
    { 0, 13 },  // EventIndex - BeatSyncLabel
};

static const utf::column_info_range track_columns_r4_[] =
{
    { 0, 14 },  // EventIndex - TargetIdentificationCondition
};

static const revision_info_ track_revisions_[] =
{
    { packed_version(0), track_columns_r0_ }, // r0
    { packed_version(1, 12, 00), track_columns_r1_ }, // r1
    { packed_version(1, 20, 03), track_columns_r2_ }, // r2
    { packed_version(1, 42, 01), track_columns_r3_ }, // r3
    { packed_version(1, 44, 01), track_columns_r4_ }, // r4
};

void read_track_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    packed_version version,
    rad::vector<track>& tracks)
{
    assert(tracks.empty() &&
        "The given output vector must be empty"
    );

    // Read raw table header.
    utf::deserializer dr(stream, utf::deserialize_type::utf, tmpAllocator);

    // Get revision info.
    const auto revisionInfo = get_revision_info_for_version_(
        track_revisions_,
        version
    );

    // Validate columns.
    if (!dr.has_columns_of_exact_types(revisionInfo->columnGroup, track_columns_))
    {
        throw std::runtime_error("Invalid or unsupported ACB Track layout");
    }

    // Read rows.
    auto& trackAllocator = tracks.allocator();
    tracks.reserve(dr.row_count());

    for (u32 i = 0; i < dr.row_count(); dr.next_row(), ++i)
    {
        auto& track = tracks.emplace_back_unchecked(trackAllocator);

        // r0
        track.eventIndex = dr.next_cell_as_u16(); // EventIndex
        track.commandIndex = dr.next_cell_as_u16(); // CommandIndex
        const auto rawLocalAisacs = dr.next_cell_as_buffer(); // LocalAisacs

        track.globalAisacStartIndex = dr.next_cell_as_u16(); // GlobalAisacStartIndex
        track.globalAisacCount = dr.next_cell_as_u16(); // GlobalAisacNumRefs

        // r1
        if (version >= track_revisions_[1].version)
        {
            const auto parameterPallet = dr.next_cell_as_u16(); // ParameterPallet // TODO
        }

        // r2
        if (version >= track_revisions_[2].version)
        {
            const auto targetType = dr.next_cell_as_u8(); // TargetType // TODO
            const auto targetName = dr.next_cell_as_string(); // TargetName // TODO
            const auto targetId = dr.next_cell_as_u32(); // TargetId // TODO
            const auto targetAcbName = dr.next_cell_as_string(); // TargetAcbName // TODO
            const auto scope = dr.next_cell_as_u8(); // Scope // TODO
            const auto targetTrackNo = dr.next_cell_as_u16(); // TargetTrackNo // TODO
        }

        // r3
        if (version >= track_revisions_[3].version)
        {
            const auto beatSyncLabel = dr.next_cell_as_u16(); // BeatSyncLabel // TODO
        }

        // r4
        if (version >= track_revisions_[4].version)
        {
            const auto targetIdentificationCondition = dr.next_cell_as_u8(); // TargetIdentificationCondition // TODO
        }

        // LocalAisacs
        deserialize_u16_array_(dr, rawLocalAisacs, track.localAisacIndices);
    }
}

void write_track_table_(
    detail_::write_params& wp,
    const rad::vector<track>& tracks)
{
    utf::serializer sr(*wp.stream, *wp.allocator);

    const auto revisionInfo = get_revision_info_for_version_(
        track_revisions_,
        wp.version
    );

    sr.start(
        "Track",
        { track_columns_, revisionInfo->get_column_count() },
        wp.encoding
    );

    // Write rows.
    for (std::size_t i = 0; i < tracks.size(); sr.next_row(), ++i)
    {
        const auto& track = tracks[i];

        // r0
        sr.push_cell_u16(track.eventIndex); // EventIndex
        sr.push_cell_u16(track.commandIndex); // CommandIndex
        sr.push_cell_buffer(track.localAisacIndices.empty()); // LocalAisacs

        sr.push_cell_u16(track.globalAisacStartIndex); // GlobalAisacStartIndex
        sr.push_cell_u16(track.globalAisacCount); // GlobalAisacNumRefs

        // r1
        if (wp.version < track_revisions_[1].version) continue;

        sr.push_cell_u16(UINT16_MAX); // ParameterPallet // TODO

        // r2
        if (wp.version < track_revisions_[2].version) continue;

        sr.push_cell_u8(0); // TargetType // TODO
        sr.push_cell_string(std::string_view{}); // TargetName // TODO
        sr.push_cell_u32(UINT32_MAX); // TargetId // TODO
        sr.push_cell_string(std::string_view{}); // TargetAcbName // TODO
        sr.push_cell_u8(0); // Scope // TODO
        sr.push_cell_u16(UINT16_MAX); // TargetTrackNo // TODO

        // r3
        if (wp.version < track_revisions_[3].version) continue;

        sr.push_cell_u16(UINT16_MAX); // BeatSyncLabel // TODO

        // r4
        if (wp.version < track_revisions_[4].version) continue;

        sr.push_cell_u8(0); // TargetIdentificationCondition // TODO
    }

    // Write buffers.
    auto br = sr.begin_buffer_data_section();

    for (const auto& track : tracks)
    {
        // LocalAisacs
        serialize_u16_array_(br, track.localAisacIndices);
    }

    // Finish writing table.
    sr.finish();
}
}
