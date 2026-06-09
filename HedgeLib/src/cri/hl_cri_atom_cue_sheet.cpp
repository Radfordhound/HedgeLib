#include <rad/rad_memory_stream.h>

#include "hedgelib/cri/hl_cri_atom_cue_sheet.h"
#include "hl_cri_atom_impl.h"

namespace hl::cri::atom
{
// TODO: Move to atom.cpp
static const md5_hash empty_md5_hash_ = { 0 };

static const utf::column_info cue_sheet_columns_[] =
{
    // reserved columns

    // group: 0, index: 0, count: 9
    { utf::cell_type::u8, "R35" },
    { utf::cell_type::u8, "R34" },
    { utf::cell_type::u8, "R33" },
    { utf::cell_type::u8, "R32" },
    { utf::cell_type::u8, "R31" },
    { utf::cell_type::u8, "R30" },
    { utf::cell_type::u8, "R29" },
    { utf::cell_type::u8, "R28" },
    { utf::cell_type::u8, "R27" },

    // group: 1, index: 9, count: 27
    { utf::cell_type::u8, "R26" },
    { utf::cell_type::u8, "R25" },
    { utf::cell_type::u8, "R24" },
    { utf::cell_type::u8, "R23" },
    { utf::cell_type::u8, "R22" },
    { utf::cell_type::u8, "R21" },
    { utf::cell_type::u8, "R20" },
    { utf::cell_type::u8, "R19" },
    { utf::cell_type::u8, "R18" },
    { utf::cell_type::u8, "R17" },
    { utf::cell_type::u8, "R16" },
    { utf::cell_type::u8, "R15" },
    { utf::cell_type::u8, "R14" },
    { utf::cell_type::u8, "R13" },
    { utf::cell_type::u8, "R12" },
    { utf::cell_type::u8, "R11" },
    { utf::cell_type::u8, "R10" },
    { utf::cell_type::u8, "R9" },
    { utf::cell_type::u8, "R8" },
    { utf::cell_type::u8, "R7" },
    { utf::cell_type::u8, "R6" },
    { utf::cell_type::u8, "R5" },
    { utf::cell_type::u8, "R4" },
    { utf::cell_type::u8, "R3" },
    { utf::cell_type::u8, "R2" },
    { utf::cell_type::u8, "R1" },
    { utf::cell_type::u8, "R0" },

    // r0 columns

    // group: 2, index: 36, count: 15
    { utf::cell_type::u32, "FileIdentifier" },
    { utf::cell_type::u32, "Size" },
    { utf::cell_type::u32, "Version" },
    { utf::cell_type::u8, "Type" },
    { utf::cell_type::u8, "Target" },
    { utf::cell_type::buffer, "AcfMd5Hash" },
    { utf::cell_type::u8, "CategoryExtension" },
    { utf::cell_type::buffer, "CueTable" },
    { utf::cell_type::buffer, "CueNameTable" },
    { utf::cell_type::buffer, "WaveformTable" },
    { utf::cell_type::buffer, "AisacTable" },
    { utf::cell_type::buffer, "GraphTable" },
    { utf::cell_type::buffer, "GlobalAisacReferenceTable" },
    { utf::cell_type::buffer, "AisacNameTable" },
    { utf::cell_type::buffer, "SynthTable" },

    // group: 3, index: 51, count: 1
    { utf::cell_type::buffer, "CommandTable" }, // "SeqCommandTable" in >= 1.30.00

    // group: 4, index: 52, count: 18
    { utf::cell_type::buffer, "TrackTable" },
    { utf::cell_type::buffer, "SequenceTable" },
    { utf::cell_type::buffer, "AisacControlNameTable" },
    { utf::cell_type::buffer, "AutoModulationTable" },
    { utf::cell_type::buffer, "StreamAwbTocWorkOld" },
    { utf::cell_type::buffer, "AwbFile" },
    { utf::cell_type::string, "VersionString" },
    { utf::cell_type::buffer, "CueLimitWorkTable" },
    { utf::cell_type::u16, "NumCueLimitListWorks" },
    { utf::cell_type::u16, "NumCueLimitNodeWorks" },
    { utf::cell_type::buffer, "AcbGuid" },
    { utf::cell_type::buffer, "StreamAwbHash" },
    { utf::cell_type::buffer, "StreamAwbTocWork_Old" },
    { utf::cell_type::f32, "AcbVolume" },
    { utf::cell_type::buffer, "StringValueTable" },
    { utf::cell_type::buffer, "OutsideLinkTable" },
    { utf::cell_type::buffer, "BlockSequenceTable" },
    { utf::cell_type::buffer, "BlockTable" },

    // group: 5, index: 70, count: 3
    { utf::cell_type::buffer, "PaddingArea" },
    { utf::cell_type::buffer, "StreamAwbTocWork" },
    { utf::cell_type::buffer, "StreamAwbAfs2Header" },

    // r1 columns

    // group: 6, index: 73, count: 1
    { utf::cell_type::string, "Name" }, // R26

    // r2 columns

    // group: 7, index: 74, count: 4
    { utf::cell_type::u8, "CharacterEncodingType" }, // R25
    { utf::cell_type::buffer, "EventTable" }, // R24
    { utf::cell_type::buffer, "ActionTrackTable" }, // R23
    { utf::cell_type::buffer, "AcfReferenceTable" }, // R22

    // r3 columns

    // group: 8, index: 78, count: 4
    { utf::cell_type::buffer, "WaveformExtensionDataTable" }, // R21
    { utf::cell_type::buffer, "BeatSyncInfoTable" }, // R20
    { utf::cell_type::u8, "CuePriorityType" }, // R19
    { utf::cell_type::u16, "NumCueLimit" }, // R18

    // r4 columns

    // group: 9, index: 82, count: 1
    { utf::cell_type::buffer, "SeqCommandTable" }, // CommandTable

    // group: 10, index: 83, count: 7
    { utf::cell_type::buffer, "TrackCommandTable" }, // R17
    { utf::cell_type::buffer, "SynthCommandTable" }, // R16
    { utf::cell_type::buffer, "TrackEventTable" }, // R15
    { utf::cell_type::buffer, "SeqParameterPalletTable" }, // R14
    { utf::cell_type::buffer, "TrackParameterPalletTable" }, // R13
    { utf::cell_type::buffer, "SynthParameterPalletTable" }, // R12
    { utf::cell_type::buffer, "SoundGeneratorTable" }, // R11

    // r5 columns

    // group 11, index: 90, count: 1
    { utf::cell_type::buffer, "ProjectKey" }, // R7

    // r6 columns

    // group: 12, index: 91, count: 3
    { utf::cell_type::buffer, "InstrumentPluginTrackTable" }, // R10
    { utf::cell_type::buffer, "InstrumentPluginParameterTable" }, // R9
    { utf::cell_type::buffer, "LipsMorphTable" }, // R8

    // group: 13, index: 94, count: 3
    { utf::cell_type::buffer, "SoundInstruments" }, // R6
    { utf::cell_type::buffer, "SoundProgramBankKey" }, // R5
    { utf::cell_type::buffer, "MIDITrackTable" }, // R4

    // r7 columns

    // group: 14, index: 97, count: 4
    { utf::cell_type::buffer, "SoundProgramBankCommandTable" }, // R35/R3
    { utf::cell_type::buffer, "ParameterAction" }, // R34/R2
    { utf::cell_type::buffer, "ParameterActionCondition" }, // R33/R1
    { utf::cell_type::buffer, "StopAction" }, // R32/R0
};

static constexpr utf::column_info_range cue_sheet_columns_r0_[] =
{
    { 36, 34 }, // FileIdentifier - BlockTable
    { 9, 27 },  // R26 - R0
    { 70, 3 },  // PaddingArea - StreamAwbAfs2Header
};

static constexpr utf::column_info_range cue_sheet_columns_r1_[] =
{
    { 36, 34 }, // FileIdentifier - BlockTable
    { 73, 1 },  // Name
    { 10, 26 }, // R25 - R0
    { 70, 3 },  // PaddingArea - StreamAwbAfs2Header
};

static constexpr utf::column_info_range cue_sheet_columns_r2_[] =
{
    { 36, 34 }, // FileIdentifier - BlockTable
    { 73, 1 },  // Name
    { 74, 4 },  // CharacterEncodingType - AcfReferenceTable
    { 14, 22 }, // R21 - R0
    { 70, 3 },  // PaddingArea - StreamAwbAfs2Header
};

static constexpr utf::column_info_range cue_sheet_columns_r3_[] =
{
    { 36, 34 }, // FileIdentifier - BlockTable
    { 73, 1 },  // Name
    { 74, 4 },  // CharacterEncodingType - AcfReferenceTable
    { 78, 4 },  // WaveformExtensionDataTable - NumCueLimit
    { 18, 18 }, // R17 - R0
    { 70, 3 },  // PaddingArea - StreamAwbAfs2Header
};

static constexpr utf::column_info_range cue_sheet_columns_r4_[] =
{
    { 36, 15 }, // FileIdentifier - SynthTable
    { 82, 1 },  // SeqCommandTable
    { 52, 18 }, // TrackTable - BlockTable
    { 73, 1 },  // Name
    { 74, 4 },  // CharacterEncodingType - AcfReferenceTable
    { 78, 4 },  // WaveformExtensionDataTable - NumCueLimit
    { 83, 7 },  // TrackCommandTable - SoundGeneratorTable
    { 25, 11 }, // R10 - R0
    { 70, 3 },  // PaddingArea - StreamAwbAfs2Header
};

static constexpr utf::column_info_range cue_sheet_columns_r5_[] =
{
    { 36, 15 }, // FileIdentifier - SynthTable
    { 82, 1 },  // SeqCommandTable
    { 52, 18 }, // TrackTable - BlockTable
    { 73, 1 },  // Name
    { 74, 4 },  // CharacterEncodingType - AcfReferenceTable
    { 78, 4 },  // WaveformExtensionDataTable - NumCueLimit
    { 83, 7 },  // TrackCommandTable - SoundGeneratorTable
    { 25, 3 },  // R10 - R8
    { 90, 1 },  // ProjectKey
    { 29, 7 },  // R6 - R0
    { 70, 3 },  // PaddingArea - StreamAwbAfs2Header
};

static constexpr utf::column_info_range cue_sheet_columns_r6_[] =
{
    { 36, 15 }, // FileIdentifier - SynthTable
    { 82, 1 },  // SeqCommandTable
    { 52, 18 }, // TrackTable - BlockTable
    { 73, 1 },  // Name
    { 74, 4 },  // CharacterEncodingType - AcfReferenceTable
    { 78, 4 },  // WaveformExtensionDataTable - NumCueLimit
    { 83, 7 },  // TrackCommandTable - SoundGeneratorTable
    { 91, 3 },  // InstrumentPluginTrackTable - LipsMorphTable
    { 90, 1 },  // ProjectKey
    { 94, 3 },  // SoundInstruments - MIDITrackTable
    { 0, 36 },  // R35 - R0
    { 70, 3 },  // PaddingArea - StreamAwbAfs2Header
};

static constexpr utf::column_info_range cue_sheet_columns_r7_[] =
{
    { 36, 15 }, // FileIdentifier - SynthTable
    { 82, 1 },  // SeqCommandTable
    { 52, 18 }, // TrackTable - BlockTable
    { 73, 1 },  // Name
    { 74, 4 },  // CharacterEncodingType - AcfReferenceTable
    { 78, 4 },  // WaveformExtensionDataTable - NumCueLimit
    { 83, 7 },  // TrackCommandTable - SoundGeneratorTable
    { 91, 3 },  // InstrumentPluginTrackTable - LipsMorphTable
    { 90, 1 },  // ProjectKey
    { 94, 3 },  // SoundInstruments - MIDITrackTable
    { 97, 4 },  // SoundProgramBankCommandTable - StopAction
    { 4, 32 },  // R31 - R0
    { 70, 3 },  // PaddingArea - StreamAwbAfs2Header
};

static constexpr revision_info_ cue_sheet_revisions_[] =
{
    // old format version (v0.00.00 - v1.37.00)
    { packed_version(0), cue_sheet_columns_r0_ }, // r0
    { packed_version(1, 12, 00), cue_sheet_columns_r1_ }, // r1
    { packed_version(1, 20, 03), cue_sheet_columns_r2_ }, // r2
    { packed_version(1, 26, 00), cue_sheet_columns_r3_ }, // r3
    { packed_version(1, 30, 00), cue_sheet_columns_r4_ }, // r4
    { packed_version(1, 32, 01), cue_sheet_columns_r5_ }, // r5

    // new format version (v1.37.00+)
    { packed_version(1, 37, 00), cue_sheet_columns_r6_ }, // r6
    { packed_version(1, 42, 01), cue_sheet_columns_r7_ }, // r7
};

static constexpr u16 cue_sheet_column_count_r0_ = 64;

static constexpr u16 cue_sheet_column_count_r6_ = 96;

static const utf::column_info string_value_columns_[] =
{
    { utf::cell_type::string, "StringValue" },
};

static void read_string_value_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    rad::vector<rad::string>& stringValues)
{
    assert(stringValues.empty() &&
        "The given output vector must be empty"
    );

    // Read raw table header.
    utf::deserializer dr(stream, utf::deserialize_type::utf, tmpAllocator);

    // Validate columns.
    if (!dr.has_columns_of_exact_types(
        string_value_columns_,
        0,
        static_cast<u16>(std::size(string_value_columns_))))
    {
        throw std::runtime_error("Invalid or unsupported ACB string value layout");
    }

    // Read rows.
    auto& stringValueAllocator = stringValues.allocator();
    stringValues.reserve(dr.row_count());

    for (u32 i = 0; i < dr.row_count(); dr.next_row(), ++i)
    {
        const auto val = dr.get_string_data(dr.next_cell_as_string()); // StringValue
        stringValues.emplace_back_unchecked(
            rad::string(stringValueAllocator, val)
        );
    }
}

