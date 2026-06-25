#include "hedgelib/hh_new/hl_hh_packed_file_info.h"
#include "hl_hh_mirage_impl.h"

namespace hl::hh_new::mirage
{
file_info packed_file_info::read_inner(
    rad::stream& stream,
    off_read_mode offsetReadMode,
    rad::allocator& tmpAllocator)
{
    clear();

    deserializer dr(stream, offsetReadMode, tmpAllocator);

    switch (dr.file_info().dataVersion)
    {
    case V0:
    {
        const auto entryCount = dr.reader().read_u32();
        dr.skip_off64_alignment();

        const auto entriesOff = dr.read_offset();

        // Read entries.
        if (entryCount)
        {
            entries.reserve(entryCount);
            dr.jump_to_offset_target(entriesOff);

            for (u32 i = 0; i < entryCount; ++i)
            {
                const auto entryOff = dr.read_offset();

                const auto curPos = dr.reader().stream().tell();
                dr.jump_to_offset_target(entryOff);

                // Read entry.
                auto& entry = entries.emplace_back_unchecked(
                    dr.read_string_at_offset(),
                    0,
                    0
                );

                entry.dataPos = dr.reader().read_u32();
                entry.dataSize = dr.reader().read_u32();

                dr.reader().stream().jump_to(curPos);
            }
        }

        break;
    }

    default:
        throw std::runtime_error("Unsupported Mirage packed file info data version");
    }

    return dr.file_info();
}

file_info packed_file_info::read(
    rad::stream& stream,
    off_read_mode offsetReadMode,
    rad::allocator& tmpAllocator)
{
    // NOTE: This function calls read_inner_
    return read_mirage_data_(*this, stream, offsetReadMode, tmpAllocator);
}

static u32 write_entry_(
    serializer& sr,
    const packed_file_entry& entry,
    u32 entryOff)
{
    sr.finish_offset(entryOff);

    const auto nameOff = sr.start_offset();
    sr.writer().write_u32(entry.dataPos);
    sr.writer().write_u32(entry.dataSize);

    return nameOff;
}

void packed_file_info::write(
    rad::stream& stream,
    file_info fileInfo,
    serialize_mode serializeMode,
    rad::allocator& tmpAllocator) const
{
    serializer sr(stream, tmpAllocator);

    sr.start(std::move(fileInfo));
    sr.start_data_section();

    if (entries.size() > UINT32_MAX) // TODO: Mark unlikely
    {
        throw std::runtime_error(
            "Mirage packed file entry count exceeds u32 range"
        );
    }

    sr.writer().write_u32(static_cast<u32>(entries.size()));
    sr.write_off64_alignment();

    const auto entriesOff = sr.start_offset();
    sr.finish_offset(entriesOff);

    if (!entries.empty())
    {
        auto entryOff = sr.start_offsets(static_cast<u32>(entries.size()));

        if (serializeMode == serialize_mode::normal_2024)
        {
            const u32 entrySize = sr.offset_size() + 8;

            // Write entries.
            auto entryPos = sr.tell_local();
            for (const auto& entry : entries)
            {
                write_entry_(sr, entry, entryOff);
                entryOff += sr.offset_size();
            }

            // Write names.
            for (const auto& entry : entries)
            {
                sr.finish_offset(entryPos);
                sr.writer().write_string(entry.name);
                entryPos += entrySize;
            }
        }
        else
        {
            for (const auto& entry : entries)
            {
                const auto nameOff = write_entry_(sr, entry, entryOff);

                sr.finish_offset(nameOff);
                sr.writer().write_string(entry.name);
                sr.pad(sr.offset_size());

                entryOff += sr.offset_size();
            }
        }
    }

    sr.finish();
}
}
