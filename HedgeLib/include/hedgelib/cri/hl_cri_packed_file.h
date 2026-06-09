#ifndef HL_CRI_PACKED_FILE_H_INCLUDED
#define HL_CRI_PACKED_FILE_H_INCLUDED

#include <ankerl/unordered_dense.h>

#include "../hl_internal.h"
#include <rad/rad_stream.h>
#include <rad/rad_vector.h>
#include <rad/rad_span.h>
#include <atomic>
#include <utility>
#include <unordered_set>
#include <unordered_map>
#include <cassert>

namespace hl::cri
{
constexpr u32 cpk_signature = make_sig("CPK ");

constexpr u32 toc_signature = make_sig("TOC ");

class packed_file;

class packed_file_entry
{
    friend packed_file;

    enum status_ : unsigned char
    {
        STATUS_HAS_DATA_ = 1 << 0,
        STATUS_HAS_UNCOMPRESSED_DATA_ = STATUS_HAS_DATA_ | 1 << 1,
        STATUS_LOADING_ = 1 << 2,
        STATUS_DECOMPRESSING_ = 1 << 3,
    };

    std::string_view        dirName_;
    std::string_view        name_;
    u32                     compressedSize_;
    u32                     extractSize_;

    union
    {
        unsigned long long  proxyDataOffset_;
        unsigned char*      data_;
    };
    
    std::atomic_uchar       curStatus_;

    static unsigned char* create_copy_of_data_(
        const void* data,
        u32 size
    );

public:
    u32                     id;
    rad::string             userData;

    inline std::string_view dir_name() const noexcept
    {
        return dirName_;
    }

    inline std::string_view name() const noexcept
    {
        return name_;
    }

    inline u32 extract_size() const noexcept
    {
        return extractSize_;
    }

    HL_API unsigned long long get_proxy_data_offset() const noexcept;

    /// @brief Gets a span to the data, or returns nullptr if data has not yet been loaded.
    ///
    /// @details If the data is currently being loaded or decompressed (i.e. by another
    /// thread), this function will block until that operation has completed, then return
    /// a span to the data.
    ///
    /// If the size of the span != extract_size(), the data is compressed.
    /// Call decompress_data() to decompress it.
    ///
    /// NOTE: Calling decompress_data() on an entry with compressed data will invalidate
    /// the data of any spans returned by any previous calls to get_data(), so be careful!
    ///
    /// @return A span to the current data, which may be compressed.
    HL_API rad::span<unsigned char> get_data() const noexcept;

    /// @brief Reads the data for the entry from the cpk, converting
    /// this entry from a proxy entry into a filled entry.
    ///
    /// @param cpkStream Stream to the cpk data this proxy entry was created from.
    ///
    /// @return True if the proxy data was successfully filled, false if the entry
    /// was already a filled entry, or if the entry is being filled already
    /// (i.e. on another thread).
    HL_API bool fill_proxy_data(rad::stream& cpkStream);

    /// @brief Decompresses the entry's data, or does nothing if the data is already
    /// not compressed. Call get_data() afterwards to retrieve the decompressed data.
    ///
    /// @details NOTE: If the entry contains compressed data, calling this function
    /// will invalidate the data of any spans returned by any previous calls to
    /// get_data(), so be careful!
    ///
    /// @return True if the entry's data was successfully decompressed (call
    /// get_data() afterwards to retrieve the resulting decompressed data), false if
    /// the entry's data was already not compressed, or if the entry is being
    /// decompressed already (i.e. on another thread).
    HL_API bool decompress_data();

    HL_API packed_file_entry& operator=(const packed_file_entry& other);

    HL_API packed_file_entry& operator=(packed_file_entry&& other) noexcept;

    HL_API packed_file_entry(
        u32 id,
        std::string_view dirNameView,
        std::string_view nameView,
        u32 compressedSize,
        u32 uncompressedSize,
        unsigned long long proxyDataOffset,
        rad::string userData = {}
    ) noexcept;

    HL_API packed_file_entry(
        u32 id,
        std::string_view dirNameView,
        std::string_view nameView,
        u32 compressedSize,
        u32 uncompressedSize,
        const void* compressedData,
        rad::string userData = {}
    );

