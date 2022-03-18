#ifndef HL_HH_MODEL_H_INCLUDED
#define HL_HH_MODEL_H_INCLUDED
#include "../hl_scene.h"
#include "../io/hl_hh_mirage.h"
#include <unordered_set>

namespace hl
{
namespace hh
{
namespace mirage
{
struct skeletal_model;
struct terrain_model;
struct node;

/**
 * @brief Seems to be D3D_PRIMITIVE_TOPOLOGY, but with 1 subtracted from each value?
 * Perhaps this was done to make it impossible to use D3D_PRIMITIVE_TOPOLOGY_UNDEFINED?
*/
enum class raw_topology_type : u32
{
    triangle_list = 3,
    triangle_strip = 4 // TODO: This was guessed from d3d's D3D_PRIMITIVE_TOPOLOGY enum - is this correct?
};

/**
 * @brief D3DDECLTYPE from the Xbox 360.
 * Thanks to Skyth for helping to figure this out!
*/
enum class raw_vertex_format : u32
{
    float1 = 0x2C83A4U,
    float2 = 0x2C23A5U,
    float3 = 0x2A23B9U,
    float4 = 0x1A23A6U,
    int1 = 0x2C83A1U,
    int2 = 0x2C23A2U,
    int4 = 0x1A23A3U,
    uint1 = 0x2C82A1U,
    uint2 = 0x2C22A2U,
    uint4 = 0x1A22A3U,
    int1_norm = 0x2C81A1U,
    int2_norm = 0x2C21A2U,
    int4_norm = 0x1A21A3U,
    uint1_norm = 0x2C80A1U,
    uint2_norm = 0x2C20A2U,
    uint4_norm = 0x1A20A3U,
    d3d_color = 0x182886U,
    ubyte4 = 0x1A2286U,
    byte4 = 0x1A2386U,
    ubyte4_norm = 0x1A2086U,
    byte4_norm = 0x1A2186U,
    short2 = 0x2C2359U,
    short4 = 0x1A235AU,
    ushort2 = 0x2C2259U,
    ushort4 = 0x1A225AU,
    short2_norm = 0x2C2159U,
    short4_norm = 0x1A215AU,
    ushort2_norm = 0x2C2059U,
    ushort4_norm = 0x1A205AU,
    udec3 = 0x2A2287U,
    dec3 = 0x2A2387U,
    udec3_norm = 0x2A2087U,
    dec3_norm = 0x2A2187U,
    udec4 = 0x1A2287U,
    dec4 = 0x1A2387U,
    udec4_norm = 0x1A2087U,
    dec4_norm = 0x1A2187U,
    uhend3 = 0x2A2290U,
    hend3 = 0x2A2390U,
    uhend3_norm = 0x2A2090U,
    hend3_norm = 0x2A2190U,
    udhen3 = 0x2A2291U,
    dhen3 = 0x2A2391U,
    udhen3_norm = 0x2A2091U,
    dhen3_norm = 0x2A2191U,
    float16_2 = 0x2C235FU,
    float16_4 = 0x1A2360U,
    last_entry = 0xFFFFFFFFU
};

/**
 * @brief D3DDECLMETHOD from Direct3D 9.
*/
enum class raw_vertex_method : u8
{
    normal = 0,
    partial_u = 1,
    partial_v = 2,
    cross_uv = 3,
    uv = 4,
    lookup = 5,
    lookup_presampled = 6
};

/**
 * @brief D3DDECLUSAGE from Direct3D 9.
*/
enum class raw_vertex_type : u8
{
    position = 0,
    blend_weight = 1,
    blend_indices = 2,
    normal = 3,
    psize = 4,
    texcoord = 5,
    tangent = 6,
    binormal = 7,
    tess_factor = 8,
    position_t = 9,
    color = 10,
    fog = 11,
    depth = 12,
    sample = 13
};

struct raw_texture_unit
{
    off32<char> name;
    u8 index;
    u8 padding1;
    u8 padding2;
    u8 padding3;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
    }
};

HL_STATIC_ASSERT_SIZE(raw_texture_unit, 8);

/**
 * @brief D3DVERTEXELEMENT9 from the Xbox 360.
*/
struct raw_vertex_element
{
    u16 stream;
    u16 offset;
    /** @brief See hl::hh::mirage::raw_vertex_format. */
    u32 format;
    raw_vertex_method method;
    raw_vertex_type type;
    u8 index;
    u8 padding;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(stream);
        hl::endian_swap(offset);
        hl::endian_swap(format);
    }

