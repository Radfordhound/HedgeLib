#ifndef HL_BINA_H_INCLUDED
#define HL_BINA_H_INCLUDED
#include "hl_hh_mirage.h"
#include "../hl_blob.h"

namespace hl
{
namespace bina
{
constexpr u32 sig = make_sig("BINA");

struct ver
{
    /* NOTE: major and minor begin with underscores to avoid a GNU-specific compilation error. */
    u8 _major;
    u8 _minor;
    u8 rev;

    constexpr ver(char _major, char _minor, char rev) noexcept :
        _major(_major),
        _minor(_minor),
        rev(rev) {}
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

HL_API void strings_write32(std::size_t dataPos,
    endian_flag endianFlag, const str_table& strTable,
    off_table& offTable, stream& stream);

HL_API void strings_write64(std::size_t dataPos,
    endian_flag endianFlag, const str_table& strTable,
    off_table& offTable, stream& stream);

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

HL_API void offsets_fix32(off_table_handle offTable,
    const endian_flag endianFlag, void* base);

HL_API void offsets_fix64(off_table_handle offTable,
    const endian_flag endianFlag, void* base);

HL_API void offsets_write_no_sort_no_pad(std::size_t dataPos,
    const off_table& offTable, stream& stream);

HL_API void offsets_write_no_sort32(std::size_t dataPos,
    const off_table& offTable, stream& stream);

HL_API void offsets_write_no_sort64(std::size_t dataPos,
    const off_table& offTable, stream& stream);

HL_API void offsets_write32(std::size_t dataPos,
    off_table& offTable, stream& stream);

HL_API void offsets_write64(std::size_t dataPos,
    off_table& offTable, stream& stream);

namespace v1
{
struct raw_header
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

    HL_API off_table_handle offsets() const noexcept;

    HL_API void fix();
};

HL_STATIC_ASSERT_SIZE(raw_header, 0x20);

inline void fix(void* rawData)
{
    raw_header* headerPtr = static_cast<raw_header*>(rawData);
    headerPtr->fix();
}

template<typename T = void>
inline const T* get_data(const void* rawData)
{
    const raw_header* headerPtr = static_cast<const raw_header*>(rawData);
    return headerPtr->data<T>();
}

template<typename T = void>
inline T* get_data(void* rawData)
{
    raw_header* headerPtr = static_cast<raw_header*>(rawData);
    return headerPtr->data<T>();
}
} // v1

namespace v2
{
constexpr ver ver_200 = ver('2', '0', '0');
constexpr ver ver_210 = ver('2', '1', '0');

enum class raw_block_type : u32
{
    /** @brief This block contains data. See block_data_header. */
    data = make_sig("DATA"),

    /**
       @brief Unknown block type. It's never used to our knowledge;
       we only know about it because Lost World Wii U checks for it.
    */
    imag = make_sig("IMAG")
};

struct raw_block_header
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

    inline const raw_block_header* next_block() const noexcept
    {
        return ptradd<raw_block_header>(this, size);
    }

    inline raw_block_header* next_block() noexcept
    {
        return ptradd<raw_block_header>(this, size);
    }
};

HL_STATIC_ASSERT_SIZE(raw_block_header, 8);

struct raw_block_data_header
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

    inline const raw_block_header* next_block() const noexcept
    {
        return ptradd<raw_block_header>(this, size);
    }

    inline raw_block_header* next_block() noexcept
    {
        return ptradd<raw_block_header>(this, size);
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

    HL_API void fix32(endian_flag endianFlag);
    
    HL_API void fix64(endian_flag endianFlag);

    HL_API static void start_write(endian_flag endianFlag,
        stream& stream);

    HL_API static void finish_write(std::size_t dataBlockPos,
        std::size_t strTablePos, std::size_t offTablePos,
        endian_flag endianFlag, stream& stream);

    HL_API static void finish_write32(std::size_t dataBlockPos,
        endian_flag endianFlag, const hl::str_table& strTable,
        hl::off_table& offTable, stream& stream);

    HL_API static void finish_write64(std::size_t dataBlockPos,
        endian_flag endianFlag, const hl::str_table& strTable,
        hl::off_table& offTable, stream& stream);
};

HL_STATIC_ASSERT_SIZE(raw_block_data_header, 0x18);

class const_block_iterator
{
    const raw_block_header* m_curBlock = nullptr;
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

    inline const raw_block_header* operator*() const noexcept
    {
        return m_curBlock;
    }

    HL_API const_block_iterator& operator++() noexcept;

    HL_API bool operator==(const const_block_iterator& other) const noexcept;

    HL_API bool operator!=(const const_block_iterator& other) const noexcept;

    inline const_block_iterator() noexcept = default;

