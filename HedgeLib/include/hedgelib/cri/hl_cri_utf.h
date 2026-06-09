#ifndef HL_CRI_UTF_H_INCLUDED
#define HL_CRI_UTF_H_INCLUDED

#include <stdexcept>
#include <utility>
#include <memory> // TODO: Do we need this?
#include <string_view>

#include <rad/rad_span.h>
#include <rad/rad_vector.h>
#include <rad/rad_string.h>

#include "../hl_internal.h"
#include "../hl_guid.h"
#include "../io/hl_endian_readers.h"
#include "../io/hl_endian_writers.h"

namespace hl::cri::utf
{
class serializer;

inline constexpr u32 signature = 0x40555446U; // @UTF

inline constexpr char null_string[7] = "<NULL>";

struct raw_string
{
    u32 dataOff;
};

struct raw_buffer
{
    u32 dataOff;
    u32 size;
};

union raw_cell
{
    u8              valueU8;
    s8              valueS8;
    u16             valueU16;
    s16             valueS16;
    u32             valueU32;
    s32             valueS32;
    u64             valueU64;
    s64             valueS64;
    float           valueF32;
    double          valueF64;
    raw_string      valueString;
    raw_buffer      valueBuffer;
    guid            valueGuid;
};

enum class cell_type : u8
{
    u8 = 0,
    s8 = 1,
    u16 = 2,
    s16 = 3,
    u32 = 4,
    s32 = 5,
    u64 = 6,
    s64 = 7,
    f32 = 8,
    f64 = 9,
    string = 10,
    buffer = 11,
    guid = 12,
};

template<cell_type T>
struct cell_type_traits
{
};

template<>
struct cell_type_traits<cell_type::u8>
{
    using value_type = u8;

    static constexpr auto cell_member_ptr = &raw_cell::valueU8;
};

template<>
struct cell_type_traits<cell_type::s8>
{
    using value_type = s8;

    static constexpr auto cell_member_ptr = &raw_cell::valueS8;
};

template<>
struct cell_type_traits<cell_type::u16>
{
    using value_type = u16;

    static constexpr auto cell_member_ptr = &raw_cell::valueU16;
};

template<>
struct cell_type_traits<cell_type::s16>
{
    using value_type = s16;

    static constexpr auto cell_member_ptr = &raw_cell::valueS16;
};

template<>
struct cell_type_traits<cell_type::u32>
{
    using value_type = u32;

    static constexpr auto cell_member_ptr = &raw_cell::valueU32;
};

template<>
struct cell_type_traits<cell_type::s32>
{
    using value_type = s32;

    static constexpr auto cell_member_ptr = &raw_cell::valueS32;
};

template<>
struct cell_type_traits<cell_type::u64>
{
    using value_type = u64;

    static constexpr auto cell_member_ptr = &raw_cell::valueU64;
};

template<>
struct cell_type_traits<cell_type::s64>
{
    using value_type = s64;

    static constexpr auto cell_member_ptr = &raw_cell::valueS64;
};

template<>
struct cell_type_traits<cell_type::f32>
{
    using value_type = float;

    static constexpr auto cell_member_ptr = &raw_cell::valueF32;
};

template<>
struct cell_type_traits<cell_type::f64>
{
    using value_type = double;

    static constexpr auto cell_member_ptr = &raw_cell::valueF64;
};

template<>
struct cell_type_traits<cell_type::string>
{
    using value_type = raw_string;

    static constexpr auto cell_member_ptr = &raw_cell::valueString;
};

template<>
struct cell_type_traits<cell_type::buffer>
{
    using value_type = raw_buffer;

    static constexpr auto cell_member_ptr = &raw_cell::valueBuffer;
};

template<>
struct cell_type_traits<cell_type::guid>
{
    using value_type = guid;