static void write_string_value_table_(
    detail_::write_params& wp,
    const rad::vector<rad::string>& stringValues)
{
    utf::serializer sr(*wp.stream, *wp.allocator);

    sr.start(
        "Strings",
        string_value_columns_,
        wp.encoding
    );

    // Write rows.
    for (const auto& strVal : stringValues)
    {
        sr.push_cell_string(strVal); // StringValue
        sr.next_row();
    }

    // Finish writing table.
    sr.finish();
}

static void write_global_aisac_reference_table_(
    detail_::write_params& wp,
    const rad::vector<rad::string>& globalAisacNames)
{
    utf::serializer sr(*wp.stream, *wp.allocator);

    sr.start(
        "GlobalAisacReference",
        { { utf::cell_type::string, "Name" } },
        wp.encoding
    );

    // Write rows.
    for (const auto& globalAisacName : globalAisacNames)
    {
        sr.push_cell_string(globalAisacName); // Name
        sr.next_row();
    }

    // Finish writing table.
    sr.finish();
}

static const utf::column_info acf_reference_columns_[] =
{
    { utf::cell_type::u8, "Type" },
    { utf::cell_type::string, "Name" },
    { utf::cell_type::string, "Name2" },
    { utf::cell_type::u32, "Id" },
};

config_ref_item_type parse_config_ref_item_type_(u8 type)
{
    if (type != static_cast<u8>(config_ref_item_type::category) &&
        type != static_cast<u8>(config_ref_item_type::aisac) &&
        type != static_cast<u8>(config_ref_item_type::aisac_control) &&
        type != static_cast<u8>(config_ref_item_type::voice_limit_group) &&
        type != static_cast<u8>(config_ref_item_type::selector_label) &&
        type != static_cast<u8>(config_ref_item_type::dsp_bus))
    {
        throw std::runtime_error("Unsupported CriAtom ACF ref item type");
    }

    return static_cast<config_ref_item_type>(type);
}

static void read_acf_reference_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    rad::vector<config_ref_item>& acfRefItems)
{
    assert(acfRefItems.empty() &&
        "The given output vector must be empty"
    );

    // Read raw table header.
    utf::deserializer dr(stream, utf::deserialize_type::utf, tmpAllocator);

    // Validate columns.
    if (!dr.has_columns_of_exact_types(
        acf_reference_columns_,
        0,
        static_cast<u16>(std::size(acf_reference_columns_))))
    {
        throw std::runtime_error("Invalid or unsupported ACB ACF reference table layout");
    }

    // Read rows.
    auto& acfRefItemAllocator = acfRefItems.allocator();
    acfRefItems.reserve(dr.row_count());

    for (u32 i = 0; i < dr.row_count(); dr.next_row(), ++i)
    {
        const auto type = parse_config_ref_item_type_(dr.next_cell_as_u8()); // Type
        const auto name = dr.get_string_data(dr.next_cell_as_string()); // Name
        const auto name2 = dr.get_string_data(dr.next_cell_as_string()); // Name2
        const auto id = dr.next_cell_as_u32(); // Id

        acfRefItems.emplace_back_unchecked(
            type,
            rad::string(acfRefItemAllocator, name),
            rad::string(acfRefItemAllocator, name2),
            id
        );
    }
}

