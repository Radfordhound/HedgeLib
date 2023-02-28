#include "../io/hl_in_rapidjson.h"
#include "hedgelib/sets/hl_set_obj_type.h"
#include <string_view>

namespace hl
{
namespace internal
{
class in_set_object_type_database_json_handler
{
    enum class in_state
    {
        none,
        top_level_object,

        file_version_number,

        enums_section,
        enums,
        enum_section,
        _enum,
        enum_type,
        enum_values_section,
        enum_values,
        enum_value_section,
        enum_value,
        enum_value_value,
        enum_value_descriptions_section,
        enum_value_descriptions,
        enum_value_description,
        enum_value_default_description,

        structs_section,
        structs,
        struct_section,
        _struct,
        struct_parent,
        struct_fields_section,
        struct_fields,
        struct_field,
        struct_field_name,
        struct_field_type,
        struct_field_subtype,
        struct_field_array_size,
        struct_field_alignment,
        struct_field_vec_array_end,
        struct_field_default,
        struct_field_min_range,
        struct_field_min_range_x,
        struct_field_min_range_y,
        struct_field_min_range_z,
        struct_field_min_range_w,
        struct_field_max_range,
        struct_field_max_range_x,
        struct_field_max_range_y,
        struct_field_max_range_z,
        struct_field_max_range_w,
        struct_field_step,
        struct_field_step_x,
        struct_field_step_y,
        struct_field_step_z,
        struct_field_step_w,
        struct_field_descriptions_section,
        struct_field_descriptions,
        struct_field_description,
        struct_field_default_description,

        objects_section,
        objects,
        object_section,
        object,
        object_struct_type,
        object_category,
    };

    set_object_type_database*       m_database;
    reflect::enum_definition*       m_curEnum = nullptr;
    reflect::enum_value_definition* m_curEnumVal = nullptr;
    std::string*                    m_curDesc = nullptr;
    reflect::struct_definition*     m_curStruct = nullptr;
    reflect::field_definition*      m_curStructField = nullptr;
    set_object_type*                m_curObj = nullptr;
    unsigned int                    m_fileVersionNumber = max_supported_version;
    in_state                        m_curState = in_state::none;

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

