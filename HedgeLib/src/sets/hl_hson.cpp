#include "../io/hl_in_rapidjson.h"
#include "hedgelib/sets/hl_hson.h"
#include "hedgelib/io/hl_file.h"
#include "hedgelib/io/hl_mem_stream.h"
#include "hedgelib/hl_reflect.h"
#include <DirectXMath.h>

namespace hl
{
namespace hson
{
class in_project_json_handler
{
    enum class in_state
    {
        none,
        top_level_object,

        file_version_number,

        metadata_section,
        metadata,
        metadata_name,
        metadata_author,
        metadata_date,
        metadata_version,
        metadata_description,

        objects_section,
        objects,
        object,
        object_id,
        object_name,
        object_parent_id,
        object_instance_of,
        object_type,
        object_position_section,
        object_position,
        object_rotation_section,
        object_rotation,
        object_scale_section,
        object_scale,
        object_is_editor_visible,
        object_is_excluded,
        object_parameters_section,
        object_parameters,
    };

    project*                    m_project;
    guid                        m_curObjID = guid::zero();
    object                      m_curObj;
    std::vector<parameter*>     m_paramStack;
    unsigned int                m_fileVersionNumber = 0;
    unsigned int                m_curVecElemIndex = 0;
    in_state                    m_curState = in_state::none;

    bool in_add_param_int(std::intmax_t val)
    {
        // If the current parameter is empty, replace it with a new value and pop it.
        auto& curParam = *m_paramStack.back();
        if (curParam.type() == parameter_type::none)
        {
            curParam = parameter(parameter_type::signed_integer);
            curParam.value_int() = val;
            m_paramStack.pop_back();
        }

        // If the current parameter is an array, add a new value to the existing array.
        else if (curParam.type() == parameter_type::array)
        {
            auto& newParam = curParam.value_array().emplace_back(
                parameter_type::signed_integer);

            newParam.value_int() = val;
        }

        // Otherwise, error.
        else
        {
            return false;
        }

        return true;
    }

    bool in_add_param_uint(std::uintmax_t val)
    {
        // Error if no parameters are in the stack.
        if (m_paramStack.empty())
        {
            return false;
        }

        // If the current parameter is empty, replace it with a new value and pop it.
        auto& curParam = *m_paramStack.back();
        if (curParam.type() == parameter_type::none)
        {
            curParam = parameter(parameter_type::unsigned_integer);
            curParam.value_uint() = val;
            m_paramStack.pop_back();
        }

        // If the current parameter is an array, add a new value to the existing array.
        else if (curParam.type() == parameter_type::array)
        {
            auto& newParam = curParam.value_array().emplace_back(
                parameter_type::unsigned_integer);

            newParam.value_uint() = val;
        }

        // Otherwise, error.
        else
        {
            return false;
        }

        return true;
    }

    bool in_insert_parameter(const char* key, radix_tree<parameter>& parameters)
    {
        // Determine what parameter tree to add this parameter to.
        radix_tree<parameter>* paramTree;
        if (m_paramStack.empty())
        {
            // If no parameters are in the stack, add parameter to the current object.
            paramTree = &parameters;
        }
        else if (m_paramStack.back()->type() == parameter_type::object)
        {
            // If an object parameter is the last in the stack, add parameter to that object.
            paramTree = &m_paramStack.back()->value_object();
        }
        else
        {
            // Otherwise, we've run into an error state.
            return false;
        }

        // Add parameter to parameter tree and push onto stack.
        m_paramStack.push_back(&paramTree->insert(key).first->second);
        return true;
    }

public:
    using Ch        = char;
    using SizeType  = rapidjson::SizeType;

    constexpr static unsigned int max_supported_version = 1;

    bool Null();
    bool Bool(bool val);
    bool Int(int val);
    bool Uint(unsigned int val);
    bool Int64(std::int64_t val);
    bool Uint64(std::uint64_t val);
    bool Double(double val);
    bool RawNumber(const Ch* str, SizeType length, bool copy);
    bool String(const Ch* str, SizeType length, bool copy);
    bool StartObject();
    bool Key(const Ch* str, SizeType length, bool copy);
    bool EndObject(SizeType memberCount);
    bool StartArray();
    bool EndArray(SizeType elementCount);

