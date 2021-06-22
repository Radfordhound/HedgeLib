#ifndef HL_SCENE_H_INCLUDED
#define HL_SCENE_H_INCLUDED
#include "hl_math.h"
#include "hl_blob.h"
#include "io/hl_stream.h"
#include <array>

namespace hl
{
class scene;
class node;
class bone;

// TODO: Move this to its own header.
class tagged_string
{
    std::string m_str;
    std::size_t m_tagsPos = 0;

    std::size_t in_get_tags_pos() const noexcept
    {
        const std::size_t tagsPos = m_str.find_first_of('@', 0);
        return (tagsPos != std::string::npos) ? tagsPos : m_str.size();
    }

    inline void in_update() noexcept
    {
        m_tagsPos = in_get_tags_pos();
    }

public:
    inline std::size_t size() const noexcept
    {
        return m_str.size();
    }

    inline std::size_t name_size() const noexcept
    {
        return m_tagsPos;
    }

    inline std::size_t tags_size() const noexcept
    {
        return (m_str.size() - m_tagsPos);
    }

    inline const char* data() const noexcept
    {
        return m_str.data();
    }

    inline const char* c_str() const noexcept
    {
        return m_str.c_str();
    }

    std::string name() const
    {
        return std::string(m_str.begin(), m_str.begin() + m_tagsPos);
    }

    inline bool has_tags() const noexcept
    {
        return (m_tagsPos < m_str.size());
    }

    inline const char* tags() const noexcept
    {
        return (m_str.c_str() + m_tagsPos);
    }

    bool name_equals(const char* name) const
    {
        return (m_str.compare(0, m_tagsPos, name) == 0);
    }

    bool name_equals(const std::string& name) const
    {
        return (m_str.compare(0, m_tagsPos, name) == 0);
    }

    inline operator const std::string&() const noexcept
    {
        return m_str;
    }

    bool operator==(const char* str) const
    {
        return (m_str == str);
    }

    bool operator==(const std::string& str) const
    {
        return (m_str == str);
    }

    tagged_string& operator=(const char* str)
    {
        m_str = str;
        in_update();
        return *this;
    }

    tagged_string& operator=(const std::string& str)
    {
        m_str = str;
        in_update();
        return *this;
    }

    tagged_string& operator=(std::string&& other) noexcept
    {
        m_str = std::move(other);
        in_update();
        return *this;
    }

    inline const char& operator[](std::size_t index) const noexcept
    {
        return m_str[index];
    }

    inline char& operator[](std::size_t index) noexcept
    {
        return m_str[index];
    }

    // TODO: Add += operator and assign/append/etc. functions

    tagged_string() = default;

    tagged_string(const char* str) :
        m_str(str),
        m_tagsPos(in_get_tags_pos()) {}

    tagged_string(const std::string& str) :
        m_str(str),
        m_tagsPos(in_get_tags_pos()) {}

    tagged_string(std::string&& str) noexcept :
        m_str(std::move(str)),
        m_tagsPos(in_get_tags_pos()) {}
};

struct texture
{
    std::string name;
    std::string utf8FilePath;

    texture(const char* name, const char* utf8FilePath) :
        name(name),
        utf8FilePath(utf8FilePath) {}

    texture(const std::string& name, const char* utf8FilePath) :
        name(name),
        utf8FilePath(utf8FilePath) {}

    texture(std::string&& name, const char* utf8FilePath) :
        name(std::move(name)),
        utf8FilePath(utf8FilePath) {}

    texture(const char* name, const std::string& utf8FilePath) :
        name(name),
        utf8FilePath(utf8FilePath) {}

    texture(const std::string& name, const std::string& utf8FilePath) :
        name(name),
        utf8FilePath(utf8FilePath) {}

    texture(std::string&& name, const std::string& utf8FilePath) :
        name(std::move(name)),
        utf8FilePath(utf8FilePath) {}

    texture(const char* name, std::string&& utf8FilePath) :
        name(name),
        utf8FilePath(std::move(utf8FilePath)) {}

    texture(const std::string& name, std::string&& utf8FilePath) :
        name(name),
        utf8FilePath(std::move(utf8FilePath)) {}

