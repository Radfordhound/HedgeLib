#include "hedgelib/cri/hl_cri_audio_cue_sheet.h"
#include "hedgelib/io/hl_endian_readers.h"
#include <rad/rad_memory_stream.h>

// TODO: Split this file up into multiple impl files: aisac, sound_element, synth, cue
namespace hl::cri::audio
{
class revision_info_
{
    utf::column_info_group  columnGroup_;
    u16                     totalColumnCount_;

public:
    constexpr const utf::column_info_group& column_group() const noexcept
    {
        return columnGroup_;
    }

    constexpr const utf::column_info_range* column_ranges() const noexcept
    {
        return columnGroup_.ranges.data();
    }

    constexpr u16 total_column_count() const noexcept
    {
        return totalColumnCount_;
    }

    constexpr revision_info_(
        rad::span<const utf::column_info_range> columnRanges) noexcept
        : columnGroup_{columnRanges}
        , totalColumnCount_(columnGroup_.get_column_count())
    {
    }
};

const revision_info_* get_revision_info_(
    const utf::deserializer& dr,
    rad::span<const revision_info_> revisionInfos) noexcept
{
    const revision_info_* result = nullptr;

    for (const auto& revisionInfo : revisionInfos)
    {
        if (dr.column_count() < revisionInfo.total_column_count()) break;

        result = &revisionInfo;
    }
    
    return result;
}

static const utf::column_info cue_sheet_columns_[] =
{
    { utf::cell_type::string, "name" },
    { utf::cell_type::u8, "ttype" },
    { utf::cell_type::buffer, "utf" },
};

static const utf::column_info graph_point_columns_[] =
{
    { utf::cell_type::u16, "in" },
    { utf::cell_type::u16, "out" },
};

static const utf::column_info graph_columns_[] =
{
    { utf::cell_type::u8, "type" },
    { utf::cell_type::f32, "imax" },
    { utf::cell_type::f32, "imin" },
    { utf::cell_type::f32, "omax" },
    { utf::cell_type::f32, "omin" },
    { utf::cell_type::buffer, "points" },
};

static const utf::column_info aisac_columns_[] =
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

static constexpr revision_info_ aisac_revisions_[] =
{
    { aisac_columns_r0_ },
    { aisac_columns_r1_ },
};

static const utf::column_info sound_element_columns_[] =
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

static constexpr revision_info_ sound_element_revisions_[] =
{
    { sound_element_columns_r0_ },
    { sound_element_columns_r1_ },
};

static const utf::column_info synth_columns_[] =
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

static constexpr revision_info_ synth_revisions_[] =
{
    { synth_columns_r0_ },
    { synth_columns_r1_ },
    { synth_columns_r2_ },
    { synth_columns_r3_ },
};

static const utf::column_info cue_columns_[] =
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

static constexpr revision_info_ cue_revisions_[] =
{
    { cue_columns_r0_ },
    { cue_columns_r1_ },
};

u32 sound_element::compute_play_duration() const noexcept
{
    // Ensure we never divide by zero.
    // TODO: Mark unlikely
    if (sampleRate == 0) return 0;

    return static_cast<u32>((static_cast<double>(sampleCount) / sampleRate) * 1000.0);
}

static rad::vector<graph_point> read_graph_point_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    rad::allocator& allocator)
{
    utf::deserializer dr(stream, utf::deserialize_type::utf, tmpAllocator);

    // Get table revision and validate columns.
    if (!dr.has_columns_of_exact_types(
        graph_point_columns_,
        0,
        static_cast<u16>(std::size(graph_point_columns_))))
    {
        throw std::runtime_error("Invalid or unsupported TBLIPT layout");
    }

    // Read rows.
    rad::vector<graph_point> points(allocator);
    points.reserve(dr.row_count());

    for (u32 i = 0; i < dr.row_count(); ++i)
    {
        // Read row.
        const auto in = dr.next_cell_as_u16();
        const auto out = dr.next_cell_as_u16();

        dr.next_row();

        // Generate graph.
        points.emplace_back(in, out);
    }

    return points;
}