    in_project_json_handler(project& proj) noexcept :
        m_project(&proj) {}
};

bool in_project_json_handler::Null()
{
    return false;
}

bool in_project_json_handler::Bool(bool val)
{
    // Handle parameters.
    if (!m_paramStack.empty())
    {
        // If the current parameter is empty, replace it with a new value and pop the stack.
        auto& curParam = *m_paramStack.back();
        if (curParam.type() == parameter_type::none)
        {
            curParam = parameter(parameter_type::boolean);
            curParam.value_bool() = val;
            m_paramStack.pop_back();
        }

        // If the current parameter is an array, add a new value to the existing array.
        else if (curParam.type() == parameter_type::array)
        {
            auto& newParam = curParam.value_array().emplace_back(
                parameter_type::boolean);

            newParam.value_bool() = val;
        }

        // Otherwise, error.
        else
        {
            return false;
        }

        return true;
    }

    switch (m_curState)
    {
    case in_state::object_is_editor_visible:
        m_curObj.isEditorVisible = val;
        m_curState = in_state::object;
        return true;

    case in_state::object_is_excluded:
        m_curObj.isExcluded = val;
        m_curState = in_state::object;
        return true;

    default:
        return false;
    }
}

bool in_project_json_handler::Int(int val)
{
    // Handle parameters.
    if (!m_paramStack.empty())
    {
        return in_add_param_int(val);
    }

    switch (m_curState)
    {
    case in_state::object_position:
        if (m_curVecElemIndex > 2) return false;
        m_curObj.position.value()[m_curVecElemIndex++] = static_cast<float>(val);
        return true;

    case in_state::object_rotation:
        if (m_curVecElemIndex > 3) return false;
        m_curObj.rotation.value()[m_curVecElemIndex++] = static_cast<float>(val);
        return true;

    case in_state::object_scale:
        if (m_curVecElemIndex > 2) return false;
        m_curObj.scale.value()[m_curVecElemIndex++] = static_cast<float>(val);
        return true;

    default:
        return false;
    }
}

bool in_project_json_handler::Uint(unsigned int val)
{
    // Handle parameters.
    if (!m_paramStack.empty())
    {
        return in_add_param_uint(val);
    }

    switch (m_curState)
    {
    case in_state::file_version_number:
        m_fileVersionNumber = val;
        m_curState = in_state::top_level_object;
        return true;

    case in_state::object_position:
        if (m_curVecElemIndex > 2) return false;
        m_curObj.position.value()[m_curVecElemIndex++] = static_cast<float>(val);
        return true;

    case in_state::object_rotation:
        if (m_curVecElemIndex > 3) return false;
        m_curObj.rotation.value()[m_curVecElemIndex++] = static_cast<float>(val);
        return true;

    case in_state::object_scale:
        if (m_curVecElemIndex > 2) return false;
        m_curObj.scale.value()[m_curVecElemIndex++] = static_cast<float>(val);
        return true;

    default:
        return false;
    }
}

bool in_project_json_handler::Int64(std::int64_t val)
{
    // Handle parameters.
    if (!m_paramStack.empty())
    {
        return in_add_param_int(val);
    }

    return false;
}

bool in_project_json_handler::Uint64(std::uint64_t val)
{
    // Handle parameters.
    if (!m_paramStack.empty())
    {
        return in_add_param_uint(val);
    }

    return false;
}

bool in_project_json_handler::Double(double val)
{
    // Handle parameters.
    if (!m_paramStack.empty())
    {
        // If the current parameter is empty, replace it with a new value and pop it.
        auto& curParam = *m_paramStack.back();
        if (curParam.type() == parameter_type::none)
        {
            curParam = parameter(parameter_type::floating);
            curParam.value_floating() = val;
            m_paramStack.pop_back();
        }

        // If the current parameter is an array, add a new value to the existing array.
        else if (curParam.type() == parameter_type::array)
        {
            auto& newParam = curParam.value_array().emplace_back(
                parameter_type::floating);

            newParam.value_floating() = val;
        }

        // Otherwise, error.
        else
        {
            return false;
        }

        return true;
    }

    switch (m_curState)
    {
    case in_state::object_position:
        if (m_curVecElemIndex > 2) return false;
        m_curObj.position.value()[m_curVecElemIndex++] = static_cast<float>(val);
        return true;

    case in_state::object_rotation:
        if (m_curVecElemIndex > 3) return false;
        m_curObj.rotation.value()[m_curVecElemIndex++] = static_cast<float>(val);
        return true;

    case in_state::object_scale:
        if (m_curVecElemIndex > 2) return false;
        m_curObj.scale.value()[m_curVecElemIndex++] = static_cast<float>(val);
        return true;

    default:
        return false;
    }
}

bool in_project_json_handler::RawNumber(
    const Ch* str, SizeType length, bool copy)
{
    return false;
}

bool in_project_json_handler::String(
    const Ch* str, SizeType length, bool copy)
{
    // Handle parameters.
    if (!m_paramStack.empty())
    {
        // If the current parameter is empty, replace it with a new value and pop it.
        auto& curParam = *m_paramStack.back();
        if (curParam.type() == parameter_type::none)
        {
            curParam = parameter(parameter_type::string);
            curParam.value_string().assign(str, length);
            m_paramStack.pop_back();
        }

        // If the current parameter is an array, add a new value to the existing array.
        else if (curParam.type() == parameter_type::array)
        {
            auto& newParam = curParam.value_array().emplace_back(
                parameter_type::string);

            newParam.value_string().assign(str, length);
        }

        // Otherwise, error.
        else
        {
            return false;
        }

        return true;
    }

    switch (m_curState)
    {
    case in_state::metadata_name:
        m_project->metadata.name.assign(str, length);
        m_curState = in_state::metadata;
        return true;

    case in_state::metadata_author:
        m_project->metadata.author.assign(str, length);
        m_curState = in_state::metadata;
        return true;
    
    case in_state::metadata_date:
        m_project->metadata.date.assign(str, length);
        m_curState = in_state::metadata;
        return true;
    
    case in_state::metadata_version:
        m_project->metadata.version.assign(str, length);
        m_curState = in_state::metadata;
        return true;
    
    case in_state::metadata_description:
        m_project->metadata.description.assign(str, length);
        m_curState = in_state::metadata;
        return true;

    case in_state::object_id:
        m_curObjID = guid(std::string_view(str, length));
        m_curState = in_state::object;
        return true;

    case in_state::object_name:
        m_curObj.name = std::string(str, length);
        m_curState = in_state::object;
        return true;

    case in_state::object_parent_id:
        m_curObj.parentID = guid(std::string_view(str, length));
        m_curState = in_state::object;
        return true;
    
    case in_state::object_instance_of:
        m_curObj.instanceOf = guid(std::string_view(str, length));
        m_curState = in_state::object;
        return true;
    
    case in_state::object_type:
        m_curObj.type.assign(str, length);
        m_curState = in_state::object;
        return true;

    default:
        return false;
    }
}

bool in_project_json_handler::StartObject()
{
    // Handle parameters.
    if (!m_paramStack.empty())
    {
        // If the current parameter is empty, replace it with a new object.
        auto& curParam = *m_paramStack.back();
        if (curParam.type() == parameter_type::none)
        {
            curParam = parameter(parameter_type::object);
        }

        // If the current parameter is an array, add a new object to the existing array.
        else if (curParam.type() == parameter_type::array)
        {
            m_paramStack.push_back(
                &curParam.value_array().emplace_back(parameter_type::object));
        }

        // Otherwise, error.
        else
        {
            return false;
        }

        return true;
    }

    switch (m_curState)
    {
    case in_state::none:
        m_curState = in_state::top_level_object;
        return true;

    case in_state::metadata_section:
        m_curState = in_state::metadata;
        return true;

    case in_state::objects:
        m_curState = in_state::object;
        return true;

    case in_state::object_parameters_section:
        m_curState = in_state::object_parameters;
        return true;

    default:
        return false;
    }
}

bool in_project_json_handler::Key(
    const Ch* str, SizeType length, bool copy)
{
    const std::string_view key(str, length);
    switch (m_curState)
    {
    case in_state::top_level_object:
        if (m_paramStack.empty())
        {
            if (key == "version")
            {
                m_curState = in_state::file_version_number;
                return true;
            }
            else if (key == "metadata")
            {
                m_curState = in_state::metadata_section;
                return true;
            }
            else if (key == "objects")
            {
                m_curState = in_state::objects_section;
                return true;
            }
        }

        // Custom properties.
        return in_insert_parameter(str, m_project->customProperties);

    case in_state::metadata:
        if (m_paramStack.empty())
        {
            if (key == "name")
            {
                m_curState = in_state::metadata_name;
                return true;
            }
            else if (key == "author")
            {
                m_curState = in_state::metadata_author;
                return true;
            }
            else if (key == "date")
            {
                m_curState = in_state::metadata_date;
                return true;
            }
            else if (key == "version")
            {
                m_curState = in_state::metadata_version;
                return true;
            }
            else if (key == "description")
            {
                m_curState = in_state::metadata_description;
                return true;
            }
        }

        // Custom properties.
        return in_insert_parameter(str, m_project->metadata.customProperties);

    case in_state::object:
        if (m_paramStack.empty())
        {
            if (key == "id")
            {
                m_curState = in_state::object_id;
                return true;
            }
            else if (key == "name")
            {
                m_curState = in_state::object_name;
                return true;
            }
            else if (key == "parentId")
            {
                m_curState = in_state::object_parent_id;
                return true;
            }
            else if (key == "instanceOf")
            {
                m_curState = in_state::object_instance_of;
                return true;
            }
            else if (key == "type")
            {
                m_curState = in_state::object_type;
                return true;
            }
            else if (key == "position")
            {
                m_curState = in_state::object_position_section;
                return true;
            }
            else if (key == "rotation")
            {
                m_curState = in_state::object_rotation_section;
                return true;
            }
            else if (key == "scale")
            {
                m_curState = in_state::object_scale_section;
                return true;
            }
            else if (key == "isEditorVisible")
            {
                m_curState = in_state::object_is_editor_visible;
                return true;
            }
            else if (key == "isExcluded")
            {
                m_curState = in_state::object_is_excluded;
                return true;
            }
            else if (key == "parameters")
            {
                m_curState = in_state::object_parameters_section;
                return true;
            }
        }

        // Custom properties.
        return in_insert_parameter(str, m_curObj.customProperties);

    case in_state::object_parameters:
        return in_insert_parameter(str, m_curObj.parameters);

    default:
        return false;
    }
}

bool in_project_json_handler::EndObject(SizeType memberCount)
{
    // Handle parameters.
    if (!m_paramStack.empty())
    {
        // Error out if the last parameter is not an object.
        if (m_paramStack.back()->type() != parameter_type::object)
        {
            return false;
        }

        // Otherwise, pop the last parameter off of the stack.
        m_paramStack.pop_back();
        return true;
    }

    switch (m_curState)
    {
    case in_state::top_level_object:
        m_curState = in_state::none;
        return true;

    case in_state::metadata:
        m_curState = in_state::top_level_object;
        return true;

    case in_state::object:
        m_project->objects.emplace((m_curObjID.empty()) ?
            guid::random() : m_curObjID, std::move(m_curObj));
        
        m_curObjID = guid::zero();
        m_curObj = object();
        m_curState = in_state::objects;
        return true;

    case in_state::object_parameters:
        m_curState = in_state::object;
        return true;

    default:
        return false;
    }
}

bool in_project_json_handler::StartArray()
{
    // Handle parameters.
    if (!m_paramStack.empty())
    {
        // If the current parameter is empty, replace it with a new array.
        auto& curParam = *m_paramStack.back();
        if (curParam.type() == parameter_type::none)
        {
            curParam = parameter(parameter_type::array);
        }

        // If the current parameter is an array, add a new array to the existing array.
        else if (curParam.type() == parameter_type::array)
        {
            m_paramStack.push_back(
                &curParam.value_array().emplace_back(parameter_type::array));
        }

        // Otherwise, error.
        else
        {
            return false;
        }

        return true;
    }

    switch (m_curState)
    {
    case in_state::objects_section:
        m_curState = in_state::objects;
        return true;

    case in_state::object_position_section:
        m_curObj.position = object::default_position;
        m_curState = in_state::object_position;
        return true;

    case in_state::object_rotation_section:
        m_curObj.rotation = object::default_rotation;
        m_curState = in_state::object_rotation;
        return true;

    case in_state::object_scale_section:
        m_curObj.scale = object::default_scale;
        m_curState = in_state::object_scale;
        return true;

    default:
        return false;
    }
}

bool in_project_json_handler::EndArray(SizeType elementCount)
{
    // Handle parameters.
    if (!m_paramStack.empty())
    {
        // Error out if the last parameter is not an array.
        if (m_paramStack.back()->type() != parameter_type::array)
        {
            return false;
        }

        // Otherwise, pop the last parameter off of the stack.
        m_paramStack.pop_back();
        return true;
    }

    switch (m_curState)
    {
    case in_state::objects:
        m_curState = in_state::top_level_object;
        return true;

    case in_state::object_position:
    case in_state::object_rotation:
    case in_state::object_scale:
        m_curVecElemIndex = 0;
        m_curState = in_state::object;
        return true;

    default:
        return false;
    }
}

void parameter::in_write_all(internal::in_rapidjson_writer& writer,
    const radix_tree<parameter>& parameters)
{
    for (const auto it : parameters)
    {
        writer.Key(it.first);
        it.second.in_write(writer);
    }
}

void parameter::in_write(internal::in_rapidjson_writer& writer) const
{
    switch (m_type)
    {
    case parameter_type::boolean:
        writer.Bool(m_valBool);
        break;

    case parameter_type::signed_integer:
        writer.Int64(m_valSignedInt);
        break;

    case parameter_type::unsigned_integer:
        writer.Uint64(m_valUnsignedInt);
        break;

    case parameter_type::floating:
        writer.Double(m_valFloating);
        break;

    case parameter_type::string:
        writer.String(m_valString);
        break;

    case parameter_type::array:
        writer.StartArray();

        for (auto& param : m_valArray)
        {
            param.in_write(writer);
        }

        writer.EndArray();
        break;

    case parameter_type::object:
        writer.StartObject();
        in_write_all(writer, m_valObject);
        writer.EndObject();
        break;
    }
}

void parameter::in_default_construct_value()
{
    switch (m_type)
    {
    case parameter_type::boolean:
        m_valBool = false;
        break;
    
    default:
    case parameter_type::signed_integer:
        m_valSignedInt = 0;
        break;

    case parameter_type::unsigned_integer:
        m_valUnsignedInt = 0;
        break;

    case parameter_type::floating:
        m_valFloating = 0.0;
        break;

    case parameter_type::string:
        new (&m_valString) std::string();
        break;

    case parameter_type::array:
        new (&m_valArray) std::vector<parameter>();
        break;

    case parameter_type::object:
        new (&m_valObject) radix_tree<parameter>();
        break;
    }
}

void parameter::in_destruct_value() noexcept
{
    switch (m_type)
    {
    case parameter_type::string:
        m_valString.~basic_string();
        break;

    case parameter_type::array:
        m_valArray.~vector();
        break;

    case parameter_type::object:
        m_valObject.~radix_tree();
        break;
    }
}

void parameter::in_copy_construct_value(const parameter& other)
{
    switch (other.m_type)
    {
    case parameter_type::boolean:
        m_valBool = other.m_valBool;
        break;
    
    case parameter_type::signed_integer:
        m_valSignedInt = other.m_valSignedInt;
        break;

    case parameter_type::unsigned_integer:
        m_valUnsignedInt = other.m_valUnsignedInt;
        break;

    case parameter_type::floating:
        m_valFloating = other.m_valFloating;
        break;

    case parameter_type::string:
        new (&m_valString) std::string(other.m_valString);
        break;

    case parameter_type::array:
        new (&m_valArray) std::vector(other.m_valArray);
        break;

    case parameter_type::object:
        new (&m_valObject) radix_tree(other.m_valObject);
        break;
    
    default:
        m_valSignedInt = 0;
        break;
    }
}

void parameter::in_move_construct_value(parameter&& other) noexcept
{
    switch (other.m_type)
    {
    case parameter_type::boolean:
        m_valBool = other.m_valBool;
        break;
    
    case parameter_type::signed_integer:
        m_valSignedInt = other.m_valSignedInt;
        break;

    case parameter_type::unsigned_integer:
        m_valUnsignedInt = other.m_valUnsignedInt;
        break;

    case parameter_type::floating:
        m_valFloating = other.m_valFloating;
        break;

    case parameter_type::string:
        new (&m_valString) std::string(std::move(other.m_valString));
        break;

    case parameter_type::array:
        new (&m_valArray) std::vector(std::move(other.m_valArray));
        break;

    case parameter_type::object:
        new (&m_valObject) radix_tree(std::move(other.m_valObject));
        break;
    
    default:
        m_valSignedInt = 0;
        break;
    }
}

std::intmax_t parameter::get_value_as_int() const
{
    switch (m_type)
    {
    case parameter_type::signed_integer:
        return m_valSignedInt;

    case parameter_type::unsigned_integer:
        return static_cast<std::intmax_t>(m_valUnsignedInt);

    case parameter_type::floating:
        return static_cast<std::intmax_t>(m_valFloating);

    default:
        throw std::runtime_error("Cannot cast value to int in a sane way");
    }
}

std::uintmax_t parameter::get_value_as_uint() const
{
    switch (m_type)
    {
    case parameter_type::signed_integer:
        return static_cast<std::uintmax_t>(m_valSignedInt);

    case parameter_type::unsigned_integer:
        return m_valUnsignedInt;

    case parameter_type::floating:
        return static_cast<std::uintmax_t>(m_valFloating);

    default:
        throw std::runtime_error("Cannot cast value to uint in a sane way");
    }
}

double parameter::get_value_as_floating() const
{
    switch (m_type)
    {
    case parameter_type::signed_integer:
        return static_cast<double>(m_valSignedInt);

    case parameter_type::unsigned_integer:
        return static_cast<double>(m_valUnsignedInt);

    case parameter_type::floating:
        return m_valFloating;

    default:
        throw std::runtime_error("Cannot cast value to floating point in a sane way");
    }
}

parameter& parameter::operator=(const parameter& other)
{
    if (&other != this)
    {
        m_type = other.m_type;
        in_destruct_value();
        in_copy_construct_value(other);
    }

    return *this;
}

parameter& parameter::operator=(parameter&& other) noexcept
{
    if (&other != this)
    {
        m_type = other.m_type;
        in_destruct_value();
        in_move_construct_value(std::move(other));
    }

    return *this;
}

parameter::parameter(parameter_type type) :
    m_type(type)
{
    in_default_construct_value();
}

parameter::parameter(const parameter& other) :
    m_type(other.m_type)
{
    in_copy_construct_value(other);
}

parameter::parameter(parameter&& other) noexcept :
    m_type(other.m_type)
{
    in_move_construct_value(std::move(other));
}

parameter::~parameter()
{
    in_destruct_value();
}

const parameter* object::in_get_parameter_from_name(const char* name,
    const ordered_map<guid, object>& objects) const
{
    // If the parameter we want is specified for this object, just return that.
    const auto param = parameters.get(name);
    if (param)
    {
        return param;
    }

    // Otherwise, if this is an instance of another object, we
    // inherit the parameter from the instanced object.
    else if (has_instance())
    {
        const auto instancedObj = objects.get(instanceOf);
        if (instancedObj)
        {
            return instancedObj->in_get_parameter_from_name(
                name, objects);
        }
    }

    // Otherwise, just return null.
    return nullptr;
}

static const parameter* in_get_parameter(
    const radix_tree<parameter>& params,
    const char* firstNameSep, const char* fullPath)
{
    // Loop through all parameter names searching for the final parameter.
    std::string curName(fullPath, firstNameSep);
    auto param = params.get(curName);

    if (param && param->type() == parameter_type::object)
    {
        const char* curNameSep = firstNameSep;
        while (true)
        {
            // Get the next parameter name after this one, if any.
            const auto nextNameSep = std::strchr(curNameSep + 1, '/');
            if (nextNameSep)
            {
                curName.assign(curNameSep + 1, nextNameSep);
            }
            else
            {
                curName.assign(curNameSep + 1);
            }

            // Get the current parameter. Break if we can't find it.
            param = param->value_object().get(curName);
            if (!param) break;

            // If this is the final parameter, return it regardless of type.
            if (!nextNameSep)
            {
                return param;
            }

            // Otherwise, ensure this parameter is of type object,
            // so we can recurse through its children.
            else if (param->type() != parameter_type::object)
            {
                break;
            }

            // Reset pointer for next loop iteration.
            curNameSep = nextNameSep;
        }
    }

    return nullptr;
}

static const parameter* in_get_parameter(
    const radix_tree<parameter>& params, const char* path)
{
    const auto firstNameSep = std::strchr(path, '/');
    return (firstNameSep) ?
        in_get_parameter(params, firstNameSep, path) :
        params.get(path);
}

const parameter* object::in_get_parameter_from_path(const char* firstNameSep,
    const char* fullPath, const ordered_map<guid, object>& objects) const
{
    // Attempt to find the parameter in the current object.
    const auto param = in_get_parameter(
        parameters, firstNameSep, fullPath);

    if (param)
    {
        return param;
    }

    // We haven't found the final parameter yet, so recursively search instances.
    if (has_instance())
    {
        const auto instancedObj = objects.get(instanceOf);
        if (instancedObj)
        {
            return instancedObj->in_get_parameter_from_path(
                firstNameSep, fullPath, objects);
        }
    }

    // The requested parameter could not be found.
    return nullptr;
}

void object::in_write(internal::in_rapidjson_writer& writer) const
{
    const auto hasInstance = has_instance();

    // Write name if necessary.
    if (name.has_value() && (hasInstance || !name->empty()))
    {
        writer.Key("name");
        writer.String(*name);
    }

    // Write parentId if necessary.
    if (parentID.has_value() && (hasInstance || !parentID->empty()))
    {
        writer.Key("parentId");
        writer.String(parentID->as_string());
    }

    // Write instanceOf if necessary
    if (hasInstance)
    {
        writer.Key("instanceOf");
        writer.String(instanceOf.as_string());
    }

    // Write type.
    if (!hasInstance || has_type())
    {
        writer.Key("type");
        writer.String(type);
    }

    // Set array formatting to same-line.
    writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);

