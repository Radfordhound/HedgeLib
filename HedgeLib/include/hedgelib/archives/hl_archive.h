/**
    @file hl_archive.h
    @brief The base classes/functions for archives in HedgeLib.
*/
#ifndef HL_ARCHIVE_H_INCLUDED
#define HL_ARCHIVE_H_INCLUDED
#include "../hl_text.h"
#include <vector>

namespace hl
{
// TODO: Make this an enum class
enum archive_entry_flags : std::size_t
{
    HL_ARC_ENTRY_IS_STREAMING_FLAG      = bit_flag(bit_count<std::size_t>() - 1U),
    HL_ARC_ENTRY_COMPRESSED_SIZE_MASK   = ~HL_ARC_ENTRY_IS_STREAMING_FLAG,
    HL_ARC_ENTRY_IS_DIR_FLAG            = bit_flag(bit_count<std::size_t>() - 2U),
    HL_ARC_ENTRY_NOT_OWNS_DATA_FLAG     = bit_flag(bit_count<std::size_t>() - 3U)
};

struct archive_entry_list;

class archive_entry
{
    friend archive_entry_list;

    /**
        @brief Various metadata related to the entry, including flags specifying what
        type of entry this is.

        If HL_ARC_ENTRY_IS_STREAMING_FLAG is set, this entry represents a file which has not
        yet been loaded into memory, and m_streamingData is format-specific information which
        can be used to quickly locate/load the data at a later time.

        Use HL_ARC_ENTRY_COMPRESSED_SIZE_MASK on m_meta when the HL_ARC_ENTRY_IS_STREAMING_FLAG
        is set to get the file's compressed size, or 0 if the data is not compressed.

        If HL_ARC_ENTRY_IS_STREAMING_FLAG is not set, and HL_ARC_ENTRY_IS_DIR_FLAG is set, this
        entry represents a directory, and m_data is a pointer to an array of archive_entry structs
        which represent the contents of the directory.

        If HL_ARC_ENTRY_IS_STREAMING_FLAG is not set, and HL_ARC_ENTRY_IS_DIR_FLAG is not
        set, this entry represents a "regular" file which is not being streamed-in from an
        archive, and m_data is either a pointer to the file's data, or null if this is a file
        reference, in which case, m_path is the absolute file path to said file on the user's
        machine.

        If HL_ARC_ENTRY_IS_STREAMING_FLAG is not set, and HL_ARC_ENTRY_NOT_OWNS_DATA_FLAG is
        set, this entry's m_data pointer is not owned by this entry, and should not be freed
        with the entry.
    */
    std::size_t m_meta;
    /**
        @brief The name of the file or directory represented by this entry, or the
        absolute path to the file if this entry is a file reference.
    */
    std::unique_ptr<nchar[]> m_path;
    /**
        @brief The uncompressed size of the file if this entry represents a file.
        Unused if this entry represents a directory.
    */
    std::size_t m_size;
    /**
        @brief Data pertaining to the actual file or directory this entry represents.

        See documentation for m_meta for specifics on what this value represents.
    */
    union
    {
        void* m_data;
        std::uintmax_t m_streamingData;
    };

#ifdef HL_IN_WIN32_UNICODE
    inline archive_entry(std::size_t meta, const char* path,
        std::size_t size, void* dataPtr) : m_meta(meta),
        m_path(text::conv_unique_ptr<text::utf8_to_native>(path)),
        m_size(size), m_data(dataPtr) {}

    inline archive_entry(std::size_t meta, const char* path,
        std::size_t size, std::uintmax_t dataNum) : m_meta(meta),
        m_path(text::conv_unique_ptr<text::utf8_to_native>(path)),
        m_size(size), m_streamingData(dataNum) {}
#endif

    inline archive_entry(std::size_t meta, const nchar* path,
        std::size_t size, void* dataPtr) : m_meta(meta),
        m_path(text::make_copy(path)), m_size(size),
        m_data(dataPtr) {}

    inline archive_entry(std::size_t meta, const nchar* path,
        std::size_t size, std::uintmax_t dataNum) : m_meta(meta),
        m_path(text::make_copy(path)), m_size(size),
        m_streamingData(dataNum) {}

public:
    inline bool is_streaming_file() const noexcept
    {
        return ((m_meta & HL_ARC_ENTRY_IS_STREAMING_FLAG) != 0);
    }

    inline bool is_dir() const noexcept
    {
        return (!is_streaming_file() && ((m_meta &
            HL_ARC_ENTRY_IS_DIR_FLAG) != 0));
    }

    inline bool is_file() const noexcept
    {
        return (is_streaming_file() || ((m_meta &
            HL_ARC_ENTRY_IS_DIR_FLAG) == 0));
    }

