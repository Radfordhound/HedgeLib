#include "hedgelib/cri/hl_cri_atom_cue_sheet.h"

namespace hl::cri_new::atom
{
static const utf::column_info waveform_columns_[] =
{
    // r0 columns

    // group: 0, index: 0, count: 1
    { utf::cell_type::u16, "Id" },

    // group: 1, index: 1, count: 4
    { utf::cell_type::u8, "EncodeType" },
    { utf::cell_type::u8, "Streaming" },
    { utf::cell_type::u8, "NumChannels" },
    { utf::cell_type::u8, "LoopFlag" },

    // group: 2, index: 5, count: 1
    { utf::cell_type::u16, "SamplingRate" }, // type == u32 in >= v1.42.01

    // group: 3, index: 6, count: 1
    { utf::cell_type::u32, "NumSamples" },

    // group: 4, index: 7, count: 1
    { utf::cell_type::buffer, "ExtensionData" }, // type == u16 in >= v1.26.00

    // r1 columns

    // group: 5, index: 8, count: 1
    { utf::cell_type::u16, "ExtensionData" },

    // group: 6, index: 9, count: 1
    { utf::cell_type::u16, "StreamAwbPortNo" },

    // r2 columns

    // group: 7, index: 10, count: 1
    { utf::cell_type::u16, "MemoryAwbId" }, // Id

    // group: 8, index: 11, count: 1
    { utf::cell_type::u16, "StreamAwbId" },

    // r3 columns

    // group: 9, index: 12, count: 1
    { utf::cell_type::u16, "LipMorthIndex" },

    // r4 columns

    // group: 10, index: 13, count: 1
    { utf::cell_type::u32, "SamplingRate" },

    // group: 11, index: 14, count: 2
    { utf::cell_type::u32, "ChConfig" },
    { utf::cell_type::u8, "HrtfType" },
};

static constexpr utf::column_info_range waveform_columns_r0_[] =
{
    { 0, 8 }, // Id - ExtensionData
};

static constexpr utf::column_info_range waveform_columns_r1_[] =
{
    { 0, 7 },   // Id - NumSamples
    { 8, 2 },   // ExtensionData - StreamAwbPortNo
};

static constexpr utf::column_info_range waveform_columns_r2_[] =
{
    { 10, 1 },  // MemoryAwbId
    { 1, 6 },   // EncodeType - NumSamples
    { 8, 2 },   // ExtensionData - StreamAwbPortNo
    { 11, 1 },  // StreamAwbId
};

static constexpr utf::column_info_range waveform_columns_r3_[] =
{
    { 10, 1 },  // MemoryAwbId
    { 1, 6 },   // EncodeType - NumSamples
    { 8, 2 },   // ExtensionData - StreamAwbPortNo
    { 11, 2 },  // StreamAwbId - LipMorthIndex
};

static constexpr utf::column_info_range waveform_columns_r4_[] =
{
    { 10, 1 },  // MemoryAwbId
    { 1, 4 },   // EncodeType - LoopFlag
    { 13, 1 },  // SamplingRate
    { 6, 1 },   // NumSamples
    { 8, 2 },   // ExtensionData - StreamAwbPortNo
    { 11, 2 },  // StreamAwbId - LipMorthIndex
    { 14, 2 },  // ChConfig - HrtfType
};

static constexpr revision_info waveform_revisions_[] =
{
    { packed_version(0), waveform_columns_r0_ }, // r0
    { packed_version(1, 26, 00), waveform_columns_r1_ }, // r1
    { packed_version(1, 27, 02), waveform_columns_r2_ }, // r2
    { packed_version(1, 37, 00), waveform_columns_r3_ }, // r3
    { packed_version(1, 42, 01), waveform_columns_r4_ }, // r4
};

static constexpr std::size_t waveform_max_column_count_ = waveform_revisions_[4].get_total_count();

void cue_sheet::write_waveform_table_(detail_::write_params& wp) const
{
    utf::table_serializer ts(*wp.stream, *wp.allocator);

    const auto revisionInfo = get_revision_info(
        waveform_revisions_,
        wp.version
    );

    rad::vector<utf::column_info> columns(*wp.allocator);
    columns.reserve(waveform_max_column_count_);

    revisionInfo->columns.append_to(waveform_columns_, columns);

    ts.start("Waveform", columns, wp.encoding);

    // Write rows.
    for (std::size_t i = 0; i < waveforms.size(); ts.next_row(), ++i)
    {
        const auto& waveform = waveforms[i];

        // r0 columns
        if (wp.version >= waveform_revisions_[2].version)
        {
            ts.write_cell_as_u16((waveform.isStreaming) ? // MemoryAwbId
                UINT16_MAX : waveform.awbId
            );
        }
        else
        {
            ts.write_cell_as_u16(waveform.awbId); // Id
        }

        ts.write_cell_as_u8(static_cast<u8>(waveform.encodeType)); // EncodeType
        ts.write_cell_as_u8(waveform.isStreaming); // Streaming
        ts.write_cell_as_u8(waveform.channelCount); // NumChannels
        ts.write_cell_as_u8(waveform.loopFlags); // LoopFlag

        if (wp.version >= waveform_revisions_[4].version)
        {
            ts.write_cell_as_u32(waveform.sampleRate); // SamplingRate
        }
        else
        {
            // TODO: Error if sampleRate is > UINT16_MAX
            ts.write_cell_as_u16(static_cast<u16>(waveform.sampleRate)); // SamplingRate
        }

        ts.write_cell_as_u32(waveform.sampleCount); // NumSamples

        if (wp.version >= waveform_revisions_[1].version)
        {
            ts.write_cell_as_u16(UINT16_MAX); // ExtensionData
        }
        else
        {
            ts.write_cell_as_buffer(); // ExtensionData
            continue;
        }

        // r1 columns
        ts.write_cell_as_u16((waveform.isStreaming) ? 0 : UINT16_MAX); // StreamAwbPortNo

        // r2 columns
        if (wp.version < waveform_revisions_[2].version) continue;

        ts.write_cell_as_u16((waveform.isStreaming) ? // StreamAwbId
            waveform.awbId : UINT16_MAX
        );

        // r3 columns
        if (wp.version < waveform_revisions_[3].version) continue;

        ts.write_cell_as_u16(UINT16_MAX); // LipMorthIndex

        // r4 columns
        if (wp.version < waveform_revisions_[4].version) continue;

        ts.write_cell_as_u32(0); // ChConfig
        ts.write_cell_as_u8(0); // HrtfType
    }

    // Finish writing rows.
    ts.finish_rows();

    // Write buffers.
    if (wp.version < waveform_revisions_[1].version)
    {
        for (std::size_t cellIndex = 0, i = 0;
            i < waveforms.size();
            cellIndex += ts.column_count(), ++i)
        {
            // TODO: Write ExtensionData; fill-in using cellIndex + 7
        }
    }

    // Finish writing table.
    ts.finish();
    wp.stream->pad(32);
}
}