    // Write position if necessary.
    if (position.has_value() && (hasInstance || *position != default_position))
    {
        writer.Key("position");
        writer.StartArray();
        writer.Double(position->x);
        writer.Double(position->y);
        writer.Double(position->z);
        writer.EndArray();
    }

    // Write rotation if necessary.
    if (rotation.has_value() && (hasInstance || *rotation != default_rotation))
    {
        writer.Key("rotation");
        writer.StartArray();
        writer.Double(rotation->x);
        writer.Double(rotation->y);
        writer.Double(rotation->z);
        writer.Double(rotation->w);
        writer.EndArray();
    }

    // Write scale if necessary.
    if (scale.has_value() && (hasInstance || *scale != default_scale))
    {
        writer.Key("scale");
        writer.StartArray();
        writer.Double(scale->x);
        writer.Double(scale->y);
        writer.Double(scale->z);
        writer.EndArray();
    }

    // Set array formatting to multi-line.
    writer.SetFormatOptions(rapidjson::kFormatDefault);

    // Write isEditorVisible if necessary.
    if (isEditorVisible.has_value() && (hasInstance ||
        *isEditorVisible != default_is_editor_visible))
    {
        writer.Key("isEditorVisible");
        writer.Bool(*isEditorVisible);
    }

    // Write isExcluded if necessary.
    if (isExcluded.has_value() && (hasInstance ||
        *isExcluded != default_is_excluded))
    {
        writer.Key("isExcluded");
        writer.Bool(*isExcluded);
    }

