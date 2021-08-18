#ifndef HL_HH_MIRAGE_H_INCLUDED
#define HL_HH_MIRAGE_H_INCLUDED
#include "hl_stream.h"

namespace hl
{
namespace hh
{
namespace mirage
{
constexpr bool needs_endian_swap =
#ifdef HL_IS_BIG_ENDIAN
    false;
#else
    true;
#endif

enum class header_type
{
    /** @brief The "standard" mirage header used in all HH1 games. */
    standard = 0,

    /**
        @brief The V1 sample chunk header used in all HH1/HH2 games
        from Sonic Lost World, up to, and including, Sonic Forces.
    */
    sample_chunk_v1,

    /**
        @brief The V2 sample chunk header used in all HH2 games
        from M&S Tokyo 2020 and beyond.
    */
    sample_chunk_v2
};


class const_off_table_handle
{
protected:
    const u32* m_offTable;
    u32 m_offCount;

public:
    inline const u32* off_table() noexcept
    {
        return m_offTable;
    }

    inline u32 off_count() const noexcept
    {
        return m_offCount;
    }

    inline const u32* begin() const noexcept
    {
        return m_offTable;
    }

    inline const u32* end() const noexcept
    {
        return (m_offTable + m_offCount);
    }

    inline const_off_table_handle(const u32* offTable, u32 offCount) noexcept :
        m_offTable(offTable), m_offCount(offCount) {}
};

struct off_table_handle : public const_off_table_handle
{
    inline u32* off_table() noexcept
    {
        return const_cast<u32*>(m_offTable);
    }

    inline u32* begin() noexcept
    {
        return const_cast<u32*>(m_offTable);
    }

    inline u32* end() noexcept
    {
        return const_cast<u32*>(m_offTable + m_offCount);
    }

    inline off_table_handle(u32* offTable, u32 offCount) noexcept :
        const_off_table_handle(offTable, offCount) {}
};

struct raw_header
{
    u32 fileSize;
    u32 version;
    u32 dataSize;
    off32<void> data;
    off32<u32> offTable;
    off32<u32> fileName;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(fileSize);
        hl::endian_swap(version);
        hl::endian_swap(dataSize);
        hl::endian_swap<swapOffsets>(data);
        hl::endian_swap<swapOffsets>(offTable);
        hl::endian_swap<swapOffsets>(fileName);
    }

    const_off_table_handle offsets() const noexcept
    {
        return const_off_table_handle(offTable.get() + 1, *offTable);
    }

    off_table_handle offsets() noexcept
    {
        return off_table_handle(offTable.get() + 1, *offTable);
    }

    HL_API void fix();

    HL_API static void start_write(u32 version, stream& stream);

    HL_API static void finish_write(std::size_t headerPos,
        off_table& offTable, stream& stream,
        const char* fileName = nullptr);

    inline static void finish_write(std::size_t headerPos,
        off_table& offTable, stream& stream,
        const std::string& fileName)
    {
        finish_write(headerPos, offTable, stream, fileName.c_str());
    }
};

HL_STATIC_ASSERT_SIZE(raw_header, 0x18);

namespace sample_chunk
{
constexpr u32 raw_header_magic = 0x133054AU;

enum class node_flags : u32
{
    /* Masks */
    flags_mask = 0xE0000000U,
    size_mask = 0x1FFFFFFFU,

    /* Flags */
    none = 0,
    is_leaf = 0x20000000U,
    is_last_child = 0x40000000U,
    is_root = 0x80000000U,
    is_last_or_root = (is_root | is_last_child)
};

HL_ENUM_CLASS_DEF_BITWISE_OPS(node_flags)

struct raw_node
{
    /** @brief See hl::hh::mirage::sample_chunk::node_flags. */
    u32 flags;
    u32 value;
    char name[8];

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(flags);
        hl::endian_swap(value);
    }

    constexpr u32 size() const noexcept
    {
        return (flags & static_cast<u32>(node_flags::size_mask));
    }