    texture(std::string&& name, std::string&& utf8FilePath) noexcept :
        name(std::move(name)),
        utf8FilePath(std::move(utf8FilePath)) {}
};

enum class map_slot_type
{
    unknown = 0,
    diffuse,
    specular,
    ambient,
    normal,
    reflection,
    displacement
};

struct texture_map
{
    map_slot_type slot;
    texture* tex;

    texture_map(map_slot_type slot, texture& tex) noexcept :
        slot(slot), tex(&tex) {}
};

class material
{
    friend hl::scene;

    hl::scene& m_sceneRef;

    material(hl::scene& scene, const char* name) :
        m_sceneRef(scene),
        name(name) {}

    material(hl::scene& scene, const std::string& name) :
        m_sceneRef(scene),
        name(name) {}

    material(hl::scene& scene, std::string&& name) noexcept :
        m_sceneRef(scene),
        name(std::move(name)) {}

public:
    tagged_string name;
    vec3 diffuseColor = vec3::one;
    vec3 specularColor = vec3::one;
    vec3 ambientColor = vec3::zero;
    vec3 emissiveColor = vec3::zero;
    std::vector<texture_map> textures;

    inline const hl::scene& scene() const noexcept
    {
        return m_sceneRef;
    }

    inline hl::scene& scene() noexcept
    {
        return m_sceneRef;
    }
};

enum class node_attribute_type
{
    mesh
};

class node_attribute
{
protected:
    node_attribute_type m_type;

    node_attribute(node_attribute_type type, const char* name) :
        m_type(type), name(name) {}

    node_attribute(node_attribute_type type, const std::string& name) :
        m_type(type), name(name) {}

    node_attribute(node_attribute_type type, std::string&& name) :
        m_type(type), name(std::move(name)) {}

public:
    std::string name;

    inline node_attribute_type type() const noexcept
    {
        return m_type;
    }

    virtual ~node_attribute() = 0;
};

using bone_ref = std::array<bone*, 4>;

struct mesh : public node_attribute
{
    constexpr static std::size_t max_uv_channel_count = 4;

    std::vector<vec4> vertices;
    std::vector<vec2> uvs[max_uv_channel_count];
    std::vector<vec4> normals;
    std::vector<vec4> tangents;
    std::vector<vec4> binormals;
    std::vector<vec4> colors;
    std::vector<vec4> boneWeights;
    std::vector<bone_ref> boneRefs;
    std::vector<unsigned short> faces;
    hl::material* material = nullptr;

    ~mesh() override {}

    mesh(const char* name) : node_attribute(
        node_attribute_type::mesh, name) {}

    mesh(const std::string& name) : node_attribute(
        node_attribute_type::mesh, name) {}

    mesh(std::string&& name) : node_attribute(
        node_attribute_type::mesh, std::move(name)) {}
};

enum class node_type
{
    node,
    bone
};

class node
{
protected:
    friend hl::scene;

    node_type m_type;
    hl::scene& m_sceneRef;
    node* m_parent;
    std::vector<node*> m_children;

    node(hl::scene& scene, const char* name, node* parent = nullptr,
        node_type type = node_type::node) :
        m_type(type),
        m_sceneRef(scene),
        m_parent(parent),
        name(name) {}

    node(hl::scene& scene, const std::string& name, node* parent = nullptr,
        node_type type = node_type::node) :
        m_type(type),
        m_sceneRef(scene),
        m_parent(parent),
        name(name) {}

    node(hl::scene& scene, std::string&& name, node* parent = nullptr,
        node_type type = node_type::node) noexcept :
        m_type(type),
        m_sceneRef(scene),
        m_parent(parent),
        name(std::move(name)) {}

public:
    std::string name;
    hl::vec3 pos = hl::vec3::zero;
    hl::quat rot = hl::quat::identity;
    hl::vec3 scale = hl::vec3::one;
    std::vector<std::unique_ptr<node_attribute>> attributes;

    virtual ~node() {}

    inline node_type type() const noexcept
    {
        return m_type;
    }

    inline const hl::scene& scene() const noexcept
    {
        return m_sceneRef;
    }

    inline hl::scene& scene() noexcept
    {
        return m_sceneRef;
    }

    inline const node* parent() const noexcept
    {
        return m_parent;
    }