static graph_type parse_graph_type_(u8 val)
{
    if (val != static_cast<u8>(graph_type::volume) &&
        val != static_cast<u8>(graph_type::pitch) &&
        val != static_cast<u8>(graph_type::bandpass_cutoff_low) &&
        val != static_cast<u8>(graph_type::bandpass_cutoff_high) &&
        val != static_cast<u8>(graph_type::bus_send_0) &&
        val != static_cast<u8>(graph_type::bus_send_1) &&
        val != static_cast<u8>(graph_type::unknown23) &&
        val != static_cast<u8>(graph_type::voice_priority) &&
        val != static_cast<u8>(graph_type::unknown26))
    {
        throw std::runtime_error("Unsupported CriAu graph type");
    }

    return static_cast<graph_type>(val);
}

static rad::vector<graph> read_graph_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    rad::allocator& allocator)
{
    utf::deserializer dr(stream, utf::deserialize_type::utf, tmpAllocator);

    // Get table revision and validate columns.
    if (!dr.has_columns_of_exact_types(
        graph_columns_,
        0,
        static_cast<u16>(std::size(graph_columns_))))
    {
        throw std::runtime_error("Invalid or unsupported TBLIGR layout");
    }

    // Read rows.
    rad::vector<graph> graphs(allocator);
    graphs.reserve(dr.row_count());

    for (u32 i = 0; i < dr.row_count(); ++i)
    {
        // Read row.
        const auto type = dr.next_cell_as_u8();
        const auto imax = dr.next_cell_as_f32();
        const auto imin = dr.next_cell_as_f32();
        const auto omax = dr.next_cell_as_f32();
        const auto omin = dr.next_cell_as_f32();
        const auto rawPoints = dr.next_cell_as_buffer();

        dr.next_row();

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
            stream.jump_to(dr.get_buffer_data_position(rawPoints));

            graph.points = read_graph_point_table_(
                stream,
                tmpAllocator,
                allocator
            );

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

void read_aisac_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    rad::vector<aisac>& aisacs)
{
    utf::deserializer dr(stream, utf::deserialize_type::utf, tmpAllocator);

    // Get table revision and validate columns.
    const auto revisionInfo = get_revision_info_(dr, aisac_revisions_);

    if (!revisionInfo || !dr.has_columns_of_exact_types(
        revisionInfo->column_group(), aisac_columns_))
    {
        throw std::runtime_error("Invalid or unsupported TBLISC layout");
    }

    // Read rows.
    auto& aisacAllocator = aisacs.allocator();
    aisacs.reserve(dr.row_count());

    for (u32 i = 0; i < dr.row_count(); ++i)
    {
        // Read row.
        const auto rawName = dr.next_cell_as_string();
        const auto rawPtPath = dr.next_cell_as_string();
        const auto type = dr.next_cell_as_u8();
        const auto rawGrph = dr.next_cell_as_buffer();

        // r1 columns
        const u8 rndrng = (
            (revisionInfo->column_ranges() == sound_element_columns_r0_) ?
            0 : dr.next_cell_as_u8()
        );

        dr.next_row();

        // Generate AISAC.
        auto& aisac = aisacs.emplace_back(
            rad::string(
                aisacAllocator,
                dr.get_string_data(rawPtPath)
            ),
            rad::string(
                aisacAllocator,
                dr.get_string_data(rawName)
            ),
            parse_aisac_type_(type),
            rndrng
        );

        if (rawGrph.size)
        {
            const auto curPos = stream.tell();
            stream.jump_to(dr.get_buffer_data_position(rawGrph));

            aisac.graphs = read_graph_table_(
                stream,
                tmpAllocator,
                aisacAllocator
            );

            stream.jump_to(curPos);
        }
        else
        {
            // TODO: Log warning "UNEXPECTED - AISAC has no graphs"
        }
    }
}

static sound_element_format parse_sound_element_format_(u8 val)
{
    if (val != static_cast<u8>(sound_element_format::aax))
    {
        throw std::runtime_error("Unsupported CriAu sound element format");
    }

    return static_cast<sound_element_format>(val);
}

void read_sound_element_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    rad::vector<sound_element>& soundElements)
{
    utf::deserializer dr(stream, utf::deserialize_type::utf, tmpAllocator);

    // Get table revision and validate columns.
    const auto revisionInfo = get_revision_info_(dr, sound_element_revisions_);

    if (!revisionInfo || !dr.has_columns_of_exact_types(
        revisionInfo->column_group(), sound_element_columns_))
    {
        throw std::runtime_error("Invalid or unsupported TBLSDL layout");
    }

    // Read rows.
    auto& soundElementAllocator = soundElements.allocator();
    soundElements.reserve(dr.row_count());

    for (u32 i = 0; i < dr.row_count(); ++i)
    {
        // Read row.
        const auto rawName = dr.next_cell_as_string();
        const auto rawData = dr.next_cell_as_buffer();
        const auto fmt = dr.next_cell_as_u8();
        const auto nch = dr.next_cell_as_u8();
        const auto stmflg = dr.next_cell_as_u8();
        const auto sfreq = dr.next_cell_as_u32();

        if (stmflg != 0 && stmflg != 1)
        {
            throw std::runtime_error("Unsupported CriAu sound element stream flags");
        }

        // r1 columns
        const u32 nsmpl = (
            (revisionInfo->column_ranges() == sound_element_columns_r0_) ?
            0 : dr.next_cell_as_u32()
        );

        dr.next_row();

        // Generate sound element.
        auto& soundElement = soundElements.emplace_back(
            rad::string(
                soundElementAllocator,
                dr.get_string_data(rawName)
            ),
            parse_sound_element_format_(fmt),
            nch,
            sfreq,
            nsmpl
        );

        // Read embedded data (if any).
        if (!stmflg)
        {
            if (!rawData.size)
            {
                throw std::runtime_error("Missing embedded data despite unset stream flag");
            }

            soundElement.embeddedData = dr.read_buffer_data(
                rawData,
                soundElementAllocator
            );
        }
        else if (rawData.size)
        {
            // TODO: Log warning "UNEXPECTED - Stream flag is set, yet embedded data "
            // "is also present. Embedded data will be ignored."
        }
    }
}