    constexpr bool is_leaf() const noexcept
    {
        return ((static_cast<node_flags>(flags) &
            node_flags::is_leaf) != node_flags::none);
    }

    constexpr bool is_last_child() const noexcept
    {
        return ((static_cast<node_flags>(flags) &
            node_flags::is_last_child) != node_flags::none);
    }

    constexpr bool is_root() const noexcept
    {
        return ((static_cast<node_flags>(flags) &
            node_flags::is_root) != node_flags::none);
    }

    constexpr bool is_last_or_root() const noexcept
    {
        return ((static_cast<node_flags>(flags) &
            node_flags::is_last_or_root) != node_flags::none);
    }

    inline const raw_node* children() const noexcept
    {
        return (is_leaf()) ? nullptr : (this + 1);
    }

    inline raw_node* children() noexcept
    {
        return (is_leaf()) ? nullptr : (this + 1);
    }

    const raw_node* next() const noexcept
    {
        return (is_last_or_root()) ? nullptr :
            ptradd<raw_node>(this, size());
    }

    raw_node* next() noexcept
    {
        return (is_last_or_root()) ? nullptr :
            ptradd<raw_node>(this, size());
    }

    HL_API const raw_node* next_of_name(const char* name) const;

    inline const raw_node* next_of_name(const std::string& name) const
    {
        return next_of_name(name.c_str());
    }

    inline raw_node* next_of_name(const char* name)
    {
        return const_cast<raw_node*>(const_cast<const raw_node*>(
            this)->next_of_name(name));
    }

    inline raw_node* next_of_name(const std::string& name)
    {
        return next_of_name(name.c_str());
    }

    template<typename T = void>
    inline const T* data() const noexcept
    {
        return reinterpret_cast<const T*>(this + 1);
    }

    template<typename T = void>
    inline T* data() noexcept
    {
        return reinterpret_cast<T*>(this + 1);
    }

    HL_API const raw_node* get_child(const char* name, bool recursive = true) const;

    inline raw_node* get_child(const char* name, bool recursive = true)
    {
        return const_cast<raw_node*>(const_cast<const raw_node*>(
            this)->get_child(name, recursive));
    }

    inline const raw_node* get_child(const std::string& name,
        bool recursive = true) const
    {
        return get_child(name.c_str(), recursive);
    }

    inline raw_node* get_child(const std::string& name, bool recursive = true)
    {
        return get_child(name.c_str(), recursive);
    }

    HL_API static void start_write(const char* name, stream& stream, u32 value = 1);

    inline static void start_write(const std::string& name, stream& stream, u32 value = 1)
    {
        start_write(name.c_str(), stream, value);
    }

    HL_API static void finish_write(std::size_t nodePos,
        node_flags flags, stream& stream);
};

HL_STATIC_ASSERT_SIZE(raw_node, 16);

struct raw_header
{
    u32 fileSize;
    /** @brief Not checked by the actual games from what we can tell. */
    u32 magic;
    off32<u32> offTable;
    u32 offCount;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(fileSize);
        hl::endian_swap(magic);
        hl::endian_swap<swapOffsets>(offTable);
        hl::endian_swap(offCount);
    }

    constexpr bool has_nodes() const noexcept
    {
        return ((fileSize & static_cast<u32>(node_flags::is_leaf)) == 0);
    }

    inline const raw_node* nodes() const noexcept
    {
        return (has_nodes()) ? reinterpret_cast<const raw_node*>(this + 1) : nullptr;
    }

    inline raw_node* nodes() noexcept
    {
        return (has_nodes()) ? reinterpret_cast<raw_node*>(this + 1) : nullptr;
    }

    HL_API const raw_node* get_node(const char* name, bool recursive = true) const;

    inline raw_node* get_node(const char* name, bool recursive = true)
    {
        return const_cast<raw_node*>(const_cast<const raw_header*>(
            this)->get_node(name, recursive));
    }

