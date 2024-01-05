#ifndef HL_PACX_H_INCLUDED
#define HL_PACX_H_INCLUDED
#include "hl_archive.h"
#include "../hl_blob.h"
#include "../io/hl_bina.h"
#include "../hl_compression.h"

namespace hl
{
namespace pacx
{
constexpr u32 sig = make_sig("PACx");
constexpr bina::ver ver_201 = bina::ver('2', '0', '1');
constexpr bina::ver ver_301 = bina::ver('3', '0', '1');
constexpr bina::ver ver_402 = bina::ver('4', '0', '2');
constexpr bina::ver ver_403 = bina::ver('4', '0', '3');

constexpr const nchar* const ext = HL_NTEXT(".pac");
HL_API extern const char* const data_types[];
HL_API extern const std::size_t data_type_count;

enum class supported_ext_kind : unsigned short
{
    /** @brief This type can appear in root PACs. */
    root = 1,
    /** @brief This type can appear in split PACs. */
    split = 2,
    /** @brief This type can appear in both root and split PACs. */
    mixed = (root | split),
    /**
        @brief The BINA string and offset tables of this type are to be "merged"
        into the global string and offset tables of the PAC file(s).
    */
    v2_merged = (root | 4)
};

struct supported_ext
{
    /**
        @brief A lowercased extension, without a dot at the beginning, that
        is supported/used by this version of PACx (e.g. "dds").
    */
    const nchar* ext;
    /**
        @brief The index of the PACx data type for this extension
        (e.g. "ResTexture") within the data_types array.
    */
    unsigned short dataTypeIndex;
    /** @brief What kind of type this is. See supported_ext_kind values for details. */
    supported_ext_kind kind;
    /**
        @brief A number indicating how this type will be sorted in
        root pacs; smaller == higher priority.
    */
    short rootSortWeight;
    /**
        @brief A number indicating how this type will be sorted in
        split pacs; smaller == higher priority.
    */
    short splitSortWeight;

    constexpr bool is_root_type() const noexcept
    {
        return (kind == supported_ext_kind::root ||
            kind == supported_ext_kind::v2_merged);
    }

    constexpr bool is_split_type() const noexcept
    {
        return (kind == supported_ext_kind::split ||
            kind == supported_ext_kind::mixed);
    }

    inline const char* data_type() const noexcept
    {
        return data_types[dataTypeIndex];
    }
};

HL_API extern const supported_ext lw_exts[];
HL_API extern const std::size_t lw_ext_count;

HL_API extern const supported_ext rio_exts[];
HL_API extern const std::size_t rio_ext_count;

HL_API extern const supported_ext forces_exts[];
HL_API extern const std::size_t forces_ext_count;

HL_API extern const supported_ext tokyo1_exts[];
HL_API extern const std::size_t tokyo1_ext_count;

HL_API extern const supported_ext tokyo2_exts[];
HL_API extern const std::size_t tokyo2_ext_count;

HL_API extern const supported_ext sakura_exts[];
HL_API extern const std::size_t sakura_ext_count;

HL_API extern const supported_ext ppt2_exts[];
HL_API extern const std::size_t ppt2_ext_count;

HL_API extern const supported_ext hite_exts[];
HL_API extern const std::size_t hite_ext_count;

HL_API extern const supported_ext rangers_exts[];
HL_API extern const std::size_t rangers_ext_count;

namespace v2
{
constexpr u32 default_split_limit = 0xA00000U;
constexpr u32 default_alignment = 16;

struct proxy_entry
{
    /**
        @brief A string formatted as such: "ext:restype", where "ext"
        is the file's extension without the dot at the beginning, and "restype"
        is the game's internal name used to represent data of this type.

        For example: "dds:ResTexture", or "material:ResMirageMaterial".
    */
    off32<char> type;
    /** @brief A string representing the proxy file's name without its extension. */
    off32<char> name;
    /**
        @brief Index of the corresponding dic_node within the
        dic for the type specified by "type".
    */
    u32 nodeIndex;

    template<bool swapOffsets = true>
    inline void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(type);
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap(nodeIndex);
    }
};

HL_STATIC_ASSERT_SIZE(proxy_entry, 12);

using proxy_table = arr32<proxy_entry>;

HL_STATIC_ASSERT_SIZE(proxy_table, 8);

enum class data_flags : u8
{
    none = 0,
    /**
        @brief Indicates that this is a "proxy entry" - that is, an entry which
        doesn't actually contain any data. The actual data is contained within
        a separate data_entry located in a separate "split" packfile.
    */
    not_here = 0x80
};

struct data_entry
{
    /**
        @brief The size of the data contained within this entry, including
        padding, starting immediately after this data_entry struct.
    */
    u32 dataSize;
    /** @brief Set internally by the game. Always 0 in actual files. */
    u32 dataPtr;
    /** @brief Always 0? */
    u32 unknown1;
    /** @brief See hl::pacx::v2::data_flags. */
    u8 flags;
    /** @brief Set internally by the game. Always 0 in actual files. */
    u8 status;
    /** @brief Always 0? */
    u16 unknown2;

    template<bool swapOffsets = true>
    inline void endian_swap() noexcept
    {
        hl::endian_swap(dataSize);
    }

    inline bool is_proxy_entry() const noexcept
    {
        return ((flags & static_cast<u8>(data_flags::not_here)) != 0);
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

    HL_API bool has_merged_bina_data(bina::off_table_handle::iterator beg,
        const bina::off_table_handle::iterator& end, const void* base) const noexcept;
};

HL_STATIC_ASSERT_SIZE(data_entry, 16);

struct dep_info
{
    /** @brief The filename + extension of this "dependency" packfile. */
    off32<char> name;

    template<bool swapOffsets = true>
    inline void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
    }
};

HL_STATIC_ASSERT_SIZE(dep_info, 4);

struct dep_table
{
    off32<dep_info> deps;
    u32 depCount;

    template<bool swapOffsets = true>
    inline void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(deps);
        hl::endian_swap(depCount);
    }

    inline const dep_info* begin() const noexcept
    {
        return deps.get();
    }