    inline bool is_regular_file() const noexcept
    {
        return (!is_streaming_file() && ((m_meta &
            HL_ARC_ENTRY_IS_DIR_FLAG) == 0));
    }

    inline bool is_reference_file() const noexcept
    {
        return (is_regular_file() && m_data == nullptr);
    }

    inline bool owns_data() const noexcept
    {
        return ((!is_regular_file()) ? true :
            ((m_meta & HL_ARC_ENTRY_NOT_OWNS_DATA_FLAG) == 0));
    }

    inline const nchar* path() const noexcept
    {
        return m_path.get();
    }

    HL_API const nchar* name() const noexcept;

    inline std::uintmax_t streaming_file_data() const noexcept
    {
        return m_streamingData;
    }

    inline const archive_entry_list& dir_entries() const noexcept
    {
        return *static_cast<const archive_entry_list*>(m_data);
    }

    inline archive_entry_list& dir_entries() noexcept
    {
        return *static_cast<archive_entry_list*>(m_data);
    }

    template<typename T = void>
    inline const T* file_data() const noexcept
    {
        return ((is_streaming_file()) ? nullptr :
            static_cast<const T*>(m_data));
    }

    template<typename T = void>
    inline T* file_data() noexcept
    {
        return ((is_streaming_file()) ? nullptr :
            static_cast<T*>(m_data));
    }

    HL_API std::size_t size() const noexcept;

    inline std::size_t compressed_size() const noexcept
    {
        return ((!is_streaming_file()) ? 0 :
            (m_meta & HL_ARC_ENTRY_COMPRESSED_SIZE_MASK));
    }

    HL_API static archive_entry make_streaming_file_utf8(const char* fileName,
        std::size_t uncompressedSize, std::size_t compressedSize = 0,
        std::uintmax_t customData = 0);

    inline static archive_entry make_streaming_file_utf8(const std::string& fileName,
        std::size_t uncompressedSize, std::size_t compressedSize = 0,
        std::uintmax_t customData = 0)
    {
        return make_streaming_file_utf8(fileName.c_str(),
            uncompressedSize, compressedSize, customData);
    }

    HL_API static archive_entry make_streaming_file(const nchar* fileName,
        std::size_t uncompressedSize, std::size_t compressedSize = 0,
        std::uintmax_t customData = 0);

    inline static archive_entry make_streaming_file(const nstring& fileName,
        std::size_t uncompressedSize, std::size_t compressedSize = 0,
        std::uintmax_t customData = 0)
    {
        return make_streaming_file(fileName.c_str(),
            uncompressedSize, compressedSize, customData);
    }

    /**
        @brief Constructs an archive_entry which represents a directory.
        @param[in] dirName  The name of the directory.

        @return The newly-constructed archive_entry.
        @ingroup archives
    */
    HL_API static archive_entry make_dir(const nchar* dirName,
        std::size_t initialEntryCount = 0);

    /**
        @brief Constructs an archive_entry which represents a directory.
        @param[in] dirName  The name of the directory.

        @return The newly-constructed archive_entry.
        @ingroup archives
    */
    inline static archive_entry make_dir(const nstring& dirName,
        std::size_t initialEntryCount = 0)
    {
        return make_dir(dirName.c_str(), initialEntryCount);
    }

    /**
        @brief Constructs an archive_entry which represents a file and which does
               *NOT* create its own copy of data, meaning you will have to manually
               free data yourself later.

        @param[in] fileName     The name of the file + its extension if it has one.
        @param[in] fileSize     The uncompressed size of the file, in bytes.
        @param[in] data         A pointer to the file's uncompressed data.
                                This pointer will *NOT* be automatically freed
                                when the archive_entry is destructed, so make sure
                                to manually free it later!

        @return The newly-constructed archive_entry.
        @ingroup archives
    */
    HL_API static archive_entry make_regular_file_no_alloc_utf8(
        const char* fileName, std::size_t fileSize, void* data);

    /**
        @brief Constructs an archive_entry which represents a file and which does
               *NOT* create its own copy of data, meaning you will have to manually
               free data yourself later.

        @param[in] fileName     The name of the file + its extension if it has one.
        @param[in] fileSize     The uncompressed size of the file, in bytes.
        @param[in] data         A pointer to the file's uncompressed data.
                                This pointer will *NOT* be automatically freed
                                when the archive_entry is destructed, so make sure
                                to manually free it later!

        @return The newly-constructed archive_entry.
        @ingroup archives
    */
    inline static archive_entry make_regular_file_no_alloc_utf8(
        const std::string& fileName, std::size_t fileSize, void* data)
    {
        return make_regular_file_no_alloc_utf8(fileName.c_str(), fileSize, data);
    }