    // Write parameters if necessary.
    if (!parameters.empty())
    {
        writer.Key("parameters");
        writer.StartObject();

        parameter::in_write_all(writer, parameters);

        writer.EndObject();
    }

    // Write custom properties if necessary.
    if (!customProperties.empty())
    {
        parameter::in_write_all(writer, customProperties);
    }
}

const std::string object::default_name = std::string();

bool object::has_inherited_parameters(
    const ordered_map<guid, object>& objects) const
{
    // If this is not an instanced object, we have no inherited parameters.
    if (!has_instance())
    {
        return false;
    }

    // If this is an instanced object, return whether it has any parameters.
    else
    {
        const auto instancedObj = objects.get(instanceOf);
        if (instancedObj)
        {
            // The instanced object has parameters, so return true.
            if (!instancedObj->parameters.empty())
            {
                return true;
            }

            // We still have to check its instanced objects recursively.
            return instancedObj->has_inherited_parameters(objects);
        }
    }

    // Fallback for if the instanced object could not be found.
    return false;
}

const std::string* object::get_inherited_name(
    const ordered_map<guid, object>& objects) const
{
    return in_get_inherited_value<&object::name>(objects);
}

const std::string* object::get_inherited_type(
    const ordered_map<guid, object>& objects) const
{
    // If the value we want is specified for this object, just return that.
    if (has_type())
    {
        return &type;
    }

    // Otherwise, if this is an instance of another object, we
    // inherit the value from the instanced object's property.
    else if (has_instance())
    {
        const auto instancedObj = objects.get(instanceOf);
        if (instancedObj)
        {
            return instancedObj->get_inherited_type(objects);
        }
    }

    // Otherwise, just return null.
    return nullptr;
}

const guid* object::get_inherited_parent_id(
    const ordered_map<guid, object>& objects) const
{
    return in_get_inherited_value<&object::parentID>(objects);
}

const vec3& object::get_local_position(
    const ordered_map<guid, object>& objects) const
{
    const auto localPos = in_get_inherited_value<&object::position>(objects);
    return (localPos) ? *localPos : default_position;
}

const quat& object::get_local_rotation(
    const ordered_map<guid, object>& objects) const
{
    const auto localRot = in_get_inherited_value<&object::rotation>(objects);
    return (localRot) ? *localRot : default_rotation;
}

const vec3& object::get_local_scale(
    const ordered_map<guid, object>& objects) const
{
    const auto localScale = in_get_inherited_value<&object::scale>(objects);
    return (localScale) ? *localScale : default_scale;
}

static DirectX::XMMATRIX in_get_matrix(const object& obj,
    const ordered_map<guid, object>& objects) noexcept
{
    // Compute matrix with the given translation and rotation.
    alignas(16) const auto pos = obj.get_local_position(objects);
    alignas(16) const auto rot = obj.get_local_rotation(objects);
    alignas(16) const auto scale = obj.get_local_scale(objects);

    return DirectX::XMMatrixAffineTransformation(
        DirectX::XMLoadFloat3A(reinterpret_cast<const DirectX::XMFLOAT3A*>(&scale.x)),
        DirectX::g_XMZero,
        DirectX::XMLoadFloat4A(reinterpret_cast<const DirectX::XMFLOAT4A*>(&rot.x)),
        DirectX::XMLoadFloat3A(reinterpret_cast<const DirectX::XMFLOAT3A*>(&pos.x))
    );
}

matrix4x4A object::get_local_transform(const ordered_map<guid, object>& objects) const
{
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT4X4A, sizeof(matrix4x4A));

