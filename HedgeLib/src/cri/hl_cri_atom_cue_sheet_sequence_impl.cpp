#include "hedgelib/cri/hl_cri_atom_cue_sheet.h"

namespace hl::cri_new::atom
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

static constexpr revision_info sequence_revisions_[] =
{
    { packed_version(0), sequence_columns_r0_ }, // r0
    { packed_version(1, 12, 00), sequence_columns_r1_ }, // r1
    { packed_version(1, 20, 03), sequence_columns_r2_ }, // r2
    { packed_version(1, 37, 00), sequence_columns_r3_ }, // r3
    { packed_version(1, 42, 01), sequence_columns_r4_ }, // r4
};

static constexpr std::size_t sequence_max_column_count_ = sequence_revisions_[2].get_total_count();

u16 sequence::compute_related_waveform_count(const cue_sheet& cueSheet) const
{
    u16 relatedWaveformCount = 0;

    // TODO: Do we need to account for sequence command table??

    for (const auto trackIndex : trackIndices)
    {
        const auto& track = cueSheet.tracks.at(trackIndex);
        relatedWaveformCount += track.compute_related_waveform_count(cueSheet);
    }

    return relatedWaveformCount;
}

void cue_sheet::write_sequence_table_(detail_::write_params& wp) const
{
    utf::table_serializer ts(*wp.stream, *wp.allocator);

    const auto revisionInfo = get_revision_info(
        sequence_revisions_,
        wp.version
    );

    rad::vector<utf::column_info> columns(*wp.allocator);
    columns.reserve(sequence_max_column_count_);

    revisionInfo->columns.append_to(sequence_columns_, columns);

    ts.start("Sequence", columns, wp.encoding);

    // Write rows.
    u16 globalAisacStartIndex = wp.globalAisacStartIndices.sequence;

    for (std::size_t i = 0; i < sequences.size(); ts.next_row(), ++i)
    {
        const auto& sequence = sequences[i];

        // r0
        // TODO: How does Tempo differ from playbackRatio ??
        ts.write_cell_as_u16(sequence.playbackRatio); // PlaybackRatio
        ts.write_cell_as_u16(sequence.trackIndices.size()); // NumTracks
        ts.write_cell_as_buffer(); // TrackIndex
        ts.write_cell_as_u16(sequence.commandIndex); // CommandIndex

        // r1 columns
        if (wp.version < sequence_revisions_[1].version) continue;

        ts.write_cell_as_buffer(); // LocalAisacs

        ts.write_cell_as_u16((sequence.globalAisacs.empty()) ?
            UINT16_MAX : globalAisacStartIndex
        ); // GlobalAisacStartIndex

        // TODO: Validate size.
        const auto globalAisacNumRefs = static_cast<u16>(sequence.globalAisacs.size());
        ts.write_cell_as_u16(globalAisacNumRefs); // GlobalAisacNumRefs

        globalAisacStartIndex += globalAisacNumRefs;

        ts.write_cell_as_u16(UINT16_MAX); // ParameterPallet

        // r2 columns
        if (wp.version < sequence_revisions_[2].version) continue;

        ts.write_cell_as_u16(UINT16_MAX); // ActionTrackStartIndex
        ts.write_cell_as_u16(0); // NumActionTracks
        ts.write_cell_as_buffer(); // TrackValues
        ts.write_cell_as_u8(static_cast<u8>(sequence.type)); // Type
        ts.write_cell_as_u16(i); // ControlWorkArea1
        ts.write_cell_as_u16(i); // ControlWorkArea2
    }

    // Finish writing rows.
    ts.finish_rows();

    // Write buffers.
    for (std::size_t cellIndex = 0, i = 0;
        i < sequences.size();
        cellIndex += ts.column_count(), ++i)
    {
        const auto& sequence = sequences[i];

        // Write TrackIndex.
        if (!sequence.trackIndices.empty())
        {
            const auto trackIndicesDataPos = wp.stream->tell();
            for (const auto trackIndex : sequence.trackIndices)
            {
                ts.writer().write_u16(trackIndex);
            }

            ts.fill_buffer_cell(cellIndex + 2, trackIndicesDataPos);
        }

        if (wp.version < sequence_revisions_[1].version) continue;

        // Write LocalAisacs.
        if (!sequence.localAisacIndices.empty())
        {
            const auto localAisacsDataPos = wp.stream->tell();
            for (const auto localAisacIndex : sequence.localAisacIndices)
            {
                ts.writer().write_u16(localAisacIndex);
            }

            ts.fill_buffer_cell(cellIndex + 4, localAisacsDataPos);
        }

        if (wp.version < sequence_revisions_[2].version) continue;

        // Write TrackValues.
        if (!sequence.trackValues.empty())
        {
            const auto trackValuesDataPos = wp.stream->tell();
            for (const auto trackValue : sequence.trackValues)
            {
                ts.writer().write_u16(trackValue);
            }

            ts.fill_buffer_cell(cellIndex + 10, trackValuesDataPos);
        }
    }

    // Finish writing table.
    ts.finish();
    wp.stream->pad(32);
}
}
