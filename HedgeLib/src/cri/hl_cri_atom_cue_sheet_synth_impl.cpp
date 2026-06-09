#include "hedgelib/cri/hl_cri_atom_cue_sheet.h"
#include "hl_cri_atom_impl.h"

namespace hl::cri::atom
{
static const utf::column_info synth_columns_[] =
{
    // r0 columns

    // group: 0, index: 0, count: 9
    { utf::cell_type::u8, "Type" },
    { utf::cell_type::string, "VoiceLimitGroupName" },
    { utf::cell_type::u16, "CommandIndex" },
    { utf::cell_type::buffer, "ReferenceItems" },
    { utf::cell_type::buffer, "LocalAisacs" },
    { utf::cell_type::u16, "GlobalAisacStartIndex" },
    { utf::cell_type::u16, "GlobalAisacNumRefs" },
    { utf::cell_type::u16, "ControlWorkArea1" },
    { utf::cell_type::u16, "ControlWorkArea2" },

    // r1 columns

    // group: 1, index: 9, count: 2
    { utf::cell_type::buffer, "TrackValues" },
    { utf::cell_type::u16, "ParameterPallet" },

    // r2 columns

    // group: 2, index: 11, count: 2
    { utf::cell_type::u16, "ActionTrackStartIndex" },
    { utf::cell_type::u16, "NumActionTracks" },
};

static constexpr utf::column_info_range synth_columns_r0_[] =
{
    { 0, 9 }, // Type - ControlWorkArea2
};

static constexpr utf::column_info_range synth_columns_r1_[] =
{
    { 0, 11 }, // Type - ParameterPallet
};

static constexpr utf::column_info_range synth_columns_r2_[] =
{
    { 0, 13 }, // Type - NumActionTracks
};

static constexpr revision_info_ synth_revisions_[] =
{
    { packed_version(0), synth_columns_r0_ }, // r0
    { packed_version(1, 12, 00), synth_columns_r1_ }, // r1
    { packed_version(1, 20, 03), synth_columns_r2_ }, // r2
};

static synth_type parse_synth_type_(u8 type)
{
    if (type != static_cast<u8>(synth_type::polyphonic) &&
        type != static_cast<u8>(synth_type::sequential) &&
        type != static_cast<u8>(synth_type::shuffle) &&
        type != static_cast<u8>(synth_type::random) &&
        type != static_cast<u8>(synth_type::random_no_repeat) &&
        type != static_cast<u8>(synth_type::switch_game_variable) &&
        type != static_cast<u8>(synth_type::combo_sequential) &&
        type != static_cast<u8>(synth_type::switch_selector) &&
        type != static_cast<u8>(synth_type::track_transition_by_selector))
    {
        throw std::runtime_error("Unsupported CriAtom synth type");
    }

    return static_cast<synth_type>(type);
}

void read_synth_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    packed_version version,
    rad::vector<synth>& synths)
{
    assert(synths.empty() &&
        "The given output vector must be empty"
    );

    // Read raw table header.
    utf::deserializer dr(stream, utf::deserialize_type::utf, tmpAllocator);

    // Get revision info.
    const auto revisionInfo = get_revision_info_for_version_(
        synth_revisions_,
        version
    );

    // Validate columns.
    if (!dr.has_columns_of_exact_types(revisionInfo->columnGroup, synth_columns_))
    {
        throw std::runtime_error("Invalid or unsupported ACB Synth layout");
    }

    // Read rows.
    auto& synthAllocator = synths.allocator();
    synths.reserve(dr.row_count());

    for (u32 i = 0; i < dr.row_count(); dr.next_row(), ++i)
    {
        // r0 
        const auto type = parse_synth_type_(dr.next_cell_as_u8()); // Type
        auto& synth = synths.emplace_back_unchecked(type, synthAllocator);

        synth.voiceLimitGroupName = dr.get_string_data( // VoiceLimitGroupName
            dr.next_cell_as_string()
        );

        synth.commandIndex = dr.next_cell_as_u16(); // CommandIndex
        const auto rawRefItems = dr.next_cell_as_buffer(); // ReferenceItems
        const auto rawLocalAisacs = dr.next_cell_as_buffer(); // LocalAisacs
        synth.globalAisacStartIndex = dr.next_cell_as_u16(); // GlobalAisacStartIndex
        synth.globalAisacCount = dr.next_cell_as_u16(); // GlobalAisacNumRefs

        dr.skip_cell(); // ControlWorkArea1
        dr.skip_cell(); // ControlWorkArea2

        // r1
        utf::raw_buffer rawTrackValues = {};

        if (version >= synth_revisions_[1].version)
        {
            rawTrackValues = dr.next_cell_as_buffer(); // TrackValues
            const auto parameterPallet = dr.next_cell_as_u16(); // ParameterPallet // TODO
        }

        // r2
        if (version >= synth_revisions_[2].version)
        {
            const auto actionTrackStartIndex = dr.next_cell_as_u16(); // ActionTrackStartIndex // TODO
            const auto numActionTracks = dr.next_cell_as_u16(); // NumActionTracks // TODO
        }

        // ReferenceItems
        deserialize_ref_items_array_(dr, rawRefItems, synth.refItems);

        // LocalAisacs
        deserialize_u16_array_(dr, rawLocalAisacs, synth.localAisacIndices);

        if (version < synth_revisions_[1].version) continue;

        // TrackValues
        deserialize_u16_array_(dr, rawTrackValues, synth.trackValues);
    }
}