    matrix4x4A result;
    DirectX::XMStoreFloat4x4A(
        reinterpret_cast<DirectX::XMFLOAT4X4A*>(&result.m11),
        in_get_matrix(*this, objects));

    return result;
}

static DirectX::XMMATRIX in_get_global_transform_matrix(
    const object& obj, const ordered_map<guid, object>& objects)
{
    auto xmLocalMtx = in_get_matrix(obj, objects);
    const auto parentID = obj.get_inherited_parent_id(objects);

    if (parentID && !parentID->empty())
    {
        const auto parentObj = objects.get(*parentID);
        if (parentObj)
        {
            return DirectX::XMMatrixMultiply(
                xmLocalMtx,
                in_get_global_transform_matrix(*parentObj, objects));
        }
    }

    return xmLocalMtx;
}

matrix4x4A object::get_global_transform(const ordered_map<guid, object>& objects) const
{
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT4X4A, sizeof(matrix4x4A));

    matrix4x4A result;
    DirectX::XMStoreFloat4x4A(
        reinterpret_cast<DirectX::XMFLOAT4X4A*>(&result.m11),
        in_get_global_transform_matrix(*this, objects));

    return result;
}

const bool* object::get_inherited_is_editor_visible(
    const ordered_map<guid, object>& objects) const
{
    return in_get_inherited_value<&object::isEditorVisible>(objects);
}

