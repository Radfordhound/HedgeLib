#ifndef HL_HH_TEXTURE_H_INCLUDED
#define HL_HH_TEXTURE_H_INCLUDED
#include "../hl_scene.h"
#include "../io/hl_hh_mirage.h"

namespace hl
{
namespace hh
{
namespace mirage
{
/* Thanks to Skyth for cracking texture wrap modes and HHTexture flags! */
enum class texture_wrap_mode : u8
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
    texture_wrap_mode wrapModeU;
    texture_wrap_mode wrapModeV;
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
    arr32<off32<char>> textureEntryNames;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(textureEntryNames);
    }

    inline void fix()
    {
#ifndef HL_IS_BIG_ENDIAN
        endian_swap<false>();
#endif
    }
};

HL_STATIC_ASSERT_SIZE(raw_texset_v0, 8);

struct texture_entry
{
    std::string name;
    std::string texName;
    std::string type;
    u8 texCoordIndex = 0;
    texture_wrap_mode wrapModeU = texture_wrap_mode::repeat;
    texture_wrap_mode wrapModeV = texture_wrap_mode::repeat;

    constexpr static const nchar ext[] = HL_NTEXT(".texture");

    HL_API static void fix(void* rawData);

    HL_API map_slot_type get_map_slot_type() const;

    HL_API void parse(const raw_texture_entry_v1& rawTexEntry);
    HL_API void parse(const void* rawData);

    HL_API void load(const nchar* filePath);

    inline void load(const nstring& filePath)
    {
        load(filePath);
    }

    texture_entry() = default;

    texture_entry(const void* rawData)
    {
        parse(rawData);
    }

    texture_entry(const nchar* filePath)
    {
        load(filePath);
    }

    inline texture_entry(const nstring& filePath) :
        texture_entry(filePath.c_str()) {}

    texture_entry(const char* name, const raw_texture_entry_v1& rawTexEntry) :
        name(name)
    {
        parse(rawTexEntry);
    }

    texture_entry(const std::string& name, const raw_texture_entry_v1& rawTexEntry) :
        name(name)
    {
        parse(rawTexEntry);
    }

    texture_entry(std::string&& name, const raw_texture_entry_v1& rawTexEntry) :
        name(std::move(name))
    {
        parse(rawTexEntry);
    }
};

struct texset : public std::vector<texture_entry>
{
    std::string name;

    constexpr static const nchar* const ext = HL_NTEXT(".texset");

    HL_API static void fix(void* rawData);

    HL_API void parse(const raw_texset_v0& rawTexset, const nchar* textureEntriesDir);

    inline void parse(const raw_texset_v0& rawTexset, const nstring& textureEntriesDir)
    {
        parse(rawTexset, textureEntriesDir.c_str());
    }

    HL_API void parse(const void* rawData, const nchar* textureEntriesDir);

    inline void parse(const void* rawData, const nstring& textureEntriesDir)
    {
        parse(rawData, textureEntriesDir.c_str());
    }

    HL_API void load(const nchar* filePath);

    inline void load(const nstring& filePath)
    {
        load(filePath.c_str());
    }

    texset() = default;

    texset(const void* rawData, const nchar* textureEntriesDir)
    {
        parse(rawData, textureEntriesDir);
    }

    texset(const void* rawData, nstring& textureEntriesDir)
    {
        parse(rawData, textureEntriesDir);
    }

    texset(const nchar* filePath)
    {
        load(filePath);
    }

    inline texset(const nstring& filePath) :
        texset(filePath.c_str()) {}
};
} // mirage
} // hh
} // hl
#endif