    HL_API void convert_to_vec4(const void* vtx, vec4& vec) const;
    HL_API void convert_to_ivec4(const void* vtx, ivec4& ivec) const;
};

HL_STATIC_ASSERT_SIZE(raw_vertex_element, 12);

struct raw_mesh
{
    off32<char> materialName;
    arr32<u16> faces;
    u32 vertexCount;
    u32 vertexSize;
    off32<void> vertices;
    off32<raw_vertex_element> vertexElements;
    arr32<u8> boneNodeIndices;
    arr32<off32<raw_texture_unit>> textureUnits;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(materialName);
        hl::endian_swap<swapOffsets>(faces);
        hl::endian_swap(vertexCount);
        hl::endian_swap(vertexSize);
        hl::endian_swap<swapOffsets>(vertices);
        hl::endian_swap<swapOffsets>(vertexElements);
        hl::endian_swap<swapOffsets>(boneNodeIndices);
        hl::endian_swap<swapOffsets>(textureUnits);
    }
};

HL_STATIC_ASSERT_SIZE(raw_mesh, 0x2C);

using raw_mesh_slot = arr32<off32<raw_mesh>>;

HL_STATIC_ASSERT_SIZE(raw_mesh_slot, 8);

struct raw_special_meshes
{
    u32 count;
    off32<off32<char>> types;               // char* types[count]
    off32<off32<u32>> meshCounts;           // u32* meshCounts[count]
    off32<off32<off32<raw_mesh>>> meshes;   // raw_mesh** meshes[count]

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(count);
        hl::endian_swap<swapOffsets>(types);
        hl::endian_swap<swapOffsets>(meshCounts);
        hl::endian_swap<swapOffsets>(meshes);
    }

    struct const_wrapper
    {
        const char* type;
        const off32<raw_mesh>* meshes;
        u32 meshCount;

        const raw_mesh* begin() const noexcept
        {
            return meshes->get();
        }

        const raw_mesh* end() const noexcept
        {
            return &((*meshes)[meshCount]);
        }

        const raw_mesh& operator[](std::size_t i) const noexcept
        {
            return ((*meshes)[i]);
        }

        const_wrapper(const char* type, u32 meshCount,
            const off32<raw_mesh>* meshes) noexcept : type(type),
            meshes(meshes), meshCount(meshCount) {}
    };

    struct wrapper
    {
        char* type;
        off32<raw_mesh>* meshes;
        u32 meshCount;

        const raw_mesh* begin() const noexcept
        {
            return meshes->get();
        }

        raw_mesh* begin() noexcept
        {
            return meshes->get();
        }

        const raw_mesh* end() const noexcept
        {
            return &((*meshes)[meshCount]);
        }

        raw_mesh* end() noexcept
        {
            return &((*meshes)[meshCount]);
        }

        const raw_mesh& operator[](std::size_t i) const noexcept
        {
            return ((*meshes)[i]);
        }

        raw_mesh& operator[](std::size_t i) noexcept
        {
            return ((*meshes)[i]);
        }

        wrapper(char* type, u32 meshCount,
            off32<raw_mesh>* meshes) noexcept : type(type),
            meshes(meshes), meshCount(meshCount) {}
    };

    class const_iterator
    {
    protected:
        const raw_special_meshes* m_slot = nullptr;
        u32 m_curIndex = 0;

    public:
        const_wrapper operator*() const noexcept
        {
            return (*m_slot)[m_curIndex];
        }

        const_iterator& operator++() noexcept
        {
            ++m_curIndex;
            return *this;
        }

        const_iterator operator++(int) noexcept
        {
            const_iterator tmpCopy(*this);
            operator++();
            return tmpCopy;
        }

        bool operator==(const const_iterator& other) const noexcept
        {
            return (m_slot == other.m_slot && m_curIndex == other.m_curIndex);
        }

        bool operator!=(const const_iterator& other) const noexcept
        {
            return (m_slot != other.m_slot || m_curIndex != other.m_curIndex);
        }

        const_iterator(const raw_special_meshes* slot,
            u32 index) noexcept : m_slot(slot), m_curIndex(index) {}
    };

    struct iterator : public const_iterator
    {
        wrapper operator*() noexcept
        {
            return (*const_cast<raw_special_meshes*>(m_slot))[m_curIndex];
        }

        iterator(raw_special_meshes* slot, u32 index) noexcept :
            const_iterator(slot, index) {}
    };

    const_iterator begin() const noexcept
    {
        return const_iterator(this, 0);
    }

    iterator begin() noexcept
    {
        return iterator(this, 0);
    }

    const_iterator end() const noexcept
    {
        return const_iterator(this, count);
    }

    iterator end() noexcept
    {
        return iterator(this, count);
    }

    const_wrapper operator[](std::size_t i) const noexcept
    {
        return const_wrapper(types[i].get(),
            *meshCounts[i], meshes[i].get());
    }

    wrapper operator[](std::size_t i) noexcept
    {
        return wrapper(types[i].get(),
            *meshCounts[i], meshes[i].get());
    }
};

