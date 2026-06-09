#include "hedgelib/cri/hl_cri_atom_cue_sheet.h"
#include "hl_cri_atom_impl.h"

namespace hl::cri::atom
{
static const utf::column_info auto_modulation_columns_[] =
{
    // r0 columns

    // group: 0, index: 0, count: 4
    { utf::cell_type::u8, "Type" },
    { utf::cell_type::u8, "TriggerType" },
    { utf::cell_type::u32, "Time" },
    { utf::cell_type::u32, "Key" },
};

static const utf::column_info aisac_control_name_columns_[] =
{
    // r0 columns

    // group: 0, index: 0, count: 2
    { utf::cell_type::string, "AisacControlName" },
    { utf::cell_type::u16, "AisacControlId" },
};

static const utf::column_info aisac_columns_[] =
{
    // r0 columns

    // group: 0, index: 0, count: 4
    { utf::cell_type::s16, "Id" },
    { utf::cell_type::u8, "Type" },
    { utf::cell_type::u16, "ControlId" },
    { utf::cell_type::f32, "RandomRange" },
    
    // group: 1, index: 4, count: 1
    { utf::cell_type::u16, "AudoModulationIndex" },

    // group: 2, index: 5, count: 1
    { utf::cell_type::buffer, "GraphIndexes" },

    // r1 columns

    // group: 3, index: 6, count: 3
    { utf::cell_type::u8, "DefaultControlFlag" },
    { utf::cell_type::f32, "DefaultControl" },
    { utf::cell_type::u8, "GraphBitFlag" },

    // r2 columns

    // group: 4, index: 9, count: 1
    { utf::cell_type::u16, "AutoModulationIndex" }, // AudoModulationIndex

    // r3 columns

    // group: 5, index: 10, count: 1
    { utf::cell_type::buffer, "GraphTypeIndexes" },
};

static constexpr utf::column_info_range aisac_columns_r0_[] =
{
    { 0, 6 }, // Id - GraphIndexes
};

static constexpr utf::column_info_range aisac_columns_r1_[] =
{
    { 0, 9 },   // Id - GraphBitFlag
};

static constexpr utf::column_info_range aisac_columns_r2_[] =
{
    { 0, 4 },   // Id - RandomRange
    { 9, 1 },   // AutoModulationIndex
    { 5, 4 },   // GraphIndexes - GraphBitFlag
};

static constexpr utf::column_info_range aisac_columns_r3_[] =
{
    { 0, 4 },   // Id - RandomRange
    { 9, 1 },   // AutoModulationIndex
    { 5, 4 },   // GraphIndexes - GraphBitFlag
    { 10, 1 },  // GraphTypeIndexes
};

static constexpr revision_info_ aisac_revisions_[] =
{
    { packed_version(0), aisac_columns_r0_ }, // r0
    { packed_version(1, 12, 00), aisac_columns_r1_ }, // r1
    { packed_version(1, 26, 00), aisac_columns_r2_ }, // r2
    { packed_version(1, 37, 00), aisac_columns_r3_ }, // r3
};

static auto_modulation_type parse_auto_modulation_type_(u8 type)
{
    if (type != static_cast<u8>(auto_modulation_type::one_shot) &&
        type != static_cast<u8>(auto_modulation_type::loop))
    {
        throw std::runtime_error("Unsupported CriAtom auto modulation type");
    }

    return static_cast<auto_modulation_type>(type);
}

static trigger_type parse_trigger_type_(u8 type)
{
    if (type != static_cast<u8>(trigger_type::playback) &&
        type != static_cast<u8>(trigger_type::key))
    {
        throw std::runtime_error("Unsupported CriAtom trigger type type");
    }

    return static_cast<trigger_type>(type);
}

void read_auto_modulation_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    packed_version version,
    rad::vector<auto_modulation>& autoModulations)
{
    assert(autoModulations.empty() &&
        "The given output vector must be empty"
    );

    // Read raw table header.
    utf::deserializer dr(stream, utf::deserialize_type::utf, tmpAllocator);

    // Validate columns.
    if (!dr.has_columns_of_exact_types(
        auto_modulation_columns_,
        0,
        static_cast<u16>(std::size(auto_modulation_columns_))))
    {
        throw std::runtime_error("Invalid or unsupported ACB Auto Modulation layout");
    }

    // Read rows.
    auto& autoModulationAllocator = autoModulations.allocator();
    autoModulations.reserve(dr.row_count());

    for (u32 i = 0; i < dr.row_count(); dr.next_row(), ++i)
    {
        // r0
        const auto type = parse_auto_modulation_type_(dr.next_cell_as_u8()); // Type
        const auto triggerType = parse_trigger_type_(dr.next_cell_as_u8()); // TriggerType
        const auto time = dr.next_cell_as_u32(); // Time
        const auto key = dr.next_cell_as_u32(); // Key

        autoModulations.emplace_back_unchecked(
            type,
            triggerType,
            time,
            key
        );
    }
}