const bool* object::get_inherited_is_excluded(
    const ordered_map<guid, object>& objects) const
{
    return in_get_inherited_value<&object::isExcluded>(objects);
}

const parameter* object::get_parameter(const char* path,
    const ordered_map<guid, object>& objects) const
{
    const auto firstNameSep = std::strchr(path, '/');
    return (firstNameSep) ?
        in_get_parameter_from_path(firstNameSep, path, objects) :
        in_get_parameter_from_name(path, objects);
}

static bool in_insert_flattened_parameters(
    const radix_tree<parameter>& params,
    radix_tree<parameter>& flatParams)
{
    bool insertedAnyParams = false;
    for (const auto it : params)
    {
        // Insert a copy of this parameter into the flattened parameters list.
        // NOTE: insert will do nothing if a parameter with this name is
        // already present within the flattened parameters list.
        const auto flatParamIt = flatParams.insert(it.first, it.second);

        // Skip if this is the first time we've inserted a parameter with this name.
        if (flatParamIt.second)
        {
            insertedAnyParams = true;
            continue;
        }

        // Recursively insert all children of this parameter if necessary.
        auto& flatParam = flatParamIt.first->second;
        if (it.second.type() == parameter_type::object &&
            flatParam.type() == parameter_type::object)
        {
            if (in_insert_flattened_parameters(
                it.second.value_object(),
                flatParam.value_object()))
            {
                insertedAnyParams = true;
            }
        }
    }

    return insertedAnyParams;
}