HL_STATIC_ASSERT_SIZE(raw_special_meshes, 16);

struct raw_mesh_group
{
    raw_mesh_slot opaq;
    raw_mesh_slot trans;
    raw_mesh_slot punch;
    raw_special_meshes special;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(opaq);
        hl::endian_swap<swapOffsets>(trans);
        hl::endian_swap<swapOffsets>(punch);
        hl::endian_swap<swapOffsets>(special);
    }

    inline const char* name() const noexcept
    {
        return reinterpret_cast<const char*>(this + 1);
    }

    inline char* name() noexcept
    {
        return reinterpret_cast<char*>(this + 1);
    }
};

HL_STATIC_ASSERT_SIZE(raw_mesh_group, 0x28);

struct raw_node
{
    s32 parentIndex;
    off32<char> name;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(parentIndex);
        hl::endian_swap<swapOffsets>(name);
    }
};

HL_STATIC_ASSERT_SIZE(raw_node, 8);

enum class raw_terrain_model_flags : u32
{
    none = 0,

    /**
        @brief This terrain model is used by one or
        more .terrain-instanceinfo files.
    */
    is_instanced = 1
};

struct raw_terrain_model_v5
{
    arr32<off32<raw_mesh_group>> meshGroups;
    off32<char> name;

    /**
        @brief See hl::hh::mirage::raw_terrain_model_flags.
        
        NOTE: This value is NOT present in Sonic Unleashed!!!
        It is only present in "revision 2" of the v5 format, used from
        Sonic Generations onwards. Attempting to use it in data from any
        lower revision will cause your code to read unrelated memory!!!

        Use is_revision2() to automagically determine if this value is available.
    */
    u32 rev2_flags;

    HL_API bool is_revision2() const;

    template<bool swapOffsets = true>
    void endian_swap(bool isRevision2) noexcept
    {
        hl::endian_swap<swapOffsets>(meshGroups);
        hl::endian_swap<swapOffsets>(name);
        
        if (isRevision2)
        {
            hl::endian_swap(rev2_flags);
        }
    }

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        endian_swap<swapOffsets>(is_revision2());
    }

    HL_API void fix();
};

HL_STATIC_ASSERT_SIZE(raw_terrain_model_v5, 16);

struct raw_skeletal_model_v2
{
    raw_mesh_slot meshes;
    arr32<void> unknown1; // TODO
    arr32<void> unknown2; // TODO
    u32 nodeCount;
    off32<off32<raw_node>> nodes;
    off32<matrix4x4> nodeMatrices;
    off32<aabb> bounds;
    off32<void> unknown3; // TODO: Seems to point to the offset table???

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(meshes);
        hl::endian_swap<swapOffsets>(unknown1);
        hl::endian_swap<swapOffsets>(unknown2);
        hl::endian_swap(nodeCount);
        hl::endian_swap<swapOffsets>(nodes);
        hl::endian_swap<swapOffsets>(nodeMatrices);
        hl::endian_swap<swapOffsets>(bounds);
        hl::endian_swap<swapOffsets>(unknown3);
    }

    HL_API void fix();
};

HL_STATIC_ASSERT_SIZE(raw_skeletal_model_v2, 0x2C);

struct raw_skeletal_model_v4
{
    arr32<off32<raw_mesh_group>> meshGroups;
    arr32<void> unknown1; // TODO
    arr32<void> unknown2; // TODO
    arr32<void> unknown3; // TODO
    u32 nodeCount;
    off32<off32<raw_node>> nodes;
    off32<matrix4x4> nodeMatrices;
    off32<aabb> bounds;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(meshGroups);
        hl::endian_swap<swapOffsets>(unknown1);
        hl::endian_swap<swapOffsets>(unknown2);
        hl::endian_swap<swapOffsets>(unknown3);
        hl::endian_swap(nodeCount);
        hl::endian_swap<swapOffsets>(nodes);
        hl::endian_swap<swapOffsets>(nodeMatrices);
        hl::endian_swap<swapOffsets>(bounds);
    }

    // TODO: Fix function.
};

