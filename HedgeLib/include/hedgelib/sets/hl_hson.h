#ifndef HL_HSON_H_INCLUDED
#define HL_HSON_H_INCLUDED

#include "../hl_radix_tree.h"
#include "../hl_ordered_map.h"
#include "../hl_text.h"
#include "../hl_guid.h"
#include "../hl_math.h"
#include <optional>

namespace hl
{
class stream;

namespace internal
{
class in_rapidjson_writer;
} // internal

namespace hson
{
class object;
class project_metadata;
class project;

inline constexpr unsigned int max_supported_version = 1;

enum class parameter_type
{
    none = 0,
    boolean,
    signed_integer,
    unsigned_integer,
    floating,
    string,
    array,
    object,
};

class parameter
{
    friend object;
    friend project_metadata;
    friend project;

    parameter_type m_type = parameter_type::none;
    union
    {
        bool m_valBool;
        std::intmax_t m_valSignedInt;
        std::uintmax_t m_valUnsignedInt;
        double m_valFloating;
        std::string m_valString;
        std::vector<parameter> m_valArray;
        radix_tree<parameter> m_valObject;
    };

protected:
    static void in_write_all(internal::in_rapidjson_writer& writer,
        const radix_tree<parameter>& parameters);

    void in_write(internal::in_rapidjson_writer& writer) const;

private:
    void in_default_construct_value();

    void in_destruct_value() noexcept;

    void in_copy_construct_value(const parameter& other);

    void in_move_construct_value(parameter&& other) noexcept;

public:
    inline parameter_type type() const noexcept
    {
        return m_type;
    }

    inline bool value_bool() const noexcept
    {
        return m_valBool;
    }

    inline bool& value_bool() noexcept
    {
        return m_valBool;
    }

    inline std::intmax_t value_int() const noexcept
    {
        return m_valSignedInt;
    }

    inline std::intmax_t& value_int() noexcept
    {
        return m_valSignedInt;
    }

    inline std::uintmax_t value_uint() const noexcept
    {
        return m_valSignedInt;
    }

    inline std::uintmax_t& value_uint() noexcept
    {
        return m_valUnsignedInt;
    }

    inline double value_floating() const noexcept
    {
        return m_valFloating;
    }

    inline double& value_floating() noexcept
    {
        return m_valFloating;
    }

    inline const std::string& value_string() const noexcept
    {
        return m_valString;
    }

    inline std::string& value_string() noexcept
    {
        return m_valString;
    }

    inline const std::vector<parameter>& value_array() const noexcept
    {
        return m_valArray;
    }

    inline std::vector<parameter>& value_array() noexcept
    {
        return m_valArray;
    }

    inline const radix_tree<parameter>& value_object() const noexcept
    {
        return m_valObject;
    }

    inline radix_tree<parameter>& value_object() noexcept
    {
        return m_valObject;
    }

    HL_API std::intmax_t get_value_as_int() const;

    HL_API std::uintmax_t get_value_as_uint() const;

    HL_API double get_value_as_floating() const;

    HL_API parameter& operator=(const parameter& other);

    HL_API parameter& operator=(parameter&& other) noexcept;

    inline parameter() noexcept :
        m_valSignedInt(0) {}

    HL_API parameter(parameter_type type);

    HL_API parameter(const parameter& other);

    HL_API parameter(parameter&& other) noexcept;

    HL_API ~parameter();
};

class object
{
    friend project;

    template<typename T>
    struct in_inherited_val_ret_type;

    template<typename T, typename C>
    struct in_inherited_val_ret_type<std::optional<T> C::*>
    {
        using type = const T*;
    };

    template<const auto MemberPointer>
    auto in_get_inherited_value(const ordered_map<guid, object>& objects) const ->
        typename in_inherited_val_ret_type<decltype(MemberPointer)>::type
    {
        // If the value we want is specified for this object, just return that.
        if ((this->*MemberPointer).has_value())
        {
            return &(this->*MemberPointer).value();
        }

        // Otherwise, if this is an instance of another object, we
        // inherit the value from the instanced object's property.
        else if (has_instance())
        {
            const auto instancedObj = objects.get(instanceOf);
            if (instancedObj)
            {
                return instancedObj->in_get_inherited_value<
                    MemberPointer>(objects);
            }
        }

        // Otherwise, just return null.
        return nullptr;
    }

    const parameter* in_get_parameter_from_name(const char* name,
        const ordered_map<guid, object>& objects) const;