void write_acf_reference_table_(
    detail_::write_params& wp,
    const rad::vector<config_ref_item>& acfRefItems)
{
    utf::serializer sr(*wp.stream, *wp.allocator);

    sr.start("AcfReference", acf_reference_columns_, wp.encoding);

    // Write rows.
    for (const auto& acfRefItem : acfRefItems)
    {
        sr.push_cell_u8(static_cast<u8>(acfRefItem.type)); // Type
        sr.push_cell_string(acfRefItem.name); // Name
        sr.push_cell_string(acfRefItem.name2); // Name2
        sr.push_cell_u32(acfRefItem.id); // Id

        sr.next_row();
    }

    sr.begin_buffer_data_section();

    // Finish writing table.
    sr.writer().stream().pad(4);
    sr.finish();
}

static void write_stream_awb_hash_table_(
    detail_::write_params& wp,
    const rad::vector<wave_bank_hash>& streamAwbHashes)
{
    utf::serializer sr(*wp.stream, *wp.allocator);

    static const utf::column_info columns[] =
    {
        { utf::cell_type::string, "Name" },
        { utf::cell_type::buffer, "Hash" },
    };

    sr.start("StreamAwb", columns, wp.encoding);

    // Write rows.
    for (const auto& streamAwbHash : streamAwbHashes)
    {
        sr.push_cell_string(streamAwbHash.name); // Name
        sr.push_cell_buffer(); // Hash
        sr.next_row();
    }

    // Write buffer data.
    auto br = sr.begin_buffer_data_section();

    for (const auto& streamAwbHash : streamAwbHashes)
    {
        br.start();
        br.stream().write(streamAwbHash.md5Hash.data(), streamAwbHash.md5Hash.size());
        br.next();
    }

    // Finish writing table.
    sr.finish();
}

static const utf::column_info stream_awb_header_columns_[] =
{
    { utf::cell_type::buffer, "Header" },
};

static void read_stream_awb_header_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    rad::vector<rad::vector<unsigned char>>& streamAwbTocData)
{
    // TODO: In versions prior to 1.30.00, this table is actually written with
    // the table name "StreamAwb", followed by the following UNUSED strings: "Name", then
    // "Hash", followed by the name of the awb, followed by "StreamAwb" again (this one is
    // unused), finally followed by the "Header" string, which IS actually used by the column.

    // Also, the data begins with the MD5 hash of the awb data, followed immediately (no padding)
    // by the actual awb toc data.

    assert(streamAwbTocData.empty() &&
        "The given output vector must be empty"
    );

    // Read raw table header.
    utf::deserializer dr(stream, utf::deserialize_type::utf, tmpAllocator);

    // Validate columns.
    if (!dr.has_columns_of_exact_types(
        stream_awb_header_columns_,
        0,
        static_cast<u16>(std::size(stream_awb_header_columns_))))
    {
        throw std::runtime_error("Invalid or unsupported ACB stream AWB header layout");
    }

    // Read rows.
    auto& streamAwbTocDataAllocator = streamAwbTocData.allocator();
    streamAwbTocData.reserve(dr.row_count());

    for (u32 i = 0; i < dr.row_count(); dr.next_row(), ++i)
    {
        const auto rawHeader = dr.next_cell_as_buffer(); // Header

        // Header
        streamAwbTocData.emplace_back_unchecked(
            dr.read_buffer_data(rawHeader, streamAwbTocDataAllocator)
        );
    }
}

static void write_stream_awb_header_table_(
    detail_::write_params& wp,
    const rad::vector<rad::vector<unsigned char>>& streamAwbTocData)
{
    // TODO: In versions prior to 1.30.00, this table is actually written with
    // the table name "StreamAwb", followed by the following UNUSED strings: "Name", then
    // "Hash", followed by the name of the awb, followed by "StreamAwb" again (this one is
    // unused), finally followed by the "Header" string, which IS actually used by the column.

    // Also, the data begins with the MD5 hash of the awb data, followed immediately (no padding)
    // by the actual awb toc data.
    if (wp.version < cue_sheet_revisions_[4].version)
    {
        // TODO
        throw std::runtime_error("Writing StreamAwb header table for this revision is not yet implemented");
    }

    utf::serializer sr(*wp.stream, *wp.allocator);

    sr.start("StreamAwbHeader", stream_awb_header_columns_, wp.encoding);

    // Write rows.
    for (const auto& streamAwbToc : streamAwbTocData)
    {
        sr.push_cell_buffer(); // Header
        sr.next_row();
    }

    // Write buffer data.
    auto br = sr.begin_buffer_data_section();

    for (const auto& streamAwbToc : streamAwbTocData)
    {
        br.start();
        br.stream().write(streamAwbToc.data(), streamAwbToc.size());
        br.next();
    }

    // Finish writing table.
    sr.writer().stream().pad(4);
    sr.finish();
}

bool cue_sheet::has_any_command_tables() const noexcept
{
    return (
        !synthCommands.empty() ||
        !sequenceCommands.empty() ||
        !trackCommands.empty() ||
        !trackEventCommands.empty()
    );
}

u16 cue_sheet::get_related_waveform_count(const synth& synth) const
{
    u16 relatedWaveformCount = 0;

    // TODO: Do we need to account for synth command table??

    for (const auto& refItem : synth.refItems)
    {
        relatedWaveformCount += get_related_waveform_count(refItem);
    }
    
    return relatedWaveformCount;
}

u16 cue_sheet::get_related_waveform_count(const command_table& cmdTable) const
{
    u16 relatedWaveformCount = 0;
    
    for (const auto cmd : cmdTable)
    {
        switch (cmd.type())
        {
        case command_type::play:
        {
            const u16* args = static_cast<const u16*>(cmd.arguments_big_endian());
            const ref_item refItem(static_cast<ref_type>(args[0]), args[1]);

            relatedWaveformCount += get_related_waveform_count(refItem);
            break;
        }

        default:
            continue;
        }
    }

    return relatedWaveformCount;
}

u16 cue_sheet::get_related_waveform_count(const track& track) const
{
    u16 relatedWaveformCount = 0;

    // TOOD: Do we also need to do this for the track command table??

    if (track.eventIndex != UINT16_MAX)
    {
        const auto& trackEventCmdTable = trackEventCommands.at(track.eventIndex);
        relatedWaveformCount += get_related_waveform_count(trackEventCmdTable);
    }
    
    return relatedWaveformCount;
}

u16 cue_sheet::get_related_waveform_count(const sequence& sequence) const
{
    u16 relatedWaveformCount = 0;

    // TODO: Do we need to account for sequence command table??

    for (const auto trackIndex : sequence.trackIndices)
    {
        const auto& track = tracks.at(trackIndex);
        relatedWaveformCount += get_related_waveform_count(track);
    }

    return relatedWaveformCount;
}

u16 cue_sheet::get_related_waveform_count(ref_item refItem) const
{
    switch (refItem.type)
    {
    case ref_type::waveform:
        return 1;

    case ref_type::synth:
    {
        const auto& synth = synths.at(refItem.index);
        return get_related_waveform_count(synth);
    }

    case ref_type::sequence:
    {
        const auto& sequence = sequences.at(refItem.index);
        return get_related_waveform_count(sequence);
    }

    case ref_type::block_sequence:
        // TODO
        throw std::runtime_error("Not yet implemented");

    default:
        throw std::runtime_error("Unsupported ref_type");
    }
}

void cue_sheet::clear() noexcept
{
    cues.clear();
    waveforms.clear();
    aisacs.clear();
    graphs.clear();
    globalAisacNames.clear();
    synths.clear();
    sequenceCommands.clear();
    tracks.clear();
    sequences.clear();
    aisacControls.clear();
    autoModulations.clear();
    embeddedAwbData.clear();
    streamAwbHashes.clear();
    stringValues.clear();
    acfRefItems.clear();
    trackCommands.clear();
    synthCommands.clear();
    trackEventCommands.clear();
    streamAwbTocData.clear();
}