    /**
        @brief Constructs an archive_entry which represents a file.
        @param[in] fileName     The name of the file + its extension if it has one.
        @param[in] fileSize     The uncompressed size of the file, in bytes.
        @param[in] data         A pointer to the file's uncompressed data.

        @return The newly-constructed archive_entry.
        @ingroup archives
    */
    HL_API static archive_entry make_regular_file_utf8(const char* fileName,
        std::size_t fileSize, const void* data);

    /**
        @brief Constructs an archive_entry which represents a file.
        @param[in] fileName     The name of the file + its extension if it has one.
        @param[in] fileSize     The uncompressed size of the file, in bytes.
        @param[in] data         A pointer to the file's uncompressed data.

        @return The newly-constructed archive_entry.
        @ingroup archives
    */
    inline static archive_entry make_regular_file_utf8(const std::string& fileName,
        std::size_t fileSize, const void* data)
    {
        return make_regular_file_utf8(fileName.c_str(), fileSize, data);
    }

    /**
        @brief Constructs an archive_entry which represents a file and which does
               *NOT* create its own copy of data, meaning you will have to manually
               free data yourself later.

        @param[in] fileName     The name of the file + its extension if it has one.
        @param[in] fileSize     The uncompressed size of the file, in bytes.
        @param[in] data         A pointer to the file's uncompressed data.
                                This pointer will *NOT* be automatically freed
                                when the archive_entry is destructed, so make sure
                                to manually free it later!

        @return The newly-constructed archive_entry.
        @ingroup archives
    */
    HL_API static archive_entry make_regular_file_no_alloc(
        const nchar* fileName, std::size_t fileSize, void* data);

    /**
        @brief Constructs an archive_entry which represents a file and which does
               *NOT* create its own copy of data, meaning you will have to manually
               free data yourself later.

        @param[in] fileName     The name of the file + its extension if it has one.
        @param[in] fileSize     The uncompressed size of the file, in bytes.
        @param[in] data         A pointer to the file's uncompressed data.
                                This pointer will *NOT* be automatically freed
                                when the archive_entry is destructed, so make sure
                                to manually free it later!

        @return The newly-constructed archive_entry.
        @ingroup archives
    */
    inline static archive_entry make_regular_file_no_alloc(
        const nstring& fileName, std::size_t fileSize, void* data)
    {
        return make_regular_file_no_alloc(fileName.c_str(), fileSize, data);
    }

    /**
        @brief Constructs an archive_entry which represents a file.
        @param[in] fileName     The name of the file + its extension if it has one.
        @param[in] fileSize     The uncompressed size of the file, in bytes.
        @param[in] data         A pointer to the file's uncompressed data.

        @return The newly-constructed archive_entry.
        @ingroup archives
    */
    HL_API static archive_entry make_regular_file(const nchar* fileName,
        std::size_t fileSize, const void* data);

    /**
        @brief Constructs an archive_entry which represents a file.
        @param[in] fileName     The name of the file + its extension if it has one.
        @param[in] fileSize     The uncompressed size of the file, in bytes.
        @param[in] data         A pointer to the file's uncompressed data.

        @return The newly-constructed archive_entry.
        @ingroup archives
    */
    inline static archive_entry make_regular_file(const nstring& fileName,
        std::size_t fileSize, const void* data)
    {
        return make_regular_file(fileName.c_str(), fileSize, data);
    }

    /**
        @brief Constructs an archive_entry which represents a file.
        @param[in] filePath     The path to the file.
        @param[in] loadData     Whether to load the data into memory.
                                If set to false, this will construct
                                a file reference instead.

        @return The newly-constructed archive_entry.
        @ingroup archives
    */
    HL_API static archive_entry make_file(const nchar* filePath,
        bool loadData = false);

    /**
        @brief Constructs an archive_entry which represents a file.
        @param[in] filePath     The path to the file.
        @param[in] loadData     Whether to load the data into memory.
                                If set to false, this will construct
                                a file reference instead.

        @return The newly-constructed archive_entry.
        @ingroup archives
    */
    inline static archive_entry make_file(const nstring& filePath,
        bool loadData = false)
    {
        return make_file(filePath.c_str(), loadData);
    }

    /**
        @brief Constructs an archive_entry which represents a reference to a file.
        @param[in] filePath     The path to the file.

        @return The newly-constructed archive_entry.
        @ingroup archives
    */
    HL_API static archive_entry make_reference_file(const nchar* filePath);