    static constexpr auto cell_member_ptr = &raw_cell::valueGuid;
};

template<template<cell_type> class Functor, typename... Args>
constexpr auto dispatch_by_cell_type(cell_type type, Args&&... args)
{
    switch (type)
    {
    case cell_type::u8:
        return Functor<cell_type::u8>{}(std::forward<Args>(args)...);

    case cell_type::s8:
        return Functor<cell_type::s8>{}(std::forward<Args>(args)...);

    case cell_type::u16:
        return Functor<cell_type::u16>{}(std::forward<Args>(args)...);

    case cell_type::s16:
        return Functor<cell_type::s16>{}(std::forward<Args>(args)...);

    case cell_type::u32:
        return Functor<cell_type::u32>{}(std::forward<Args>(args)...);

    case cell_type::s32:
        return Functor<cell_type::s32>{}(std::forward<Args>(args)...);

    case cell_type::u64:
        return Functor<cell_type::u64>{}(std::forward<Args>(args)...);

    case cell_type::s64:
        return Functor<cell_type::s64>{}(std::forward<Args>(args)...);

    case cell_type::f32:
        return Functor<cell_type::f32>{}(std::forward<Args>(args)...);

    case cell_type::f64:
        return Functor<cell_type::f64>{}(std::forward<Args>(args)...);

    case cell_type::string:
        return Functor<cell_type::string>{}(std::forward<Args>(args)...);

    case cell_type::buffer:
        return Functor<cell_type::buffer>{}(std::forward<Args>(args)...);

    case cell_type::guid:
        return Functor<cell_type::guid>{}(std::forward<Args>(args)...);

    default:
        throw std::runtime_error("Unsupported cell type");
    }
}

template<cell_type T>
class get_size_of_cell_functor
{
public:
    constexpr std::size_t operator()() noexcept
    {
        return sizeof(typename cell_type_traits<T>::value_type);
    }
};

constexpr std::size_t get_size_of_cell(cell_type type)
{
    return dispatch_by_cell_type<get_size_of_cell_functor>(type);
}

/// @brief Returns whether the given cellType is compatible with the expected cell type.
/// @details All cell types are obviously compatible with the same cell type
/// (e.g. f32 is always compatible with f32).
///
/// Additionally, two integer types are compatible if:
/// - They have the same signedness
/// - The expected type's bit-width is >= the given type's.
///
/// Specifically, this means:
/// - u8 is compatible with u16, u32, and u64.
/// - u16 is compatible with u32 and u64.
/// - u32 is compatible with u64.
/// - s8 is compatible with s16, s32, and s64.
/// - s16 is compatible with s32 and s64.
/// - s32 is compatible with s64.
///
/// Please note that this does not go both ways.
/// For example: u64 is NOT compatible with u32.
///
/// @example `assert(is_cell_type_compatible(cell_type::u16, cell_type::u64));`
/// @param cellType The cell type to check against.
/// @param expectedCellType The expected cell type.
/// @return Whether the given cellType is compatible with the expected cell type.
HL_API bool is_cell_type_compatible(
    cell_type cellType,
    cell_type expectedCellType
) noexcept;

enum column_flags : u8
{
    COLUMN_MASK_TYPE = 0x0F,
    COLUMN_MASK_FLAGS = 0xF0,

    /// @brief Whether the column has a name, or is unnamed.
    COLUMN_FLAGS_HAS_NAME = 16,

    /// @brief Whether the column has a default value, which
    /// is used as the value of all of the column's cells,
    /// unless the column also has per-row data.
    COLUMN_FLAGS_HAS_DEFAULT_VALUE = 32,

    /// @brief Whether the column has per-row data.
    /// 
    /// If this flag is not set, the column has no data, unless
    /// the HAS_DEFAULT_VALUE flag is set, in which case, every
    /// row simply uses the default value.
    COLUMN_FLAGS_HAS_PER_ROW_DATA = 64,
};

struct raw_column
{
    u8 flags;
    raw_string name;
    raw_cell defaultValue;

    constexpr cell_type type() const noexcept
    {
        return static_cast<cell_type>(flags & COLUMN_MASK_TYPE);
    }

    constexpr bool has_name() const noexcept
    {
        return ((flags & COLUMN_FLAGS_HAS_NAME) != 0);
    }

    constexpr bool has_default_value() const noexcept
    {
        return ((flags & COLUMN_FLAGS_HAS_DEFAULT_VALUE) != 0);
    }