void write_auto_modulation_table_(
    detail_::write_params& wp,
    const rad::vector<auto_modulation>& autoModulations)
{
    utf::serializer sr(*wp.stream, *wp.allocator);

    sr.start(
        "AutoModulation",
        auto_modulation_columns_,
        wp.encoding
    );

    // Write rows.
    for (std::size_t i = 0; i < autoModulations.size(); sr.next_row(), ++i)
    {
        const auto& autoModulation = autoModulations[i];

        // r0
        sr.push_cell_u8(static_cast<u8>(autoModulation.type)); // Type
        sr.push_cell_u8(static_cast<u8>(autoModulation.triggerType)); // TriggerType
        sr.push_cell_u32(autoModulation.time); // Time
        sr.push_cell_u32(autoModulation.key); // Key
    }

    // Finish writing table.
    sr.finish();
}

void read_aisac_control_name_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    packed_version version,
    rad::vector<aisac_control>& aisacControls)
{
    assert(aisacControls.empty() &&
        "The given output vector must be empty"
    );

    // Read raw table header.
    utf::deserializer dr(stream, utf::deserialize_type::utf, tmpAllocator);

    // Validate columns.
    if (!dr.has_columns_of_exact_types(
        aisac_control_name_columns_,
        0,
        static_cast<u16>(std::size(aisac_control_name_columns_))))
    {
        throw std::runtime_error("Invalid or unsupported ACB Aisac Control Name layout");
    }

    // Read rows.
    auto& aisacControlAllocator = aisacControls.allocator();
    aisacControls.reserve(dr.row_count());

    for (u32 i = 0; i < dr.row_count(); dr.next_row(), ++i)
    {
        // r0
        const auto name = dr.next_cell_as_string(); // AisacControlName
        const auto id = dr.next_cell_as_u16(); // AisacControlId

        aisacControls.emplace_back_unchecked(
            rad::string(aisacControlAllocator, dr.get_string_data(name)),
            id
        );
    }
}

void write_aisac_control_name_table_(
    detail_::write_params& wp,
    const rad::vector<aisac_control>& aisacControls)
{
    utf::serializer sr(*wp.stream, *wp.allocator);

    sr.start(
        "AisacControlName",
        aisac_control_name_columns_,
        wp.encoding
    );

    // Write rows.
    for (std::size_t i = 0; i < aisacControls.size(); sr.next_row(), ++i)
    {
        const auto& aisacControl = aisacControls[i];

        // r0
        sr.push_cell_string(aisacControl.name); // AisacControlName
        sr.push_cell_u16(aisacControl.id); // AisacControlId
    }

    // Finish writing table.
    sr.finish();
}

static aisac_type parse_aisac_type_(u8 type)
{
    if (type != static_cast<u8>(aisac_type::simple) &&
        type != static_cast<u8>(aisac_type::automation))
    {
        throw std::runtime_error("Unsupported CriAtom aisac type");
    }

    return static_cast<aisac_type>(type);
}