    inline dep_info* begin() noexcept
    {
        return deps.get();
    }

    inline const dep_info* end() const noexcept
    {
        return &deps[depCount];
    }

    inline dep_info* end() noexcept
    {
        return &deps[depCount];
    }

    inline const dep_info& operator[](std::size_t i) const noexcept
    {
        return deps[i];
    }

    inline dep_info& operator[](std::size_t i) noexcept
    {
        return deps[i];
    }
};

HL_STATIC_ASSERT_SIZE(dep_table, 8);

template<typename T>
struct dic_node
{
    off32<char> name;
    off32<T> data;

    template<bool swapOffsets = true>
    inline void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap<swapOffsets>(data);
    }
};

template<typename node_t>
using dic = arr32<node_t>;

using file_dic_node = dic_node<data_entry>;
HL_STATIC_ASSERT_SIZE(file_dic_node, 8);

using file_dic = dic<file_dic_node>;
HL_STATIC_ASSERT_SIZE(file_dic, 8);

struct type_dic_node : dic_node<file_dic>
{
    HL_API const char* type_sep() const;
    HL_API std::string ext() const;
    HL_API const char* res_type() const;

    HL_API bool is_of_type(const char* type) const;

    inline bool is_of_type(const std::string& type) const
    {
        return is_of_type(type.c_str());
    }

    HL_API bool is_dep_table() const;
};

HL_STATIC_ASSERT_SIZE(type_dic_node, 8);

struct type_dic : dic<type_dic_node>
{
    HL_API const file_dic* files_of_type(const char* type) const noexcept;

    inline file_dic* files_of_type(const char* type) noexcept
    {
        return const_cast<file_dic*>(const_cast<const type_dic*>(
            this)->files_of_type(type));
    }

    inline const file_dic* files_of_type(const std::string& type) const noexcept
    {
        return files_of_type(type.c_str());
    }

    inline file_dic* files_of_type(const std::string& type) noexcept
    {
        return files_of_type(type.c_str());
    }
};

HL_STATIC_ASSERT_SIZE(type_dic, 8);

struct block_data_header
{
    /** @brief Used to determine what type of block this is. See hl::bina::v2::block_type. */
    u32 signature;
    /** @brief The complete size of the block, including this header. */
    u32 size;
    /**
        @brief The combined size, in bytes, of every data_entry
        (and its corresponding data) in the file, including padding.
    */
    u32 dataEntriesSize;
    /**
       @brief The combined size, in bytes, of every dic
       (and its corresponding dic_nodes) in the file.
    */
    u32 dicsSize;
    /**
        @brief The combined size, in bytes, of the proxy_table and
        every proxy_entry in the file, including padding.
    */
    u32 proxiesSize;
    /** @brief The size of the string table in bytes, including padding. */
    u32 strTableSize;
    /** @brief The size of the offset table in bytes, including padding. */
    u32 offTableSize;
    /** @brief Always 1? Probably a boolean? */
    u8 unknown1;
    /** @brief Included so garbage data doesn't get writtten. */
    u8 padding1;
    /** @brief Included so garbage data doesn't get writtten. */
    u16 padding2;

    template<bool swapOffsets = true>
    inline void endian_swap() noexcept
    {
        hl::endian_swap(size);
        hl::endian_swap(dataEntriesSize);
        hl::endian_swap(dicsSize);
        hl::endian_swap(proxiesSize);
        hl::endian_swap(strTableSize);
        hl::endian_swap(offTableSize);
    }

    inline const type_dic& types() const noexcept
    {
        return *reinterpret_cast<const type_dic*>(this + 1);
    }

    inline type_dic& types() noexcept
    {
        return *reinterpret_cast<type_dic*>(this + 1);
    }

    inline const data_entry* data_entries() const noexcept
    {
        return ptradd<data_entry>(&types(), dicsSize);
    }

    inline data_entry* data_entries() noexcept
    {
        return ptradd<data_entry>(&types(), dicsSize);
    }

    inline const proxy_table* proxies() const noexcept
    {
        return ptradd<proxy_table>(data_entries(), dataEntriesSize);
    }

    inline proxy_table* proxies() noexcept
    {
        return ptradd<proxy_table>(data_entries(), dataEntriesSize);
    }

    inline const char* str_table() const noexcept
    {
        return ptradd<char>(proxies(), proxiesSize);
    }

    inline char* str_table() noexcept
    {
        return ptradd<char>(proxies(), proxiesSize);
    }

    inline const u8* off_table() const noexcept
    {
        return ptradd(str_table(), strTableSize);
    }

    inline u8* off_table() noexcept
    {
        return ptradd(str_table(), strTableSize);
    }

    inline bina::off_table_handle offsets() const noexcept
    {
        return bina::off_table_handle(off_table(), offTableSize);
    }

    HL_API void fix(bina::endian_flag endianFlag, void* header);
    HL_API void parse(const void* header, bina::endian_flag endianFlag,
        archive_entry_list& hlArc, bool skipProxies = true) const;

    HL_API static void start_write(stream& stream);

    HL_API static void finish_write(std::size_t dataBlockPos,
        std::size_t typesPos, std::size_t dataEntriesPos,
        std::size_t proxyTablePos, std::size_t strTablePos,
        std::size_t offTablePos, bina::endian_flag endianFlag,
        stream& stream);

    HL_API static void finish_write(std::size_t headerPos,
        std::size_t dataBlockPos, std::size_t typesPos,
        std::size_t dataEntriesPos, std::size_t proxyTablePos,
        bina::endian_flag endianFlag, const hl::str_table& strTable,
        hl::off_table& offTable, stream& stream);
};

HL_STATIC_ASSERT_SIZE(block_data_header, 0x20);

