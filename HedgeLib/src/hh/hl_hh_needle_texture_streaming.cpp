#include "hedgelib/hh/hl_hh_needle_texture_streaming.h"

namespace hl
{
namespace hh
{
namespace needle
{
namespace texture_streaming
{

void raw_package_entry::fix(void* base) noexcept
{
    name.fix(base);
}

static void in_fix_raw_package(raw_package& package, bool headerAlreadySwapped)
{
#if HL_IS_BIG_ENDIAN
    if (!headerAlreadySwapped)
    {
        package.endian_swap<true>();
    }

    for (u32 i = 0; i < package.entryCount; i++)
    {
        package.entries()[i].endian_swap<true>();
    }

    for (u32 i = 0; i < package.blobCount; i++)
    {
        package.blobs()[i].endian_swap<true>();
    }
#endif

    for (u32 i = 0; i < package.entryCount; i++)
    {
        package.entries()[i].fix(&package);
    }
}

void raw_package::fix() noexcept
{
    in_fix_raw_package(*this, false);
}

readonly_package_wrapper::~readonly_package_wrapper()
{
    operator delete(m_package);
}

bool readonly_package_wrapper::try_load(hl::stream* stream)
{
    operator delete(m_package);
    m_package = nullptr;
    m_stream = stream;

    raw_package header;
    m_stream->read_obj(header);

#ifdef HL_IS_BIG_ENDIAN
    header.endian_swap();
#endif        

    if (header.signature == signature_package && header.version == version_package &&
        header.headerSize >= sizeof(header))
    {
        m_package = (raw_package*)operator new(header.headerSize);

        memcpy(m_package, &header, sizeof(header));

        const size_t remainingSize = header.headerSize - sizeof(*m_package);
        m_stream->read(remainingSize, m_package + 1);

        in_fix_raw_package(*m_package, true);
        return true;
    }

    return false;
}

const raw_package_entry* readonly_package_wrapper::get_entry(const char* str) const
{
    // Binary search in the entry array as it's guaranteed to be sorted.
    const raw_package_entry* first = m_package->entries();
    const raw_package_entry* last = first + m_package->entryCount;

    const u32 nameHash = compute_name_hash(str);

    const raw_package_entry* result = std::lower_bound(first, last, nameHash, 
        [](const raw_package_entry& entry, u32 hash)
        {
            return entry.nameHash < hash;
        });

    return result != last ? result : nullptr;
}

hl::blob readonly_package_wrapper::load_blob(u32 index) const
{
    const raw_package_blob& rawBlob = m_package->blobs()[index];
    hl::blob blob(static_cast<size_t>(rawBlob.dataSize));

    m_stream->seek(hl::seek_mode::beg, rawBlob.dataOffset);
    m_stream->read(blob.size(), blob.data());

    return blob;
}

readonly_info_wrapper::~readonly_info_wrapper()
{
    operator delete(m_info);
}

bool readonly_info_wrapper::try_load(hl::stream& stream)
{
    raw_info header;
    stream.read_obj(header);

#ifdef HL_IS_BIG_ENDIAN
    header.endian_swap();
#endif

    if (header.signature == signature_info && header.version == version_info)
    {
        const size_t remainingSize =
            header.packageNameSize + header.mip4x4Size + dds_header_max_size;

        m_info = (raw_info*)operator new(sizeof(raw_info) + remainingSize);
        memcpy(m_info, &header, sizeof(header));
        stream.read(remainingSize, m_info + 1);

        return true;
    }

    return false;
}

} // texture_streaming
} // needle
} // hh
} // hl
