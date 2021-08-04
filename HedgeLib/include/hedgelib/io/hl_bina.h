#ifndef HL_BINA_H_INCLUDED
#define HL_BINA_H_INCLUDED
#include "hl_hh_mirage.h"
#include "../hl_blob.h"

/* GNU-specific compilation fix */
#ifdef major
#undef major
#endif

#ifdef minor
#undef minor
#endif

namespace hl
{
namespace bina
{
constexpr u32 sig = make_sig("BINA");

struct ver
{
    u8 major;
    u8 minor;
    u8 rev;

    constexpr ver(char major, char minor, char rev) noexcept :
        major(major), minor(minor), rev(rev) {}
};

HL_STATIC_ASSERT_SIZE(ver, 3);

enum class endian_flag : u8
{
    big = 'B',
    little = 'L'
};

/**
    @brief  Returns whether data in a BINA file with the given endian flag
            needs to be endian-swapped or not.

    @param flag     The endian flag to check.
    @return         Whether data in a BINA file with the given flag needs to be swapped.
*/
constexpr bool needs_swap(const endian_flag flag) noexcept
{
#ifdef HL_IS_BIG_ENDIAN
    return (flag == endian_flag::little);
#else
    return (flag == endian_flag::big);
#endif
}

bool has_v1_header(const void* rawData);
bool has_v2_header(const void* rawData);

struct pac_pack_meta
{
    u8 signature[16];
    u32 offCount;

    template<bool swapOffsets = true>
    inline void endian_swap() noexcept
    {
        hl::endian_swap(offCount);
    }

    inline const u32* off_table() const noexcept
    {
        return reinterpret_cast<const u32*>(this + 1);
    }

    inline u32* off_table() noexcept
    {
        return reinterpret_cast<u32*>(this + 1);
    }

    inline hh::mirage::const_off_table_handle offsets() const noexcept
    {
        return hh::mirage::const_off_table_handle(off_table(), offCount);
    }

    inline hh::mirage::off_table_handle offsets() noexcept
    {
        return hh::mirage::off_table_handle(off_table(), offCount);
    }

    inline const u8* str_table() const noexcept
    {
        return reinterpret_cast<const u8*>(off_table() + offCount);
    }

    inline u8* str_table() noexcept
    {
        return reinterpret_cast<u8*>(off_table() + offCount);
    }

    inline endian_flag guess_endianness(const void* rawData, std::size_t dataSize) const noexcept
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

    inline endian_flag guess_endianness(const blob& rawData) const noexcept
    {
        return guess_endianness(rawData.data(), rawData.size());
    }

    HL_API void fix(void* rawData, endian_flag endianFlag);
};

HL_STATIC_ASSERT_SIZE(pac_pack_meta, 20);

HL_API const pac_pack_meta* get_pac_pack_meta(const void* rawData, std::size_t dataSize);

inline pac_pack_meta* get_pac_pack_meta(void* rawData, std::size_t dataSize)
{
    return const_cast<pac_pack_meta*>(get_pac_pack_meta(
        const_cast<const void*>(rawData), dataSize));
}

inline const pac_pack_meta* get_pac_pack_meta(const blob& rawData)
{
    return get_pac_pack_meta(rawData.data(), rawData.size());
}

inline pac_pack_meta* get_pac_pack_meta(blob& rawData)
{
    return get_pac_pack_meta(rawData.data(), rawData.size());
}

template<template<typename> class off_t>
void strings_write(std::size_t dataPos,
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
        off_t<void> off(static_cast<typename off_t<void>::val_t>(
            curStrPos - dataPos));

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
    stream.pad(sizeof(off_t<void>));
}

inline void strings_write32(std::size_t dataPos,
    endian_flag endianFlag, const str_table& strTable,
    off_table& offTable, stream& stream)
{
    strings_write<off32>(dataPos, endianFlag,
        strTable, offTable, stream);
}

inline void strings_write64(std::size_t dataPos,
    endian_flag endianFlag, const str_table& strTable,
    off_table& offTable, stream& stream)
{
    strings_write<off64>(dataPos, endianFlag,
        strTable, offTable, stream);
}

enum class offset_flags : u8
{
    /* Masks */
    size_mask = 0xC0U,
    data_mask = 0x3FU,