HL_STATIC_ASSERT_SIZE(raw_skeletal_model_v4, 0x30);

struct raw_skeletal_model_v5
{
    arr32<off32<raw_mesh_group>> meshGroups;
    arr32<void> unknown1; // TODO
    u32 nodeCount;
    off32<off32<raw_node>> nodes;
    off32<matrix4x4> nodeMatrices;
    off32<aabb> bounds;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(meshGroups);
        hl::endian_swap<swapOffsets>(unknown1);
        hl::endian_swap(nodeCount);
        hl::endian_swap<swapOffsets>(nodes);
        hl::endian_swap<swapOffsets>(nodeMatrices);
        hl::endian_swap<swapOffsets>(bounds);
    }

    HL_API void fix();
};

HL_STATIC_ASSERT_SIZE(raw_skeletal_model_v5, 0x20);

struct texture_unit
{
    std::string name;
    u8 index;

    HL_API void write(writer& writer) const;

    texture_unit() = default;
    texture_unit(const char* name, u8 index = 0) :
        name(name), index(index) {}

    texture_unit(const std::string& name, u8 index = 0) :
        name(name), index(index) {}

    texture_unit(std::string&& name, u8 index = 0) :
        name(std::move(name)), index(index) {}

    HL_API texture_unit(const raw_texture_unit& rawTexUnit);
};

struct mesh
{
    std::string materialName;
    std::vector<u16> faces;
    std::vector<raw_vertex_element> vertexElements;
    std::unique_ptr<u8[]> vertices;
    u32 vertexCount;
    u32 vertexSize;
    std::vector<u8> boneNodeIndices;
    std::vector<texture_unit> textureUnits;

    HL_API hl::mesh& add_to_node(hl::node& node,
        raw_topology_type topType = raw_topology_type::triangle_strip,
        const std::vector<mirage::node>* hhNodes = nullptr,
        bool includeLibGensTags = true,
        const char* libGensLayerName = nullptr) const;

    HL_API void write(writer& writer) const;

    mesh() = default;
    HL_API mesh(const raw_mesh& rawMesh);
};

struct mesh_slot : public std::vector<mesh>
{
    HL_API void get_unique_material_names(std::unordered_set<std::string>& uniqueMatNames) const;

    HL_API void add_to_node(hl::node& node,
        raw_topology_type topType = raw_topology_type::triangle_strip,
        const std::vector<mirage::node>* hhNodes = nullptr,
        bool includeLibGensTags = true,
        const char* libGensLayerName = nullptr) const;

    HL_API void write(writer& writer) const;

    mesh_slot() = default;
    HL_API mesh_slot(const raw_mesh_slot& rawSlot);
};

struct special_mesh_slot : public mesh_slot
{
    std::string type;

    inline void add_to_node(hl::node& node,
        raw_topology_type topType = raw_topology_type::triangle_strip,
        const std::vector<mirage::node>* hhNodes = nullptr,
        bool includeLibGensTags = true) const
    {
        mesh_slot::add_to_node(node, topType,
            hhNodes, includeLibGensTags, type.c_str());
    }

    // TODO: Write function!!!

    special_mesh_slot(const char* type) : type(type) {}
    special_mesh_slot(const std::string& type) : type(type) {}
    special_mesh_slot(std::string&& type) : type(std::move(type)) {}
    HL_API special_mesh_slot(const raw_special_meshes::const_wrapper& rawSpecialSlot);
};

struct special_meshes : public std::vector<special_mesh_slot>
{
    special_meshes() = default;
    HL_API special_meshes(const raw_special_meshes& rawSpecial);
};

struct mesh_group
{
    std::string name;
    mesh_slot opaq;
    mesh_slot trans;
    mesh_slot punch;
    special_meshes special;

    HL_API void get_unique_material_names(std::unordered_set<std::string>& uniqueMatNames) const;

    HL_API void add_to_node(hl::node& node,
        raw_topology_type topType = raw_topology_type::triangle_strip,
        const std::vector<mirage::node>* hhNodes = nullptr,
        bool includeLibGensTags = true) const;

    HL_API void write(writer& writer, u32 revision = 2) const;

    mesh_group() = default;
    mesh_group(const char* name) : name(name) {}
    mesh_group(const std::string& name) : name(name) {}
    mesh_group(std::string&& name) : name(std::move(name)) {}