    constexpr bool has_per_row_data() const noexcept
    {
        return ((flags & COLUMN_FLAGS_HAS_PER_ROW_DATA) != 0);
    }

    constexpr std::size_t get_size_of_cell() const
    {
        return utf::get_size_of_cell(type());
    }
};

enum class encoding_type : u8
{
    shift_jis = 0,
    utf8 = 1,
};

struct table_header
{
    u8 unknown1 = 0; // TODO: Probably version?
    encoding_type encoding = encoding_type::utf8;
    u16 rowsOff = 0;
    u32 stringTableOff = 0;
    u32 bufferDataOff = 0;
    raw_string name = {};
    u16 columnCount = 0;
    u16 rowSize = 0;
    u32 rowCount = 0;
};

struct header
{
    u32 signature = utf::signature;
    u32 tableSize = 0;
};

struct column_info
{
    cell_type type;
    const char* name;

    constexpr column_info(cell_type type, const char* name = nullptr) noexcept
        : type(type)
        , name(name)
    {
    }
};

struct column_info_range
{
    u16 beginIndex = 0;
    u16 count = 0;
};

struct column_info_group
{
    rad::span<const column_info_range> ranges;

    constexpr u16 get_column_count() const noexcept
    {
        u16 totalColumnCount = 0;

        for (const auto& columnRange : ranges)
        {
            totalColumnCount += columnRange.count;
        }

        return totalColumnCount;
    }

    HL_API void append_to(
        const column_info* columnInfo,
        rad::vector<column_info>& output
    ) const;
};

class reader
    : public io::big_endian_reader
{
public:
    HL_API rad::string read_string(
        encoding_type encoding,
        rad::allocator& allocator = rad::default_allocator
    );

    HL_API raw_string read_raw_string();

    HL_API raw_buffer read_raw_buffer();

    HL_API guid read_guid();

    HL_API void read_cell_value(cell_type type, void* dst);

    HL_API raw_cell read_raw_cell(cell_type type);

    HL_API raw_column read_raw_column();

    HL_API table_header read_table_header();

    HL_API header read_header();

    inline reader(rad::stream& stream) noexcept
        : big_endian_reader(stream)
    {
    }
};

class writer
    : public io::big_endian_writer
{
public:
    HL_API void write_string(
        rad::cstring_view utf8Str,
        encoding_type writeEncoding
    );

    HL_API void write_raw_string(raw_string rawStr);

    HL_API void write_raw_buffer(raw_buffer rawBuf);

    HL_API void write_empty_raw_buffer();

    HL_API void write_guid(const guid& guid);

    HL_API void write_cell_value(cell_type type, const void* src);

    inline void write_raw_cell(cell_type type, const raw_cell& rawCell)
    {
        write_cell_value(type, &rawCell);
    }

    HL_API void write_raw_column(const raw_column& rawColumn);

    HL_API void write_table_header(const table_header& tableHeader);

    HL_API void write_header(header header);

    inline writer(rad::stream& stream) noexcept
        : big_endian_writer(stream)
    {
    }
};

enum class deserialize_type
{
    utf,
    inner_table
};

class deserializer
{
    rad::allocator*         allocator_ = &rad::default_allocator;
    reader                  reader_;
    unsigned long long      tablePos_ = 0;
    table_header            tableHeader_;
    raw_column*             columns_ = nullptr;
    const char*             stringTable_ = nullptr;
    bool                    ownsStringTable_ = false;
    u16                     nextColumnIndex_ = 0;
    u32                     curRowIndex_ = 0;

    unsigned long long go_to_table_header_(deserialize_type type);

    const raw_column* next_column_();

    void go_to_row_(u32 rowIndex);

    void validate_raw_string_(raw_string rawStr) const;

    void destruct_() noexcept;

    void load_string_table_();

public:
    inline const utf::reader& reader() const noexcept
    {
        return reader_;
    }

    inline utf::reader& reader() noexcept
    {
        return reader_;
    }

    inline unsigned long long header_pos() const noexcept
    {
        return tablePos_;
    }

    inline const table_header& header() const noexcept
    {
        return tableHeader_;
    }

    inline rad::span<const raw_column> columns() const noexcept
    {
        return { columns_, tableHeader_.columnCount };
    }