struct header
{
    /** @brief "PACx" signature. */
    u32 signature;
    /** @brief Version number. */
    bina::ver version;
    /** @brief 'B' for Big Endian, 'L' for Little Endian. See hl::bina::endian_flag. */
    u8 endianFlag;
    /** @brief The size of the entire file, including this header. */
    u32 fileSize;
    /** @brief How many blocks are in the file. */
    u16 blockCount;
    /** @brief Included so garbage data doesn't get writtten. */
    u16 padding;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(fileSize);
        hl::endian_swap(blockCount);
    }

    inline bina::endian_flag endian_flag() const noexcept
    {
        return static_cast<bina::endian_flag>(endianFlag);
    }

    inline const bina::v2::raw_block_header* first_block() const noexcept
    {
        return reinterpret_cast<const bina::v2::raw_block_header*>(this + 1);
    }

    inline bina::v2::raw_block_header* first_block() noexcept
    {
        return reinterpret_cast<bina::v2::raw_block_header*>(this + 1);
    }

    inline bina::v2::const_block_iterator blocks() const noexcept
    {
        return bina::v2::const_block_iterator(first_block(), blockCount);
    }

    inline bina::v2::block_iterator blocks() noexcept
    {
        return bina::v2::block_iterator(first_block(), blockCount);
    }

    HL_API const bina::v2::raw_block_header* get_block(
        bina::v2::raw_block_type type) const noexcept;

    inline bina::v2::raw_block_header* get_block(
        bina::v2::raw_block_type type) noexcept
    {
        return const_cast<bina::v2::raw_block_header*>(const_cast<
            const header*>(this)->get_block(type));
    }

    inline const block_data_header* get_data_block() const noexcept
    {
        return reinterpret_cast<const block_data_header*>(
            get_block(bina::v2::raw_block_type::data));
    }

    inline block_data_header* get_data_block() noexcept
    {
        return reinterpret_cast<block_data_header*>(
            get_block(bina::v2::raw_block_type::data));
    }

    HL_API void fix();
    HL_API void parse(archive_entry_list& hlArc, bool skipProxies = true) const;

    HL_API static void start_write(bina::ver version,
        bina::endian_flag endianFlag, stream& stream);

    HL_API static void finish_write(std::size_t headerPos, u16 blockCount,
        bina::endian_flag endianFlag, stream& stream);
};

HL_STATIC_ASSERT_SIZE(header, 16);

inline const block_data_header* get_data_block(const void* pac)
{
    const header* headerPtr = static_cast<const header*>(pac);
    return headerPtr->get_data_block();
}

inline void fix(void* pac)
{
    header* headerPtr = static_cast<header*>(pac);
    headerPtr->fix();
}

inline void parse(const void* pac, archive_entry_list& hlArc,
    bool skipProxies = true)
{
    const header* headerPtr = static_cast<const header*>(pac);
    headerPtr->parse(hlArc, skipProxies);
}

HL_API void read(blob& pac, archive_entry_list* hlArc,
    std::vector<blob>* pacs = nullptr);

inline void load_single(const nchar* filePath, archive_entry_list* hlArc,
    std::vector<blob>* pacs = nullptr)
{
    // Load data into blob.
    blob pac(filePath);

    // Read data and parse it as necessary.
    read(pac, hlArc, pacs);
}

inline void load_single(const nstring& filePath, archive_entry_list* hlArc,
    std::vector<blob>* pacs = nullptr)
{
    load_single(filePath.c_str(), hlArc, pacs);
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

HL_API void load(const nchar* filePath, archive_entry_list* hlArc,
    std::vector<blob>* pacs = nullptr);

inline void load(const nstring& filePath, archive_entry_list* hlArc,
    std::vector<blob>* pacs = nullptr)
{
    load(filePath.c_str(), hlArc, pacs);
}

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

HL_API void save(const archive_entry_list& arc, bina::endian_flag endianFlag,
    const supported_ext* exts, const std::size_t extCount,
    const nchar* filePath, u32 splitLimit = default_split_limit,
    u32 dataAlignment = default_alignment, packed_file_info* pfi = nullptr);

inline void save(const archive_entry_list& arc, bina::endian_flag endianFlag,
    const supported_ext* exts, const std::size_t extCount,
    const nstring& filePath, u32 splitLimit = default_split_limit,
    u32 dataAlignment = default_alignment, packed_file_info* pfi = nullptr)
{
    save(arc, endianFlag, exts, extCount,
        filePath, splitLimit, dataAlignment, pfi);
}
} // v2

namespace v3
{
constexpr u32 default_split_limit = 0x1E00000U;
constexpr u32 default_alignment = 16;

// Thanks to Skyth for cracking the majority of the PACxV3 format!
struct dep_info
{
    /** @brief The filename + extension of this "dependency" packfile. */
    off64<char> name;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
    }
};

HL_STATIC_ASSERT_SIZE(dep_info, 8);

using dep_table = arr64<dep_info>;
HL_STATIC_ASSERT_SIZE(dep_table, 16);

enum class data_flags : u64
{
    regular_file = 0,
    not_here = 1,
    bina_file = 2
};

HL_ENUM_CLASS_DEF_BITWISE_OPS(data_flags)

struct data_entry
{
    /**
        @brief Same as hl::pacx::v3::header::uid in PACxV3.
        Different number for each data entry in PACxV4.
    */
    u32 uid;
    u32 dataSize;
    /** @brief Always 0? unknown1 from v2::data_entry?? */
    u64 unknown2;
    off64<void> data;
    /** @brief Always 0? unknown2 from v2::data_entry?? */
    u64 unknown3;
    off64<char> ext;
    /**
        @brief See hl::pacx::v3::data_flags.
        Probably actually just a single byte with 7 bytes of padding??
    */
    u64 flags;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(uid);
        hl::endian_swap(dataSize);
        hl::endian_swap(unknown2);
        hl::endian_swap<swapOffsets>(data);
        hl::endian_swap(unknown3);
        hl::endian_swap<swapOffsets>(ext);
        hl::endian_swap(flags);
    }

    inline bool is_proxy_entry() const noexcept
    {
        return ((flags & static_cast<u64>(data_flags::not_here)) != 0);
    }

    inline bool is_bina_file() const noexcept
    {
        return ((flags & static_cast<u64>(data_flags::bina_file)) != 0);
    }
};

HL_STATIC_ASSERT_SIZE(data_entry, 0x30);