    inline node* parent() noexcept
    {
        return m_parent;
    }

    inline const std::vector<node*>& children() const noexcept
    {
        return m_children;
    }

    HL_API const node* find_child(const char* name, bool recursive = true) const;

    HL_API const node* find_child(const std::string& name, bool recursive = true) const;

    template<typename T>
    inline const T* find_child(const char* name, bool recursive = true) const
    {
        return static_cast<const T*>(find_child(name, recursive));
    }

    template<typename T>
    inline const T* find_child(const std::string& name, bool recursive = true) const
    {
        return static_cast<const T*>(find_child(name, recursive));
    }

    template<typename T = node>
    inline T* find_child(const char* name, bool recursive = true)
    {
        return const_cast<T*>(const_cast<const node*>(
            this)->find_child<T>(name, recursive));
    }

    template<typename T = node>
    inline T* find_child(const std::string& name, bool recursive = true)
    {
        return const_cast<T*>(const_cast<const node*>(
            this)->find_child<T>(name, recursive));
    }

    template<typename T = node>
    T& add_child(const char* name);

    template<typename T = node>
    T& add_child(const std::string& name);
    
    template<typename T = node>
    T& add_child(std::string&& name);

    HL_API bool has_attributes_of_type(node_attribute_type type) const;
};

class bone : public node
{
    friend hl::scene;

    bone(hl::scene& scene, const char* name, node* parent = nullptr) :
        node(scene, name, parent, node_type::bone) {}

    bone(hl::scene& scene, const std::string& name, node* parent = nullptr) :
        node(scene, name, parent, node_type::bone) {}

    bone(hl::scene& scene, std::string&& name, node* parent = nullptr,
        node_type type = node_type::node) noexcept :
        node(scene, std::move(name), parent, node_type::bone) {}

public:
    ~bone() override {}
};

enum class fbx_type
{
    binary = 0,
    ascii
};

enum class fbx_version
{
    fbx_53_mb55,
    fbx_60,
    fbx_2005,
    fbx_2006_02,
    fbx_2006_08,
    fbx_2006_11,
    fbx_2009_00,
    fbx_2009_00_V7,
    fbx_2010,
    fbx_2011,
    fbx_2012,
    fbx_2013,
    fbx_2014,
    fbx_2016,
    fbx_2018,
    fbx_2019,
    fbx_2020
};

constexpr fbx_version fbx_default_version = fbx_version::fbx_2020;

class scene
{
    std::vector<std::unique_ptr<node>> m_nodes;
    std::vector<std::unique_ptr<material>> m_materials;
    std::vector<std::unique_ptr<texture>> m_textures;
    node* m_rootNodePtr;

    HL_API node* in_create_root_node();

public:
    std::string name;

    inline std::size_t node_count() const noexcept
    {
        return m_nodes.size();
    }

    inline const node& root_node() const noexcept
    {
        return *m_rootNodePtr;
    }

    inline node& root_node() noexcept
    {
        return *m_rootNodePtr;
    }

    inline const std::vector<std::unique_ptr<node>>& nodes() const noexcept
    {
        return m_nodes;
    }

    inline const std::vector<std::unique_ptr<material>>& materials() const noexcept
    {
        return m_materials;
    }

    inline const std::vector<std::unique_ptr<texture>>& textures() const noexcept
    {
        return m_textures;
    }

    template<typename T = node>
    inline const T& get_node(std::size_t index) const noexcept
    {
        return static_cast<const T&>(*m_nodes[index].get());
    }

    template<typename T = node>
    inline T& get_node(std::size_t index) noexcept
    {
        return static_cast<T&>(*m_nodes[index].get());
    }

    template<typename T = node>
    inline const T* find_node(const char* name, bool recursive = true) const
    {
        return root_node().find_child<T>(name, recursive);
    }

    template<typename T = node>
    inline const T* find_node(const std::string& name, bool recursive = true) const
    {
        return root_node().find_child<T>(name, recursive);
    }

    template<typename T = node>
    inline T* find_node(const char* name, bool recursive = true)
    {
        return root_node().find_child<T>(name, recursive);
    }

    template<typename T = node>
    inline T* find_node(const std::string& name, bool recursive = true)
    {
        return root_node().find_child<T>(name, recursive);
    }