static synth_link_type parse_synth_link_type_(u8 val)
{
    if (val != static_cast<u8>(synth_link_type::sound_element) &&
        val != static_cast<u8>(synth_link_type::synth))
    {
        throw std::runtime_error("Unsupported CriAu synth link type");
    }

    return static_cast<synth_link_type>(val);
}

static synth_complex_type parse_synth_complex_type_(u8 val)
{
    if (val != static_cast<u8>(synth_complex_type::polyphonic) &&
        val != static_cast<u8>(synth_complex_type::random_no_repeat) &&
        val != static_cast<u8>(synth_complex_type::sequential) &&
        val != static_cast<u8>(synth_complex_type::random) &&
        val != static_cast<u8>(synth_complex_type::sequential_no_loop))
    {
        // NOTE: From what I can tell, this is NOT an actual CriAu synth complex type!
        // However, SonicAudioTools uses this value as the "normal" type, which it writes
        // to CSB files. So, we have to support it, otherwise some mod CSB files will fail
        // to parse. The actual CriAu libraries seem to just treat unknown types as
        // polyphonic, so it also works this way in-game.
        if (val == 255)
        {
            return synth_complex_type::polyphonic;
        }

        throw std::runtime_error("Unsupported CriAu synth complex type");
    }

    return static_cast<synth_complex_type>(val);
}