template<typename T>
struct node
{
    off64<char> name;
    off64<T> data;
    off64<s32> childIndices;
    s32 parentIndex;
    s32 globalIndex;
    s32 dataIndex;
    u16 childCount;
    u8 hasData;
    /** @brief Where this node's name should be copied to within the global name buffer. */
    u8 bufStartIndex;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap<swapOffsets>(data);
        hl::endian_swap<swapOffsets>(childIndices);
        hl::endian_swap(parentIndex);
        hl::endian_swap(globalIndex);
        hl::endian_swap(dataIndex);
        hl::endian_swap(childCount);
    }

    inline const node<T>* find_child(const char* name, const node<T>* nodes) const
    {
        const s32* curChildIndices = childIndices.get();
        u16 i = 0, curChildCount = childCount;

        while (i < curChildCount)
        {
            // Skip nodes without a name.
            const node<T>& curNode = nodes[curChildIndices[i]];
            const char* curNodeName = curNode.name.get();
            if (!curNodeName) continue;

            // If the node name partially matches, recurse through its child nodes.
            const std::size_t curNodeNameLen = text::len(curNodeName);
            const int compResult = text::compare(name,
                curNodeName, curNodeNameLen);

            if (!compResult)
            {
                // Increase name pointer.
                name += curNodeNameLen;

                // If the name *entirely* matched, return the node pointer.
                if (*name == '\0') return &curNode;

                // Recurse through this child node's children instead.
                curChildIndices = curNode.childIndices.get();
                curChildCount = curNode.childCount;
                i = 0;
            }

            // OPTIMIZATION: Return null early if the first non-matching character
            // of the current node's name if greater than the first character of name.
            // We can safely do this with all valid PACxV3 data as all valid PACx3 data
            // is alphabetized.

            // TODO: Ensure this is *really* safe to do and actually implement this.

            // Increment i.
            ++i;
        }

        return nullptr;
    }

    inline const node<T>* find_child(const std::string& name) const
    {
        return find_child(name.c_str());
    }
};

template<typename node_t>
struct node_tree
{
    u32 nodeCount;
    u32 dataNodeCount;
    off64<node_t> nodes;
    off64<s32> dataNodeIndices;

    template<bool swapOffsets = true>
    inline void endian_swap() noexcept
    {
        hl::endian_swap(nodeCount);
        hl::endian_swap(dataNodeCount);
        hl::endian_swap<swapOffsets>(nodes);
        hl::endian_swap<swapOffsets>(dataNodeIndices);
    }

    inline const node_t* begin() const noexcept
    {
        return nodes.get();
    }

    inline node_t* begin() noexcept
    {
        return nodes.get();
    }

    inline const node_t* end() const noexcept
    {
        return &nodes[nodeCount];
    }

    inline node_t* end() noexcept
    {
        return &nodes[nodeCount];
    }

    inline const node_t* find_node(const char* name) const
    {
        for (auto& node : *this)
        {
            // Skip nodes without a name.
            const char* curNodeName = node.name.get();
            if (!curNodeName) continue;

            // If the node name partially matches, recurse through its child nodes.
            const std::size_t curNodeNameLen = text::len(curNodeName);
            const int compResult = text::compare(name,
                curNodeName, curNodeNameLen);

            if (!compResult)
            {
                // Increase name pointer.
                name += curNodeNameLen;

                // If the name *entirely* matched, return the node pointer.
                if (*name == '\0') return &node;

                // Recurse through child nodes.
                return node.find_child(name);
            }

            // OPTIMIZATION: Return null early if the first non-matching character
            // of the current node's name if greater than the first character of name.
            // We can safely do this with all valid PACxV3 data as all valid PACx3 data
            // is alphabetized.
            
            // TODO: Ensure this is *really* safe to do and actually implement this.
        }

        return nullptr;
    }

    inline const node_t* find_node(const std::string& name) const
    {
        return find_node(name.c_str());
    }

    inline const node_t& operator[](std::size_t i) const noexcept
    {
        return nodes[i];
    }

    inline node_t& operator[](std::size_t i) noexcept
    {
        return nodes[i];
    }
};

using file_node = node<data_entry>;
HL_STATIC_ASSERT_SIZE(file_node, 0x28);

using file_tree = node_tree<file_node>;
HL_STATIC_ASSERT_SIZE(file_tree, 0x18);

using type_node = node<file_tree>;
HL_STATIC_ASSERT_SIZE(file_node, 0x28);

using type_tree = node_tree<type_node>;
HL_STATIC_ASSERT_SIZE(type_tree, 0x18);

enum class pac_type : u16
{
    is_root = 1,
    is_split = 2,
    has_splits = 4,
    unknown = 8
};

HL_ENUM_CLASS_DEF_BITWISE_OPS(pac_type)

enum class pac_flags : u16
{
    unknown1 = 8,
    deflate_compressed = 0x100U,
    lz4_compressed = 0x200U
};

HL_ENUM_CLASS_DEF_BITWISE_OPS(pac_flags)

