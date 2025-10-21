#include "hedgelib/cri/hl_cri_atom_cue_sheet.h"
#include <rad/rad_memory_stream.h>

namespace hl::cri_new::atom
{
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

static constexpr revision_info cue_sheet_revisions_[] =
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

const revision_info* get_revision_info(
    rad::span<const revision_info> revisionInfos,
    packed_version version) noexcept
{
    const revision_info* result = nullptr;

    for (const auto& revisionInfo : revisionInfos)
    {
        if (version < revisionInfo.version) break;

        result = &revisionInfo;
    }
    
    return result;
}

u16 reference_item::compute_related_waveform_count(const cue_sheet& cueSheet) const
{
    switch (type)
    {
    case reference_type::waveform:
        return 1;

    case reference_type::synth:
    {
        const auto& synth = cueSheet.synths.at(index);
        return synth.compute_related_waveform_count(cueSheet);
    }

    case reference_type::sequence:
    {
        const auto& sequence = cueSheet.sequences.at(index);
        return sequence.compute_related_waveform_count(cueSheet);
    }

    case reference_type::block_sequence:
        // TODO
        throw std::runtime_error("Not yet implemented");

    case reference_type::none:
        return 0;

    default:
        throw std::runtime_error("Unsupported ref type");
    }
}

command_table::const_iterator& command_table::const_iterator::operator++() noexcept
{
    const auto& cmd = *(*this);
    ptr_ += (sizeof(u16) + 1 + cmd.argument_count());
    return *this;
}

command_table::const_iterator command_table::const_iterator::operator++(int) noexcept
{
    const_iterator it;
    ++(*this);
    return it;
}

u16 command_table::compute_related_waveform_count(const cue_sheet& cueSheet) const
{
    u16 relatedWaveformCount = 0;
    
    for (const auto cmd : *this)
    {
        switch (cmd.type())
        {
        case command_type::play:
        {
            const u16* args = static_cast<const u16*>(cmd.arguments_big_endian());
            const reference_item refItem(static_cast<reference_type>(args[0]), args[1]);

            relatedWaveformCount += refItem.compute_related_waveform_count(cueSheet);
            break;
        }

        default:
            continue;
        }
    }

    return relatedWaveformCount;
}

void command_table::write(rad::stream& stream) const
{
    stream.write_as(rawData.data(), rawData.size());
}

command_table::command_table(rad::vector<unsigned char> data) noexcept
    : rawData(std::move(data))
{
}

config_reference_item::config_reference_item(
    config_reference_item_type type,
    rad::string name,
    rad::string name2,
    unsigned long id) noexcept
    : type(type)
    , name(std::move(name))
    , name2(std::move(name2))
    , id(id)
{
}

namespace detail_
{
    void validate_reference_type(u8 type)
    {
        if (type != static_cast<u8>(reference_type::waveform) &&
            type != static_cast<u8>(reference_type::synth) &&
            type != static_cast<u8>(reference_type::sequence) &&
            type != static_cast<u8>(reference_type::block_sequence))
        {
            throw std::runtime_error("Unsupported reference type");
        }
    }
}

u16 cue_sheet::compute_global_aisac_start_indices_(
    detail_::write_params& wp) const
{
    u16 startIndex = 0;

    for (const auto& synth : synths)
    {
        startIndex += synth.globalAisacs.size();
    }

    wp.globalAisacStartIndices.track = startIndex;

    for (const auto& track : tracks)
    {
        startIndex += track.globalAisacs.size();
    }

    wp.globalAisacStartIndices.sequence = startIndex;

    for (const auto& sequence : sequences)
    {
        startIndex += sequence.globalAisacs.size();
    }

    return startIndex;
}

void cue_sheet::write_string_value_table_(detail_::write_params& wp) const
{
    utf::table_serializer ts(*wp.stream, *wp.allocator);

    ts.start(
        "Strings",
        { { utf::cell_type::string, "StringValue" } },
        wp.encoding
    );

    // Write rows.
    for (const auto& strVal : stringValues)
    {
        ts.write_cell_as_string(strVal); // StringValue
        ts.next_row();
    }

    // Finish writing table.
    ts.finish();
    wp.stream->pad(32);
}

static void write_global_aisac_reference_row_(
    utf::table_serializer& ts,
    const rad::vector<rad::string>& globalAisacs)
{
    for (const auto& globalAisacName : globalAisacs)
    {
        ts.write_cell_as_string(globalAisacName); // Name
        ts.next_row();
    }
}

void cue_sheet::write_global_aisac_reference_table_(detail_::write_params& wp) const
{
    utf::table_serializer ts(*wp.stream, *wp.allocator);

    ts.start(
        "GlobalAisacReference",
        { { utf::cell_type::string, "Name" } },
        wp.encoding
    );

    // Write rows.
    for (const auto& synth : synths)
    {
        write_global_aisac_reference_row_(ts, synth.globalAisacs);
    }

    for (const auto& track : tracks)
    {
        write_global_aisac_reference_row_(ts, track.globalAisacs);
    }

    for (const auto& sequence : sequences)
    {
        write_global_aisac_reference_row_(ts, sequence.globalAisacs);
    }

    // Finish writing table.
    ts.finish();
    wp.stream->pad(32);
}

void cue_sheet::write_acf_reference_table_(detail_::write_params& wp) const
{
    utf::table_serializer ts(*wp.stream, *wp.allocator);

    static const utf::column_info columns[] =
    {
        { utf::cell_type::u8, "Type" },
        { utf::cell_type::string, "Name" },
        { utf::cell_type::string, "Name2" },
        { utf::cell_type::u32, "Id" },
    };

    ts.start("AcfReference", columns, wp.encoding);

    // Write rows.
    for (const auto& acfRefItem : acfRefItems)
    {
        ts.write_cell_as_u8(static_cast<u8>(acfRefItem.type)); // Type
        ts.write_cell_as_string(acfRefItem.name); // Name
        ts.write_cell_as_string(acfRefItem.name2); // Name2
        ts.write_cell_as_u32(acfRefItem.id); // Id

        ts.next_row();
    }

    // Finish writing table.
    ts.finish();
    wp.stream->pad(32);
}

void cue_sheet::write_stream_awb_hash_table_(
    detail_::write_params& wp,
    rad::span<const unsigned char> streamingAwbHash) const // TODO: Support multiple streaming awbs
{
    utf::table_serializer ts(*wp.stream, *wp.allocator);

    static const utf::column_info columns[] =
    {
        { utf::cell_type::string, "Name" },
        { utf::cell_type::buffer, "Hash" },
    };

    ts.start("StreamAwb", columns, wp.encoding);

    // Write rows.
    ts.write_cell_as_string(name); // Name
    const auto hashBufId = ts.write_cell_as_buffer(); // Hash

    ts.finish_rows();

    // Write data.
    const auto hashDataPos = ts.stream().tell();
    ts.stream().write(streamingAwbHash.data(), streamingAwbHash.size());
    ts.fill_buffer_cell(hashBufId, hashDataPos);

    // Finish writing table.
    ts.finish();
}

void cue_sheet::write_stream_awb_header_table_(
    detail_::write_params& wp,
    rad::span<const unsigned char> streamingAwbToc) // TODO: Support multiple streaming awbs
{
    // TODO: In versions prior to 1.30.00, this table is actually written with
    // the table name "StreamAwb", followed by the following UNUSED strings: "Name", then
    // "Hash", followed by the name of the awb, followed by "StreamAwb" again (this one is
    // unused), finally followed by the "Header" string, which IS actually used by the column.

    // Also, the data begins with the MD5 hash of the awb data, followed immediately (no padding)
    // by the actual awb toc data.

    utf::table_serializer ts(*wp.stream, *wp.allocator);

    static const utf::column_info columns[] =
    {
        { utf::cell_type::buffer, "Header" },
    };

    ts.start("StreamAwbHeader", columns, wp.encoding);

    // Write rows.
    const auto headerBufId = ts.write_cell_as_buffer(); // Header

    ts.finish_rows();

    // Write data.
    const auto headerDataPos = ts.stream().tell();
    ts.stream().write(streamingAwbToc.data(), streamingAwbToc.size());
    ts.fill_buffer_cell(headerBufId, headerDataPos);

    // Finish writing table.
    ts.finish();
}

void cue_sheet::read_columns_ex_(
    utf::table_deserializer& td,
    packed_version version)
{
    // r1
    if (version < cue_sheet_revisions_[1].version) return;

    const auto name = td.read_cell_as_string();
    this->name = td.get_string_data(name);

    // r2
    if (version < cue_sheet_revisions_[2].version) return;

    const auto characterEncodingType = td.read_cell_as_u8(); // TODO
    const auto eventTable = td.read_cell_as_buffer(); // TODO
    const auto actionTrackTable = td.read_cell_as_buffer(); // TODO
    const auto acfReferenceTable = td.read_cell_as_buffer(); // TODO

    // r3, v1.26.00+
    const auto waveformExtensionDataTable = td.read_cell_as_buffer(); // TODO
    const auto beatSyncInfoTable = td.read_cell_as_buffer(); // TODO
    const auto cuePriorityType = td.read_cell_as_u8(); // TODO
    const auto numCueLimit = td.read_cell_as_u16(); // TODO

    // r4, v1.30.00+
    const auto trackCommandTable = td.read_cell_as_buffer(); // TODO
    const auto synthCommandTable = td.read_cell_as_buffer(); // TODO
    const auto trackEventTable = td.read_cell_as_buffer(); // TODO
    const auto seqParameterPalletTable = td.read_cell_as_buffer(); // TODO
    const auto trackParameterPalletTable = td.read_cell_as_buffer(); // TODO
    const auto synthParameterPalletTable = td.read_cell_as_buffer(); // TODO
    const auto soundGeneratorTable = td.read_cell_as_buffer(); // TODO

    // r6, v1.37.00+
    const auto instrumentPluginTrackTable = td.read_cell_as_buffer(); // TODO
    const auto instrumentPluginParameterTable = td.read_cell_as_buffer(); // TODO
    const auto lipsMorphTable = td.read_cell_as_buffer(); // TODO

    // r5, v1.32.01+
    const auto projectKey = td.read_cell_as_buffer(); // TODO

    // r6, v1.37.00+
    const auto soundInstruments = td.read_cell_as_buffer(); // TODO
    const auto soundProgramBankKey = td.read_cell_as_buffer(); // TODO
    const auto midiTrackTable = td.read_cell_as_buffer(); // TODO

    // r7, v1.42.01+
    const auto soundProgramBankCommandTable = td.read_cell_as_buffer(); // TODO
    const auto parameterAction = td.read_cell_as_buffer(); // TODO
    const auto parameterActionCondition = td.read_cell_as_buffer(); // TODO
    const auto stopAction = td.read_cell_as_buffer(); // TODO
}

bool cue_sheet::has_any_command_tables() const noexcept
{
    return (
        !synthCmdTables.empty() ||
        !sequenceCmdTables.empty() ||
        !trackCmdTables.empty() ||
        !trackEventCmdTables.empty()
    );
}

void cue_sheet::clear() noexcept
{
    acfRefItems.clear();
    cues.clear();
    synths.clear();
    synthCmdTables.clear();
    sequences.clear();
    sequenceCmdTables.clear();
    tracks.clear();
    trackCmdTables.clear();
    trackEventCmdTables.clear();
    waveforms.clear();
    aisacs.clear();
    autoModulations.clear();
    graphs.clear();
    aisacControls.clear();
    stringValues.clear();
    name.clear();
}

void cue_sheet::read_inner(rad::stream& stream)
{
    clear();

    utf::table_deserializer td(stream, utf::table_deserialize_type::inner_table);

    // Validate base columns.
    if (td.row_count() != 1 || td.column_count() < cue_sheet_column_count_r0_ ||
        !td.are_columns_exact_types(cue_sheet_columns_, 0, 3))
    {
        throw std::runtime_error("Invalid or unsupported ACB Header base layout");
    }

    // Read base columns.
    const auto fileIdentifier = td.read_cell_as_u32(); // TODO
    const auto size = td.read_cell_as_u32(); // TODO
    const packed_version version(td.read_cell_as_u32());

    // Validate revision columns.

    // TODO: Log warning if version > latest_supported_version 

    const auto revisionInfo = get_revision_info(
        cue_sheet_revisions_,
        version
    );

    if (!revisionInfo->columns.validate(cue_sheet_columns_, td))
    {
        throw std::runtime_error("Invalid or unsupported ACB Header revision layout");
    }

    // Read base columns.
    const auto type = td.read_cell_as_u8(); // TODO
    const auto target = td.read_cell_as_u8(); // TODO
    const auto acfMd5Hash = td.read_cell_as_buffer(); // TODO
    const auto categoryExtension = td.read_cell_as_u8(); // TODO
    const auto cueTable = td.read_cell_as_buffer(); // TODO
    const auto cueNameTable = td.read_cell_as_buffer(); // TODO
    const auto waveformTable = td.read_cell_as_buffer(); // TODO
    const auto aisacTable = td.read_cell_as_buffer(); // TODO
    const auto graphTable = td.read_cell_as_buffer(); // TODO
    const auto globalAisacReferenceTable = td.read_cell_as_buffer(); // TODO
    const auto aisacNameTable = td.read_cell_as_buffer(); // TODO
    const auto synthTable = td.read_cell_as_buffer(); // TODO
    const auto seqCommandTable = td.read_cell_as_buffer(); // TODO
    const auto trackTable = td.read_cell_as_buffer(); // TODO
    const auto sequenceTable = td.read_cell_as_buffer(); // TODO
    const auto aisacControlNameTable = td.read_cell_as_buffer(); // TODO
    const auto autoModulationTable = td.read_cell_as_buffer(); // TODO
    const auto streamAwbTocWorkOld = td.read_cell_as_buffer(); // TODO
    const auto awbFile = td.read_cell_as_buffer(); // TODO
    const auto versionString = td.read_cell_as_string(); // TODO
    const auto cueLimitWorkTable = td.read_cell_as_buffer(); // TODO
    const auto numCueLimitListWorks = td.read_cell_as_u16(); // TODO
    const auto numCueLimitNodeWorks = td.read_cell_as_u16(); // TODO
    const auto acbGuid = td.read_cell_as_buffer(); // TODO
    const auto streamAwbHash = td.read_cell_as_buffer(); // TODO
    const auto streamAwbTocWork_Old = td.read_cell_as_buffer(); // TODO
    const auto acbVolume = td.read_cell_as_f32(); // TODO
    const auto stringValueTable = td.read_cell_as_buffer(); // TODO
    const auto outsideLinkTable = td.read_cell_as_buffer(); // TODO
    const auto blockSequenceTable = td.read_cell_as_buffer(); // TODO
    const auto blockTable = td.read_cell_as_buffer(); // TODO

    // Read extra columns.
    read_columns_ex_(td, version);

    // Read footer columns.
    const auto paddingArea = td.read_cell_as_buffer();
    const auto streamAwbTocWork = td.read_cell_as_buffer();
    const auto streamAwbAfs2Header = td.read_cell_as_buffer(); // TODO

    // Read ACF MD5 hash.
    if (acfMd5Hash.size != sizeof(this->acfMD5Hash))
    {
        // TODO: Is it also valid to have the buffer be empty?
        throw std::runtime_error("Invalid ACF MD5 hash");
    }
    else
    {
        stream.jump_to(td.get_buffer_data_position(acfMd5Hash));
        stream.read(this->acfMD5Hash, sizeof(this->acfMD5Hash));
    }

    // Read cues.
    if (cueTable.size)
    {
        stream.jump_to(td.get_buffer_data_position(cueTable));
        read_cue_table_(stream, version);
    }

    // TODO: Validate new format version columns

    // TODO: Read new format version columns
}

void cue_sheet::read(rad::stream& stream)
{
    // Read UTF header.
    utf::table_reader reader(stream);
    const auto rawUtfHeader = reader.read_raw_utf_header();

    // Read inner table data.
    if (stream.capabilities().can_nocost_read() &&
        stream.capabilities().can_seek())
    {
        // Directly read the data from the stream.
        const auto rawTablePos = stream.tell();
        read_inner(stream);

        // Jump to the end of the table.
        stream.jump_to(rawTablePos + rawUtfHeader.tableSize);
    }
    else
    {
        // Read all of the table data from the stream into
        // a buffer, then parse all of the data in the buffer.

        std::unique_ptr<unsigned char[]> tableData(
            new unsigned char[rawUtfHeader.tableSize]
        );

        stream.read(tableData.get(), rawUtfHeader.tableSize);

        rad::readonly_memory_stream tableDataStream(
            tableData.get(),
            rawUtfHeader.tableSize
        );

        read_inner(tableDataStream);
    }
}

static void start_write_command_utf_table_(
    utf::table_serializer& ts,
    std::string_view name,
    utf::encoding_type encoding)
{
    static const utf::column_info columns[] =
    {
        { utf::cell_type::buffer, "Command" },
    };

    ts.start(name, columns, encoding);
}

static void write_command_utf_table_rows_(
    utf::table_serializer& ts,
    const rad::vector<command_table>& cmdTables)
{
    for (const auto& cmdTable : cmdTables)
    {
        ts.write_cell_as_buffer(); // Command
        ts.next_row();
    }
}

static void write_command_utf_table_buffers_(
    utf::table_serializer& ts,
    const rad::vector<command_table>& cmdTables,
    unsigned long long utfPos)
{
    constexpr unsigned long rowsOff = 5;
    unsigned long long curBufferPos = utfPos + 32 + rowsOff;

    for (const auto& cmdTable : cmdTables)
    {
        const auto cmdTableDataPos = ts.stream().tell();
        cmdTable.write(ts.stream());

        ts.fill_buffer(curBufferPos, cmdTableDataPos);
        curBufferPos += 8;
    }
}

static void write_command_utf_tables_(
    utf::table_serializer& ts,
    std::string_view name,
    utf::encoding_type encoding,
    const rad::vector<command_table>& cmdTables)
{
    // Write rows.
    const auto utfPos = ts.stream().tell();
    start_write_command_utf_table_(ts, name, encoding);
    write_command_utf_table_rows_(ts, cmdTables);

    // Write buffers.
    ts.finish_rows(32);
    write_command_utf_table_buffers_(ts, cmdTables, utfPos);

    // Finish writing table.
    ts.finish();
    ts.stream().pad(32);
}

void cue_sheet::write(
    rad::stream& stream,
    rad::span<const unsigned char> embeddedAwb,
    rad::span<const unsigned char> streamingAwbToc,
    packed_version version,
    utf::encoding_type encoding) const
{
    detail_::write_params wp;
    wp.stream = &stream;
    wp.allocator = &rad::default_allocator; // TODO !!
    wp.version = version;
    wp.encoding = encoding;
    wp.useGlobalCmdTable = (version < cue_sheet_revisions_[4].version);

    const auto totalGlobalAisacCount = compute_global_aisac_start_indices_(wp);

    //unsigned int id = 0;

    const auto revisionInfo = get_revision_info(
        cue_sheet_revisions_,
        version
    );

    rad::vector<utf::column_info> columns(*wp.allocator);

    columns.reserve((version < cue_sheet_revisions_[6].version) ?
        cue_sheet_column_count_r0_ : cue_sheet_column_count_r6_
    );

    revisionInfo->columns.append_to(cue_sheet_columns_, columns);

    // Write rows.
    utf::table_serializer ts(stream);
    ts.start("Header", columns, encoding, false, false);

    ts.write_cell_as_u32(0); // FileIdentifier
    ts.write_cell_as_u32(0); // Size
    ts.write_cell_as_u32(version.value()); // Version
    ts.write_cell_as_u8(0); // Type
    ts.write_cell_as_u8(0); // Target

    ts.write_cell_as_buffer(); // AcfMd5Hash
    ts.write_cell_as_u8(0); // CategoryExtension

    ts.write_cell_as_buffer(); // CueTable
    ts.write_cell_as_buffer(); // CueNameTable
    ts.write_cell_as_buffer(); // WaveformTable
    ts.write_cell_as_buffer(); // AisacTable
    ts.write_cell_as_buffer(); // GraphTable
    ts.write_cell_as_buffer(); // GlobalAisacReferenceTable
    ts.write_cell_as_buffer(); // AisacNameTable
    ts.write_cell_as_buffer(); // SynthTable
    ts.write_cell_as_buffer(); // SeqCommandtable
    ts.write_cell_as_buffer(); // TrackTable
    ts.write_cell_as_buffer(); // SequenceTable
    ts.write_cell_as_buffer(); // AisacControlNameTable
    ts.write_cell_as_buffer(); // AutoModulationTable
    ts.write_cell_as_buffer(); // StreamAwbTocWorkOld
    ts.write_cell_as_buffer(); // AwbFile

    {
        // TODO: Optimize this with snprintf or something
        std::string versionString("\nACB Format/PC Ver."); // TODO: Other platforms
        versionString += std::to_string(version.get_major());
        versionString += ".";
        versionString += std::to_string(version.get_minor());
        versionString += ".";
        versionString += std::to_string(version.get_revision());
        versionString += " Build:\n";

        ts.write_cell_as_string(std::move(versionString)); // VersionString
    }

    ts.write_cell_as_buffer(); // CueLimitWorkTable
    ts.write_cell_as_u16(0); // NumCueLimitListWorks
    ts.write_cell_as_u16(0); // NumCueLimitNodeWorks
    ts.write_cell_as_buffer(); // AcbGuid

    // TODO: StreamAwbHash becomes a UTF table in the newer versions!
    const auto streamAwbHashBufId = ts.write_cell_as_buffer(); // StreamAwbHash

    ts.write_cell_as_buffer(); // StreamAwbTocWork_Old
    ts.write_cell_as_f32(1.0f); // AcbVolume
    const auto stringValTableBufId = ts.write_cell_as_buffer(); // StringValueTable
    ts.write_cell_as_buffer(); // OutsideLinkTable
    ts.write_cell_as_buffer(); // BlockSequenceTable
    ts.write_cell_as_buffer(); // BlockTable

    if (version >= cue_sheet_revisions_[1].version)
    {
        ts.write_cell_as_string(name); // Name
    }
    else
    {
        ts.write_cell_as_u8(0); // R26
    }

    unsigned long long acfRefTableBufId;

    if (version >= cue_sheet_revisions_[2].version)
    {
        ts.write_cell_as_u8(0); // CharacterEncodingType
        ts.write_cell_as_buffer(); // EventTable
        ts.write_cell_as_buffer(); // ActionTrackTable
        acfRefTableBufId = ts.write_cell_as_buffer(); // AcfReferenceTable
    }
    else
    {
        ts.write_cell_as_u8(0); // R25
        ts.write_cell_as_u8(0); // R24
        ts.write_cell_as_u8(0); // R23
        ts.write_cell_as_u8(0); // R22
    }

    if (version >= cue_sheet_revisions_[3].version)
    {
        ts.write_cell_as_buffer(); // WaveformExtensionDataTable
        ts.write_cell_as_buffer(); // BeatSyncInfoTable
        ts.write_cell_as_u8(255); // CuePriorityType
        ts.write_cell_as_u16(0); // NumCueLimit
    }
    else
    {
        ts.write_cell_as_u8(0); // R21
        ts.write_cell_as_u8(0); // R20
        ts.write_cell_as_u8(0); // R19
        ts.write_cell_as_u8(0); // R18
    }

    unsigned long long trackCmdTableBufId;
    unsigned long long synthCmdTableBufId;
    unsigned long long trackEventCmdTableBufId;

    if (version >= cue_sheet_revisions_[4].version)
    {
        trackCmdTableBufId = ts.write_cell_as_buffer(); // TrackCommandTable
        synthCmdTableBufId = ts.write_cell_as_buffer(); // SynthCommandTable
        trackEventCmdTableBufId = ts.write_cell_as_buffer(); // TrackEventTable
        ts.write_cell_as_buffer(); // SeqParameterPalletTable
        ts.write_cell_as_buffer(); // TrackParameterPalletTable
        ts.write_cell_as_buffer(); // SynthParameterPalletTable
        ts.write_cell_as_buffer(); // SoundGeneratorTable
    }
    else
    {
        ts.write_cell_as_u8(0); // R17
        ts.write_cell_as_u8(0); // R16
        ts.write_cell_as_u8(0); // R15
        ts.write_cell_as_u8(0); // R14
        ts.write_cell_as_u8(0); // R13
        ts.write_cell_as_u8(0); // R12
        ts.write_cell_as_u8(0); // R11
    }

    if (version >= cue_sheet_revisions_[6].version)
    {
        ts.write_cell_as_buffer(); // InstrumentPluginTrackTable
        ts.write_cell_as_buffer(); // InstrumentPluginParameterTable
        ts.write_cell_as_buffer(); // LipsMorphTable
        ts.write_cell_as_buffer(); // ProjectKey
        ts.write_cell_as_buffer(); // SoundInstruments
        ts.write_cell_as_buffer(); // SoundProgramBankKey
        ts.write_cell_as_buffer(); // MIDITrackTable

        if (version >= cue_sheet_revisions_[7].version)
        {
            ts.write_cell_as_buffer(); // SoundProgramBankCommandTable
            ts.write_cell_as_buffer(); // ParameterAction
            ts.write_cell_as_buffer(); // ParameterActionCondition
            ts.write_cell_as_buffer(); // StopAction
        }
        else
        {
            ts.write_cell_as_u8(0); // R35
            ts.write_cell_as_u8(0); // R34
            ts.write_cell_as_u8(0); // R33
            ts.write_cell_as_u8(0); // R32
        }

        ts.write_cell_as_u8(0); // R31
        ts.write_cell_as_u8(0); // R30
        ts.write_cell_as_u8(0); // R29
        ts.write_cell_as_u8(0); // R28
        ts.write_cell_as_u8(0); // R27
        ts.write_cell_as_u8(0); // R26
        ts.write_cell_as_u8(0); // R25
        ts.write_cell_as_u8(0); // R24
        ts.write_cell_as_u8(0); // R23
        ts.write_cell_as_u8(0); // R22
        ts.write_cell_as_u8(0); // R21
        ts.write_cell_as_u8(0); // R20
        ts.write_cell_as_u8(0); // R19
        ts.write_cell_as_u8(0); // R18
        ts.write_cell_as_u8(0); // R17
        ts.write_cell_as_u8(0); // R16
        ts.write_cell_as_u8(0); // R15
        ts.write_cell_as_u8(0); // R14
        ts.write_cell_as_u8(0); // R13
        ts.write_cell_as_u8(0); // R12
        ts.write_cell_as_u8(0); // R11
        ts.write_cell_as_u8(0); // R10
        ts.write_cell_as_u8(0); // R9
        ts.write_cell_as_u8(0); // R8
        ts.write_cell_as_u8(0); // R7
    }
    else
    {
        ts.write_cell_as_u8(0); // R10
        ts.write_cell_as_u8(0); // R9
        ts.write_cell_as_u8(0); // R8

        if (version >= cue_sheet_revisions_[5].version)
        {
            ts.write_cell_as_buffer(); // ProjectKey
        }
        else
        {
            ts.write_cell_as_u8(0); // R7
        }
    }

    ts.write_cell_as_u8(0); // R6
    ts.write_cell_as_u8(0); // R5
    ts.write_cell_as_u8(0); // R4
    ts.write_cell_as_u8(0); // R3
    ts.write_cell_as_u8(0); // R2
    ts.write_cell_as_u8(0); // R1
    ts.write_cell_as_u8(0); // R0

    ts.write_cell_as_buffer(); // PaddingArea
    const auto streamAwbTocWorkBufId = ts.write_cell_as_buffer(); // StreamAwbTocWork
    const auto streamAwbAfs2HeaderBufId = ts.write_cell_as_buffer(); // StreamAwbAfs2Header

    // Finish writing rows.
    ts.finish_rows(32);

    // Write AcfMd5Hash.
    {
        const auto acfMd5HashDataPos = stream.tell();

        stream.write_as(acfMD5Hash);

        ts.fill_buffer_cell(5, acfMd5HashDataPos);
        stream.pad(32);
    }

    // Write CueTable.
    if (!cues.empty())
    {
        const auto cueTableDataPos = stream.tell();
        write_cue_table_(wp);
        ts.fill_buffer_cell(7, cueTableDataPos);
    }

    // Write CueNameTable.
    if (!cues.empty())
    {
        const auto cueNameTableDataPos = stream.tell();
        write_cue_name_table_(wp);
        ts.fill_buffer_cell(8, cueNameTableDataPos);
    }

    // Write WaveformTable.
    if (!waveforms.empty())
    {
        const auto waveformTableDataPos = stream.tell();
        write_waveform_table_(wp);
        ts.fill_buffer_cell(9, waveformTableDataPos);
    }

    // Write AisacTable
    {
        const auto aisacTableDataPos = stream.tell();
        detail_::write_aisac_table(wp, aisacs);
        ts.fill_buffer_cell(10, aisacTableDataPos);
    }

    // Write GraphTable
    {
        const auto graphTableDataPos = stream.tell();
        detail_::write_graph_table(wp, graphs);
        ts.fill_buffer_cell(11, graphTableDataPos);
    }

    // Write GlobalAisacReferenceTable.
    if (totalGlobalAisacCount != 0)
    {
        const auto globalAisacRefTableDataPos = stream.tell();
        write_global_aisac_reference_table_(wp);
        ts.fill_buffer_cell(12, globalAisacRefTableDataPos);
    }

    // Write AisacNameTable.
    // TODO: Figure out the structure of this table and write them.

    // Write SynthTable.
    if (!synths.empty())
    {
        const auto synthTableDataPos = stream.tell();
        write_synth_table_(wp);
        ts.fill_buffer_cell(14, synthTableDataPos);
    }

    // Write CommandTable.
    utf::table_serializer cmdTs(stream);

    if (wp.useGlobalCmdTable)
    {
        if (has_any_command_tables())
        {
            const auto cmdTableDataPos = stream.tell();

            // Write rows.
            start_write_command_utf_table_(
                cmdTs,
                "Command",
                encoding
            );

            // TODO: Collapse IDs and write in this order instead:
            // write_synths()
            // write_sequences() {
            //   write_track_event_cmds();
            //   write_track_cmds();
            //   write_sequence_cmds();
            // }

            write_command_utf_table_rows_(cmdTs, synthCmdTables);
            // TODO: Write the rest of the command tables!!!

            // Write buffers.
            cmdTs.finish_rows(32);
            write_command_utf_table_buffers_(cmdTs, synthCmdTables, cmdTableDataPos);
            // TODO: Write the rest of the command tables!!!

            // Finish writing table.
            cmdTs.finish();
            cmdTs.stream().pad(32);
            ts.fill_buffer_cell(15, cmdTableDataPos);
        }
    }

    // Write SeqCommandTable.
    else if (!sequenceCmdTables.empty())
    {
        const auto seqCmdTableDataPos = stream.tell();

        write_command_utf_tables_(
            cmdTs,
            "SequenceCommand",
            encoding,
            sequenceCmdTables
        );

        ts.fill_buffer_cell(15, seqCmdTableDataPos);
    }

    // Write TrackTable.
    if (!tracks.empty())
    {
        const auto trackTableDataPos = stream.tell();
        write_track_table_(wp);
        ts.fill_buffer_cell(16, trackTableDataPos);
    }

    // Write SequenceTable.
    if (!sequences.empty())
    {
        const auto seqTableDataPos = stream.tell();
        write_sequence_table_(wp);
        ts.fill_buffer_cell(17, seqTableDataPos);
    }

    // Write AisacControlNameTable.
    if (!aisacControls.empty())
    {
        const auto aisacControlNameTableDataPos = stream.tell();
        detail_::write_aisac_control_name_table(wp, aisacControls);
        ts.fill_buffer_cell(18, aisacControlNameTableDataPos);
    }

    // Write AutoModulationTable.
    if (!autoModulations.empty())
    {
        const auto autoModulationTableDataPos = stream.tell();
        detail_::write_auto_modulation_table(wp, autoModulations);
        ts.fill_buffer_cell(19, autoModulationTableDataPos);
    }

    // Write StreamAwbTocWorkOld.
    // TODO: Does this ever actually get written?

    // Write AwbFile.
    if (!embeddedAwb.empty())
    {
        const auto awbFileDataPos = stream.tell();
        stream.write_as(embeddedAwb.data(), embeddedAwb.size());
        ts.fill_buffer_cell(21, awbFileDataPos);
        //stream.pad(32);
    }

    // Write CueLimitWorkTable.
    // TODO

    // Write AcbGuid.
    // TODO

    // Write StreamAwbHash.
    {
        // TODO: Don't hardcode this!!!
        const unsigned char streamAwbHash[16] = {
            //0xFE, 0x3B, 0xF2, 0xA3, 0x51, 0x61, 0x09, 0x38, 0xA1, 0xCE, 0xD7, 0x24, 0xA5, 0xD8, 0x8F, 0x24
            //0xF7, 0xE3, 0xC7, 0x23, 0x19, 0xD6, 0xBF, 0xD7, 0x20, 0x21, 0x7D, 0x14, 0xAE, 0x63, 0xDD, 0x11
            //0x1F, 0xE1, 0x68, 0x20, 0xB5, 0x86, 0xE2, 0xE5, 0x3B, 0xF6, 0x7B, 0x4F, 0x6F, 0xFA, 0x83, 0xDF
        };

        if (!streamingAwbToc.empty())
        {
            // TODO: Generate MD5 hash from awb and store it in streamAwbHash
        }

        const auto streamAwbHashDataPos = stream.tell();

        if (!streamingAwbToc.empty() && version >= packed_version(1, 26, 00)) // TODO: Is this the correct version??
        {
            write_stream_awb_hash_table_(wp, streamAwbHash);
        }
        else
        {
            stream.write_as(streamAwbHash);
        }

        ts.fill_buffer_cell(streamAwbHashBufId, streamAwbHashDataPos);
        stream.pad(32);
    }

    // TODO

    // Write StringValueTable.
    if (!stringValues.empty())
    {
        const auto stringValTableDataPos = stream.tell();
        write_string_value_table_(wp);
        ts.fill_buffer_cell(stringValTableBufId, stringValTableDataPos);
    }

    // TODO

    // Write AcfReferenceTable.
    if (version >= cue_sheet_revisions_[2].version)
    {
        const auto acfRefTableDataPos = stream.tell();
        write_acf_reference_table_(wp);
        ts.fill_buffer_cell(acfRefTableBufId, acfRefTableDataPos);
    }

    // TODO

    // Write TrackCommandTable.
    if (!wp.useGlobalCmdTable && !trackCmdTables.empty())
    {
        const auto trackCmdTableDataPos = stream.tell();

        write_command_utf_tables_(
            cmdTs,
            "TrackCommand",
            encoding,
            trackCmdTables
        );

        ts.fill_buffer_cell(trackCmdTableBufId, trackCmdTableDataPos);
    }

    // Write SynthCommandTable.
    if (!wp.useGlobalCmdTable && !synthCmdTables.empty())
    {
        const auto synthCmdTableDataPos = stream.tell();

        write_command_utf_tables_(
            cmdTs,
            "SynthCommand",
            encoding,
            synthCmdTables
        );

        ts.fill_buffer_cell(synthCmdTableBufId, synthCmdTableDataPos);
    }

    // Write TrackEventTable.
    if (!wp.useGlobalCmdTable && !trackEventCmdTables.empty())
    {
        const auto trackEventCmdTableDataPos = stream.tell();

        write_command_utf_tables_(
            cmdTs,
            "TrackEvent",
            encoding,
            trackEventCmdTables
        );

        ts.fill_buffer_cell(trackEventCmdTableBufId, trackEventCmdTableDataPos);
    }

    // TODO

    // Write streaming awb toc data.
    if (!streamingAwbToc.empty())
    {
        // Write StreamAwbTocWork.
        const auto streamAwbTocWorkDataPos = stream.tell();

        stream.write_nulls(
            (version >= cue_sheet_revisions_[3].version) ?
            0x850 : 0x800
        );

        ts.fill_buffer_cell(streamAwbTocWorkBufId, streamAwbTocWorkDataPos);
        stream.pad(32);

        // Write StreamAwbAfs2Header.
        const auto streamAwbAfs2HeaderDataPos = stream.tell();

        if (version >= cue_sheet_revisions_[3].version)
        {
            write_stream_awb_header_table_(wp, streamingAwbToc);
        }
        else
        {
            stream.write_as(streamingAwbToc.data(), streamingAwbToc.size());
        }

        ts.fill_buffer_cell(streamAwbAfs2HeaderBufId, streamAwbAfs2HeaderDataPos);
        stream.pad(32);
    }

    // Finish writing table.
    ts.finish();
}

cue_sheet::cue_sheet(rad::stream& stream)
{
    read(stream);
}
}