void read_synth_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    rad::vector<synth>& synths)
{
    utf::deserializer dr(stream, utf::deserialize_type::utf, tmpAllocator);

    // Get table revision and validate columns.
    const auto revisionInfo = get_revision_info_(dr, synth_revisions_);

    if (!revisionInfo || !dr.has_columns_of_exact_types(
        revisionInfo->column_group(), synth_columns_))
    {
        throw std::runtime_error("Invalid or unsupported TBLSYN layout");
    }

    // Read rows.
    auto& synthAllocator = synths.allocator();
    synths.reserve(dr.row_count());

    for (u32 i = 0; i < dr.row_count(); ++i)
    {
        const auto rawSynname = dr.next_cell_as_string();
        const auto syntype = parse_synth_link_type_(dr.next_cell_as_u8());
        const auto cmplxtype = parse_synth_complex_type_(dr.next_cell_as_u8());

        auto& synth = synths.emplace_back(
            rad::string(synthAllocator, dr.get_string_data(rawSynname)),
            syntype,
            cmplxtype,
            synthAllocator
        );

        // Parse link names.
        const auto rawLnkname = dr.next_cell_as_string();

        if (syntype == synth_link_type::synth)
        {
            // Iterate through each line in the string.
            std::string_view linkNames = dr.get_string_data(rawLnkname);

            while (true)
            {
                const auto curLinkNameEnd = linkNames.find('\n');
                if (curLinkNameEnd == linkNames.npos) break;

                // Add the current line to the link names.
                synth.linkNames.emplace_back(
                    synthAllocator,
                    linkNames.data(),
                    curLinkNameEnd
                );

                const auto nextLinkNameStart = (curLinkNameEnd + 1);
                linkNames = std::string_view(
                    linkNames.data() + nextLinkNameStart,
                    linkNames.size() - nextLinkNameStart
                );
            }
        }
        else
        {
            synth.linkNames.emplace_back(
                synthAllocator,
                dr.get_string_data(rawLnkname)
            );
        }

        // Parse AISAC names.
        const auto rawIssetname = dr.next_cell_as_string();

        if (rawIssetname.dataOff)
        {
            // Iterate through each line in the string.
            std::string_view isSetNames = dr.get_string_data(rawIssetname);

            while (true)
            {
                const auto curIsSetNameEnd = isSetNames.find('\n');
                if (curIsSetNameEnd == isSetNames.npos) break;

                // Ensure the current line contains the substring "::".
                const auto sepChPtr = static_cast<const char*>(
                    std::memchr(isSetNames.data(), ':', curIsSetNameEnd)
                );

                if (!sepChPtr || sepChPtr[1] != ':')
                {
                    throw std::runtime_error("Invalid CSB issetname");
                }

                // Get the AISAC name after the "::" substring.
                const auto aisacNamePtr = (sepChPtr + 2);
                const auto aisacNameSize = static_cast<std::size_t>(
                    (isSetNames.data() + curIsSetNameEnd) - aisacNamePtr
                );

                // Add the AISAC name to the synth.
                synth.aisacNames.emplace_back(
                    synthAllocator,
                    aisacNamePtr,
                    aisacNameSize
                );

                const auto nextIsSetNameStart = (curIsSetNameEnd + 1);
                isSetNames = std::string_view(
                    isSetNames.data() + nextIsSetNameStart,
                    isSetNames.size() - nextIsSetNameStart
                );
            }
        }

        synth.volume = dr.next_cell_as_s16();
        synth.pitch = dr.next_cell_as_s16();
        synth.delayTime = dr.next_cell_as_u32();
        synth.sControl = dr.next_cell_as_u8();

        // TODO: Use C++20 delegated initializers.
        synth.envelopeInfo = {
            dr.next_cell_as_u16(),
            dr.next_cell_as_u16(),
            dr.next_cell_as_u16(),
            dr.next_cell_as_u16(),
            dr.next_cell_as_u16(),
            dr.next_cell_as_u16()
        };

        // TODO: Use C++20 delegated initializers.
        synth.filter0Info = {
            dr.next_cell_as_u8(),
            dr.next_cell_as_u16(),
            dr.next_cell_as_u16(),
            dr.next_cell_as_u16(),
            dr.next_cell_as_u8()
        };

        synth.dryOutName = rad::optional_string(
            synthAllocator,
            dr.get_optional_string_data(dr.next_cell_as_string())
        );

        synth.mtxrtr = rad::optional_string(
            synthAllocator,
            dr.get_optional_string_data(dr.next_cell_as_string())
        );

        for (std::size_t i = 0; i < synth.dry.size(); ++i)
        {
            synth.dry[i] = dr.next_cell_as_u16();
        }

        synth.wetOutName = rad::optional_string(
            synthAllocator,
            dr.get_optional_string_data(dr.next_cell_as_string())
        );

        for (std::size_t i = 0; i < synth.wet.size(); ++i)
        {
            synth.wet[i] = dr.next_cell_as_u16();
        }

        for (std::size_t i = 0; i < synth.wetCnct.size(); ++i)
        {
            synth.wetCnct[i] = rad::optional_string(
                synthAllocator,
                dr.get_optional_string_data(dr.next_cell_as_string())
            );
        }

        // TODO: Use C++20 delegated initializers.
        synth.voiceLimitInfo = {
            rad::optional_string(
                synthAllocator,
                dr.get_optional_string_data(dr.next_cell_as_string())
            ),
            dr.next_cell_as_u8(),
            dr.next_cell_as_u8(),
            dr.next_cell_as_u16(),
            dr.next_cell_as_s8()
        };

        // r1 columns
        if (revisionInfo->column_ranges() != synth_columns_r0_)
        {
            // TODO: Use C++20 delegated initializers.
            synth.pan3dInfo = {
                dr.next_cell_as_s16(),
                dr.next_cell_as_s16(),
                dr.next_cell_as_s16(),
                dr.next_cell_as_s16(),
                dr.next_cell_as_s16(),
                dr.next_cell_as_s16()
            };

            for (std::size_t i = 0; i < synth.dryG.size(); ++i)
            {
                synth.dryG[i] = dr.next_cell_as_u8();
            }

            for (std::size_t i = 0; i < synth.wetG.size(); ++i)
            {
                synth.wetG[i] = dr.next_cell_as_u8();
            }

            // TODO: Use C++20 delegated initializers.
            synth.filter1Info = {
                dr.next_cell_as_u8(),
                dr.next_cell_as_u16(),
                dr.next_cell_as_u16(),
                dr.next_cell_as_u16(),
                dr.next_cell_as_u16()
            };

            // TODO: Use C++20 delegated initializers.
            synth.filter2Info = {
                dr.next_cell_as_u8(),
                dr.next_cell_as_u16(),
                dr.next_cell_as_u16(),
                dr.next_cell_as_u16(),
                dr.next_cell_as_u16()
            };

            // r2 columns
            if (revisionInfo->column_ranges() != synth_columns_r1_)
            {
                synth.probability = dr.next_cell_as_u8();
                synth.numLimitChildren = dr.next_cell_as_u8();
                synth.repeat = (dr.next_cell_as_u8() != 0);

                // r3 columns
                if (revisionInfo->column_ranges() != synth_columns_r2_)
                {
                    synth.comboTime = dr.next_cell_as_u32();
                    synth.comboLoopBack = dr.next_cell_as_u8();
                }
            }
        }

        dr.next_row();
    }
}

