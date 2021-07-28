#pragma once
#include "../hl_math.h"
#include "../io/hl_hh_mirage.h"

namespace hl
{
namespace hh
{
namespace mirage
{
struct raw_lit_element
{
    /**
        @brief The indices of the omni light entries within the
        stage's light list that light up this element.
    */
    arr32<u32> lightIndices;
    /**
        @brief The indices of the vertices within the mesh that are
        lit up by the lights referenced by lightIndices. Stored as
        triangle strips.

        NOTE: These *AREN'T* indices to *faces* within the
        HH mesh, but instead, are indices to *vertices* within
        the HH mesh. Simply put, these are the same thing as the
        actual faces stored within the HH mesh.
    */
    arr32<u16> litVtxIndices;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(lightIndices);
        hl::endian_swap<swapOffsets>(litVtxIndices);
    }
};

HL_STATIC_ASSERT_SIZE(raw_lit_element, 16);

struct raw_lit_mesh
{
    arr32<off32<raw_lit_element>> litElements;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(litElements);
    }
};

HL_STATIC_ASSERT_SIZE(raw_lit_mesh, 8);

using raw_lit_mesh_slot = arr32<off32<raw_lit_mesh>>;

HL_STATIC_ASSERT_SIZE(raw_lit_mesh_slot, 8);

struct raw_lit_mesh_group
{
    raw_lit_mesh_slot opaq;
    raw_lit_mesh_slot trans;
    raw_lit_mesh_slot punch;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(opaq);
        hl::endian_swap<swapOffsets>(trans);
        hl::endian_swap<swapOffsets>(punch);
    }
};

HL_STATIC_ASSERT_SIZE(raw_lit_mesh_group, 0x18);

struct raw_terrain_instance_info_v0
{
    off32<char> modelName;
    off32<matrix4x4> matrix;
    off32<char> instanceName;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(modelName);
        hl::endian_swap<swapOffsets>(matrix);
        hl::endian_swap<swapOffsets>(instanceName);
    }

    HL_API void fix();
};

HL_STATIC_ASSERT_SIZE(raw_terrain_instance_info_v0, 12);

struct raw_terrain_instance_info_v5
{
    off32<char> modelName;
    off32<matrix4x4> matrix;
    off32<char> instanceName;
    arr32<off32<raw_lit_mesh_group>> litMeshGroups;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(modelName);
        hl::endian_swap<swapOffsets>(matrix);
        hl::endian_swap<swapOffsets>(instanceName);
        hl::endian_swap<swapOffsets>(litMeshGroups);
    }

    HL_API void fix();
};

HL_STATIC_ASSERT_SIZE(raw_terrain_instance_info_v5, 0x14);

struct lit_element
{
    /**
        @brief The indices of the omni light entries within the
        stage's light list that light up this element.
    */
    std::vector<u32> lightIndices;
    /**
        @brief The indices of the vertices within the mesh that are
        lit up by the lights referenced by lightIndices. Stored as
        triangle strips.

        NOTE: These *AREN'T* indices to *faces* within the
        HH mesh, but instead, are indices to *vertices* within
        the HH mesh. Simply put, these are the same thing as the
        actual faces stored within the HH mesh.
    */
    std::vector<u16> litVtxIndices;

    HL_API void write(std::size_t basePos, stream& stream, off_table& offTable) const;

    lit_element() = default;
    HL_API lit_element(const raw_lit_element& rawElem);

    inline lit_element(const off32<raw_lit_element>& rawElemOff) :
        lit_element(*rawElemOff) {}
};

struct lit_mesh
{
    std::vector<lit_element> litElements;

    HL_API void write(std::size_t basePos, stream& stream, off_table& offTable) const;

    lit_mesh() = default;
    HL_API lit_mesh(const raw_lit_mesh& rawMesh);

    inline lit_mesh(const off32<raw_lit_mesh>& rawMeshOff) :
        lit_mesh(*rawMeshOff) {}
};

struct lit_mesh_slot : public std::vector<lit_mesh>
{
    HL_API void write(std::size_t basePos, stream& stream, off_table& offTable) const;

    lit_mesh_slot() = default;
    HL_API lit_mesh_slot(const raw_lit_mesh_slot& rawSlot);
};

struct lit_mesh_group
{
    lit_mesh_slot opaq;
    lit_mesh_slot trans;
    lit_mesh_slot punch;

    HL_API void write(std::size_t basePos,
        stream& stream, off_table& offTable) const;

    lit_mesh_group() = default;

    HL_API lit_mesh_group(const raw_lit_mesh_group& rawGroup);
};

struct terrain_instance_info
{
    std::string modelName;
    std::string instanceName;
    matrix4x4 matrix = matrix4x4::identity;
    std::vector<lit_mesh_group> litMeshGroups;

    constexpr static const nchar ext[] = HL_NTEXT(".terrain-instanceinfo");

    HL_API static void fix(void* rawData);

    HL_API void parse(const raw_terrain_instance_info_v0& rawInstInfo);
    HL_API void parse(const raw_terrain_instance_info_v5& rawInstInfo);
    HL_API void parse(const void* rawData);

    HL_API void load(const nchar* filePath);

    inline void load(const nstring& filePath)
    {
        load(filePath);
    }

    HL_API void write(stream& stream, off_table& offTable, u32 version) const;
    HL_API void save(stream& stream, u32 version, const char* fileName = "") const;
    HL_API void save(const nchar* filePath, u32 version) const;

    inline void save(const nstring& filePath, u32 version) const
    {
        save(filePath.c_str(), version);
    }

    HL_API void save(stream& stream) const;
    HL_API void save(const nchar* filePath) const;

    inline void save(const nstring& filePath) const
    {
        save(filePath.c_str());
    }

    terrain_instance_info() = default;
    terrain_instance_info(const void* rawData)
    {
        parse(rawData);
    }

    terrain_instance_info(const nchar* filePath)
    {
        load(filePath);
    }

    inline terrain_instance_info(const nstring& filePath) :
        terrain_instance_info(filePath.c_str()) {}
};
} // mirage
} // hh
} // hl
