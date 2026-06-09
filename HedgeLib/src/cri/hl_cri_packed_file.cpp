#include "hedgelib/cri/hl_cri_packed_file.h"
#include "hedgelib/cri/hl_cri_utf.h"
#include "hedgelib/io/hl_endian_readers.h"
#include <rad/rad_path_unix.h>
#include <rad/rad_memory_stream.h>
#include <cstring>

namespace hl::cri
{
static const utf::column_info cpk_header_columns_[] =
{
    { utf::cell_type::u64, "UpdateDateTime" },
    { utf::cell_type::u64, "FileSize" },
    { utf::cell_type::u64, "ContentOffset" },
    { utf::cell_type::u64, "ContentSize" },
    { utf::cell_type::u64, "TocOffset" },
    { utf::cell_type::u64, "TocSize" },
    { utf::cell_type::u32, "TocCrc" },
    { utf::cell_type::u64, "EtocOffset" },
    { utf::cell_type::u64, "EtocSize" },
    { utf::cell_type::u64, "ItocOffset" },
    { utf::cell_type::u64, "ItocSize" },
    { utf::cell_type::u32, "ITocCrc" },
    { utf::cell_type::u64, "GtocOffset" },
    { utf::cell_type::u64, "GtocSize" },
    { utf::cell_type::u32, "GTocCrc" },
    { utf::cell_type::u64, "EnabledPackedSize" },
    { utf::cell_type::u64, "EnabledDataSize" },
    { utf::cell_type::u64, "TotalDataSize" },
    { utf::cell_type::u32, "Tocs" },
    { utf::cell_type::u32, "Files" },
    { utf::cell_type::u32, "Groups" },
    { utf::cell_type::u32, "Attrs" },
    { utf::cell_type::u32, "TotalFiles" },
    { utf::cell_type::u32, "Directories" },
    { utf::cell_type::u32, "Updates" },
    { utf::cell_type::u16, "Version" },
    { utf::cell_type::u16, "Revision" },
    { utf::cell_type::u16, "Align" },
    { utf::cell_type::u16, "Sorted" },
    { utf::cell_type::u16, "EID" },
    { utf::cell_type::u32, "CpkMode" },
    { utf::cell_type::string, "Tvers" },
    { utf::cell_type::string, "Comment" },
    { utf::cell_type::u32, "Codec" },
    { utf::cell_type::u32, "DpkItoc" },

    // TODO: new format version (forces+ ?)
    // EnableTocCrc
    // EnableFileCrc
    // CrcMode
    // CrcTable
};

static const utf::column_info toc_info_columns_[] =
{
    { utf::cell_type::string, "DirName" },
    { utf::cell_type::string, "FileName" },
    { utf::cell_type::u32, "FileSize" },
    { utf::cell_type::u32, "ExtractSize" },
    { utf::cell_type::u64, "FileOffset" },
    { utf::cell_type::u32, "ID" },
    { utf::cell_type::string, "UserString" },
};

unsigned char* packed_file_entry::create_copy_of_data_(
    const void* data,
    u32 size)
{
    const auto dataCopy = new unsigned char[size];
    std::memcpy(dataCopy, data, size);
    return dataCopy;
}

unsigned long long packed_file_entry::get_proxy_data_offset() const noexcept
{
    return (curStatus_.load(std::memory_order_relaxed) == 0) ?
        proxyDataOffset_ : 0;
}

rad::span<unsigned char> packed_file_entry::get_data() const noexcept
{
    // Block until any in-progress loads/decompresses complete.
    while (curStatus_.load(std::memory_order_relaxed) &
        (STATUS_LOADING_ | STATUS_DECOMPRESSING_))
    {
    }
    
    // Return the data.
    const auto status = curStatus_.load(std::memory_order_acquire);

    if (status & STATUS_HAS_DATA_)
    {
        const auto size = (status & STATUS_HAS_UNCOMPRESSED_DATA_) ?
            extractSize_ : compressedSize_;

        return { data_, size };
    }
    else
    {
        return nullptr;
    }
}

bool packed_file_entry::fill_proxy_data(rad::stream& cpkStream)
{
    // Do nothing if this is not a proxy entry.
    unsigned char expectedStatus = 0;
    if (!curStatus_.compare_exchange_strong(
        expectedStatus,
        STATUS_LOADING_,
        std::memory_order_relaxed))
    {
        return false;
    }
    
    // Read data from CPK.
    cpkStream.jump_to(proxyDataOffset_);

    std::unique_ptr<unsigned char[]> data(new unsigned char[compressedSize_]);
    cpkStream.read(data.get(), compressedSize_);

    // Store data in entry.
    data_ = data.release();

    curStatus_.store(
        (compressedSize_ == extractSize_) ?
            STATUS_HAS_UNCOMPRESSED_DATA_ :
            STATUS_HAS_DATA_,
        std::memory_order_release
    );

    return true;
}

bool packed_file_entry::decompress_data()
{
    // Do nothing if we don't have compressed data.
    unsigned char expectedStatus = STATUS_HAS_DATA_;
    if (!curStatus_.compare_exchange_strong(
        expectedStatus,
        STATUS_DECOMPRESSING_,
        std::memory_order_acquire))
    {
        return false;
    }
    
    // Decompress data.
    // TODO
    throw std::runtime_error("Not yet implemented");

    //// Store decompressed data in entry.
    //curStatus_.store(
        //STATUS_HAS_DATA_,
        //std::memory_order_release
    //);

    //return true;
}

packed_file_entry& packed_file_entry::operator=(const packed_file_entry& other)
{
    if (&other != this)
    {
        dirName_ = other.dirName_;
        name_ = other.name_;
        compressedSize_ = other.compressedSize_;
        extractSize_ = other.extractSize_;
        id = other.id;
        userData = other.userData;

        const auto curStatus = other.curStatus_.load(std::memory_order_relaxed);

        if (curStatus == 0)
        {
            proxyDataOffset_ = other.proxyDataOffset_;

            curStatus_.store(0, std::memory_order_release);
        }
        else
        {
            const auto data = other.get_data();
            
            // NOTE: This line is unsafe if other is filled or decompressed
            // while this line is running (invalid usage!), as the old data
            // might be invalidated while the copy is occurring.
            data_ = create_copy_of_data_(data.data(), data.size());

            curStatus_.store(
                (data.size() == other.extractSize_) ?
                    STATUS_HAS_UNCOMPRESSED_DATA_ :
                    STATUS_HAS_DATA_,
                std::memory_order_release
            );
        }
    }

    return *this;
}

packed_file_entry& packed_file_entry::operator=(packed_file_entry&& other) noexcept
{
    if (&other != this)
    {
        dirName_ = other.dirName_;
        name_ = other.name_;
        compressedSize_ = other.compressedSize_;
        extractSize_ = other.extractSize_;
        id = other.id;
        userData = std::move(other.userData);

        const auto curStatus = other.curStatus_.load(std::memory_order_relaxed);

        if (curStatus == 0)
        {
            proxyDataOffset_ = other.proxyDataOffset_;

            curStatus_.store(0, std::memory_order_release);
        }
        else
        {
            const auto data = other.get_data();
            
            // NOTE: This line is unsafe if other is filled or decompressed
            // while this line is running (invalid usage!), as the old data
            // pointer might be invalidated afterwards.
            data_ = data.data();

            curStatus_.store(
                (data.size() == other.extractSize_) ?
                    STATUS_HAS_UNCOMPRESSED_DATA_ :
                    STATUS_HAS_DATA_,
                std::memory_order_release
            );

            // NOTE: The following lines are unsafe if other is filled or decompressed
            // while they are running (invalid usage!), as other's data pointer might
            // be replaced while they are running.
            other.data_ = nullptr;
            other.curStatus_.store(
                STATUS_HAS_UNCOMPRESSED_DATA_,
                std::memory_order_release
            );
        }
    }

    return *this;
}

packed_file_entry::packed_file_entry(
    u32 id,
    std::string_view dirNameView,
    std::string_view nameView,
    u32 compressedSize,
    u32 uncompressedSize,
    unsigned long long proxyDataOffset,
    rad::string userData) noexcept
    : dirName_(dirNameView)
    , name_(nameView)
    , compressedSize_(compressedSize)
    , extractSize_(uncompressedSize)
    , proxyDataOffset_(proxyDataOffset)
    , curStatus_(0)
    , id(id)
    , userData(std::move(userData))
{
}

packed_file_entry::packed_file_entry(
    u32 id,
    std::string_view dirNameView,
    std::string_view nameView,
    u32 compressedSize,
    u32 uncompressedSize,
    const void* compressedData,
    rad::string userData)
    : dirName_(dirNameView)
    , name_(nameView)
    , compressedSize_(compressedSize)
    , extractSize_(uncompressedSize)
    , data_(create_copy_of_data_(compressedData, compressedSize))
    , curStatus_((compressedSize == uncompressedSize) ?
        STATUS_HAS_UNCOMPRESSED_DATA_ :
        STATUS_HAS_DATA_)
    , id(id)
    , userData(std::move(userData))
{
}

packed_file_entry::packed_file_entry(
    u32 id,
    std::string_view dirNameView,
    std::string_view nameView,
    u32 size,
    const void* data,
    rad::string userData)
    : dirName_(dirNameView)
    , name_(nameView)
    , compressedSize_(size)
    , extractSize_(size)
    , data_(create_copy_of_data_(data, size))
    , curStatus_(STATUS_HAS_UNCOMPRESSED_DATA_)
    , id(id)
    , userData(std::move(userData))
{
}

packed_file_entry::packed_file_entry(const packed_file_entry& other)
    : dirName_(other.dirName_)
    , name_(other.name_)
    , compressedSize_(other.compressedSize_)
    , extractSize_(other.extractSize_)
    , curStatus_(0)
    , id(other.id)
    , userData(other.userData)
{
    const auto curStatus = other.curStatus_.load(std::memory_order_relaxed);

    if (curStatus == 0)
    {
        proxyDataOffset_ = other.proxyDataOffset_;
    }
    else
    {
        const auto data = other.get_data();
        
        // NOTE: This line is unsafe if other is filled or decompressed
        // while this line is running (invalid usage!), as the old data
        // might be invalidated while the copy is occurring.
        data_ = create_copy_of_data_(data.data(), data.size());

        curStatus_.store(
            (data.size() == other.extractSize_) ?
                STATUS_HAS_UNCOMPRESSED_DATA_ :
                STATUS_HAS_DATA_,
            std::memory_order_relaxed
        );
    }
}

packed_file_entry::packed_file_entry(packed_file_entry&& other) noexcept
    : dirName_(other.dirName_)
    , name_(other.name_)
    , compressedSize_(other.compressedSize_)
    , extractSize_(other.extractSize_)
    , curStatus_(0)
    , id(other.id)
    , userData(std::move(other.userData))
{
    const auto curStatus = other.curStatus_.load(std::memory_order_relaxed);

    if (curStatus == 0)
    {
        proxyDataOffset_ = other.proxyDataOffset_;
    }
    else
    {
        const auto data = other.get_data();
        
        // NOTE: This line is unsafe if other is filled or decompressed
        // while this line is running (invalid usage!), as the old data
        // pointer might be invalidated afterwards.
        data_ = data.data();

        curStatus_.store(
            (data.size() == other.extractSize_) ?
                STATUS_HAS_UNCOMPRESSED_DATA_ :
                STATUS_HAS_DATA_,
            std::memory_order_relaxed
        );

        // NOTE: The following lines are unsafe if other is filled or decompressed
        // while they are running (invalid usage!), as other's data pointer might
        // be replaced while they are running.
        other.data_ = nullptr;
        other.curStatus_.store(
            STATUS_HAS_UNCOMPRESSED_DATA_,
            std::memory_order_release
        );
    }
}

packed_file_entry::~packed_file_entry()
{
    const auto data = get_data();
    if (data)
    {
        delete[] data.data();
    }
}

std::string_view packed_file::insert_dir_name_(
    std::string_view canonicalFilePath)
{
    auto dirName = rad::path::get_parent_unix(canonicalFilePath);
    rad::path::remove_trailing_separators_unix(dirName);
    
    dirNames_.insert(dirName);

    return dirName;
}

void packed_file::finish_entry_initialization_(
    const typename file_entries_map_t_::iterator& it)
{
    it->second.name_ = rad::path::get_name_unix(it->first);

    try
    {
        it->second.dirName_ = insert_dir_name_(it->first);
    }
    catch (...)
    {
        // NOTE: This is noexcept in most implementations, but
        // it's not guaranteed by the standard. In the rare event
        // that this is not noexcept and it throws an exception,
        // we're left with an entry with a null dirName_ field
        // in the fileEntries_ map.
        fileEntries_.erase(it);
        throw;
    }
}

auto packed_file::insert_proxy_(
    rad::string canonicalPath,
    u32 id,
    u32 compressedSize,
    u32 uncompressedSize,
    unsigned long long proxyDataOffset,
    rad::string userData)
    -> std::pair<iterator, bool>
{
    const auto p = fileEntries_.try_emplace(
        std::move(canonicalPath),
        id,
        std::string_view{},
        std::string_view{},
        compressedSize,
        uncompressedSize,
        proxyDataOffset,
        std::move(userData)
    );

    if (p.second)
    {
        finish_entry_initialization_(p.first);
    }
    
    return p;
}

static void decrypt_section_(rad::span<unsigned char> data)
{
    // This function is based off of the implementation used by CriPakTools:
    // https://github.com/wmltogether/CriPakTools/blob/ab58c3d23035c54fd9321e28e556c39652a83136/LibCPK/CPK.cs#L590

    const long t = 0x00004115;
    long m = 0x0000655f;

    for (auto& v : data)
    {
        v ^= static_cast<unsigned char>(m & 0xFF);
        m *= t;
    }
}

static void read_section_(
    rad::stream& stream,
    u32 expectedSig,
    rad::vector<unsigned char>& section)
{
    // Read section header.
    io::little_endian_reader reader(stream);
    
    if (reader.read_u32() != expectedSig)
    {
        throw std::runtime_error("Unsupported CPK data format");
    }

    const auto unknown1 = reader.read_u32(); // unknown1
    const auto dataSize = reader.read_u32();
    reader.read_u32(); // unknown2

    // Read section data.
    section.resize(rad::no_value_init, dataSize);
    stream.read(section.data(), dataSize);

    // Decrypt section data if necessary.
    if (unknown1 == 0) // TODO: Is this check correct??
    {
        decrypt_section_(section);
    }
}

void packed_file::read_toc_section_(read_params_& rp, unsigned long long contentPos)
{
    // Read section.
    read_section_(*rp.stream, toc_signature, rp.section);

    rad::readonly_memory_stream sectionStream(rp.section);
    utf::deserializer dr(sectionStream);

    // Validate columns.
    if (!dr.has_columns_of_exact_types(
        toc_info_columns_,
        0,
        static_cast<u16>(std::size(toc_info_columns_))))
    {
        throw std::runtime_error("Invalid or unsupported CpkTocInfo layout");
    }

    // Parse rows.
    reserve(dr.row_count());

    for (u32 i = 0; i < dr.row_count(); ++i)
    {
        // Read row.
        const auto rawDirName = dr.next_cell_as_string();
        const auto rawFileName = dr.next_cell_as_string();
        const auto fileSize = dr.next_cell_as_u32();
        const auto extractSize = dr.next_cell_as_u32();
        const auto fileOffset = dr.next_cell_as_u64();
        const auto id = dr.next_cell_as_u32();
        const auto rawUserString = dr.next_cell_as_string();

        dr.next_row();

        // Add entry.
        auto canonicalPath = rad::path::combine_unix(
            dr.get_string_data(rawDirName),
            dr.get_string_data(rawFileName)
        );

        const auto p = insert_proxy_(
            rad::string(canonicalPath),
            id,
            fileSize,
            extractSize,
            contentPos + fileOffset,
            dr.get_string_data(rawUserString)
        );

        switch (rp.dataReadMode)
        {
        case data_read_mode::skip:
            // Do nothing more.
            break;

        case data_read_mode::load_as_is:
            // Fill proxy data.
            p.first->second.fill_proxy_data(*rp.stream);
            break;

        case data_read_mode::load_and_decompress:
            // Fill proxy data.
            p.first->second.fill_proxy_data(*rp.stream);

            // Decompress proxy data if necessary.
            p.first->second.decompress_data();
            break;

        default:
            throw std::runtime_error("Invalid or unsupported data_read_mode");
        }
    }
}

void packed_file::clear() noexcept
{
    dirNames_.clear();
    fileEntries_.clear();
}

auto packed_file::insert(
    rad::string canonicalPath,
    u32 id,
    u32 compressedSize,
    u32 uncompressedSize,
    const void* compressedData,
    rad::string userData)
    -> std::pair<iterator, bool>
{
    const auto p = fileEntries_.try_emplace(
        std::move(canonicalPath),
        id,
        std::string_view{},
        std::string_view{},
        compressedSize,
        uncompressedSize,
        compressedData,
        std::move(userData)
    );

    if (p.second)
    {
        finish_entry_initialization_(p.first);
    }
    
    return p;
}

auto packed_file::insert(
    rad::string canonicalPath,
    u32 id,
    u32 size,
    const void* data,
    rad::string userData)
    -> std::pair<iterator, bool>
{
    const auto p = fileEntries_.try_emplace(
        std::move(canonicalPath),
        id,
        std::string_view{},
        std::string_view{},
        size,
        data,
        std::move(userData)
    );

    if (p.second)
    {
        finish_entry_initialization_(p.first);
    }
    
    return p;
}

struct cpk_header_
{
    u64 updateDateTime = 0;
    u64 fileSize = 0;
    u64 contentOffset = 0;
    u64 contentSize = 0;
    u64 tocOffset = 0;
    u64 tocSize = 0;
    u32 tocCrc = 0;
    u64 etocOffset = 0;
    u64 etocSize = 0;
    u64 itocOffset = 0;
    u64 itocSize = 0;
    u32 iTocCrc = 0;
    u64 gtocOffset = 0;
    u64 gtocSize = 0;
    u32 gTocCrc = 0;
    u64 enabledPackedSize = 0;
    u64 enabledDataSize = 0;
    u64 totalDataSize = 0;
    u32 tocs = 0;
    u32 files = 0;
    u32 groups = 0;
    u32 attrs = 0;
    u32 totalFiles = 0;
    u32 directories = 0;
    u32 updates = 0;
    u16 version = 0;
    u16 revision = 0;
    u16 align = 0;
    u16 sorted = 0;
    u16 eID = 0;
    u32 cpkMode = 0;
    rad::string tvers;
    rad::string comment;
    u32 codec = 0;
    u32 dpkItoc = 0;