static const utf::column_info command_table_columns_[] =
{
    { utf::cell_type::buffer, "Command" },
};

static const utf::column_info stream_awb_hash_columns[] =
{
    { utf::cell_type::string, "Name" },
    { utf::cell_type::buffer, "Hash" },
};

static void read_stream_awb_hash_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    rad::vector<wave_bank_hash>& streamAwbHashes)
{
    assert(streamAwbHashes.empty() &&
        "The given output vector must be empty"
    );

    // Read raw table header.
    utf::deserializer dr(stream, utf::deserialize_type::utf, tmpAllocator);

    // Validate columns.
    if (!dr.has_columns_of_exact_types(
        stream_awb_hash_columns,
        0,
        static_cast<u16>(std::size(stream_awb_hash_columns))))
    {
        throw std::runtime_error("Invalid or unsupported ACB stream AWB hash layout");
    }

    // Read rows.
    auto& streamAwbHashAllocator = streamAwbHashes.allocator();
    streamAwbHashes.reserve(dr.row_count());

    for (u32 i = 0; i < dr.row_count(); dr.next_row(), ++i)
    {
        const auto name = dr.get_string_data(dr.next_cell_as_string()); // Name
        const auto rawHash = dr.next_cell_as_buffer(); // Hash

        // Hash
        md5_hash hash;

        if (rawHash.size != hash.size())
        {
            throw std::runtime_error("Invalid stream AWB hash size");
        }

        dr.reader().stream().jump_to(dr.get_buffer_data_position(rawHash));
        dr.reader().stream().read(hash.data(), hash.size());

        streamAwbHashes.emplace_back_unchecked(
            rad::string(streamAwbHashAllocator, name),
            hash
        );
    }
}

static void read_command_tables_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    rad::vector<command_table>& commandTables)
{
    assert(commandTables.empty() &&
        "The given output vector must be empty"
    );

    // Read raw table header.
    utf::deserializer dr(stream, utf::deserialize_type::utf, tmpAllocator);

    // Validate columns.
    if (!dr.has_columns_of_exact_types(
        command_table_columns_,
        0,
        static_cast<u16>(std::size(command_table_columns_))))
    {
        throw std::runtime_error("Invalid or unsupported ACB command tables layout");
    }

    // Read rows.
    auto& commandTableAllocator = commandTables.allocator();
    commandTables.reserve(dr.row_count());

    for (u32 i = 0; i < dr.row_count(); dr.next_row(), ++i)
    {
        const auto rawCommand = dr.next_cell_as_buffer(); // Command

        // Command
        commandTables.emplace_back_unchecked(
            dr.read_buffer_data(rawCommand, commandTableAllocator)
        );
    }
}

void read_graph_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    packed_version version,
    rad::vector<graph>& graphs
);

void read_auto_modulation_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    packed_version version,
    rad::vector<auto_modulation>& autoModulations
);

void read_aisac_control_name_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    packed_version version,
    rad::vector<aisac_control>& aisacControls
);

void read_aisac_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    packed_version version,
    rad::vector<aisac>& aisacs
);

void read_waveform_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    packed_version version,
    rad::vector<waveform>& waveforms
);

void read_synth_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    packed_version version,
    rad::vector<synth>& synths
);

void read_track_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    packed_version version,
    rad::vector<track>& tracks
);

void read_sequence_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    packed_version version,
    rad::vector<sequence>& sequences
);

void read_cue_name_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    packed_version version,
    rad::vector<cue>& cues
);

void read_cue_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    packed_version version,
    rad::vector<cue>& cues
);

