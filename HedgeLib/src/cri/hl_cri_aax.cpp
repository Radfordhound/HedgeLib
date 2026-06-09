#include "hedgelib/cri/hl_cri_aax.h"
#include "hedgelib/cri/hl_cri_utf.h"
#include <rad/rad_stream.h>

namespace hl::cri::audio
{
static const utf::column_info aax_columns_[] =
{
    { utf::cell_type::buffer, "data" },
    { utf::cell_type::u8, "lpflg" },
};

rad::stack_or_heap_array<raw_aax_entry, 2> read_aax_entries(
    rad::stream& stream)
{
    utf::deserializer dr(stream);

    // Validate columns.
    if (!dr.has_columns_of_exact_types(aax_columns_, 0, 2))
    {
        throw std::runtime_error("Invalid or unsupported AAX layout");
    }

    // Read rows.
    rad::stack_or_heap_array<raw_aax_entry, 2> aaxEntries(
        rad::no_value_init,
        dr.row_count()
    );

    for (u32 i = 0; i < dr.row_count(); ++i)
    {
        // Read row.
        const auto rawData = dr.next_cell_as_buffer();
        const auto lpflg = dr.next_cell_as_u8();

        dr.next_row();

        // Generate aax_entry.
        aaxEntries[i].dataPosition = dr.get_buffer_data_position(rawData);
        aaxEntries[i].dataSize = rawData.size;
        aaxEntries[i].doesLoop = (lpflg != 0);
    }

    return aaxEntries;
}
}
