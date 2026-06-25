#ifndef HL_HH_MIRAGE_H_INCLUDED
#define HL_HH_MIRAGE_H_INCLUDED

#include <array>

#include <rad/rad_vector.h>
#include <rad/rad_string.h>
#include <rad/rad_span.h>

#include "../hl_internal.h"
#include "../io/hl_endian_readers.h"
#include "../io/hl_endian_writers.h"

namespace hl::hh_new::mirage
{
enum class container_type : unsigned char
{
    /// @brief Used in all HH1 games up to (and including) "Sonic Lost World".
    simple,
    /// @brief Used in "Sonic Lost World",
    /// "Mario & Sonic at the Rio 2016 Olympic Games",
    /// and "Sonic Forces".
    sample_chunk_r0,
    /// @brief Used in "Olympic Games Tokyo 2020",
    /// and all HH2 games released afterwards.
    sample_chunk_r1,
};

enum class off_type : unsigned char
{
    u32,
    u64,
};

struct file_info
{
    container_type  containerType = container_type::simple;
    off_type        offsetType = off_type::u32;
    u32             dataVersion = 0;
    rad::string     fileName;
};

struct simple_header
{
    u32 fileSize = 0;
    u32 dataVersion = 0;
    u32 dataSize = 0;
    u32 dataPos = 0;
    u32 offTablePos = 0;
    u32 fileNamePos = 0;
};

namespace sample_chunk
{
    inline constexpr u32 marker = 0x80000000U;

    inline constexpr u32 v1 = 20120906; // 2012-09-06

    struct header
    {
        u32 fileSize = 0;
        u32 version = v1;
        u32 offTablePos = 0;
        u32 offCount = 0;
    };

    struct name
    {
        std::array<char, 8> data;

        constexpr std::string_view view() const noexcept
        {
            return std::string_view{ data.data(), data.size() };
        }

        HL_API void assign(std::string_view str);

        constexpr bool operator==(name other) const noexcept
        {
            return view() == other.view();
        }

        constexpr bool operator!=(name other) const noexcept
        {
            return view() != other.view();
        }

        constexpr bool operator==(std::string_view str) const noexcept
        {
            return view() == str;
        }

        constexpr bool operator!=(std::string_view str) const noexcept
        {
            return view() != str;
        }

        constexpr bool operator==(const char* str) const noexcept
        {
            return view() == str;
        }

        constexpr bool operator!=(const char* str) const noexcept
        {
            return view() != str;
        }

        inline name(std::string_view str)
        {
            assign(str);
        }

        inline name(const char* str)
        {
            assign(str);
        }

        constexpr name(std::array<char, 8> data) noexcept
            : data(data)
        {
        }
    };

    struct property
    {
        sample_chunk::name name;
        u32 value;

        constexpr property(sample_chunk::name name, u32 value = 1) noexcept
            : name(name)
            , value(value)
        {
        }
    };

    enum node_flags : u32
    {
        NODE_MASK_FLAGS = 0x60000000U,
        NODE_MASK_SIZE = 0x1FFFFFFFU,

        NODE_FLAGS_IS_LEAF = 0x20000000U,
        NODE_FLAGS_IS_LAST_CHILD = 0x40000000U,
    };

    struct node
    {
        u32 flags;
        property prop;

        constexpr u32 size() const noexcept
        {
            return flags & NODE_MASK_SIZE;
        }

        constexpr bool is_leaf() const noexcept
        {
            return flags & NODE_FLAGS_IS_LEAF;
        }

        constexpr bool is_last_child() const noexcept
        {
            return flags & NODE_FLAGS_IS_LAST_CHILD;
        }