    /* Sizes */
    size_six_bit = 0x40U,
    size_fourteen_bit = 0x80U,
    size_thirty_bit = 0xC0U
};

HL_ENUM_CLASS_DEF_BITWISE_OPS(offset_flags)

class off_table_handle
{
protected:
    const u8* m_offTableBeg;
    const u8* m_offTableEnd;

    HL_API static const u8* in_get_real_off_table_end(
        const u8* offTable, u32 offTableSize) noexcept;

public:
    class iterator
    {
        const u8* m_curOffTablePtr = nullptr;
        u32 m_curRelOffPos = 0;

        HL_API u32 in_get_cur_rel_off_pos() const noexcept;
        HL_API const u8* in_get_next_off_table_ptr() const noexcept;

    public:
        HL_API bool next() noexcept;

        inline iterator& operator++() noexcept
        {
            next();
            return *this;
        }

        inline iterator operator++(int) noexcept
        {
            iterator tmpCopy(*this);
            operator++();
            return tmpCopy;
        }

        inline u32 operator*() const noexcept
        {
            return m_curRelOffPos;
        }

        inline bool operator==(const iterator& other) const noexcept
        {
            return (m_curOffTablePtr == other.m_curOffTablePtr);
        }

        inline bool operator!=(const iterator& other) const noexcept
        {
            return (m_curOffTablePtr != other.m_curOffTablePtr);
        }

        inline iterator() noexcept = default;

        inline iterator(const u8* offTable) noexcept :
            m_curOffTablePtr(offTable),
            m_curRelOffPos(in_get_cur_rel_off_pos()) {}
    };

    inline iterator begin() const noexcept
    {
        return iterator(m_offTableBeg);
    }

    inline iterator end() const noexcept
    {
        return iterator(m_offTableEnd);
    }

    inline off_table_handle(const u8* offTable, u32 offTableSize) noexcept :
        m_offTableBeg(offTable), m_offTableEnd(in_get_real_off_table_end(
        offTable, offTableSize)) {}
};

template<template<typename> class off_t>
void offsets_fix(off_table_handle offTable,
    const endian_flag endianFlag, void* base)
{
    for (u32 relOffPos : offTable)
    {
        // Get pointer to current offset.
        off_t<void>* curOff = ptradd<off_t<void>>(base, relOffPos);

        // Endian-swap offset if necessary.
        if (needs_swap(endianFlag))
        {
            hl::endian_swap(*curOff);
        }

        // Fix offset.
        curOff->fix(base);
    }
}

inline void offsets_fix32(off_table_handle offTable,
    const endian_flag endianFlag, void* base)
{
    offsets_fix<off32>(offTable, endianFlag, base);
}

inline void offsets_fix64(off_table_handle offTable,
    const endian_flag endianFlag, void* base)
{
    offsets_fix<off64>(offTable, endianFlag, base);
}

HL_API void offsets_write_no_sort_no_pad(std::size_t dataPos,
    const off_table& offTable, stream& stream);

template<template<typename> class off_t>
inline void offsets_write_no_sort(std::size_t dataPos,
    const off_table& offTable, stream& stream)
{
    // Write offsets.
    offsets_write_no_sort_no_pad(dataPos, offTable, stream);

    // Write padding.
    stream.pad(sizeof(off_t<void>));
}

inline void offsets_write_no_sort32(std::size_t dataPos,
    const off_table& offTable, stream& stream)
{
    offsets_write_no_sort<off32>(dataPos, offTable, stream);
}

inline void offsets_write_no_sort64(std::size_t dataPos,
    const off_table& offTable, stream& stream)
{
    offsets_write_no_sort<off64>(dataPos, offTable, stream);
}

template<template<typename> class off_t>
inline void offsets_write(std::size_t dataPos,
    off_table& offTable, stream& stream)
{
    // Sort offset table.
    std::sort(offTable.begin(), offTable.end());

    // Write sorted offsets.
    offsets_write_no_sort<off_t>(dataPos, offTable, stream);
}

inline void offsets_write32(std::size_t dataPos,
    off_table& offTable, stream& stream)
{
    offsets_write<off32>(dataPos, offTable, stream);
}

inline void offsets_write64(std::size_t dataPos,
    off_table& offTable, stream& stream)
{
    offsets_write<off64>(dataPos, offTable, stream);
}

namespace v1
{
struct header
{
    /** @brief The size of the entire file, including this header. */
    u32 fileSize;
    /** @brief The non-absolute offset to the offset table. */
    off32<u8> offTable;
    /** @brief The size of the offset table. */
    u32 offTableSize;
    /** @brief Seems to just be padding. */
    u32 unknown1;
    u16 unknownFlag1;
    /** @brief IsFooterMagicPresent? */
    u16 unknownFlag2;
    u16 unknown2;
    /** @brief Major version number. */
    u8 version;
    /** @brief 'B' for Big Endian, 'L' for Little Endian. See hl::bina::endian_flag. */
    u8 endianFlag;
    /** @brief "BINA" signature. */
    u32 signature;
    /** @brief Included so garbage data doesn't get writtten. */
    u32 padding;