    /**
        @brief Constructs an archive_entry which represents a reference to a file.
        @param[in] filePath     The path to the file.

        @return The newly-constructed archive_entry.
        @ingroup archives
    */
    inline static archive_entry make_reference_file(const nstring& filePath)
    {
        return make_reference_file(filePath.c_str());
    }

    HL_API archive_entry& operator=(const archive_entry& other);
    HL_API archive_entry& operator=(archive_entry&& other) noexcept;

    HL_API archive_entry(const archive_entry& other);
    HL_API archive_entry(archive_entry&& other) noexcept;
    HL_API ~archive_entry();
};

struct archive_entry_list : public std::vector<archive_entry>
{
    HL_API void extract(const nchar* dirPath, bool recursive = true) const;

    inline void extract(const nstring& dirPath, bool recursive = true) const
    {
        extract(dirPath.c_str(), recursive);
    }

    inline void add_file(const nchar* filePath, bool loadData = false)
    {
        emplace_back(archive_entry::make_file(filePath, loadData));
    }

    inline void add_file(const nstring& filePath, bool loadData = false)
    {
        add_file(filePath.c_str(), loadData);
    }

    /**
        @brief Adds an archive_entry which represents a file.
        @param[in] fileName     The name of the file + its extension if it has one.
        @param[in] fileSize     The uncompressed size of the file, in bytes.
        @param[in] data         A pointer to the file's uncompressed data.

        @ingroup archives
    */
    inline void add_file(const nchar* fileName,
        std::size_t fileSize, const void* data)
    {
        emplace_back(archive_entry::make_regular_file(
            fileName, fileSize, data));
    }

    /**
        @brief Adds an archive_entry which represents a file.
        @param[in] fileName     The name of the file + its extension if it has one.
        @param[in] fileSize     The uncompressed size of the file, in bytes.
        @param[in] data         A pointer to the file's uncompressed data.

        @ingroup archives
    */
    inline void add_file(const nstring& fileName,
        std::size_t fileSize, const void* data)
    {
        emplace_back(archive_entry::make_regular_file(
            fileName, fileSize, data));
    }

    /**
        @brief Adds an archive_entry which represents a file.
        @param[in] fileName     The name of the file + its extension if it has one.
        @param[in] fileSize     The uncompressed size of the file, in bytes.
        @param[in] data         A pointer to the file's uncompressed data.

        @ingroup archives
    */
    inline void add_file_utf8(const char* fileName,
        std::size_t fileSize, const void* data)
    {
        emplace_back(archive_entry::make_regular_file_utf8(
            fileName, fileSize, data));
    }

    /**
        @brief Adds an archive_entry which represents a file.
        @param[in] fileName     The name of the file + its extension if it has one.
        @param[in] fileSize     The uncompressed size of the file, in bytes.
        @param[in] data         A pointer to the file's uncompressed data.

        @ingroup archives
    */
    inline void add_file_utf8(const std::string& fileName,
        std::size_t fileSize, const void* data)
    {
        emplace_back(archive_entry::make_regular_file_utf8(
            fileName, fileSize, data));
    }

    HL_API void add_dir_contents(const nchar* dirPath,
        bool loadData = false, bool recursive = true);

    inline void add_dir_contents(const nstring& dirPath,
        bool loadData = false, bool recursive = true)
    {
        add_dir_contents(dirPath.c_str(), loadData, recursive);
    }

    inline archive_entry_list() = default;
    
    inline archive_entry_list(const nchar* dirPath,
        bool loadData = false, bool recursive = true)
    {
        add_dir_contents(dirPath, loadData, recursive);
    }

    inline archive_entry_list(const nstring& dirPath,
        bool loadData = false, bool recursive = true)
    {
        add_dir_contents(dirPath, loadData, recursive);
    }
};

using archive = archive_entry_list;

struct packed_file_entry
{
    /** @brief The name this entry represents. */
    std::string name;
    /** @brief The absolute position of the file within the packed data (e.g. within the .pfd). */
    std::size_t dataPos;
    /** @brief The size of the file within the packed data (e.g. within the .pfd). */
    std::size_t dataSize;

    inline packed_file_entry(const char* name,
        std::size_t dataPos, std::size_t dataSize) :
        name(name), dataPos(dataPos), dataSize(dataSize) {}

    inline packed_file_entry(const std::string& name,
        std::size_t dataPos, std::size_t dataSize) :
        name(name), dataPos(dataPos), dataSize(dataSize) {}

    inline packed_file_entry(std::string&& name,
        std::size_t dataPos, std::size_t dataSize) :
        name(name), dataPos(dataPos), dataSize(dataSize) {}
};

using packed_file_info = std::vector<packed_file_entry>;
} // hl
#endif