        constexpr node(u32 flags, property prop) noexcept
            : flags(flags)
            , prop(prop)
        {
        }
    };
}

enum class off_read_mode : unsigned char
{
    auto_detect = 0,
    u32,
    u64,
};

class reader
    : public io::big_endian_reader
{
public:
    HL_API rad::string read_string(
        rad::allocator& allocator = rad::default_allocator
    );

    HL_API rad::string read_string_at_offset(
        unsigned long long basePos,
        u64 stringOffset,
        rad::allocator& allocator = rad::default_allocator
    );

    HL_API rad::vector<u32> read_offset_table(
        u32 offsetCount,
        rad::allocator& allocator = rad::default_allocator
    );

    HL_API sample_chunk::node read_sample_chunk_node();

    HL_API simple_header read_simple_header();

    HL_API sample_chunk::header read_sample_chunk_header();

    inline reader(rad::stream& stream) noexcept
        : big_endian_reader(stream)
    {
    }
};

class writer
    : public io::big_endian_writer
{
public:
    inline void write_offset(u64 offset, off_type offType)
    {
        if (offType == off_type::u64)
        {
            write_u64(offset);
        }
        else
        {
            write_u32(static_cast<u32>(offset));
        }
    }

    HL_API void write_string(rad::cstring_view utf8Str);

    HL_API void write_offset_table(const rad::vector<u32>& offTable);

    HL_API void write_sample_chunk_node(const sample_chunk::node& node);

    HL_API void write_simple_header(const simple_header& header);

    HL_API void write_sample_chunk_header(const sample_chunk::header& header);

    inline writer(rad::stream& stream) noexcept
        : big_endian_writer(stream)
    {
    }
};

namespace sample_chunk
{
    class node_deserializer
    {
        mirage::reader              reader_;
        unsigned long long          curNodePos_;
        node                        curNode_;
        //std::size_t                 depth_ = 0;
        //rad::vector<std::size_t>    nextSiblingDepths_;

        node read_node_();

    public:
        inline mirage::reader& reader() noexcept
        {
            return reader_;
        }

        inline const node& current_node() const noexcept
        {
            return curNode_;
        }

        HL_API bool try_read_first_child();

        HL_API bool try_read_next_sibling();

        HL_API node_deserializer get_child_deserializer();

        /*
        inline std::size_t depth() const noexcept
        {
            return depth_;
        }

        HL_API bool try_go_to_next(std::size_t minDepth = 0);

        explicit node_deserializer(
            rad::stream& stream,
            rad::allocator& allocator = rad::default_allocator) noexcept
            : reader_(stream)
            , curNodePos_(stream.tell())
            , curNode_(reader_.read_sample_chunk_node())
            , nextSiblingDepths_(allocator)
        {
        }
        */

        HL_API explicit node_deserializer(rad::stream& stream);
    };

    /*
    class node_serializer
    {
        writer              writer_;
        unsigned long long  nodePos_ = 0;
        unsigned long long  lastChildPos_ = 0;

    public:
        HL_API void start(property prop);

        HL_API void finish();

        inline explicit node_serializer(rad::stream& stream) noexcept
            : writer_(stream)
        {
        }
    };
    */
}

class deserializer
{
    struct deserialize_info_
    {
        u32                 baseOff = 0;
        u32                 offTableOff = 0;
        u32                 offCount = 0;
        mirage::file_info   fileInfo;
    };

    mirage::reader                  reader_;
    unsigned long long              headerPos_;
    rad::vector<sample_chunk::node> sampleChunkNodes_;
    deserialize_info_               info_;
    unsigned long long              basePos_;

    off_type determine_offset_type_(
        off_read_mode offsetReadMode,
        u32 offCount,
        u32 baseOff
    );

    deserialize_info_ read_deserialize_info_(
        off_read_mode offsetReadMode,
        rad::allocator& allocator
    );

public:
    inline mirage::reader& reader() noexcept
    {
        return reader_;
    }

    inline const mirage::file_info& file_info() const noexcept
    {
        return info_.fileInfo;
    }

    inline bool has_sample_chunk_nodes() const noexcept
    {
        return info_.fileInfo.containerType != container_type::simple;
    }

    inline unsigned long long offset_table_position() const noexcept
    {
        return headerPos_ + info_.offTableOff;
    }

    inline u32 offset_count() const noexcept
    {
        return info_.offCount;
    }

    inline unsigned long long base_position() const noexcept
    {
        return basePos_;
    }

    inline std::size_t align(std::size_t stride)
    {
        return reader_.stream().align(stride, headerPos_);
    }

    HL_API rad::vector<u32> read_offset_table(
        rad::allocator& allocator = rad::default_allocator
    );

    inline unsigned long long get_offset_target(
        u64 offset) const noexcept
    {
        return basePos_ + offset;
    }

    inline void jump_to_offset_target(u64 offset)
    {
        reader_.stream().jump_to(get_offset_target(offset));
    }

