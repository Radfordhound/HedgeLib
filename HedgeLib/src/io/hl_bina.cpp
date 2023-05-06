#include "hedgelib/io/hl_bina.h"
#include <cstring>
#include <cassert>

namespace hl
{
namespace bina
{
endian_flag pac_pack_meta::guess_endianness(
    const void* rawData, std::size_t dataSize) const noexcept
{
    // If offCount is greater than the largest possible amount of
    // offsets in the file, the endianness needs to be swapped.
    const u32* eof = ptradd<u32>(rawData, dataSize);

    return (offCount > static_cast<u32>(eof - off_table())) ?
#ifdef HL_IS_BIG_ENDIAN
        endian_flag::little : endian_flag::big;
#else
        endian_flag::big : endian_flag::little;
#endif
}

void pac_pack_meta::fix(void* rawData, endian_flag endianFlag)
{
    // Swap metadata header if necessary.
    if (needs_swap(endianFlag))
    {
        endian_swap();
    }

    // Fix offsets.
    for (hl::u32& relOffPos : offsets())
    {
        // Endian swap offset position if necessary.
        if (needs_swap(endianFlag))
        {
            hl::endian_swap(relOffPos);
        }

        // Get pointer to current offset.
        off32<void>* curOff = ptradd<off32<void>>(rawData, relOffPos);

        // Endian swap offset if necessary.
#ifndef HL_IS_BIG_ENDIAN
        hl::endian_swap(*curOff);
#endif

        // Fix offset.
        curOff->fix(rawData);
    }
}

const pac_pack_meta* get_pac_pack_meta(const void* rawData, std::size_t dataSize)
{
    const u8* curDataPtr = static_cast<const u8*>(rawData);
    const u8* endDataPtr = (curDataPtr + dataSize);

    // Search file for "PACPACK_METADATA" and return pointer if found.
    while ((curDataPtr + sizeof(pac_pack_meta)) < endDataPtr)
    {
        if (std::memcmp(curDataPtr, "PACPACK_METADATA", 16) == 0)
        {
            return reinterpret_cast<const pac_pack_meta*>(curDataPtr);
        }

        ++curDataPtr;
    }

    return nullptr;
}

template<typename addr_t>
static void in_strings_write(std::size_t dataPos,
    endian_flag endianFlag, const str_table& strTable,
    off_table& offTable, stream& stream)
{
    std::unique_ptr<bool[]> isDuplicateEntry(new bool[strTable.size()]());

    // Write strings and fix offsets in string entries.
    for (std::size_t i = 0; i < strTable.size(); ++i)
    {
        // Skip the current string entry if it's a duplicate.
        if (isDuplicateEntry[i]) continue;

        // Add offset position to offset table.
        const str_table_entry& curEntry = strTable[i];
        offTable.push_back(curEntry.offPos);

        // Jump to offset position.
        const std::size_t curStrPos = stream.tell();
        stream.jump_to(curEntry.offPos);

        // Compute offset.
        auto off = static_cast<addr_t>(curStrPos - dataPos);

        // Swap offset if necessary.
        if (needs_swap(endianFlag))
        {
            hl::endian_swap(off);
        }

        // Write fixed offset.
        stream.write_obj(off);

        // Mark duplicate string entries and fix duplicate offsets.
        for (std::size_t i2 = (i + 1); i2 < strTable.size(); ++i2)
        {
            // If we've found a duplicate string entry...
            const str_table_entry& dupEntry = strTable[i2];
            if (curEntry.str == dupEntry.str)
            {
                // Add offset value to offset table.
                offTable.push_back(dupEntry.offPos);

                // Jump to offset position.
                stream.jump_to(dupEntry.offPos);

                // Write fixed offset.
                stream.write_obj(off);

                // Mark this string entry as a duplicate.
                isDuplicateEntry[i2] = true;
            }
        }

        // Jump to string position.
        stream.jump_to(curStrPos);

        // Write string.
        stream.write_str(curEntry.str);
    }

    // Write padding.
    // NOTE: We pad to 4 even when writing 64-bit data, like Sonic Team.
    stream.pad(4);
}

void strings_write32(std::size_t dataPos,
    endian_flag endianFlag, const str_table& strTable,
    off_table& offTable, stream& stream)
{
    in_strings_write<u32>(dataPos, endianFlag,
        strTable, offTable, stream);
}

void strings_write64(std::size_t dataPos,
    endian_flag endianFlag, const str_table& strTable,
    off_table& offTable, stream& stream)
{
    in_strings_write<u64>(dataPos, endianFlag,
        strTable, offTable, stream);
}

const u8* off_table_handle::in_get_real_off_table_end(
    const u8* offTable, u32 offTableSize) noexcept
{
    // Get "real" end of offset table (not counting padding bytes).
    const u8* offTableEnd = (offTable + offTableSize);
    while (offTableEnd > offTable && *(offTableEnd - 1) == 0)
    {
        --offTableEnd;
    }

    return offTableEnd;
}

static u32 in_grab_six_bits(const u8*& curOffTablePtr) noexcept
{
    return static_cast<u32>(*(curOffTablePtr++) &
        static_cast<u8>(offset_flags::data_mask));
}

static u32 in_grab_eight_bits(const u8*& curOffTablePtr) noexcept
{
    return static_cast<u32>(*(curOffTablePtr++));
}

static u32 in_grab_fourteen_bits(const u8*& curOffTablePtr) noexcept
{
    u32 val = (in_grab_six_bits(curOffTablePtr) << 8);
    val |= in_grab_eight_bits(curOffTablePtr);
    return val;
}

static u32 in_grab_thirty_bits(const u8*& curOffTablePtr) noexcept
{
    u32 val = (in_grab_six_bits(curOffTablePtr) << 24);
    val |= (in_grab_eight_bits(curOffTablePtr) << 16);
    val |= (in_grab_eight_bits(curOffTablePtr) << 8);
    val |= in_grab_eight_bits(curOffTablePtr);
    return val;
}

u32 off_table_handle::iterator::in_get_cur_rel_off_pos() const noexcept
{
    // Return the relative offset position at this position.
    const u8* curOffTablePtr = m_curOffTablePtr;
    switch (*curOffTablePtr & static_cast<u8>(offset_flags::size_mask))
    {
    case static_cast<u8>(offset_flags::size_six_bit) :
        return (in_grab_six_bits(curOffTablePtr) * sizeof(u32));

    case static_cast<u8>(offset_flags::size_fourteen_bit) :
        return (in_grab_fourteen_bits(curOffTablePtr) * sizeof(u32));

    case static_cast<u8>(offset_flags::size_thirty_bit) :
        return (in_grab_thirty_bits(curOffTablePtr) * sizeof(u32));

    default:
        // A size of 0 indicates that we've reached the end of the offset table.
        // (NOTE: This shouldn't happen under normal circumstances as we
        // skip padding already when constructing these in off_table_handle.)
        return 0;
    }
}

const u8* off_table_handle::iterator::in_get_next_off_table_ptr() const noexcept
{
    // Return the "next" offset table pointer.
    switch (*m_curOffTablePtr & static_cast<u8>(offset_flags::size_mask))
    {
    case static_cast<u8>(offset_flags::size_six_bit) :
        return (m_curOffTablePtr + 1);

    case static_cast<u8>(offset_flags::size_fourteen_bit) :
        return (m_curOffTablePtr + 2);

    case static_cast<u8>(offset_flags::size_thirty_bit) :
        return (m_curOffTablePtr + 4);

    default:
        // A size of 0 indicates that we've reached the end of the offset table.
        // (NOTE: This shouldn't happen under normal circumstances as we
        // skip padding already when constructing these in off_table_handle.)
        return m_curOffTablePtr;
    }
}

bool off_table_handle::iterator::next() noexcept
{
    // Increase the current offset table pointer.
    m_curOffTablePtr = in_get_next_off_table_ptr();

    // If we're not at the end of the offset table yet...
    if (*m_curOffTablePtr)
    {
        // Increase the current relative offset table position and return true.
        m_curRelOffPos += in_get_cur_rel_off_pos();
        return true;
    }

    // We're at the end of the offset table; return false.
    else
    {
        return false;
    }
}

template<template<typename> class off_t>
static void in_offsets_fix(off_table_handle offTable,
    const endian_flag endianFlag, void* base)
{
    for (u32 relOffPos : offTable)
    {
        // Get pointer to current offset.
        const auto curOffPtr = ptradd<off_t<void>>(base, relOffPos);

        // Endian-swap offset if necessary.
        if (needs_swap(endianFlag))
        {
            hl::endian_swap(*curOffPtr);
        }

        // Fix offset.
        curOffPtr->fix(base);
    }
}

void offsets_fix32(off_table_handle offTable,
    const endian_flag endianFlag, void* base)
{
    in_offsets_fix<off32>(offTable, endianFlag, base);
}

void offsets_fix64(off_table_handle offTable,
    const endian_flag endianFlag, void* base)
{
    in_offsets_fix<off64>(offTable, endianFlag, base);
}

template<template<typename> class off_t>
void in_offsets_copy(off_table_handle srcOffTable,
    const void* srcBase, void* dstBase)
{
    const auto srcBaseAddr = reinterpret_cast<std::uintptr_t>(srcBase);
    const auto dstBaseAddr = reinterpret_cast<std::uintptr_t>(dstBase);

    for (u32 relOffPos : srcOffTable)
    {
        // Get pointer to current source offset.
        const auto srcOffPtr = ptradd<off_t<void>>(srcBase, relOffPos);

        // Get the address of the value the offset points to.
        const auto offVal = reinterpret_cast<std::uintptr_t>(srcOffPtr->get());

        // Convert it to an address relative to the source base.
        const auto offRelVal = (offVal - srcBaseAddr);

        // Get pointer to current destination offset.
        const auto dstOffPtr = ptradd<off_t<void>>(dstBase, relOffPos);

        // Fix destination offset.
        *dstOffPtr = reinterpret_cast<void*>(dstBaseAddr + offRelVal);
    }
}

void offsets_copy32(off_table_handle srcOffTable,
    const void* srcBase, void* dstBase)
{
    // NOTE: We only fix up the copied offsets if they are absolute.
    // If the offsets are relative, nothing needs to be done.

#if UINTPTR_MAX > UINT32_MAX
    in_offsets_copy<off32>(srcOffTable, srcBase, dstBase);
#endif
}

void offsets_copy64(off_table_handle srcOffTable,
    const void* srcBase, void* dstBase)
{
    // NOTE: We only fix up the copied offsets if they are absolute.
    // If the offsets are relative, nothing needs to be done.

#if UINTPTR_MAX > UINT64_MAX
    in_offsets_copy<off64>(srcOffTable, srcBase, dstBase);
#endif
}

void offsets_write_no_sort(std::size_t dataPos,
    const off_table& offTable, stream& stream)
{
    std::size_t lastOffPos = dataPos;
    for (auto curOffPos : offTable)
    {
        // Get current offset position.
        const std::size_t curRelOffPos = ((curOffPos - lastOffPos) >> 2);
        std::size_t binaRelOffPosSize;
        u8 binaRelOffPos[4];

        // Compute BINA relative offset position.
        if (curRelOffPos <= 0x3FU) // 6-bit position.
        {
            // Store first 6 bits of relative offset position.
            binaRelOffPos[0] = static_cast<u8>(curRelOffPos &
                static_cast<u8>(offset_flags::data_mask));

            // Store 6-bit size marker.
            binaRelOffPos[0] |= static_cast<u8>(offset_flags::size_six_bit);

            // State that this BINA relative offset is 1 byte in size.
            binaRelOffPosSize = 1;
        }
        else if (curRelOffPos <= 0x3FFFU) // 14-bit position.
        {
            // Store first 6 bits of relative offset position.
            binaRelOffPos[0] = static_cast<u8>((curRelOffPos & 0x3F00U) >> 8);

            // Store 14-bit size marker.
            binaRelOffPos[0] |= static_cast<u8>(offset_flags::size_fourteen_bit);

            // Store last 8 bits of relative offset position.
            binaRelOffPos[1] = static_cast<u8>(curRelOffPos & 0xFFU);

            // State that this BINA relative offset is 2 bytes in size.
            binaRelOffPosSize = 2;
        }
        else if (curRelOffPos <= 0x3FFFFFFFU) // 30-bit position.
        {
            // Store first 6 bits of relative offset position.
            binaRelOffPos[0] = static_cast<u8>((curRelOffPos & 0x3F000000U) >> 24);

            // Store 30-bit size marker.
            binaRelOffPos[0] |= static_cast<u8>(offset_flags::size_thirty_bit);

            // Store last 24 bits of relative offset position.
            binaRelOffPos[1] = static_cast<u8>((curRelOffPos & 0xFF0000U) >> 16);
            binaRelOffPos[2] = static_cast<u8>((curRelOffPos & 0xFF00U) >> 8);
            binaRelOffPos[3] = static_cast<u8>(curRelOffPos & 0xFFU);

            // State that this BINA relative offset is 4 bytes in size.
            binaRelOffPosSize = 4;
        }
        else
        {
            // BINA relative offset positions *must* fit within 30 bits.
            throw out_of_range_exception();
        }

        // Write BINA relative offset position to file.
        stream.write_all(binaRelOffPosSize, binaRelOffPos);
        
        // Set last offset position for next iteration.
        lastOffPos = curOffPos;
    }
}

void offsets_write(std::size_t dataPos,
    off_table& offTable, stream& stream)
{
    // Sort offset table.
    std::sort(offTable.begin(), offTable.end());

    // Write sorted offsets.
    offsets_write_no_sort(dataPos, offTable, stream);
}

namespace v1
{
off_table_handle raw_header::offsets() const noexcept
{
    return off_table_handle(offTable.get(), offTableSize);
}

void raw_header::fix()
{
    // Endian-swap header if necessary.
    if (needs_swap(endianFlag))
    {
        endian_swap();
    }

    // Fix offset table offset.
    void* dataPtr = data();
    offTable.fix(dataPtr);

    // Fix offsets.
    offsets_fix32(offsets(), endianFlag, dataPtr);
}
} // v1

namespace v2
{
template<template<typename> class off_t>
static void in_fix(raw_data_block_header& dataBlock, endian_flag endianFlag)
{
    // Swap header if necessary.
    if (needs_swap(endianFlag))
    {
        dataBlock.endian_swap();
    }

    // Fix string table offset.
    void* dataPtr = dataBlock.data();
    dataBlock.strTable.fix(dataPtr);

    // Fix data offsets.
    in_offsets_fix<off_t>(dataBlock.offsets(), endianFlag, dataPtr);
}

void raw_data_block_header::fix32(endian_flag endianFlag)
{
    in_fix<off32>(*this, endianFlag);
}

void raw_data_block_header::fix64(endian_flag endianFlag)
{
    in_fix<off64>(*this, endianFlag);
}

void raw_data_block_header::start_write(endian_flag endianFlag,
    stream& stream)
{
    // Generate data block header.
    raw_data_block_header dataBlock =
    {
        static_cast<u32>(raw_block_type::data),                         // signature
        0U,                                                             // size
        0U,                                                             // strTable
        0U,                                                             // strTableSize
        0U,                                                             // offTableSize
        sizeof(raw_data_block_header)                                   // relativeDataOffset
    };

    // Endian swap if necessary.
    if (needs_swap(endianFlag))
    {
        hl::endian_swap(dataBlock.relativeDataOffset);
    }

    // Write data block to file.
    stream.write_obj(dataBlock);

    // HACK: Re-use dataBlock memory to write padding.
    dataBlock.signature = 0;
    dataBlock.relativeDataOffset = 0;

    // Write padding.
    stream.write_obj(dataBlock);
}

void raw_data_block_header::finish_write(std::size_t dataBlockPos,
    std::size_t strTablePos, std::size_t offTablePos,
    endian_flag endianFlag, stream& stream)
{
    // Get end of stream position.
    const std::size_t endPos = stream.tell();

    // Jump to data block size position.
    stream.jump_to(dataBlockPos + offsetof(raw_data_block_header, size));

    // Compute data block header values.
    const std::size_t dataPos = (dataBlockPos + (sizeof(raw_data_block_header) * 2));
    struct
    {
        u32 size;
        off32<char> strTable;
        u32 strTableSize;
        u32 offTableSize;
    }
    values;

    values.size = static_cast<u32>(endPos - dataBlockPos);
    values.strTable = off32<char>(static_cast<u32>(strTablePos - dataPos));
    values.strTableSize = static_cast<u32>(offTablePos - strTablePos);
    values.offTableSize = static_cast<u32>(endPos - offTablePos);
    
    // Endian-swap data block header values if necessary.
    if (needs_swap(endianFlag))
    {
        hl::endian_swap(values.size);
        hl::endian_swap(values.strTable);
        hl::endian_swap(values.strTableSize);
        hl::endian_swap(values.offTableSize);
    }

    // Fill-in data block header values.
    stream.write_obj(values);

    // Jump back to end of stream.
    stream.jump_to(endPos);
}

template<typename addr_t>
static void in_finish_write(std::size_t dataBlockPos,
    endian_flag endianFlag, const hl::str_table& strTable,
    hl::off_table& offTable, stream& stream)
{
    const std::size_t dataPos = (dataBlockPos + (sizeof(raw_data_block_header) * 2));

    // Write string table.
    const std::size_t strTablePos = stream.tell();
    in_strings_write<addr_t>(dataPos, endianFlag, strTable, offTable, stream);

    // Write offset table.
    const std::size_t offTablePos = stream.tell();
    offsets_write(dataPos, offTable, stream);

    // NOTE: We pad to 4 even when writing 64-bit data, like Sonic Team.
    stream.pad(4);

    // Fill-in data block header values.
    raw_data_block_header::finish_write(dataBlockPos,
        strTablePos, offTablePos, endianFlag, stream);
}

void raw_data_block_header::finish_write32(std::size_t dataBlockPos,
    endian_flag endianFlag, const hl::str_table& strTable,
    hl::off_table& offTable, stream& stream)
{
    in_finish_write<u32>(dataBlockPos, endianFlag,
        strTable, offTable, stream);
}

void raw_data_block_header::finish_write64(std::size_t dataBlockPos,
    endian_flag endianFlag, const hl::str_table& strTable,
    hl::off_table& offTable, stream& stream)
{
    in_finish_write<u64>(dataBlockPos, endianFlag,
        strTable, offTable, stream);
}

const_block_iterator& const_block_iterator::operator++() noexcept
{
    if (++m_curBlockIndex > m_blockCount)
    {
        m_curBlock = nullptr;
        m_curBlockIndex = m_blockCount = 0;
    }
    else
    {
        m_curBlock = m_curBlock->next_block();
    }

    return *this;
}

bool const_block_iterator::operator==(const const_block_iterator& other) const noexcept
{
    return (m_curBlock == other.m_curBlock &&
        m_curBlockIndex == other.m_curBlockIndex &&
        m_blockCount == other.m_blockCount);
}

bool const_block_iterator::operator!=(const const_block_iterator& other) const noexcept
{
    return (m_curBlock != other.m_curBlock ||
        m_curBlockIndex != other.m_curBlockIndex ||
        m_blockCount != other.m_blockCount);
}

block_iterator& block_iterator::operator++() noexcept
{
    if (++m_curBlockIndex >= m_blockCount)
    {
        m_curBlock = nullptr;
        m_curBlockIndex = m_blockCount = 0;
    }
    else
    {
        m_curBlock = m_curBlock->next_block();
    }

    return *this;
}

bool block_iterator::operator==(const block_iterator& other) const noexcept
{
    return (m_curBlock == other.m_curBlock &&
        m_curBlockIndex == other.m_curBlockIndex &&
        m_blockCount == other.m_blockCount);
}

bool block_iterator::operator!=(const block_iterator& other) const noexcept
{
    return (m_curBlock != other.m_curBlock ||
        m_curBlockIndex != other.m_curBlockIndex ||
        m_blockCount != other.m_blockCount);
}

const raw_block_header* raw_header::get_block(raw_block_type type) const noexcept
{
    for (auto block : blocks())
    {
        // Return if this is the block we're looking for.
        if (block->signature == static_cast<u32>(type))
        {
            return block;
        }
    }

    return nullptr;
}

template<template<typename> class off_t>
static void in_fix(raw_header& header)
{
    // Swap header if necessary.
    if (needs_swap(header.endianFlag))
    {
        header.endian_swap();
    }

    // Fix blocks.
    for (auto block : header.blocks())
    {
        switch (block->signature)
        {
        case static_cast<u32>(raw_block_type::data):
        {
            const auto dataBlock = reinterpret_cast<raw_data_block_header*>(block);
            in_fix<off_t>(*dataBlock, header.endianFlag);
            break;
        }

        default:
            throw unsupported_exception();
        }
    }
}

void raw_header::fix32()
{
    in_fix<off32>(*this);
}

void raw_header::fix64()
{
    in_fix<off64>(*this);
}

void raw_header::start_write(ver version,
    bina::endian_flag endianFlag, stream& stream)
{
    // Generate BINAV2 header.
    const raw_header binaHeader =
    {
        sig,            // signature
        version,        // version
        endianFlag,     // endianFlag
        0,              // fileSize
        0               // blockCount
    };

    // NOTE: We don't need to swap the header yet since the only values
    // that ever need to be swapped are going to be filled-in later.

    // Write BINAV2 header.
    stream.write_obj(binaHeader);
}

void raw_header::finish_write(std::size_t headerPos, u16 blockCount,
    bina::endian_flag endianFlag, stream& stream)
{
    // Get current stream position.
    const std::size_t curPos = stream.tell();

    // Jump to header fileSize position.
    stream.jump_to(headerPos + offsetof(raw_header, fileSize));

    // Compute file size.
    u32 fileSize = static_cast<u32>(curPos - headerPos);

    // Swap file size and block count if necessary.
    if (needs_swap(endianFlag))
    {
        hl::endian_swap(fileSize);
        hl::endian_swap(blockCount);
    }

    // Fill-in file size and block count.
    stream.write_obj(fileSize);
    stream.write_obj(blockCount);
    
    // Jump back to end of stream.
    stream.jump_to(curPos);
}

template<template<typename> class off_t>
static void in_fix(void* rawData, std::size_t dataSize)
{
    // BINA V2
    if (has_v2_header(rawData))
    {
        const auto headerPtr = static_cast<raw_header*>(rawData);
        in_fix<off_t>(*headerPtr);
    }

    // PACPACK_METADATA
    else
    {
        // Compatibility with files extracted using PacPack.
        const auto pacPackMetadata = get_pac_pack_meta(rawData, dataSize);
        if (!pacPackMetadata) return;

        const auto endianFlag = pacPackMetadata->guess_endianness(
            rawData, dataSize);

        pacPackMetadata->fix(rawData, endianFlag);
    }
}

void fix32(void* rawData, std::size_t dataSize)
{
    in_fix<off32>(rawData, dataSize);
}

void fix64(void* rawData, std::size_t dataSize)
{
    in_fix<off64>(rawData, dataSize);
}

const raw_data_block_header* get_data_block(const void* rawData)
{
    // BINA V2
    if (has_v2_header(rawData))
    {
        const raw_header* headerPtr = static_cast<const raw_header*>(rawData);
        return headerPtr->get_data_block();
    }

    // PACPACK_METADATA
    else
    {
        return nullptr;
    }
}

void writer32::start(bina::endian_flag endianFlag, ver version)
{
    // Store header position and endian flag.
    m_headerPos = m_stream->tell();
    m_basePos = m_headerPos;
    m_endianFlag = endianFlag;
    m_blockCount = 0;

    // Start writing header.
    raw_header::start_write(version, endianFlag, *m_stream);
}

void writer32::start_data_block()
{
    assert(!m_isWritingDataBlock &&
        "You must call finish_data_block exactly once per start_data_block call");

    m_strings.clear();
    m_offsets.clear();

    raw_data_block_header::start_write(m_endianFlag, *m_stream);

    m_basePos = m_stream->tell();
    ++m_blockCount;
    m_isWritingDataBlock = true;
}

void writer32::add_string(std::string str, std::size_t offPos)
{
    m_strings.emplace_back(std::move(str), offPos);
}

std::size_t writer32::write_str(const char* str)
{
    // Write placeholder string offset.
    const auto strOffPos = m_stream->tell();
    m_stream->write_nulls(sizeof(off32<char>));

    // Add string to strings table, so it will be written later.
    std::string tmpStr(str);
    const auto strSize = (tmpStr.size() + 1);
    add_string(std::move(tmpStr), strOffPos);

    // Return the size of the string + the null terminator.
    return strSize;
}

std::size_t writer32::write_str(const std::string& str)
{
    // Write placeholder string offset.
    const auto strOffPos = m_stream->tell();

    // Add string to strings table, so it will be written later.
    m_stream->write_nulls(sizeof(off32<char>));
    add_string(str, strOffPos);

    // Return the size of the string + the null terminator.
    return (str.size() + 1);
}

void writer32::fix_offset(std::size_t pos)
{
    m_stream->fix_off32(m_basePos, pos, needs_swap(m_endianFlag), m_offsets);
}

void writer32::finish_data_block()
{
    assert(m_isWritingDataBlock &&
        "You must call finish_data_block exactly once per start_data_block call");

    raw_data_block_header::finish_write32(
        m_basePos - (sizeof(raw_data_block_header) * 2),
        m_endianFlag, m_strings, m_offsets, *m_stream);

    m_isWritingDataBlock = false;
}

void writer32::finish()
{
    if (m_isWritingDataBlock)
    {
        finish_data_block();
    }

    raw_header::finish_write(m_headerPos,
        m_blockCount, m_endianFlag, *m_stream);
}

writer32::writer32(hl::stream& stream) :
    in_writer_base(stream),
    m_headerPos(stream.tell()),
    m_basePos(m_headerPos) {}

std::size_t writer64::write_str(const char* str)
{
    const auto strOffPos = m_stream->tell();
    m_stream->write_nulls(sizeof(off64<char>));

    std::string tmpStr(str);
    const auto strSize = (tmpStr.size() + 1);
    add_string(std::move(tmpStr), strOffPos);

    return strSize;
}

std::size_t writer64::write_str(const std::string& str)
{
    const auto strOffPos = m_stream->tell();
    m_stream->write_nulls(sizeof(off64<char>));
    add_string(str, strOffPos);
    return (str.size() + 1);
}

void writer64::fix_offset(std::size_t pos)
{
    m_stream->fix_off64(m_basePos, pos, needs_swap(m_endianFlag), m_offsets);
}

void writer64::finish_data_block()
{
    assert(m_isWritingDataBlock &&
        "You must call finish_data_block exactly once per start_data_block call");

    raw_data_block_header::finish_write64(
        m_basePos - (sizeof(raw_data_block_header) * 2),
        m_endianFlag, m_strings, m_offsets, *m_stream);

    m_isWritingDataBlock = false;
}

void writer64::finish()
{
    if (m_isWritingDataBlock)
    {
        finish_data_block();
    }

    raw_header::finish_write(m_headerPos,
        m_blockCount, m_endianFlag, *m_stream);
}
} // v2

template<template<typename> class off_t>
static void in_fix(void* rawData, std::size_t dataSize)
{
    if (has_v1_header(rawData, dataSize))
    {
        v1::fix(rawData);
    }
    else
    {
        // NOTE: v2::in_fix also handles PACPACK_METADATA.
        v2::in_fix<off_t>(rawData, dataSize);
    }
}

void fix32(void* rawData, std::size_t dataSize)
{
    in_fix<off32>(rawData, dataSize);
}

void fix64(void* rawData, std::size_t dataSize)
{
    in_fix<off64>(rawData, dataSize);
}
} // bina
} // hl
