#include "hedgelib/hl_reflect.h"

namespace hl
{
namespace reflect
{
const char* const integral_type_ids[] =
{
    "int8",
    "int16",
    "int32",
    "int64",

    "uint8",
    "uint16",
    "uint32",
    "uint64"
};

const char* const builtin_type_ids[] =
{
    builtin_type_id_int8,
    builtin_type_id_int16,
    builtin_type_id_int32,
    builtin_type_id_int64,

    builtin_type_id_uint8,
    builtin_type_id_uint16,
    builtin_type_id_uint32,
    builtin_type_id_uint64,

    builtin_type_id_float32,
    builtin_type_id_float64,

    builtin_type_id_char,
    builtin_type_id_string,

    builtin_type_id_bool,
    builtin_type_id_array,
    builtin_type_id_object_reference,
    builtin_type_id_vector2,
    builtin_type_id_vector3,
    builtin_type_id_vector4,
    builtin_type_id_quaternion
};

const char* enum_definition::get_name_of_value(std::uintmax_t val) const
{
    for (const auto it : values)
    {
        if (it.second.value == val)
        {
            return it.first;
        }
    }

    return nullptr;
}

void field_definition::in_destruct_default_val() noexcept
{
    // Destruct any type that has a non-trivial destructor.
    if (m_type == builtin_type_id_string)
    {
        m_defaultValString.~basic_string();
    }
}

void field_definition::in_set_default_val_for_type(const std::string& type) noexcept
{
    if (type == builtin_type_id_float32 || type == builtin_type_id_float64)
    {
        m_defaultValFloating = 0.0;
    }
    else if (type == builtin_type_id_char)
    {
        m_defaultValChar = '\0';
    }
    else if (type == builtin_type_id_string)
    {
        m_defaultValString = "";
    }
    else if (type == builtin_type_id_bool)
    {
        m_defaultValBool = false;
    }
    else if (type == builtin_type_id_vector2)
    {
        m_defaultValVec2 = vec2::zero();
    }
    else if (type == builtin_type_id_vector3)
    {
        m_defaultValVec3 = vec3::zero();
    }
    else if (type == builtin_type_id_vector4)
    {
        m_defaultValVec4 = vec4::zero();
    }
    else if (type == builtin_type_id_quaternion)
    {
        m_defaultValQuat = quat::identity();
    }
    else
    {
        m_defaultValUnsignedInt = 0;
    }
}

void field_definition::in_copy_construct_default_val(const field_definition& other)
{
    if (is_int())
    {
        m_defaultValSignedInt = other.m_defaultValSignedInt;
    }
    else if (is_uint())
    {
        m_defaultValUnsignedInt = other.m_defaultValUnsignedInt;
    }
    else if (is_floating())
    {
        m_defaultValFloating = other.m_defaultValFloating;
    }
    else if (is_char())
    {
        m_defaultValChar = other.m_defaultValChar;
    }
    else if (is_string())
    {
        new (&m_defaultValString) std::string(other.m_defaultValString);
    }
    else if (is_bool())
    {
        m_defaultValBool = other.m_defaultValBool;
    }
    else if (is_vec2())
    {
        new (&m_defaultValVec2) vec2(other.m_defaultValVec2);
    }
    else if (is_vec3())
    {
        new (&m_defaultValVec3) vec3(other.m_defaultValVec3);
    }
    else if (is_vec4())
    {
        new (&m_defaultValVec4) vec4(other.m_defaultValVec4);
    }
    else if (is_quat())
    {
        new (&m_defaultValQuat) quat(other.m_defaultValQuat);
    }
}

void field_definition::in_move_construct_default_val(field_definition&& other) noexcept
{
    if (is_int())
    {
        m_defaultValSignedInt = other.m_defaultValSignedInt;
    }
    else if (is_uint())
    {
        m_defaultValUnsignedInt = other.m_defaultValUnsignedInt;
    }
    else if (is_floating())
    {
        m_defaultValFloating = other.m_defaultValFloating;
    }
    else if (is_char())
    {
        m_defaultValChar = other.m_defaultValChar;
    }
    else if (is_string())
    {
        new (&m_defaultValString) std::string(
            std::move(other.m_defaultValString));
    }
    else if (is_bool())
    {
        m_defaultValBool = other.m_defaultValBool;
    }
    else if (is_vec2())
    {
        new (&m_defaultValVec2) vec2(
            std::move(other.m_defaultValVec2));
    }
    else if (is_vec3())
    {
        new (&m_defaultValVec3) vec3(
            std::move(other.m_defaultValVec3));
    }
    else if (is_vec4())
    {
        new (&m_defaultValVec4) vec4(
            std::move(other.m_defaultValVec4));
    }
    else if (is_quat())
    {
        new (&m_defaultValQuat) quat(
            std::move(other.m_defaultValQuat));
    }
}

std::size_t field_definition::get_real_alignment(
    const enum_definition_database& enumsDB,
    const struct_definition_database& structsDB,
    const std::size_t* builtinTypeAlignments) const
{
    // If this field actually has an alignment, just return it.
    if (alignment)
    {
        return alignment;
    }

    // Otherwise, compute alignment, starting with array types.
    std::string_view type(m_type);
    if (type == builtin_type_id_array)
    {
        // Dynamic-sized arrays
        if (!m_arrayCount)
        {
            return builtinTypeAlignments[builtin_type::array];
        }

        // Fixed-sized arrays.
        else
        {
            type = m_subtype;
        }
    }

    // Check all other built-in types.
    if (type == builtin_type_id_int8)
    {
        return builtinTypeAlignments[builtin_type::int8];
    }
    else if (type == builtin_type_id_int16)
    {
        return builtinTypeAlignments[builtin_type::int16];
    }
    else if (type == builtin_type_id_int32)
    {
        return builtinTypeAlignments[builtin_type::int32];
    }
    else if (type == builtin_type_id_int64)
    {
        return builtinTypeAlignments[builtin_type::int64];
    }
    else if (type == builtin_type_id_uint8)
    {
        return builtinTypeAlignments[builtin_type::uint8];
    }
    else if (type == builtin_type_id_uint16)
    {
        return builtinTypeAlignments[builtin_type::uint16];
    }
    else if (type == builtin_type_id_uint32)
    {
        return builtinTypeAlignments[builtin_type::uint32];
    }
    else if (type == builtin_type_id_uint64)
    {
        return builtinTypeAlignments[builtin_type::uint64];
    }
    else if (type == builtin_type_id_float32)
    {
        return builtinTypeAlignments[builtin_type::float32];
    }
    else if (type == builtin_type_id_float64)
    {
        return builtinTypeAlignments[builtin_type::float64];
    }
    else if (type == builtin_type_id_char)
    {
        return builtinTypeAlignments[builtin_type::_char];
    }
    else if (type == builtin_type_id_string)
    {
        return builtinTypeAlignments[builtin_type::string];
    }
    else if (type == builtin_type_id_bool)
    {
        return builtinTypeAlignments[builtin_type::_bool];
    }
    else if (type == builtin_type_id_object_reference)
    {
        return builtinTypeAlignments[builtin_type::object_reference];
    }
    else if (type == builtin_type_id_vector2)
    {
        return builtinTypeAlignments[builtin_type::vector2];
    }
    else if (type == builtin_type_id_vector3)
    {
        return builtinTypeAlignments[builtin_type::vector3];
    }
    else if (type == builtin_type_id_vector4)
    {
        return builtinTypeAlignments[builtin_type::vector4];
    }
    else if (type == builtin_type_id_quaternion)
    {
        return builtinTypeAlignments[builtin_type::quaternion];
    }
    
    // If this is NOT a built-in type, compute alignment for enums and/or structs.
    else
    {
        // Compute alignment for enum types.
        {
            const auto enumDef = enumsDB.get(type.data());
            if (enumDef)
            {
                return builtinTypeAlignments[
                    static_cast<builtin_type>(enumDef->type)];
            }
        }

        // Compute alignment for struct types.
        {
            const auto structDef = structsDB.get(type.data());
            if (structDef)
            {
                return structDef->get_real_alignment(
                    enumsDB, structsDB, builtinTypeAlignments);
            }
        }
        
        // Unknown type.
        throw std::runtime_error(
            "Cannot compute field alignment; unknown or unsupported type");
    }
}

void field_definition::set_default_val_int(std::intmax_t val) noexcept
{
    in_destruct_default_val();
    m_defaultValSignedInt = val;
}

void field_definition::set_default_val_uint(std::uintmax_t val) noexcept
{
    in_destruct_default_val();
    m_defaultValUnsignedInt = val;
}

void field_definition::set_default_val_floating(double val) noexcept
{
    in_destruct_default_val();
    m_defaultValFloating = val;
}

void field_definition::set_default_val_char(char val) noexcept
{
    in_destruct_default_val();
    m_defaultValChar = val;
}

void field_definition::set_default_val_string(std::string val) noexcept
{
    in_destruct_default_val();
    m_defaultValString = std::move(val);
}

void field_definition::set_default_val_bool(bool val) noexcept
{
    in_destruct_default_val();
    m_defaultValBool = val;
}

void field_definition::set_default_val_vec2(vec2 val) noexcept
{
    in_destruct_default_val();
    m_defaultValVec2 = std::move(val);
}

void field_definition::set_default_val_vec3(vec3 val) noexcept
{
    in_destruct_default_val();
    m_defaultValVec3 = std::move(val);
}

void field_definition::set_default_val_vec4(vec4 val) noexcept
{
    in_destruct_default_val();
    m_defaultValVec4 = std::move(val);
}

void field_definition::set_default_val_quat(quat val) noexcept
{
    in_destruct_default_val();
    m_defaultValQuat = std::move(val);
}

void field_definition::set_type(std::string type) noexcept
{
    m_type = std::move(type);
    m_subtype.clear();
    m_arrayCount = 0;
    in_destruct_default_val();
    in_set_default_val_for_type(m_type);
}

void field_definition::set_subtype(std::string subtype) noexcept
{
    m_subtype = std::move(subtype);
}

field_definition& field_definition::operator=(const field_definition& other)
{
    if (&other != this)
    {
        m_type = other.m_type;
        m_subtype = other.m_subtype;
        m_arrayCount = other.m_arrayCount;

        in_destruct_default_val();
        in_copy_construct_default_val(other);

        name = other.name;
        descriptions = other.descriptions;
        defaultDescription = other.defaultDescription;
        alignment = other.alignment;
    }
    
    return *this;
}

field_definition& field_definition::operator=(field_definition&& other) noexcept
{
    if (&other != this)
    {
        m_type = std::move(other.m_type);
        m_subtype = std::move(other.m_subtype);
        m_arrayCount = other.m_arrayCount;

        in_destruct_default_val();
        in_move_construct_default_val(std::move(other));

        name = std::move(other.name);
        descriptions = std::move(other.descriptions);
        defaultDescription = std::move(other.defaultDescription);
        alignment = other.alignment;
    }
    
    return *this;
}

field_definition::field_definition(std::string name) noexcept :
    name(std::move(name)) {}

field_definition::field_definition(std::string name, std::string desc) noexcept :
    name(std::move(name)),
    defaultDescription(std::move(defaultDescription)) {}

field_definition::field_definition(const field_definition& other) :
    m_type(other.m_type),
    m_subtype(other.m_subtype),
    m_arrayCount(other.m_arrayCount),
    name(other.name),
    descriptions(other.descriptions),
    defaultDescription(other.defaultDescription),
    alignment(other.alignment)
{
    in_copy_construct_default_val(other);
}

field_definition::field_definition(field_definition&& other) noexcept :
    m_type(std::move(other.m_type)),
    m_subtype(std::move(other.m_subtype)),
    m_arrayCount(other.m_arrayCount),
    name(std::move(other.name)),
    descriptions(std::move(other.descriptions)),
    defaultDescription(std::move(other.defaultDescription)),
    alignment(other.alignment)
{
    in_move_construct_default_val(std::move(other));
}

std::size_t struct_definition::get_real_alignment(
    const enum_definition_database& enumsDB,
    const struct_definition_database& structsDB,
    const std::size_t* builtinTypeAlignments) const
{
    // If this struct actually has an alignment, just return it.
    if (alignment)
    {
        return alignment;
    }

    // Otherwise, we need to compute the actual ("real") alignment.
    // Start by recursively determining alignment of any parent structs.
    std::size_t largestAlignment = 1; // NOTE: Minimum valid "real" alignment is 1.
    if (!parent.empty())
    {
        const auto parentStructDef = structsDB.get(parent);
        if (!parentStructDef)
        {
            throw std::runtime_error("Could not find parent struct type in database");
        }

        largestAlignment = parentStructDef->get_real_alignment(
            enumsDB, structsDB, builtinTypeAlignments);
    }

    // Iterate through each of the struct's fields and
    // determine the largest real alignment required.
    for (const auto& it : fields)
    {
        const auto fieldAlignment = it.get_real_alignment(
            enumsDB, structsDB, builtinTypeAlignments);

        if (fieldAlignment > largestAlignment)
        {
            largestAlignment = fieldAlignment;
        }
    }

    return largestAlignment;
}

std::intmax_t field_reader::read_int(std::string_view type, std::size_t alignment)
{
    // NOTE: Checks are sorted based on supposed frequency.
    if (type == builtin_type_id_int32)
    {
        return read<s32>(alignment);
    }
    else if (type == builtin_type_id_int8)
    {
        return read<s8>(alignment);
    }
    else if (type == builtin_type_id_int16)
    {
        return read<s16>(alignment);
    }
    else if (type == builtin_type_id_int64)
    {
        return read<s64>(alignment);
    }
    else
    {
        throw std::runtime_error("Unknown int type");
    }
}

std::uintmax_t field_reader::read_uint(std::string_view type, std::size_t alignment)
{
    // NOTE: Checks are sorted based on supposed frequency.
    if (type == builtin_type_id_uint32)
    {
        return read<u32>(alignment);
    }
    else if (type == builtin_type_id_uint8)
    {
        return read<u8>(alignment);
    }
    else if (type == builtin_type_id_uint16)
    {
        return read<u16>(alignment);
    }
    else if (type == builtin_type_id_uint64)
    {
        return read<u64>(alignment);
    }
    else
    {
        throw std::runtime_error("Unknown uint type");
    }
}

std::uintmax_t field_reader::read_integral(integral_type type, std::size_t alignment)
{
    switch (type)
    {
    case integral_type::int8:
        return read<s8>(alignment);

    case integral_type::int16:
        return read<s16>(alignment);

    case integral_type::int32:
        return read<s32>(alignment);

    case integral_type::int64:
        return read<s64>(alignment);

    case integral_type::uint8:
        return read<u8>(alignment);

    case integral_type::uint16:
        return read<u16>(alignment);

    case integral_type::uint32:
        return read<u32>(alignment);

    case integral_type::uint64:
        return read<u64>(alignment);

    default:
        throw std::runtime_error("Invalid integral_type");
    }
}

double field_reader::read_floating(std::string_view type, std::size_t alignment)
{
    if (type == builtin_type_id_float32)
    {
        return read<float>(alignment);
    }
    else if (type == builtin_type_id_float64)
    {
        return read<double>(alignment);
    }
    else
    {
        throw std::runtime_error("Unknown floating type");
    }
}

void write_int(stream& stream, std::string_view type,
    std::intmax_t val, std::size_t alignment)
{
    // NOTE: Checks are sorted based on supposed frequency.
    if (type == builtin_type_id_int32)
    {
        stream.write_obj(static_cast<s32>(val), alignment);
    }
    else if (type == builtin_type_id_int8)
    {
        stream.write_obj(static_cast<s8>(val), alignment);
    }
    else if (type == builtin_type_id_int16)
    {
        stream.write_obj(static_cast<s16>(val), alignment);
    }
    else if (type == builtin_type_id_int64)
    {
        stream.write_obj(static_cast<s64>(val), alignment);
    }
    else
    {
        throw std::runtime_error("Unknown int type");
    }
}

void write_uint(stream& stream, std::string_view type,
    std::uintmax_t val, std::size_t alignment)
{
    // NOTE: Checks are sorted based on supposed frequency.
    if (type == builtin_type_id_uint32)
    {
        stream.write_obj(static_cast<u32>(val), alignment);
    }
    else if (type == builtin_type_id_uint8)
    {
        stream.write_obj(static_cast<u8>(val), alignment);
    }
    else if (type == builtin_type_id_uint16)
    {
        stream.write_obj(static_cast<u16>(val), alignment);
    }
    else if (type == builtin_type_id_uint64)
    {
        stream.write_obj(static_cast<u64>(val), alignment);
    }
    else
    {
        throw std::runtime_error("Unknown uint type");
    }
}

void write_integral(stream& stream, integral_type type,
    std::uintmax_t val, std::size_t alignment)
{
    switch (type)
    {
    case integral_type::int8:
        stream.write_obj(static_cast<s8>(val), alignment);
        break;

    case integral_type::int16:
        stream.write_obj(static_cast<s16>(val), alignment);
        break;

    case integral_type::int32:
        stream.write_obj(static_cast<s32>(val), alignment);
        break;

    case integral_type::int64:
        stream.write_obj(static_cast<s64>(val), alignment);
        break;

    case integral_type::uint8:
        stream.write_obj(static_cast<u8>(val), alignment);
        break;

    case integral_type::uint16:
        stream.write_obj(static_cast<u16>(val), alignment);
        break;

    case integral_type::uint32:
        stream.write_obj(static_cast<u32>(val), alignment);
        break;

    case integral_type::uint64:
        stream.write_obj(static_cast<u64>(val), alignment);
        break;

    default:
        throw std::runtime_error("Invalid integral_type");
    }
}

void write_floating(stream& stream, std::string_view type,
    double val, std::size_t alignment)
{
    if (type == builtin_type_id_float32)
    {
        stream.write_obj(static_cast<float>(val), alignment);
    }
    else if (type == builtin_type_id_float64)
    {
        stream.write_obj(static_cast<double>(val), alignment);
    }
    else
    {
        throw std::runtime_error("Unknown floating type");
    }
}
} // reflect
} // hl
