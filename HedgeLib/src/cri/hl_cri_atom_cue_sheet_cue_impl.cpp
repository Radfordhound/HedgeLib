#include "hedgelib/cri/hl_cri_atom_cue_sheet.h"
#include "hl_cri_atom_impl.h"

namespace hl::cri::atom
{
static const utf::column_info cue_columns_[] =
{
    // r0 columns

    // group: 0, index: 0, count: 8
    { utf::cell_type::u32, "CueId" },
    { utf::cell_type::u8, "ReferenceType" },
    { utf::cell_type::u16, "ReferenceIndex" },
    { utf::cell_type::string, "UserData" },
    { utf::cell_type::u16, "Worksize" },
    { utf::cell_type::buffer, "AisacControlMap" },
    { utf::cell_type::u32, "Length" },
    { utf::cell_type::u8, "NumAisacControlMaps" },

    // r1 columns

    // group: 1, index: 8, count: 1
    { utf::cell_type::u8, "HeaderVisibility" },

    // r2 columns

    // group: 2, index: 9, count: 1
    { utf::cell_type::u16, "NumRelatedWaveforms" },
};

static const utf::column_info_range cue_columns_r0_[] =
{
    { 0, 8 },   // CueId - NumAisacControlMaps
};

static const utf::column_info_range cue_columns_r1_[] =
{
    { 0, 9 },   // CueId - HeaderVisibility
};

static const utf::column_info_range cue_columns_r2_[] =
{
    { 0, 10 },  // CueId - NumRelatedWaveforms
};

static const revision_info_ cue_revisions_[] =
{
    { packed_version(0), cue_columns_r0_ }, // r0
    { packed_version(1, 20, 03), cue_columns_r1_ }, // r1
    { packed_version(1, 37, 00), cue_columns_r2_ }, // r2
};

static const utf::column_info cue_name_columns_[] =
{
    // r0 columns
    { utf::cell_type::string, "CueName" },
    { utf::cell_type::u16, "CueIndex" },
};

void read_cue_name_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    packed_version version,
    rad::vector<cue>& cues)
{
    // Read raw table header.
    utf::deserializer dr(stream, utf::deserialize_type::utf, tmpAllocator);

    // Validate columns.
    if (!dr.has_columns_of_exact_types(cue_name_columns_,
        0,
        static_cast<u16>(std::size(cue_name_columns_))))
    {
        throw std::runtime_error("Invalid of unsupported ACB CueName layout");
    }

    // Read rows.
    for (u32 i = 0; i < dr.row_count(); dr.next_row(), ++i)
    {
        const auto name = dr.next_cell_as_string(); // CueName
        const auto cueIndex = dr.next_cell_as_u16(); // CueIndex

        if (cueIndex >= cues.size())
        {
            // TODO: Log warning
            continue;
        }

        cues[cueIndex].name.assign(dr.get_string_data(name));
    }
}

void read_cue_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    packed_version version,
    rad::vector<cue>& cues)
{
    // Read raw table header.
    utf::deserializer dr(stream, utf::deserialize_type::utf, tmpAllocator);

    // Get revision info.
    const auto revisionInfo = get_revision_info_for_version_(
        cue_revisions_,
        version
    );

    // Validate columns.
    if (!dr.has_columns_of_exact_types(revisionInfo->columnGroup, cue_columns_))
    {
        throw std::runtime_error("Invalid or unsupported ACB Cue layout");
    }

    // Read rows.
    auto& cueAllocator = cues.allocator();
    cues.reserve(dr.row_count());

    for (u32 i = 0; i < dr.row_count(); dr.next_row(), ++i)
    {
        // r0
        const auto id = dr.next_cell_as_u32(); // CueId
        const auto refType = parse_ref_type_(dr.next_cell_as_u8()); // ReferenceType
        const auto refIndex = dr.next_cell_as_u16(); // ReferenceIndex

        auto& cue = cues.emplace_back(
            id,
            ref_item{refType, refIndex},
            rad::optional_string{cueAllocator},
            cueAllocator
        );

        const auto rawUserData = dr.next_cell_as_string(); // UserData
        const auto userData = dr.get_string_data(rawUserData);

        if (*userData)
        {
            cue.userData = userData;
        }

        const auto worksize = dr.next_cell_as_u16(); // Worksize // TODO
        const auto rawAisacControlMap = dr.next_cell_as_buffer(); // AisacControlMap
        cue.playDuration = dr.next_cell_as_u32(); // Length
        const auto aisacControlMapCount = dr.next_cell_as_u8(); // NumAisacControlMaps

        // r1
        if (version >= cue_revisions_[1].version)
        {
            cue.headerVisibility = (dr.next_cell_as_u8() != 0); // HeaderVisibility
        }

        // r2
        if (version >= cue_revisions_[2].version)
        {
            cue.relatedWaveformCount = dr.next_cell_as_u16(); // NumRelatedWaveforms
        }

        // AisacControlMap
        if (aisacControlMapCount != 0)
        {
            if (aisacControlMapCount > rawAisacControlMap.size)
            {
                throw std::runtime_error("Invalid AISAC Control Map Count");
            }
            else if (aisacControlMapCount != rawAisacControlMap.size)
            {
                // TODO: Log warning
            }

            cue.aisacControlMap.assign(rad::no_value_init, aisacControlMapCount);

            stream.jump_to(dr.get_buffer_data_position(rawAisacControlMap));
            stream.read(cue.aisacControlMap.data(), aisacControlMapCount);
        }
    }
}