struct header
{
    /** @brief "PACx" signature. */
    u32 signature;
    /** @brief Version number. */
    bina::ver version;
    /** @brief 'B' for Big Endian, 'L' for Little Endian. See hl::bina::endian_flag. */
    u8 endianFlag;
    /** @brief A random number that serves as a unique identifier for this pac file. */
    u32 uid;
    /** @brief The size of the entire file, including this header. */
    u32 fileSize;
    u32 treesSize;
    u32 depTableSize;
    u32 dataEntriesSize;
    /** @brief The size of the string table in bytes, including padding. */
    u32 strTableSize;
    u32 fileDataSize;
    /** @brief The size of the offset table in bytes, including padding. */
    u32 offTableSize;
    /** @brief Bitwise-and this with values from the hl::pacx::v3::pac_type enum. */
    u16 type;
    /** @brief Bitwise-and this with values from the hl::pacx::v3::pac_flags enum. */
    u16 flags;
    u32 depCount;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(uid);
        hl::endian_swap(fileSize);
        hl::endian_swap(treesSize);
        hl::endian_swap(depTableSize);
        hl::endian_swap(dataEntriesSize);
        hl::endian_swap(strTableSize);
        hl::endian_swap(fileDataSize);
        hl::endian_swap(offTableSize);
        hl::endian_swap(type);
        hl::endian_swap(flags);
        hl::endian_swap(depCount);
    }

    inline bina::endian_flag endian_flag() const noexcept
    {
        return static_cast<bina::endian_flag>(endianFlag);
    }

    inline const type_tree& types() const noexcept
    {
        return *reinterpret_cast<const type_tree*>(this + 1);
    }

    inline type_tree& types() noexcept
    {
        return *reinterpret_cast<type_tree*>(this + 1);
    }

    const v3::dep_table* dep_table() const noexcept
    {
        return ptradd<v3::dep_table>(&types(), treesSize);
    }

    v3::dep_table* dep_table() noexcept
    {
        return ptradd<v3::dep_table>(&types(), treesSize);
    }

    const data_entry* data_entries() const noexcept
    {
        return ptradd<data_entry>(dep_table(), depTableSize);
    }

    data_entry* data_entries() noexcept
    {
        return ptradd<data_entry>(dep_table(), depTableSize);
    }

    const char* str_table() const noexcept
    {
        return ptradd<char>(data_entries(), dataEntriesSize);
    }

    char* str_table() noexcept
    {
        return ptradd<char>(data_entries(), dataEntriesSize);
    }

    const void* file_data() const noexcept
    {
        return ptradd(str_table(), strTableSize);
    }

    void* file_data() noexcept
    {
        return ptradd(str_table(), strTableSize);
    }

    const u8* off_table() const noexcept
    {
        return ptradd(file_data(), fileDataSize);
    }

    u8* off_table() noexcept
    {
        return ptradd(file_data(), fileDataSize);
    }

    bina::off_table_handle offsets() const noexcept
    {
        return bina::off_table_handle(off_table(), offTableSize);
    }

    HL_API void fix();
    HL_API void parse(archive_entry_list& hlArc, bool skipProxies = true) const;

    HL_API static void start_write(bina::ver version,
        u32 uid, pac_type type, compress_type compressType,
        bina::endian_flag endianFlag, stream& stream);

    HL_API static void finish_write(std::size_t headerPos,
        std::size_t treesPos, std::size_t depTablePos,
        std::size_t dataEntriesPos, std::size_t strTablePos,
        std::size_t fileDataPos, std::size_t offTablePos,
        u32 depCount, bina::endian_flag endianFlag, stream& stream);

    HL_API static void finish_write(std::size_t headerPos,
        std::size_t treesPos, std::size_t depTablePos,
        std::size_t dataEntriesPos, std::size_t strTablePos,
        std::size_t fileDataPos, u32 depCount,
        bina::endian_flag endianFlag, hl::off_table& offTable,
        stream& stream);
};

HL_STATIC_ASSERT_SIZE(header, 0x30);

inline void fix(void* pac)
{
    header* headerPtr = static_cast<header*>(pac);
    headerPtr->fix();
}

inline void parse(const void* pac, archive_entry_list& hlArc,
    bool skipProxies = true)
{
    const header* headerPtr = static_cast<const header*>(pac);
    headerPtr->parse(hlArc, skipProxies);
}

HL_API void read(blob& pac, archive_entry_list* hlArc,
    std::vector<blob>* pacs = nullptr);

inline void load_single(const nchar* filePath, archive_entry_list* hlArc,
    std::vector<blob>* pacs = nullptr)
{
    // Load data into blob.
    blob pac(filePath);

    // Read data and parse it as necessary.
    read(pac, hlArc, pacs);
}

inline void load_single(const nstring& filePath, archive_entry_list* hlArc,
    std::vector<blob>* pacs = nullptr)
{
    load_single(filePath.c_str(), hlArc, pacs);
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

HL_API void load(const nchar* filePath, archive_entry_list* hlArc,
    std::vector<blob>* pacs = nullptr);

inline void load(const nstring& filePath, archive_entry_list* hlArc,
    std::vector<blob>* pacs = nullptr)
{
    load(filePath.c_str(), hlArc, pacs);
}

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

HL_API u32 generate_uid();

HL_API void save(const archive_entry_list& arc, bina::endian_flag endianFlag,
    const supported_ext* exts, const std::size_t extCount,
    const nchar* filePath, u32 splitLimit = default_split_limit,
    u32 dataAlignment = default_alignment, packed_file_info* pfi = nullptr);

inline void save(const archive_entry_list& arc, bina::endian_flag endianFlag,
    const supported_ext* exts, const std::size_t extCount,
    const nstring& filePath, u32 splitLimit = default_split_limit,
    u32 dataAlignment = default_alignment, packed_file_info* pfi = nullptr)
{
    save(arc, endianFlag, exts, extCount,
        filePath, splitLimit, dataAlignment, pfi);
}
} // v3

namespace v4
{
constexpr u32 default_split_limit = 0x1E00000U;
constexpr u32 default_alignment = 16;
constexpr u32 default_lz4_max_chunk_size = 65536U;
constexpr u32 default_deflate_max_chunk_size = 99999U;

enum class pac_flags : u16
{
    none = 0,
    unknown1 = 1,
    has_parents = 2,
    has_metadata = 0x80U
};

HL_ENUM_CLASS_DEF_BITWISE_OPS(pac_flags)

/**
   @brief Information explaining how to decompress a blob of (LZ4?) compressed data.

   When decompressing the root pac allocate a buffer of
   size header.rootUncompressedSize, then loop through these
   chunks and decompress each one, one-by-one, into that buffer.

   If you try to decompress all at once instead the data can be corrupted.
*/
struct chunk
{
    u32 compressedSize;
    u32 uncompressedSize;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(compressedSize);
        hl::endian_swap(uncompressedSize);
    }

    inline chunk() = default;

    constexpr chunk(u32 compressedSize, u32 uncompressedSize) noexcept :
        compressedSize(compressedSize), uncompressedSize(uncompressedSize) {}
};

HL_STATIC_ASSERT_SIZE(chunk, 8);

