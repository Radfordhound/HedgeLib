#include "hedgelib/cri/hl_cri_atom_cue_sheet.h"

namespace hl::cri_new::atom
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

static constexpr revision_info synth_revisions_[] =
{
    { packed_version(0), synth_columns_r0_ }, // r0
    { packed_version(1, 12, 00), synth_columns_r1_ }, // r1
    { packed_version(1, 20, 03), synth_columns_r2_ }, // r2
};

u16 synth::compute_related_waveform_count(const cue_sheet& cueSheet) const
{
    u16 relatedWaveformCount = 0;

    // TODO: Do we need to account for synth command table??

    for (const auto& refItem : refItems)
    {
        relatedWaveformCount += refItem.compute_related_waveform_count(cueSheet);
    }
    
    return relatedWaveformCount;
}

void cue_sheet::write_synth_table_(detail_::write_params& wp) const
{
    utf::table_serializer ts(*wp.stream, *wp.allocator);

    const auto revisionInfo = get_revision_info(
        synth_revisions_,
        wp.version
    );

    ts.start(
        "Synth",
        { synth_columns_, revisionInfo->columns.get_total_count() },
        wp.encoding
    );

    // Write rows.
    u16 globalAisacStartIndex = 0;

    for (std::size_t i = 0; i < synths.size(); ts.next_row(), ++i)
    {
        const auto& synth = synths[i];

        // r0 columns
        ts.write_cell_as_u8(static_cast<u8>(synth.type)); // Type
        ts.write_cell_as_string(synth.voiceLimitGroupName); // VoiceLimitGroupName

        ts.write_cell_as_u16(
            wp.get_command_index(synth.commandIndex)
        ); // CommandIndex

        ts.write_cell_as_buffer(); // ReferenceItems
        ts.write_cell_as_buffer(); // LocalAisacs

        ts.write_cell_as_u16((synth.globalAisacs.empty()) ?
            UINT16_MAX : globalAisacStartIndex
        ); // GlobalAisacStartIndex

        // TODO: Validate size.
        const auto globalAisacNumRefs = static_cast<u16>(synth.globalAisacs.size());
        ts.write_cell_as_u16(globalAisacNumRefs); // GlobalAisacNumRefs

        globalAisacStartIndex += globalAisacNumRefs;

        if (synth.type == synth_type::random ||
            synth.type == synth_type::random_no_repeat)
        {
            ts.write_cell_as_u16(UINT16_MAX); // ControlWorkArea1
            ts.write_cell_as_u16(UINT16_MAX); // ControlWorkArea2
        }
        else
        {
            ts.write_cell_as_u16(static_cast<u16>(i)); // ControlWorkArea1
            ts.write_cell_as_u16(static_cast<u16>(i)); // ControlWorkArea2
        }

        // r1 columns
        if (wp.version <= synth_revisions_[1].version) continue;

        ts.write_cell_as_buffer(); // TrackValues
        ts.write_cell_as_u16(UINT16_MAX); // ParameterPallet

        // r2 columns
        if (wp.version <= synth_revisions_[2].version) continue;

        ts.write_cell_as_u16(UINT16_MAX); // ActionTrackStartIndex
        ts.write_cell_as_u16(0); // NumActionTracks
    }

    // Finish writing rows.
    ts.finish_rows();

    // Write buffers.
    for (std::size_t cellIndex = 0, i = 0;
        i < synths.size();
        cellIndex += ts.column_count(), ++i)
    {
        const auto& synth = synths[i];

        // Write ReferenceItems.
        if (!synth.refItems.empty())
        {
            const auto refItemsDataPos = wp.stream->tell();
            for (const auto& refItem : synth.refItems)
            {
                ts.writer().write_u16(static_cast<u16>(refItem.type));
                ts.writer().write_u16(refItem.index);
            }

            ts.fill_buffer_cell(cellIndex + 3, refItemsDataPos);
        }

        // Write LocalAisacs.
        if (!synth.localAisacIndices.empty())
        {
            const auto localAisacsDataPos = wp.stream->tell();
            for (const auto localAisacIndex : synth.localAisacIndices)
            {
                ts.writer().write_u16(localAisacIndex);
            }

            ts.fill_buffer_cell(cellIndex + 4, localAisacsDataPos);
        }

        // TODO: Other buffers.

        if (wp.version <= synth_revisions_[1].version) continue;

        // Write TrackValues.
        if (!synth.trackValues.empty())
        {
            const auto trackValuesDataPos = wp.stream->tell();
            for (const auto trackValue : synth.trackValues)
            {
                ts.writer().write_u16(trackValue);
            }

            ts.fill_buffer_cell(cellIndex + 9, trackValuesDataPos);
        }
    }

    // Finish writing table.
    //writer.stream().pad(16); // TODO: Is this correct?
    ts.finish();
    wp.stream->pad(32);
}
}