    HL_API packed_file_entry(
        u32 id,
        std::string_view dirNameView,
        std::string_view nameView,
        u32 size,
        const void* data,
        rad::string userData = {}
    );

    HL_API packed_file_entry(const packed_file_entry& other);

    HL_API packed_file_entry(packed_file_entry&& other) noexcept;

    HL_API ~packed_file_entry();
};

class packed_file
{
    enum class data_read_mode_
    {
        /// @brief Read all file entries as proxy entries, which are
        /// entries that only have metadata, and do not have any actual data.
        skip,

        /// @brief Read all file entries as filled entries, which are
        /// entries that have all metadata and actual data. Load the data
        /// as it is in the CPK - compressed or uncompressed.
        load_as_is,

        /// @brief Read all file entries as filled entries, which are
        /// entries that have all metadata and actual data. If the data is
        /// compressed, also decompress it.
        load_and_decompress,
    };

    struct read_params_
    {
        rad::stream*                stream;
        data_read_mode_             dataReadMode;
        rad::vector<unsigned char>  section;
    };

    using file_entries_map_t_ = std::unordered_map<
        rad::string,
        packed_file_entry
    >;

    using dir_names_set_t_ = ankerl::unordered_dense::set<std::string_view>;

    file_entries_map_t_     fileEntries_;
    dir_names_set_t_        dirNames_;

    std::string_view insert_dir_name_(std::string_view canonicalFilePath);

    void finish_entry_initialization_(const typename file_entries_map_t_::iterator& it);

    std::pair<typename file_entries_map_t_::iterator, bool> insert_proxy_(
        rad::string canonicalPath,
        u32 id,
        u32 compressedSize,
        u32 uncompressedSize,
        unsigned long long proxyDataOffset,
        rad::string userData = {}
    );

    void read_toc_section_(read_params_& rp, unsigned long long contentPos);

public:
    using data_read_mode    = data_read_mode_;
    using const_iterator    = typename file_entries_map_t_::const_iterator;
    using iterator          = typename file_entries_map_t_::iterator;
    using value_type        = typename file_entries_map_t_::value_type;

    inline const dir_names_set_t_& dir_names() const noexcept
    {
        return dirNames_;
    }

    inline const_iterator cbegin() const noexcept
    {
        return fileEntries_.cbegin();
    }

    inline const_iterator begin() const noexcept
    {
        return fileEntries_.begin();
    }

    inline iterator begin() noexcept
    {
        return fileEntries_.begin();
    }

    inline const_iterator cend() const noexcept
    {
        return fileEntries_.cend();
    }

    inline const_iterator end() const noexcept
    {
        return fileEntries_.end();
    }

    inline iterator end() noexcept
    {
        return fileEntries_.end();
    }

    inline void reserve(std::size_t capacity)
    {
        fileEntries_.reserve(capacity);
    }

    HL_API void clear() noexcept;

    HL_API std::pair<iterator, bool> insert(
        rad::string canonicalPath,
        u32 id,
        u32 compressedSize,
        u32 uncompressedSize,
        const void* compressedData,
        rad::string userData = {}
    );

    HL_API std::pair<iterator, bool> insert(
        rad::string canonicalPath,
        u32 id,
        u32 size,
        const void* data,
        rad::string userData = {}
    );

    inline const_iterator find(const rad::string& canonicalPath) const
    {
        return fileEntries_.find(canonicalPath);
    }

    inline iterator find(const rad::string& canonicalPath)
    {
        return fileEntries_.find(canonicalPath);
    }

    inline const packed_file_entry& at(const rad::string& canonicalPath) const
    {
        return fileEntries_.at(canonicalPath);
    }

    inline packed_file_entry& at(const rad::string& canonicalPath)
    {
        return fileEntries_.at(canonicalPath);
    }

    HL_API void read(rad::stream& stream, data_read_mode dataReadMode);

    packed_file() noexcept = default;

    HL_API explicit packed_file(rad::stream& stream, data_read_mode dataReadMode);
};
}

#endif