void read_aisac_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    packed_version version,
    rad::vector<aisac>& aisacs)
{
    assert(aisacs.empty() &&
        "The given output vector must be empty"
    );

    // Read raw table header.
    utf::deserializer dr(stream, utf::deserialize_type::utf, tmpAllocator);

    // Get revision info.
    const auto revisionInfo = get_revision_info_for_version_(
        aisac_revisions_,
        version
    );

    // Validate columns.
    if (!dr.has_columns_of_exact_types(revisionInfo->columnGroup, aisac_columns_))
    {
        throw std::runtime_error("Invalid or unsupported ACB Aisac layout");
    }

    // Read rows.
    auto& aisacAllocator = aisacs.allocator();
    aisacs.reserve(dr.row_count());

    for (u32 i = 0; i < dr.row_count(); dr.next_row(), ++i)
    {
        // r0
        const auto id = dr.next_cell_as_s16(); // Id
        const auto type = parse_aisac_type_(dr.next_cell_as_u8()); // Type
        const auto controlId = dr.next_cell_as_u16(); // ControlId

        auto& aisac = aisacs.emplace_back_unchecked(
            type,
            controlId,
            aisacAllocator
        );

        aisac.id = id;
        aisac.randomRange = dr.next_cell_as_f32(); // RandomRange
        aisac.autoModulationIndex = dr.next_cell_as_u16(); // AudoModulationIndex
        const auto rawGraphIndexes = dr.next_cell_as_buffer(); // GraphIndexes

        // r1
        if (version >= aisac_revisions_[1].version)
        {
            const auto defaultControlFlag = dr.next_cell_as_u8(); // DefaultControlFlag
            if (defaultControlFlag > 1)
            {
                throw std::runtime_error("Unsupported CriAtom Aisac DefaultControlFlag value");
            }

            const auto defaultControl = dr.next_cell_as_f32(); // DefaultControl

            if (defaultControlFlag)
            {
                aisac.defaultControl = defaultControl;
            }

            const auto graphBitFlag = dr.next_cell_as_u8(); // GraphBitFlag // TODO
        }

        // r3
        utf::raw_buffer rawGraphTypeIndexes = {};

        if (version >= aisac_revisions_[3].version)
        {
            rawGraphTypeIndexes = dr.next_cell_as_buffer(); // GraphTypeIndexes // TODO
        }

        // GraphIndexes
        deserialize_u16_array_(dr, rawGraphIndexes, aisac.graphIndices);
    }
}

void write_aisac_table_(
    detail_::write_params& wp,
    const rad::vector<aisac>& aisacs)
{
    utf::serializer sr(*wp.stream, *wp.allocator);

    const auto revisionInfo = get_revision_info_for_version_(
        aisac_revisions_,
        wp.version
    );

    rad::vector<utf::column_info> columns(*wp.allocator);
    revisionInfo->columnGroup.append_to(aisac_columns_, columns);

    sr.start("Aisac", columns, wp.encoding);

    // Write rows.
    for (std::size_t i = 0; i < aisacs.size(); sr.next_row(), ++i)
    {
        const auto& aisac = aisacs[i];

        // r0
        sr.push_cell_s16(aisac.id); // Id
        sr.push_cell_u8(static_cast<u8>(aisac.type)); // Type
        sr.push_cell_u16(aisac.controlId); // ControlId
        sr.push_cell_f32(aisac.randomRange); // RandomRange
        sr.push_cell_u16(aisac.autoModulationIndex); // AudoModulationIndex
        sr.push_cell_buffer(aisac.graphIndices.empty()); // GraphIndexes

        // r1
        if (wp.version < aisac_revisions_[1].version) continue;

        sr.push_cell_u8(aisac.defaultControl.has_value()); // DefaultControlFlag
        sr.push_cell_f32(aisac.defaultControl.value_or(0.0f)); // DefaultControl
        sr.push_cell_u8(0); // GraphBitFlag

        // r3
        if (wp.version < aisac_revisions_[3].version) continue;

        sr.push_cell_buffer(); // GraphTypeIndexes
    }

    // Write buffers.
    auto br = sr.begin_buffer_data_section();

    for (const auto& aisac : aisacs)
    {
        // GraphIndexes
        serialize_u16_array_(br, aisac.graphIndices);

        if (wp.version < aisac_revisions_[3].version) continue;

        // GraphTypeIndexes
        // TODO: GraphTypeIndexes
        br.next();
    }

    // Finish writing table.
    sr.finish();
}
}
