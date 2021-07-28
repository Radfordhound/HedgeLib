#include "hedgelib/terrain/hl_hh_terrain.h"
#include "hedgelib/io/hl_file.h"
#include "hedgelib/hl_blob.h"

namespace hl
{
namespace hh
{
namespace mirage
{
void raw_terrain_instance_info_v0::fix()
{
#ifndef HL_IS_BIG_ENDIAN
    // Swap terrain instance info header.
    endian_swap<false>();

    // Swap matrix.
    matrix->endian_swap<false>();
#endif
}

static void in_swap_recursive(raw_lit_element& elem)
{
    // Swap lit element.
    elem.endian_swap<false>();

    // Swap light indices.
    for (auto& lightIndex : elem.lightIndices)
    {
        hl::endian_swap(lightIndex);
    }

    // Swap lit vertex indices.
    for (auto& litVtxIndex : elem.litVtxIndices)
    {
        hl::endian_swap(litVtxIndex);
    }
}

static void in_swap_recursive(raw_lit_mesh& mesh)
{
    // Swap mesh.
    mesh.endian_swap<false>();

    // Swap lit elements.
    for (auto& elem : mesh.litElements)
    {
        in_swap_recursive(*elem);
    }
}

static void in_swap_recursive(raw_lit_mesh_slot& slot)
{
    for (auto& meshOff : slot)
    {
        in_swap_recursive(*meshOff);
    }
}

static void in_swap_recursive(raw_lit_mesh_group& group)
{
    // Swap the mesh group.
    group.endian_swap<false>();

    // Recursively swap mesh slots.
    in_swap_recursive(group.opaq);
    in_swap_recursive(group.punch);
    in_swap_recursive(group.trans);
}

static void in_swap_recursive(arr32<off32<raw_lit_mesh_group>>& groups)
{
    for (auto& groupOff : groups)
    {
        in_swap_recursive(*groupOff);
    }
}

void raw_terrain_instance_info_v5::fix()
{
#ifndef HL_IS_BIG_ENDIAN
    // Swap terrain instance info header.
    endian_swap<false>();

    // Swap matrix.
    matrix->endian_swap<false>();

    // Swap lit mesh groups.
    in_swap_recursive(litMeshGroups);
#endif
}

void lit_element::write(std::size_t basePos, stream& stream, off_table& offTable) const
{
    // Generate raw lit element.
    const std::size_t rawLitElemPos = stream.tell();
    raw_lit_element rawLitElem =
    {
        { static_cast<u32>(lightIndices.size()), static_cast<u32>(      // lightIndices
            rawLitElemPos + sizeof(raw_lit_element) - basePos) },

        { static_cast<u32>(litVtxIndices.size()), nullptr }             // litVtxIndices
    };

    // Endian-swap raw lit element if necessary.
#ifndef HL_IS_BIG_ENDIAN
    rawLitElem.endian_swap();
#endif

    // Write raw lit element to stream.
    stream.write_obj(rawLitElem);

    // Add light indices offset to offset table.
    offTable.push_back(rawLitElemPos + 4);

    // Write light indices.
#ifndef HL_IS_BIG_ENDIAN
    for (auto lightIndex : lightIndices)
    {
        hl::endian_swap(lightIndex);
        stream.write_obj(lightIndex);
    }
#else
    stream.write_arr(lightIndices.size(), lightIndices.data());
#endif

    // Fix lit vertex indices offset.
    stream.fix_off32(basePos, rawLitElemPos + 12,
        needs_endian_swap, offTable);

    // Write lit vertex indices.
#ifndef HL_IS_BIG_ENDIAN
    for (auto litVtxIndex : litVtxIndices)
    {
        hl::endian_swap(litVtxIndex);
        stream.write_obj(litVtxIndex);
    }
#else
    stream.write_arr(litVtxIndices.size(), litVtxIndices.data());
#endif

    stream.pad(4);
}

lit_element::lit_element(const raw_lit_element& rawElem) :
    lightIndices(rawElem.lightIndices.begin(), rawElem.lightIndices.end()),
    litVtxIndices(rawElem.litVtxIndices.begin(), rawElem.litVtxIndices.end()) {}

void lit_mesh::write(std::size_t basePos, stream& stream, off_table& offTable) const
{
    // Generate raw lit mesh.
    const std::size_t rawLitMeshPos = stream.tell();
    raw_lit_mesh rawLitMesh =
    {
        { static_cast<u32>(litElements.size()), static_cast<u32>(       // litElements
            rawLitMeshPos + sizeof(raw_lit_mesh) - basePos) }
    };

    // Endian-swap raw lit mesh if necessary.
#ifndef HL_IS_BIG_ENDIAN
    rawLitMesh.endian_swap();
#endif

    // Write raw lit mesh to stream.
    stream.write_obj(rawLitMesh);

    // Add lit elements offset to offset table.
    offTable.push_back(rawLitMeshPos + 4);

    // Write placeholder lit elements offsets.
    std::size_t curLitElemOffPos = stream.tell();
    stream.write_nulls(sizeof(off32<raw_lit_element>) * litElements.size());

    // Write lit elements and fill-in placeholder lit element offsets.
    for (auto& litElem : litElements)
    {
        // Fill-in placeholder lit element offset.
        stream.fix_off32(basePos, curLitElemOffPos, needs_endian_swap, offTable);

        // Write lit element.
        litElem.write(basePos, stream, offTable);

        // Increase current lit element offset position.
        curLitElemOffPos += sizeof(off32<raw_lit_element>);
    }
}

lit_mesh::lit_mesh(const raw_lit_mesh& rawMesh) :
    litElements(rawMesh.litElements.begin(), rawMesh.litElements.end()) {}

void lit_mesh_slot::write(std::size_t basePos, stream& stream, off_table& offTable) const
{
    // Write placeholder lit mesh offsets.
    std::size_t curLitMeshOffPos = stream.tell();
    stream.write_nulls(sizeof(off32<raw_lit_mesh>) * size());

    // Write lit meshes and fill-in placeholder lit mesh offsets.
    for (auto& litMesh : *this)
    {
        // Fill-in placeholder lit mesh offset.
        stream.fix_off32(basePos, curLitMeshOffPos, needs_endian_swap, offTable);

        // Write lit mesh.
        litMesh.write(basePos, stream, offTable);

        // Increase current lit mesh offset position.
        curLitMeshOffPos += sizeof(off32<raw_lit_mesh>);
    }
}

lit_mesh_slot::lit_mesh_slot(const raw_lit_mesh_slot& rawSlot) :
    vector(rawSlot.begin(), rawSlot.end()) {}

void lit_mesh_group::write(std::size_t basePos,
    stream& stream, off_table& offTable) const
{
    // Generate raw lit mesh group.
    const std::size_t rawLitMeshGroupPos = stream.tell();
    raw_lit_mesh_group rawLitMeshGroup =
    {
        { static_cast<u32>(opaq.size()), nullptr },                     // opaq
        { static_cast<u32>(trans.size()), nullptr },                    // trans
        { static_cast<u32>(punch.size()), nullptr }                     // punch
    };

    // Endian-swap raw lit mesh group if necessary.
#ifndef HL_IS_BIG_ENDIAN
    rawLitMeshGroup.endian_swap();
#endif

    // Write raw lit mesh group to stream.
    stream.write_obj(rawLitMeshGroup);

    // Write lit mesh slots.
    stream.fix_off32(basePos, rawLitMeshGroupPos + 4, needs_endian_swap, offTable);
    opaq.write(basePos, stream, offTable);

    stream.fix_off32(basePos, rawLitMeshGroupPos + 12, needs_endian_swap, offTable);
    trans.write(basePos, stream, offTable);

    stream.fix_off32(basePos, rawLitMeshGroupPos + 20, needs_endian_swap, offTable);
    punch.write(basePos, stream, offTable);
}

lit_mesh_group::lit_mesh_group(const raw_lit_mesh_group& rawGroup) :
    opaq(rawGroup.opaq),
    trans(rawGroup.trans),
    punch(rawGroup.punch) {}

void terrain_instance_info::fix(void* rawData)
{
    // Fix mirage data.
    mirage::fix(rawData);

    // Get terrain instance info data and version number.
    u32 version;
    void* instInfoData = get_data(rawData, &version);
    if (!instInfoData) return;

    // Fix terrain instance info data based on version number.
    switch (version)
    {
    case 0:
        static_cast<raw_terrain_instance_info_v0*>(instInfoData)->fix();
        break;

    case 5:
        static_cast<raw_terrain_instance_info_v5*>(instInfoData)->fix();
        break;

    default:
        throw std::runtime_error("Unsupported HH terrain instance info version");
    }
}

void terrain_instance_info::parse(const raw_terrain_instance_info_v0& rawInstInfo)
{
    // Parse terrain instance data.
    modelName = rawInstInfo.modelName.get();
    instanceName = rawInstInfo.instanceName.get();
    matrix = *rawInstInfo.matrix;
}

void terrain_instance_info::parse(const raw_terrain_instance_info_v5& rawInstInfo)
{
    // Parse terrain instance data.
    modelName = rawInstInfo.modelName.get();
    instanceName = rawInstInfo.instanceName.get();
    matrix = *rawInstInfo.matrix;

    // Parse lit mesh groups.
    litMeshGroups.reserve(rawInstInfo.litMeshGroups.count);
    for (auto& rawLitMeshGroup : rawInstInfo.litMeshGroups)
    {
        litMeshGroups.emplace_back(*rawLitMeshGroup);
    }
}

void terrain_instance_info::parse(const void* rawData)
{
    // Get terrain instance info data and version number.
    u32 version;
    const void* instInfoData = get_data(rawData, &version);
    if (!instInfoData) return; // TODO: Should this be an error?

    // Parse terrain instance info data based on version number.
    switch (version)
    {
    case 0:
        parse(*static_cast<const raw_terrain_instance_info_v0*>(instInfoData));
        break;

    case 5:
        parse(*static_cast<const raw_terrain_instance_info_v5*>(instInfoData));
        break;

    default:
        throw std::runtime_error("Unsupported HH terrain instance info version");
    }
}

void terrain_instance_info::load(const nchar* filePath)
{
    blob rawInstInfo(filePath);
    fix(rawInstInfo);
    parse(rawInstInfo);
}

void terrain_instance_info::write(stream& stream,
    off_table& offTable, u32 version) const
{
    // Write terrain instance info header based on version.
    const std::size_t basePos = stream.tell();
    switch (version)
    {
    case 0:
    {
        // Generate raw V0 terrain instance info header.
        raw_terrain_instance_info_v0 rawInstInfo =
        {
            static_cast<u32>(sizeof(raw_terrain_instance_info_v0)),     // modelName
            nullptr,                                                    // matrix
            nullptr                                                     // instanceName
        };

        // Endian-swap header if necessary.
#ifndef HL_IS_BIG_ENDIAN
        rawInstInfo.endian_swap();
#endif

        // Write header to stream.
        stream.write_obj(rawInstInfo);
        break;
    }

    case 5:
    {
        // Generate raw V5 terrain instance info header.
        raw_terrain_instance_info_v5 rawInstInfo =
        {
            static_cast<u32>(sizeof(raw_terrain_instance_info_v5)),     // modelName
            nullptr,                                                    // matrix
            nullptr,                                                    // instanceName
            { static_cast<u32>(litMeshGroups.size()), nullptr }         // litMeshGroups
        };

        // Endian-swap header if necessary.
#ifndef HL_IS_BIG_ENDIAN
        rawInstInfo.endian_swap();
#endif

        // Write header to stream.
        stream.write_obj(rawInstInfo);
        break;
    }

    default:
        throw std::runtime_error("Unsupported HH terrain instance info version");
    }

    // Write model name.
    offTable.push_back(basePos);
    stream.write_str(modelName);
    stream.pad(4);

    // Write matrix.
    {
#ifndef HL_IS_BIG_ENDIAN
        matrix4x4 matrix = this->matrix;
        matrix.endian_swap();
#endif

        stream.fix_off32(basePos, basePos + 4,
            needs_endian_swap, offTable);

        stream.write_obj(matrix);
    }

    // Write instance name.
    stream.fix_off32(basePos, basePos + 8,
        needs_endian_swap, offTable);

    stream.write_str(instanceName);
    stream.pad(4);

    // Write lit meshes/mesh groups as necessary.
    switch (version)
    {
    case 5:
        // Fill-in lit mesh groups offset.
        stream.fix_off32(basePos, basePos + 16, needs_endian_swap, offTable);

        // Write placeholder offsets for mesh groups.
        std::size_t curLitMeshGroupOffPos = stream.tell();
        stream.write_nulls(sizeof(off32<raw_lit_mesh_group>) * litMeshGroups.size());

        // Write lit mesh groups and fill-in placeholder offsets.
        for (auto& litMeshGroup : litMeshGroups)
        {
            // Fill-in placeholder mesh group offset.
            stream.fix_off32(basePos, curLitMeshGroupOffPos, needs_endian_swap, offTable);

            // Write lit mesh group.
            litMeshGroup.write(basePos, stream, offTable);

            // Increase current lit mesh group offset position.
            curLitMeshGroupOffPos += sizeof(off32<raw_lit_mesh_group>);
        }
        break;
    }
}

void terrain_instance_info::save(stream& stream, u32 version, const char* fileName) const
{
    // Start writing header.
    const std::size_t headerPos = stream.tell();
    raw_header::start_write(version, stream);

    // Write terrain instance info data.
    off_table offTable;
    write(stream, offTable, version);

    // Finish writing header.
    raw_header::finish_write(headerPos, offTable, stream, fileName);
}

void terrain_instance_info::save(const nchar* filePath, u32 version) const
{
    file_stream stream(filePath, file::mode::write);
    save(stream, version, (version == 5) ? "" : nullptr);
}

void terrain_instance_info::save(stream& stream) const
{
    const u32 version = (litMeshGroups.empty()) ? 0 : 5;
    save(stream, version, (version == 5) ? "" : nullptr);
}

void terrain_instance_info::save(const nchar* filePath) const
{
    file_stream stream(filePath, file::mode::write);
    save(stream);
}
} // mirage
} // hh
} // hl