    inline const char* string_table() const noexcept
    {
        return stringTable_;
    }

    inline u32 string_table_size() const noexcept
    {
        return tableHeader_.bufferDataOff - tableHeader_.stringTableOff;
    }

    inline u16 column_count() const noexcept
    {
        return tableHeader_.columnCount;
    }

    inline u32 row_count() const noexcept
    {
        return tableHeader_.rowCount;
    }

    HL_API bool has_column_of_compatible_type(
        u16 columnIndex,
        cell_type expectedColumnType
    ) const noexcept;

    HL_API bool has_column_of_exact_type(
        u16 columnIndex,
        cell_type expectedColumnType
    ) const noexcept;

    HL_API bool has_columns_of_compatible_types(
        const column_info* expectedColumnInfo,
        u16 tableStartIndex,
        u16 columnCount
    ) const noexcept;

    HL_API bool has_columns_of_compatible_types(
        const column_info_group& expectedColumnInfoGroup,
        const column_info* expectedColumnInfo
    ) const noexcept;

    HL_API bool has_columns_of_exact_types(
        const column_info* expectedColumnInfo,
        u16 tableStartIndex,
        u16 columnCount
    ) const noexcept;

    HL_API bool has_columns_of_exact_types(
        const column_info_group& expectedColumnInfoGroup,
        const column_info* expectedColumnInfo
    ) const noexcept;

    inline unsigned long long get_string_data_position(
        raw_string rawStr) const noexcept
    {
        return (tablePos_ + tableHeader_.stringTableOff + rawStr.dataOff);
    }

    inline unsigned long long get_buffer_data_position(
        raw_buffer rawBuf) const noexcept
    {
        return (tablePos_ + tableHeader_.bufferDataOff + rawBuf.dataOff);
    }

    HL_API const char* get_string_data(raw_string rawStr) const;

    HL_API const char* get_optional_string_data(raw_string rawStr) const;

    HL_API rad::vector<unsigned char> read_buffer_data(
        raw_buffer rawBuf,
        rad::allocator& allocator = rad::default_allocator
    );

    HL_API u16 get_column_index(const char* name) const;

    HL_API u8 next_cell_as_u8();

    HL_API s8 next_cell_as_s8();

    HL_API u16 next_cell_as_u16();

    HL_API s16 next_cell_as_s16();

    HL_API u32 next_cell_as_u32();

    HL_API s32 next_cell_as_s32();

    HL_API u64 next_cell_as_u64();

    HL_API s64 next_cell_as_s64();

    HL_API float next_cell_as_f32();

    HL_API double next_cell_as_f64();

    HL_API raw_string next_cell_as_string();

    HL_API raw_buffer next_cell_as_buffer();

    HL_API guid next_cell_as_guid();

    HL_API std::pair<cell_type, raw_cell> next_cell();

    HL_API void skip_cell();

    HL_API void next_row();

    HL_API void go_to_cell(u16 columnIndex, u32 rowIndex = 0);

    HL_API bool try_go_to_cell(const char* columnName, u32 rowIndex = 0);

    deserializer& operator=(const deserializer& other) = delete;

    HL_API deserializer& operator=(deserializer&& other) noexcept;

    HL_API deserializer(
        rad::stream& stream,
        deserialize_type type = deserialize_type::utf,
        rad::allocator& allocator = rad::default_allocator
    );

    deserializer(const deserializer& other) = delete;

    HL_API deserializer(deserializer&& other) noexcept;

    HL_API ~deserializer();
};

class buffers_resolver
{
    friend serializer;

    serializer*             sr_;
    std::size_t             curBufCellIndex_ = 0;
    //u16                     curBufColumnIndex_ = 0;
    //u32                     curBufRowIndex_ = 0;
    unsigned long long      curBufDataPos_ = 0;

    std::size_t get_next_buffer_cell_index_(std::size_t cellIndex) const;

    explicit buffers_resolver(serializer& sr);

public:
    rad::stream& stream() const noexcept;

    const utf::writer& writer() const noexcept;

    utf::writer& writer() noexcept;

    HL_API void start();

    HL_API void next();