cue_sheet::serialize_info cue_sheet::read_inner(
    rad::stream& stream,
    rad::allocator& tmpAllocator)
{
    clear();

    utf::deserializer dr(
        stream,
        utf::deserialize_type::inner_table,
        tmpAllocator
    );

    // Validate base columns.
    if (dr.row_count() != 1 || !dr.has_columns_of_exact_types(
        cue_sheet_columns_ + 36, 0, 3))
    {
        throw std::runtime_error("Invalid or unsupported ACB Header base layout");
    }

    // Read base columns.
    fileIdentifier = dr.next_cell_as_u32();
    const auto size = dr.next_cell_as_u32(); // TODO
    const packed_version version(dr.next_cell_as_u32());

    if (version > latest_supported_version)
    {
        // TODO: Log warning
    }

    // Validate revision columns.
    const auto revisionInfo = get_revision_info_for_version_(
        cue_sheet_revisions_,
        version
    );

    if (!dr.has_columns_of_exact_types(revisionInfo->columnGroup, cue_sheet_columns_))
    {
        throw std::runtime_error("Invalid or unsupported ACB Header revision layout");
    }

    // Read base columns.
    const auto type = dr.next_cell_as_u8(); // TODO
    const auto target = dr.next_cell_as_u8(); // TODO
    const auto rawAcfMd5Hash = dr.next_cell_as_buffer(); // AcfMd5Hash
    const auto categoryExtension = dr.next_cell_as_u8(); // TODO
    const auto cueTable = dr.next_cell_as_buffer(); // CueTable
    const auto cueNameTable = dr.next_cell_as_buffer(); // CueNameTable
    const auto waveformTable = dr.next_cell_as_buffer(); // WaveformTable
    const auto aisacTable = dr.next_cell_as_buffer(); // AisacTable
    const auto graphTable = dr.next_cell_as_buffer(); // GraphTable
    const auto globalAisacReferenceTable = dr.next_cell_as_buffer(); // TODO
    const auto aisacNameTable = dr.next_cell_as_buffer(); // TODO
    const auto synthTable = dr.next_cell_as_buffer(); // SynthTable
    const auto seqCommandTable = dr.next_cell_as_buffer(); // SeqCommandTable
    const auto trackTable = dr.next_cell_as_buffer(); // TrackTable
    const auto sequenceTable = dr.next_cell_as_buffer(); // SequenceTable
    const auto aisacControlNameTable = dr.next_cell_as_buffer(); // AisacControlNameTable
    const auto autoModulationTable = dr.next_cell_as_buffer(); // AutoModulationTable
    const auto streamAwbTocWorkOld = dr.next_cell_as_buffer(); // TODO
    const auto awbFile = dr.next_cell_as_buffer(); // AwbFile
    const auto versionString = dr.get_string_data(dr.next_cell_as_string()); // VersionString
    const auto cueLimitWorkTable = dr.next_cell_as_buffer(); // TODO
    const auto numCueLimitListWorks = dr.next_cell_as_u16(); // TODO
    const auto numCueLimitNodeWorks = dr.next_cell_as_u16(); // TODO
    const auto acbGuid = dr.next_cell_as_buffer(); // AcbGuid
    const auto streamAwbHash = dr.next_cell_as_buffer(); // StreamAwbHash
    const auto streamAwbTocWork_Old = dr.next_cell_as_buffer(); // TODO
    volume = dr.next_cell_as_f32(); // AcbVolume
    const auto stringValueTable = dr.next_cell_as_buffer(); // StringValueTable
    const auto outsideLinkTable = dr.next_cell_as_buffer(); // TODO
    const auto blockSequenceTable = dr.next_cell_as_buffer(); // TODO
    const auto blockTable = dr.next_cell_as_buffer(); // TODO

    // r1
    if (version >= cue_sheet_revisions_[1].version)
    {
        name.assign(dr.get_string_data(dr.next_cell_as_string())); // Name
    }

    // r2
    utf::raw_buffer acfReferenceTable = {};

    if (version >= cue_sheet_revisions_[2].version)
    {
        const auto characterEncodingType = dr.next_cell_as_u8(); // TODO
        const auto eventTable = dr.next_cell_as_buffer(); // TODO
        const auto actionTrackTable = dr.next_cell_as_buffer(); // TODO
        acfReferenceTable = dr.next_cell_as_buffer(); // AcfReferenceTable
    }

    // r3
    if (version >= cue_sheet_revisions_[3].version)
    {
        const auto waveformExtensionDataTable = dr.next_cell_as_buffer(); // TODO
        const auto beatSyncInfoTable = dr.next_cell_as_buffer(); // TODO
        const auto cuePriorityType = dr.next_cell_as_u8(); // TODO
        const auto numCueLimit = dr.next_cell_as_u16(); // TODO
    }

    // r4
    utf::raw_buffer trackCommandTable = {};
    utf::raw_buffer synthCommandTable = {};
    utf::raw_buffer trackEventTable = {};

    if (version >= cue_sheet_revisions_[4].version)
    {
        trackCommandTable = dr.next_cell_as_buffer(); // TrackCommandTable
        synthCommandTable = dr.next_cell_as_buffer(); // SynthCommandTable
        trackEventTable = dr.next_cell_as_buffer(); // TrackEventTable
        const auto seqParameterPalletTable = dr.next_cell_as_buffer(); // TODO
        const auto trackParameterPalletTable = dr.next_cell_as_buffer(); // TODO
        const auto synthParameterPalletTable = dr.next_cell_as_buffer(); // TODO
        const auto soundGeneratorTable = dr.next_cell_as_buffer(); // TODO
    }

    // r5
    if (version >= cue_sheet_revisions_[5].version)
    {
        if (version >= cue_sheet_revisions_[6].version)
        {
            // r6
            const auto instrumentPluginTrackTable = dr.next_cell_as_buffer(); // TODO
            const auto instrumentPluginParameterTable = dr.next_cell_as_buffer(); // TODO
            const auto lipsMorphTable = dr.next_cell_as_buffer(); // TODO
        }
        else
        {
            dr.skip_cell();
            dr.skip_cell();
            dr.skip_cell();
        }

        const auto projectKey = dr.next_cell_as_buffer(); // TODO
    }

    // r6
    if (version >= cue_sheet_revisions_[6].version)
    {
        const auto soundInstruments = dr.next_cell_as_buffer(); // TODO
        const auto soundProgramBankKey = dr.next_cell_as_buffer(); // TODO
        const auto midiTrackTable = dr.next_cell_as_buffer(); // TODO
    }

    // r7
    if (version >= cue_sheet_revisions_[7].version)
    {
        const auto soundProgramBankCommandTable = dr.next_cell_as_buffer(); // TODO
        const auto parameterAction = dr.next_cell_as_buffer(); // TODO
        const auto parameterActionCondition = dr.next_cell_as_buffer(); // TODO
        const auto stopAction = dr.next_cell_as_buffer(); // TODO
    }

    // Read footer columns.
    dr.go_to_cell((version >= cue_sheet_revisions_[6].version) ? 95 : 63);

    //const auto paddingArea = dr.next_cell_as_buffer();
    //const auto streamAwbTocWork = dr.next_cell_as_buffer();
    const auto streamAwbAfs2Header = dr.next_cell_as_buffer(); // StreamAwbAfs2Header

    // Read ACF MD5 hash.
    if (rawAcfMd5Hash.size == acfMd5Hash.size())
    {
        stream.jump_to(dr.get_buffer_data_position(rawAcfMd5Hash));
        stream.read(acfMd5Hash.data(), acfMd5Hash.size());
    }
    else
    {
        // TODO: Log warning.
        acfMd5Hash.fill(0);
    }

    // CueTable
    if (cueTable.size)
    {
        stream.jump_to(dr.get_buffer_data_position(cueTable));
        read_cue_table_(stream, tmpAllocator, version, cues);
    }

    // CueNameTable
    if (cueNameTable.size)
    {
        stream.jump_to(dr.get_buffer_data_position(cueNameTable));
        read_cue_name_table_(stream, tmpAllocator, version, cues);
    }

    // WaveformTable
    if (waveformTable.size)
    {
        stream.jump_to(dr.get_buffer_data_position(waveformTable));
        read_waveform_table_(stream, tmpAllocator, version, waveforms);
    }

    // AisacTable
    if (aisacTable.size)
    {
        stream.jump_to(dr.get_buffer_data_position(aisacTable));
        read_aisac_table_(stream, tmpAllocator, version, aisacs);
    }

    // GraphTable
    if (graphTable.size)
    {
        stream.jump_to(dr.get_buffer_data_position(graphTable));
        read_graph_table_(stream, tmpAllocator, version, graphs);
    }

    // TODO

    // SynthTable
    if (synthTable.size)
    {
        stream.jump_to(dr.get_buffer_data_position(synthTable));
        read_synth_table_(stream, tmpAllocator, version, synths);
    }

    // SeqCommandTable
    if (seqCommandTable.size)
    {
        stream.jump_to(dr.get_buffer_data_position(seqCommandTable));
        read_command_tables_(stream, tmpAllocator, sequenceCommands);
    }

    // TrackTable
    if (trackTable.size)
    {
        stream.jump_to(dr.get_buffer_data_position(trackTable));
        read_track_table_(stream, tmpAllocator, version, tracks);
    }

    // SequenceTable
    if (sequenceTable.size)
    {
        stream.jump_to(dr.get_buffer_data_position(sequenceTable));
        read_sequence_table_(stream, tmpAllocator, version, sequences);
    }

    // AisacControlNameTable
    if (aisacControlNameTable.size)
    {
        stream.jump_to(dr.get_buffer_data_position(aisacControlNameTable));
        read_aisac_control_name_table_(stream, tmpAllocator, version, aisacControls);
    }

    // AutoModulationTable
    if (autoModulationTable.size)
    {
        stream.jump_to(dr.get_buffer_data_position(autoModulationTable));
        read_auto_modulation_table_(stream, tmpAllocator, version, autoModulations);
    }

    // AwbFile
    if (awbFile.size)
    {
        stream.jump_to(dr.get_buffer_data_position(awbFile));
        embeddedAwbData.resize(rad::no_value_init, awbFile.size);
        stream.read(embeddedAwbData.data(), embeddedAwbData.size());
    }

    // TODO
    
    // AcbGuid
    if (acbGuid.size == id.data.size())
    {
        stream.jump_to(dr.get_buffer_data_position(acbGuid));
        stream.read(id.data.data(), id.data.size());
    }
    else
    {
        // TODO: Log warning
        id = guid::zero();
    }

    // StreamAwbHash
    if (streamAwbHash.size)
    {
        stream.jump_to(dr.get_buffer_data_position(streamAwbHash));

        if (streamAwbHash.size == 16)
        {
            md5_hash hash;
            stream.read(hash.data(), hash.size());

            if (hash != empty_md5_hash_)
            {
                streamAwbHashes.emplace_back(name, hash);
            }
        }
        else
        {
            read_stream_awb_hash_table_(stream, tmpAllocator, streamAwbHashes);
        }
    }

    // TODO

    // StringValueTable
    if (stringValueTable.size)
    {
        stream.jump_to(dr.get_buffer_data_position(stringValueTable));
        read_string_value_table_(stream, tmpAllocator, stringValues);
    }

    // TODO

    if (version >= cue_sheet_revisions_[2].version)
    {
        // TODO

        // AcfReferenceTable
        if (acfReferenceTable.size)
        {
            stream.jump_to(dr.get_buffer_data_position(acfReferenceTable));
            read_acf_reference_table_(stream, tmpAllocator, acfRefItems);
        }
    }

    // TODO

    if (version >= cue_sheet_revisions_[4].version)
    {
        // TrackCommandTable
        if (trackCommandTable.size)
        {
            stream.jump_to(dr.get_buffer_data_position(trackCommandTable));
            read_command_tables_(stream, tmpAllocator, trackCommands);
        }

        // SynthCommandTable
        if (synthCommandTable.size)
        {
            stream.jump_to(dr.get_buffer_data_position(synthCommandTable));
            read_command_tables_(stream, tmpAllocator, synthCommands);
        }

        // TrackEventTable
        if (trackEventTable.size)
        {
            stream.jump_to(dr.get_buffer_data_position(trackEventTable));
            read_command_tables_(stream, tmpAllocator, trackEventCommands);
        }
    }
    else
    {
        // TODO: Split up sequenceCommands into sequenceCommands,
        // trackCommands, synthCommands, and trackEventCommands
    }

    // TODO

    // StreamAwbAfs2Header
    if (streamAwbAfs2Header.size)
    {
        stream.jump_to(dr.get_buffer_data_position(streamAwbAfs2Header));

        if (version >= cue_sheet_revisions_[3].version)
        {
            read_stream_awb_header_table_(stream, tmpAllocator, streamAwbTocData);
        }
        else
        {
            streamAwbTocData.emplace_back(
                dr.read_buffer_data(
                    streamAwbAfs2Header,
                    streamAwbTocData.allocator())
            );
        }
    }

    return serialize_info{ version, encoding_type::utf8 }; // TODO: Pass correct encoding!!!
}

