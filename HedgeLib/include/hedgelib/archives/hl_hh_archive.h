#ifndef HL_HH_ARCHIVE_H_INCLUDED
#define HL_HH_ARCHIVE_H_INCLUDED
#include "hl_archive.h"
#include "../hl_compression.h"
#include"../io/hl_stream.h"

namespace hl
{
namespace hh
{
namespace arl
{
constexpr u32 sig = make_sig("ARL2");
constexpr const nchar* const ext = HL_NTEXT(".arl");

struct header
{
    /** @brief "ARL2" */
    u32 signature;
    /** @brief The total number of split archives. */
    u32 splitCount;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(signature);
        hl::endian_swap(splitCount);
    }
};

HL_STATIC_ASSERT_SIZE(header, 8);
} // arl

namespace ar
{
constexpr u32 default_split_limit   = 0xA01000U;
constexpr u32 default_alignment     = 0x40U;

constexpr const nchar* const ext = HL_NTEXT(".ar");

struct header
{
    /** @brief Always 0? */
    u32 unknown1;
    /** @brief Size of this header. Always 0x10. The game doesn't seem to use it. */
    u32 headerSize;
    /** @brief Size of a single file entry struct. Always 0x14. The game doesn't seem to use it. */
    u32 entrySize;
    /** @brief The multiple all the data within the archive is aligned to. */
    u32 dataAlignment;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(unknown1);
        hl::endian_swap(headerSize);
        hl::endian_swap(entrySize);
        hl::endian_swap(dataAlignment);
    }

    HL_API void fix(std::size_t hhArcSize);

    HL_API void parse(std::size_t hhArcSize,
        archive_entry_list& hlArc) const;
};

HL_STATIC_ASSERT_SIZE(header, 16);

struct file_entry
{
    /** @brief The complete size of this entry, including padding and the file's data. */
    u32 entrySize;
    /** @brief The size of this file's data. */
    u32 dataSize;
    /** @brief The offset to this file's data relative to the beginning of this struct. */
    u32 dataOffset;
    /** @brief Hash or date? Unleashed/Gens don't care if you set this to 0. */
    u32 unknown1;
    /** @brief Hash or date? Unleashed/Gens don't care if you set this to 0. */
    u32 unknown2;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(entrySize);
        hl::endian_swap(dataSize);
        hl::endian_swap(dataOffset);
        hl::endian_swap(unknown1);
        hl::endian_swap(unknown2);
    }

    inline const char* name() const noexcept
    {
        return reinterpret_cast<const char*>(this + 1);
    }

    inline const void* data() const noexcept
    {
        return ptradd(this, dataOffset);
    }

    inline void* data() noexcept
    {
        return ptradd(this, dataOffset);
    }
};

HL_STATIC_ASSERT_SIZE(file_entry, 0x14);

HL_API nstring get_root_path(const nchar* filePath);
HL_API nstring get_root_path(const nstring& filePath);

inline void fix(void* hhArc, std::size_t hhArcSize)
{
    header* ar = static_cast<header*>(hhArc);
    ar->fix(hhArcSize);
}

inline void fix(blob& hhArc)
{
    fix(hhArc.data(), hhArc.size());
}

inline void parse(const void* hhArc, std::size_t hhArcSize,
    archive_entry_list& hlArc)
{
    const header* ar = static_cast<const header*>(hhArc);
    ar->parse(hhArcSize, hlArc);
}

inline void parse(const blob& hhArc, archive_entry_list& hlArc)
{
    parse(hhArc.data(), hhArc.size(), hlArc);
}

HL_API void read(blob& hhArc, archive_entry_list* hlArc,
    std::vector<blob>* hhArcs = nullptr);

HL_API void load_single(const nchar* filePath,
    archive_entry_list* hlArc,
    std::vector<blob>* hhArcs = nullptr);

inline void load_single(const nstring& filePath,
    archive_entry_list* hlArc,
    std::vector<blob>* hhArcs = nullptr)
{
    load_single(filePath.c_str(), hlArc, hhArcs);
}

inline archive load_single(const nchar* filePath)
{
    archive hlArc;
    load_single(filePath, &hlArc);
    return hlArc;
}

inline archive load_single(const nstring& filePath)
{
    return load_single(filePath.c_str());
}

HL_API void load(const nchar* filePath,
    archive_entry_list* hlArc,
    std::vector<blob>* hhArcs = nullptr);

HL_API void load(const nstring& filePath,
    archive_entry_list* hlArc,
    std::vector<blob>* hhArcs = nullptr);

