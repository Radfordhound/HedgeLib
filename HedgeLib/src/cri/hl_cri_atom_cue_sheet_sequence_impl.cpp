#include "hedgelib/cri/hl_cri_atom_cue_sheet.h"
#include "hl_cri_atom_impl.h"

namespace hl::cri::atom
{
static const utf::column_info sequence_columns_[] =
{
    // r0 columns

    // group: 0, index: 0, count: 1
    { utf::cell_type::u16, "Tempo" },

    // group: 1, index: 1, count: 3
    { utf::cell_type::u16, "NumTracks" },
    { utf::cell_type::buffer, "TrackIndex" },
    { utf::cell_type::u16, "CommandIndex" },

    // r1 columns

    // group: 2, index: 4, count: 1
    { utf::cell_type::u16, "PlaybackRatio" }, // Tempo

    // group: 3, index: 5, count: 4
    { utf::cell_type::buffer, "LocalAisacs" },
    { utf::cell_type::u16, "GlobalAisacStartIndex" },
    { utf::cell_type::u16, "GlobalAisacNumRefs" },
    { utf::cell_type::u16, "ParameterPallet" },

    // r2 columns

    // group: 4, index: 9, count: 5
    { utf::cell_type::u16, "ActionTrackStartIndex" },
    { utf::cell_type::u16, "NumActionTracks" },
    { utf::cell_type::buffer, "TrackValues" },
    { utf::cell_type::u8, "Type" },
    { utf::cell_type::u16, "ControlWorkArea1" },

    // group: 5, index: 14, count: 1
    { utf::cell_type::u16, "ControlWorkArea2" },

    // r3 columns
    
    // group: 6, index: 15, count: 2
    { utf::cell_type::u16, "InstPluginTrackStartIndex" },
    { utf::cell_type::u16, "NumInstPluginTracks" },

    // r4 columns

    // group: 7, index: 17, count: 1
    { utf::cell_type::buffer, "NumPlaybackTrackNoHistories" }, // ControlWorkArea2

    // group: 8, index: 18, count: 6
    { utf::cell_type::u16, "MIDITrackStartIndex" },
    { utf::cell_type::u16, "NumMIDITracks" },
    { utf::cell_type::u16, "WatchActionStartIndex" },
    { utf::cell_type::u16, "NumWatchAction" },
    { utf::cell_type::u16, "StopActionStartIndex" },
    { utf::cell_type::u16, "NumStopAction" },
};

static constexpr utf::column_info_range sequence_columns_r0_[] =
{
    { 0, 4 },   // Tempo - CommandIndex
};

static constexpr utf::column_info_range sequence_columns_r1_[] =
{
    { 4, 1 },   // PlaybackRatio
    { 1, 3 },   // NumTracks - CommandIndex
    { 5, 4 },   // LocalAisacs - ParameterPallet
};

static constexpr utf::column_info_range sequence_columns_r2_[] =
{
    { 4, 1 },   // PlaybackRatio
    { 1, 3 },   // NumTracks - CommandIndex
    { 5, 10 },  // LocalAisacs - ControlWorkArea2
};

static constexpr utf::column_info_range sequence_columns_r3_[] =
{
    { 4, 1 },   // PlaybackRatio
    { 1, 3 },   // NumTracks - CommandIndex
    { 5, 12 },  // LocalAisacs - NumInstPluginTracks
};

static constexpr utf::column_info_range sequence_columns_r4_[] =
{
    { 4, 1 },   // PlaybackRatio
    { 1, 3 },   // NumTracks - CommandIndex
    { 5, 9 },   // LocalAisacs - ControlWorkArea1
    { 17, 1 },  // NumPlaybackTrackNoHistories
    { 15, 2 },  // InstPluginTrackStartIndex - NumInstPluginTracks
    { 18, 6 },  // MIDITrackStartIndex - NumStopAction
};

static constexpr revision_info_ sequence_revisions_[] =
{
    { packed_version(0), sequence_columns_r0_ }, // r0
    { packed_version(1, 12, 00), sequence_columns_r1_ }, // r1
    { packed_version(1, 20, 03), sequence_columns_r2_ }, // r2
    { packed_version(1, 37, 00), sequence_columns_r3_ }, // r3
    { packed_version(1, 42, 01), sequence_columns_r4_ }, // r4
};

static sequence_type parse_sequence_type_(u8 type)
{
    if (type != static_cast<u8>(sequence_type::polyphonic) &&
        type != static_cast<u8>(sequence_type::sequential) &&
        type != static_cast<u8>(sequence_type::shuffle) &&
        type != static_cast<u8>(sequence_type::random) &&
        type != static_cast<u8>(sequence_type::random_no_repeat) &&
        type != static_cast<u8>(sequence_type::switch_game_variable) &&
        type != static_cast<u8>(sequence_type::combo_sequential) &&
        type != static_cast<u8>(sequence_type::switch_selector) &&
        type != static_cast<u8>(sequence_type::track_transition_by_selector))
    {
        throw std::runtime_error("Unsupported CriAtom sequence type");
    }

    return static_cast<sequence_type>(type);
}

void read_sequence_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    packed_version version,
    rad::vector<sequence>& sequences)
{
    assert(sequences.empty() &&
        "The given output vector must be empty"
    );

    // Read raw table header.
    utf::deserializer dr(stream, utf::deserialize_type::utf, tmpAllocator);

    // Get revision info.
    const auto revisionInfo = get_revision_info_for_version_(
        sequence_revisions_,
        version
    );

    // Validate columns.
    if (!dr.has_columns_of_exact_types(revisionInfo->columnGroup, sequence_columns_))
    {
        throw std::runtime_error("Invalid or unsupported ACB Sequence layout");
    }

    // Read rows.
    auto& sequenceAllocator = sequences.allocator();
    sequences.reserve(dr.row_count());

    for (u32 i = 0; i < dr.row_count(); dr.next_row(), ++i)
    {
        auto& sequence = sequences.emplace_back_unchecked(
            sequence_type::polyphonic,
            sequenceAllocator
        );

        // r0
        // TODO: Write Tempo correctly for r0 files. How does Tempo differ from playbackRatio ??
        sequence.playbackRatio = dr.next_cell_as_u16(); // PlaybackRatio
        const auto trackCount = dr.next_cell_as_u16(); // NumTracks
        const auto rawTrackIndex = dr.next_cell_as_buffer(); // TrackIndex
        sequence.commandIndex = dr.next_cell_as_u16(); // CommandIndex

        // r1
        utf::raw_buffer rawLocalAisacs = {};

        if (version >= sequence_revisions_[1].version)
        {
            rawLocalAisacs = dr.next_cell_as_buffer(); // LocalAisacs
            sequence.globalAisacStartIndex = dr.next_cell_as_u16(); // GlobalAisacStartIndex
            sequence.globalAisacCount = dr.next_cell_as_u16(); // GlobalAisacNumRefs
            const auto parameterPallet = dr.next_cell_as_u16(); // ParameterPallet // TODO
        }

        // r2
        utf::raw_buffer rawTrackValues = {};
        utf::raw_buffer rawNumPlaybackTrackNoHistories = {};

        if (version >= sequence_revisions_[2].version)
        {
            const auto actionTrackStartIndex = dr.next_cell_as_u16(); // ActionTrackStartIndex // TODO
            const auto numActionTracks = dr.next_cell_as_u16(); // NumActionTracks // TODO
            rawTrackValues = dr.next_cell_as_buffer(); // TrackValues
            sequence.type = parse_sequence_type_(dr.next_cell_as_u8()); // Type
            dr.skip_cell(); // ControlWorkArea1

            if (version >= sequence_revisions_[4].version)
            {
                rawNumPlaybackTrackNoHistories = dr.next_cell_as_buffer(); // NumPlaybackTrackNoHistories
            }
            else
            {
                dr.skip_cell(); // ControlWorkArea2
            }
        }

        // r3
        if (version >= sequence_revisions_[3].version)
        {
            const auto instPluginTrackStartIndex = dr.next_cell_as_u16(); // InstPluginTrackStartIndex // TODO
            const auto numInstPluginTracks = dr.next_cell_as_u16(); // NumInstPluginTracks // TODO
        }

        // r4
        if (version >= sequence_revisions_[4].version)
        {
            const auto midiTrackStartIndex = dr.next_cell_as_u16(); // MIDITrackStartIndex // TODO
            const auto numMidiTracks = dr.next_cell_as_u16(); // NumMIDITracks // TODO
            const auto watchActionStartIndex = dr.next_cell_as_u16(); // WatchActionStartIndex // TODO
            const auto numWatchAction = dr.next_cell_as_u16(); // NumWatchAction // TODO
            const auto stopActionStartIndex = dr.next_cell_as_u16(); // StopActionStartIndex // TODO
            const auto numStopAction = dr.next_cell_as_u16(); // NumStopAction // TODO
        }

        // TrackIndex
        deserialize_u16_array_(dr, rawTrackIndex, sequence.trackIndices);

        if (version < sequence_revisions_[1].version) continue;

        // LocalAisacs
        deserialize_u16_array_(dr, rawLocalAisacs, sequence.localAisacIndices);

        if (version < sequence_revisions_[1].version) continue;

        // TrackValues
        deserialize_u16_array_(dr, rawTrackValues, sequence.trackValues);

        if (version >= sequence_revisions_[4].version)
        {
            // NumPlaybackTrackNoHistories
            // TODO
        }
    }
}