    inline const raw_node* get_node(const std::string& name,
        bool recursive = true) const
    {
        return get_node(name.c_str(), recursive);
    }

    inline raw_node* get_node(const std::string& name, bool recursive = true)
    {
        return get_node(name.c_str(), recursive);
    }

    inline const_off_table_handle offsets() const noexcept
    {
        return const_off_table_handle(offTable.get(), offCount);
    }

    inline off_table_handle offsets() noexcept
    {
        return off_table_handle(offTable.get(), offCount);
    }

    HL_API void fix();

    HL_API static void start_write(stream& stream);

    HL_API static void finish_write(std::size_t headerPos,
        std::size_t dataPos, off_table& offTable, stream& stream);
};

HL_STATIC_ASSERT_SIZE(raw_header, 16);

class property
{
    char m_name[9];

public:
    u32 value = 0;

    inline const char* get_name() const noexcept
    {
        return m_name;
    }

    inline char* get_name() noexcept
    {
        return m_name;
    }

    HL_API void set_name(const char* name);
    
    inline void set_name(const std::string& name)
    {
        set_name(name.c_str());
    }

    HL_API property() noexcept;
    HL_API property(const char* name, u32 value = 0);

    inline property(const std::string& name, u32 value = 0) :
        property(name.c_str(), value) {}

    HL_API property(const raw_node& node);
};

inline void fix(void* rawData)
{
    raw_header* sampleChunkHeader = static_cast<raw_header*>(rawData);
    sampleChunkHeader->fix();
}

template<typename T = void>
const T* get_data(const void* rawData, u32* version = nullptr)
{
    // Get contexts node; return null if there was none.
    const raw_header* sampleChunkHeader = static_cast<const raw_header*>(rawData);
    const raw_node* contextsNode = sampleChunkHeader->get_node("Contexts");
    if (!contextsNode) return nullptr;

    // Get version number if requested.
    if (version) *version = contextsNode->value;

    // Get data pointer and return it.
    return contextsNode->data<T>();
}

template<typename T = void>
T* get_data(void* rawData, u32* version = nullptr)
{
    return const_cast<T*>(get_data<T>(const_cast<
        const void*>(rawData), version));
}

HL_API void make_node_name(const char* name, char* dst);
} // sample_chunk

inline bool has_sample_chunk_header_fixed(const void* rawData)
{
    // Check if the first uint in the data has the is_root flag set.
    return ((*static_cast<const u32*>(rawData) &
        static_cast<u32>(sample_chunk::node_flags::is_root)) != 0);
}

inline bool has_sample_chunk_header_unfixed(const void* rawData)
{
#ifdef HL_IS_BIG_ENDIAN
    return has_sample_chunk_header_fixed(rawData);
#else
    return ((*static_cast<const u32*>(rawData) & 0x80U) != 0);
#endif
}

HL_API void fix(void* rawData);

template<typename T = void>
const T* get_data(const void* rawData, u32* version = nullptr)
{
    /* Sample chunk header */
    if (has_sample_chunk_header_fixed(rawData))
    {
        return sample_chunk::get_data<T>(rawData, version);
    }

    /* Standard header */
    else
    {
        // Get version number if requested.
        const raw_header* hhHeader = static_cast<const raw_header*>(rawData);
        if (version) *version = hhHeader->version;

        // Get data pointer and return it.
        return static_cast<const T*>(hhHeader->data.get());
    }
}

template<typename T = void>
T* get_data(void* rawData, u32* version = nullptr)
{
    return const_cast<T*>(get_data<T>(const_cast<
        const void*>(rawData), version));
}

HL_API void offsets_fix(off_table_handle offsets, void* base);
HL_API void offsets_write_no_sort(std::size_t dataPos,
    const off_table& offTable, stream& stream);

HL_API void offsets_write(std::size_t dataPos,
    off_table& offTable, stream& stream);
} // mirage
} // hh
} // hl
#endif
