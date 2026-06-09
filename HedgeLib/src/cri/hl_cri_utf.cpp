#include "hedgelib/cri/hl_cri_utf.h"

namespace hl::cri::utf
{
bool is_cell_type_compatible(
    cell_type cellType,
    cell_type expectedCellType) noexcept
{
    // Exact types are always compatible.
    if (cellType == expectedCellType)
    {
        return true;
    }

    // Integer types are also compatible in cases where the actual cell type
    // is an integer of the same signedness but smaller bit width
    // than the expected cell type (e.g. [actual: u8, expected: u16] is OK).

    // This is because it's perfectly acceptable to read, for example, a
    // u8 value into a u16 variable, without any bit loss.

    switch (expectedCellType)
    {
    case cell_type::u64:
        if (cellType == cell_type::u32) return true;
        [[fallthrough]];

    case cell_type::u32:
        if (cellType == cell_type::u16) return true;
        [[fallthrough]];

    case cell_type::u16:
        return (cellType == cell_type::u8);

    case cell_type::s64:
        if (cellType == cell_type::s32) return true;
        [[fallthrough]];

    case cell_type::s32:
        if (cellType == cell_type::s16) return true;
        [[fallthrough]];

    case cell_type::s16:
        return (cellType == cell_type::s8);

    default:
        return false;
    }
}

void column_info_group::append_to(
    const utf::column_info* columnInfo,
    rad::vector<utf::column_info>& output) const
{
    assert(columnInfo &&
        "columnInfo argument must not be null"
    );

    output.reserve(output.size() + get_column_count());

    for (const auto& columnRange : ranges)
    {
        const auto rangeBegin = (columnInfo + columnRange.beginIndex);
        const auto rangeEnd = (rangeBegin + columnRange.count);

        output.append(rangeBegin, rangeEnd); // TODO: Switch to append_unchecked
    }
}

rad::string reader::read_string(
    encoding_type encoding,
    rad::allocator& allocator)
{
    switch (encoding)
    {
    case encoding_type::shift_jis:
        // TODO: Read Shift-JIS encoded string instead of this!!!
        return stream().read_string8(allocator);

    case encoding_type::utf8:
        return stream().read_string8(allocator);

    default:
        throw std::logic_error("Unsupported encoding");
    }
}

raw_string reader::read_raw_string()
{
    return raw_string{ read_u32() };
}

raw_buffer reader::read_raw_buffer()
{
    return raw_buffer{ read_u32(), read_u32() };
}

guid reader::read_guid()
{
    //guid val;
    //stream().read_as(val);
    //return val;

    // TODO: Are UTF guids for sure 16-byte, and are they endian-swapped at all??
    throw std::runtime_error("Not yet implemented");
}

void reader::read_cell_value(cell_type type, void* dst)
{
    switch (type)
    {
    case cell_type::u8:
        *static_cast<u8*>(dst) = read_u8();
        break;

    case cell_type::s8:
        *static_cast<s8*>(dst) = read_s8();
        break;

    case cell_type::u16:
        *static_cast<u16*>(dst) = read_u16();
        break;

    case cell_type::s16:
        *static_cast<s16*>(dst) = read_s16();
        break;

    case cell_type::u32:
        *static_cast<u32*>(dst) = read_u32();
        break;

    case cell_type::s32:
        *static_cast<s32*>(dst) = read_s32();
        break;

    case cell_type::u64:
        *static_cast<u64*>(dst) = read_u64();
        break;

    case cell_type::s64:
        *static_cast<s64*>(dst) = read_s64();
        break;

    case cell_type::f32:
        *static_cast<float*>(dst) = read_f32();
        break;

    case cell_type::f64:
        *static_cast<double*>(dst) = read_f64();
        break;

    case cell_type::string:
        *static_cast<raw_string*>(dst) = read_raw_string();
        break;

    case cell_type::buffer:
        *static_cast<raw_buffer*>(dst) = read_raw_buffer();
        break;

    case cell_type::guid:
        *static_cast<guid*>(dst) = read_guid();
        break;
    }
}

raw_cell reader::read_raw_cell(cell_type type)
{
    raw_cell cell;
    read_cell_value(type, &cell);
    return cell;
}

raw_column reader::read_raw_column()
{
    raw_column rawColumn;
    rawColumn.flags = read_u8();

    if (rawColumn.has_name())
    {
        rawColumn.name = read_raw_string();
    }

    if (rawColumn.has_default_value())
    {
        read_cell_value(rawColumn.type(), &rawColumn.defaultValue);
    }
    else if (!rawColumn.has_per_row_data())
    {
        // Handle empty columns by using a "zero-initialized" default value.
        // This means it's always valid to read the default value if !has_per_row_data().
        // NOTE: These are actually used, for example in cpk files.
        std::memset(&rawColumn.defaultValue, 0, sizeof(rawColumn.defaultValue));
    }

    return rawColumn;
}

table_header reader::read_table_header()
{
    return table_header{
        // TODO: Use C++20 delegated initializers.
        read_u8(),
        static_cast<encoding_type>(read_u8()),
        read_u16(),
        read_u32(),
        read_u32(),
        read_raw_string(),
        read_u16(),
        read_u16(),
        read_u32()
    };
}

header reader::read_header()
{
    return header{
        // TODO: Use C++20 delegated initializers.
        read_u32(),
        read_u32()
    };
}

void writer::write_string(
    rad::cstring_view utf8Str,
    encoding_type writeEncoding)
{
    switch (writeEncoding)
    {
    case encoding_type::shift_jis:
        // TODO: Write as Shift-JIS encoded string instead of this!!!
        stream().write_string8(utf8Str);
        break;

    case encoding_type::utf8:
        stream().write_string8(utf8Str);
        break;

    default:
        throw std::logic_error("Unsupported encoding");
    }
}

void writer::write_raw_string(raw_string rawStr)
{
    write_u32(rawStr.dataOff);
}

void writer::write_raw_buffer(raw_buffer rawBuf)
{
    write_u32(rawBuf.dataOff);
    write_u32(rawBuf.size);
}

void writer::write_empty_raw_buffer()
{
    const raw_buffer rawBuf = {};
    stream().write_as(rawBuf);
}

void writer::write_guid(const guid& guid)
{
    // TODO: Are UTF guids for sure 16-byte, and are they endian-swapped at all??
    throw std::runtime_error("Not yet implemented");
}

void writer::write_cell_value(cell_type type, const void* src)
{
    switch (type)
    {
    case cell_type::u8:
        write_u8(*static_cast<const u8*>(src));
        break;

    case cell_type::s8:
        write_s8(*static_cast<const s8*>(src));
        break;

    case cell_type::u16:
        write_u16(*static_cast<const u16*>(src));
        break;

    case cell_type::s16:
        write_s16(*static_cast<const s16*>(src));
        break;

    case cell_type::u32:
        write_u32(*static_cast<const u32*>(src));
        break;

    case cell_type::s32:
        write_s32(*static_cast<const s32*>(src));
        break;

    case cell_type::u64:
        write_u64(*static_cast<const u64*>(src));
        break;

    case cell_type::s64:
        write_s64(*static_cast<const s64*>(src));
        break;

    case cell_type::f32:
        write_f32(*static_cast<const float*>(src));
        break;

    case cell_type::f64:
        write_f64(*static_cast<const double*>(src));
        break;

    case cell_type::string:
        write_raw_string(*static_cast<const raw_string*>(src));
        break;

    case cell_type::buffer:
        write_raw_buffer(*static_cast<const raw_buffer*>(src));
        break;

    case cell_type::guid:
        write_guid(*static_cast<const guid*>(src));
        break;

    default:
        throw std::logic_error("Unsupported cell type");
    }
}

void writer::write_raw_column(const raw_column& rawColumn)
{
    write_u8(rawColumn.flags);

    if (rawColumn.has_name())
    {
        write_raw_string(rawColumn.name);
    }

    if (rawColumn.has_default_value())
    {
        if (rawColumn.has_per_row_data())
        {
            throw std::runtime_error("Invalid column flags combination");
        }

        write_raw_cell(rawColumn.type(), rawColumn.defaultValue);
    }
}

void writer::write_table_header(
    const table_header& tableHeader)
{
    write_u8(tableHeader.unknown1);
    write_u8(static_cast<u8>(tableHeader.encoding));
    write_u16(tableHeader.rowsOff);
    write_u32(tableHeader.stringTableOff);
    write_u32(tableHeader.bufferDataOff);
    write_raw_string(tableHeader.name);
    write_u16(tableHeader.columnCount);
    write_u16(tableHeader.rowSize);
    write_u32(tableHeader.rowCount);
}

void writer::write_header(header header)
{
    write_u32(header.signature);
    write_u32(header.tableSize);
}

unsigned long long deserializer::go_to_table_header_(
    deserialize_type type)
{
    if (type == deserialize_type::utf)
    {
        // Read past UTF header and verify it.
        const auto header = reader_.read_header();
        if (header.signature != signature)
        {
            throw std::runtime_error("Unsupported UTF data format");
        }
    }

    return reader_.stream().tell();
}

const raw_column* deserializer::next_column_()
{
    assert(nextColumnIndex_ < column_count() &&
        "You're reading too many cells from the current row! "
        "Please remember to call next_row() as appropriate and "
        "validate that your expected column layout matches what's "
        "in the file before reading."
    );

    return columns_ + nextColumnIndex_++;
}

void deserializer::go_to_row_(u32 rowIndex)
{
    reader_.stream().jump_to(
        tablePos_ + tableHeader_.rowsOff + (tableHeader_.rowSize * rowIndex)
    );

    nextColumnIndex_ = 0;
    curRowIndex_ = rowIndex;
}

void deserializer::validate_raw_string_(raw_string rawStr) const
{
    if (rawStr.dataOff >= string_table_size())
    {
        throw std::runtime_error("Invalid string data offset; "
            "offset was outside of the range of the string table"
        );
    }
}

void deserializer::destruct_() noexcept
{
    if (ownsStringTable_)
    {
        allocator_->free(const_cast<char*>(stringTable_));
    }

    allocator_->free(columns_);
}

void deserializer::load_string_table_()
{
    // Validate string table data region.
    if (tableHeader_.stringTableOff > tableHeader_.bufferDataOff)
    {
        throw std::runtime_error(
            "Invalid UTF data; "
            "buffer data should not come before string table"
        );
    }

    reader_.stream().jump_to(tablePos_ + tableHeader_.stringTableOff);
    const auto stringTableSize = string_table_size();

    if (!stringTableSize) return;

    // OPTIMIZATION: Just set the pointer directly if we can; no allocations.
    if (reader_.stream().capabilities().can_get_data_pointer())
    {
        // Ensure the string table memory ends before the end of the stream.
        if ((reader_.stream().tell() + stringTableSize) > reader_.stream().get_size())
        {
            throw std::runtime_error("Invalid UTF data; "
                "string table extends past the end of the stream"
            );
        }

        stringTable_ = static_cast<const char*>(reader_.stream().get_data_pointer());
        ownsStringTable_ = false;

        // Ensure string table memory ends with a null-terminator, so
        // it is always safe to use strcmp when searching for a string.
        if (stringTable_[stringTableSize - 1] == '\0') return;

        // If string table does not end with a null-terminator, continue
        // and fallback to creating copy of string table data which does.
    }

    // Read the string table from the stream.
    // NOTE: We allocate one additional char at the end
    // to use as a failsafe null-terminator.
    const auto strTableBuf = static_cast<char*>(
        allocator_->allocate(
            static_cast<std::size_t>(stringTableSize) + 1,
            alignof(char))
    );

    stringTable_ = strTableBuf;
    ownsStringTable_ = true;

    reader_.stream().read(strTableBuf, stringTableSize);

    // Ensure string table memory ends with a null-terminator, so
    // it is always safe to use strcmp when searching for a string.
    strTableBuf[stringTableSize] = '\0';
}

bool deserializer::has_column_of_compatible_type(
    u16 columnIndex,
    cell_type expectedColumnType) const noexcept
{
    if (columnIndex >= column_count())
    {
        return false;
    }

    const auto columnType = columns_[columnIndex].type();
    return is_cell_type_compatible(columnType, expectedColumnType);
}

bool deserializer::has_column_of_exact_type(
    u16 columnIndex,
    cell_type expectedColumnType) const noexcept
{
    if (columnIndex >= column_count())
    {
        return false;
    }

    const auto columnType = columns_[columnIndex].type();
    return (columnType == expectedColumnType);
}

bool deserializer::has_columns_of_compatible_types(
    const column_info* expectedColumnInfo,
    u16 tableStartIndex,
    u16 columnCount) const noexcept
{
    assert(expectedColumnInfo &&
        "expectedColumnInfo argument must not be null"
    );

    const auto endIndex = tableStartIndex + columnCount;

    if (endIndex > column_count())
    {
        return false;
    }

    for (u16 i = tableStartIndex; i < endIndex; ++i)
    {
        if (!is_cell_type_compatible(
            columns_[i].type(),
            expectedColumnInfo->type))
        {
            return false;
        }

        ++expectedColumnInfo;
    }

    return true;
}

bool deserializer::has_columns_of_compatible_types(
    const column_info_group& expectedColumnInfoGroup,
    const column_info* expectedColumnInfo) const noexcept
{
    assert(expectedColumnInfo &&
        "expectedColumnInfo argument must not be null"
    );

    // Validate column types.
    u16 utfTableStartIndex = 0;

    for (const auto& columnInfoRange : expectedColumnInfoGroup.ranges)
    {
        if (!has_columns_of_compatible_types(
            expectedColumnInfo + columnInfoRange.beginIndex,
            utfTableStartIndex,
            columnInfoRange.count))
        {
            return false;
        }

        utfTableStartIndex += columnInfoRange.count;
    }

    return true;
}

bool deserializer::has_columns_of_exact_types(
    const column_info* expectedColumnInfo,
    u16 tableStartIndex,
    u16 columnCount) const noexcept
{
    assert(expectedColumnInfo &&
        "expectedColumnInfo argument must not be null"
    );

    const auto endIndex = tableStartIndex + columnCount;

    if (endIndex > column_count())
    {
        return false;
    }

    for (u16 i = tableStartIndex; i < endIndex; ++i)
    {
        if (columns_[i].type() != expectedColumnInfo->type)
        {
            return false;
        }

        ++expectedColumnInfo;
    }

    return true;
}

bool deserializer::has_columns_of_exact_types(
    const column_info_group& expectedColumnInfoGroup,
    const column_info* expectedColumnInfo) const noexcept
{
    assert(expectedColumnInfo &&
        "expectedColumnInfo argument must not be null"
    );

    // Validate column types.
    u16 utfTableStartIndex = 0;

    for (const auto& columnInfoRange : expectedColumnInfoGroup.ranges)
    {
        if (!has_columns_of_exact_types(
            expectedColumnInfo + columnInfoRange.beginIndex,
            utfTableStartIndex,
            columnInfoRange.count))
        {
            return false;
        }

        utfTableStartIndex += columnInfoRange.count;
    }

    return true;
}

const char* deserializer::get_string_data(raw_string rawStr) const
{
    validate_raw_string_(rawStr);
    return stringTable_ + rawStr.dataOff;
}

const char* deserializer::get_optional_string_data(raw_string rawStr) const
{
    const auto str = get_string_data(rawStr);

    return (std::strcmp(str, "<NULL>") == 0) ?
        nullptr : str;
}

rad::vector<unsigned char> deserializer::read_buffer_data(
    raw_buffer rawBuf,
    rad::allocator& allocator)
{
    const auto curPos = reader_.stream().tell();
    reader_.stream().jump_to(get_buffer_data_position(rawBuf));

    auto buf = reader_.read_bytes(rawBuf.size, allocator);

    reader_.stream().jump_to(curPos);
    return buf;
}

u16 deserializer::get_column_index(const char* name) const
{
    for (u16 i = 0; i < tableHeader_.columnCount; ++i)
    {
        const auto& rawColumn = columns_[i];
        if (!rawColumn.has_name()) continue;

        validate_raw_string_(rawColumn.name);

        const auto columnName = stringTable_ + rawColumn.name.dataOff;
        if (std::strcmp(columnName, name) == 0)
        {
            return i;
        }
    }
    
    return UINT16_MAX;
}

#define ASSERT_HAS_COMPATIBLE_CELL_TYPE_(expectedType)\
    assert(is_cell_type_compatible(curColumn->type(), expectedType) &&\
        "You're reading a cell using an incompatible type! "\
        "Please validate that your expected column layout "\
        "matches what's in the file before reading."\
    )

#define READ_OR_DEFAULT_(valueType, defaultValFieldName)\
    (curColumn->has_per_row_data()) ?\
        reader_.read_##valueType() :\
        curColumn->defaultValue.defaultValFieldName

u8 deserializer::next_cell_as_u8()
{
    const auto curColumn = next_column_();
    ASSERT_HAS_COMPATIBLE_CELL_TYPE_(cell_type::u8);

    return READ_OR_DEFAULT_(u8, valueU8);
}

s8 deserializer::next_cell_as_s8()
{
    const auto curColumn = next_column_();
    ASSERT_HAS_COMPATIBLE_CELL_TYPE_(cell_type::s8);

    return READ_OR_DEFAULT_(s8, valueS8);
}

u16 deserializer::next_cell_as_u16()
{
    const auto curColumn = next_column_();
    ASSERT_HAS_COMPATIBLE_CELL_TYPE_(cell_type::u16);

    switch (curColumn->type())
    {
    case cell_type::u16:
        return READ_OR_DEFAULT_(u16, valueU16);

    case cell_type::u8:
        return READ_OR_DEFAULT_(u8, valueU8);

    default:
        throw std::runtime_error("Cannot read cell as u16");
    }
}

s16 deserializer::next_cell_as_s16()
{
    const auto curColumn = next_column_();
    ASSERT_HAS_COMPATIBLE_CELL_TYPE_(cell_type::s16);

    switch (curColumn->type())
    {
    case cell_type::s16:
        return READ_OR_DEFAULT_(s16, valueS16);

    case cell_type::s8:
        return READ_OR_DEFAULT_(s8, valueS8);

    default:
        throw std::runtime_error("Cannot read cell as s16");
    }
}

u32 deserializer::next_cell_as_u32()
{
    const auto curColumn = next_column_();
    ASSERT_HAS_COMPATIBLE_CELL_TYPE_(cell_type::u32);

    switch (curColumn->type())
    {
    case cell_type::u32:
        return READ_OR_DEFAULT_(u32, valueU32);

    case cell_type::u16:
        return READ_OR_DEFAULT_(u16, valueU16);

    case cell_type::u8:
        return READ_OR_DEFAULT_(u8, valueU8);

    default:
        throw std::runtime_error("Cannot read cell as u32");
    }
}

s32 deserializer::next_cell_as_s32()
{
    const auto curColumn = next_column_();
    ASSERT_HAS_COMPATIBLE_CELL_TYPE_(cell_type::s32);

    switch (curColumn->type())
    {
    case cell_type::s32:
        return READ_OR_DEFAULT_(s32, valueS32);

    case cell_type::s16:
        return READ_OR_DEFAULT_(s16, valueS16);

    case cell_type::s8:
        return READ_OR_DEFAULT_(s8, valueS8);

    default:
        throw std::runtime_error("Cannot read cell as s32");
    }
}

u64 deserializer::next_cell_as_u64()
{
    const auto curColumn = next_column_();
    ASSERT_HAS_COMPATIBLE_CELL_TYPE_(cell_type::u64);

    switch (curColumn->type())
    {
    case cell_type::u64:
        return READ_OR_DEFAULT_(u64, valueU64);

    case cell_type::u32:
        return READ_OR_DEFAULT_(u32, valueU32);

    case cell_type::u16:
        return READ_OR_DEFAULT_(u16, valueU16);

    case cell_type::u8:
        return READ_OR_DEFAULT_(u8, valueU8);

    default:
        throw std::runtime_error("Cannot read cell as u64");
    }
}

s64 deserializer::next_cell_as_s64()
{
    const auto curColumn = next_column_();
    ASSERT_HAS_COMPATIBLE_CELL_TYPE_(cell_type::s64);

    switch (curColumn->type())
    {
    case cell_type::s64:
        return READ_OR_DEFAULT_(s64, valueS64);

    case cell_type::s32:
        return READ_OR_DEFAULT_(s32, valueS32);

    case cell_type::s16:
        return READ_OR_DEFAULT_(s16, valueS16);

    case cell_type::s8:
        return READ_OR_DEFAULT_(s8, valueS8);

    default:
        throw std::runtime_error("Cannot read cell as s64");
    }
}

float deserializer::next_cell_as_f32()
{
    const auto curColumn = next_column_();
    ASSERT_HAS_COMPATIBLE_CELL_TYPE_(cell_type::f32);

    return READ_OR_DEFAULT_(f32, valueF32);
}

double deserializer::next_cell_as_f64()
{
    const auto curColumn = next_column_();
    ASSERT_HAS_COMPATIBLE_CELL_TYPE_(cell_type::f64);

    return READ_OR_DEFAULT_(f64, valueF64);
}

raw_string deserializer::next_cell_as_string()
{
    const auto curColumn = next_column_();
    ASSERT_HAS_COMPATIBLE_CELL_TYPE_(cell_type::string);

    return READ_OR_DEFAULT_(raw_string, valueString);
}

raw_buffer deserializer::next_cell_as_buffer()
{
    const auto curColumn = next_column_();
    ASSERT_HAS_COMPATIBLE_CELL_TYPE_(cell_type::buffer);

    return READ_OR_DEFAULT_(raw_buffer, valueBuffer);
}

guid deserializer::next_cell_as_guid()
{
    const auto curColumn = next_column_();
    ASSERT_HAS_COMPATIBLE_CELL_TYPE_(cell_type::guid);

    return READ_OR_DEFAULT_(guid, valueGuid);
}

std::pair<cell_type, raw_cell> deserializer::next_cell()
{
    raw_cell c;
    const auto curColumn = next_column_();

    switch (curColumn->type())
    {
    case cell_type::u8:
        c.valueU8 = READ_OR_DEFAULT_(u8, valueU8);
        break;

    case cell_type::s8:
        c.valueS8 = READ_OR_DEFAULT_(s8, valueS8);
        break;

    case cell_type::u16:
        c.valueU16 = READ_OR_DEFAULT_(u16, valueU16);
        break;

    case cell_type::s16:
        c.valueS16 = READ_OR_DEFAULT_(s16, valueS16);
        break;

    case cell_type::u32:
        c.valueU32 = READ_OR_DEFAULT_(u32, valueU32);
        break;

    case cell_type::s32:
        c.valueS32 = READ_OR_DEFAULT_(s32, valueS32);
        break;

    case cell_type::u64:
        c.valueU64 = READ_OR_DEFAULT_(u64, valueU64);
        break;

    case cell_type::s64:
        c.valueS64 = READ_OR_DEFAULT_(s64, valueS64);
        break;

    case cell_type::f32:
        c.valueF32 = READ_OR_DEFAULT_(f32, valueF32);
        break;

    case cell_type::f64:
        c.valueF64 = READ_OR_DEFAULT_(f64, valueF64);
        break;

    case cell_type::string:
        c.valueString = READ_OR_DEFAULT_(raw_string, valueString);
        break;

    case cell_type::buffer:
        c.valueBuffer = READ_OR_DEFAULT_(raw_buffer, valueBuffer);
        break;

    case cell_type::guid:
        c.valueGuid = READ_OR_DEFAULT_(guid, valueGuid);
        break;

    default:
        throw std::runtime_error("Unsupported cell type");
    }

    return { curColumn->type(), c };
}

void deserializer::skip_cell()
{
    const auto curColumn = next_column_();

    if (!curColumn->has_per_row_data()) return;

    switch (curColumn->type())
    {
    case cell_type::u8:
    case cell_type::s8:
        reader_.stream().jump_ahead(1);
        break;

    case cell_type::u16:
    case cell_type::s16:
        reader_.stream().jump_ahead(2);
        break;

    case cell_type::u32:
    case cell_type::s32:
    case cell_type::f32:
    case cell_type::string:
        reader_.stream().jump_ahead(4);
        break;

    case cell_type::u64:
    case cell_type::f64:
    case cell_type::buffer:
        reader_.stream().jump_ahead(8);
        break;

    case cell_type::guid:
        reader_.stream().jump_ahead(16);
        break;

    default:
        throw std::runtime_error("Unsupported cell type");
    }
}

void deserializer::next_row()
{
    assert(curRowIndex_ < tableHeader_.rowCount &&
        "next_row() is being called too many times! "
        "Please ensure you are only calling it up to "
        "row_count() times."
    );

    go_to_row_(curRowIndex_ + 1);
}

void deserializer::go_to_cell(u16 columnIndex, u32 rowIndex)
{
    assert(columnIndex < tableHeader_.columnCount &&
        "Invalid column index was given; please ensure "
        "you check the column index against column_count()"
    );

    assert(rowIndex < tableHeader_.rowCount &&
        "Invalid row index was given; please ensure "
        "you check the row index against row_count()"
    );

    go_to_row_(rowIndex);

    for (u16 i = 0; i < columnIndex; ++i)
    {
        skip_cell();
    }
}

bool deserializer::try_go_to_cell(const char* columnName, u32 rowIndex)
{
    const auto columnIndex = get_column_index(columnName);
    if (columnIndex == UINT16_MAX) return false;

    go_to_cell(columnIndex, rowIndex);
    return true;
}

deserializer& deserializer::operator=(deserializer&& other) noexcept
{
    if (&other != this)
    {
        destruct_();

        allocator_ = other.allocator_;
        reader_ = std::move(other.reader_);
        tablePos_ = other.tablePos_;
        tableHeader_ = other.tableHeader_;
        columns_ = other.columns_;
        stringTable_ = other.stringTable_;
        ownsStringTable_ = other.ownsStringTable_;
        nextColumnIndex_ = other.nextColumnIndex_;
        curRowIndex_ = other.curRowIndex_;

        other.columns_ = nullptr;
        other.stringTable_ = nullptr;
        // NOTE: We don't have to set other.ownsStringTable_ to false
    }

    return *this;
}

deserializer::deserializer(
    rad::stream& stream,
    deserialize_type type,
    rad::allocator& allocator)
    : allocator_(&allocator)
    , reader_(stream)
    , tablePos_(go_to_table_header_(type))
    , tableHeader_(reader_.read_table_header())
    , columns_(allocator_->create<raw_column>(
        rad::no_value_init, tableHeader_.columnCount))
{
    try
    {
        // Read columns.
        for (u16 i = 0; i < tableHeader_.columnCount; ++i)
        {
            columns_[i] = reader_.read_raw_column();
        }

        // Read string table.
        load_string_table_();
        
        // Jump to rows position.
        reader_.stream().jump_to(tablePos_ + tableHeader_.rowsOff);
    }
    catch (...)
    {
        destruct_();
        throw;
    }
}

deserializer::deserializer(deserializer&& other) noexcept
    : allocator_(other.allocator_)
    , reader_(std::move(other.reader_))
    , tablePos_(other.tablePos_)
    , tableHeader_(other.tableHeader_)
    , columns_(other.columns_)
    , stringTable_(other.stringTable_)
    , ownsStringTable_(other.ownsStringTable_)
    , nextColumnIndex_(other.nextColumnIndex_)
    , curRowIndex_(other.curRowIndex_)
{
    other.columns_ = nullptr;
    other.stringTable_ = nullptr;
    // NOTE: We don't have to set other.ownsStringTable_ to false
}

deserializer::~deserializer()
{
    destruct_();
}

std::size_t buffers_resolver::get_next_buffer_cell_index_(
    std::size_t cellIndex) const
{
    const auto& columns = sr_->columns_;

    while (cellIndex < sr_->cells_.size())
    {
        const auto columnIndex = cellIndex % columns.size();
        const auto& column = columns[columnIndex];

        if (column.type() == cell_type::buffer)
        {
            break;
        }

        ++cellIndex;
    }

    return cellIndex;
}

buffers_resolver::buffers_resolver(serializer& sr)
    : sr_(&sr)
    , curBufCellIndex_(get_next_buffer_cell_index_(0))
{
}

void buffers_resolver::start()
{
    // TODO: Validate serializer sequence.

    assert(curBufDataPos_ == 0 && curBufCellIndex_ < sr_->cells_.size() &&
        "start() must be called only once per buffer"
    );

    assert(sr_->columns_[curBufCellIndex_ % sr_->columns_.size()].hasCells &&
        "start() must not be called for buffer cells which were skipped"
    );

    // HACK: buffer dataOff field is used as a "willBeEmpty" marker.
    assert(!sr_->cells_[curBufCellIndex_].valueBuffer.dataOff &&
        "start() must not be called for buffer cells which were promised to be empty"
    );

    const auto curPos = stream().tell();

    if (curPos < sr_->bufDataPos_)
    {
        throw std::runtime_error("UTF buffer data cannot start "
            "outside of the range of the buffer data section"
        );
    }

    curBufDataPos_ = curPos;
}

void buffers_resolver::next()
{
    //assert(curBufDataPos_ != 0 &&
        //"start() must be called before finish()"
    //);

    // TODO: Validate serializer sequence.

    assert(curBufCellIndex_ < sr_->cells_.size() &&
        "next() must be called only once per buffer"
    );

    if (curBufDataPos_ != 0)
    {
        // Compute and validate buffer data size.
        auto& writer = sr_->writer();
        const auto dataEndPos = writer.stream().tell();

        if (dataEndPos < curBufDataPos_)
        {
            throw std::runtime_error(
                "UTF buffer end position is less than "
                "buffer data start position"
            );
        }

        if ((dataEndPos - curBufDataPos_) > UINT32_MAX)
        {
            throw std::runtime_error("UTF buffer size exceeds u32 range");
        }

        const auto bufDataSize = static_cast<u32>(dataEndPos - curBufDataPos_);

        // Compute and validate buffer position.
        const auto& columns = sr_->columns_;
        const auto columnIndex = curBufCellIndex_ % columns.size();
        const auto rowIndex = curBufCellIndex_ / columns.size();

        const auto tablePos = sr_->utfPos_ + 8;
        const auto perRowDataPos = tablePos + sr_->rowsOff_;

        const auto bufferPos = (
            perRowDataPos +
            (sr_->rowSize_ * rowIndex) +
            columns[columnIndex].dataOff
        );

        assert(bufferPos >= sr_->utfPos_ && bufferPos < sr_->stringTablePos_ &&
            "The computed buffer position was not within the per-row-data region"
        );

        // Jump to buffer position.
        const auto pos = writer.stream().tell();
        writer.stream().jump_to(bufferPos);

        // Fill-in buffer data position and size.
        writer.write_raw_buffer({
            static_cast<u32>(curBufDataPos_ - sr_->bufDataPos_),
            bufDataSize
        });

        // Jump back to previous stream position.
        writer.stream().jump_to(pos);
    }

    // Get next buffer cell index.
    curBufCellIndex_ = get_next_buffer_cell_index_(curBufCellIndex_ + 1);
    curBufDataPos_ = 0;
}

serializer::column_meta_::column_meta_(cell_type type) noexcept
    : flags(static_cast<u8>(type))
{
}

raw_string serializer::append_string_(std::string_view str)
{
    // TODO: Check if strings_.size() exceeds u32 range ?
    // TODO: Handle SHIFT-JIS encoding !!!

    const raw_string rawStr{ static_cast<u32>(strings_.size()) };

    strings_.reserve(strings_.size() + str.size() + 1);

    // TODO: Switch to append_unchecked
    strings_.append(
        str.begin(),
        str.end()
    );

    strings_.push_back_unchecked('\0');

    return rawStr;
}

raw_cell& serializer::push_cell_()
{
    assert(lastSeqStep_ > SEQ_NONE_ &&
        "start() must be called before pushing or skipping a cell!"
    );

    assert(lastSeqStep_ < SEQ_STRINGS_ &&
        "No more cells should be pushed or skipped after "
        "a call to begin_buffer_data_section() or finish()"
    );

    assert(curColumnIndex_ < column_count() &&
        "You're pushing too many cells to the current row! "
        "Please remember to call next_row() as appropriate and "
        "validate that you're pushing cells in accordance with your "
        "column layout specified by the columnsInfo you passed into start()."
    );

    return cells_.push_back(rad::no_value_init);
}

std::size_t serializer::next_cell_() noexcept
{
    columns_[curColumnIndex_].hasCells = true;
    ++curColumnIndex_;

    return cells_.back_index();
}

template<cell_type CellType>
class set_default_value_functor_
{
public:
    bool operator()(
        raw_cell& output,
        const rad::vector<raw_cell>& cells,
        const char* strings,
        u16 firstCellIndex,
        u16 columnCount)
    {
        if constexpr (CellType == cell_type::buffer)
        {
            for (std::size_t i = firstCellIndex;
                i < cells.size();
                i += columnCount)
            {
                // HACK: buffer dataOff field is used as a "willBeEmpty" marker.
                if (!cells[i].valueBuffer.dataOff) return false;
            }

            // If all buffers will be empty, copy an empty buffer value to the output.
            output.valueBuffer = raw_buffer{};
            return true;
        }
        else if constexpr (CellType == cell_type::string)
        {
            std::size_t i = firstCellIndex;
            const auto firstCellVal = cells[i].valueString;

            while ((i += columnCount) < cells.size())
            {
                if (std::strcmp(
                    strings + cells[i].valueString.dataOff,
                    strings + firstCellVal.dataOff) != 0)
                {
                    return false;
                }
            }

            // If all strings are equal, copy that cell value to the output.
            output.valueString = firstCellVal;
            return true;
        }
        else
        {
            constexpr auto valMemberPtr = cell_type_traits<CellType>::cell_member_ptr;

            // Loop through all cells. If all cell values are not equal, return false.
            std::size_t i = firstCellIndex;
            const auto firstCellVal = cells[i].*valMemberPtr;

            while ((i += columnCount) < cells.size())
            {
                if (std::memcmp(
                    &(cells[i].*valMemberPtr),
                    &firstCellVal,
                    sizeof(firstCellVal)) != 0)
                {
                    return false;
                }
            }

            // If all cell values are equal, copy that cell value to the output.
            output.*valMemberPtr = firstCellVal;
            return true;
        }
    }
};

u32 serializer::compute_buffer_size_(
    unsigned long long dataStartPos) const
{
    const auto dataEndPos = writer_.stream().tell();

    assert(dataEndPos >= dataStartPos &&
        "Buffer data position is invalid"
    );

    // TODO: Should this be an exception?
    assert((dataEndPos - dataStartPos) <= UINT32_MAX &&
        "Buffer size is too large"
    );

    return static_cast<u32>(dataEndPos - dataStartPos);
}

void serializer::start(
    std::string_view tableName,
    rad::span<const column_info> columnsInfo,
    encoding_type encoding,
    u32 flags)
{
    assert(lastSeqStep_ == SEQ_NONE_ &&
        "start() must not be called again until after "
        "a matching call to finish()"
    );

    if (columnsInfo.size() > UINT16_MAX)
    {
        throw std::runtime_error("UTF column count exceeded u16 range");
    };

    // Reset fields.
    encoding_ = encoding;
    curColumnIndex_ = 0;
    rowCount_ = 0;
    strings_.clear();
    bufDataPos_ = stringTablePos_ = utfPos_ = writer_.stream().tell();

    append_string_(tableName);
    
    flags_ = flags;
    rowsOff_ = 0;
    rowSize_ = 0;
    columns_.clear();
    cells_.clear();

    // Populate column metadata.
    columns_.reserve(columnsInfo.size());

    for (const auto& columnInfo : columnsInfo)
    {
        auto& columnMeta = columns_.emplace_back(columnInfo.type);

        // Set name if necessary.
        if (columnInfo.name)
        {
            columnMeta.name = append_string_(columnInfo.name);
            columnMeta.flags |= COLUMN_FLAGS_HAS_NAME;
        }
    }

    // Write placeholder UTF and table headers.
    writer_.stream().write_nulls(32);
    lastSeqStep_ = SEQ_HEADER_;
}

std::size_t serializer::push_cell_u8(u8 val)
{
    push_cell_().valueU8 = val;
    return next_cell_();
}

std::size_t serializer::push_cell_s8(s8 val)
{
    push_cell_().valueS8 = val;
    return next_cell_();
}

std::size_t serializer::push_cell_u16(u16 val)
{
    push_cell_().valueU16 = val;
    return next_cell_();
}

std::size_t serializer::push_cell_s16(s16 val)
{
    push_cell_().valueS16 = val;
    return next_cell_();
}

std::size_t serializer::push_cell_u32(u32 val)
{
    push_cell_().valueU32 = val;
    return next_cell_();
}

std::size_t serializer::push_cell_s32(s32 val)
{
    push_cell_().valueS32 = val;
    return next_cell_();
}

std::size_t serializer::push_cell_u64(u64 val)
{
    push_cell_().valueU64 = val;
    return next_cell_();
}

std::size_t serializer::push_cell_s64(s64 val)
{
    push_cell_().valueS64 = val;
    return next_cell_();
}

std::size_t serializer::push_cell_f32(float val)
{
    push_cell_().valueF32 = val;
    return next_cell_();
}

std::size_t serializer::push_cell_f64(double val)
{
    push_cell_().valueF64 = val;
    return next_cell_();
}

std::size_t serializer::push_cell_string(const char* val)
{
    assert((val || flags_ & SERIALIZER_FLAG_SUPPORT_NULL_STRING) &&
        "nullptr must not be passed to push_cell_string unless null "
        "string support was requested in the flags passed to start()"
    );

    push_cell_().valueString = ((!val) ?
        raw_string{} :
        append_string_(val)
    );

    return next_cell_();
}

std::size_t serializer::push_cell_string(std::string_view val)
{
    push_cell_().valueString = append_string_(val);
    return next_cell_();
}

std::size_t serializer::push_cell_buffer(bool willBeEmpty)
{
    // HACK: buffer dataOff field is used as a "willBeEmpty" marker.
    push_cell_().valueBuffer = raw_buffer{ willBeEmpty };
    return next_cell_();
}

std::size_t serializer::push_cell_guid(guid val)
{
    push_cell_().valueGuid = val;
    return next_cell_();
}

void serializer::skip_cell()
{
    std::memset(&push_cell_(), 0, sizeof(raw_cell));
    ++curColumnIndex_;
}

// TODO: assert column type in all write_cell calls!

void serializer::next_row()
{
    assert(lastSeqStep_ > SEQ_NONE_ &&
        "start() must be called before next_row()"
    );

    assert(lastSeqStep_ < SEQ_STRINGS_ &&
        "next_row() should not be called again after "
        "a call to begin_buffer_data_section() or finish()"
    );

    // Skip unpushed cells.
    const auto columnCount = column_count();
    while (curColumnIndex_ < columnCount)
    {
        skip_cell();
    }

    // Start next row.
    curColumnIndex_ = 0;
    ++rowCount_;
}

class string_table_
{
    const char* strings_;
    rad::vector<char> data_;

public:
    inline const char* data() const noexcept
    {
        return data_.data();
    }

