#include "hedgelib/cri/hl_cri_atom_cue_sheet.h"
#include "hl_cri_atom_impl.h"

namespace hl::cri::atom
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

static constexpr revision_info_ waveform_revisions_[] =
{
    { packed_version(0), waveform_columns_r0_ }, // r0
    { packed_version(1, 26, 00), waveform_columns_r1_ }, // r1
    { packed_version(1, 27, 02), waveform_columns_r2_ }, // r2
    { packed_version(1, 37, 00), waveform_columns_r3_ }, // r3
    { packed_version(1, 42, 01), waveform_columns_r4_ }, // r4
};

static waveform_encode_type parse_waveform_encode_type_(u8 type)
{
    if (type != static_cast<u8>(waveform_encode_type::adx) &&
        type != static_cast<u8>(waveform_encode_type::hca) &&
        type != static_cast<u8>(waveform_encode_type::hca_mx) &&
        type != static_cast<u8>(waveform_encode_type::cwav) &&
        type != static_cast<u8>(waveform_encode_type::atrac9))
    {
        throw std::runtime_error("Unsupported CriAtom waveform encode type");
    }

    return static_cast<waveform_encode_type>(type);
}

static waveform_stream_type parse_waveform_stream_type_(u8 type)
{
    if (type != static_cast<u8>(waveform_stream_type::memory) &&
        type != static_cast<u8>(waveform_stream_type::stream) &&
        type != static_cast<u8>(waveform_stream_type::stream_no_latency))
    {
        throw std::runtime_error("Unsupported CriAtom waveform stream type");
    }

    return static_cast<waveform_stream_type>(type);
}

static waveform_loop_type parse_waveform_loop_type_(u8 type)
{
    if (type != static_cast<u8>(waveform_loop_type::one_shot) &&
        type != static_cast<u8>(waveform_loop_type::loop))
    {
        throw std::runtime_error("Unsupported CriAtom waveform loop type");
    }

    return static_cast<waveform_loop_type>(type);
}

void read_waveform_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    packed_version version,
    rad::vector<waveform>& waveforms)
{
    assert(waveforms.empty() &&
        "The given output vector must be empty"
    );

    // Read raw table header.
    utf::deserializer dr(stream, utf::deserialize_type::utf, tmpAllocator);

    // Get revision info.
    const auto revisionInfo = get_revision_info_for_version_(
        waveform_revisions_,
        version
    );

    // Validate columns.
    if (!dr.has_columns_of_exact_types(revisionInfo->columnGroup, waveform_columns_))
    {
        throw std::runtime_error("Invalid or unsupported ACB Waveform layout");
    }

    // Read rows.
    waveforms.reserve(dr.row_count());

    for (u32 i = 0; i < dr.row_count(); dr.next_row(), ++i)
    {
        // r0 
        u16 memoryAwbId = dr.next_cell_as_u16(); // MemoryAwbId
        const auto encodeType = parse_waveform_encode_type_(dr.next_cell_as_u8()); // EncodeType
        const auto streamType = parse_waveform_stream_type_(dr.next_cell_as_u8()); // Streaming
        const auto channelCount = dr.next_cell_as_u8(); // NumChannels
        const auto loopType = parse_waveform_loop_type_(dr.next_cell_as_u8()); // LoopFlag
        const auto sampleRate = dr.next_cell_as_u32(); // SamplingRate
        const auto sampleCount = dr.next_cell_as_u32(); // NumSamples

        if (version >= waveform_revisions_[1].version)
        {
            const auto extDataIndex = dr.next_cell_as_u16(); // ExtensionData
            // TODO
        }
        else
        {
            const auto extDataBuf = dr.next_cell_as_buffer(); // ExtensionData
            // TODO
        }

        // r1
        const u16 streamAwbPortNo = (version >= waveform_revisions_[1].version) ?
            dr.next_cell_as_u16() : UINT16_MAX; // StreamAwbPortNo

        // r2
        u16 streamAwbId;
        if (version >= waveform_revisions_[2].version)
        {
            streamAwbId = dr.next_cell_as_u16(); // StreamAwbId
        }
        else if (streamType == waveform_stream_type::memory)
        {
            streamAwbId = UINT16_MAX;
        }
        else
        {
            streamAwbId = memoryAwbId;
            memoryAwbId = UINT16_MAX;
        }

        // r3
        const u16 lipMorthIndex = (version >= waveform_revisions_[3].version) ?
            dr.next_cell_as_u16() : UINT16_MAX; // LipMorthIndex

        // r4
        const u32 chConfig = (version >= waveform_revisions_[4].version) ?
            dr.next_cell_as_u32() : 0; // ChConfig

        const u8 hrtfType = (version >= waveform_revisions_[4].version) ?
            dr.next_cell_as_u32() : 0; // HrtfType

        // Generate waveform.
        waveforms.emplace_back_unchecked(
            memoryAwbId,
            streamAwbId,
            encodeType,
            streamType,
            loopType,
            channelCount,
            sampleRate,
            sampleCount,
            // TODO: ExtensionData
            streamAwbPortNo
            // TODO: LipMorthIndex
            // TODO: ChConfig
            // TODO: HrtfType
        );
    }
}