void write_sequence_table_(
    detail_::write_params& wp,
    const rad::vector<sequence>& sequences)
{
    utf::serializer sr(*wp.stream, *wp.allocator);

    const auto revisionInfo = get_revision_info_for_version_(
        sequence_revisions_,
        wp.version
    );

    rad::vector<utf::column_info> columns(*wp.allocator);
    revisionInfo->columnGroup.append_to(sequence_columns_, columns);

    sr.start("Sequence", columns, wp.encoding);

    // Write rows.
    for (std::size_t i = 0; i < sequences.size(); sr.next_row(), ++i)
    {
        const auto& sequence = sequences[i];

        // r0
        // TODO: Write Tempo correctly for r0 files. How does Tempo differ from playbackRatio ??
        sr.push_cell_u16(sequence.playbackRatio); // PlaybackRatio
        sr.push_cell_u16(sequence.trackIndices.size()); // NumTracks
        sr.push_cell_buffer(sequence.trackIndices.empty()); // TrackIndex
        sr.push_cell_u16(sequence.commandIndex); // CommandIndex

        // r1
        if (wp.version < sequence_revisions_[1].version) continue;

        sr.push_cell_buffer(sequence.localAisacIndices.empty()); // LocalAisacs
        sr.push_cell_u16(sequence.globalAisacStartIndex); // GlobalAisacStartIndex
        sr.push_cell_u16(sequence.globalAisacCount); // GlobalAisacNumRefs
        sr.push_cell_u16(UINT16_MAX); // ParameterPallet // TODO

        // r2
        if (wp.version < sequence_revisions_[2].version) continue;

        sr.push_cell_u16(UINT16_MAX); // ActionTrackStartIndex // TODO
        sr.push_cell_u16(0); // NumActionTracks // TODO
        sr.push_cell_buffer(sequence.trackValues.empty()); // TrackValues
        sr.push_cell_u8(static_cast<u8>(sequence.type)); // Type
        sr.push_cell_u16(i); // ControlWorkArea1

        if (wp.version >= sequence_revisions_[4].version)
        {
            sr.push_cell_buffer(); // NumPlaybackTrackNoHistories
        }
        else
        {
            sr.push_cell_u16(i); // ControlWorkArea2
        }

        // r3
        if (wp.version < sequence_revisions_[3].version) continue;

        sr.push_cell_u16(UINT16_MAX); // InstPluginTrackStartIndex // TODO
        sr.push_cell_u16(0); // NumInstPluginTracks // TODO

        // r4
        if (wp.version < sequence_revisions_[4].version) continue;

        sr.push_cell_u16(UINT16_MAX); // MIDITrackStartIndex // TODO
        sr.push_cell_u16(0); // NumMIDITracks // TODO
        sr.push_cell_u16(UINT16_MAX); // WatchActionStartIndex // TODO
        sr.push_cell_u16(0); // NumWatchAction // TODO
        sr.push_cell_u16(UINT16_MAX); // StopActionStartIndex // TODO
        sr.push_cell_u16(0); // NumStopAction // TODO
    }

    // Write buffers.
    auto br = sr.begin_buffer_data_section();

    for (const auto& sequence : sequences)
    {
        // TrackIndex
        serialize_u16_array_(br, sequence.trackIndices);

        if (wp.version < sequence_revisions_[1].version) continue;

        // LocalAisacs
        serialize_u16_array_(br, sequence.localAisacIndices);

        if (wp.version < sequence_revisions_[2].version) continue;

        // TrackValues
        serialize_u16_array_(br, sequence.trackValues);

        if (wp.version >= sequence_revisions_[4].version)
        {
            // NumPlaybackTrackNoHistories
            // TODO
            br.next();
        }
    }

    // Finish writing table.
    sr.finish();
}
}