void read_cue_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    rad::vector<cue>& cues)
{
    utf::deserializer dr(stream, utf::deserialize_type::utf, tmpAllocator);

    // Get table revision and validate columns.
    const auto revisionInfo = get_revision_info_(dr, cue_revisions_);

    if (!revisionInfo || !dr.has_columns_of_exact_types(
        revisionInfo->column_group(), cue_columns_))
    {
        throw std::runtime_error("Invalid or unsupported TBLCUE layout");
    }

    // Read rows.
    auto& cueAllocator = cues.allocator();
    cues.reserve(dr.row_count());

    for (u32 i = 0; i < dr.row_count(); ++i)
    {
        // Read row.
        const auto rawName = dr.next_cell_as_string();
        const auto id = dr.next_cell_as_u32();
        const auto rawSynth = dr.next_cell_as_string();
        const auto rawUdata = dr.next_cell_as_string();

        const u8 flags = (
            (revisionInfo->column_ranges() == cue_columns_r0_) ?
            CUE_FLAGS_NONE : dr.next_cell_as_u8()
        );

        dr.next_row();

        // Validate flags.
        if (flags != CUE_FLAGS_NONE &&
            flags != CUE_FLAGS_DOES_LOOP)
        {
            throw std::runtime_error("Unsupported CriAu cue flags");
        }

        // Generate cue.
        cues.emplace_back(
            id,
            rad::string(
                cueAllocator,
                dr.get_string_data(rawSynth)
            ),
            flags,
            rad::optional_string(
                cueAllocator,
                dr.get_optional_string_data(rawName)
            ),
            rad::optional_string(
                cueAllocator,
                dr.get_optional_string_data(rawUdata)
            )
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

void cue_sheet::read_inner(rad::stream& stream, rad::allocator& tmpAllocator)
{
    clear();

    utf::deserializer dr(
        stream,
        utf::deserialize_type::inner_table,
        tmpAllocator
    );

    // Validate columns.
    if (!dr.has_columns_of_exact_types(cue_sheet_columns_, 0, 3))
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

    for (u32 i = 0; i < dr.row_count(); ++i)
    {
        // Read row.
        const auto rawName = dr.next_cell_as_string();
        const auto ttype = dr.next_cell_as_u8();
        const auto utf = dr.next_cell_as_buffer();

        dr.next_row();

        // Store raw table information for later, based on table type.

        // NOTE: We compare using name because it's what the actual games
        // seem to use; they do NOT seem to utilize the ttype column.

        const auto name = dr.get_string_data(rawName);
        raw_csb_table_type_ type = CSB_TABLE_TYPE_NONE;

        if (std::strcmp(name, "INFO") == 0)
        {
            type = CSB_TABLE_TYPE_INFO;
        }
        else if (std::strcmp(name, "CUE") == 0)
        {
            type = CSB_TABLE_TYPE_CUE;
        }
        else if (std::strcmp(name, "SYNTH") == 0)
        {
            type = CSB_TABLE_TYPE_SYNTH;
        }
        else if (std::strcmp(name, "SOUND_ELEMENT") == 0)
        {
            type = CSB_TABLE_TYPE_SOUND_ELEMENT;
        }
        else if (std::strcmp(name, "ISAAC") == 0)
        {
            type = CSB_TABLE_TYPE_ISAAC;
        }
        else if (std::strcmp(name, "VOICE_LIMIT_GROUP") == 0)
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
    // TODO: Read INFO

    if (rawTables[CSB_TABLE_TYPE_ISAAC].size)
    {
        stream.jump_to(dr.get_buffer_data_position(
            rawTables[CSB_TABLE_TYPE_ISAAC]
        ));

        read_aisac_table_(stream, tmpAllocator, aisacs);
    }

    if (rawTables[CSB_TABLE_TYPE_SOUND_ELEMENT].size)
    {
        stream.jump_to(dr.get_buffer_data_position(
            rawTables[CSB_TABLE_TYPE_SOUND_ELEMENT]
        ));

        read_sound_element_table_(stream, tmpAllocator, soundElements);
    }

    // TODO: Read VOICE_LIMIT_GROUP

    if (rawTables[CSB_TABLE_TYPE_SYNTH].size)
    {
        stream.jump_to(dr.get_buffer_data_position(
            rawTables[CSB_TABLE_TYPE_SYNTH]
        ));

        read_synth_table_(stream, tmpAllocator, synths);
    }

    if (rawTables[CSB_TABLE_TYPE_CUE].size)
    {
        stream.jump_to(dr.get_buffer_data_position(
            rawTables[CSB_TABLE_TYPE_CUE]
        ));

        read_cue_table_(stream, tmpAllocator, cues);
    }
}

void cue_sheet::read(rad::stream& stream, rad::allocator& tmpAllocator)
{
    // Read UTF header.
    utf::reader reader(stream);
    const auto header = reader.read_header();

    // Read inner table data.
    if (stream.capabilities().can_nocost_read() &&
        stream.capabilities().can_seek())
    {
        // Directly read the data from the stream.
        const auto rawTablePos = stream.tell();
        read_inner(stream, tmpAllocator);

        // Jump to the end of the table.
        stream.jump_to(rawTablePos + header.tableSize);
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

        read_inner(tableDataStream, tmpAllocator);
    }
}
}