void write_synth_table_(
    detail_::write_params& wp,
    const rad::vector<synth>& synths)
{
    utf::serializer sr(*wp.stream, *wp.allocator);

    const auto revisionInfo = get_revision_info_for_version_(
        synth_revisions_,
        wp.version
    );

    sr.start(
        "Synth",
        { synth_columns_, revisionInfo->get_column_count() },
        wp.encoding
    );

    // Write rows.
    for (std::size_t i = 0; i < synths.size(); sr.next_row(), ++i)
    {
        const auto& synth = synths[i];

        // r0
        sr.push_cell_u8(static_cast<u8>(synth.type)); // Type
        sr.push_cell_string(synth.voiceLimitGroupName); // VoiceLimitGroupName

        sr.push_cell_u16(
            wp.get_command_index(synth.commandIndex)
        ); // CommandIndex

        sr.push_cell_buffer(synth.refItems.empty()); // ReferenceItems
        sr.push_cell_buffer(synth.localAisacIndices.empty()); // LocalAisacs

        sr.push_cell_u16(synth.globalAisacStartIndex); // GlobalAisacStartIndex
        sr.push_cell_u16(synth.globalAisacCount); // GlobalAisacNumRefs

        if (synth.type == synth_type::random ||
            synth.type == synth_type::random_no_repeat)
        {
            sr.push_cell_u16(UINT16_MAX); // ControlWorkArea1
            sr.push_cell_u16(UINT16_MAX); // ControlWorkArea2
        }
        else
        {
            sr.push_cell_u16(static_cast<u16>(i)); // ControlWorkArea1
            sr.push_cell_u16(static_cast<u16>(i)); // ControlWorkArea2
        }

        // r1
        if (wp.version < synth_revisions_[1].version) continue;

        sr.push_cell_buffer(synth.trackValues.empty()); // TrackValues
        sr.push_cell_u16(UINT16_MAX); // ParameterPallet // TODO

        // r2
        if (wp.version < synth_revisions_[2].version) continue;

        sr.push_cell_u16(UINT16_MAX); // ActionTrackStartIndex // TODO
        sr.push_cell_u16(0); // NumActionTracks // TODO
    }

    // Write buffers.
    auto br = sr.begin_buffer_data_section();

    //for (std::size_t cellIndex = 0, i = 0;
        //i < synths.size();
        //cellIndex += sr.column_count(), ++i)
    for (const auto& synth : synths)
    {
        // ReferenceItems
        serialize_ref_items_array_(br, synth.refItems);

        // LocalAisacs
        serialize_u16_array_(br, synth.localAisacIndices);

        if (wp.version < synth_revisions_[1].version) continue;

        // TrackValues
        serialize_u16_array_(br, synth.trackValues);
    }

    // Finish writing table.
    sr.writer().stream().pad(4);
    sr.finish();
}
}
