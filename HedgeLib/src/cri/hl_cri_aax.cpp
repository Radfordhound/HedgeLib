#include "hedgelib/cri/hl_cri_aax.h"
#include "hedgelib/cri/hl_cri_utf.h"
#include <rad/rad_stream.h>

namespace hl::cri_new::audio
{
static const utf::column_info aax_columns_[] =
{
    { utf::cell_type::buffer, "data" },
    { utf::cell_type::u8, "lpflg" },
};

rad::stack_or_heap_array<aax_entry, 2> read_aax_entries(
    rad::stream& stream)
{
    utf::table_deserializer td(stream);

    // Validate columns.
    if (!td.are_columns_exact_types(aax_columns_, 0, 2))
    {
        throw std::runtime_error("Invalid or unsupported AAX layout");
    }

    // Read rows.
    rad::stack_or_heap_array<aax_entry, 2> aaxEntries(
        rad::no_value_init,
        td.row_count()
    );

    for (u32 i = 0; i < td.row_count(); ++i)
    {
        // Read row.
        const auto rawData = td.read_cell_as_buffer();
        const auto lpflg = td.read_cell_as_u8();

        td.next_row();

        // Generate aax_entry.
        aaxEntries[i].dataPosition = td.get_buffer_data_position(rawData);
        aaxEntries[i].dataSize = rawData.size;
        aaxEntries[i].doesLoop = (lpflg != 0);
    }

    return aaxEntries;
}
}