    inline std::size_t size() const noexcept
    {
        return data_.size();
    }

    raw_string append(raw_string rawStr)
    {
        const auto str = strings_ + rawStr.dataOff;
        const auto strSize = std::strlen(str) + 1;

        rawStr.dataOff = static_cast<u32>(data_.size());
        // TODO: Switch to append_unchecked
        data_.append(str, str + strSize);

        return rawStr;
    }

    string_table_(const rad::vector<char>& strings, u32 serializerFlags)
        : strings_(strings.data())
        , data_(strings.allocator())
    {
        const auto exSize = (
            ((serializerFlags & SERIALIZER_FLAG_SUPPORT_NULL_STRING) ?
            std::size(null_string) : static_cast<std::size_t>(0))
        );

        data_.reserve(strings.size() + exSize);

        // Append null string if necessary.
        if (serializerFlags & SERIALIZER_FLAG_SUPPORT_NULL_STRING)
        {
            // TODO: Switch to append_unchecked
            data_.append(std::begin(null_string), std::end(null_string));
        }

        // Append table name.
        const auto tableName = strings.data(); // NOTE: First string is always the table name.
        const auto tableNameSize = std::strlen(tableName) + 1;

        // TODO: Switch to append_unchecked
        data_.append(tableName, tableName + tableNameSize);
    }
};

buffers_resolver serializer::begin_buffer_data_section(
    unsigned short bufferDataAlignment)
{
    assert(lastSeqStep_ > SEQ_NONE_ &&
        "start() must be called before begin_buffer_data_section()"
    );

    assert(lastSeqStep_ < SEQ_STRINGS_ &&
        "begin_buffer_data_section() should not be called "
        "more than once per call to start()"
    );

    // Commit any uncommitted cells.
    if (curColumnIndex_ != 0)
    {
        next_row();
    }

    // Validate cell count.
    const auto columnCount = column_count();

    assert(cells_.size() == (static_cast<std::size_t>(columnCount) * rowCount_) &&
        "UTF table cell size must be equal to (columnCount * rowCount)"
    );

    // Write columns.
    string_table_ stringTable(strings_, flags_);
    std::size_t cellIndex = 0;
    u16 columnIndex, rowSize = 0;

    for (columnIndex = 0; columnIndex < columnCount; ++columnIndex)
    {
        auto& columnMeta = columns_[columnIndex];
        raw_column rawColumn;

        // Append column name to string table.
        rawColumn.name = stringTable.append(columnMeta.name);

        // Determine flags to use.
        if (columnMeta.hasCells)
        {
            // Set default value if necessary.
            if (/*useDefaultValues_ &&*/ rowCount_ > 1 &&
                dispatch_by_cell_type<set_default_value_functor_>(
                    columnMeta.type(),
                    rawColumn.defaultValue,
                    cells_,
                    strings_.data(),
                    cellIndex,
                    columnCount))
            {
                columnMeta.flags |= COLUMN_FLAGS_HAS_DEFAULT_VALUE;

                if (columnMeta.type() == cell_type::string)
                {
                    // Copy default value to string table.
                    rawColumn.defaultValue.valueString = stringTable.append(
                        rawColumn.defaultValue.valueString
                    );
                }
            }

            // Set has-per-row-data flag if necessary.
            else
            {
                columnMeta.flags |= COLUMN_FLAGS_HAS_PER_ROW_DATA;

                columnMeta.dataOff = rowSize;
                rowSize += get_size_of_cell(columnMeta.type());
            }

            ++cellIndex;
        }

        rawColumn.flags = columnMeta.flags;

        // Write column.
        writer_.write_raw_column(rawColumn);
    }

    // Write rows.
    const auto perRowDataStartPos = writer_.stream().tell();
    const auto tablePos = (utfPos_ + 8);

    if (perRowDataStartPos - tablePos > UINT16_MAX)
    {
        throw std::runtime_error("UTF rows offset exceeds u16 range");
    }

    cellIndex = 0;
    rowsOff_ = static_cast<u16>(perRowDataStartPos - tablePos);
    rowSize_ = rowSize;

    for (u32 rowIndex = 0; rowIndex < rowCount_; ++rowIndex)
    {
        for (columnIndex = 0; columnIndex < columnCount; ++columnIndex)
        {
            const auto& columnMeta = columns_[columnIndex];

            if (columnMeta.hasCells)
            {
                if (columnMeta.flags & COLUMN_FLAGS_HAS_PER_ROW_DATA)
                {
                    if (columnMeta.type() == cell_type::string)
                    {
                        const auto rawStr = stringTable.append(
                            cells_[cellIndex].valueString
                        );

                        writer_.write_raw_string(rawStr);
                    }
                    else if (columnMeta.type() == cell_type::buffer)
                    {
                        writer_.write_empty_raw_buffer();
                    }
                    else
                    {
                        writer_.write_raw_cell(
                            columnMeta.type(),
                            cells_[cellIndex]
                        );
                    }
                }

                ++cellIndex;
            }
        }
    }

    // Write string table.
    stringTablePos_ = writer_.stream().tell();
    writer_.stream().write(stringTable.data(), stringTable.size());
    writer_.stream().pad(bufferDataAlignment);

    // Update state.
    bufDataPos_ = writer_.stream().tell();
    lastSeqStep_ = SEQ_STRINGS_;

    return buffers_resolver(*this);
}

void serializer::finish()
{
    assert(lastSeqStep_ >= SEQ_HEADER_ &&
        "finish() must not be called until after "
        "a matching call to start()"
    );

    // Write table data if begin_buffer_data_section() was not called.
    if (lastSeqStep_ < SEQ_STRINGS_)
    {
        begin_buffer_data_section();
    }

    // Fill-in UTF header.
    const auto endPos = writer_.stream().tell();
    writer_.stream().jump_to(utfPos_);

    const auto tablePos = (utfPos_ + 8);
    writer_.write_header(
        header{
            // TODO: Use C++20 delegated initializers.
            signature,
            static_cast<u32>(endPos - tablePos)
        }
    );

    // Fill-in table header.
    writer_.write_table_header(
        table_header{
            // TODO: Use C++20 delegated initializers.
            0,
            encoding_,
            rowsOff_,
            static_cast<u32>(stringTablePos_ - tablePos),
            static_cast<u32>(bufDataPos_ - tablePos),
            {
                static_cast<u32>(
                    (flags_ & SERIALIZER_FLAG_SUPPORT_NULL_STRING) ?
                    std::size(null_string) : 0)
            },
            column_count(),
            rowSize_,
            rowCount_
        }
    );

    writer_.stream().jump_to(endPos);
    lastSeqStep_ = SEQ_NONE_;
}

serializer::serializer(
    rad::stream& stream,
    rad::allocator& allocator) noexcept
    : writer_(stream)
    , strings_(allocator)
    , columns_(allocator)
    , cells_(allocator)
{
}
}