    HL_API mesh_group(const raw_mesh_group& rawGroup);
    HL_API mesh_group(const raw_mesh_slot& rawSlot);
};

struct node
{
    std::string name;
    std::vector<sample_chunk::property> scaParams;
    matrix4x4 matrix = matrix4x4::identity;
    long parentIndex = -1;

    HL_API hl::node& add_to_node(hl::node& parentNode,
        const std::vector<node>* nodes = nullptr,
        bool addAsBone = false) const;

    HL_API void parse_sample_chunk_params(
        const sample_chunk::raw_node& rawNodePrmsNode);

    HL_API void write_sample_chunk_params(u32 nodeIndex, writer& writer) const;

    HL_API void write(writer& writer) const;

    node() = default;
    
    node(const char* name, long parentIndex = -1) :
        name(name), parentIndex(parentIndex) {}

    node(const std::string& name, long parentIndex = -1) :
        name(name), parentIndex(parentIndex) {}

    node(std::string&& name, long parentIndex = -1) :
        name(std::move(name)), parentIndex(parentIndex) {}

    node(const raw_node& rawNode, const matrix4x4& matrix) :
        name(rawNode.name.get()), parentIndex(rawNode.parentIndex),
        matrix(matrix) {}
};

class model
{
protected:
    HL_API bool in_has_per_node_params(std::size_t nodeCount,
        const node* nodes) const noexcept;

    HL_API void in_parse_mesh_groups(const arr32<off32<raw_mesh_group>>& rawMeshGroups);

    HL_API void in_parse_sample_chunk_nodes(const void* rawData,
        std::size_t nodeCount, node* nodes);

    HL_API void in_write_sample_chunk_nodes(std::size_t nodeCount,
        const node* nodes, writer& writer) const;

    model() = default;

public:
    using const_iterator = std::vector<mesh_group>::const_iterator;
    using iterator = std::vector<mesh_group>::iterator;

    std::vector<mesh_group> meshGroups;
    std::vector<sample_chunk::property> params;

    inline bool has_per_model_params() const noexcept
    {
        return !params.empty();
    }

    HL_API const sample_chunk::property* get_param(const char* name) const;

    inline const sample_chunk::property* get_param(const std::string& name) const
    {
        return get_param(name.c_str());
    }

    inline sample_chunk::property* get_param(const char* name)
    {
        return const_cast<sample_chunk::property*>(const_cast<
            const model*>(this)->get_param(name));
    }

    inline sample_chunk::property* get_param(const std::string& name)
    {
        return const_cast<sample_chunk::property*>(const_cast<
            const model*>(this)->get_param(name));
    }

    HL_API raw_topology_type get_topology_type() const;

    HL_API void get_unique_material_names(std::unordered_set<std::string>& uniqueMatNames) const;

    HL_API std::unordered_set<std::string> get_unique_material_names() const;

    HL_API void import_materials(const nchar* materialDir, scene& scene,
        bool merge = true, bool includeLibGensTags = true);

    inline void import_materials(const nstring& materialDir, scene& scene,
        bool merge = true, bool includeLibGensTags = true)
    {
        import_materials(materialDir.c_str(), scene, merge, includeLibGensTags);
    }

    inline const_iterator begin() const noexcept
    {
        return meshGroups.begin();
    }

    inline iterator begin() noexcept
    {
        return meshGroups.begin();
    }

    inline const_iterator end() const noexcept
    {
        return meshGroups.end();
    }

    inline iterator end() noexcept
    {
        return meshGroups.end();
    }

    inline const mesh_group& operator[](std::size_t i) const noexcept
    {
        return meshGroups[i];
    }

    inline mesh_group& operator[](std::size_t i) noexcept
    {
        return meshGroups[i];
    }
};

class terrain_model : public model
{
    HL_API header_type in_get_default_header_type() const;

public:
    node rootNode;
    bool isInstanced;

    inline constexpr static nchar ext[] = HL_NTEXT(".terrain-model");

    HL_API static void fix(void* rawData);

    bool has_per_node_params() const noexcept
    {
        return in_has_per_node_params(1, &rootNode);
    }

    bool has_params() const noexcept
    {
        return (has_per_model_params() || has_per_node_params());
    }

    HL_API void add_to_node(hl::node& parentNode, bool includeLibGensTags = true) const;

    void add_to_scene(scene& scene, bool includeLibGensTags = true) const
    {
        add_to_node(scene.root_node(), includeLibGensTags);
    }