bool object::get_flattened_parameters(
    const ordered_map<guid, object>& objects,
    radix_tree<parameter>& flatParams,
    const char* rootPath) const
{
    // Add all parameters from the current object.
    bool insertedAnyParams = false;
    if (rootPath && *rootPath != '\0')
    {
        const auto param = in_get_parameter(parameters, rootPath);
        if (param && param->type() == parameter_type::object)
        {
            insertedAnyParams = in_insert_flattened_parameters(
                param->value_object(), flatParams);
        }
    }
    else
    {
        insertedAnyParams = in_insert_flattened_parameters(
            parameters, flatParams);
    }

    // Recurse through instanced objects.
    if (has_instance())
    {
        const auto instancedObj = objects.get(instanceOf);
        if (instancedObj && instancedObj->get_flattened_parameters(
            objects, flatParams, rootPath))
        {
            insertedAnyParams = true;
        }
    }

    return insertedAnyParams;
}

radix_tree<parameter> object::get_flattened_parameters(
    const ordered_map<guid, object>& objects,
    const char* rootPath) const
{
    radix_tree<parameter> result;
    get_flattened_parameters(objects, result);
    return result;
}

void project_metadata::in_write(internal::in_rapidjson_writer& writer) const
{
    // Write name if necessary.
    if (!name.empty())
    {
        writer.Key("name");
        writer.String(name);
    }

    // Write author if necessary.
    if (!author.empty())
    {
        writer.Key("author");
        writer.String(author);
    }

    // Write date if necessary.
    if (!date.empty())
    {
        writer.Key("date");
        writer.String(date);
    }

    // Write version if necessary.
    if (!version.empty())
    {
        writer.Key("version");
        writer.String(version);
    }

    // Write description if necessary.
    if (!description.empty())
    {
        writer.Key("description");
        writer.String(description);
    }

    // Write custom properties if necessary.
    if (!customProperties.empty())
    {
        parameter::in_write_all(writer, customProperties);
    }
}