cue_sheet::serialize_info cue_sheet::read(
    rad::stream& stream,
    rad::allocator& tmpAllocator)
{
    // Read UTF header.
    utf::reader reader(stream);
    const auto header = reader.read_header();

    // Read inner table data.
    if (stream.capabilities().can_nocost_read() &&
        stream.capabilities().can_seek())
    {
        // Directly read the data from the stream.
        const auto tablePos = stream.tell();
        const auto serializeInfo = read_inner(stream, tmpAllocator);

        // Jump to the end of the table.
        stream.jump_to(tablePos + header.tableSize);

        return serializeInfo;
    }
    else
    {
        // Read all of the table data from the stream into
        // a buffer, then parse all of the data in the buffer.
        rad::vector<unsigned char> tableData(
            rad::no_value_init,
            tmpAllocator,
            header.tableSize
        );

        stream.read(tableData.data(), header.tableSize);

        rad::readonly_memory_stream tableDataStream(
            tableData.data(),
            header.tableSize
        );

        return read_inner(tableDataStream, tmpAllocator);
    }
}

static void start_write_command_utf_table_(
    utf::serializer& sr,
    std::string_view name,
    utf::encoding_type encoding)
{
    sr.start(name, command_table_columns_, encoding);
}

static void write_command_utf_table_rows_(
    utf::serializer& sr,
    const rad::vector<command_table>& cmdTables)
{
    for (const auto& cmdTable : cmdTables)
    {
        sr.push_cell_buffer(); // Command
        sr.next_row();
    }
}

static void write_command_utf_table_buffers_(
    utf::buffers_resolver& br,
    const rad::vector<command_table>& cmdTables)
{
    //constexpr unsigned long rowsOff = 5;
    //unsigned long long curBufferPos = utfPos + 32 + rowsOff;

    for (const auto& cmdTable : cmdTables)
    {
        br.start();
        //const auto cmdTableDataPos = sr.stream().tell();
        cmdTable.write(br.stream());
        br.next();

        //sr.fill_buffer(curBufferPos, cmdTableDataPos);
        //curBufferPos += 8;
    }
}

static void write_command_utf_tables_(
    utf::serializer& sr,
    std::string_view name,
    utf::encoding_type encoding,
    const rad::vector<command_table>& cmdTables)
{
    // Write rows.
    //const auto utfPos = sr.stream().tell();
    start_write_command_utf_table_(sr, name, encoding);
    write_command_utf_table_rows_(sr, cmdTables);

    // Write buffers.
    auto br = sr.begin_buffer_data_section();
    write_command_utf_table_buffers_(br, cmdTables);

    // Finish writing table.
    sr.writer().stream().pad(4);
    sr.finish();
}

void write_graph_table_(
    detail_::write_params& wp,
    const rad::vector<graph>& graphs
);

void write_auto_modulation_table_(
    detail_::write_params& wp,
    const rad::vector<auto_modulation>& autoModulations
);

void write_aisac_control_name_table_(
    detail_::write_params& wp,
    const rad::vector<aisac_control>& aisacControls
);

void write_aisac_table_(
    detail_::write_params& wp,
    const rad::vector<aisac>& aisacs
);

void write_waveform_table_(
    detail_::write_params& wp,
    const rad::vector<waveform>& waveforms
);

void write_synth_table_(
    detail_::write_params& wp,
    const rad::vector<synth>& synths
);

void write_track_table_(
    detail_::write_params& wp,
    const rad::vector<track>& tracks
);

void write_sequence_table_(
    detail_::write_params& wp,
    const rad::vector<sequence>& sequences
);

void write_cue_name_table_(
    detail_::write_params& wp,
    const rad::vector<const cue*>& sortedCues
);

void write_cue_table_(
    detail_::write_params& wp,
    const rad::vector<const cue*>& sortedCues,
    const rad::vector<synth>& synths
);

static const md5_hash& get_stream_awb_md5_hash_(
    const rad::string& name,
    const rad::vector<wave_bank_hash>& streamAwbHashes)
{
    for (const auto& streamAwbHash : streamAwbHashes)
    {
        if (streamAwbHash.name == name)
        {
            return streamAwbHash.md5Hash;
        }
    }

    return empty_md5_hash_;
}

