#include "hedgelib/textures/hl_hh_texture.h"
#include "hedgelib/io/hl_path.h"
#include "hedgelib/hl_blob.h"

namespace hl
{
namespace hh
{
namespace mirage
{
void texture_entry::in_parse(const raw_texture_entry_v1& rawTexEntry)
{
    texName = rawTexEntry.texName.get();
    texCoordIndex = rawTexEntry.texCoordIndex;
    wrapModeU = rawTexEntry.wrapModeU;
    wrapModeV = rawTexEntry.wrapModeV;
    type = rawTexEntry.type.get();
}

void texture_entry::in_parse(const void* rawData)
{
    // Get texture entry data and version number.
    u32 version;
    const auto texEntryData = get_data(rawData, &version);
    if (!texEntryData) return;

    // Parse texture entry data based on version number.
    switch (version)
    {
    case 1:
        in_parse(*static_cast<const raw_texture_entry_v1*>(texEntryData));
        break;

    default:
        throw std::runtime_error("Unsupported HH mirage texture entry version");
    }
}

void texture_entry::in_load(const nchar* filePath)
{
    // Load and parse texture entry.
    blob rawTexEntry(filePath);
    fix(rawTexEntry);
    in_parse(rawTexEntry);
}

void texture_entry::in_clear() noexcept
{
    texDataPtr = nullptr;
}

void texture_entry::fix(void* rawData)
{
    // Fix mirage data.
    mirage::fix(rawData);

    // Get texture entry data and version number.
    u32 version;
    const auto texEntryData = get_data(rawData, &version);
    if (!texEntryData) return;

    // Fix texture entry data based on version number.
    switch (version)
    {
    case 1:
        static_cast<raw_texture_entry_v1*>(texEntryData)->fix();
        break;

    default:
        throw std::runtime_error("Unsupported HH mirage texture entry version");
    }
}

map_slot_type texture_entry::get_hl_map_slot_type() const
{
    if (type == "diffuse")
    {
        return map_slot_type::diffuse;
    }
    else if (type == "specular")
    {
        return map_slot_type::specular;
    }
    else if (type == "ambient")
    {
        return map_slot_type::ambient;
    }
    else if (type == "normal")
    {
        return map_slot_type::normal;
    }
    else if (type == "reflection")
    {
        return map_slot_type::reflection;
    }
    else if (type == "displacement")
    {
        return map_slot_type::displacement;
    }
    else
    {
        //__debugbreak(); // TODO: REMOVE THIS LINE!!!
        return map_slot_type::unknown;
    }
}

void texture_entry::parse(const raw_texture_entry_v1& rawTexEntry, std::string name)
{
    // Clear any existing data.
    in_clear();

    // Set new name.
    this->name = std::move(name);

    // Parse data.
    in_parse(rawTexEntry);
}

void texture_entry::parse(const void* rawData, std::string name)
{
    // Clear any existing data.
    in_clear();

    // Set new name.
    this->name = std::move(name);

    // Parse data.
    in_parse(rawData);
}

void texture_entry::load(const nchar* filePath)
{
    // Clear any existing data.
    in_clear();

    // Set new name.
    name = std::move(get_res_name(filePath));

    // Load data.
    in_load(filePath);
}

texture_entry::texture_entry(const raw_texture_entry_v1& rawTexEntry, std::string name) :
    res_base(std::move(name))
{
    in_parse(rawTexEntry);
}

texture_entry::texture_entry(const void* rawData, std::string name) :
    res_base(std::move(name))
{
    in_parse(rawData);
}

texture_entry::texture_entry(const nchar* filePath) :
    res_base(std::move(get_res_name(filePath)))
{
    in_load(filePath);
}

void texset::in_parse(const raw_texset_v0& rawTexset, const nchar* texEntriesDir)
{
    // Reserve space in the texset for all of the texture entries.
    reserve(rawTexset.texEntryNames.count);

    // If a texture entries directory was provided, load all of the texture entries in the texset.
    if (texEntriesDir)
    {
        // Get texture entry path.
        nstring texEntryPath(texEntriesDir);
        if (path::combine_needs_sep1(texEntryPath))
        {
            texEntryPath += path::separator;
        }

        // Load all of the texture entries.
        const auto texEntryNamePos = texEntryPath.size();
        for (auto& rawTexEntryName : rawTexset.texEntryNames)
        {
            // Add the texture entry name to the texture entry path.
            texEntryPath +=
#ifdef HL_IN_WIN32_UNICODE
                text::conv<text::utf8_to_native>(rawTexEntryName.get());
#else
                rawTexEntryName.get();
#endif

            // Load the texture entry and place it into the texset.
            emplace_back(texEntryPath);

            // Remove the texture entry name from the texture entry path.
            texEntryPath.erase(texEntryNamePos);
        }
    }

    // Otherwise, just store the names of the texture entries.
    else
    {
        for (auto& rawTexEntryName : rawTexset.texEntryNames)
        {
            auto& texEntry = emplace_back();
            texEntry.name = rawTexEntryName.get();
        }
    }
}

void texset::in_parse(const void* rawData, const nchar* texEntriesDir)
{
    // Get texset data and version number.
    u32 version;
    const auto texsetData = get_data(rawData, &version);
    if (!texsetData) return;

    // Parse texset data based on version number.
    switch (version)
    {
    case 0:
        in_parse(*static_cast<const raw_texset_v0*>(texsetData), texEntriesDir);
        break;

    default:
        throw std::runtime_error("Unsupported HH mirage texset version");
    }
}

void texset::in_load(const nchar* filePath)
{
    // Load and parse texset.
    blob rawTexset(filePath);
    fix(rawTexset);
    in_parse(rawTexset, path::get_parent(filePath).c_str());
}

void texset::fix(void* rawData)
{
    // Fix mirage data.
    mirage::fix(rawData);

    // Get texset data and version number.
    u32 version;
    const auto texsetData = get_data(rawData, &version);
    if (!texsetData) return;

    // Fix texset data based on version number.
    switch (version)
    {
    case 0:
        static_cast<raw_texset_v0*>(texsetData)->fix();
        break;

    default:
        throw std::runtime_error("Unsupported HH mirage texset version");
    }
}

const texture_entry* texset::get(const char* type) const
{
    for (auto& texEntry : *this)
    {
        if (texEntry.type == type)
        {
            return &texEntry;
        }
    }

    return nullptr;
}

const texture_entry* texset::get(const std::string& type) const
{
    for (auto& texEntry : *this)
    {
        if (texEntry.type == type)
        {
            return &texEntry;
        }
    }

    return nullptr;
}

void texset::parse(const void* rawData, std::string name, const nchar* texEntriesDir)
{
    // Clear any existing data.
    clear();

    // Set new name.
    this->name = std::move(name);

    // Parse data.
    in_parse(rawData, texEntriesDir);
}

void texset::parse(const void* rawData, std::string name, const nstring& texEntriesDir)
{
    // Clear any existing data.
    clear();

    // Set new name.
    this->name = std::move(name);

    // Parse data.
    in_parse(rawData, texEntriesDir.c_str());
}

void texset::load(const nchar* filePath)
{
    // Clear any existing data.
    clear();

    // Set new name.
    name = std::move(get_res_name(filePath));

    // Load texset.
    in_load(filePath);
}

texset::texset(const void* rawData, std::string name,
    const nchar* texEntriesDir) :
    res_base(std::move(name))
{
    in_parse(rawData, texEntriesDir);
}

texset::texset(const void* rawData, std::string name,
    const nstring& texEntriesDir) :
    res_base(std::move(name))
{
    in_parse(rawData, texEntriesDir.c_str());
}

texset::texset(const nchar* filePath) :
    res_base(std::move(get_res_name(filePath)))
{
    in_load(filePath);
}
} // mirage
} // hh
} // hl