    template<bool swapOffsets = true>
    inline void endian_swap() noexcept
    {
        hl::endian_swap(fileSize);
        hl::endian_swap<swapOffsets>(offTable);
        hl::endian_swap(offTableSize);
        hl::endian_swap(unknown1);
        hl::endian_swap(unknownFlag1);
        hl::endian_swap(unknownFlag2);
        hl::endian_swap(unknown2);
    }

    inline bina::endian_flag endian_flag() const noexcept
    {
        return static_cast<bina::endian_flag>(endianFlag);
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

    inline off_table_handle offsets() const noexcept
    {
        return off_table_handle(offTable.get(), offTableSize);
    }

    HL_API void fix();
};

HL_STATIC_ASSERT_SIZE(header, 0x20);

inline void fix(void* rawData)
{
    header* headerPtr = static_cast<header*>(rawData);
    headerPtr->fix();
}

template<typename T = void>
inline const T* get_data(const void* rawData)
{
    const header* headerPtr = static_cast<const header*>(rawData);
    return headerPtr->data<T>();
}

template<typename T = void>
inline T* get_data(void* rawData)
{
    header* headerPtr = static_cast<header*>(rawData);
    return headerPtr->data<T>();
}

// TODO
} // v1

namespace v2
{
constexpr ver ver_200 = ver('2', '0', '0');
constexpr ver ver_210 = ver('2', '1', '0');

enum class block_type : u32
{
    /** @brief This block contains data. See block_data_header. */
    data = make_sig("DATA"),

    /**
       @brief Unknown block type. It's never used to our knowledge;
       we only know about it because Lost World Wii U checks for it.
    */
    imag = make_sig("IMAG")
};

struct block_header
{
    /** @brief Used to determine what type of block this is. See hl::bina::v2::block_type. */
    u32 signature;
    /** @brief The complete size of the block, including this header. */
    u32 size;

    template<bool swapOffsets = true>
    inline void endian_swap() noexcept
    {
        hl::endian_swap(size);
    }

    inline const block_header* next_block() const noexcept
    {
        return ptradd<block_header>(this, size);
    }

    inline block_header* next_block() noexcept
    {
        return ptradd<block_header>(this, size);
    }
};

HL_STATIC_ASSERT_SIZE(block_header, 8);

struct block_data_header
{
    /** @brief Used to determine what type of block this is. See hl::bina::v2::block_type. */
    u32 signature;
    /** @brief The complete size of the block, including this header. */
    u32 size;
    /** @brief Offset to the beginning of the string table. */
    off32<char> strTable;
    /** @brief The size of the string table in bytes, including padding. */
    u32 strTableSize;
    /** @brief The size of the offset table in bytes, including padding. */
    u32 offTableSize;
    /** @brief The offset to the data relative to the end of this struct. */
    u16 relativeDataOffset;
    /** @brief Included so garbage data doesn't get writtten. */
    u16 padding;

    template<bool swapOffsets = true>
    inline void endian_swap() noexcept
    {
        hl::endian_swap(size);
        hl::endian_swap<swapOffsets>(strTable);
        hl::endian_swap(strTableSize);
        hl::endian_swap(offTableSize);
        hl::endian_swap(relativeDataOffset);
    }

    inline const block_header* next_block() const noexcept
    {
        return ptradd<block_header>(this, size);
    }

    inline block_header* next_block() noexcept
    {
        return ptradd<block_header>(this, size);
    }

