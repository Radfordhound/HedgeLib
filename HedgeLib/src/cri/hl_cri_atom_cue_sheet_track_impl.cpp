#include "hedgelib/cri/hl_cri_atom_cue_sheet.h"

namespace hl::cri_new::atom
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

static const revision_info track_revisions_[] =
{
    { packed_version(0), track_columns_r0_ }, // r0
    { packed_version(1, 12, 00), track_columns_r1_ }, // r1
    { packed_version(1, 20, 03), track_columns_r2_ }, // r2
    { packed_version(1, 42, 01), track_columns_r3_ }, // r3
    { packed_version(1, 44, 01), track_columns_r4_ }, // r4
};

u16 track::compute_related_waveform_count(const cue_sheet& cueSheet) const
{
    u16 relatedWaveformCount = 0;

    // TOOD: Do we also need to do this for the track command table??

    if (eventIndex != UINT16_MAX)
    {
        const auto& trackEventCmdTable = cueSheet.trackEventCmdTables.at(eventIndex);
        relatedWaveformCount += trackEventCmdTable.compute_related_waveform_count(cueSheet);
    }
    
    return relatedWaveformCount;
}

void cue_sheet::write_track_table_(detail_::write_params& wp) const
{
    utf::table_serializer ts(*wp.stream, *wp.allocator);

    const auto revisionInfo = get_revision_info(
        track_revisions_,
        wp.version
    );

    ts.start(
        "Track",
        { track_columns_, revisionInfo->columns.get_total_count() },
        wp.encoding
    );

    // Write rows.
    u16 globalAisacStartIndex = wp.globalAisacStartIndices.track;

    for (std::size_t i = 0; i < tracks.size(); ts.next_row(), ++i)
    {
        const auto& track = tracks[i];

        // r0 columns
        ts.write_cell_as_u16(track.eventIndex); // EventIndex
        ts.write_cell_as_u16(track.commandIndex); // CommandIndex
        ts.write_cell_as_buffer(); // LocalAisacs

        ts.write_cell_as_u16((track.globalAisacs.empty()) ?
            UINT16_MAX : globalAisacStartIndex
        ); // GlobalAisacStartIndex

        // TODO: Validate size.
        const auto globalAisacNumRefs = static_cast<u16>(track.globalAisacs.size());
        ts.write_cell_as_u16(globalAisacNumRefs); // GlobalAisacNumRefs

        globalAisacStartIndex += globalAisacNumRefs;

        // r1 columns
        if (wp.version < track_revisions_[1].version) continue;

        ts.write_cell_as_u16(UINT16_MAX); // ParameterPallet

        // r2 columns
        if (wp.version < track_revisions_[2].version) continue;

        ts.write_cell_as_u8(0); // TargetType
        ts.write_cell_as_string(""); // TargetName
        ts.write_cell_as_u32(UINT32_MAX); // TargetId
        ts.write_cell_as_string(""); // TargetAcbName
        ts.write_cell_as_u8(0); // Scope
        ts.write_cell_as_u16(UINT16_MAX); // TargetTrackNo

        // r3 columns
        if (wp.version < track_revisions_[3].version) continue;

        ts.write_cell_as_u16(UINT16_MAX); // BeatSyncLabel

        // r4 columns
        if (wp.version < track_revisions_[4].version) continue;

        ts.write_cell_as_u8(0); // TargetIdentificationCondition
    }

    // Finish writing rows.
    ts.finish_rows();

    // Write buffers.
    for (std::size_t cellIndex = 0, i = 0;
        i < tracks.size();
        cellIndex += ts.column_count(), ++i)
    {
        const auto& track = tracks[i];

        // Write LocalAisacs.
        if (!track.localAisacIndices.empty())
        {
            const auto localAisacsDataPos = wp.stream->tell();
            for (const auto localAisacIndex : track.localAisacIndices)
            {
                ts.writer().write_u16(localAisacIndex);
            }

            ts.fill_buffer_cell(cellIndex + 2, localAisacsDataPos);
        }
    }

    // Finish writing table.
    ts.finish();
    wp.stream->pad(32);
}
}