    inline void skip_off64_alignment()
    {
        if (info_.fileInfo.offsetType == off_type::u64)
        {
            reader_.stream().jump_ahead(4);
        }
    }

    HL_API u64 read_offset();

    inline rad::string read_string_at_offset(
        u64 stringOffset,
        rad::allocator& allocator = rad::default_allocator)
    {
        return reader_.read_string_at_offset(
            basePos_,
            stringOffset,
            allocator
        );
    }

    inline rad::string read_string_at_offset(
        rad::allocator& allocator = rad::default_allocator)
    {
        return reader_.read_string_at_offset(
            basePos_,
            read_offset(),
            allocator
        );
    }

    HL_API explicit deserializer(
        rad::stream& stream,
        off_read_mode offsetReadMode = off_read_mode::auto_detect,
        rad::allocator& allocator = rad::default_allocator
    );
};

enum serializer_flag : unsigned char
{
    SERIALIZER_FLAG_NONE = 0,
    SERIALIZER_FLAG_NO_SORT_OFF_TABLE = (1 << 0),
};

class serializer
{
    enum sequence_ : unsigned char
    {
        SEQ_NONE_,
        SEQ_HEADER_,
        SEQ_DATA_,
        SEQ_DATA_END_,
    };

    struct node_info_
    {
        unsigned long long  nodePos;
        u32 parentIndex;
        u32 lastChildIndex = 0;
        u32 flags = (
            sample_chunk::marker | // HACK: Marker indicates the node is not finished
            sample_chunk::NODE_FLAGS_IS_LAST_CHILD |
            sample_chunk::NODE_FLAGS_IS_LEAF
        );

        inline node_info_(
            unsigned long long nodePos,
            u32 parentIndex) noexcept
            : nodePos(nodePos)
            , parentIndex(parentIndex)
        {
        }
    };

    mirage::writer                      writer_;
    mirage::file_info                   fileInfo_;
    unsigned long long                  headerPos_ = 0;
    unsigned long long                  basePos_ = 0;
    unsigned long long                  dataEndPos_ = 0;
    rad::vector<u32>                    offTable_;
    rad::vector<node_info_>             nodeInfo_;
    u32                                 curNodeIndex_ = UINT32_MAX;
    unsigned char                       offSize_ = 4;
    unsigned char                       flags_ = SERIALIZER_FLAG_NONE;
    sequence_                           lastSeqStep_ = SEQ_NONE_;

public:
    inline mirage::writer& writer() noexcept
    {
        return writer_;
    }

    inline const file_info& file_info() const noexcept
    {
        return fileInfo_;
    }

    inline unsigned long long header_position() const noexcept
    {
        return headerPos_;
    }

    inline unsigned long long base_position() const noexcept
    {
        return headerPos_;
    }

    inline u32 offset_size() const noexcept
    {
        return offSize_;
    }

    HL_API void start(
        mirage::file_info fileInfo,
        unsigned char flags = SERIALIZER_FLAG_NONE
    );

    inline std::size_t pad(std::size_t stride)
    {
        return writer_.stream().pad(stride, headerPos_);
    }

    inline void write_off64_alignment()
    {
        if (fileInfo_.offsetType == off_type::u64)
        {
            writer_.stream().write_as<u32>(0);
        }
    }

    HL_API void start_sample_chunk_node(sample_chunk::property prop);

    HL_API void finish_sample_chunk_node();

    HL_API void write_sample_chunk_leaf(sample_chunk::property prop);

    HL_API void start_data_section();

    HL_API u32 tell_local() const;

    HL_API u32 start_offset();

    HL_API void finish_offset(u32 localOffPos, u64 offTarget);

    inline void finish_offset(u32 localOffPos)
    {
        finish_offset(localOffPos, tell_local());
    }

    HL_API u32 start_offsets(u32 offCount);

    HL_API void finish_data_section();

    HL_API void finish();

    serializer& operator=(const serializer& other) = delete;

    serializer& operator=(serializer&& other) noexcept = default;

    HL_API explicit serializer(
        rad::stream& stream,
        rad::allocator& allocator = rad::default_allocator
    );

    serializer(const serializer& other) = delete;

    serializer(serializer&& other) noexcept = default;
};
}

#endif