bool project_metadata::empty() const noexcept
{
    return (name.empty() && author.empty() &&
        date.empty() && version.empty() &&
        description.empty() && customProperties.empty());
}

void project_metadata::clear() noexcept
{
    name.clear();
    author.clear();
    date.clear();
    version.clear();
    description.clear();
    customProperties.clear();
}

void project::write(stream& stream) const
{
    // Create RapidJSON writer.
    internal::in_rapidjson_output_stream_wrapper jsonStream(stream);
    internal::in_rapidjson_writer writer(jsonStream);

    // Write HSON header.
    writer.StartObject();
    writer.Key("version");
    writer.Uint(max_supported_version);

    // Write metadata if necessary.
    if (!metadata.empty())
    {
        writer.Key("metadata");
        writer.StartObject();
        metadata.in_write(writer);
        writer.EndObject();
    }

    // Write objects if necessary.
    if (!objects.empty())
    {
        writer.Key("objects");
        writer.StartArray();

        // Determine which objects need to have their ids written.
        const bool writeAllIDs = true; // TODO: Make this optional.
        robin_hood::unordered_set<guid> requiredIDs;
        requiredIDs.reserve(objects.size());

        if (!writeAllIDs)
        {
            for (const auto& it : objects)
            {
                // If this object references a parent, the parent's ID must be included.
                if (it.second.parentID.has_value() && !it.second.parentID->empty())
                {
                    requiredIDs.insert(*it.second.parentID);
                }

                // If this object is an instance of another object,
                // that object's ID must be included.
                if (it.second.has_instance())
                {
                    requiredIDs.insert(it.second.instanceOf);
                }

                // TODO: Handle object references in parameters!!!
            }
        }

        // Write objects.
        for (const auto& it : objects)
        {
            writer.StartObject();
            
            // Write ID if necessary.
            if (writeAllIDs || requiredIDs.contains(it.first))
            {
                writer.Key("id");
                writer.String(it.first.as_string());
            }

            it.second.in_write(writer);
            writer.EndObject();
        }

        writer.EndArray();
    }

    // Write custom properties if necessary.
    if (!customProperties.empty())
    {
        parameter::in_write_all(writer, customProperties);
    }

    writer.EndObject();
}

void project::save(const nchar* filePath) const
{
    file_stream stream(filePath, file::mode::write);
    write(stream);
}

void project::in_parse(const void* rawData, std::size_t rawDataSize)
{
    in_project_json_handler handler(*this);
    internal::in_parse_json(handler, rawData, rawDataSize);
}

void project::in_read(stream& stream)
{
    in_project_json_handler handler(*this);
    internal::in_read_json(handler, stream);
}

void project::in_load(const nchar* filePath)
{
    in_project_json_handler handler(*this);
    internal::in_load_json(handler, filePath);
}

void project::clear() noexcept
{
    metadata.clear();
    objects.clear();
    customProperties.clear();
}

void project::parse(const void* rawData, std::size_t rawDataSize)
{
    clear();
    in_parse(rawData, rawDataSize);
}

void project::read(stream& stream)
{
    clear();
    in_read(stream);
}

void project::load(const nchar* filePath)
{
    clear();
    in_load(filePath);
}

project::project(const void* rawData, std::size_t rawDataSize)
{
    in_parse(rawData, rawDataSize);
}

project::project(stream& stream)
{
    in_read(stream);
}

project::project(const nchar* filePath)
{
    in_load(filePath);
}
} // hson
} // hl
