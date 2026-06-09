#include "hedgelib/cri/hl_cri_atom_cue_sheet.h"
#include "hl_cri_atom_impl.h"

namespace hl::cri::atom
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

static constexpr revision_info_ graph_revisions_[] =
{
    { packed_version(0), graph_columns_r0_ }, // r0
};

static graph_type parse_graph_type_(u16 type)
{
    if (type != static_cast<u16>(graph_type::volume) &&
        type != static_cast<u16>(graph_type::pitch) &&
        type != static_cast<u16>(graph_type::bandpass_cutoff_high) &&
        type != static_cast<u16>(graph_type::bandpass_cutoff_low) &&
        type != static_cast<u16>(graph_type::biquad_cutoff) &&
        type != static_cast<u16>(graph_type::biquad_q) &&
        type != static_cast<u16>(graph_type::bus_send_0) &&
        type != static_cast<u16>(graph_type::bus_send_1) &&
        type != static_cast<u16>(graph_type::bus_send_2) &&
        type != static_cast<u16>(graph_type::bus_send_3) &&
        type != static_cast<u16>(graph_type::bus_send_4) &&
        type != static_cast<u16>(graph_type::bus_send_5) &&
        type != static_cast<u16>(graph_type::bus_send_6) &&
        type != static_cast<u16>(graph_type::bus_send_7) &&
        type != static_cast<u16>(graph_type::pan3d_angle) &&
        type != static_cast<u16>(graph_type::pan3d_volume) &&
        type != static_cast<u16>(graph_type::pan3d_internal_distance) &&
        type != static_cast<u16>(graph_type::aisac_control_1) &&
        type != static_cast<u16>(graph_type::aisac_control_2) &&
        type != static_cast<u16>(graph_type::aisac_control_3) &&
        type != static_cast<u16>(graph_type::aisac_control_4) &&
        type != static_cast<u16>(graph_type::aisac_control_5) &&
        type != static_cast<u16>(graph_type::aisac_control_6) &&
        type != static_cast<u16>(graph_type::aisac_control_7) &&
        type != static_cast<u16>(graph_type::aisac_control_8) &&
        type != static_cast<u16>(graph_type::aisac_control_9) &&
        type != static_cast<u16>(graph_type::aisac_control_10) &&
        type != static_cast<u16>(graph_type::aisac_control_11) &&
        type != static_cast<u16>(graph_type::aisac_control_12) &&
        type != static_cast<u16>(graph_type::aisac_control_13) &&
        type != static_cast<u16>(graph_type::aisac_control_14) &&
        type != static_cast<u16>(graph_type::aisac_control_15) &&
        type != static_cast<u16>(graph_type::voice_priority) &&
        type != static_cast<u16>(graph_type::predelay_time) &&
        type != static_cast<u16>(graph_type::biquad_gain) &&
        type != static_cast<u16>(graph_type::pan3d_center) &&
        type != static_cast<u16>(graph_type::pan3d_lfe) &&
        type != static_cast<u16>(graph_type::eg_attack_time) &&
        type != static_cast<u16>(graph_type::eg_release_time) &&
        type != static_cast<u16>(graph_type::playback_rate) &&
        type != static_cast<u16>(graph_type::pan3d_spread))
    {
        throw std::runtime_error("Unsupported CriAtom graph type");
    }

    return static_cast<graph_type>(type);
}

void read_graph_table_(
    rad::stream& stream,
    rad::allocator& tmpAllocator,
    packed_version version,
    rad::vector<graph>& graphs)
{
    assert(graphs.empty() &&
        "The given output vector must be empty"
    );

    // Read raw table header.
    utf::deserializer dr(stream, utf::deserialize_type::utf, tmpAllocator);

    // Get revision info.
    const auto revisionInfo = get_revision_info_for_version_(
        graph_revisions_,
        version
    );

    // Validate columns.
    if (!dr.has_columns_of_exact_types(revisionInfo->columnGroup, graph_columns_))
    {
        throw std::runtime_error("Invalid or unsupported ACB Graph layout");
    }

    // Read rows.
    auto& graphAllocator = graphs.allocator();
    graphs.reserve(dr.row_count());

    for (u32 i = 0; i < dr.row_count(); dr.next_row(), ++i)
    {
        // r0
        const auto type = parse_graph_type_(dr.next_cell_as_u16()); // Type
        auto& graph = graphs.emplace_back_unchecked(type, graphAllocator);

        const auto rawControls = dr.next_cell_as_buffer(); // Controls
        const auto rawDestinations = dr.next_cell_as_buffer(); // Destinations
        const auto rawCurve = dr.next_cell_as_buffer(); // Curve

        dr.skip_cell(); // ControlWorkArea
        dr.skip_cell(); // DestinationWorkArea

        // Controls
        deserialize_f32_array_(dr, rawControls, graph.controls);

        // Destinations
        deserialize_u16_array_(dr, rawDestinations, graph.destinations);

        // Curve
        deserialize_u16_array_(dr, rawCurve, graph.curves);
    }
}

void write_graph_table_(
    detail_::write_params& wp,
    const rad::vector<graph>& graphs)
{
    utf::serializer sr(*wp.stream, *wp.allocator);

    const auto revisionInfo = get_revision_info_for_version_(
        graph_revisions_,
        wp.version
    );

    sr.start(
        "Graph",
        { graph_columns_, revisionInfo->get_column_count() },
        wp.encoding
    );

    // Write rows.
    float controlWorkArea = -1.0f;
    float destinationWorkArea = 0.0f;

    for (std::size_t i = 0; i < graphs.size(); sr.next_row(), ++i)
    {
        const auto& graph = graphs[i];

        // r0
        sr.push_cell_u16(static_cast<u16>(graph.type)); // Type
        sr.push_cell_buffer(graph.controls.empty()); // Controls
        sr.push_cell_buffer(graph.destinations.empty()); // Destinations
        sr.push_cell_buffer(graph.curves.empty()); // Curve
        sr.push_cell_f32(controlWorkArea); // ControlWorkArea
        sr.push_cell_f32(destinationWorkArea); // DestinationWorkArea

        controlWorkArea -= 999.0f;
        destinationWorkArea += 1.0f;
    }

    // Write buffers.
    auto br = sr.begin_buffer_data_section();

    for (const auto& graph : graphs)
    {
        // Controls
        serialize_f32_array_(br, graph.controls);

        // Destinations
        serialize_u16_array_(br, graph.destinations);

        // Curve
        serialize_u16_array_(br, graph.curves);
    }

    // Finish writing table.
    sr.finish();
}
}