    const parameter* in_get_parameter_from_path(const char* firstNameSep,
        const char* fullPath, const ordered_map<guid, object>& objects) const;

    void in_write(internal::in_rapidjson_writer& writer) const;

public:
    static const std::string default_name;

    static inline constexpr guid default_parent_id = guid::zero();

    static inline constexpr guid default_instance_of = guid::zero();

    static inline constexpr vec3 default_position = vec3::zero();

    static inline constexpr quat default_rotation = quat::identity();

    static inline constexpr vec3 default_scale = vec3::one();

    static inline constexpr bool default_is_editor_visible = true;

    static inline constexpr bool default_is_excluded = false;

    std::optional<std::string> name;
    std::string type;
    std::optional<guid> parentID;
    guid instanceOf = default_instance_of;
    std::optional<vec3> position;
    std::optional<quat> rotation;
    std::optional<vec3> scale;
    std::optional<bool> isEditorVisible;
    std::optional<bool> isExcluded;
    radix_tree<parameter> parameters;
    radix_tree<parameter> customProperties;

    inline bool has_type() const noexcept
    {
        return !type.empty();
    }

    inline bool has_instance() const noexcept
    {
        return !instanceOf.empty();
    }

    HL_API bool has_inherited_parameters(
        const ordered_map<guid, object>& objects) const;

    HL_API const std::string* get_inherited_name(
        const ordered_map<guid, object>& objects) const;

    HL_API const std::string* get_inherited_type(
        const ordered_map<guid, object>& objects) const;

    HL_API const guid* get_inherited_parent_id(
        const ordered_map<guid, object>& objects) const;

    HL_API const vec3& get_local_position(
        const ordered_map<guid, object>& objects) const;

    HL_API const quat& get_local_rotation(
        const ordered_map<guid, object>& objects) const;

    HL_API const vec3& get_local_scale(
        const ordered_map<guid, object>& objects) const;

    HL_API matrix4x4A get_local_transform(
        const ordered_map<guid, object>& objects) const;

    HL_API matrix4x4A get_global_transform(
        const ordered_map<guid, object>& objects) const;

    HL_API const bool* get_inherited_is_editor_visible(
        const ordered_map<guid, object>& objects) const;

    HL_API const bool* get_inherited_is_excluded(
        const ordered_map<guid, object>& objects) const;

    HL_API const parameter* get_parameter(const char* path,
        const ordered_map<guid, object>& objects) const;

    inline const parameter* get_parameter(const std::string& path,
        const ordered_map<guid, object>& objects) const
    {
        return get_parameter(path.c_str(), objects);
    }

    inline parameter* get_parameter(const char* path,
        const ordered_map<guid, object>& objects)
    {
        return const_cast<parameter*>(const_cast<const object*>(
            this)->get_parameter(path, objects));
    }

    inline parameter* get_parameter(const std::string& path,
        const ordered_map<guid, object>& objects)
    {
        return get_parameter(path.c_str(), objects);
    }

    HL_API bool get_flattened_parameters(const ordered_map<guid, object>& objects,
        radix_tree<parameter>& flatParams, const char* rootPath = nullptr) const;

    inline bool get_flattened_parameters(const ordered_map<guid, object>& objects,
        radix_tree<parameter>& flatParams, const std::string& rootPath) const
    {
        return get_flattened_parameters(objects,
            flatParams, rootPath.c_str());
    }

    HL_API radix_tree<parameter> get_flattened_parameters(
        const ordered_map<guid, object>& objects, const char* rootPath = nullptr) const;

    inline radix_tree<parameter> get_flattened_parameters(
        const ordered_map<guid, object>& objects, const std::string& rootPath) const
    {
        return get_flattened_parameters(objects, rootPath.c_str());
    }

    bool has_inherited_parameters(const project& project) const;

    const std::string* get_inherited_name(const project& project) const;

    const std::string* get_inherited_type(const project& project) const;

    const guid* get_inherited_parent_id(const project& project) const;

    const vec3& get_local_position(const project& project) const;

    const quat& get_local_rotation(const project& project) const;

    const vec3& get_local_scale(const project& project) const;

    matrix4x4A get_local_transform(const project& project) const;

    matrix4x4A get_global_transform(const project& project) const;

    const bool* get_inherited_is_editor_visible(const project& project) const;

    const bool* get_inherited_is_excluded(const project& project) const;

    const parameter* get_parameter(const char* path,
        const project& project) const;