    template<typename T = node>
    T& add_node(const char* name, node* parent = nullptr)
    {
        std::unique_ptr<T> newNode(new T(*this, name, parent));
        T* nodePtr = newNode.get();
        m_nodes.emplace_back(std::move(newNode));
        return *nodePtr;
    }

    template<typename T = node>
    T& add_node(const std::string& name, node* parent = nullptr)
    {
        std::unique_ptr<T> newNode(new T(*this, name, parent));
        T* nodePtr = newNode.get();
        m_nodes.emplace_back(std::move(newNode));
        return *nodePtr;
    }
    
    template<typename T = node>
    T& add_node(std::string&& name, node* parent = nullptr)
    {
        std::unique_ptr<T> newNode(new T(*this, std::move(name), parent));
        T* nodePtr = newNode.get();
        m_nodes.emplace_back(std::move(newNode));
        return *nodePtr;
    }

    HL_API const material* find_material(const char* name) const;
    HL_API const material* find_material(const std::string& name) const;

    inline material* find_material(const char* name)
    {
        return const_cast<material*>(const_cast<const scene*>(
            this)->find_material(name));
    }

    inline material* find_material(const std::string& name)
    {
        return const_cast<material*>(const_cast<const scene*>(
            this)->find_material(name));
    }

    HL_API material& add_material(const char* name);
    HL_API material& add_material(const std::string& name);
    HL_API material& add_material(std::string&& name);

    HL_API const texture* find_texture(const char* name) const;
    HL_API const texture* find_texture(const std::string& name) const;

    inline texture* find_texture(const char* name)
    {
        return const_cast<texture*>(const_cast<const scene*>(
            this)->find_texture(name));
    }

    inline texture* find_texture(const std::string& name)
    {
        return const_cast<texture*>(const_cast<const scene*>(
            this)->find_texture(name));
    }

    HL_API texture& add_texture(const char* name, const char* utf8FilePath);
    HL_API texture& add_texture(const std::string& name, const char* utf8FilePath);
    HL_API texture& add_texture(std::string&& name, const char* utf8FilePath);

    HL_API texture& add_texture(const char* name, const std::string& utf8FilePath);
    HL_API texture& add_texture(const std::string& name, const std::string& utf8FilePath);
    HL_API texture& add_texture(std::string&& name, const std::string& utf8FilePath);

    HL_API texture& add_texture(const char* name, std::string&& utf8FilePath);
    HL_API texture& add_texture(const std::string& name, std::string&& utf8FilePath);
    HL_API texture& add_texture(std::string&& name, std::string&& utf8FilePath);

    HL_API void import_fbx(stream& stream);
    HL_API void import_fbx(const nchar* filePath);

    inline void import_fbx(const nstring& filePath)
    {
        import_fbx(filePath.c_str());
    }

    HL_API void export_fbx(stream& stream, fbx_type type = fbx_type::binary,
        fbx_version version = fbx_default_version) const;

    HL_API void export_fbx(const nchar* filePath, fbx_type type = fbx_type::binary,
        fbx_version version = fbx_default_version) const;

    inline void export_fbx(const nstring& filePath, fbx_type type = fbx_type::binary,
        fbx_version version = fbx_default_version) const
    {
        export_fbx(filePath.c_str(), type, version);
    }

    scene() : m_rootNodePtr(in_create_root_node()) {}
    scene(const char* name) : m_rootNodePtr(in_create_root_node()), name(name) {}
    scene(const std::string& name) : m_rootNodePtr(in_create_root_node()), name(name) {}
    scene(std::string&& name) : m_rootNodePtr(in_create_root_node()), name(std::move(name)) {}
};

template<typename T>
T& node::add_child(const char* name)
{
    T& child = m_sceneRef.add_node<T>(name, this);
    m_children.push_back(&child);
    return child;
}

template<typename T>
T& node::add_child(const std::string& name)
{
    T& child = m_sceneRef.add_node<T>(name, this);
    m_children.push_back(&child);
    return child;
}

template<typename T>
T& node::add_child(std::string&& name)
{
    T& child = m_sceneRef.add_node<T>(std::move(name), this);
    m_children.push_back(&child);
    return child;
}
} // hl
#endif
