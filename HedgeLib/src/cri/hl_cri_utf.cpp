#include "hedgelib/cri/hl_cri_utf.h"
#include <rad/rad_stack_or_heap_array.h>

namespace hl::cri_new::utf
{
bool column_info_group::validate(
    const utf::column_info* columnInfo,
    const utf::table_deserializer& td) const noexcept
{
    // Validate column count.
    const auto totalCount = get_total_count();
    if (td.column_count() < totalCount)
    {
        return false;
    }

    // Validate column types.
    u16 utfTableStartIndex = 0;

    for (const auto& columnRange : ranges)
    {
        if (!td.are_columns_exact_types(
            columnInfo + columnRange.beginIndex,
            utfTableStartIndex,
            columnRange.count))
        {
            return false;
        }

        utfTableStartIndex += columnRange.count;
    }

    return true;
}

void column_info_group::append_to(
    const utf::column_info* columnInfo,
    rad::vector<utf::column_info>& columns) const
{
    const auto totalCount = get_total_count();
    columns.reserve(columns.size() + totalCount);

    for (const auto& columnRange : ranges)
    {
        const auto rangeBegin = (columnInfo + columnRange.beginIndex);
        const auto rangeEnd = (rangeBegin + columnRange.count);

        columns.append(rangeBegin, rangeEnd); // TODO: Switch to append_unchecked
    }
}

u16 table_reader::read_as_u16(cell_type type)
{
    switch (type)
    {
    case cell_type::u16:
        return read_u16();

    case cell_type::u8:
        return read_u8();

    default:
        throw std::runtime_error("Cannot properly read value as u16");
    }
}

s16 table_reader::read_as_s16(cell_type type)
{
    switch (type)
    {
    case cell_type::s16:
        return read_s16();

    case cell_type::s8:
        return read_s8();

    default:
        throw std::runtime_error("Cannot properly read value as s16");
    }
}

u32 table_reader::read_as_u32(cell_type type)
{
    switch (type)
    {
    case cell_type::u32:
        return read_u32();

    case cell_type::u16:
        return read_u16();

    case cell_type::u8:
        return read_u8();

    default:
        throw std::runtime_error("Cannot properly read value as u32");
    }
}

s32 table_reader::read_as_s32(cell_type type)
{
    switch (type)
    {
    case cell_type::s32:
        return read_s32();

    case cell_type::s16:
        return read_s16();

    case cell_type::s8:
        return read_s8();

    default:
        throw std::runtime_error("Cannot properly read value as s32");
    }
}

u64 table_reader::read_as_u64(cell_type type)
{
    switch (type)
    {
    case cell_type::u64:
        return read_u64();

    case cell_type::u32:
        return read_u32();

    case cell_type::u16:
        return read_u16();

    case cell_type::u8:
        return read_u8();

    default:
        throw std::runtime_error("Cannot properly read value as u64");
    }
}

s64 table_reader::read_as_s64(cell_type type)
{
    switch (type)
    {
    case cell_type::s64:
        return read_s64();

    case cell_type::s32:
        return read_s32();

    case cell_type::s16:
        return read_s16();

    case cell_type::s8:
        return read_s8();

    default:
        throw std::runtime_error("Cannot properly read value as s64");
    }
}

raw_string table_reader::read_raw_string()
{
    return raw_string{ read_u32() };
}

raw_buffer table_reader::read_raw_buffer()
{
    return raw_buffer{ read_u32(), read_u32() };
}

guid table_reader::read_guid()
{
    //guid val;
    //stream().read_as(val);
    //return val;

    // TODO: Are UTF guids for sure 16-byte, and are they endian-swapped at all??
    throw std::runtime_error("Not yet implemented");
}

raw_table_header table_reader::read_raw_table_header()
{
    raw_table_header header;
    header.unknown1 = read_u8();
    header.encoding = static_cast<encoding_type>(read_u8());
    header.rowsOff = read_u16();
    header.stringsOff = read_u32();
    header.bufferDataOff = read_u32();
    header.name = read_raw_string();
    header.columnCount = read_u16();
    header.rowSize = read_u16();
    header.rowCount = read_u32();

    return header;
}

raw_utf_header table_reader::read_raw_utf_header()
{
    raw_utf_header header;
    header.signature = read_u32();
    header.tableSize = read_u32();

    return header;
}

rad::string table_reader::read_string(
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

rad::vector<unsigned char> table_reader::read_bytes(
    std::size_t size,
    rad::allocator& allocator)
{
    rad::vector<unsigned char> data(rad::no_value_init, allocator, size);
    stream().read(data.data(), size);
    return data;
}

void table_reader::read_raw_cell_value(cell_type type, void* dst)
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

raw_cell table_reader::read_raw_cell(cell_type type)
{
    raw_cell val;
    read_raw_cell_value(type, &val);
    return val;
}

raw_column table_reader::read_raw_column()
{
    raw_column val;
    val.flags = read_u8();

    if (val.has_name())
    {
        val.name = read_raw_string();
    }

    if (val.has_default_value())
    {
        read_raw_cell_value(val.type(), &val.defaultValue);
    }
    else if (!val.has_per_row_data())
    {
        // HACK: Handle empty columns by using a "zero-initialized" default value.
        // NOTE: These are actually used, for example in cpk files.
        //val.flags |= COLUMN_FLAGS_HAS_DEFAULT_VALUE;
        std::memset(&val.defaultValue, 0, sizeof(val.defaultValue));
    }

    return val;
}

void table_writer::write_string(
    const char* utf8Str,
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

void table_writer::write_raw_string(raw_string str)
{
    write_u32(str.dataOff);
}

void table_writer::write_raw_buffer(raw_buffer buffer)
{
    write_u32(buffer.dataOff);
    write_u32(buffer.size);
}

void table_writer::write_empty_raw_buffer()
{
    const raw_buffer rawBuffer = {};
    stream().write_as(rawBuffer);
}

void table_writer::write_guid(const guid& guid)
{
    // TODO: Are UTF guids for sure 16-byte, and are they endian-swapped at all??
    throw std::runtime_error("Not yet implemented");
}

void table_writer::write_raw_table_header(
    const raw_table_header& rawTblHeader)
{
    write_u8(rawTblHeader.unknown1);
    write_u8(rawTblHeader.encoding);
    write_u16(rawTblHeader.rowsOff);
    write_u32(rawTblHeader.stringsOff);
    write_u32(rawTblHeader.bufferDataOff);
    write_raw_string(rawTblHeader.name);
    write_u16(rawTblHeader.columnCount);
    write_u16(rawTblHeader.rowSize);
    write_u32(rawTblHeader.rowCount);
}

void table_writer::write_raw_utf_header(raw_utf_header rawUtfHeader)
{
    write_u32(rawUtfHeader.signature);
    write_u32(rawUtfHeader.tableSize);
}

void table_writer::write_raw_cell_value(cell_type type, const void* src)
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

void table_writer::write_raw_column(const raw_column& column)
{
    write_u8(column.flags);

    if (column.has_name())
    {
        write_raw_string(column.name);
    }

    if (column.has_default_value())
    {
        if (column.has_per_row_data())
        {
            throw std::runtime_error("Invalid column flags combination");
        }

        write_raw_cell(column.type(), column.defaultValue);
    }
}

unsigned long long table_deserializer::get_start_of_inner_table_(
    table_deserialize_type type)
{
    if (type == table_deserialize_type::utf)
    {
        const auto utfHeader = reader_.read_raw_utf_header();
        if (utfHeader.signature != signature)
        {
            throw std::runtime_error("Unsupported UTF data format");
        }
    }

    return reader_.stream().tell();
}

const raw_column* table_deserializer::next_column_()
{
    assert(nextColumnIndex_ + 1 <= column_count() &&
        "You're reading too many cells from the current row! "
        "Please remember to call next_row() as appropriate and "
        "validate that your expected column layout matches what's "
        "in the file before reading."
    );

    return columns_ + nextColumnIndex_++;
}

void table_deserializer::go_to_row_(u32 rowIndex)
{
    reader_.stream().jump_to(
        tablePos_ + header_.rowsOff + (header_.rowSize * rowIndex)
    );

    nextColumnIndex_ = 0;
    curRowIndex_ = rowIndex;
}

void table_deserializer::destruct_() noexcept
{
    if (!reader_.stream().capabilities().can_get_data_pointer())
    {
        allocator_->free(const_cast<char*>(stringTable_));
    }

    allocator_->free(columns_);
}

static bool is_cell_type_compatible_(
    cell_type cellType,
    cell_type expectedCellType) noexcept
{
    if (cellType != expectedCellType)
    {
        // Types are also compatible in cases where the actual cell type
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

    return true;
}

bool table_deserializer::is_column_compatible_type(
    u16 columnIndex,
    cell_type expectedColumnType) const noexcept
{
    assert(columnIndex < column_count() &&
        "Invalid column index; please ensure "
        "columnIndex < column_count() before calling"
    );

    const auto columnType = columns_[columnIndex].type();
    return is_cell_type_compatible_(columnType, expectedColumnType);
}

bool table_deserializer::is_column_exact_type(
    u16 columnIndex,
    cell_type expectedColumnType) const noexcept
{
    assert(columnIndex < column_count() &&
        "Invalid column index; please ensure "
        "columnIndex < column_count() before calling"
    );

    const auto columnType = columns_[columnIndex].type();
    return (columnType == expectedColumnType);
}

bool table_deserializer::are_columns_compatible_types(
    const column_info* columns,
    u16 beginIndex,
    u16 endIndex) const noexcept
{
    assert(beginIndex <= endIndex &&
        "Invalid index range; endIndex must be >= beginIndex"
    );

    assert(endIndex <= column_count() &&
        "Invalid index range; please ensure "
        "endIndex <= column_count() before calling"
    );

    for (u16 i = beginIndex; i < endIndex; ++i)
    {
        if (!is_column_compatible_type(i, columns[i].type))
        {
            return false;
        }
    }

    return true;
}

bool table_deserializer::are_columns_exact_types(
    const column_info* columns,
    u16 utfTableStartIndex,
    u16 columnCount) const noexcept
{
    assert(utfTableStartIndex <= column_count() &&
        "Invalid index range; please ensure "
        "utfTableStartIndex <= column_count() before calling"
    );

    const auto endIndex = utfTableStartIndex + columnCount;

    assert(endIndex <= column_count() &&
        "Invalid index range; please ensure "
        "utfTableStartIndex + columnCount <= column_count() "
        "before calling"
    );

    for (u16 i = utfTableStartIndex; i < endIndex; ++i)
    {
        if (!is_column_exact_type(i, columns->type))
        {
            return false;
        }

        ++columns;
    }

    return true;
}

rad::string table_deserializer::get_string_data(
    raw_string rawString,
    rad::allocator& allocator)
{
    if (rawString.dataOff >= string_table_size())
    {
        throw std::runtime_error("Invalid string data offset; "
            "offset was outside of the range of the string table"
        );
    }

    return rad::string(allocator, stringTable_ + rawString.dataOff);
}

rad::vector<unsigned char> table_deserializer::get_buffer_data(
    raw_buffer rawBuffer,
    rad::allocator& allocator)
{
    const auto curPos = reader_.stream().tell();
    reader_.stream().jump_to(get_buffer_data_position(rawBuffer));

    auto buf = reader_.read_bytes(rawBuffer.size, allocator);
    reader_.stream().jump_to(curPos);

    return buf;
}

u16 table_deserializer::get_column_index(const char* name) const
{
    for (u16 i = 0; i < header_.columnCount; ++i)
    {
        const auto& column = columns_[i];
        if (!column.has_name()) continue;

        const char* columnName = stringTable_ + column.name.dataOff;
        if (std::strcmp(columnName, name) == 0)
        {
            return i;
        }
    }
    
    return UINT16_MAX;
}

#define VALIDATE_READ_CELL_TYPE_(expectedType)\
    assert(is_cell_type_compatible_(curColumn->type(), expectedType) &&\
        "You're reading a cell using the incorrect type! "\
        "Please validate that your expected column layout "\
        "matches what's in the file before reading!"\
    )

#define READ_AS_OR_DEFAULT_(valueType, defaultValFieldName)\
    (curColumn->has_per_row_data()) ?\
        reader_.read_as_##valueType(curColumn->type()) :\
        curColumn->defaultValue.defaultValFieldName

#define READ_OR_DEFAULT_(valueType, defaultValFieldName)\
    (curColumn->has_per_row_data()) ?\
        reader_.read_##valueType() :\
        curColumn->defaultValue.defaultValFieldName

u8 table_deserializer::read_cell_as_u8()
{
    const auto curColumn = next_column_();
    VALIDATE_READ_CELL_TYPE_(cell_type::u8);

    return READ_OR_DEFAULT_(u8, valueU8);
}

s8 table_deserializer::read_cell_as_s8()
{
    const auto curColumn = next_column_();
    VALIDATE_READ_CELL_TYPE_(cell_type::s8);

    return READ_OR_DEFAULT_(s8, valueS8);
}

u16 table_deserializer::read_cell_as_u16()
{
    const auto curColumn = next_column_();
    VALIDATE_READ_CELL_TYPE_(cell_type::u16);

    return READ_AS_OR_DEFAULT_(u16, valueU16);
}

s16 table_deserializer::read_cell_as_s16()
{
    const auto curColumn = next_column_();
    VALIDATE_READ_CELL_TYPE_(cell_type::s16);

    return READ_AS_OR_DEFAULT_(s16, valueS16);
}

u32 table_deserializer::read_cell_as_u32()
{
    const auto curColumn = next_column_();
    VALIDATE_READ_CELL_TYPE_(cell_type::u32);

    return READ_AS_OR_DEFAULT_(u32, valueU32);
}

s32 table_deserializer::read_cell_as_s32()
{
    const auto curColumn = next_column_();
    VALIDATE_READ_CELL_TYPE_(cell_type::s32);

    return READ_AS_OR_DEFAULT_(s32, valueS32);
}

u64 table_deserializer::read_cell_as_u64()
{
    const auto curColumn = next_column_();
    VALIDATE_READ_CELL_TYPE_(cell_type::u64);

    return READ_AS_OR_DEFAULT_(u64, valueU64);
}

s64 table_deserializer::read_cell_as_s64()
{
    const auto curColumn = next_column_();
    VALIDATE_READ_CELL_TYPE_(cell_type::s64);

    return READ_AS_OR_DEFAULT_(s64, valueS64);
}

float table_deserializer::read_cell_as_f32()
{
    const auto curColumn = next_column_();
    VALIDATE_READ_CELL_TYPE_(cell_type::f32);

    return READ_OR_DEFAULT_(f32, valueF32);
}

double table_deserializer::read_cell_as_f64()
{
    const auto curColumn = next_column_();
    VALIDATE_READ_CELL_TYPE_(cell_type::f64);

    return READ_OR_DEFAULT_(f64, valueF64);
}

raw_string table_deserializer::read_cell_as_string()
{
    const auto curColumn = next_column_();
    VALIDATE_READ_CELL_TYPE_(cell_type::string);

    return READ_OR_DEFAULT_(raw_string, valueString);
}

raw_buffer table_deserializer::read_cell_as_buffer()
{
    const auto curColumn = next_column_();
    VALIDATE_READ_CELL_TYPE_(cell_type::buffer);

    return READ_OR_DEFAULT_(raw_buffer, valueBuffer);
}

guid table_deserializer::read_cell_as_guid()
{
    const auto curColumn = next_column_();
    VALIDATE_READ_CELL_TYPE_(cell_type::guid);

    return READ_OR_DEFAULT_(guid, valueGuid);
}

raw_cell table_deserializer::read_cell(cell_type* cellType)
{
    raw_cell c;
    const auto curColumn = next_column_();

    if (cellType)
    {
        *cellType = curColumn->type();
    }

    switch (curColumn->type())
    {
    case cell_type::u8:
        c.valueU8 = READ_OR_DEFAULT_(u8, valueU8);
        break;

    case cell_type::s8:
        c.valueS8 = READ_OR_DEFAULT_(s8, valueS8);
        break;

    case cell_type::u16:
        c.valueU16 = READ_AS_OR_DEFAULT_(u16, valueU16);
        break;

    case cell_type::s16:
        c.valueS16 = READ_AS_OR_DEFAULT_(s16, valueS16);
        break;

    case cell_type::u32:
        c.valueU32 = READ_AS_OR_DEFAULT_(u32, valueU32);
        break;

    case cell_type::s32:
        c.valueS32 = READ_AS_OR_DEFAULT_(s32, valueS32);
        break;

    case cell_type::u64:
        c.valueU64 = READ_AS_OR_DEFAULT_(u64, valueU64);
        break;

    case cell_type::s64:
        c.valueS64 = READ_AS_OR_DEFAULT_(s64, valueS64);
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

    return c;
}

void table_deserializer::skip_cell()
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

void table_deserializer::next_row()
{
    assert(curRowIndex_ < header_.rowCount &&
        "next_row() is being called too many times! "
        "Please ensure you are only calling it up to "
        "row_count() times."
    );

    go_to_row_(curRowIndex_ + 1);
}

void table_deserializer::go_to_cell(u16 columnIndex, u32 rowIndex)
{
    assert(columnIndex < header_.columnCount &&
        "Invalid column index was given; please ensure "
        "you check the column index against column_count()"
    );

    assert(rowIndex < header_.rowCount &&
        "Invalid row index was given; please ensure "
        "you check the row index against row_count()"
    );

    go_to_row_(rowIndex);

    for (u16 i = 0; i < columnIndex; ++i)
    {
        skip_cell();
    }
}

bool table_deserializer::try_go_to_cell(const char* columnName, u32 rowIndex)
{
    const auto columnIndex = get_column_index(columnName);
    if (columnIndex == UINT16_MAX) return false;

    go_to_cell(columnIndex, rowIndex);
    return true;
}

table_deserializer& table_deserializer::operator=(
    table_deserializer&& other) noexcept
{
    if (&other != this)
    {
        destruct_();

        allocator_ = other.allocator_;
        reader_ = std::move(other.reader_);
        tablePos_ = other.tablePos_;
        header_ = other.header_;
        columns_ = other.columns_;
        stringTable_ = other.stringTable_;
        nextColumnIndex_ = other.nextColumnIndex_;
        curRowIndex_ = other.curRowIndex_;

        other.columns_ = nullptr;
        other.stringTable_ = nullptr;
    }

    return *this;
}

table_deserializer::table_deserializer(
    rad::stream& stream,
    table_deserialize_type type,
    rad::allocator& allocator)
    : allocator_(&allocator)
    , reader_(stream)
    , tablePos_(get_start_of_inner_table_(type))
    , header_(reader_.read_raw_table_header())
    , columns_(allocator_->create<raw_column>(rad::no_value_init, header_.columnCount))
{
    try
    {
        // Read columns.
        for (u16 i = 0; i < header_.columnCount; ++i)
        {
            columns_[i] = reader_.read_raw_column();
        }

        // Read string table.
        if (header_.stringsOff > header_.bufferDataOff)
        {
            throw std::runtime_error(
                "Invalid UTF data; "
                "buffer data should not come before string table"
            );
        }

        reader_.stream().jump_to(tablePos_ + header_.stringsOff);
        const auto stringTableSize = string_table_size();

        if (reader_.stream().capabilities().can_get_data_pointer())
        {
            // Ensure the string table ends before the end of the stream.
            // NOTE: This is not necessary in the else block, because we
            // call read, which will throw in this case.
            if (reader_.stream().tell() + stringTableSize > reader_.stream().get_size())
            {
                throw std::runtime_error("Invalid UTF data; "
                    "string table extends past the end of the stream"
                );
            }

            // OPTIMIZATION: Just set the pointer directly if we can; no allocations.
            stringTable_ = static_cast<const char*>(reader_.stream().get_data_pointer());
        }
        else
        {
            // Read the string table from the stream.
            stringTable_ = static_cast<const char*>(
                allocator_->allocate(stringTableSize, alignof(char))
            );

            reader_.stream().read(const_cast<char*>(stringTable_), stringTableSize);
        }

        // Ensure string table ends with a null-terminator, so it is
        // always safe to use strcmp when searching for a string.
        if (stringTableSize && stringTable_[stringTableSize - 1] != '\0')
        {
            throw std::runtime_error("Invalid UTF data; "
                "string table does not end with null-terminator"
            );
        }
        
        // Jump to rows position.
        reader_.stream().jump_to(tablePos_ + header_.rowsOff);
    }
    catch (...)
    {
        destruct_();
        throw;
    }
}

table_deserializer::table_deserializer(
    table_deserializer&& other) noexcept
    : allocator_(other.allocator_)
    , reader_(std::move(other.reader_))
    , tablePos_(other.tablePos_)
    , header_(other.header_)
    , columns_(other.columns_)
    , stringTable_(other.stringTable_)
    , nextColumnIndex_(other.nextColumnIndex_)
    , curRowIndex_(other.curRowIndex_)
{
    other.columns_ = nullptr;
    other.stringTable_ = nullptr;
}

table_deserializer::~table_deserializer()
{
    destruct_();
}

table_serializer::column_::column_(cell_type type) noexcept
    : flags(static_cast<u8>(type))
{
}

raw_string table_serializer::add_string_(std::string_view str)
{
    const raw_string rawStr{ static_cast<u32>(strings_.size()) };

    strings_.append(
        str.begin(),
        str.end()
    );

    strings_.push_back('\0');

    return rawStr;
}

template<cell_type CellType>
class set_default_value_functor_
{
public:
    bool operator()(
        raw_cell& output,
        const rad::vector<raw_cell>& cells,
        unsigned short firstCellIndex,
        unsigned short columnCount)
    {
        if constexpr (CellType == cell_type::buffer)
        {
            // Don't bother checking buffers for equality since
            // they're all just placeholders at this point anyway.
            return false;
        }
        else
        {
            constexpr auto valMemberPtr = cell_type_traits<CellType>::cell_member_ptr;

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

            output.*valMemberPtr = cells[firstCellIndex].*valMemberPtr;
            return true;
        }
    }
};

void table_serializer::write_table_()
{
    // Commit any uncommitted cells.
    if (curColumnIndex_ != 0)
    {
        next_row();
    }

    // Validate column/row counts.
    assert(columns_.size() <= 65535 &&
        "UTF tables cannot have more than 65535 columns"
    );

    const auto columnCount = static_cast<unsigned short>(columns_.size());

    assert(cells_.size() == (static_cast<std::size_t>(columnCount) * rowCount_) &&
        "UTF table cell size was not equal to (columnCount * rowCount)"
    );

    // Write columns.
    std::size_t cellIndex = 0;
    unsigned short columnIndex, rowSize = 0;

    rad::stack_or_heap_array<bool, 128> skipColumn(
        rad::no_value_init,
        allocator(),
        columnCount
    );

    for (columnIndex = 0; columnIndex < columnCount; ++columnIndex)
    {
        // Set type.
        auto& column = columns_[columnIndex];

        raw_column rawColumn;
        rawColumn.flags = column.flags;
        rawColumn.name = column.name;

        if (column.hasCells)
        {
            // Set default value if necessary.
            if (useDefaultValues_ && dispatch_by_cell_type<set_default_value_functor_>(
                column.type(),
                rawColumn.defaultValue,
                cells_,
                cellIndex,
                columnCount))
            {
                rawColumn.flags |= COLUMN_FLAGS_HAS_DEFAULT_VALUE;
                skipColumn[columnIndex] = true;
            }

            // Set has-per-row-data flag if necessary.
            else
            {
                rawColumn.flags |= COLUMN_FLAGS_HAS_PER_ROW_DATA;
                skipColumn[columnIndex] = false;

                column.dataOff = rowSize;
                rowSize += get_size_of_cell(column.type());
            }

            ++cellIndex;
        }
        else
        {
            skipColumn[columnIndex] = true;
        }

        // Write column.
        writer_.write_raw_column(rawColumn);
    }

    // Write rows.
    const auto perRowDataStartPos = writer_.stream().tell();
    const auto tablePos = (utfPos_ + 8);

    cellIndex = 0;
    rowsOff_ = static_cast<unsigned short>(perRowDataStartPos - tablePos);
    rowSize_ = rowSize;

    for (unsigned long rowIndex = 0; rowIndex < rowCount_; ++rowIndex)
    {
        for (columnIndex = 0; columnIndex < columnCount; ++columnIndex)
        {
            if (!skipColumn[columnIndex])
            {
                writer_.write_raw_cell(
                    columns_[columnIndex].type(),
                    cells_[cellIndex]
                );
            }

            ++cellIndex;
        }
    }

    lastSeqStep_ = SEQ_TABLE_;
}

u32 table_serializer::compute_buffer_size_(
    unsigned long long dataStartPos) const
{
    const auto dataEndPos = writer_.stream().tell();

    assert(dataEndPos >= dataStartPos &&
        "Buffer data position is invalid"
    );

    assert((dataEndPos - dataStartPos) <= UINT32_MAX &&
        "Buffer size is too large"
    );

    return static_cast<u32>(dataEndPos - dataStartPos);
}

void table_serializer::start(
    std::string_view tableName,
    rad::span<const column_info> columnsInfo,
    encoding_type encoding,
    bool writeNullString,
    bool useDefaultValues)
{
    // Reset fields.
    encoding_ = encoding;
    curColumnIndex_ = 0;
    rowCount_ = 0;
    strings_.clear();
    bufDataPos_ = stringsPos_ = utfPos_ = writer_.stream().tell();

    if (writeNullString)
    {
        add_string_(null_string);
    }

    tableName_ = add_string_(tableName);
    hasNullString_ = writeNullString;
    useDefaultValues_ = useDefaultValues;
    rowsOff_ = 0;
    rowSize_ = 0;
    cells_.clear();

    columns_.clear();
    columns_.reserve(columnsInfo.size());

    for (const auto& columnInfo : columnsInfo)
    {
        auto& column = columns_.emplace_back(columnInfo.type);

        // Set name if necessary.
        if (columnInfo.name)
        {
            column.name = add_string_(columnInfo.name);
            column.flags |= COLUMN_FLAGS_HAS_NAME;
        }
    }

    // Write placeholder UTF and table headers.
    writer_.stream().write_nulls(32);
    lastSeqStep_ = SEQ_HEADER_;
}

std::size_t table_serializer::write_cell_as_u8(u8 val)
{
    cells_.push_back(rad::no_value_init).valueU8 = val;

    columns_[curColumnIndex_].hasCells = true;
    ++curColumnIndex_;

    return cells_.back_index();
}

std::size_t table_serializer::write_cell_as_s8(s8 val)
{
    cells_.push_back(rad::no_value_init).valueS8 = val;

    columns_[curColumnIndex_].hasCells = true;
    ++curColumnIndex_;

    return cells_.back_index();
}

std::size_t table_serializer::write_cell_as_u16(u16 val)
{
    cells_.push_back(rad::no_value_init).valueU16 = val;

    columns_[curColumnIndex_].hasCells = true;
    ++curColumnIndex_;

    return cells_.back_index();
}

std::size_t table_serializer::write_cell_as_s16(s16 val)
{
    cells_.push_back(rad::no_value_init).valueS16 = val;

    columns_[curColumnIndex_].hasCells = true;
    ++curColumnIndex_;

    return cells_.back_index();
}

std::size_t table_serializer::write_cell_as_u32(u32 val)
{
    cells_.push_back(rad::no_value_init).valueU32 = val;

    columns_[curColumnIndex_].hasCells = true;
    ++curColumnIndex_;

    return cells_.back_index();
}

std::size_t table_serializer::write_cell_as_s32(s32 val)
{
    cells_.push_back(rad::no_value_init).valueS32 = val;

    columns_[curColumnIndex_].hasCells = true;
    ++curColumnIndex_;

    return cells_.back_index();
}

std::size_t table_serializer::write_cell_as_u64(u64 val)
{
    cells_.push_back(rad::no_value_init).valueU64 = val;

    columns_[curColumnIndex_].hasCells = true;
    ++curColumnIndex_;

    return cells_.back_index();
}

std::size_t table_serializer::write_cell_as_s64(s64 val)
{
    cells_.push_back(rad::no_value_init).valueS64 = val;

    columns_[curColumnIndex_].hasCells = true;
    ++curColumnIndex_;

    return cells_.back_index();
}

std::size_t table_serializer::write_cell_as_f32(float val)
{
    cells_.push_back(rad::no_value_init).valueF32 = val;

    columns_[curColumnIndex_].hasCells = true;
    ++curColumnIndex_;

    return cells_.back_index();
}

std::size_t table_serializer::write_cell_as_f64(double val)
{
    cells_.push_back(rad::no_value_init).valueF64 = val;

    columns_[curColumnIndex_].hasCells = true;
    ++curColumnIndex_;

    return cells_.back_index();
}

std::size_t table_serializer::write_cell_as_string(std::string_view val)
{
    cells_.push_back(rad::no_value_init).valueString = (
        (hasNullString_ && val == null_string) ?
        raw_string{} : add_string_(val)
    );

    columns_[curColumnIndex_].hasCells = true;
    ++curColumnIndex_;

    return cells_.back_index();
}

std::size_t table_serializer::write_cell_as_buffer()
{
    cells_.push_back(rad::no_value_init).valueBuffer = raw_buffer{};

    columns_[curColumnIndex_].hasCells = true;
    ++curColumnIndex_;

    return cells_.back_index();
}

std::size_t table_serializer::write_cell_as_guid(guid val)
{
    cells_.push_back(rad::no_value_init).valueGuid = val;

    columns_[curColumnIndex_].hasCells = true;
    ++curColumnIndex_;

    return cells_.back_index();
}

void table_serializer::skip_cell()
{
    std::memset(&cells_.push_back(rad::no_value_init), 0, sizeof(raw_cell));
    ++curColumnIndex_;
}

// TODO: assert curColumnIndex at the beginning of all write_cell and skip_cell calls!

void table_serializer::next_row()
{
    curColumnIndex_ = 0;
    ++rowCount_;
}

void table_serializer::finish_rows(unsigned short bufferDataAlignment)
{
    // Write table data.
    write_table_();

    // Write strings.
    stringsPos_ = writer_.stream().tell();
    writer_.stream().write(strings_.data(), strings_.size());
    writer_.stream().pad(bufferDataAlignment);

    // Update state.
    bufDataPos_ = writer_.stream().tell();
    lastSeqStep_ = SEQ_STRINGS_;
}

void table_serializer::fill_buffer(
    unsigned long long bufferPos,
    unsigned long long dataPos,
    u32 dataSize)
{
    // TODO: Assert sequence (>= SEQ_STRINGS_)
    // NOTE: The check against stringsPos_ in the following assert is unsafe until we do this!

    assert(bufferPos >= utfPos_ && bufferPos < stringsPos_ &&
        "The given buffer position must be within the per-row-data region"
    );

    assert(dataPos >= bufDataPos_ &&
        "The given buffer data position must be within the buffer data region"
    );

    // Jump to buffer position.
    const auto pos = writer_.stream().tell();
    writer_.stream().jump_to(bufferPos);

    // Fill-in buffer data position and size.
    writer_.write_raw_buffer({
        static_cast<u32>(dataPos - bufDataPos_),
        dataSize
    });
    
    // Jump back to previous stream position.
    writer_.stream().jump_to(pos);
    lastSeqStep_ = SEQ_BUFFER_DATA_;
}

void table_serializer::fill_buffer(
    unsigned long long bufferPos,
    unsigned long long dataPos)
{
    const auto dataSize = compute_buffer_size_(dataPos);
    fill_buffer(bufferPos, dataPos, dataSize);
}

void table_serializer::fill_buffer_cell(
    std::size_t cellIndex,
    unsigned long long dataPos,
    u32 dataSize)
{
    assert(cellIndex < cells_.size() &&
        "Invalid cell index"
    );

    const auto columnIndex = cellIndex % columns_.size();
    const auto rowIndex = cellIndex / columns_.size();
    const auto tablePos = utfPos_ + 8;
    const auto perRowDataPos = tablePos + rowsOff_;

    const auto bufferPos = (
        perRowDataPos +
        (rowSize_ * rowIndex) +
        columns_[columnIndex].dataOff
    );

    fill_buffer(bufferPos, dataPos, dataSize);
}

void table_serializer::fill_buffer_cell(
    std::size_t cellIndex,
    unsigned long long dataPos)
{
    const auto dataSize = compute_buffer_size_(dataPos);
    fill_buffer_cell(cellIndex, dataPos, dataSize);
}

void table_serializer::finish()
{
    // TODO: Assert sequence

    if (lastSeqStep_ == SEQ_HEADER_)
    {
        finish_rows();
    }

    const auto endPos = writer_.stream().tell();

    writer_.stream().jump_to(utfPos_);

    // Fill-in UTF header.
    const auto tablePos = (utfPos_ + 8);
    writer_.write_raw_utf_header({
        signature,
        static_cast<u32>(endPos - tablePos)
    });

    // Fill-in table header.
    raw_table_header rawTblHeader;
    rawTblHeader.unknown1 = 0;
    rawTblHeader.encoding = encoding_;
    rawTblHeader.rowsOff = rowsOff_;
    rawTblHeader.stringsOff = static_cast<u32>(stringsPos_ - tablePos);
    rawTblHeader.bufferDataOff = static_cast<u32>(bufDataPos_ - tablePos);
    rawTblHeader.name = tableName_;
    rawTblHeader.columnCount = static_cast<u16>(columns_.size());
    rawTblHeader.rowSize = rowSize_;
    rawTblHeader.rowCount = rowCount_;

    writer_.write_raw_table_header(rawTblHeader);
    writer_.stream().jump_to(endPos);

    lastSeqStep_ = SEQ_NONE_;
}

table_serializer::table_serializer(
    rad::stream& stream,
    rad::allocator& allocator) noexcept
    : writer_(stream)
    , strings_(allocator)
    , columns_(allocator)
    , cells_(allocator)
{
}
}
