#include "hedgelib/archives/hl_archive.h"
#include "hedgelib/io/hl_path.h"
#include "hedgelib/io/hl_file.h"
#include <utility>
#include <cstring>

namespace hl
{
const nchar* archive_entry::name() const noexcept
{
    return (is_reference_file()) ?
        path::get_name(m_path.get()) :
        m_path.get();
}

std::size_t archive_entry::size() const noexcept
{
    return ((is_dir()) ? dir_entries().size() : m_size);
}

archive_entry archive_entry::make_streaming_file_utf8(const char* fileName,
    std::size_t uncompressedSize, std::size_t compressedSize,
    std::uintmax_t customData)
{
    return archive_entry(
        (HL_ARC_ENTRY_IS_STREAMING_FLAG | compressedSize),
        fileName, uncompressedSize, customData);
}

archive_entry archive_entry::make_streaming_file(const nchar* fileName,
    std::size_t uncompressedSize, std::size_t compressedSize,
    std::uintmax_t customData)
{
    return archive_entry(
        (HL_ARC_ENTRY_IS_STREAMING_FLAG | compressedSize),
        fileName, uncompressedSize, customData);
}

archive_entry archive_entry::make_dir(const nchar* dirName,
    std::size_t initialEntryCount)
{
    std::vector<archive_entry>* subEntries = new std::vector<archive_entry>();
    subEntries->reserve(initialEntryCount);

    try
    {
        return archive_entry(HL_ARC_ENTRY_IS_DIR_FLAG,
            dirName, 0, subEntries);
    }
    catch (const std::exception& ex)
    {
        delete subEntries;
        throw ex;
    }
}

archive_entry archive_entry::make_regular_file_no_alloc_utf8(
    const char* fileName, std::size_t fileSize, void* data)
{
    return archive_entry(
        HL_ARC_ENTRY_NOT_OWNS_DATA_FLAG,
        fileName, fileSize, data);
}

archive_entry archive_entry::make_regular_file_utf8(const char* fileName,
    std::size_t fileSize, const void* data)
{
    // Create copy of data.
    u8* dataPtr = new u8[fileSize];
    std::memcpy(dataPtr, data, fileSize);

    // Create archive_entry and return it.
    try
    {
        return archive_entry(0,
            fileName, fileSize, dataPtr);
    }
    catch (const std::exception& ex)
    {
        delete[] dataPtr;
        throw ex;
    }
}

archive_entry archive_entry::make_regular_file_no_alloc(
    const nchar* fileName, std::size_t fileSize, void* data)
{
    return archive_entry(
        HL_ARC_ENTRY_NOT_OWNS_DATA_FLAG,
        fileName, fileSize, data);
}

archive_entry archive_entry::make_regular_file(const nchar* fileName,
    std::size_t fileSize, const void* data)
{
    // Create copy of data.
    u8* dataPtr = new u8[fileSize];
    std::memcpy(dataPtr, data, fileSize);

    // Create archive_entry and return it.
    try
    {
        return archive_entry(0,
            fileName, fileSize, dataPtr);
    }
    catch (const std::exception& ex)
    {
        delete[] dataPtr;
        throw ex;
    }
}

archive_entry archive_entry::make_file(const nchar* filePath, bool loadData)
{
    if (loadData)
    {
        // Load the file's data.
        std::size_t fileSize;
        std::unique_ptr<u8[]> fileData = file::load(filePath, &fileSize);

        // Construct a regular archive entry.
        archive_entry entry(0, path::get_name(filePath),
            fileSize, fileData.get());

        // The fileData is managed by the archive entry now, so we don't want to free it here.
        fileData.release();
        return entry;
    }
    else
    {
        // Construct a reference archive entry.
        return archive_entry::make_reference_file(filePath);
    }
}

archive_entry archive_entry::make_reference_file(const nchar* filePath)
{
    return archive_entry(0, filePath,
        path::get_size(filePath), nullptr);
}

archive_entry& archive_entry::operator=(const archive_entry& other)
{
    if (&other != this)
    {
        std::unique_ptr<nchar[]> newPath = text::make_copy(other.m_path.get());
        if (other.is_streaming_file())
        {
            m_streamingData = other.m_streamingData;
        }
        else
        {
            void* newData;
            if (other.is_dir())
            {
                newData = new std::vector<archive_entry>(other.dir_entries());
                if (owns_data()) delete &dir_entries();
            }
            else if (other.is_reference_file())
            {
                newData = nullptr;
            }
            else
            {
                newData = new u8[other.m_size];
                std::memcpy(newData, other.m_data, other.m_size);
                if (owns_data()) delete[] file_data<u8>();
            }

            m_data = newData;
        }

        m_meta = other.m_meta;
        m_path = std::move(newPath);
        m_size = other.m_size;
    }

    return *this;
}

archive_entry& archive_entry::operator=(archive_entry&& other) noexcept
{
    if (&other != this)
    {
        m_meta = other.m_meta;
        m_path = std::move(other.m_path);
        m_size = other.m_size;

        if (other.is_streaming_file())
        {
            m_streamingData = other.m_streamingData;
            other.m_streamingData = 0;
        }
        else
        {
            m_data = other.m_data;
            other.m_data = nullptr;
        }
    }

    return *this;
}

archive_entry::archive_entry(const archive_entry& other) :
    m_meta(other.m_meta), m_path(text::make_copy(other.m_path.get())),
    m_size(other.m_size)
{
    if (other.is_streaming_file())
    {
        m_streamingData = other.m_streamingData;
    }
    else if (other.is_dir())
    {
        m_data = new std::vector<archive_entry>(other.dir_entries());
    }
    else if (other.is_reference_file())
    {
        m_data = nullptr;
    }
    else
    {
        m_data = new u8[other.m_size];
        std::memcpy(m_data, other.m_data, other.m_size);
    }
}

archive_entry::archive_entry(archive_entry&& other) noexcept :
    m_meta(other.m_meta), m_path(std::move(other.m_path)),
    m_size(other.m_size)
{
    if (other.is_streaming_file())
    {
        m_streamingData = other.m_streamingData;
        other.m_streamingData = 0;
    }
    else
    {
        m_data = other.m_data;
        other.m_data = nullptr;
    }
}

archive_entry::~archive_entry()
{
    if (!is_streaming_file() && owns_data())
    {
        if (is_dir())
        {
            delete &dir_entries();
        }
        else
        {
            delete[] file_data<u8>();
        }
    }
}

static void in_archive_extract(const std::vector<archive_entry>& entries,
    bool recursive, nstring& pathBuf)
{
    // Append path combine separator if necessary.
    if (path::combine_needs_sep1(pathBuf))
    {
        pathBuf += path::separator;
    }

    // Store path buffer length for later.
    const std::size_t pathBufLen = pathBuf.length();

    // Extract entries.
    for (const auto& entry : entries)
    {
        // Skip streaming entries.
        if (entry.is_streaming_file()) continue;

        // Get entry name.
        const nchar* name = entry.name();

        // Ensure entry name doesn't begin with a path separator.
        if (!path::combine_needs_sep2(name)) ++name;

        // Append entry name to end of path buffer.
        pathBuf += name;

        // Extract regular file entries.
        if (entry.is_regular_file())
        {
            // Copy referenced files.
            if (entry.is_reference_file())
            {
                path::copy_file(entry.path(), pathBuf.c_str());
            }

            // Extract normal files.
            else
            {
                file::save(entry.file_data(), entry.size(), pathBuf);
            }
        }

        // Extract directory entries.
        else if (recursive)
        {
            // Create directory.
            path::create_dir(pathBuf);

            // Recursively extract sub-entries.
            in_archive_extract(entry.dir_entries(), recursive, pathBuf);
        }

        // Remove name from end of path.
        pathBuf.erase(pathBufLen);
    }
}

void archive_entry_list::extract(const nchar* dirPath, bool recursive) const
{
    // Ensure extraction directory exists.
    nstring pathBuf(dirPath);
    path::create_dir(pathBuf);

    // Extract files/directories as requested.
    in_archive_extract(*this, recursive, pathBuf);
}

static void in_archive_add_dir_contents(nstring& pathBuf,
    bool loadData, bool recursive, std::vector<archive_entry>& entries)
{
    // Append path combine separator if necessary.
    if (path::combine_needs_sep1(pathBuf))
    {
        pathBuf += path::separator;
    }

    // Store path buffer length for later.
    const std::size_t pathBufLen = pathBuf.length();

    // Add directory contents.
    for (auto& entry : path::dir(pathBuf))
    {
        const nchar* name = entry.name();
        const path::dir_entry_type type = entry.type();
        
        // Ensure name doesn't begin with a path separator.
        if (!path::combine_needs_sep2(name)) ++name;

        // Append name to the end of the path.
        pathBuf += name;

        // Setup files.
        if (type == path::dir_entry_type::regular)
        {
            entries.emplace_back(archive_entry::make_file(pathBuf, loadData));
        }

        // Setup directories.
        else if (recursive && type == path::dir_entry_type::directory)
        {
            // Add sub-directory entry.
            entries.emplace_back(archive_entry::make_dir(
                path::get_name(pathBuf)));

            // Recurse through sub-directory contents.
            in_archive_add_dir_contents(pathBuf, loadData,
                recursive, entries.back().dir_entries());
        }

        // Remove name from end of path.
        pathBuf.erase(pathBufLen);
    }
}

void archive_entry_list::add_dir_contents(const nchar* dirPath,
    bool loadData, bool recursive)
{
    // Add files/directories within the given directory as requested.
    nstring pathBuf(dirPath);
    in_archive_add_dir_contents(pathBuf, loadData, recursive, *this);
}
} // hl
