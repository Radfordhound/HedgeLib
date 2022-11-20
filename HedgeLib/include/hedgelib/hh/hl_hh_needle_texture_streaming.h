#ifndef HL_HH_NEEDLE_TEXTURE_STREAMING_INCLUDED
#define HL_HH_NEEDLE_TEXTURE_STREAMING_INCLUDED

#include "../hl_blob.h"
#include "../hl_text.h"
#include "../io/hl_stream.h"

namespace hl
{
namespace hh
{
namespace needle
{
namespace texture_streaming
{
constexpr u32 signature_package = make_sig("PSTN");
constexpr u32 signature_info = make_sig("NTSI");
constexpr u32 signature_dx10 = make_sig("DX10");

constexpr u32 version_package = 1u;
constexpr u32 version_info = 1u;

constexpr u32 dds_header_format_off = 0x54;
constexpr u32 dds_header_default_size = 0x80;
constexpr u32 dds_header_dx10_size = 0x94;
constexpr u32 dds_header_max_size = dds_header_dx10_size;

struct raw_package_blob
{
    u64 dataOffset;
    u64 dataSize;

    template<bool swapOffsets = true>
    inline void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(dataOffset);
        hl::endian_swap<swapOffsets>(dataSize);
    }
};

struct raw_package_entry
{
    u32 nameHash;
    u32 blobIndex;
    u32 blobCount;
    u16 width;
    u16 height;
    off64<char> name;

    template<bool swapOffsets = true>
    inline void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(nameHash);
        hl::endian_swap<swapOffsets>(blobIndex);
        hl::endian_swap<swapOffsets>(blobCount);
        hl::endian_swap<swapOffsets>(width);
        hl::endian_swap<swapOffsets>(height);
        hl::endian_swap<swapOffsets>(name);
    }

    void fix(void* base) noexcept;
};

inline u32 compute_name_hash(const char* str, const char* strEnd = nullptr) noexcept
{
    u32 hash = 0;

    char c;
    while (str != strEnd && (c = *str++))
        hash = hash * 31 + c;

    return hash & 0x7FFFFFFF;
}

struct raw_package
{
    u32 signature;
    u32 version;
    u32 entryCount;
    u32 blobCount;
    u64 headerSize;

    template<bool swapOffsets = true>
    inline void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(signature);
        hl::endian_swap<swapOffsets>(version);
        hl::endian_swap<swapOffsets>(entryCount);
        hl::endian_swap<swapOffsets>(blobCount);
        hl::endian_swap<swapOffsets>(headerSize);
    }

    inline const raw_package_entry* entries() const noexcept
    {
        return hl::ptradd<const raw_package_entry>(this, sizeof(raw_package));
    }   
    
    inline raw_package_entry* entries() noexcept
    {
        return hl::ptradd<raw_package_entry>(this, sizeof(raw_package));
    }

    inline const raw_package_blob* blobs() const noexcept
    {
        return reinterpret_cast<const raw_package_blob*>(entries() + entryCount);
    }   
    
    inline raw_package_blob* blobs() noexcept
    {
        return reinterpret_cast<raw_package_blob*>(entries() + entryCount);
    }

    void fix() noexcept;
};

class readonly_package_wrapper
{
protected:
    raw_package* m_package;
    hl::stream* m_stream;

public:
    readonly_package_wrapper()
        : m_package(nullptr), m_stream(nullptr)
    {
    }

    ~readonly_package_wrapper();

    inline bool valid() const noexcept
    {
        return m_package != nullptr;
    }

    inline const raw_package* get() const noexcept
    {
        return m_package;
    }

    bool try_load(hl::stream* stream);

    const raw_package_entry* get_entry(const char* str) const;
    hl::blob load_blob(u32 index) const;
};

struct raw_info
{
    u32 signature;
    u32 version;
    u32 unknown0;
    u32 packageNameSize;
    u32 mip4x4Size;
    u32 mip4x4Index;

    template<bool swapOffsets = true>
    inline void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(signature);
        hl::endian_swap<swapOffsets>(version);
        hl::endian_swap<swapOffsets>(unknown0);
        hl::endian_swap<swapOffsets>(packageNameSize);
        hl::endian_swap<swapOffsets>(mip4x4Size);
        hl::endian_swap<swapOffsets>(mip4x4Index);
    }

    inline const char* package_name() const noexcept
    {
        return hl::ptradd<const char>(this, sizeof(raw_info));
    }   
    
    inline char* package_name() noexcept
    {
        return hl::ptradd<char>(this, sizeof(raw_info));
    }

    inline const u8* mip_4x4() const noexcept
    {
        return reinterpret_cast<const u8*>(package_name() + packageNameSize);
    }  
    
    inline u8* mip_4x4() noexcept
    {
        return reinterpret_cast<u8*>(package_name() + packageNameSize);
    }

    inline const u8* dds_header() const noexcept
    {
        return reinterpret_cast<const u8*>(mip_4x4() + mip4x4Size);
    }

    inline u8* dds_header() noexcept
    {
        return reinterpret_cast<u8*>(mip_4x4() + mip4x4Size);
    }

    inline u32 dds_header_size() const noexcept
    {
        return *hl::ptradd<u32>(dds_header(), dds_header_format_off) == signature_dx10 ? 
            dds_header_dx10_size : dds_header_default_size;
    }

    template<bool fixed = false>
    inline bool valid() const noexcept
    {
#ifdef HL_IS_BIG_ENDIAN
        if constexpr (fixed)
        {
            return signature == signature_info && version == version_info;
        }
        else
        {
            return HL_SWAP_U32(signature) == signature_info && HL_SWAP_U32(version) == version_info;
        }
#else
        return signature == signature_info && version == version_info;
#endif 
    }

    inline void fix() noexcept
    {
#ifdef HL_IS_BIG_ENDIAN
        endian_swap();
#endif
    }
};

class readonly_info_wrapper
{
protected:
    raw_info* m_info;

public:
    readonly_info_wrapper()
        : m_info(nullptr)
    {

    }

    ~readonly_info_wrapper();

    inline bool valid() const noexcept
    {
        return m_info != nullptr;
    }

    const raw_info* get() const noexcept
    {
        return m_info;
    }

    bool try_load(hl::stream& stream);
};

} // texture_streaming
} // needle
} // hh
} // hl

#endif