    inline const_block_iterator(const raw_block_header* firstBlock, u16 blockCount) noexcept :
        m_curBlock((blockCount) ? firstBlock : nullptr),
        m_blockCount(blockCount) {}
};

class block_iterator
{
    raw_block_header* m_curBlock = nullptr;
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

    inline const raw_block_header* operator*() const noexcept
    {
        return m_curBlock;
    }

    inline raw_block_header* operator*() noexcept
    {
        return m_curBlock;
    }

    HL_API block_iterator& operator++() noexcept;

    HL_API bool operator==(const block_iterator& other) const noexcept;

    HL_API bool operator!=(const block_iterator& other) const noexcept;

    inline block_iterator() noexcept = default;
    inline block_iterator(raw_block_header* firstBlock, u16 blockCount) noexcept :
        m_curBlock((blockCount) ? firstBlock : nullptr),
        m_blockCount(blockCount) {}
};

struct raw_header
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

    inline const raw_block_header* first_block() const noexcept
    {
        return reinterpret_cast<const raw_block_header*>(this + 1);
    }

    inline raw_block_header* first_block() noexcept
    {
        return reinterpret_cast<raw_block_header*>(this + 1);
    }

    inline const_block_iterator blocks() const noexcept
    {
        return const_block_iterator(first_block(), blockCount);
    }

    inline block_iterator blocks() noexcept
    {
        return block_iterator(first_block(), blockCount);
    }

    HL_API const raw_block_header* get_block(raw_block_type type) const noexcept;

    inline raw_block_header* get_block(raw_block_type type) noexcept
    {
        return const_cast<raw_block_header*>(const_cast<
            const raw_header*>(this)->get_block(type));
    }

    inline const raw_block_data_header* get_data_block() const noexcept
    {
        return reinterpret_cast<const raw_block_data_header*>(
            get_block(raw_block_type::data));
    }

    inline raw_block_data_header* get_data_block() noexcept
    {
        return reinterpret_cast<raw_block_data_header*>(
            get_block(raw_block_type::data));
    }

    template<typename T = void>
    inline const T* get_data() const noexcept
    {
        // Get data block, if any.
        const raw_block_data_header* dataBlock = get_data_block();
        if (!dataBlock) return nullptr;

        // Get data.
        return dataBlock->data<T>();
    }

    template<typename T = void>
    inline T* get_data() noexcept
    {
        // Get data block, if any.
        raw_block_data_header* dataBlock = get_data_block();
        if (!dataBlock) return nullptr;

        // Get data.
        return dataBlock->data<T>();
    }

    HL_API void fix32();

    HL_API void fix64();

    HL_API static void start_write(ver version,
        bina::endian_flag endianFlag, stream& stream);

    HL_API static void finish_write(std::size_t headerPos, u16 blockCount,
        bina::endian_flag endianFlag, stream& stream);
};

HL_STATIC_ASSERT_SIZE(raw_header, 16);

HL_API void fix32(void* rawData, std::size_t dataSize);

inline void fix32(blob& rawData)
{
    fix32(rawData.data(), rawData.size());
}

HL_API void fix64(void* rawData, std::size_t dataSize);

inline void fix64(blob& rawData)
{
    fix64(rawData.data(), rawData.size());
}

HL_API const raw_block_data_header* get_data_block(const void* rawData);

inline raw_block_data_header* get_data_block(void* rawData)
{
    return const_cast<raw_block_data_header*>(
        get_data_block(const_cast<const void*>(rawData)));
}

template<typename T = void>
inline const T* get_data(const void* rawData)
{
    if (has_v2_header(rawData))
    {
        const auto headerPtr = static_cast<const raw_header*>(rawData);
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
        const auto headerPtr = static_cast<raw_header*>(rawData);
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
    const auto header = static_cast<const v1::raw_header*>(rawData);
    return (header->signature == sig);
}

inline bool has_v1_header(const void* rawData, std::size_t dataSize)
{
    return (dataSize >= sizeof(v1::raw_header) && has_v1_header(rawData));
}

inline bool has_v1_header(const blob& rawData)
{
    return has_v1_header(rawData.data(), rawData.size());
}

inline bool has_v2_header(const void* rawData)
{
    const auto header = static_cast<const v2::raw_header*>(rawData);
    return (header->signature == sig);
}

inline bool has_v2_header(const void* rawData, std::size_t dataSize)
{
    return (dataSize >= sizeof(v2::raw_header) && has_v2_header(rawData));
}

inline bool has_v2_header(const blob& rawData)
{
    return has_v2_header(rawData.data(), rawData.size());
}

HL_API void fix32(void* rawData, std::size_t dataSize);

inline void fix32(blob& rawData)
{
    fix32(rawData.data(), rawData.size());
}

HL_API void fix64(void* rawData, std::size_t dataSize);

inline void fix64(blob& rawData)
{
    fix64(rawData.data(), rawData.size());
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
