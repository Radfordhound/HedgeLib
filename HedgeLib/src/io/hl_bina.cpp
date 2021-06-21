#include "hedgelib/io/hl_bina.h"
#include <cstring>

namespace hl
{
namespace bina
{
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

void offsets_write_no_sort_no_pad(std::size_t dataPos,
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
            HL_ERROR(error_type::out_of_range);
        }

        // Write BINA relative offset position to file.
        stream.write_all(binaRelOffPosSize, binaRelOffPos);
        
        // Set last offset position for next iteration.
        lastOffPos = curOffPos;
    }
}

namespace v1
{
void header::fix()
{
    // Endian-swap header if necessary.
    if (needs_swap(endian_flag()))
    {
        endian_swap();
    }

    // Fix offset table offset.
    void* dataPtr = data();
    offTable.fix(dataPtr);

    // Fix offsets.
    offsets_fix32(offsets(), endian_flag(), dataPtr);
}
} // v1

namespace v2
{
void block_data_header::start_write(endian_flag endianFlag,
    stream& stream)
{
    // Generate data block header.
    block_data_header dataBlock =
    {
        static_cast<u32>(block_type::data), // signature
        0U,                                 // size
        0U,                                 // strTable
        0U,                                 // strTableSize
        0U,                                 // offTableSize
        sizeof(block_data_header),          // relativeDataOffset
        0U                                  // padding
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

void block_data_header::finish_write(std::size_t dataBlockPos,
    std::size_t strTablePos, std::size_t offTablePos,
    endian_flag endianFlag, stream& stream)
{
    // Get end of stream position.
    const std::size_t endPos = stream.tell();

    // Jump to data block size position.
    stream.jump_to(dataBlockPos + offsetof(block_data_header, size));

    // Compute data block header values.
    const std::size_t dataPos = (dataBlockPos + (sizeof(block_data_header) * 2));
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

const block_header* header::get_block(block_type type) const noexcept
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

void header::start_write(ver version,
    bina::endian_flag endianFlag, stream& stream)
{
    // Generate BINAV2 header.
    const header binaHeader =
    {
        sig,                            // signature
        version,                        // version
        static_cast<u8>(endianFlag),    // endianFlag
        0,                              // fileSize
        0,                              // blockCount
        0                               // padding
    };

    // NOTE: We don't need to swap the header yet since the only values
    // that ever need to be swapped are going to be filled-in later.

    // Write BINAV2 header.
    stream.write_obj(binaHeader);
}

void header::finish_write(std::size_t headerPos, u16 blockCount,
    bina::endian_flag endianFlag, stream& stream)
{
    // Get current stream position.
    const std::size_t curPos = stream.tell();

    // Jump to header fileSize position.
    stream.jump_to(headerPos + offsetof(header, fileSize));

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

const block_data_header* get_data_block(const void* rawData)
{
    // BINA V2
    if (has_v2_header(rawData))
    {
        const header* headerPtr = static_cast<const header*>(rawData);
        return headerPtr->get_data_block();
    }

    // PACPACK_METADATA
    else
    {
        return nullptr;
    }
}
} // v2
} // bina
} // hl
