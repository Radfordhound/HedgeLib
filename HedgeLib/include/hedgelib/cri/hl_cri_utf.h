#ifndef HL_CRI_UTF_H_INCLUDED
#define HL_CRI_UTF_H_INCLUDED

#include "../hl_internal.h"
#include "../hl_guid.h"
#include "../common/io/hl_endian_readers.h"
#include "../common/io/hl_endian_writers.h"
#include <rad/rad_span.h>
#include <rad/rad_vector.h>
#include <rad/rad_stack_or_heap_array.h>
#include <stdexcept>
#include <utility>
#include <memory>
#include <string>

namespace hl::cri_new::utf
{
class table_deserializer;

inline constexpr u32 signature = 0x40555446U; // @UTF

inline constexpr std::string_view null_string = "<NULL>";

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
        return sizeof(cell_type_traits<T>::value_type);
    }
};

constexpr std::size_t get_size_of_cell(cell_type type)
{
    return dispatch_by_cell_type<get_size_of_cell_functor>(type);
}

enum raw_column_flags : u8
{
    COLUMN_MASK_TYPE = 0xf,

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

enum encoding_type : u8
{
    shift_jis = 0,
    utf8 = 1,
};

struct raw_table_header
{
    u8 unknown1;
    encoding_type encoding;
    u16 rowsOff;
    u32 stringsOff;
    u32 bufferDataOff;
    raw_string name;
    u16 columnCount;
    u16 rowSize;
    u32 rowCount;
};

struct raw_utf_header
{
    u32 signature;
    u32 tableSize;
};

struct column_info
{
    cell_type type;
    const char* name;
};

struct column_info_range
{
    u16 beginIndex;
    u16 count;
};

struct column_info_group
{
    rad::span<const column_info_range> ranges;

    constexpr u16 get_total_count() const noexcept
    {
        u16 totalColumnCount = 0;

        for (const auto& columnRange : ranges)
        {
            totalColumnCount += columnRange.count;
        }

        return totalColumnCount;
    }

    HL_API bool validate(
        const column_info* columnInfo,
        const table_deserializer& td
    ) const noexcept;

    HL_API void append_to(
        const column_info* columnInfo,
        rad::vector<column_info>& columns
    ) const;
};

class table_reader
    : public big_endian_reader
{
public:
    HL_API u16 read_as_u16(cell_type type);

    HL_API s16 read_as_s16(cell_type type);

    HL_API u32 read_as_u32(cell_type type);

    HL_API s32 read_as_s32(cell_type type);

    HL_API u64 read_as_u64(cell_type type);

    HL_API s64 read_as_s64(cell_type type);

    HL_API raw_string read_raw_string();

    HL_API raw_buffer read_raw_buffer();

    HL_API guid read_guid();

    HL_API raw_table_header read_raw_table_header();

    HL_API raw_utf_header read_raw_utf_header();

    HL_API rad::string read_string(
        encoding_type encoding,
        rad::allocator& allocator = rad::default_allocator
    );

    HL_API rad::vector<unsigned char> read_bytes(
        std::size_t size,
        rad::allocator& allocator = rad::default_allocator
    );

    HL_API void read_raw_cell_value(cell_type type, void* dst);

    HL_API raw_cell read_raw_cell(cell_type type);

    HL_API raw_column read_raw_column();

    inline table_reader(rad::stream& stream) noexcept
        : big_endian_reader(stream)
    {
    }
};

class table_writer
    : public big_endian_writer
{
public:
    HL_API void write_string(
        const char* utf8Str,
        encoding_type writeEncoding
    );

    HL_API void write_raw_string(raw_string str);

    HL_API void write_raw_buffer(raw_buffer buffer);

    HL_API void write_empty_raw_buffer();

    HL_API void write_guid(const guid& guid);

    HL_API void write_raw_table_header(const raw_table_header& rawTblHeader);

    HL_API void write_raw_utf_header(raw_utf_header rawUtfHeader);

    HL_API void write_raw_cell_value(cell_type type, const void* src);

    inline void write_raw_cell(cell_type type, const raw_cell& cell)
    {
        write_raw_cell_value(type, &cell);
    }

    HL_API void write_raw_column(const raw_column& column);

    inline table_writer(rad::stream& stream) noexcept
        : big_endian_writer(stream)
    {
    }
};

enum class table_deserialize_type
{
    utf,
    inner_table
};

class table_deserializer
{
    rad::allocator*         allocator_;
    table_reader            reader_;
    unsigned long long      tablePos_;
    raw_table_header        header_;
    raw_column*             columns_;
    const char*             stringTable_ = nullptr;
    u16                     nextColumnIndex_ = 0;
    u32                     curRowIndex_ = 0;