    in_set_object_type_database_json_handler(set_object_type_database& db) noexcept :
        m_database(&db) {}
};

bool in_set_object_type_database_json_handler::Null()
{
    return false;
}

bool in_set_object_type_database_json_handler::Bool(bool val)
{
    switch (m_curState)
    {
    case in_state::struct_field_default:
        if (m_curStructField->is_bool())
        {
            m_curStructField->set_default_val_bool(val);
        }
        else if (m_curStructField->is_int())
        {
            m_curStructField->set_default_val_int(val);
        }
        else if (m_curStructField->is_uint())
        {
            m_curStructField->set_default_val_uint(val);
        }
        else if (m_curStructField->is_char())
        {
            m_curStructField->set_default_val_char(val);
        }
        else
        {
            return false;
        }

        m_curState = in_state::struct_field;
        return true;
        
    default:
        return false;
    }
}

bool in_set_object_type_database_json_handler::Int(int val)
{
    switch (m_curState)
    {
    case in_state::enum_value_value:
        m_curEnumVal->value = val;
        m_curState = in_state::enum_value;
        return true;
        
    case in_state::struct_field_default:
        if (m_curStructField->is_int())
        {
            m_curStructField->set_default_val_int(val);
        }
        else if (m_curStructField->is_uint())
        {
            m_curStructField->set_default_val_uint(val);
        }
        else if (m_curStructField->is_floating())
        {
            m_curStructField->set_default_val_floating(val);
        }
        else
        {
            return false;
        }

        m_curState = in_state::struct_field;
        return true;

    case in_state::struct_field_min_range:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field;
        return true;

    case in_state::struct_field_max_range:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field;
        return true;

    case in_state::struct_field_step:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field;
        return true;

    case in_state::struct_field_min_range_x:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_min_range_y;
        return true;
        
    case in_state::struct_field_min_range_y:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_min_range_z;
        return true;
        
    case in_state::struct_field_min_range_z:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_min_range_w;
        return true;
        
    case in_state::struct_field_min_range_w:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_vec_array_end;
        return true;
        
    case in_state::struct_field_max_range_x:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_max_range_y;
        return true;
        
    case in_state::struct_field_max_range_y:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_max_range_z;
        return true;
        
    case in_state::struct_field_max_range_z:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_max_range_w;
        return true;
        
    case in_state::struct_field_max_range_w:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_vec_array_end;
        return true;
        
    case in_state::struct_field_step_x:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_step_y;
        return true;
        
    case in_state::struct_field_step_y:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_step_z;
        return true;
        
    case in_state::struct_field_step_z:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_step_w;
        return true;
        
    case in_state::struct_field_step_w:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_vec_array_end;
        return true;
        
    default:
        return false;
    }
}

bool in_set_object_type_database_json_handler::Uint(unsigned int val)
{
    switch (m_curState)
    {
    case in_state::file_version_number:
        m_fileVersionNumber = val;
        m_curState = in_state::top_level_object;
        return true;

    case in_state::enum_value_value:
        m_curEnumVal->value = val;
        m_curState = in_state::enum_value;
        return true;

    case in_state::struct_field_array_size:
        m_curStructField->set_array_count(val);
        m_curState = in_state::struct_field;
        return true;

    case in_state::struct_field_alignment:
        m_curStructField->alignment = val;
        m_curState = in_state::struct_field;
        return true;
        
    case in_state::struct_field_default:
        if (m_curStructField->is_int())
        {
            m_curStructField->set_default_val_int(val);
        }
        else if (m_curStructField->is_uint())
        {
            m_curStructField->set_default_val_uint(val);
        }
        else if (m_curStructField->is_floating())
        {
            m_curStructField->set_default_val_floating(val);
        }
        else
        {
            return false;
        }

        m_curState = in_state::struct_field;
        return true;
        
    case in_state::struct_field_min_range:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field;
        return true;

    case in_state::struct_field_max_range:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field;
        return true;

    case in_state::struct_field_step:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field;
        return true;

    case in_state::struct_field_min_range_x:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_min_range_y;
        return true;
        
    case in_state::struct_field_min_range_y:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_min_range_z;
        return true;
        
    case in_state::struct_field_min_range_z:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_min_range_w;
        return true;
        
    case in_state::struct_field_min_range_w:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_vec_array_end;
        return true;
        
    case in_state::struct_field_max_range_x:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_max_range_y;
        return true;
        
    case in_state::struct_field_max_range_y:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_max_range_z;
        return true;
        
    case in_state::struct_field_max_range_z:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_max_range_w;
        return true;
        
    case in_state::struct_field_max_range_w:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_vec_array_end;
        return true;
        
    case in_state::struct_field_step_x:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_step_y;
        return true;
        
    case in_state::struct_field_step_y:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_step_z;
        return true;
        
    case in_state::struct_field_step_z:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_step_w;
        return true;
        
    case in_state::struct_field_step_w:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_vec_array_end;
        return true;
        
    default:
        return false;
    }
}

bool in_set_object_type_database_json_handler::Int64(std::int64_t val)
{
    switch (m_curState)
    {
    case in_state::enum_value_value:
        m_curEnumVal->value = val;
        m_curState = in_state::enum_value;
        return true;

    case in_state::struct_field_default:
        if (m_curStructField->is_int())
        {
            m_curStructField->set_default_val_int(val);
        }
        else if (m_curStructField->is_uint())
        {
            m_curStructField->set_default_val_uint(val);
        }
        else if (m_curStructField->is_floating())
        {
            m_curStructField->set_default_val_floating(val);
        }
        else
        {
            return false;
        }

        m_curState = in_state::struct_field;
        return true;
        
    case in_state::struct_field_min_range:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field;
        return true;

    case in_state::struct_field_max_range:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field;
        return true;

    case in_state::struct_field_step:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field;
        return true;

    case in_state::struct_field_min_range_x:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_min_range_y;
        return true;
        
    case in_state::struct_field_min_range_y:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_min_range_z;
        return true;
        
    case in_state::struct_field_min_range_z:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_min_range_w;
        return true;
        
    case in_state::struct_field_min_range_w:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_vec_array_end;
        return true;
        
    case in_state::struct_field_max_range_x:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_max_range_y;
        return true;
        
    case in_state::struct_field_max_range_y:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_max_range_z;
        return true;
        
    case in_state::struct_field_max_range_z:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_max_range_w;
        return true;
        
    case in_state::struct_field_max_range_w:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_vec_array_end;
        return true;
        
    case in_state::struct_field_step_x:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_step_y;
        return true;
        
    case in_state::struct_field_step_y:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_step_z;
        return true;
        
    case in_state::struct_field_step_z:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_step_w;
        return true;
        
    case in_state::struct_field_step_w:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_vec_array_end;
        return true;
        
    default:
        return false;
    }
}

bool in_set_object_type_database_json_handler::Uint64(std::uint64_t val)
{
    switch (m_curState)
    {
    case in_state::enum_value_value:
        m_curEnumVal->value = val;
        m_curState = in_state::enum_value;
        return true;
        
    case in_state::struct_field_array_size:
        m_curStructField->set_array_count(val);
        m_curState = in_state::struct_field;
        return true;
        
    case in_state::struct_field_default:
        if (m_curStructField->is_int())
        {
            m_curStructField->set_default_val_int(
                static_cast<std::intmax_t>(val));
        }
        else if (m_curStructField->is_uint())
        {
            m_curStructField->set_default_val_uint(val);
        }
        else if (m_curStructField->is_floating())
        {
            m_curStructField->set_default_val_floating(val);
        }
        else
        {
            return false;
        }

        m_curState = in_state::struct_field;
        return true;
        
    case in_state::struct_field_min_range:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field;
        return true;

    case in_state::struct_field_max_range:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field;
        return true;

    case in_state::struct_field_step:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field;
        return true;

    case in_state::struct_field_min_range_x:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_min_range_y;
        return true;
        
    case in_state::struct_field_min_range_y:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_min_range_z;
        return true;
        
    case in_state::struct_field_min_range_z:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_min_range_w;
        return true;
        
    case in_state::struct_field_min_range_w:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_vec_array_end;
        return true;
        
    case in_state::struct_field_max_range_x:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_max_range_y;
        return true;
        
    case in_state::struct_field_max_range_y:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_max_range_z;
        return true;
        
    case in_state::struct_field_max_range_z:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_max_range_w;
        return true;
        
    case in_state::struct_field_max_range_w:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_vec_array_end;
        return true;
        
    case in_state::struct_field_step_x:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_step_y;
        return true;
        
    case in_state::struct_field_step_y:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_step_z;
        return true;
        
    case in_state::struct_field_step_z:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_step_w;
        return true;
        
    case in_state::struct_field_step_w:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_vec_array_end;
        return true;
        
    default:
        return false;
    }
}

bool in_set_object_type_database_json_handler::Double(double val)
{
    switch (m_curState)
    {
    case in_state::struct_field_default:
        if (m_curStructField->is_floating())
        {
            m_curStructField->set_default_val_floating(val);
        }
        else
        {
            return false;
        }

        m_curState = in_state::struct_field;
        return true;
        
    case in_state::struct_field_min_range:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field;
        return true;

    case in_state::struct_field_max_range:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field;
        return true;

    case in_state::struct_field_step:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field;
        return true;

    case in_state::struct_field_min_range_x:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_min_range_y;
        return true;
        
    case in_state::struct_field_min_range_y:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_min_range_z;
        return true;
        
    case in_state::struct_field_min_range_z:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_min_range_w;
        return true;
        
    case in_state::struct_field_min_range_w:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_vec_array_end;
        return true;
        
    case in_state::struct_field_max_range_x:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_max_range_y;
        return true;
        
    case in_state::struct_field_max_range_y:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_max_range_z;
        return true;
        
    case in_state::struct_field_max_range_z:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_max_range_w;
        return true;
        
    case in_state::struct_field_max_range_w:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_vec_array_end;
        return true;
        
    case in_state::struct_field_step_x:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_step_y;
        return true;
        
    case in_state::struct_field_step_y:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_step_z;
        return true;
        
    case in_state::struct_field_step_z:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_step_w;
        return true;
        
    case in_state::struct_field_step_w:
        // TODO: Handle min/max range.
        m_curState = in_state::struct_field_vec_array_end;
        return true;
        
    default:
        return false;
    }
}

bool in_set_object_type_database_json_handler::RawNumber(
    const Ch* str, SizeType length, bool copy)
{
    return false;
}

bool in_set_object_type_database_json_handler::String(
    const Ch* str, SizeType length, bool copy)
{
    switch (m_curState)
    {
    case in_state::enum_type:
        m_curEnum->type = reflect::get_integral_type(std::string_view(str, length));
        m_curState = in_state::_enum;
        return true;

    case in_state::enum_value_description:
        m_curDesc->assign(str, length);
        m_curState = in_state::enum_value_descriptions;
        return true;

    case in_state::enum_value_default_description:
        m_curEnumVal->defaultDescription.assign(str, length);
        m_curState = in_state::enum_value;
        return true;

    case in_state::struct_parent:
        m_curStruct->parent.assign(str, length);
        m_curState = in_state::_struct;
        return true;

    case in_state::struct_field_name:
        m_curStructField->name.assign(str, length);
        m_curState = in_state::struct_field;
        return true;

    case in_state::struct_field_type:
        m_curStructField->set_type(std::string(str, length));
        m_curState = in_state::struct_field;
        return true;

    case in_state::struct_field_subtype:
        m_curStructField->set_subtype(std::string(str, length));
        m_curState = in_state::struct_field;
        return true;

    case in_state::struct_field_default:
        if (m_curStructField->is_string())
        {
            m_curStructField->set_default_val_string(std::string(str, length));
        }
        else
        {
            const auto enumType = m_database->enums.get(m_curStructField->type());
            if (!enumType) return false;

            const auto enumVal = enumType->values.get(str);
            if (!enumVal) return false;

            m_curStructField->set_default_val_uint(enumVal->value);
        }

        m_curState = in_state::struct_field;
        return true;
        
    case in_state::struct_field_description:
        m_curDesc->assign(str, length);
        m_curState = in_state::struct_field_descriptions;
        return true;

    case in_state::struct_field_default_description:
        m_curStructField->defaultDescription.assign(str, length);
        m_curState = in_state::struct_field;
        return true;

    case in_state::object_struct_type:
        m_curObj->structType.assign(str, length);
        m_curState = in_state::object;
        return true;

    case in_state::object_category:
        m_curObj->category.assign(str, length);
        m_curState = in_state::object;
        return true;

    default:
        return false;
    }
}

bool in_set_object_type_database_json_handler::StartObject()
{
    switch (m_curState)
    {
    case in_state::none:
        m_curState = in_state::top_level_object;
        return true;

    case in_state::enums_section:
        m_curState = in_state::enums;
        return true;

    case in_state::enum_section:
        m_curState = in_state::_enum;
        return true;

    case in_state::enum_values_section:
        m_curState = in_state::enum_values;
        return true;

    case in_state::enum_value_section:
        m_curState = in_state::enum_value;
        return true;

    case in_state::enum_value_descriptions_section:
        m_curState = in_state::enum_value_descriptions;
        return true;

    case in_state::structs_section:
        m_curState = in_state::structs;
        return true;

    case in_state::struct_section:
        m_curState = in_state::_struct;
        return true;

    case in_state::struct_fields:
        m_curStructField = &m_curStruct->fields.emplace_back();
        m_curState = in_state::struct_field;
        return true;

    case in_state::struct_field_descriptions_section:
        m_curState = in_state::struct_field_descriptions;
        return true;

    case in_state::objects_section:
        m_curState = in_state::objects;
        return true;

    case in_state::object_section:
        m_curState = in_state::object;
        return true;

    default:
        return false;
    }
}

bool in_set_object_type_database_json_handler::Key(
    const Ch* str, SizeType length, bool copy)
{
    const std::string_view key(str, length);
    switch (m_curState)
    {
    case in_state::top_level_object:
        if (key == "version")
        {
            m_curState = in_state::file_version_number;
            return true;
        }
        else if (key == "enums")
        {
            m_curState = in_state::enums_section;
            return true;
        }
        else if (key == "structs")
        {
            m_curState = in_state::structs_section;
            return true;
        }
        else if (key == "objects")
        {
            m_curState = in_state::objects_section;
            return true;
        }
        return false;

    case in_state::enums:
        m_curEnum = &m_database->enums.insert(str).first->second;
        m_curState = in_state::enum_section;
        return true;

    case in_state::_enum:
        if (key == "type")
        {
            m_curState = in_state::enum_type;
            return true;
        }
        else if (key == "values")
        {
            m_curState = in_state::enum_values_section;
            return true;
        }
        return false;

    case in_state::enum_values:
        m_curEnumVal = &m_curEnum->values.insert(str).first->second;
        m_curState = in_state::enum_value_section;
        return true;

    case in_state::enum_value:
        if (key == "value")
        {
            m_curState = in_state::enum_value_value;
            return true;
        }
        else if (key == "descriptions")
        {
            m_curState = in_state::enum_value_descriptions_section;
            return true;
        }
        else if (key == "description")
        {
            m_curState = in_state::enum_value_default_description;
            return true;
        }
        return false;

    case in_state::enum_value_descriptions:
        m_curDesc = &m_curEnumVal->descriptions.insert(str).first->second;
        m_curState = in_state::enum_value_description;
        return true;

    case in_state::structs:
        m_curStruct = &m_database->structs.insert(str).first->second;
        m_curState = in_state::struct_section;
        return true;

    case in_state::_struct:
        if (key == "parent")
        {
            m_curState = in_state::struct_parent;
            return true;
        }
        else if (key == "fields")
        {
            m_curState = in_state::struct_fields_section;
            return true;
        }
        return false;

    case in_state::struct_field:
        if (key == "name")
        {
            m_curState = in_state::struct_field_name;
            return true;
        }
        else if (key == "type")
        {
            m_curState = in_state::struct_field_type;
            return true;
        }
        else if (key == "subtype")
        {
            m_curState = in_state::struct_field_subtype;
            return true;
        }
        else if (key == "array_size")
        {
            m_curState = in_state::struct_field_array_size;
            return true;
        }
        else if (key == "alignment")
        {
            m_curState = in_state::struct_field_alignment;
            return true;
        }
        else if (key == "default")
        {
            m_curState = in_state::struct_field_default;
            return true;
        }
        else if (key == "min_range")
        {
            m_curState = in_state::struct_field_min_range;
            return true;
        }
        else if (key == "max_range")
        {
            m_curState = in_state::struct_field_max_range;
            return true;
        }
        else if (key == "step")
        {
            m_curState = in_state::struct_field_step;
            return true;
        }
        else if (key == "descriptions")
        {
            m_curState = in_state::struct_field_descriptions_section;
            return true;
        }
        else if (key == "description")
        {
            m_curState = in_state::struct_field_default_description;
            return true;
        }
        return false;

    case in_state::struct_field_descriptions:
        m_curDesc = &m_curStructField->descriptions.insert(str).first->second;
        m_curState = in_state::struct_field_description;
        return true;

    case in_state::objects:
        m_curObj = &m_database->insert(str).first->second;
        m_curState = in_state::object_section;
        return true;

    case in_state::object:
        if (key == "struct")
        {
            m_curState = in_state::object_struct_type;
            return true;
        }
        else if (key == "category")
        {
            m_curState = in_state::object_category;
            return true;
        }
        return true;

    default:
        return false;
    }
}

bool in_set_object_type_database_json_handler::EndObject(
    SizeType memberCount)
{
    switch (m_curState)
    {
    case in_state::top_level_object:
        m_curState = in_state::none;
        return true;

    case in_state::enums:
    case in_state::structs:
    case in_state::objects:
        m_curState = in_state::top_level_object;
        return true;

    case in_state::_enum:
        // TODO: Auto-compute enum base type if none was specified.
        m_curState = in_state::enums;
        return true;

    case in_state::enum_values:
        m_curState = in_state::_enum;
        return true;

    case in_state::enum_value:
        m_curState = in_state::enum_values;
        return true;

    case in_state::enum_value_descriptions:
        m_curState = in_state::enum_value;
        return true;

    case in_state::_struct:
        m_curStruct = nullptr;
        m_curState = in_state::structs;
        return true;

    case in_state::struct_field:
        m_curStructField = nullptr;
        m_curState = in_state::struct_fields;
        return true;

    case in_state::struct_field_descriptions:
        m_curState = in_state::struct_field;
        return true;

    case in_state::object:
        m_curObj = nullptr;
        m_curState = in_state::objects;
        return true;

    default:
        return false;
    }
}

bool in_set_object_type_database_json_handler::StartArray()
{
    switch (m_curState)
    {
    case in_state::struct_fields_section:
        m_curState = in_state::struct_fields;
        return true;

    // TODO: Handle default vector values.

    case in_state::struct_field_min_range:
        m_curState = in_state::struct_field_min_range_x;
        return true;

    case in_state::struct_field_max_range:
        m_curState = in_state::struct_field_max_range_x;
        return true;

    case in_state::struct_field_step:
        m_curState = in_state::struct_field_step_x;
        return true;

    default:
        return false;
    }
}

bool in_set_object_type_database_json_handler::EndArray(SizeType elementCount)
{
    switch (m_curState)
    {
    case in_state::struct_fields:
        m_curState = in_state::_struct;
        return true;

    // TODO: Handle default vector values.
    
    case in_state::struct_field_vec_array_end:
    case in_state::struct_field_min_range_x:
    case in_state::struct_field_min_range_y:
    case in_state::struct_field_min_range_z:
    case in_state::struct_field_min_range_w:
    case in_state::struct_field_max_range_x:
    case in_state::struct_field_max_range_y:
    case in_state::struct_field_max_range_z:
    case in_state::struct_field_max_range_w:
    case in_state::struct_field_step_x:
    case in_state::struct_field_step_y:
    case in_state::struct_field_step_z:
    case in_state::struct_field_step_w:
        m_curState = in_state::struct_field;
        return true;

    default:
        return false;
    }
}
} // internal

void set_object_type_database::in_parse(
    const void* rawData, std::size_t rawDataSize)
{
    internal::in_set_object_type_database_json_handler handler(*this);
    internal::in_parse_json(handler, rawData, rawDataSize);
}

void set_object_type_database::in_read(stream& stream)
{
    internal::in_set_object_type_database_json_handler handler(*this);
    internal::in_read_json(handler, stream);
}

void set_object_type_database::in_load(const nchar* filePath)
{
    internal::in_set_object_type_database_json_handler handler(*this);
    internal::in_load_json(handler, filePath);
}

void set_object_type_database::parse(
    const void* rawData, std::size_t rawDataSize)
{
    clear();
    in_parse(rawData, rawDataSize);
}

void set_object_type_database::read(stream& stream)
{
    clear();
    in_read(stream);
}

void set_object_type_database::load(const nchar* filePath)
{
    clear();
    in_load(filePath);
}

set_object_type_database::set_object_type_database(
    const void* rawData, std::size_t rawDataSize)
{
    in_parse(rawData, rawDataSize);
}

set_object_type_database::set_object_type_database(stream& stream)
{
    in_read(stream);
}

set_object_type_database::set_object_type_database(const nchar* filePath)
{
    in_load(filePath);
}
} // hl