    template<typename T = void>
    inline const T* data() const noexcept
    {
        return ptradd<T>(this + 1, relativeDataOffset);
    }

    template<typename T = void>
    inline T* data() noexcept
    {
        return ptradd<T>(this + 1, relativeDataOffset);
    }

    // TODO: Add string iterators.

    inline const u8* off_table() const noexcept
    {
        return ptradd(strTable.get(), strTableSize);
    }

    inline u8* off_table() noexcept
    {
        return ptradd(strTable.get(), strTableSize);
    }

    inline off_table_handle offsets() const noexcept
    {
        return off_table_handle(off_table(), offTableSize);
    }

    template<template<typename> class off_t>
    inline void fix(endian_flag endianFlag)
    {
        // Swap header if necessary.
        if (needs_swap(endianFlag))
        {
            endian_swap();
        }

        // Fix string table offset.
        void* dataPtr = data();
        strTable.fix(dataPtr);

        // Fix data offsets.
        offsets_fix<off_t>(offsets(), endianFlag, dataPtr);
    }

    inline void fix32(endian_flag endianFlag)
    {
        fix<off32>(endianFlag);
    }

    inline void fix64(endian_flag endianFlag)
    {
        fix<off64>(endianFlag);
    }

    HL_API static void start_write(endian_flag endianFlag,
        stream& stream);

    HL_API static void finish_write(std::size_t dataBlockPos,
        std::size_t strTablePos, std::size_t offTablePos,
        endian_flag endianFlag, stream& stream);

    template<template<typename> class off_t>
    inline static void finish_write(std::size_t dataBlockPos,
        endian_flag endianFlag, const hl::str_table& strTable,
        hl::off_table& offTable, stream& stream)
    {
        const std::size_t dataPos = (dataBlockPos + (sizeof(block_data_header) * 2));

        // Write string table.
        const std::size_t strTablePos = stream.tell();
        strings_write<off_t>(dataPos, endianFlag, strTable, offTable, stream);

        // Write offset table.
        const std::size_t offTablePos = stream.tell();
        offsets_write<off_t>(dataPos, offTable, stream);

        // Fill-in data block header values.
        finish_write(dataBlockPos, strTablePos, offTablePos, endianFlag, stream);
    }

    inline static void finish_write32(std::size_t dataBlockPos,
        endian_flag endianFlag, const hl::str_table& strTable,
        hl::off_table& offTable, stream& stream)
    {
        finish_write<off32>(dataBlockPos, endianFlag,
            strTable, offTable, stream);
    }

    inline static void finish_write64(std::size_t dataBlockPos,
        endian_flag endianFlag, const hl::str_table& strTable,
        hl::off_table& offTable, stream& stream)
    {
        finish_write<off64>(dataBlockPos, endianFlag,
            strTable, offTable, stream);
    }
};

HL_STATIC_ASSERT_SIZE(block_data_header, 0x18);

class const_block_iterator
{
    const block_header* m_curBlock = nullptr;
    u16 m_curBlockIndex = 0, m_blockCount = 0;

public:
    inline const_block_iterator& begin() noexcept
    {
        return *this;
    }

    inline const_block_iterator end() const noexcept
    {
        return const_block_iterator();
    }

    inline const block_header* operator*() const noexcept
    {
        return m_curBlock;
    }

    inline const_block_iterator& operator++() noexcept
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

    inline bool operator==(const const_block_iterator& other) const noexcept
    {
        return (m_curBlock == other.m_curBlock &&
            m_curBlockIndex == other.m_curBlockIndex &&
            m_blockCount == other.m_blockCount);
    }

    inline bool operator!=(const const_block_iterator& other) const noexcept
    {
        return (m_curBlock != other.m_curBlock ||
            m_curBlockIndex != other.m_curBlockIndex ||
            m_blockCount != other.m_blockCount);
    }

    inline const_block_iterator() noexcept = default;
    inline const_block_iterator(const block_header* firstBlock,
        u16 blockCount) noexcept : m_curBlock(firstBlock),
        m_blockCount(blockCount) {}
};

class block_iterator
{
    block_header* m_curBlock = nullptr;
    u16 m_curBlockIndex = 0, m_blockCount = 0;

public:
    inline block_iterator& begin() noexcept
    {
        return *this;
    }

    inline block_iterator end() const noexcept
    {
        return block_iterator();
    }