struct lz4_dep_info
{
    off64<char> name;
    u32 compressedSize;
    u32 uncompressedSize;
    u32 dataPos;
    u32 chunkCount;
    off64<chunk> chunks;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap(compressedSize);
        hl::endian_swap(uncompressedSize);
        hl::endian_swap(dataPos);
        hl::endian_swap(chunkCount);
        hl::endian_swap<swapOffsets>(chunks);
    }

    HL_API blob decompress_dep(const void* pac) const;
};

HL_STATIC_ASSERT_SIZE(lz4_dep_info, 0x20);

using lz4_dep_table = arr64<lz4_dep_info>;
HL_STATIC_ASSERT_SIZE(lz4_dep_table, 16);

struct deflate_dep_info
{
    off64<char> name;
    u32 compressedSize;
    u32 uncompressedSize;
    u32 dataPos;
    u32 padding;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap(compressedSize);
        hl::endian_swap(uncompressedSize);
        hl::endian_swap(dataPos);
    }

    HL_API blob decompress_dep(const void* pac) const;
};

HL_STATIC_ASSERT_SIZE(deflate_dep_info, 0x18);

using deflate_dep_table = arr64<deflate_dep_info>;
HL_STATIC_ASSERT_SIZE(deflate_dep_table, 16);

struct chunk_table
{
    u32 count;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(count);
    }

    inline const chunk* chunks() const noexcept
    {
        return reinterpret_cast<const chunk*>(this + 1);
    }

    inline chunk* chunks() noexcept
    {
        return reinterpret_cast<chunk*>(this + 1);
    }

    inline const chunk* begin() const noexcept
    {
        return chunks();
    }

    inline chunk* begin() noexcept
    {
        return chunks();
    }

    inline const chunk* end() const noexcept
    {
        return (chunks() + count);
    }

    inline chunk* end() noexcept
    {
        return (chunks() + count);
    }

    inline const chunk& operator[](u32 i) const noexcept
    {
        return *(chunks() + i);
    }

    inline chunk& operator[](u32 i) noexcept
    {
        return *(chunks() + i);
    }
};

HL_STATIC_ASSERT_SIZE(chunk_table, 4);

namespace v02
{
struct header
{
    /** @brief "PACx" */
    u32 signature;
    /** @brief Version Number. */
    bina::ver version;
    /** @brief 'B' for Big Endian, 'L' for Little Endian. */
    u8 endianFlag;
    /** @brief A random number that serves as a unique identifier for this pac file. */
    u32 uid;
    u32 fileSize;
    off32<void> root;
    /**
       @brief The size, in bytes, of the compressed root
       PACxV3 pac data within this PACxV4 file.

       If this is the same as rootUncompresedSize, it means
       that the root PACxV3 pac data is uncompressed.
    */
    u32 rootCompressedSize;
    /**
       @brief The size, in bytes, of the uncompressed root
       PACxV3 pac data within this PACxV4 file.

       If this is the same as rootCompresedSize, it means
       that the root PACxV3 pac data is uncompressed.
    */
    u32 rootUncompressedSize;
    /** @brief Bitwise-and this with values from the hl::pacx::v4::pac_flags enum. */
    u16 flagsV4;
    /** @brief Bitwise-and this with values from the hl::pacx::v3::pac_flags enum. */
    u16 flagsV3;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(uid);
        hl::endian_swap(fileSize);
        hl::endian_swap<swapOffsets>(root);
        hl::endian_swap(rootCompressedSize);
        hl::endian_swap(rootUncompressedSize);
        hl::endian_swap(flagsV4);
        hl::endian_swap(flagsV3);
    }

    inline bina::endian_flag endian_flag() const noexcept
    {
        return static_cast<bina::endian_flag>(endianFlag);
    }

    inline const v4::chunk_table* root_chunks() const noexcept
    {
        return reinterpret_cast<const v4::chunk_table*>(this + 1);
    }

    inline v4::chunk_table* root_chunks() noexcept
    {
        return reinterpret_cast<v4::chunk_table*>(this + 1);
    }

    HL_API void fix();
    HL_API blob decompress_root() const;

    HL_API static void start_write(u32 uid, compress_type compressType,
        bina::endian_flag endianFlag, stream& stream);

    HL_API static void finish_write(std::size_t headerPos,
        std::size_t rootPos, u32 rootUncompressedSize,
        bina::endian_flag endianFlag, stream& stream);
};

HL_STATIC_ASSERT_SIZE(header, 0x20);

inline void fix(void* pac)
{
    header* headerPtr = static_cast<header*>(pac);
    headerPtr->fix();
}

inline blob decompress_root(const void* pac)
{
    const header* headerPtr = static_cast<const header*>(pac);
    return headerPtr->decompress_root();
}

HL_API void read(void* pac, archive_entry_list* hlArc,
    std::vector<blob>* pacs = nullptr, bool readSplits = true);

HL_API void write(const archive_entry_list& arc,
    const nchar* pacName, u32 maxChunkSize,
    compress_type compressType, bina::endian_flag endianFlag,
    const std::size_t extCount, const supported_ext* exts,
    stream& stream, u32 splitLimit = default_split_limit,
    u32 dataAlignment = default_alignment, bool noCompress = false);

HL_API void save(archive_entry_list& arc, u32 maxChunkSize,
    compress_type compressType, bina::endian_flag endianFlag,
    const std::size_t extCount, const supported_ext* exts,
    const nchar* filePath, u32 splitLimit = default_split_limit,
    u32 dataAlignment = default_alignment, bool noCompress = false);

inline void save(archive_entry_list& arc, u32 maxChunkSize,
    compress_type compressType, bina::endian_flag endianFlag,
    const std::size_t extCount, const supported_ext* exts,
    const nstring& filePath, u32 splitLimit = default_split_limit,
    u32 dataAlignment = default_alignment, bool noCompress = false)
{
    save(arc, maxChunkSize, compressType, endianFlag,
        extCount, exts, filePath, splitLimit, dataAlignment,
        noCompress);
}
} // v02

