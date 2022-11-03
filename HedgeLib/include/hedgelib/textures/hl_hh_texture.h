#ifndef HL_HH_TEXTURE_H_INCLUDED
#define HL_HH_TEXTURE_H_INCLUDED
#include "../hl_scene.h"
#include "../hl_resource.h"
#include "../io/hl_hh_mirage.h"

namespace hl
{
namespace hh
{
namespace mirage
{
/* Thanks to Skyth for cracking texture wrap modes and HH texture flags! */
enum class raw_texture_wrap_mode : u8
{
    repeat = 0,
    mirror = 1,
    clamp = 2,
    mirror_once = 3,
    border = 4
};

struct raw_texture_entry_v1
{
    off32<char> texName;
    u8 texCoordIndex;
    raw_texture_wrap_mode wrapModeU;
    raw_texture_wrap_mode wrapModeV;
    u8 padding;
    off32<char> type;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(texName);
        hl::endian_swap<swapOffsets>(type);
    }

    inline void fix()
    {
#ifndef HL_IS_BIG_ENDIAN
        endian_swap<false>();
#endif
    }
};

HL_STATIC_ASSERT_SIZE(raw_texture_entry_v1, 12);

struct raw_texset_v0
{
    arr32<off32<char>> texEntryNames;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(texEntryNames);
    }

    inline void fix()
    {
#ifndef HL_IS_BIG_ENDIAN
        endian_swap<false>();
#endif
    }
};

HL_STATIC_ASSERT_SIZE(raw_texset_v0, 8);

using texture_wrap_mode = raw_texture_wrap_mode;

class texture_entry : public res_base
{
    HL_API void in_parse(const raw_texture_entry_v1& rawTexEntry);

    HL_API void in_parse(const void* rawData);

    HL_API void in_load(const nchar* filePath);

    HL_API void in_clear() noexcept;

public:
    std::string texName;
    void* texDataPtr = nullptr;
    std::string type;
    u8 texCoordIndex = 0;
    texture_wrap_mode wrapModeU = texture_wrap_mode::repeat;
    texture_wrap_mode wrapModeV = texture_wrap_mode::repeat;

    inline constexpr static nchar ext[] = HL_NTEXT(".texture");

    HL_API static void fix(void* rawData);

    HL_API map_slot_type get_hl_map_slot_type() const;

    HL_API void parse(const raw_texture_entry_v1& rawTexEntry, std::string name);

    HL_API void parse(const void* rawData, std::string name);

    HL_API void load(const nchar* filePath);

    inline void load(const nstring& filePath)
    {
        load(filePath.c_str());
    }

    texture_entry() noexcept :
        res_base("default_texture") {}

    HL_API texture_entry(const raw_texture_entry_v1& rawTexEntry, std::string name);

    HL_API texture_entry(const void* rawData, std::string name);

    HL_API texture_entry(const nchar* filePath);

    inline texture_entry(const nstring& filePath) :
        texture_entry(filePath.c_str()) {}
};

class texset : public res_base, public std::vector<texture_entry>
{
    HL_API void in_parse(const raw_texset_v0& rawTexset, const nchar* texEntriesDir);

    HL_API void in_parse(const void* rawData, const nchar* texEntriesDir);

    HL_API void in_load(const nchar* filePath);

public:
    inline constexpr static nchar ext[] = HL_NTEXT(".texset");

    HL_API static void fix(void* rawData);

    HL_API const texture_entry* get(const char* type) const;

    HL_API const texture_entry* get(const std::string& type) const;

    inline texture_entry* get(const char* type)
    {
        return const_cast<texture_entry*>(
            const_cast<const texset*>(this)->get(type));
    }

    inline texture_entry* get(const std::string& type)
    {
        return const_cast<texture_entry*>(
            const_cast<const texset*>(this)->get(type));
    }

    HL_API void parse(const void* rawData, std::string name,
        const nchar* texEntriesDir = nullptr);

    HL_API void parse(const void* rawData, std::string name,
        const nstring& texEntriesDir);

    HL_API void load(const nchar* filePath);

    inline void load(const nstring& filePath)
    {
        load(filePath.c_str());
    }

    texset() noexcept :
        res_base("default_texset") {}

    HL_API texset(const void* rawData, std::string name,
        const nchar* texEntriesDir = nullptr);

    HL_API texset(const void* rawData, std::string name,
        const nstring& texEntriesDir);

    HL_API texset(const nchar* filePath);

    inline texset(const nstring& filePath) :
        texset(filePath.c_str()) {}
};
} // mirage
} // hh
} // hl
#endif