    inline const block_header* operator*() const noexcept
    {
        return m_curBlock;
    }

    inline block_header* operator*() noexcept
    {
        return m_curBlock;
    }

    inline block_iterator& operator++() noexcept
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

    inline bool operator==(const block_iterator& other) const noexcept
    {
        return (m_curBlock == other.m_curBlock &&
            m_curBlockIndex == other.m_curBlockIndex &&
            m_blockCount == other.m_blockCount);
    }

    inline bool operator!=(const block_iterator& other) const noexcept
    {
        return (m_curBlock != other.m_curBlock ||
            m_curBlockIndex != other.m_curBlockIndex ||
            m_blockCount != other.m_blockCount);
    }

    inline block_iterator() noexcept = default;
    inline block_iterator(block_header* firstBlock, u16 blockCount) noexcept :
        m_curBlock((blockCount) ? firstBlock : nullptr),
        m_blockCount(blockCount) {}
};

struct header
{
    /** @brief "BINA" signature. */
    u32 signature;
    /** @brief Version number. */
    ver version;
    /** @brief 'B' for Big Endian, 'L' for Little Endian. See hl::bina::endian_flag. */
    u8 endianFlag;
    /** @brief The size of the entire file, including this header. */
    u32 fileSize;
    /** @brief How many blocks are in the file. */
    u16 blockCount;
    /** @brief Included so garbage data doesn't get writtten. */
    u16 padding;

    template<bool swapOffsets = true>
    inline void endian_swap() noexcept
    {
        hl::endian_swap(fileSize);
        hl::endian_swap(blockCount);
    }

    inline bina::endian_flag endian_flag() const noexcept
    {
        return static_cast<bina::endian_flag>(endianFlag);
    }

    inline const block_header* first_block() const noexcept
    {
        return reinterpret_cast<const block_header*>(this + 1);
    }

    inline block_header* first_block() noexcept
    {
        return reinterpret_cast<block_header*>(this + 1);
    }

    inline const_block_iterator blocks() const noexcept
    {
        return const_block_iterator(first_block(), blockCount);
    }

    inline block_iterator blocks() noexcept
    {
        return block_iterator(first_block(), blockCount);
    }

    HL_API const block_header* get_block(block_type type) const noexcept;

    inline block_header* get_block(block_type type) noexcept
    {
        return const_cast<block_header*>(const_cast<
            const header*>(this)->get_block(type));
    }

    inline const block_data_header* get_data_block() const noexcept
    {
        return reinterpret_cast<const block_data_header*>(
            get_block(block_type::data));
    }

    inline block_data_header* get_data_block() noexcept
    {
        return reinterpret_cast<block_data_header*>(
            get_block(block_type::data));
    }

    template<typename T = void>
    inline const T* get_data() const noexcept
    {
        // Get data block, if any.
        const block_data_header* dataBlock = get_data_block();
        if (!dataBlock) return nullptr;

        // Get data.
        return dataBlock->data<T>();
    }

    template<typename T = void>
    inline T* get_data() noexcept
    {
        // Get data block, if any.
        block_data_header* dataBlock = get_data_block();
        if (!dataBlock) return nullptr;

        // Get data.
        return dataBlock->data<T>();
    }

    template<template<typename> class off_t>
    inline void fix()
    {
        // Swap header if necessary.
        if (needs_swap(endian_flag()))
        {
            endian_swap();
        }

        // Fix blocks.
        for (auto block : blocks())
        {
            switch (block->signature)
            {
            case static_cast<u32>(block_type::data):
            {
                block_data_header* dataBlock = reinterpret_cast<block_data_header*>(block);
                dataBlock->fix<off_t>(endian_flag());
                break;
            }

            default:
                HL_ERROR(error_type::unsupported);
                return;
            }
        }
    }

    inline void fix32()
    {
        fix<off32>();
    }

    inline void fix64()
    {
        fix<off64>();
    }

    HL_API static void start_write(ver version,
        bina::endian_flag endianFlag, stream& stream);

