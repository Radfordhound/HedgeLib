#include "hedgelib/cri/hl_cri_atom_cue_sheet.h"

namespace hl::cri_new::atom
{
static const utf::column_info graph_columns_[] =
{
    // r0 columns

    // group: 0, index: 0, count: 6
    { utf::cell_type::u16, "Type" },
    { utf::cell_type::buffer, "Controls" },
    { utf::cell_type::buffer, "Destinations" },
    { utf::cell_type::buffer, "Curve" },
    { utf::cell_type::f32, "ControlWorkArea" },
    { utf::cell_type::f32, "DestinationWorkArea" },
};

static constexpr utf::column_info_range graph_columns_r0_[] =
{
    { 0, 6 }, // Type - DestinationWorkArea
};

static constexpr revision_info graph_revisions_[] =
{
    { packed_version(0), graph_columns_r0_ }, // r0
};

namespace detail_
{
    void write_graph_table(write_params& wp, const rad::vector<graph>& graphs)
    {
        utf::table_serializer ts(*wp.stream, *wp.allocator);

        const auto revisionInfo = get_revision_info(
            graph_revisions_,
            wp.version
        );

        ts.start(
            "Graph",
            { graph_columns_, revisionInfo->columns.get_total_count() },
            wp.encoding
        );

        // Write rows.
        float controlWorkArea = -1.0f;
        float destinationWorkArea = 0.0f;

        for (std::size_t i = 0; i < graphs.size(); ts.next_row(), ++i)
        {
            const auto& graph = graphs[i];

            // r0 columns
            ts.write_cell_as_u16(static_cast<u16>(graph.type)); // Type
            ts.write_cell_as_buffer(); // Controls
            ts.write_cell_as_buffer(); // Destinations
            ts.write_cell_as_buffer(); // Curve
            ts.write_cell_as_f32(controlWorkArea); // ControlWorkArea
            ts.write_cell_as_f32(destinationWorkArea); // DestinationWorkArea

            controlWorkArea -= 999.0f;
            destinationWorkArea += 1.0f;
        }

        // Finish writing rows.
        ts.finish_rows();

        // Write buffers.
        for (std::size_t cellIndex = 0, i = 0;
            i < graphs.size();
            cellIndex += ts.column_count(), ++i)
        {
            const auto& graph = graphs[i];

            if (!graph.points.empty())
            {
                // Controls
                {
                    const auto controlsDataPos = wp.stream->tell();
                    
                    for (const auto& point : graph.points)
                    {
                        ts.writer().write_f32(point.control);
                    }

                    ts.fill_buffer_cell(cellIndex + 1, controlsDataPos);
                }

                // Destinations
                {
                    const auto destinationsDataPos = wp.stream->tell();
                    
                    for (const auto& point : graph.points)
                    {
                        ts.writer().write_u16(point.destination);
                    }

                    ts.fill_buffer_cell(cellIndex + 2, destinationsDataPos);
                }

                // Curve
                {
                    const auto curvesDataPos = wp.stream->tell();
                    
                    for (const auto& point : graph.points)
                    {
                        ts.writer().write_u16(point.curve);
                    }

                    ts.fill_buffer_cell(cellIndex + 3, curvesDataPos);
                }
            }
        }

        // Finish writing table.
        ts.finish();
        wp.stream->pad(32);
    }
}
}
