#include "hedgelib/hh/hl_hh_needle.h"
#include "hedgelib/io/hl_file.h"
#include "hedgelib/io/hl_path.h"
#include <cstring>

namespace hl
{
namespace hh
{
namespace needle
{
const u32& raw_archive_entry::size() const
{
    const auto ext = extension();
    return *reinterpret_cast<const u32*>(align(
        reinterpret_cast<std::uintptr_t>(
        ext + std::strlen(ext) + 1), 4));
}

u32& raw_archive_entry::size()
{
    const auto ext = extension();
    return *reinterpret_cast<u32*>(align(
        reinterpret_cast<std::uintptr_t>(
        ext + std::strlen(ext) + 1), 4));
}

void raw_archive_entry::extract(const nchar* path) const
{
    const auto fileSize = size();
    const auto fileData = data();

    file::save(fileData, fileSize, path);
}

raw_archive_entry_const_iterator& raw_archive_entry_const_iterator::operator++()
{
    const auto dataSize = m_curRawArcEntry->size();
    m_curRawArcEntry = reinterpret_cast<pointer>(
        m_curRawArcEntry->data<u8>() + dataSize);

    return *this;
}

raw_archive_entry_iterator& raw_archive_entry_iterator::operator++()
{
    const auto dataSize = m_curRawArcEntry->size();
    m_curRawArcEntry = reinterpret_cast<pointer>(
        m_curRawArcEntry->data<u8>() + dataSize);

    return *this;
}

const raw_archive_entry* raw_archive::entries_sec() const
{
    const auto ext = extension();
    return reinterpret_cast<const raw_archive_entry*>(align(
        reinterpret_cast<std::uintptr_t>(
        ext + std::strlen(ext) + 1), 4));
}

raw_archive_entry* raw_archive::entries_sec()
{
    const auto ext = extension();
    return reinterpret_cast<raw_archive_entry*>(align(
        reinterpret_cast<std::uintptr_t>(
        ext + std::strlen(ext) + 1), 4));
}
} // needle
} // hh
} // hl