void cue_sheet::write(
    rad::stream& stream,
    const serialize_info& serializeInfo,
    rad::allocator& tmpAllocator) const
{
    detail_::write_params wp;
    wp.stream = &stream;
    wp.allocator = &tmpAllocator;
    wp.version = serializeInfo.version;
    wp.encoding = (serializeInfo.encoding == encoding_type::shift_jis) ?
        utf::encoding_type::shift_jis : utf::encoding_type::utf8;

    wp.useGlobalCmdTable = (wp.version < cue_sheet_revisions_[4].version);

    //const auto totalGlobalAisacCount = compute_global_aisac_start_indices_(wp);

    //unsigned int id = 0;

    const auto revisionInfo = get_revision_info_for_version_(
        cue_sheet_revisions_,
        wp.version
    );

    rad::vector<utf::column_info> columns(tmpAllocator);

    columns.reserve((wp.version < cue_sheet_revisions_[6].version) ?
        cue_sheet_column_count_r0_ : cue_sheet_column_count_r6_
    );

    revisionInfo->columnGroup.append_to(cue_sheet_columns_, columns);

    // Write rows.
    utf::serializer sr(stream, tmpAllocator);
    sr.start(
        "Header",
        columns,
        wp.encoding
    );

    sr.push_cell_u32(0); // FileIdentifier
    sr.push_cell_u32(0); // Size
    sr.push_cell_u32(wp.version.value()); // Version
    sr.push_cell_u8(0); // Type
    sr.push_cell_u8(0); // Target

    sr.push_cell_buffer(); // AcfMd5Hash
    sr.push_cell_u8(0); // CategoryExtension

    sr.push_cell_buffer(cues.empty()); // CueTable
    sr.push_cell_buffer(cues.empty()); // CueNameTable
    sr.push_cell_buffer(waveforms.empty()); // WaveformTable
    sr.push_cell_buffer(aisacs.empty()); // AisacTable
    sr.push_cell_buffer(graphs.empty()); // GraphTable
    sr.push_cell_buffer(globalAisacNames.empty()); // GlobalAisacReferenceTable
    sr.push_cell_buffer(); // AisacNameTable
    sr.push_cell_buffer(synths.empty()); // SynthTable
    sr.push_cell_buffer(); // SeqCommandtable
    sr.push_cell_buffer(tracks.empty()); // TrackTable
    sr.push_cell_buffer(sequences.empty()); // SequenceTable
    sr.push_cell_buffer(aisacControls.empty()); // AisacControlNameTable
    sr.push_cell_buffer(autoModulations.empty()); // AutoModulationTable
    sr.push_cell_buffer(); // StreamAwbTocWorkOld
    sr.push_cell_buffer(embeddedAwbData.empty()); // AwbFile

    {
        char versionString[39];
        std::snprintf(
            versionString,
            sizeof(versionString),
            "\nACB Format/PC Ver.%hhu.%02hhu.%02hhu Build:\n",
            wp.version.get_major(),
            wp.version.get_minor(),
            wp.version.get_revision()
        );

        sr.push_cell_string(std::string_view{versionString}); // VersionString
    }

    sr.push_cell_buffer(); // CueLimitWorkTable
    sr.push_cell_u16(0); // NumCueLimitListWorks
    sr.push_cell_u16(0); // NumCueLimitNodeWorks
    sr.push_cell_buffer(); // AcbGuid
    sr.push_cell_buffer(); // StreamAwbHash
    sr.push_cell_buffer(); // StreamAwbTocWork_Old
    sr.push_cell_f32(1.0f); // AcbVolume
    sr.push_cell_buffer(stringValues.empty()); // StringValueTable
    sr.push_cell_buffer(); // OutsideLinkTable
    sr.push_cell_buffer(); // BlockSequenceTable
    sr.push_cell_buffer(); // BlockTable

    if (wp.version >= cue_sheet_revisions_[1].version)
    {
        sr.push_cell_string(name); // Name
    }
    else
    {
        sr.push_cell_u8(0); // R26
    }

    unsigned long long acfRefTableBufId;

    if (wp.version >= cue_sheet_revisions_[2].version)
    {
        sr.push_cell_u8(static_cast<u8>(serializeInfo.encoding)); // CharacterEncodingType
        sr.push_cell_buffer(); // EventTable
        sr.push_cell_buffer(); // ActionTrackTable
        acfRefTableBufId = sr.push_cell_buffer(acfRefItems.empty()); // AcfReferenceTable
    }
    else
    {
        sr.push_cell_u8(0); // R25
        sr.push_cell_u8(0); // R24
        sr.push_cell_u8(0); // R23
        sr.push_cell_u8(0); // R22
    }

    if (wp.version >= cue_sheet_revisions_[3].version)
    {
        sr.push_cell_buffer(); // WaveformExtensionDataTable
        sr.push_cell_buffer(); // BeatSyncInfoTable
        sr.push_cell_u8(255); // CuePriorityType
        sr.push_cell_u16(0); // NumCueLimit
    }
    else
    {
        sr.push_cell_u8(0); // R21
        sr.push_cell_u8(0); // R20
        sr.push_cell_u8(0); // R19
        sr.push_cell_u8(0); // R18
    }

    unsigned long long trackCmdTableBufId;
    unsigned long long synthCmdTableBufId;
    unsigned long long trackEventCmdTableBufId;

    if (wp.version >= cue_sheet_revisions_[4].version)
    {
        trackCmdTableBufId = sr.push_cell_buffer(trackCommands.empty()); // TrackCommandTable
        synthCmdTableBufId = sr.push_cell_buffer(synthCommands.empty()); // SynthCommandTable
        trackEventCmdTableBufId = sr.push_cell_buffer(trackEventCommands.empty()); // TrackEventTable
        sr.push_cell_buffer(); // SeqParameterPalletTable
        sr.push_cell_buffer(); // TrackParameterPalletTable
        sr.push_cell_buffer(); // SynthParameterPalletTable
        sr.push_cell_buffer(); // SoundGeneratorTable
    }
    else
    {
        sr.push_cell_u8(0); // R17
        sr.push_cell_u8(0); // R16
        sr.push_cell_u8(0); // R15
        sr.push_cell_u8(0); // R14
        sr.push_cell_u8(0); // R13
        sr.push_cell_u8(0); // R12
        sr.push_cell_u8(0); // R11
    }

    if (wp.version >= cue_sheet_revisions_[6].version)
    {
        sr.push_cell_buffer(); // InstrumentPluginTrackTable
        sr.push_cell_buffer(); // InstrumentPluginParameterTable
        sr.push_cell_buffer(); // LipsMorphTable
        sr.push_cell_buffer(); // ProjectKey
        sr.push_cell_buffer(); // SoundInstruments
        sr.push_cell_buffer(); // SoundProgramBankKey
        sr.push_cell_buffer(); // MIDITrackTable

        if (wp.version >= cue_sheet_revisions_[7].version)
        {
            sr.push_cell_buffer(); // SoundProgramBankCommandTable
            sr.push_cell_buffer(); // ParameterAction
            sr.push_cell_buffer(); // ParameterActionCondition
            sr.push_cell_buffer(); // StopAction
        }
        else
        {
            sr.push_cell_u8(0); // R35
            sr.push_cell_u8(0); // R34
            sr.push_cell_u8(0); // R33
            sr.push_cell_u8(0); // R32
        }

        sr.push_cell_u8(0); // R31
        sr.push_cell_u8(0); // R30
        sr.push_cell_u8(0); // R29
        sr.push_cell_u8(0); // R28
        sr.push_cell_u8(0); // R27
        sr.push_cell_u8(0); // R26
        sr.push_cell_u8(0); // R25
        sr.push_cell_u8(0); // R24
        sr.push_cell_u8(0); // R23
        sr.push_cell_u8(0); // R22
        sr.push_cell_u8(0); // R21
        sr.push_cell_u8(0); // R20
        sr.push_cell_u8(0); // R19
        sr.push_cell_u8(0); // R18
        sr.push_cell_u8(0); // R17
        sr.push_cell_u8(0); // R16
        sr.push_cell_u8(0); // R15
        sr.push_cell_u8(0); // R14
        sr.push_cell_u8(0); // R13
        sr.push_cell_u8(0); // R12
        sr.push_cell_u8(0); // R11
        sr.push_cell_u8(0); // R10
        sr.push_cell_u8(0); // R9
        sr.push_cell_u8(0); // R8
        sr.push_cell_u8(0); // R7
    }
    else
    {
        sr.push_cell_u8(0); // R10
        sr.push_cell_u8(0); // R9
        sr.push_cell_u8(0); // R8

        if (wp.version >= cue_sheet_revisions_[5].version)
        {
            sr.push_cell_buffer(); // ProjectKey
        }
        else
        {
            sr.push_cell_u8(0); // R7
        }
    }

    sr.push_cell_u8(0); // R6
    sr.push_cell_u8(0); // R5
    sr.push_cell_u8(0); // R4
    sr.push_cell_u8(0); // R3
    sr.push_cell_u8(0); // R2
    sr.push_cell_u8(0); // R1
    sr.push_cell_u8(0); // R0

    sr.push_cell_buffer(); // PaddingArea
    sr.push_cell_buffer(); // StreamAwbTocWork
    sr.push_cell_buffer(); // StreamAwbAfs2Header

    // Finish writing rows.
    auto br = sr.begin_buffer_data_section(32);

    // Write AcfMd5Hash.
    br.start();
    stream.write_as(acfMd5Hash);
    br.next();

    {
        // Sort cues by ID.
        rad::vector<const cue*> sortedCues(
            rad::no_value_init,
            *wp.allocator,
            cues.size()
        );

        for (std::size_t i = 0; i < cues.size(); ++i)
        {
            sortedCues[i] = cues.data() + i;
        }

        std::sort(sortedCues.begin(), sortedCues.end(),
            [](const cue* a, const cue* b)
            {
                return a->id < b->id;
            }
        );

        // Write CueTable.
        if (!cues.empty())
        {
            stream.pad(32);
            br.start();
            write_cue_table_(wp, sortedCues, synths);
        }

        br.next();

        // Write CueNameTable.
        if (!cues.empty())
        {
            stream.pad(32);
            br.start();
            write_cue_name_table_(wp, sortedCues);
        }

        br.next();
    }

    // Write WaveformTable.
    if (!waveforms.empty())
    {
        stream.pad(64);
        br.start();
        write_waveform_table_(wp, waveforms);
    }
    
    br.next();

    // Write AisacTable
    if (!aisacs.empty())
    {
        stream.pad(64);
        br.start();
        write_aisac_table_(wp, aisacs);
    }

    br.next();

    // Write GraphTable
    if (!graphs.empty())
    {
        stream.pad(32);
        br.start();
        write_graph_table_(wp, graphs);
    }

    br.next();

    // Write GlobalAisacReferenceTable.
    //if (totalGlobalAisacCount != 0)
    if (!globalAisacNames.empty())
    {
        stream.pad(32);
        br.start();
        write_global_aisac_reference_table_(wp, globalAisacNames);
    }
    
    br.next();

    // Write AisacNameTable.
    // TODO: Figure out the structure of this table and write it.
    br.next();

    // Write SynthTable.
    if (!synths.empty())
    {
        stream.pad(32);
        br.start();
        write_synth_table_(wp, synths);
    }
    
    br.next();

    // Write CommandTable.
    utf::serializer cmdTs(stream);

    if (wp.useGlobalCmdTable)
    {
        if (has_any_command_tables())
        {
            stream.pad(64);
            br.start();

            // Write rows.
            start_write_command_utf_table_(
                cmdTs,
                "Command",
                wp.encoding
            );

            // TODO: Collapse IDs and write in this order instead:
            // write_synths()
            // write_track_event_cmds()
            // write_track_cmds()
            // write_sequence_cmds();

            write_command_utf_table_rows_(cmdTs, synthCommands);
            // TODO: Write the rest of the command tables!!!

            // Write buffers.
            auto cmdBr = cmdTs.begin_buffer_data_section(32);
            write_command_utf_table_buffers_(cmdBr, synthCommands);
            // TODO: Write the rest of the command tables!!!

            // Finish writing table.
            cmdTs.finish();
        }
    }

    // Write SeqCommandTable.
    else if (!sequenceCommands.empty())
    {
        stream.pad(64);
        br.start();

        write_command_utf_tables_(
            cmdTs,
            "SequenceCommand",
            wp.encoding,
            sequenceCommands
        );
    }
    
    br.next();

    // Write TrackTable.
    if (!tracks.empty())
    {
        stream.pad(32);
        br.start();
        write_track_table_(wp, tracks);
    }
    
    br.next();

    // Write SequenceTable.
    if (!sequences.empty())
    {
        stream.pad(32);
        br.start();
        write_sequence_table_(wp, sequences);
    }
    
    br.next();

    // Write AisacControlNameTable.
    if (!aisacControls.empty())
    {
        stream.pad(32);
        br.start();
        write_aisac_control_name_table_(wp, aisacControls);
    }
    
    br.next();

    // Write AutoModulationTable.
    if (!autoModulations.empty())
    {
        stream.pad(32);
        br.start();
        write_auto_modulation_table_(wp, autoModulations);
    }
    
    br.next();

    // Write StreamAwbTocWorkOld.
    // TODO: Does this ever actually get written?
    br.next();

    // Write AwbFile.
    if (!embeddedAwbData.empty())
    {
        stream.pad(32);
        br.start();
        stream.write_as(embeddedAwbData.data(), embeddedAwbData.size());
        br.next();
    }
    else
    {
        br.next();
    }

    // Write CueLimitWorkTable.
    // TODO
    br.next();

    // Write AcbGuid.
    stream.pad(32);
    br.start();
    br.writer().stream().write(id.data.data(), id.data.size());
    br.next();

    // Write StreamAwbHash.
    stream.pad(32);
    br.start();

    if (wp.version >= cue_sheet_revisions_[3].version) // TODO: Is this the correct version??
    {
        if (!streamAwbHashes.empty())
        {
            write_stream_awb_hash_table_(wp, streamAwbHashes);
        }
        else
        {
            stream.write(empty_md5_hash_.data(), empty_md5_hash_.size());
        }
    }
    else
    {
        if (streamAwbHashes.size() > 1)
        {
            // TODO: Log warning that this revision only supports using up to 1 stream awb file
        }

        const auto streamAwbMd5Hash = get_stream_awb_md5_hash_(
            name,
            streamAwbHashes
        );

        stream.write(streamAwbMd5Hash.data(), streamAwbMd5Hash.size());
    }

    br.next();

    // Write StreamAwbTocWork_Old
    br.next();

    // Write StringValueTable.
    if (!stringValues.empty())
    {
        stream.pad(32);
        br.start();
        write_string_value_table_(wp, stringValues);
    }
    
    br.next();

    // Write OutsideLinkTable
    // TODO
    br.next();

    // Write BlockSequenceTable
    br.next();

    // Write BlockTable
    br.next();

    if (wp.version >= cue_sheet_revisions_[2].version)
    {
        // Write EventTable
        // TODO
        br.next();

        // Write ActionTrackTable
        br.next();

        // Write AcfReferenceTable
        if (!acfRefItems.empty())
        {
            stream.pad(32);
            br.start();
            write_acf_reference_table_(wp, acfRefItems);
        }

        br.next();
    }

    if (wp.version >= cue_sheet_revisions_[3].version)
    {
        // Write WaveformExtensionDataTable
        // TODO
        br.next();

        // Write BeatSyncInfoTable
        // TODO
        br.next();
    }

    if (wp.version >= cue_sheet_revisions_[4].version)
    {
        // Write TrackCommandTable.
        if (!wp.useGlobalCmdTable && !trackCommands.empty())
        {
            stream.pad(32);
            br.start();

            write_command_utf_tables_(
                cmdTs,
                "TrackCommand",
                wp.encoding,
                trackCommands
            );
        }

        br.next();

        // Write SynthCommandTable.
        if (!wp.useGlobalCmdTable && !synthCommands.empty())
        {
            stream.pad(32);
            br.start();

            write_command_utf_tables_(
                cmdTs,
                "SynthCommand",
                wp.encoding,
                synthCommands
            );
        }
        
        br.next();

        // Write TrackEventTable.
        if (!wp.useGlobalCmdTable && !trackEventCommands.empty())
        {
            stream.pad(32);
            br.start();

            write_command_utf_tables_(
                cmdTs,
                "TrackEvent",
                wp.encoding,
                trackEventCommands
            );
        }
        
        br.next();

        // Write SeqParameterPalletTable
        // TODO
        br.next();

        // Write TrackParameterPalletTable
        // TODO
        br.next();

        // Write SynthParameterPalletTable
        // TODO
        br.next();

        // Write SoundGeneratorTable
        // TODO
        br.next();
    }

    if (wp.version >= cue_sheet_revisions_[6].version)
    {
        // Write InstrumentPluginTrackTable
        // TODO
        br.next();

        // Write InstrumentPluginParameterTable
        // TODO
        br.next();

        // Write LipsMorphTable
        // TODO
        br.next();
    }

    if (wp.version >= cue_sheet_revisions_[5].version)
    {
        // Write ProjectKey
        // TODO
        br.next();
    }

    if (wp.version >= cue_sheet_revisions_[6].version)
    {
        // Write SoundInstruments
        // TODO
        br.next();

        // Write SoundProgramBankKey
        // TODO
        br.next();

        // Write MIDITrackTable
        // TODO
        br.next();
    }

    if (wp.version >= cue_sheet_revisions_[7].version)
    {
        // Write SoundProgramBankCommandTable
        // TODO
        br.next();

        // Write ParameterAction
        // TODO
        br.next();

        // Write ParameterActionCondition
        // TODO
        br.next();

        // Write StopAction
        // TODO
        br.next();
    }

    // PaddingArea
    // TODO
    br.next();

    // Write streaming awb toc data.
    if (!streamAwbTocData.empty())
    {
        // StreamAwbTocWork
        stream.pad(32);
        br.start();

        stream.write_nulls(
            (wp.version >= cue_sheet_revisions_[3].version) ?
            0x850 : 0x800
        );

        br.next();

        // StreamAwbAfs2Header
        stream.pad(32);
        br.start();

        if (wp.version >= cue_sheet_revisions_[3].version)
        {
            write_stream_awb_header_table_(wp, streamAwbTocData);
        }
        else
        {
            if (streamAwbTocData.size() > 1)
            {
                // TODO: Log warning that this revision only supports using up to 1 stream awb file
            }

            stream.write(streamAwbTocData[0].data(), streamAwbTocData[0].size());
        }

        br.next();
        stream.pad(32);
    }

    // Finish writing table.
    sr.finish();
}