void write_waveform_table_(
    detail_::write_params& wp,
    const rad::vector<waveform>& waveforms)
{
    utf::serializer sr(*wp.stream, *wp.allocator);

    const auto revisionInfo = get_revision_info_for_version_(
        waveform_revisions_,
        wp.version
    );

    rad::vector<utf::column_info> columns(*wp.allocator);
    revisionInfo->columnGroup.append_to(waveform_columns_, columns);

    sr.start("Waveform", columns, wp.encoding);

    // Write rows.
    for (std::size_t i = 0; i < waveforms.size(); sr.next_row(), ++i)
    {
        const auto& waveform = waveforms[i];

        // r0
        if (wp.version >= waveform_revisions_[2].version)
        {
            sr.push_cell_u16(waveform.memoryAwbId); // MemoryAwbId
        }
        else
        {
            if (waveform.streamType == waveform_stream_type::stream_no_latency)
            {
                // TODO: Log warning about stream_no_latency being unsupported
                // by this revision, and mention that the value specified by 
                // memoryAwbId will be ignored.
            }

            sr.push_cell_u16(
                (waveform.streamType == waveform_stream_type::memory) ?
                waveform.memoryAwbId : waveform.streamAwbId
            ); // Id
        }

        sr.push_cell_u8(static_cast<u8>(waveform.encodeType)); // EncodeType
        sr.push_cell_u8(static_cast<u8>(waveform.streamType)); // Streaming
        sr.push_cell_u8(waveform.channelCount); // NumChannels
        sr.push_cell_u8(static_cast<u8>(waveform.loopType)); // LoopFlag

        if (wp.version >= waveform_revisions_[4].version)
        {
            sr.push_cell_u32(waveform.sampleRate); // SamplingRate
        }
        else
        {
            if (waveform.sampleRate > UINT16_MAX) // TODO: Mark unlikely
            {
                throw std::overflow_error(
                    "ACB waveform sample rate exceeds u16 range "
                    "allowed by this revision."
                );
            }

            sr.push_cell_u16(static_cast<u16>(waveform.sampleRate)); // SamplingRate
        }

        sr.push_cell_u32(waveform.sampleCount); // NumSamples

        if (wp.version >= waveform_revisions_[1].version)
        {
            // TODO
            sr.push_cell_u16(UINT16_MAX); // ExtensionData
        }
        else
        {
            sr.push_cell_buffer(); // ExtensionData
            continue;
        }

        // r1
        sr.push_cell_u16(waveform.streamAwbPort); // StreamAwbPortNo

        // r2
        if (wp.version < waveform_revisions_[2].version) continue;

        sr.push_cell_u16(waveform.streamAwbId); // StreamAwbId

        // r3
        if (wp.version < waveform_revisions_[3].version) continue;

        // TODO
        sr.push_cell_u16(UINT16_MAX); // LipMorthIndex

        // r4
        if (wp.version < waveform_revisions_[4].version) continue;

        // TODO
        sr.push_cell_u32(0); // ChConfig
        sr.push_cell_u8(0); // HrtfType
    }

    // Write buffers.
    auto br = sr.begin_buffer_data_section();

    if (wp.version < waveform_revisions_[1].version)
    {
        for (const auto& waveform : waveforms)
        {
            // ExtensionData
            // TODO: Write ExtensionData
            br.next();
        }
    }

    // Finish writing table.
    sr.writer().stream().pad(4);
    sr.finish();
}
}