void write_cue_name_table_(
    detail_::write_params& wp,
    const rad::vector<const cue*>& sortedCues)
{
    if (sortedCues.size() > UINT16_MAX) // TODO: Mark unlikely
    {
        for (std::size_t i = UINT16_MAX; i < sortedCues.size(); ++i)
        {
            if (sortedCues[i]->name)
            {
                throw std::overflow_error("ACB cue count exceeds u16 range");
            }
        }
    }

    utf::serializer sr(*wp.stream, *wp.allocator);

    sr.start("CueName", cue_name_columns_, wp.encoding);

    // Sort cue names.
    struct cue_name
    {
        std::string_view name;
        hl::u16 sortedCueIndex;

        inline cue_name(std::string_view name, hl::u16 sortedCueIndex) noexcept
            : name(name)
            , sortedCueIndex(sortedCueIndex)
        {
        }
    };

    rad::vector<cue_name> sortedCueNames(*wp.allocator);
    sortedCueNames.reserve(sortedCues.size());

    for (std::size_t i = 0; i < sortedCues.size(); ++i)
    {
        const auto* cue = sortedCues[i];

        if (cue->name)
        {
            sortedCueNames.emplace_back_unchecked(
                cue->name.value(),
                static_cast<hl::u16>(i)
            );
        }
    }

    std::sort(sortedCueNames.begin(), sortedCueNames.end(),
        [](const cue_name& a, const cue_name& b)
        {
            return a.name < b.name;
        }
    );

    // Write rows.
    for (const auto& cueName : sortedCueNames)
    {
        sr.push_cell_string(cueName.name); // CueName
        sr.push_cell_u16(cueName.sortedCueIndex); // CueIndex

        sr.next_row();
    }

    sr.begin_buffer_data_section();

    // Finish writing table.
    sr.writer().stream().pad(4);
    sr.finish();
}

void write_cue_table_(
    detail_::write_params& wp,
    const rad::vector<const cue*>& sortedCues,
    const rad::vector<synth>& synths)
{
    utf::serializer sr(*wp.stream, *wp.allocator);

    const auto revisionInfo = get_revision_info_for_version_(
        cue_revisions_,
        wp.version
    );

    sr.start(
        "Cue",
        { cue_columns_, revisionInfo->get_column_count() },
        wp.encoding
    );

    // Write rows.
    for (std::size_t i = 0; i < sortedCues.size(); sr.next_row(), ++i)
    {
        const auto& cue = *sortedCues[i];

        // r0
        sr.push_cell_u32(cue.id); // CueId
        sr.push_cell_u8(static_cast<u8>(cue.refItem.type)); // ReferenceType
        sr.push_cell_u16(cue.refItem.index); // ReferenceIndex
        sr.push_cell_string(cue.userData); // UserData
        
        sr.push_cell_u16(
            (cue.refItem.type == ref_type::synth &&
            synths.size() > cue.refItem.index &&
            synths[cue.refItem.index].type == synth_type::random_no_repeat) ?
            16 : 0
        ); // Worksize

        sr.push_cell_buffer(cue.aisacControlMap.empty()); // AisacControlMap
        sr.push_cell_u32(cue.playDuration); // Length

        if (cue.aisacControlMap.size() > UINT8_MAX)
        {
            throw std::overflow_error("ACB AISAC Control Map count exceeds u8 range");
        }

        sr.push_cell_u8(static_cast<u8>(cue.aisacControlMap.size())); // NumAisacControlMaps

        // r1
        if (wp.version < cue_revisions_[1].version) continue;

        sr.push_cell_u8(cue.headerVisibility); // HeaderVisibility

        // r2
        if (wp.version < cue_revisions_[2].version) continue;

        sr.push_cell_u16(cue.relatedWaveformCount); // NumRelatedWaveforms
    }

    // Write buffers.
    auto br = sr.begin_buffer_data_section();

    for (const auto cue : sortedCues)
    {
        // AisacControlMap
        if (!cue->aisacControlMap.empty())
        {
            br.start();
            br.stream().write(cue->aisacControlMap.data(), cue->aisacControlMap.size());
        }

        br.next();
    }

    // Finish writing table.
    sr.writer().stream().pad(4);
    sr.finish();
}
}