    const parameter* get_parameter(const std::string& path,
        const project& project) const;
    
    parameter* get_parameter(const char* path, const project& project);

    parameter* get_parameter(const std::string& path, const project& project);

    bool get_flattened_parameters(const project& project,
        radix_tree<parameter>& flatParams, const char* rootPath = nullptr) const;

    inline bool get_flattened_parameters(const project& project,
        radix_tree<parameter>& flatParams, const std::string& rootPath) const
    {
        return get_flattened_parameters(project, flatParams, rootPath.c_str());
    }

    radix_tree<parameter> get_flattened_parameters(
        const project& project, const char* rootPath = nullptr) const;

    inline radix_tree<parameter> get_flattened_parameters(
        const project& project, const std::string& rootPath) const
    {
        return get_flattened_parameters(project, rootPath.c_str());
    }
};

class project_metadata
{
    friend project;

protected:
    void in_write(internal::in_rapidjson_writer& writer) const;

public:
    std::string name;
    std::string author;
    std::string date; // TODO: Should this be a string?
    std::string version;
    std::string description;
    radix_tree<parameter> customProperties;

    [[nodiscard]] HL_API bool empty() const noexcept;

    HL_API void clear() noexcept;
};

class project
{
    void in_parse(const void* rawData, std::size_t rawDataSize);

    void in_read(stream& stream);

    void in_load(const nchar* filePath);

public:
    project_metadata metadata;
    ordered_map<guid, object> objects;
    radix_tree<parameter> customProperties;

    HL_API void clear() noexcept;

    HL_API void parse(const void* rawData, std::size_t rawDataSize);

    HL_API void read(stream& stream);

    HL_API void load(const nchar* filePath);

    inline void load(const nstring& filePath)
    {
        load(filePath.c_str());
    }

    HL_API void write(stream& stream) const;

    HL_API void save(const nchar* filePath) const;

    inline void save(const nstring& filePath) const
    {
        save(filePath.c_str());
    }

    project() noexcept(noexcept(
        typename ordered_map<guid, object>::ordered_map())) = default;

    HL_API project(const void* rawData, std::size_t rawDataSize);

    HL_API project(stream& stream);

    HL_API project(const nchar* filePath);

    inline project(const nstring& filePath) :
        project(filePath.c_str()) {}
};

inline bool object::has_inherited_parameters(const project& project) const
{
    return has_inherited_parameters(project.objects);
}

inline const std::string* object::get_inherited_name(const project& project) const
{
    return get_inherited_name(project.objects);
}

inline const std::string* object::get_inherited_type(const project& project) const
{
    return get_inherited_type(project.objects);
}

inline const guid* object::get_inherited_parent_id(const project& project) const
{
    return get_inherited_parent_id(project.objects);
}

inline const vec3& object::get_local_position(const project& project) const
{
    return get_local_position(project.objects);
}

inline const quat& object::get_local_rotation(const project& project) const
{
    return get_local_rotation(project.objects);
}

inline const vec3& object::get_local_scale(const project& project) const
{
    return get_local_scale(project.objects);
}

inline matrix4x4A object::get_local_transform(const project& project) const
{
    return get_local_transform(project.objects);
}

inline matrix4x4A object::get_global_transform(const project& project) const
{
    return get_global_transform(project.objects);
}

inline const bool* object::get_inherited_is_editor_visible(const project& project) const
{
    return get_inherited_is_editor_visible(project.objects);
}

inline const bool* object::get_inherited_is_excluded(const project& project) const
{
    return get_inherited_is_excluded(project.objects);
}

inline const parameter* object::get_parameter(
    const char* path, const project& project) const
{
    return get_parameter(path, project.objects);
}

inline const parameter* object::get_parameter(
    const std::string& path, const project& project) const
{
    return get_parameter(path.c_str(), project);
}

inline parameter* object::get_parameter(
    const char* path, const project& project)
{
    return get_parameter(path, project.objects);
}

inline parameter* object::get_parameter(
    const std::string& path, const project& project)
{
    return get_parameter(path.c_str(), project);
}

inline bool object::get_flattened_parameters(const project& project,
    radix_tree<parameter>& flatParams, const char* rootPath) const
{
    return get_flattened_parameters(project.objects, flatParams, rootPath);
}

inline radix_tree<parameter> object::get_flattened_parameters(
    const project& project, const char* rootPath) const
{
    return get_flattened_parameters(project.objects, rootPath);
}
} // hson
} // hl
#endif