namespace v03
{
HL_API extern const nchar* const dependencies_file_name;

struct parent_info
{
    off64<char> path;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(path);
    }
};

HL_STATIC_ASSERT_SIZE(parent_info, 8);

using parent_table = arr64<parent_info>;
HL_STATIC_ASSERT_SIZE(parent_table, 16);

struct metadata_header
{
    u32 parentsSize;
    u32 chunkTableSize;
    u32 strTableSize;
    u32 offTableSize;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(parentsSize);
        hl::endian_swap(chunkTableSize);
        hl::endian_swap(strTableSize);
        hl::endian_swap(offTableSize);
    }

    inline const parent_table* parents() const noexcept
    {
        return reinterpret_cast<const parent_table*>(this + 1);
    }

    inline parent_table* parents() noexcept
    {
        return reinterpret_cast<parent_table*>(this + 1);
    }

    inline const v4::chunk_table* chunk_table() const noexcept
    {
        return ptradd<v4::chunk_table>(parents(), parentsSize);
    }

    inline v4::chunk_table* chunk_table() noexcept
    {
        return ptradd<v4::chunk_table>(parents(), parentsSize);
    }

    inline const char* str_table() const noexcept
    {
        return ptradd<char>(chunk_table(), chunkTableSize);
    }

    inline char* str_table() noexcept
    {
        return ptradd<char>(chunk_table(), chunkTableSize);
    }

    inline const u8* off_table() const noexcept
    {
        return ptradd(str_table(), strTableSize);
    }

    inline u8* off_table() noexcept
    {
        return ptradd(str_table(), strTableSize);
    }

    HL_API void fix(void* pac);
    HL_API void parse_parents(std::vector<std::string>& parentPaths) const;

    HL_API static void start_write(stream& stream);

    HL_API static void finish_write(std::size_t metadataHeaderPos,
        std::size_t chunkTablePos, std::size_t strTablePos,
        std::size_t offTablePos, bina::endian_flag endianFlag,
        stream& stream);

    HL_API static void finish_write(std::size_t headerPos,
        std::size_t metadataHeaderPos, std::size_t chunkTablePos,
        bina::endian_flag endianFlag, const hl::str_table& strTable,
        hl::off_table& offTable, stream& stream);
};

HL_STATIC_ASSERT_SIZE(metadata_header, 16);

struct header
{
    /** @brief "PACx" */
    u32 signature;
    /** @brief Version Number. */
    bina::ver version;
    /** @brief 'B' for Big Endian, 'L' for Little Endian. */
    u8 endianFlag;
    /** @brief A random number that serves as a unique identifier for this pac file. */
    u32 uid;
    u32 fileSize;
    off32<void> root;
    /**
        @brief The size, in bytes, of the compressed root
        PACxV3 pac data within this PACxV4 file.

        If this is the same as rootUncompresedSize, it means
        that the root PACxV3 pac data is uncompressed.
    */
    u32 rootCompressedSize;
    /**
        @brief The size, in bytes, of the uncompressed root
        PACxV3 pac data within this PACxV4 file.

        If this is the same as rootCompresedSize, it means
        that the root PACxV3 pac data is uncompressed.
    */
    u32 rootUncompressedSize;
    /** @brief Bitwise-and this with values from the hl::pacx::v4::pac_flags enum. */
    u16 flagsV4;
    /** @brief Bitwise-and this with values from the hl::pacx::v3::pac_flags enum. */
    u16 flagsV3;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(uid);
        hl::endian_swap(fileSize);
        hl::endian_swap<swapOffsets>(root);
        hl::endian_swap(rootCompressedSize);
        hl::endian_swap(rootUncompressedSize);
        hl::endian_swap(flagsV4);
        hl::endian_swap(flagsV3);
    }

    constexpr bool has_parents() const noexcept
    {
        return ((flagsV4 & static_cast<u16>(pac_flags::has_parents)) != 0);
    }

    constexpr bool has_metadata() const noexcept
    {
        return ((flagsV4 & static_cast<u16>(pac_flags::has_metadata)) != 0);
    }

    constexpr bina::endian_flag endian_flag() const noexcept
    {
        return static_cast<bina::endian_flag>(endianFlag);
    }

    inline const metadata_header* metadata() const noexcept
    {
        return reinterpret_cast<const metadata_header*>(this + 1);
    }

    inline metadata_header* metadata() noexcept
    {
        return reinterpret_cast<metadata_header*>(this + 1);
    }

    HL_API void fix();
    HL_API blob decompress_root() const;

    HL_API static void start_write(u32 uid, bool hasParents,
        compress_type compressType, bina::endian_flag endianFlag,
        stream& stream);

    HL_API static void finish_write(std::size_t headerPos,
        std::size_t rootPos, u32 rootUncompressedSize,
        bina::endian_flag endianFlag, stream& stream);
};

HL_STATIC_ASSERT_SIZE(header, 0x20);

inline void fix(void* pac)
{
    header* headerPtr = static_cast<header*>(pac);
    headerPtr->fix();
}

inline blob decompress_root(const void* pac)
{
    const header* headerPtr = static_cast<const header*>(pac);
    return headerPtr->decompress_root();
}

HL_API void read(void* pac, archive_entry_list* hlArc,
    std::vector<blob>* pacs = nullptr, bool readSplits = true,
    std::vector<std::string>* parentPaths = nullptr);

HL_API std::vector<std::string> parse_dependencies_file(
    const char* depsFile, std::size_t depsFileSize);

inline std::vector<std::string> parse_dependencies_file(
    const blob& depsFile)
{
    return parse_dependencies_file(depsFile.data<char>(), depsFile.size());
}

HL_API archive_entry generate_dependencies_file(
    const std::vector<std::string>& parentPaths);

HL_API void write(const archive_entry_list& arc,
    const std::vector<std::string>* parentPaths,
    const nchar* pacName, u32 maxChunkSize,
    compress_type compressType, bina::endian_flag endianFlag,
    const std::size_t extCount, const supported_ext* exts,
    stream& stream, u32 splitLimit = default_split_limit,
    u32 dataAlignment = default_alignment, bool noCompress = false);