    buffers_resolver& operator=(const buffers_resolver& other) = delete;

    buffers_resolver& operator=(buffers_resolver&& other) noexcept = default;

    buffers_resolver(const buffers_resolver& other) = delete;

    buffers_resolver(buffers_resolver&& other) noexcept = default;
};

enum serializer_flags : u32
{
    SERIALIZER_FLAG_NONE = 0,
    SERIALIZER_FLAG_SUPPORT_NULL_STRING = (1 << 0),
};

class serializer
{
    friend buffers_resolver;

    enum sequence_ : unsigned char
    {
        SEQ_NONE_,
        SEQ_HEADER_,
        SEQ_STRINGS_,
        SEQ_BUFFER_DATA_,
    };

    struct column_meta_
    {
        u16             dataOff;
        bool            hasCells = false;
        u8              flags;
        raw_string      name;

        inline cell_type type() const noexcept
        {
            return static_cast<cell_type>(flags & COLUMN_MASK_TYPE);
        }

        column_meta_(cell_type type) noexcept;
    };

    writer                              writer_;
    sequence_                           lastSeqStep_ = SEQ_NONE_;
    encoding_type                       encoding_;
    u16                                 curColumnIndex_;
    u32                                 rowCount_;
    rad::vector<char>                   strings_;
    unsigned long long                  utfPos_;
    unsigned long long                  stringTablePos_;
    unsigned long long                  bufDataPos_;
    u32                                 flags_ = SERIALIZER_FLAG_NONE;
    //raw_string                          tableName_;
    //bool                                hasNullString_;
    //bool                                useDefaultValues_;
    u16                                 rowsOff_;
    u16                                 rowSize_;
    rad::vector<column_meta_>           columns_;
    rad::vector<raw_cell>               cells_;

    raw_string append_string_(std::string_view str);

    raw_cell& push_cell_();

    std::size_t next_cell_() noexcept;

    u32 compute_buffer_size_(unsigned long long dataStartPos) const;

public:
    inline const utf::writer& writer() const noexcept
    {
        return writer_;
    }

    inline utf::writer& writer() noexcept
    {
        return writer_;
    }

    inline rad::allocator& allocator() const noexcept
    {
        return strings_.allocator();
    }

    inline u16 column_count() const noexcept
    {
        return static_cast<u16>(columns_.size());
    }

    HL_API void start(
        std::string_view tableName,
        rad::span<const column_info> columnsInfo,
        encoding_type encoding = encoding_type::utf8,
        u32 flags = SERIALIZER_FLAG_NONE
        //bool writeNullString = false,
        //bool useDefaultValues = true
    );

    HL_API std::size_t push_cell_u8(u8 val);

    HL_API std::size_t push_cell_s8(s8 val);

    HL_API std::size_t push_cell_u16(u16 val);

    HL_API std::size_t push_cell_s16(s16 val);

    HL_API std::size_t push_cell_u32(u32 val);

    HL_API std::size_t push_cell_s32(s32 val);

    HL_API std::size_t push_cell_u64(u64 val);

    HL_API std::size_t push_cell_s64(s64 val);

    HL_API std::size_t push_cell_f32(float val);

    HL_API std::size_t push_cell_f64(double val);

    HL_API std::size_t push_cell_string(const char* val);

    HL_API std::size_t push_cell_string(std::string_view val);

    HL_API std::size_t push_cell_buffer(bool willBeEmpty = false);

    HL_API std::size_t push_cell_guid(guid val);

    HL_API void skip_cell();

    HL_API void next_row();

    HL_API buffers_resolver begin_buffer_data_section(
        unsigned short bufferDataAlignment = 1
    );

    HL_API void finish();

    HL_API explicit serializer(
        rad::stream& stream,
        rad::allocator& allocator = rad::default_allocator
    ) noexcept;
};

inline rad::stream& buffers_resolver::stream() const noexcept
{
    return sr_->writer_.stream();
}

inline const utf::writer& buffers_resolver::writer() const noexcept
{
    return sr_->writer_;
}

inline utf::writer& buffers_resolver::writer() noexcept
{
    return sr_->writer_;
}
}

#endif