    HL_API static void finish_write(std::size_t headerPos, u16 blockCount,
        bina::endian_flag endianFlag, stream& stream);
};

HL_STATIC_ASSERT_SIZE(header, 16);

template<template<typename> class off_t>
inline void fix(void* rawData, std::size_t dataSize)
{
    // BINA V2
    if (has_v2_header(rawData))
    {
        header* headerPtr = static_cast<header*>(rawData);
        headerPtr->fix<off_t>();
    }

    // PACPACK_METADATA
    else
    {
        // Compatibility with files extracted using PacPack.
        pac_pack_meta* pacPackMetadata = get_pac_pack_meta(rawData, dataSize);
        if (!pacPackMetadata) return;

        const endian_flag endianFlag = pacPackMetadata->guess_endianness(rawData, dataSize);

        pacPackMetadata->fix(rawData, endianFlag);
    }
}

template<template<typename> class off_t>
inline void fix(blob& rawData)
{
    fix<off_t>(rawData.data(), rawData.size());
}

inline void fix32(void* rawData, std::size_t dataSize)
{
    fix<off32>(rawData, dataSize);
}

inline void fix32(blob& rawData)
{
    fix<off32>(rawData);
}

inline void fix64(void* rawData, std::size_t dataSize)
{
    fix<off64>(rawData, dataSize);
}

inline void fix64(blob& rawData)
{
    fix<off64>(rawData);
}

HL_API const block_data_header* get_data_block(const void* rawData);

inline block_data_header* get_data_block(void* rawData)
{
    return const_cast<block_data_header*>(get_data_block(
        const_cast<const void*>(rawData)));
}

template<typename T = void>
inline const T* get_data(const void* rawData)
{
    if (has_v2_header(rawData))
    {
        const header* headerPtr = static_cast<const header*>(rawData);
        return headerPtr->get_data<T>();
    }
    else
    {
        return static_cast<const T*>(rawData);
    }
}

template<typename T = void>
inline T* get_data(void* rawData)
{
    // BINA V2
    if (has_v2_header(rawData))
    {
        header* headerPtr = static_cast<header*>(rawData);
        return headerPtr->get_data<T>();
    }

    // PACPACK_METADATA
    else
    {
        return static_cast<T*>(rawData);
    }
}
} // v2

inline bool has_v1_header(const void* rawData)
{
    const v1::header* header = static_cast<const v1::header*>(rawData);
    return (header->signature == sig);
}

inline bool has_v1_header(const void* rawData, std::size_t dataSize)
{
    return (dataSize >= sizeof(v1::header) && has_v1_header(rawData));
}

inline bool has_v1_header(const blob& rawData)
{
    return has_v1_header(rawData.data(), rawData.size());
}

inline bool has_v2_header(const void* rawData)
{
    const v2::header* header = static_cast<const v2::header*>(rawData);
    return (header->signature == sig);
}

inline bool has_v2_header(const void* rawData, std::size_t dataSize)
{
    return (dataSize >= sizeof(v2::header) && has_v2_header(rawData));
}

inline bool has_v2_header(const blob& rawData)
{
    return has_v2_header(rawData.data(), rawData.size());
}

template<template<typename> class off_t>
inline void fix(void* rawData, std::size_t dataSize)
{
    if (has_v1_header(rawData, dataSize))
    {
        v1::fix(rawData);
    }
    else
    {
        // NOTE: v2::fix also handles PACPACK_METADATA.
        v2::fix<off_t>(rawData, dataSize);
    }
}

template<template<typename> class off_t>
inline void fix(blob& rawData)
{
    fix<off_t>(rawData.data(), rawData.size());
}

inline void fix32(void* rawData, std::size_t dataSize)
{
    fix<off32>(rawData, dataSize);
}

inline void fix32(blob& rawData)
{
    fix<off32>(rawData);
}

inline void fix64(void* rawData, std::size_t dataSize)
{
    fix<off64>(rawData, dataSize);
}

inline void fix64(blob& rawData)
{
    fix<off64>(rawData);
}

template<typename T = void>
inline const T* get_data(const void* rawData)
{
    if (has_v2_header(rawData))
    {
        return v2::get_data<T>(rawData);
    }
    else if (has_v1_header(rawData))
    {
        return v1::get_data<T>(rawData);
    }
    else
    {
        return static_cast<const T*>(rawData);
    }
}

template<typename T = void>
inline T* get_data(void* rawData)
{
    return const_cast<T*>(get_data<T>(
        const_cast<const void*>(rawData)));
}
} // bina
} // hl
#endif