HL_API void save(const archive_entry_list& arc,
    const std::vector<std::string>* parentPaths,
    u32 maxChunkSize, compress_type compressType,
    bina::endian_flag endianFlag, const std::size_t extCount, 
    const supported_ext* exts, const nchar* filePath,
    u32 splitLimit = default_split_limit,
    u32 dataAlignment = default_alignment,
    bool noCompress = false);

inline void save(const archive_entry_list& arc,
    const std::vector<std::string>* parentPaths,
    u32 maxChunkSize, compress_type compressType,
    bina::endian_flag endianFlag, const std::size_t extCount,
    const supported_ext* exts, const nstring& filePath,
    u32 splitLimit = default_split_limit,
    u32 dataAlignment = default_alignment,
    bool noCompress = false)
{
    save(arc, parentPaths, maxChunkSize, compressType,
        endianFlag, extCount, exts, filePath.c_str(),
        splitLimit, dataAlignment, noCompress);
}

HL_API void save(archive_entry_list& arc, u32 maxChunkSize,
    compress_type compressType, bina::endian_flag endianFlag,
    const std::size_t extCount, const supported_ext* exts,
    const nchar* filePath, u32 splitLimit = default_split_limit,
    u32 dataAlignment = default_alignment, bool noCompress = false);

inline void save(archive_entry_list& arc, u32 maxChunkSize,
    compress_type compressType, bina::endian_flag endianFlag,
    const std::size_t extCount, const supported_ext* exts,
    const nstring& filePath, u32 splitLimit = default_split_limit,
    u32 dataAlignment = default_alignment, bool noCompress = false)
{
    save(arc, maxChunkSize, compressType, endianFlag,
        extCount, exts, filePath.c_str(), splitLimit, dataAlignment,
        noCompress);
}
} // v03

using header = v03::header;

HL_API void fix(void* pac);

HL_API void decompress_no_alloc_lz4(u32 chunkCount,
    const chunk* chunks, u32 srcSize,
    const void* src, u32 dstSize, void* dst);

HL_API std::unique_ptr<u8[]> decompress_lz4(u32 chunkCount,
    const chunk* chunks, u32 srcSize,
    const void* src, u32 dstSize);

HL_API blob decompress_lz4_blob(u32 chunkCount,
    const chunk* chunks, u32 srcSize,
    const void* src, u32 dstSize);

HL_API void decompress_no_alloc_deflate(u32 srcSize,
    const void* src, u32 dstSize, void* dst);

HL_API std::unique_ptr<u8[]> decompress_deflate(u32 srcSize,
    const void* src, u32 dstSize);

HL_API blob decompress_deflate_blob(u32 srcSize,
    const void* src, u32 dstSize);

HL_API std::size_t compress_no_alloc_lz4(u32 maxChunkSize,
    std::size_t srcSize, const void* src, std::size_t dstBufSize,
    void* dst, std::vector<chunk>& chunks);

HL_API std::unique_ptr<u8[]> compress_lz4(u32 maxChunkSize,
    std::size_t srcSize, const void* src, std::size_t& dstSize,
    std::vector<chunk>& chunks);

HL_API blob compress_blob_lz4(u32 maxChunkSize,
    std::size_t srcSize, const void* src,
    std::vector<chunk>& chunks);

HL_API std::size_t compress_no_alloc_deflate(std::size_t srcSize,
    const void* src, std::size_t dstBufSize, void* dst);

HL_API std::unique_ptr<u8[]> compress_deflate(std::size_t srcSize,
    const void* src, std::size_t& dstSize);

HL_API blob compress_blob_deflate(std::size_t srcSize, const void* src);
HL_API blob decompress_root(const void* pac);

HL_API void read(void* pac, archive_entry_list* hlArc,
    std::vector<blob>* pacs = nullptr, bool readSplits = true,
    std::vector<std::string>* parentPaths = nullptr);

HL_API void load(const nchar* filePath,
    std::vector<std::string>* parentPaths,
    archive_entry_list* hlArc, std::vector<blob>* pacs = nullptr,
    bool readSplits = true);

inline void load(const nstring& filePath,
    std::vector<std::string>* parentPaths,
    archive_entry_list* hlArc, std::vector<blob>* pacs = nullptr,
    bool readSplits = true)
{
    load(filePath.c_str(), parentPaths, hlArc, pacs, readSplits);
}

HL_API void load(const nchar* filePath,
    archive_entry_list* hlArc, std::vector<blob>* pacs = nullptr,
    bool readSplits = true);

inline void load(const nstring& filePath,
    archive_entry_list* hlArc, std::vector<blob>* pacs = nullptr,
    bool readSplits = true)
{
    load(filePath.c_str(), hlArc, pacs, readSplits);
}

inline archive load(const nchar* filePath,
    std::vector<std::string>* parentPaths,
    bool readSplits = true)
{
    archive hlArc;
    load(filePath, parentPaths, &hlArc, nullptr, readSplits);
    return hlArc;
}

inline archive load(const nstring& filePath,
    std::vector<std::string>* parentPaths,
    bool readSplits = true)
{
    return load(filePath.c_str(), parentPaths, readSplits);
}

inline archive load(const nchar* filePath,
    bool readSplits = true)
{
    archive hlArc;
    load(filePath, &hlArc, nullptr, readSplits);
    return hlArc;
}

inline archive load(const nstring& filePath,
    bool readSplits = true)
{
    archive hlArc;
    load(filePath, &hlArc, nullptr, readSplits);
    return hlArc;
}
} // v4

HL_API nstring get_root_path(const nchar* filePath);

inline nstring get_root_path(const nstring& filePath)
{
    return get_root_path(filePath.c_str());
}

HL_API void load(const nchar* filePath, archive_entry_list* hlArc,
    std::vector<blob>* pacs = nullptr);

inline void load(const nstring& filePath, archive_entry_list* hlArc,
    std::vector<blob>* pacs = nullptr)
{
    load(filePath.c_str(), hlArc, pacs);
}

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
} // pacx
} // hl
#endif