    HL_API void parse(const raw_terrain_model_v5& rawMdl);
    HL_API void parse(const void* rawData);

    HL_API void load(const nchar* filePath);

    inline void load(const nstring& filePath)
    {
        load(filePath);
    }

    HL_API void write(writer& writer, header_type headerType,
        u32 version = 5, u32 revision = 2, const char* fileName = nullptr) const;

    inline void write(writer& writer, header_type headerType,
        u32 version, u32 revision, const std::string& fileName) const
    {
        write(writer, headerType, version, revision, fileName.c_str());
    }

    HL_API void save(stream& stream, header_type headerType,
        u32 version = 5, u32 revision = 2, const char* fileName = nullptr) const;

    inline void save(stream& stream, header_type headerType,
        u32 version, u32 revision, const std::string& fileName) const
    {
        save(stream, headerType, version, revision, fileName.c_str());
    }

    HL_API void save(const nchar* filePath, header_type headerType,
        u32 version, u32 revision) const;

    inline void save(const nstring& filePath, header_type headerType,
        u32 version, u32 revision) const
    {
        save(filePath.c_str(), headerType, version, revision);
    }

    HL_API void save(const nchar* filePath, header_type headerType,
        u32 version = 5) const;

    inline void save(const nstring& filePath, header_type headerType,
        u32 version = 5) const
    {
        save(filePath.c_str(), headerType, version);
    }

    HL_API void save(stream& stream) const;
    HL_API void save(const nchar* filePath) const;

    inline void save(const nstring& filePath) const
    {
        save(filePath.c_str());
    }

    terrain_model() = default;
    terrain_model(const void* rawData)
    {
        parse(rawData);
    }

    terrain_model(const nchar* filePath)
    {
        load(filePath);
    }

    inline terrain_model(const nstring& filePath) :
        terrain_model(filePath.c_str()) {}
};

class skeletal_model : public model
{
    HL_API header_type in_get_default_header_type() const;

public:
    // TODO: unknown1
    // TODO: unknown2
    // TODO: unknown3
    std::vector<node> nodes;
    // TODO: Should bounds be a field, or just computed automatically at write time?
    // TODO: the other unknown3

    inline constexpr static nchar ext[] = HL_NTEXT(".model");

    HL_API static void fix(void* rawData);

    HL_API bool has_per_node_params() const noexcept;

    bool has_params() const noexcept
    {
        return (has_per_model_params() || has_per_node_params());
    }

    HL_API void add_to_node(hl::node& parentNode, bool includeLibGensTags = true) const;

    inline void add_to_scene(scene& scene, bool includeLibGensTags = true) const
    {
        add_to_node(scene.root_node(), includeLibGensTags);
    }

    HL_API void parse(const raw_skeletal_model_v2& rawMdl);
    HL_API void parse(const raw_skeletal_model_v4& rawMdl);
    HL_API void parse(const raw_skeletal_model_v5& rawMdl);
    HL_API void parse(const void* rawData);

    HL_API void load(const nchar* filePath);

    inline void load(const nstring& filePath)
    {
        load(filePath);
    }

    HL_API void write(writer& writer, header_type headerType,
        u32 version = 5, const char* fileName = nullptr) const;

    inline void write(writer& writer, header_type headerType,
        u32 version, const std::string& fileName) const
    {
        write(writer, headerType, version, fileName.c_str());
    }

    HL_API void save(stream& stream, header_type headerType,
        u32 version = 5, const char* fileName = nullptr) const;

    inline void save(stream& stream, header_type headerType,
        u32 version, const std::string& fileName) const
    {
        save(stream, headerType, version, fileName.c_str());
    }

    HL_API void save(const nchar* filePath, header_type headerType, u32 version = 5) const;

    inline void save(const nstring& filePath, header_type headerType, u32 version = 5) const
    {
        save(filePath.c_str(), headerType, version);
    }

    HL_API void save(stream& stream) const;
    HL_API void save(const nchar* filePath) const;

    inline void save(const nstring& filePath) const
    {
        save(filePath.c_str());
    }

    skeletal_model() = default;
    skeletal_model(const void* rawData)
    {
        parse(rawData);
    }

    skeletal_model(const nchar* filePath)
    {
        load(filePath);
    }

    inline skeletal_model(const nstring& filePath) :
        skeletal_model(filePath.c_str()) {}
};
} // mirage
} // hh
} // hl
#endif