    unsigned long long get_start_of_inner_table_(
        table_deserialize_type type);

    const raw_column* next_column_();

    void go_to_row_(u32 rowIndex);

    void destruct_() noexcept;

public:
    inline unsigned long long header_pos() const noexcept
    {
        return tablePos_;
    }

    inline const raw_table_header& header() const noexcept
    {
        return header_;
    }

    inline rad::span<const raw_column> columns() const noexcept
    {
        return { columns_, header_.columnCount };
    }

    inline const char* string_table() const noexcept
    {
        return stringTable_;
    }

    inline u32 string_table_size() const noexcept
    {
        return header_.bufferDataOff - header_.stringsOff;
    }

    inline u16 column_count() const noexcept
    {
        return header_.columnCount;
    }

    inline u32 row_count() const noexcept
    {
        return header_.rowCount;
    }

    HL_API bool is_column_compatible_type(
        u16 columnIndex,
        cell_type expectedColumnType
    ) const noexcept;

    HL_API bool is_column_exact_type(
        u16 columnIndex,
        cell_type expectedColumnType
    ) const noexcept;

    HL_API bool are_columns_compatible_types(
        const column_info* columns,
        u16 utfTableStartIndex,
        u16 columnCount
    ) const noexcept;

    HL_API bool are_columns_exact_types(
        const column_info* columns,
        u16 utfTableStartIndex,
        u16 columnCount
    ) const noexcept;

    inline unsigned long long get_string_data_position(
        raw_string rawString) const noexcept
    {
        return (tablePos_ + header_.stringsOff + rawString.dataOff);
    }

    inline unsigned long long get_buffer_data_position(
        raw_buffer rawBuffer) const noexcept
    {
        return (tablePos_ + header_.bufferDataOff + rawBuffer.dataOff);
    }

    HL_API rad::string get_string_data(
        raw_string rawString,
        rad::allocator& allocator = rad::default_allocator
    );

    HL_API rad::vector<unsigned char> get_buffer_data(
        raw_buffer rawBuffer,
        rad::allocator& allocator = rad::default_allocator
    );

    HL_API u16 get_column_index(const char* name) const;

    HL_API u8 read_cell_as_u8();

    HL_API s8 read_cell_as_s8();

    HL_API u16 read_cell_as_u16();

    HL_API s16 read_cell_as_s16();

    HL_API u32 read_cell_as_u32();

    HL_API s32 read_cell_as_s32();

    HL_API u64 read_cell_as_u64();

    HL_API s64 read_cell_as_s64();

    HL_API float read_cell_as_f32();

    HL_API double read_cell_as_f64();

    HL_API raw_string read_cell_as_string();

    HL_API raw_buffer read_cell_as_buffer();

    HL_API guid read_cell_as_guid();

    HL_API raw_cell read_cell(cell_type* cellType = nullptr);

    HL_API void skip_cell();

    HL_API void next_row();

    HL_API void go_to_cell(u16 columnIndex, u32 rowIndex = 0);

    HL_API bool try_go_to_cell(const char* columnName, u32 rowIndex = 0);

    table_deserializer& operator=(const table_deserializer& other) = delete;

    HL_API table_deserializer& operator=(table_deserializer&& other) noexcept;