cue_sheet::cue_sheet(
    rad::string name,
    guid id,
    rad::allocator& allocator) noexcept
    : cues(allocator)
    , waveforms(allocator)
    , aisacs(allocator)
    , graphs(allocator)
    , globalAisacNames(allocator)
    , synths(allocator)
    , sequenceCommands(allocator)
    , tracks(allocator)
    , sequences(allocator)
    , aisacControls(allocator)
    , autoModulations(allocator)
    , embeddedAwbData(allocator)
    , id(std::move(id))
    , streamAwbHashes(allocator)
    , stringValues(allocator)
    , name(std::move(name))
    , acfRefItems(allocator)
    , trackCommands(allocator)
    , synthCommands(allocator)
    , trackEventCommands(allocator)
    , streamAwbTocData(allocator)
{
}

cue_sheet::cue_sheet(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    rad::allocator& allocator)
    : cues(allocator)
    , waveforms(allocator)
    , aisacs(allocator)
    , graphs(allocator)
    , globalAisacNames(allocator)
    , synths(allocator)
    , sequenceCommands(allocator)
    , tracks(allocator)
    , sequences(allocator)
    , aisacControls(allocator)
    , autoModulations(allocator)
    , embeddedAwbData(allocator)
    , streamAwbHashes(allocator)
    , stringValues(allocator)
    , name(allocator)
    , acfRefItems(allocator)
    , trackCommands(allocator)
    , synthCommands(allocator)
    , trackEventCommands(allocator)
    , streamAwbTocData(allocator)
{
    read(stream, tmpAllocator);
}
}
