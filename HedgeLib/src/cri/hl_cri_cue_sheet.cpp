#include "hedgelib/cri/hl_cri_cue_sheet.h"
#include "hedgelib/common/io/hl_endian_readers.h"
#include <rad/rad_memory_stream.h>

namespace hl::cri_new::audio
{
static const utf::column_info cue_sheet_columns_[] =
{
    { utf::cell_type::string, "name" },
    { utf::cell_type::u8, "ttype" },
    { utf::cell_type::buffer, "utf" },
};

const utf::column_info graph_point_columns[] =
{
    { utf::cell_type::u16, "in" },
    { utf::cell_type::u16, "out" },
};

const utf::column_info graph_columns[] =
{
    { utf::cell_type::u8, "type" },
    { utf::cell_type::f32, "imax" },
    { utf::cell_type::f32, "imin" },
    { utf::cell_type::f32, "omax" },
    { utf::cell_type::f32, "omin" },
    { utf::cell_type::buffer, "points" },
};

const utf::column_info aisac_columns[] =
{
    // r0 columns
    
    // group: 0, index: 0, count: 4
    { utf::cell_type::string, "name" },
    { utf::cell_type::string, "ptname" },
    { utf::cell_type::u8, "type" },
    { utf::cell_type::buffer, "grph" },

    // r1 columns
    // (First seen in Sonic Unleashed)

    // group: 1, index: 4, count: 1
    { utf::cell_type::u8, "rndrng" },
};

static constexpr utf::column_info_range aisac_columns_r0_[] =
{
    { 0, 4 },   // name - grph
};

static constexpr utf::column_info_range aisac_columns_r1_[] =
{
    { 0, 5 },   // name - rndrng
};

static constexpr revision_info aisac_revisions_[] =
{
    { aisac_columns_r0_ },
    { aisac_columns_r1_ },
};

const rad::span<const revision_info> aisac_revisions = aisac_revisions_;

const utf::column_info sound_element_columns[] =
{
    // r0 columns

    // group: 0, index: 0, count: 6
    { utf::cell_type::string, "name" },
    { utf::cell_type::buffer, "data" },
    { utf::cell_type::u8, "fmt" },
    { utf::cell_type::u8, "nch" },
    { utf::cell_type::u8, "stmflg" },
    { utf::cell_type::u32, "sfreq" },

    // r1 columns
    // (First seen in Sonic Unleashed)

    // group: 1, index: 6, count: 1
    { utf::cell_type::u32, "nsmpl" },
};

static constexpr utf::column_info_range sound_element_columns_r0_[] =
{
    { 0, 6 },   // name - sfreq
};

static constexpr utf::column_info_range sound_element_columns_r1_[] =
{
    { 0, 7 },   // name - nsmpl
};

static constexpr revision_info sound_element_revisions_[] =
{
    { sound_element_columns_r0_ },
    { sound_element_columns_r1_ },
};

const rad::span<const revision_info> sound_element_revisions = sound_element_revisions_;

const utf::column_info synth_columns[] =
{
    // r0 columns

    // group: 0, index: 0, count: 7
    { utf::cell_type::string, "synname" },
    { utf::cell_type::u8, "syntype" },
    { utf::cell_type::u8, "cmplxtype" },
    { utf::cell_type::string, "lnkname" },
    { utf::cell_type::string, "issetname" },
    { utf::cell_type::s16, "volume" },
    { utf::cell_type::s16, "pitch" },

    // group: 1, index: 7, count: 1
    { utf::cell_type::u16, "dlytim" },

    // group: 2, index: 8, count: 14
    { utf::cell_type::u8, "s_cntrl" },
    { utf::cell_type::u16, "eg_dly" },
    { utf::cell_type::u16, "eg_atk" },
    { utf::cell_type::u16, "eg_hld" },
    { utf::cell_type::u16, "eg_dcy" },
    { utf::cell_type::u16, "eg_rel" },
    { utf::cell_type::u16, "eg_sus" },
    { utf::cell_type::u8, "f_type" },
    { utf::cell_type::u16, "f_cof1" },
    { utf::cell_type::u16, "f_cof2" },
    { utf::cell_type::u16, "f_reso" },
    { utf::cell_type::u8, "f_roff" },
    { utf::cell_type::string, "dryoname" },
    { utf::cell_type::string, "mtxrtr" },

    // group: 3, index: 22, count: 8
    { utf::cell_type::u8, "dry0" },
    { utf::cell_type::u8, "dry1" },
    { utf::cell_type::u8, "dry2" },
    { utf::cell_type::u8, "dry3" },
    { utf::cell_type::u8, "dry4" },
    { utf::cell_type::u8, "dry5" },
    { utf::cell_type::u8, "dry6" },
    { utf::cell_type::u8, "dry7" },

    // group: 4, index: 30, count: 1
    { utf::cell_type::string, "wetoname" },

    // group: 5, index: 31, count: 8
    { utf::cell_type::u8, "wet0" },
    { utf::cell_type::u8, "wet1" },
    { utf::cell_type::u8, "wet2" },
    { utf::cell_type::u8, "wet3" },
    { utf::cell_type::u8, "wet4" },
    { utf::cell_type::u8, "wet5" },
    { utf::cell_type::u8, "wet6" },
    { utf::cell_type::u8, "wet7" },

    // group: 6, index: 39, count: 13
    { utf::cell_type::string, "wcnct0" },
    { utf::cell_type::string, "wcnct1" },
    { utf::cell_type::string, "wcnct2" },
    { utf::cell_type::string, "wcnct3" },
    { utf::cell_type::string, "wcnct4" },
    { utf::cell_type::string, "wcnct5" },
    { utf::cell_type::string, "wcnct6" },
    { utf::cell_type::string, "wcnct7" },
    { utf::cell_type::string, "vl_gname" },
    { utf::cell_type::u8, "vl_type" },
    { utf::cell_type::u8, "vl_prio" },
    { utf::cell_type::u16, "vl_phtime" },
    { utf::cell_type::s8, "vl_pcdlt" },

    // r1 columns
    // (First seen in Sonic Unleashed)

    // group: 7, index: 52, count: 1
    { utf::cell_type::u32, "dlytim" },

    // group: 8, index: 53, count: 8
    { utf::cell_type::u16, "dry0" },
    { utf::cell_type::u16, "dry1" },
    { utf::cell_type::u16, "dry2" },
    { utf::cell_type::u16, "dry3" },
    { utf::cell_type::u16, "dry4" },
    { utf::cell_type::u16, "dry5" },
    { utf::cell_type::u16, "dry6" },
    { utf::cell_type::u16, "dry7" },

    // group: 9, index: 61, count: 8
    { utf::cell_type::u16, "wet0" },
    { utf::cell_type::u16, "wet1" },
    { utf::cell_type::u16, "wet2" },
    { utf::cell_type::u16, "wet3" },
    { utf::cell_type::u16, "wet4" },
    { utf::cell_type::u16, "wet5" },
    { utf::cell_type::u16, "wet6" },
    { utf::cell_type::u16, "wet7" },

    // group: 10, index: 69, count: 32
    { utf::cell_type::s16, "p3d_vo" },
    { utf::cell_type::s16, "p3d_vg" },
    { utf::cell_type::s16, "p3d_ao" },
    { utf::cell_type::s16, "p3d_ag" },
    { utf::cell_type::s16, "p3d_ido" },
    { utf::cell_type::s16, "p3d_idg" },
    { utf::cell_type::u8, "dry0g" },
    { utf::cell_type::u8, "dry1g" },
    { utf::cell_type::u8, "dry2g" },
    { utf::cell_type::u8, "dry3g" },
    { utf::cell_type::u8, "dry4g" },
    { utf::cell_type::u8, "dry5g" },
    { utf::cell_type::u8, "dry6g" },
    { utf::cell_type::u8, "dry7g" },
    { utf::cell_type::u8, "wet0g" },
    { utf::cell_type::u8, "wet1g" },
    { utf::cell_type::u8, "wet2g" },
    { utf::cell_type::u8, "wet3g" },
    { utf::cell_type::u8, "wet4g" },
    { utf::cell_type::u8, "wet5g" },
    { utf::cell_type::u8, "wet6g" },
    { utf::cell_type::u8, "wet7g" },
    { utf::cell_type::u8, "f1_type" },
    { utf::cell_type::u16, "f1_cofo" },
    { utf::cell_type::u16, "f1_cofg" },
    { utf::cell_type::u16, "f1_resoo" },
    { utf::cell_type::u16, "f1_resog" },
    { utf::cell_type::u8, "f2_type" },
    { utf::cell_type::u16, "f2_coflo" },
    { utf::cell_type::u16, "f2_coflg" },
    { utf::cell_type::u16, "f2_cofho" },
    { utf::cell_type::u16, "f2_cofhg" },

    // r2 columns
    // (First seen in Sonic Colors)

    // group: 11, index: 101, count: 3
    { utf::cell_type::u8, "probability" },
    { utf::cell_type::u8, "n_lmt_children" },
    { utf::cell_type::u8, "repeat" },

    // r3 columns
    // (First seen in Sonic Generations 2011)

    // group: 12, index: 104, count: 2
    { utf::cell_type::u32, "combo_time" },
    { utf::cell_type::u8, "combo_loop_back" },
};

static constexpr utf::column_info_range synth_columns_r0_[] =
{
    { 0, 52 },  // name - vl_pcdlt
};

static constexpr utf::column_info_range synth_columns_r1_[] =
{
    { 0, 7 },   // name - pitch
    { 52, 1 },  // dlytim
    { 8, 14 },  // s_cntrl - mtxrtr
    { 53, 8 },  // dry0 - dry7
    { 30, 1 },  // wetoname
    { 61, 8 },  // wet0 - wet7
    { 39, 13 }, // wcnct0 - vl_pcdlt
    { 69, 32 }, // p3d_vo - f2_cofhg
};

static constexpr utf::column_info_range synth_columns_r2_[] =
{
    { 0, 7 },   // name - pitch
    { 52, 1 },  // dlytim
    { 8, 14 },  // s_cntrl - mtxrtr
    { 53, 8 },  // dry0 - dry7
    { 30, 1 },  // wetoname
    { 61, 8 },  // wet0 - wet7
    { 39, 13 }, // wcnct0 - vl_pcdlt
    { 69, 35 }, // p3d_vo - repeat
};

static constexpr utf::column_info_range synth_columns_r3_[] =
{
    { 0, 7 },   // name - pitch
    { 52, 1 },  // dlytim
    { 8, 14 },  // s_cntrl - mtxrtr
    { 53, 8 },  // dry0 - dry7
    { 30, 1 },  // wetoname
    { 61, 8 },  // wet0 - wet7
    { 39, 13 }, // wcnct0 - vl_pcdlt
    { 69, 37 }, // p3d_vo - combo_loop_back
};

static constexpr revision_info synth_revisions_[] =
{
    { synth_columns_r0_ },
    { synth_columns_r1_ },
    { synth_columns_r2_ },
    { synth_columns_r3_ },
};

const rad::span<const revision_info> synth_revisions = synth_revisions_;

const utf::column_info cue_columns[] =
{
    // r0 columns

    // group: 0, index: 0, count: 4
    { utf::cell_type::string, "name" },
    { utf::cell_type::u32, "id" },
    { utf::cell_type::string, "synth" },
    { utf::cell_type::string, "udata" },

    // r1 columns
    // (First seen in Sonic Unleashed)

    // group: 0, index: 4, count: 1
    { utf::cell_type::u8, "flags" },
};

static constexpr utf::column_info_range cue_columns_r0_[] =
{
    { 0, 4 },   // name - udata
};

static constexpr utf::column_info_range cue_columns_r1_[] =
{
    { 0, 5 },   // name - flags
};

static constexpr revision_info cue_revisions_[] =
{
    { cue_columns_r0_ },
    { cue_columns_r1_ },
};

const rad::span<const revision_info> cue_revisions = cue_revisions_;

const revision_info* get_revision_info(
    const utf::table_deserializer& td,
    rad::span<const revision_info> revisionInfos) noexcept
{
    const revision_info* result = nullptr;

    for (const auto& revisionInfo : revisionInfos)
    {
        if (td.column_count() < revisionInfo.total_column_count()) break;

        result = &revisionInfo;
    }
    
    return result;
}

sound_element_format parse_sound_element_format(u8 val)
{
    if (val != SOUND_ELEMENT_FORMAT_AAX)
    {
        throw std::runtime_error("Unsupported sound element format");
    }

    return static_cast<sound_element_format>(val);
}

u32 sound_element::compute_play_duration() const noexcept
{
    return static_cast<u32>((static_cast<double>(sampleCount) / sampleRate) * 1000.0);
}

synth_link_type parse_synth_link_type(u8 val)
{
    if (val != SYNTH_LINK_TYPE_SOUND_ELEMENT &&
        val != SYNTH_LINK_TYPE_SYNTH)
    {
        throw std::runtime_error("Unsupported synth type");
    }

    return static_cast<synth_link_type>(val);
}

synth_complex_type parse_synth_complex_type(u8 val)
{
    if (val != SYNTH_COMPLEX_TYPE_POLYPHONIC &&
        val != SYNTH_COMPLEX_TYPE_RANDOM_NO_REPEAT &&
        val != SYNTH_COMPLEX_TYPE_SEQUENTIAL &&
        val != SYNTH_COMPLEX_TYPE_RANDOM &&
        val != SYNTH_COMPLEX_TYPE_SEQUENTIAL_NO_LOOP)
    {
        throw std::runtime_error("Unsupported synth complex type");
    }

    return static_cast<synth_complex_type>(val);
}

u32 synth::compute_play_duration(const cue_sheet& cueSheet) const
{
    auto synth = this;
    u32 longestPlayDuration = 0;

    // TODO: Account for synth properties, such as delayTime??

    if (linkType == SYNTH_LINK_TYPE_SOUND_ELEMENT)
    {
        for (const auto& nodeLinkName : nodeLinkNames)
        {
            const auto& soundElement = cueSheet.soundElements.at(nodeLinkName);
            const auto playDuration = soundElement.compute_play_duration();

            if (playDuration > longestPlayDuration)
            {
                longestPlayDuration = playDuration;
            }
        }
    }
    else if (SYNTH_LINK_TYPE_SYNTH)
    {
        for (const auto nodeLinkName : nodeLinkNames)
        {
            const auto& synth = cueSheet.synths.at(nodeLinkName);
            const auto playDuration = synth.compute_play_duration(cueSheet);

            if (playDuration > longestPlayDuration)
            {
                longestPlayDuration = playDuration;
            }
        }
    }
    else
    {
        throw std::runtime_error("Unknown synth link type");
    }

    return longestPlayDuration;
}

void validate_cue_flags(u8 val)
{
    if (val != CUE_FLAGS_NONE &&
        val != CUE_FLAGS_DOES_LOOP)
    {
        throw std::runtime_error("Unsupported cue flags");
    }
}

const synth& cue::get_synth(const cue_sheet& cueSheet) const
{
    return cueSheet.synths.at(synthName);
}

u32 cue::compute_play_duration(const cue_sheet& cueSheet) const
{
    const auto& synth = get_synth(cueSheet);
    return synth.compute_play_duration(cueSheet);
}

cue::cue(
    u32 id,
    rad::string synthName,
    u8 flags,
    rad::optional_string name,
    rad::optional_string userData) noexcept
    : id(id)
    , synthName(std::move(synthName))
    , flags(flags)
    , name(std::move(name))
    , userData(std::move(userData))
{
}

static rad::vector<graph_point> read_graph_point_table_(
    rad::stream& stream,
    rad::allocator& allocator = rad::default_allocator)
{
    utf::table_deserializer td(stream);

    // Get table revision and validate columns.
    const auto graphPointColumnCount = static_cast<u16>(std::size(graph_point_columns));

    if (td.column_count() < graphPointColumnCount || !td.are_columns_exact_types(
        graph_point_columns,
        0,
        static_cast<u16>(std::size(graph_point_columns))))
    {
        throw std::runtime_error("Invalid or unsupported TBLIPT layout");
    }

    // Read rows.
    rad::vector<graph_point> points(allocator);
    points.reserve(td.row_count());

    for (u32 i = 0; i < td.row_count(); ++i)
    {
        // Read row.
        const auto in = td.read_cell_as_u16();
        const auto out = td.read_cell_as_u16();

        td.next_row();

        // Generate graph.
        points.emplace_back(in, out);
    }

    return points;
}

static graph_type parse_graph_type_(u8 val)
{
    if (val != static_cast<u8>(graph_type::volume) &&
        val != static_cast<u8>(graph_type::bandpass_cutoff_low) &&
        val != static_cast<u8>(graph_type::bandpass_cutoff_high) &&
        val != static_cast<u8>(graph_type::bus_send_0) &&
        val != static_cast<u8>(graph_type::bus_send_1))
    {
        throw std::runtime_error("Unsupported graph type");
    }

    return static_cast<graph_type>(val);
}

static rad::vector<graph> read_graph_table_(
    rad::stream& stream,
    rad::allocator& allocator = rad::default_allocator)
{
    utf::table_deserializer td(stream);

    // Get table revision and validate columns.
    const auto graphColumnCount = static_cast<u16>(std::size(graph_columns));

    if (td.column_count() < graphColumnCount || !td.are_columns_exact_types(
        graph_columns,
        0,
        static_cast<u16>(std::size(graph_columns))))
    {
        throw std::runtime_error("Invalid or unsupported TBLIGR layout");
    }

    // Read rows.
    rad::vector<graph> graphs(allocator);
    graphs.reserve(td.row_count());

    for (u32 i = 0; i < td.row_count(); ++i)
    {
        // Read row.
        const auto type = td.read_cell_as_u8();
        const auto imax = td.read_cell_as_f32();
        const auto imin = td.read_cell_as_f32();
        const auto omax = td.read_cell_as_f32();
        const auto omin = td.read_cell_as_f32();
        const auto rawPoints = td.read_cell_as_buffer();

        td.next_row();

        // Generate graph.
        auto& graph = graphs.emplace_back(
            parse_graph_type_(type),
            imax,
            imin,
            omax,
            omin
        );

        if (rawPoints.size)
        {
            const auto curPos = stream.tell();
            stream.jump_to(td.get_buffer_data_position(rawPoints));

            graph.points = read_graph_point_table_(stream, allocator);

            stream.jump_to(curPos);
        }
    }

    return graphs;
}

static aisac_type parse_aisac_type_(u8 val)
{
    if (val != static_cast<u8>(aisac_type::simple))
    {
        throw std::runtime_error("Unsupported AISAC type");
    }

    return static_cast<aisac_type>(val);
}

void cue_sheet::read_aisac_table_(rad::stream& stream)
{
    utf::table_deserializer td(stream);

    // Get table revision and validate columns.
    const auto revisionInfo = get_revision_info(td, aisac_revisions);

    if (!revisionInfo || !revisionInfo->columns().validate(aisac_columns, td))
    {
        throw std::runtime_error("Invalid or unsupported TBLISC layout");
    }

    // Read rows.
    aisacs.reserve(td.row_count());

    for (u32 i = 0; i < td.row_count(); ++i)
    {
        // Read row.
        const auto rawName = td.read_cell_as_string();
        const auto rawPtPath = td.read_cell_as_string();
        const auto type = td.read_cell_as_u8();
        const auto rawGrph = td.read_cell_as_buffer();

        // r1 columns
        const u8 rndrng = (
            (revisionInfo->columns().ranges.data() == sound_element_columns_r0_) ?
            0 : td.read_cell_as_u8()
        );

        td.next_row();

        // Generate AISAC.
        const auto p = aisacs.try_emplace(
            td.get_string_data(rawPtPath)
        );

        if (!p.second)
        {
            // Skip any AISACS with duplicate paths.
            // TODO: Log warning about this.
            continue;
        }

        auto& aisac = p.first->second;

        aisac.controlName = td.get_string_data(rawName);
        aisac.type = parse_aisac_type_(type);
        aisac.randomRange = rndrng;

        if (rawGrph.size)
        {
            const auto curPos = stream.tell();
            stream.jump_to(td.get_buffer_data_position(rawGrph));

            aisac.graphs = read_graph_table_(stream); // TODO: Pass allocator

            stream.jump_to(curPos);
        }
    }
}

void cue_sheet::read_sound_element_table_(rad::stream& stream)
{
    utf::table_deserializer td(stream);

    // Get table revision and validate columns.
    const auto revisionInfo = get_revision_info(td, sound_element_revisions);

    if (!revisionInfo || !revisionInfo->columns().validate(sound_element_columns, td))
    {
        throw std::runtime_error("Invalid or unsupported TBLSDL layout");
    }

    // Read rows.
    soundElements.reserve(td.row_count());

    for (u32 i = 0; i < td.row_count(); ++i)
    {
        // Read row.
        const auto rawName = td.read_cell_as_string();
        const auto rawData = td.read_cell_as_buffer();
        const auto fmt = td.read_cell_as_u8();
        const auto nch = td.read_cell_as_u8();
        const auto stmflg = td.read_cell_as_u8();
        const auto sfreq = td.read_cell_as_u32();

        // r1 columns
        const u32 nsmpl = (
            (revisionInfo->columns().ranges.data() == sound_element_columns_r0_) ?
            0 : td.read_cell_as_u32()
        );

        td.next_row();

        // Generate sound element.
        const auto p = soundElements.try_emplace(
            td.get_string_data(rawName)
        );

        if (!p.second)
        {
            // Skip any sound elements with duplicate names.
            // TODO: Log warning about this.
            continue;
        }

        auto& soundElement = p.first->second;

        soundElement.format = parse_sound_element_format(fmt);
        soundElement.channelCount = nch;

        if (stmflg != 0 && stmflg != 1)
        {
            throw std::runtime_error("Unsupported stream flags");
        }

        if (!stmflg)
        {
            auto embeddedData = td.get_buffer_data(rawData);
            if (embeddedData.empty())
            {
                throw std::runtime_error("Missing sound element data");
            }

            soundElement.embeddedData = std::move(embeddedData);
        }

        soundElement.sampleRate = sfreq;
        soundElement.sampleCount = nsmpl;
    }
}

void cue_sheet::read_synth_table_(rad::stream& stream)
{
    utf::table_deserializer td(stream);

    // Get table revision and validate columns.
    const auto revisionInfo = get_revision_info(td, synth_revisions);

    if (!revisionInfo || !revisionInfo->columns().validate(synth_columns, td))
    {
        throw std::runtime_error("Invalid or unsupported TBLSYN layout");
    }

    // Read rows.
    synths.reserve(td.row_count());

    for (u32 i = 0; i < td.row_count(); ++i)
    {
        // Read row.
        const auto rawSynname = td.read_cell_as_string();
        const auto syntype = td.read_cell_as_u8();
        const auto cmplxtype = td.read_cell_as_u8();
        const auto rawLnkname = td.read_cell_as_string();
        const auto rawIssetname = td.read_cell_as_string();
        const auto volume = td.read_cell_as_s16();
        const auto pitch = td.read_cell_as_s16();
        const auto dlytim = td.read_cell_as_u32();
        const auto sCntrl = td.read_cell_as_u8();
        const auto egDly = td.read_cell_as_u16();
        const auto egAtk = td.read_cell_as_u16();
        const auto egHld = td.read_cell_as_u16();
        const auto egDcy = td.read_cell_as_u16();
        const auto egRel = td.read_cell_as_u16();
        const auto egSus = td.read_cell_as_u16();
        const auto fType = td.read_cell_as_u8();
        const auto fCof1 = td.read_cell_as_u16();
        const auto fCof2 = td.read_cell_as_u16();
        const auto fReso = td.read_cell_as_u16();
        const auto fRoff = td.read_cell_as_u8();
        const auto rawDryoname = td.read_cell_as_string();
        const auto rawMtxrtr = td.read_cell_as_string();
        const auto dry0 = td.read_cell_as_u16();
        const auto dry1 = td.read_cell_as_u16();
        const auto dry2 = td.read_cell_as_u16();
        const auto dry3 = td.read_cell_as_u16();
        const auto dry4 = td.read_cell_as_u16();
        const auto dry5 = td.read_cell_as_u16();
        const auto dry6 = td.read_cell_as_u16();
        const auto dry7 = td.read_cell_as_u16();
        const auto rawWetoname = td.read_cell_as_string();
        const auto wet0 = td.read_cell_as_u16();
        const auto wet1 = td.read_cell_as_u16();
        const auto wet2 = td.read_cell_as_u16();
        const auto wet3 = td.read_cell_as_u16();
        const auto wet4 = td.read_cell_as_u16();
        const auto wet5 = td.read_cell_as_u16();
        const auto wet6 = td.read_cell_as_u16();
        const auto wet7 = td.read_cell_as_u16();
        const auto rawWcnct0 = td.read_cell_as_string();
        const auto rawWcnct1 = td.read_cell_as_string();
        const auto rawWcnct2 = td.read_cell_as_string();
        const auto rawWcnct3 = td.read_cell_as_string();
        const auto rawWcnct4 = td.read_cell_as_string();
        const auto rawWcnct5 = td.read_cell_as_string();
        const auto rawWcnct6 = td.read_cell_as_string();
        const auto rawWcnct7 = td.read_cell_as_string();
        const auto rawVlGname = td.read_cell_as_string();
        const auto vlType = td.read_cell_as_u8();
        const auto vlPrio = td.read_cell_as_u8();
        const auto vlPhtime = td.read_cell_as_u16();
        const auto vlPcdlt = td.read_cell_as_s8();

        s16 p3dVo = 0;
        s16 p3dVg = 0;
        s16 p3dAo = 0;
        s16 p3dAg = 0;
        s16 p3dIdo = 0;
        s16 p3dIdg = 0;
        u8 dry0g = 0;
        u8 dry1g = 0;
        u8 dry2g = 0;
        u8 dry3g = 0;
        u8 dry4g = 0;
        u8 dry5g = 0;
        u8 dry6g = 0;
        u8 dry7g = 0;
        u8 wet0g = 0;
        u8 wet1g = 0;
        u8 wet2g = 0;
        u8 wet3g = 0;
        u8 wet4g = 0;
        u8 wet5g = 0;
        u8 wet6g = 0;
        u8 wet7g = 0;
        u8 f1Type = 0;
        u16 f1Cofo = 0;
        u16 f1Cofg = 0;
        u16 f1Resoo = 0;
        u16 f1Resog = 0;
        u8 f2Type = 0;
        u16 f2Coflo = 0;
        u16 f2Coflg = 0;
        u16 f2Cofho = 0;
        u16 f2Cofhg = 0;
        u8 probability = 0;
        u8 nLmtChildren = 0;
        u8 repeat = 0;
        u32 comboTime = 0;
        u8 comboLoopBack = 0;

        // r1 columns
        if (revisionInfo->columns().ranges.data() != synth_columns_r0_)
        {
            p3dVo = td.read_cell_as_s16();
            p3dVg = td.read_cell_as_s16();
            p3dAo = td.read_cell_as_s16();
            p3dAg = td.read_cell_as_s16();
            p3dIdo = td.read_cell_as_s16();
            p3dIdg = td.read_cell_as_s16();
            dry0g = td.read_cell_as_u8();
            dry1g = td.read_cell_as_u8();
            dry2g = td.read_cell_as_u8();
            dry3g = td.read_cell_as_u8();
            dry4g = td.read_cell_as_u8();
            dry5g = td.read_cell_as_u8();
            dry6g = td.read_cell_as_u8();
            dry7g = td.read_cell_as_u8();
            wet0g = td.read_cell_as_u8();
            wet1g = td.read_cell_as_u8();
            wet2g = td.read_cell_as_u8();
            wet3g = td.read_cell_as_u8();
            wet4g = td.read_cell_as_u8();
            wet5g = td.read_cell_as_u8();
            wet6g = td.read_cell_as_u8();
            wet7g = td.read_cell_as_u8();
            f1Type = td.read_cell_as_u8();
            f1Cofo = td.read_cell_as_u16();
            f1Cofg = td.read_cell_as_u16();
            f1Resoo = td.read_cell_as_u16();
            f1Resog = td.read_cell_as_u16();
            f2Type = td.read_cell_as_u8();
            f2Coflo = td.read_cell_as_u16();
            f2Coflg = td.read_cell_as_u16();
            f2Cofho = td.read_cell_as_u16();
            f2Cofhg = td.read_cell_as_u16();

            // r2 columns
            if (revisionInfo->columns().ranges.data() != synth_columns_r1_)
            {
                probability = td.read_cell_as_u8();
                nLmtChildren = td.read_cell_as_u8();
                repeat = td.read_cell_as_u8();

                // r3 columns
                if (revisionInfo->columns().ranges.data() != synth_columns_r2_)
                {
                    comboTime = td.read_cell_as_u32();
                    comboLoopBack = td.read_cell_as_u8();
                }
            }
        }

        td.next_row();

        // Generate synth.
        const auto p = synths.try_emplace(td.get_string_data(rawSynname));

        if (!p.second)
        {
            // Skip duplicate synth names.

            // NOTE: Some cue sheets used in mods, such as "bb/cmn200/42OBJ_CMN.csb" from
            // "Sonic Unleashed Jason Griffith Voice v4.0" for Sonic Generations (2011),
            // have multiple synths of the same name, which are exact duplicates.

            // TODO: Log warning about how duplicate synth name was skipped.
            continue;
        }

        auto& synth = p.first->second;

        synth.linkType = parse_synth_link_type(syntype);
        synth.complexType = parse_synth_complex_type(cmplxtype);

        // Parse node link names.
        if (syntype == SYNTH_LINK_TYPE_SYNTH)
        {
            // Iterate through each line in the string.
            const auto multiLinkName = td.get_string_data(rawLnkname);
            const char* curLinkName = multiLinkName.data();
            auto linkNamesSize = multiLinkName.size();

            while (true)
            {
                const auto curLinkNameEnd = static_cast<const char*>(
                    std::memchr(curLinkName, '\n', linkNamesSize)
                );

                if (curLinkNameEnd)
                {
                    // Add the current line to the node link names.
                    const auto curLinkNameSize = (curLinkNameEnd - curLinkName);
                    synth.nodeLinkNames.emplace_back(
                        rad::string(curLinkName, curLinkNameSize)
                    );

                    curLinkName = curLinkNameEnd + 1;
                    linkNamesSize -= curLinkNameSize;
                }
                else break;
            }
        }
        else
        {
            synth.nodeLinkNames.emplace_back(td.get_string_data(rawLnkname));
        }

        // Parse AISAC names.
        if (rawIssetname.dataOff)
        {
            // Iterate through each line in the string.
            const auto isSetName = td.get_string_data(rawIssetname);
            const char* curIsSetName = isSetName.data();
            auto isSetNamesSize = isSetName.size();

            while (true)
            {
                const auto curIsSetNameEnd = static_cast<const char*>(
                    std::memchr(curIsSetName, '\n', isSetNamesSize)
                );

                if (curIsSetNameEnd)
                {
                    // Ensure the current line contains the substring "::".
                    const auto curIsSetNameSize = (curIsSetNameEnd - curIsSetName);
                    const auto sepChPtr = static_cast<const char*>(
                        std::memchr(curIsSetName, ':', curIsSetNameSize)
                    );

                    if (!sepChPtr || sepChPtr[1] != ':')
                    {
                        throw std::runtime_error("Invalid CSB issetname");
                    }

                    // Add the AISAC name after the "::" substring.
                    const auto aisacNamePtr = (sepChPtr + 2);
                    const auto aisacNameSize = (curIsSetNameEnd - aisacNamePtr);

                    synth.aisacNames.emplace_back(
                        rad::string(aisacNamePtr, aisacNameSize)
                    );

                    curIsSetName = curIsSetNameEnd + 1;
                    isSetNamesSize -= curIsSetNameSize;
                }
                else break;
            }
        }

        synth.volume = volume;
        synth.pitch = pitch;
        synth.delayTime = dlytim;
        synth.sControl = sCntrl;
        synth.egDelay = egDly;
        synth.egAttack = egAtk;
        synth.egHold = egHld;
        synth.egDecay = egDcy;
        synth.egRelease = egRel;
        synth.egSuspend = egSus;

        // TODO: Parse other row fields.
    }
}

void cue_sheet::read_cue_table_(rad::stream& stream)
{
    utf::table_deserializer td(stream);

    // Get table revision and validate columns.
    const auto revisionInfo = get_revision_info(td, cue_revisions);

    if (!revisionInfo || !revisionInfo->columns().validate(cue_columns, td))
    {
        throw std::runtime_error("Invalid or unsupported TBLCUE layout");
    }

    // Read rows.
    cues.reserve(td.row_count());

    for (u32 i = 0; i < td.row_count(); ++i)
    {
        // Read row.
        const auto rawName = td.read_cell_as_string();
        const auto id = td.read_cell_as_u32();
        const auto rawSynth = td.read_cell_as_string();
        const auto rawUdata = td.read_cell_as_string();

        const u8 flags = (
            (revisionInfo->columns().ranges.data() == cue_columns_r0_) ?
            0 : td.read_cell_as_u8()
        );

        td.next_row();

        // Validate flags.
        validate_cue_flags(flags);

        // Generate cue.
        cues.emplace_back(
            id,
            td.get_string_data(rawSynth),
            flags,
            td.get_string_data(rawName),
            td.get_string_data(rawUdata)
        );
    }
}

void cue_sheet::clear() noexcept
{
    cues.clear();
    synths.clear();
    soundElements.clear();
    aisacs.clear();
}

void cue_sheet::read_inner(rad::stream& stream)
{
    clear();

    utf::table_deserializer td(stream, utf::table_deserialize_type::inner_table);

    // Validate columns.
    if (td.column_count() == 3 && !td.are_columns_exact_types(
        cue_sheet_columns_, 0, 3))
    {
        throw std::runtime_error("Invalid or unsupported TBLCSB layout");
    }

    // Read rows.
    enum raw_csb_table_type_ : u8
    {
        CSB_TABLE_TYPE_NONE = 0,
        CSB_TABLE_TYPE_CUE = 1,
        CSB_TABLE_TYPE_SYNTH = 2,
        CSB_TABLE_TYPE_SOUND_ELEMENT = 4,
        CSB_TABLE_TYPE_ISAAC = 5,
        CSB_TABLE_TYPE_VOICE_LIMIT_GROUP = 6,
        CSB_TABLE_TYPE_INFO = 7,
        CSB_TABLES_COUNT
    };

    utf::raw_buffer rawTables[CSB_TABLES_COUNT] = {};

    for (u32 i = 0; i < td.row_count(); ++i)
    {
        // Read row.
        const auto rawName = td.read_cell_as_string();
        const auto ttype = td.read_cell_as_u8();
        const auto utf = td.read_cell_as_buffer();

        td.next_row();

        // Store raw table information for later, based on table type.

        // NOTE: We compare using name because it's what the actual games
        // seem to use; they do NOT seem to utilize the ttype column.

        const auto name = td.get_string_data(rawName);
        raw_csb_table_type_ type = CSB_TABLE_TYPE_NONE;

        if (name == "INFO")
        {
            type = CSB_TABLE_TYPE_INFO;
        }
        else if (name == "CUE")
        {
            type = CSB_TABLE_TYPE_CUE;
        }
        else if (name == "SYNTH")
        {
            type = CSB_TABLE_TYPE_SYNTH;
        }
        else if (name == "SOUND_ELEMENT")
        {
            type = CSB_TABLE_TYPE_SOUND_ELEMENT;
        }
        else if (name == "ISAAC")
        {
            type = CSB_TABLE_TYPE_ISAAC;
        }
        else if (name == "VOICE_LIMIT_GROUP")
        {
            type = CSB_TABLE_TYPE_VOICE_LIMIT_GROUP;
        }
        else
        {
            // TODO: Warn about unknown type.
            continue;
        }

        rawTables[type] = utf;
    }

    // Read tables in a specific order.
    if (rawTables[CSB_TABLE_TYPE_ISAAC].size)
    {
        stream.jump_to(td.get_buffer_data_position(
            rawTables[CSB_TABLE_TYPE_ISAAC]
        ));

        read_aisac_table_(stream);
    }

    if (rawTables[CSB_TABLE_TYPE_SOUND_ELEMENT].size)
    {
        stream.jump_to(td.get_buffer_data_position(
            rawTables[CSB_TABLE_TYPE_SOUND_ELEMENT]
        ));

        read_sound_element_table_(stream);
    }

    if (rawTables[CSB_TABLE_TYPE_SYNTH].size)
    {
        stream.jump_to(td.get_buffer_data_position(
            rawTables[CSB_TABLE_TYPE_SYNTH]
        ));

        read_synth_table_(stream);
    }

    if (rawTables[CSB_TABLE_TYPE_CUE].size)
    {
        stream.jump_to(td.get_buffer_data_position(
            rawTables[CSB_TABLE_TYPE_CUE]
        ));

        read_cue_table_(stream);
    }
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

cue_sheet::cue_sheet(rad::stream& stream)
{
    read(stream);
}
}