    HL_API table_deserializer(
        rad::stream& stream,
        table_deserialize_type type = table_deserialize_type::utf,
        rad::allocator& allocator = rad::default_allocator
    );

    table_deserializer(const table_deserializer& other) = delete;

    HL_API table_deserializer(table_deserializer&& other) noexcept;

    HL_API ~table_deserializer();
};

class table_serializer
{
    enum sequence_ : unsigned char
    {
        SEQ_NONE_,
        SEQ_HEADER_,
        SEQ_TABLE_,
        SEQ_STRINGS_,
        SEQ_BUFFER_DATA_,
    };

    struct column_
    {
        u8              flags;
        bool            hasCells = false;
        unsigned short  dataOff;
        raw_string      name;

        inline cell_type type() const noexcept
        {
            return static_cast<cell_type>(flags & COLUMN_MASK_TYPE);
        }

        column_(cell_type type) noexcept;
    };

    table_writer                        writer_;
    sequence_                           lastSeqStep_ = SEQ_NONE_;
    encoding_type                       encoding_;
    unsigned short                      curColumnIndex_;
    unsigned long                       rowCount_;
    rad::vector<char>                   strings_;
    unsigned long long                  utfPos_;
    unsigned long long                  stringsPos_;
    unsigned long long                  bufDataPos_;
    raw_string                          tableName_;
    bool                                hasNullString_;
    bool                                useDefaultValues_;
    unsigned short                      rowsOff_;
    unsigned short                      rowSize_;
    rad::vector<column_>                columns_;
    rad::vector<raw_cell>               cells_;

    raw_string add_string_(std::string_view str);

    void next_column_() noexcept;

    void write_table_();

    u32 compute_buffer_size_(unsigned long long dataStartPos) const;

public:
    inline rad::stream& stream() const noexcept
    {
        return writer_.stream();
    }

    inline const table_writer& writer() const noexcept
    {
        return writer_;
    }

    inline table_writer& writer() noexcept
    {
        return writer_;
    }

    inline rad::allocator& allocator() const noexcept
    {
        return strings_.allocator();
    }

    inline std::size_t column_count() const noexcept
    {
        return columns_.size();
    }

    HL_API void start(
        std::string_view tableName,
        rad::span<const column_info> columnsInfo,
        encoding_type encoding = encoding_type::utf8,
        bool writeNullString = false,
        bool useDefaultValues = true
    );

    HL_API std::size_t write_cell_as_u8(u8 val);

    HL_API std::size_t write_cell_as_s8(s8 val);

    HL_API std::size_t write_cell_as_u16(u16 val);

    HL_API std::size_t write_cell_as_s16(s16 val);

    HL_API std::size_t write_cell_as_u32(u32 val);

    HL_API std::size_t write_cell_as_s32(s32 val);

    HL_API std::size_t write_cell_as_u64(u64 val);

    HL_API std::size_t write_cell_as_s64(s64 val);

    HL_API std::size_t write_cell_as_f32(float val);

    HL_API std::size_t write_cell_as_f64(double val);

    HL_API std::size_t write_cell_as_string(std::string_view val);

    HL_API std::size_t write_cell_as_buffer();

    HL_API std::size_t write_cell_as_guid(guid val);

    HL_API void skip_cell();

    HL_API void next_row();

    HL_API void finish_rows(unsigned short bufferDataAlignment = 16);

    HL_API void fill_buffer(
        unsigned long long bufferPos,
        unsigned long long dataPos,
        u32 dataSize
    );

    HL_API void fill_buffer(
        unsigned long long bufferPos,
        unsigned long long dataPos
    );

    HL_API void fill_buffer_cell(
        std::size_t cellIndex,
        unsigned long long dataPos,
        u32 dataSize
    );

    HL_API void fill_buffer_cell(
        std::size_t cellIndex,
        unsigned long long dataPos
    );

    HL_API void finish();

    HL_API table_serializer(
        rad::stream& stream,
        rad::allocator& allocator = rad::default_allocator
    ) noexcept;
};
}

#endif