inline archive load(const nchar* filePath)
{
    archive hlArc;
    load(filePath, &hlArc);
    return hlArc;
}

inline archive load(const nstring& filePath)
{
    return load(filePath.c_str());
}

HL_API void save(const archive_entry_list& arc,
    const nchar* filePath, u32 splitLimit = default_split_limit,
    u32 dataAlignment = default_alignment, compress_type compressType = compress_type::none,
    bool generateARL = true, packed_file_info* pfi = nullptr);

inline void save(const archive_entry_list& arc,
    const nstring& filePath, u32 splitLimit = default_split_limit,
    u32 dataAlignment = default_alignment, compress_type compressType = compress_type::none,
    bool generateARL = true, packed_file_info* pfi = nullptr)
{
    save(arc, filePath.c_str(), splitLimit, dataAlignment,
        compressType, generateARL, pfi);
}
} // ar

namespace pfi
{
constexpr const nchar* const ext = HL_NTEXT(".pfi");

struct file_entry
{
    /** @brief Offset to the name of the file this entry represents. */
    off32<char> name;
    /** @brief The absolute position of the file within the packed data (e.g. within the .pfd). */
    u32 dataPos;
    /** @brief The size of the file within the packed data (e.g. within the .pfd). */
    u32 dataSize;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap(dataPos);
        hl::endian_swap(dataSize);
    }
};

HL_STATIC_ASSERT_SIZE(file_entry, 12);

namespace v0
{
struct header
{
    arr32<off32<file_entry>> entries;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(entries);
    }

    HL_API void fix();
    HL_API void parse(packed_file_info& hlPfi) const;
};

HL_STATIC_ASSERT_SIZE(header, 8);

inline void fix(void* hhPfi)
{
    header* pfi = static_cast<header*>(hhPfi);
    pfi->fix();
}

inline void parse(const void* hhPfi, packed_file_info& hlPfi)
{
    const header* pfi = static_cast<const header*>(hhPfi);
    pfi->parse(hlPfi);
}

HL_API void write(const packed_file_info& pfi,
    std::size_t dataPos, off_table& offTable,
    stream& stream);
} // v0

HL_API void read(void* hhPfi, packed_file_info& hlPfi);

inline void load(const nchar* filePath, packed_file_info& hlPfi)
{
    // Load data into blob.
    blob pfi(filePath);

    // Read data and parse it as necessary.
    read(pfi, hlPfi);
}

inline void load(const nstring& filePath,
    packed_file_info& hlPfi)
{
    load(filePath.c_str(), hlPfi);
}

inline packed_file_info load(const nchar* filePath)
{
    packed_file_info hlPfi;
    load(filePath, hlPfi);
    return hlPfi;
}

inline packed_file_info load(const nstring& filePath)
{
    return load(filePath.c_str());
}

HL_API void write(const packed_file_info& pfi,
    u32 version, std::size_t dataPos, off_table& offTable,
    stream& stream);

HL_API void save(const packed_file_info& pfi,
    u32 version, const nchar* filePath);

inline void save(const packed_file_info& pfi,
    u32 version, const nstring& filePath)
{
    save(pfi, version, filePath.c_str());
}
} // pfi

namespace pfd
{
constexpr u32 default_alignment = 0x800U;

constexpr const nchar* const ext = HL_NTEXT(".pfd");

inline void load(const nchar* filePath,
    archive_entry_list* hlArc,
    std::vector<blob>* hhArcs = nullptr)
{
    ar::load_single(filePath, hlArc, hhArcs);
}

inline void load(const nstring& filePath,
    archive_entry_list* hlArc,
    std::vector<blob>* hhArcs = nullptr)
{
    load(filePath.c_str(), hlArc, hhArcs);
}

inline archive load(const nchar* filePath)
{
    return ar::load_single(filePath);
}

inline archive load(const nstring& filePath)
{
    return load(filePath.c_str());
}

inline void save(const archive_entry_list& arc,
    const nchar* filePath, u32 dataAlignment = default_alignment,
    packed_file_info* pfi = nullptr)
{
    ar::save(arc, filePath, 0, dataAlignment,
        compress_type::none, false, pfi);
}

inline void save(const archive_entry_list& arc,
    const nstring& filePath, u32 dataAlignment = default_alignment,
    packed_file_info* pfi = nullptr)
{
    save(arc, filePath.c_str(), dataAlignment, pfi);
}
} // pfd
} // hh
} // hl
#endif