    cpk_header_(rad::stream& stream, rad::vector<unsigned char>& section)
    {
        // Read section.
        read_section_(stream, cpk_signature, section);

        rad::readonly_memory_stream sectionStream(section);
        utf::deserializer dr(sectionStream);

        // Validate columns.
        if (dr.row_count() != 1)
            //dr.column_count() < std::size(cpk_header_columns_) ||
            //!dr.are_columns_exact_types(
            //cpk_header_columns_,
            //0,
            //std::size(cpk_header_columns_)))
        {
            throw std::runtime_error("Invalid or unsupported CpkHeader layout");
        }

        // Read fields.
        if (dr.try_go_to_cell("UpdateDateTime"))
        {
            updateDateTime = dr.next_cell_as_u64();
        }

        if (dr.try_go_to_cell("FileSize"))
        {
            fileSize = dr.next_cell_as_u64();
        }

        if (dr.try_go_to_cell("ContentOffset"))
        {
            contentOffset = dr.next_cell_as_u64();
        }

        if (dr.try_go_to_cell("ContentSize"))
        {
            contentSize = dr.next_cell_as_u64();
        }

        if (dr.try_go_to_cell("TocOffset"))
        {
            tocOffset = dr.next_cell_as_u64();
        }

        if (dr.try_go_to_cell("TocSize"))
        {
            tocSize = dr.next_cell_as_u64();
        }

        if (dr.try_go_to_cell("TocCrc"))
        {
            tocCrc = dr.next_cell_as_u32();
        }

        if (dr.try_go_to_cell("EtocOffset"))
        {
            etocOffset = dr.next_cell_as_u64();
        }

        if (dr.try_go_to_cell("EtocSize"))
        {
            etocSize = dr.next_cell_as_u64();
        }

        if (dr.try_go_to_cell("ItocOffset"))
        {
            itocOffset = dr.next_cell_as_u64();
        }

        if (dr.try_go_to_cell("ItocSize"))
        {
            itocSize = dr.next_cell_as_u64();
        }

        if (dr.try_go_to_cell("ItocCrc"))
        {
            iTocCrc = dr.next_cell_as_u32();
        }

        if (dr.try_go_to_cell("GtocOffset"))
        {
            gtocOffset = dr.next_cell_as_u64();
        }

        if (dr.try_go_to_cell("GtocSize"))
        {
            gtocSize = dr.next_cell_as_u64();
        }

        if (dr.try_go_to_cell("GtocCrc"))
        {
            gTocCrc = dr.next_cell_as_u32();
        }

        if (dr.try_go_to_cell("EnabledPackedSize"))
        {
            enabledPackedSize = dr.next_cell_as_u64();
        }

        if (dr.try_go_to_cell("EnabledDataSize"))
        {
            enabledDataSize = dr.next_cell_as_u64();
        }

        if (dr.try_go_to_cell("TotalDataSize"))
        {
            totalDataSize = dr.next_cell_as_u64();
        }

        if (dr.try_go_to_cell("Tocs"))
        {
            tocs = dr.next_cell_as_u32();
        }

        if (dr.try_go_to_cell("Files"))
        {
            files = dr.next_cell_as_u32();
        }

        if (dr.try_go_to_cell("Groups"))
        {
            groups = dr.next_cell_as_u32();
        }

        if (dr.try_go_to_cell("Attrs"))
        {
            attrs = dr.next_cell_as_u32();
        }

        if (dr.try_go_to_cell("TotalFiles"))
        {
            totalFiles = dr.next_cell_as_u32();
        }

        if (dr.try_go_to_cell("Directories"))
        {
            directories = dr.next_cell_as_u32();
        }

        if (dr.try_go_to_cell("Updates"))
        {
            updates = dr.next_cell_as_u32();
        }

        if (dr.try_go_to_cell("Version"))
        {
            version = dr.next_cell_as_u16();
        }

        if (dr.try_go_to_cell("Revision"))
        {
            revision = dr.next_cell_as_u16();
        }

        if (dr.try_go_to_cell("Align"))
        {
            align = dr.next_cell_as_u16();
        }

        if (dr.try_go_to_cell("Sorted"))
        {
            sorted = dr.next_cell_as_u16();
        }

        if (dr.try_go_to_cell("EID"))
        {
            eID = dr.next_cell_as_u16();
        }

        if (dr.try_go_to_cell("CpkMode"))
        {
            cpkMode = dr.next_cell_as_u32();
        }

        if (dr.try_go_to_cell("Tvers"))
        {
            const auto rawTvers = dr.next_cell_as_string();
            tvers = dr.get_string_data(rawTvers);
        }

        if (dr.try_go_to_cell("Comment"))
        {
            const auto rawComment = dr.next_cell_as_string();
            comment = dr.get_string_data(rawComment);
        }

        if (dr.try_go_to_cell("Codec"))
        {
            codec = dr.next_cell_as_u32();
        }

        if (dr.try_go_to_cell("DpkItoc"))
        {
            dpkItoc = dr.next_cell_as_u32();
        }

        // TODO: New format version.
    }
};

void packed_file::read(rad::stream& stream, data_read_mode dataReadMode)
{
    clear();

    // Setup read parameters.
    read_params_ rp;
    rp.stream = &stream;
    rp.dataReadMode = dataReadMode;

    // Read header.
    const cpk_header_ cpkHeader(stream, rp.section);

    // Read TOC.
    stream.jump_to(cpkHeader.tocOffset);

    read_toc_section_(rp, std::min<>(cpkHeader.contentOffset, cpkHeader.tocOffset));
        //(cpkHeader.contentOffset) ?
        //(cpkHeader.version > 7 || (cpkHeader.version == 7 && cpkHeader.revision >= 2)) ?
        //cpkHeader.contentOffset : cpkHeader.tocOffset
    
    // TODO
}

packed_file::packed_file(rad::stream& stream, data_read_mode dataReadMode)
{
    read(stream, dataReadMode);
}
}
