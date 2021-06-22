#include "hedgelib/textures/hl_hh_texture.h"
#include "hedgelib/io/hl_path.h"
#include "hedgelib/hl_blob.h"

namespace hl
{
namespace hh
{
namespace mirage
{
void texture_entry::fix(void* rawData)
{
    // Fix mirage data.
    mirage::fix(rawData);

    // Get texture entry data and version number.
    u32 version;
    void* texEntryData = get_data(rawData, &version);
    if (!texEntryData) return;

    // Fix texture entry data based on version number.
    switch (version)
    {
    case 1:
        static_cast<raw_texture_entry_v1*>(texEntryData)->fix();
        break;

    default:
        throw std::runtime_error("Unsupported HH texture entry version");
    }
}

map_slot_type texture_entry::get_map_slot_type() const
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

void texture_entry::parse(const raw_texture_entry_v1& rawTexEntry)
{
    texName = rawTexEntry.texName.get();
    texCoordIndex = rawTexEntry.texCoordIndex;
    wrapModeU = rawTexEntry.wrapModeU;
    wrapModeV = rawTexEntry.wrapModeV;
    type = rawTexEntry.type.get();
}

void texture_entry::parse(const void* rawData)
{
    // Get texture entry data and version number.
    u32 version;
    const void* texEntryData = get_data(rawData, &version);
    if (!texEntryData) return; // TODO: Should this be an error?

    // Parse texture entry data based on version number.
    switch (version)
    {
    case 1:
        parse(*static_cast<const raw_texture_entry_v1*>(texEntryData));
        break;

    default:
        throw std::runtime_error("Unsupported HH texture entry version");
    }
}

void texture_entry::load(const nchar* filePath)
{
    // Get texture entry name from file path.
#ifdef HL_IN_WIN32_UNICODE
    name = text::conv<text::native_to_utf8>(
        path::remove_ext(path::get_name(filePath)));
#else
    name = path::remove_ext(path::get_name(filePath));
#endif

    // Load and parse texture entry.
    blob rawTexEntry(filePath);
    fix(rawTexEntry);
    parse(rawTexEntry);
}

void texset::fix(void* rawData)
{
    // Fix mirage data.
    mirage::fix(rawData);

    // Get texset data and version number.
    u32 version;
    void* texsetData = get_data(rawData, &version);
    if (!texsetData) return;

    // Fix texset data based on version number.
    switch (version)
    {
    case 0:
        static_cast<raw_texset_v0*>(texsetData)->fix();
        break;

    default:
        throw std::runtime_error("Unsupported HH texset version");
    }
}

void texset::parse(const raw_texset_v0& rawTexset, const nchar* textureEntriesDir)
{
    // Get texture entry path.
    nstring texEntryPath = textureEntriesDir;
    if (path::combine_needs_sep1(texEntryPath))
    {
        texEntryPath += path::separator;
    }

    // Reserve space in the texset for all of the texture entries.
    const std::size_t texEntryNamePos = texEntryPath.size();
    reserve(rawTexset.textureEntryNames.count);

    // Load all of the texture entries.
    for (auto& rawTexEntryName : rawTexset.textureEntryNames)
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

void texset::parse(const void* rawData, const nchar* textureEntriesDir)
{
    // Get texset data and version number.
    u32 version;
    const void* texsetData = get_data(rawData, &version);
    if (!texsetData) return; // TODO: Should this be an error?

    // Parse texset data based on version number.
    switch (version)
    {
    case 0:
        parse(*static_cast<const raw_texset_v0*>(texsetData), textureEntriesDir);
        break;

    default:
        throw std::runtime_error("Unsupported HH texset version");
    }
}

void texset::load(const nchar* filePath)
{
    // Get texset name from file path.
#ifdef HL_IN_WIN32_UNICODE
    name = text::conv<text::native_to_utf8>(
        path::remove_ext(path::get_name(filePath)));
#else
    name = path::remove_ext(path::get_name(filePath));
#endif

    // Load and parse texset.
    blob rawTexset(filePath);
    fix(rawTexset);
    parse(rawTexset, path::get_parent(filePath));
}
} // mirage
} // hh
} // hl
