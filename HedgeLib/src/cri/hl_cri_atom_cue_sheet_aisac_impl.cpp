#include "hedgelib/cri/hl_cri_atom_cue_sheet.h"

namespace hl::cri_new::atom
{
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

static constexpr revision_info aisac_revisions_[] =
{
    { packed_version(0), aisac_columns_r0_ }, // r0
    { packed_version(1, 12, 00), aisac_columns_r1_ }, // r1
    { packed_version(1, 26, 00), aisac_columns_r2_ }, // r2
    { packed_version(1, 37, 00), aisac_columns_r3_ }, // r3
};

static constexpr std::size_t aisac_max_column_count_ = aisac_revisions_[3].get_total_count();

static const utf::column_info aisac_control_name_columns_[] =
{
    // r0 columns

    // group: 0, index: 0, count: 2
    { utf::cell_type::string, "AisacControlName" },
    { utf::cell_type::u16, "AisacControlId" },
};

static const utf::column_info auto_modulation_columns_[] =
{
    // r0 columns

    // group: 0, index: 0, count: 4
    { utf::cell_type::u8, "Type" },
    { utf::cell_type::u8, "TriggerType" },
    { utf::cell_type::u32, "Time" },
    { utf::cell_type::u32, "Key" },
};

namespace detail_
{
    void write_aisac_table(write_params& wp, const rad::vector<aisac>& aisacs)
    {
        utf::table_serializer ts(*wp.stream, *wp.allocator);

        const auto revisionInfo = get_revision_info(
            aisac_revisions_,
            wp.version
        );

        rad::vector<utf::column_info> columns(*wp.allocator);
        columns.reserve(aisac_max_column_count_);

        revisionInfo->columns.append_to(aisac_columns_, columns);

        ts.start("Aisac", columns, wp.encoding);

        // Write rows.
        for (std::size_t i = 0; i < aisacs.size(); ts.next_row(), ++i)
        {
            const auto& aisac = aisacs[i];

            // r0 columns
            ts.write_cell_as_s16(aisac.id); // Id
            ts.write_cell_as_u8(static_cast<u8>(aisac.type)); // Type
            ts.write_cell_as_u16(aisac.controlID); // ControlId
            ts.write_cell_as_f32(aisac.randomRange); // RandomRange
            ts.write_cell_as_u16(aisac.autoModulationIndex); // AudoModulationIndex
            ts.write_cell_as_buffer(); // GraphIndexes

            // r1 columns
            if (wp.version < aisac_revisions_[1].version) continue;

            ts.write_cell_as_u8(aisac.defaultControl.has_value()); // DefaultControlFlag
            ts.write_cell_as_f32(aisac.defaultControl.value_or(0.0f)); // DefaultControl
            ts.write_cell_as_u8(0); // GraphBitFlag

            // r3 columns
            if (wp.version < aisac_revisions_[3].version) continue;

            ts.write_cell_as_buffer(); // GraphTypeIndexes
        }

        // Finish writing rows.
        ts.finish_rows();

        // Write buffers.
        for (std::size_t cellIndex = 0, i = 0;
            i < aisacs.size();
            cellIndex += ts.column_count(), ++i)
        {
            const auto& aisac = aisacs[i];

            // GraphIndexes
            if (!aisac.graphIndices.empty())
            {
                const auto graphIndexesDataPos = wp.stream->tell();
                
                for (const auto graphIndex : aisac.graphIndices)
                {
                    ts.writer().write_u16(graphIndex);
                }

                ts.fill_buffer_cell(cellIndex + 5, graphIndexesDataPos);
            }

            if (wp.version < aisac_revisions_[3].version) continue;

            // TODO: GraphTypeIndexes
        }

        // Finish writing table.
        ts.finish();
        wp.stream->pad(32);
    }

    void write_aisac_control_name_table(
        write_params& wp,
        const rad::vector<aisac_control>& aisacControls)
    {
        utf::table_serializer ts(*wp.stream, *wp.allocator);

        ts.start(
            "AisacControlName",
            aisac_control_name_columns_,
            wp.encoding
        );

        // Write rows.
        for (std::size_t i = 0; i < aisacControls.size(); ts.next_row(), ++i)
        {
            const auto& aisacControl = aisacControls[i];

            // r0 columns
            ts.write_cell_as_string(aisacControl.name); // AisacControlName
            ts.write_cell_as_u16(aisacControl.id); // AisacControlId
        }

        // Finish writing rows.
        ts.finish_rows();

        // Finish writing table.
        ts.finish();
        wp.stream->pad(32);
    }

    void write_auto_modulation_table(
        write_params& wp,
        const rad::vector<auto_modulation>& autoModulations)
    {
        utf::table_serializer ts(*wp.stream, *wp.allocator);

        ts.start(
            "AutoModulation",
            auto_modulation_columns_,
            wp.encoding
        );

        // Write rows.
        for (std::size_t i = 0; i < autoModulations.size(); ts.next_row(), ++i)
        {
            const auto& autoModulation = autoModulations[i];

            // r0 columns
            ts.write_cell_as_u8(static_cast<u8>(autoModulation.type)); // Type
            ts.write_cell_as_u8(static_cast<u8>(autoModulation.triggerType)); // TriggerType
            ts.write_cell_as_u32(autoModulation.time); // Time
            ts.write_cell_as_u32(autoModulation.key); // Key
        }

        // Finish writing rows.
        ts.finish_rows();

        // Finish writing table.
        ts.finish();
        wp.stream->pad(32);
    }
}
}
