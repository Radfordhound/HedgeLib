#ifndef HL_REFLECT_H_INCLUDED
#define HL_REFLECT_H_INCLUDED
#include "hl_radix_tree.h"
#include "io/hl_stream.h"
#include "hl_math.h"
#include <string_view>

namespace hl
{
namespace reflect
{
class enum_definition;
class struct_definition;

using enum_definition_database = radix_tree<enum_definition>;
using struct_definition_database = radix_tree<struct_definition>;

enum class integral_type
{
    int8,
    int16,
    int32,
    int64,

    uint8,
    uint16,
    uint32,
    uint64,
};

HL_API extern const char* const integral_type_ids[];

constexpr integral_type get_integral_type(std::string_view str)
{
    if (str == "int8")
    {
        return integral_type::int8;
    }
    else if (str == "int16")
    {
        return integral_type::int16;
    }
    else if (str == "int32")
    {
        return integral_type::int32;
    }
    else if (str == "int64")
    {
        return integral_type::int64;
    }
    else if (str == "uint8")
    {
        return integral_type::uint8;
    }
    else if (str == "uint16")
    {
        return integral_type::uint16;
    }
    else if (str == "uint32")
    {
        return integral_type::uint32;
    }
    else if (str == "uint64")
    {
        return integral_type::uint64;
    }
    else
    {
        throw std::runtime_error("Unsupported integral type");
    }
}

constexpr bool is_signed_type(integral_type type) noexcept
{
    switch (type)
    {
    case integral_type::int8:
    case integral_type::int16:
    case integral_type::int32:
    case integral_type::int64:
        return true;

    default:
        return false;
    }
}

namespace internal
{
namespace _in_builtin_type
{
enum in_builtin_type
{
    int8,
    int16,
    int32,
    int64,

    uint8,
    uint16,
    uint32,
    uint64,

    float32,
    float64,

    _char,
    string,

    _bool,
    array,
    object_reference,
    vector2,
    vector3,
    vector4,
    quaternion,
};
} // in_builtin_type
} // internal

using builtin_type = internal::_in_builtin_type::in_builtin_type;

inline constexpr char builtin_type_id_int8[]                = "int8";
inline constexpr char builtin_type_id_int16[]               = "int16";
inline constexpr char builtin_type_id_int32[]               = "int32";
inline constexpr char builtin_type_id_int64[]               = "int64";

inline constexpr char builtin_type_id_uint8[]               = "uint8";
inline constexpr char builtin_type_id_uint16[]              = "uint16";
inline constexpr char builtin_type_id_uint32[]              = "uint32";
inline constexpr char builtin_type_id_uint64[]              = "uint64";

inline constexpr char builtin_type_id_float32[]             = "float32";
inline constexpr char builtin_type_id_float64[]             = "float64";

inline constexpr char builtin_type_id_char[]                = "char";
inline constexpr char builtin_type_id_string[]              = "string";

inline constexpr char builtin_type_id_bool[]                = "bool";
inline constexpr char builtin_type_id_array[]               = "array";
inline constexpr char builtin_type_id_object_reference[]    = "object_reference";
inline constexpr char builtin_type_id_vector2[]             = "vector2";
inline constexpr char builtin_type_id_vector3[]             = "vector3";
inline constexpr char builtin_type_id_vector4[]             = "vector4";
inline constexpr char builtin_type_id_quaternion[]          = "quaternion";

HL_API extern const char* const builtin_type_ids[];

constexpr bool is_int_type(std::string_view type)
{
    return (
        type == builtin_type_id_int8 ||
        type == builtin_type_id_int16 ||
        type == builtin_type_id_int32 ||
        type == builtin_type_id_int64
    );
}

constexpr bool is_uint_type(std::string_view type)
{
    return (
        type == builtin_type_id_uint8 ||
        type == builtin_type_id_uint16 ||
        type == builtin_type_id_uint32 ||
        type == builtin_type_id_uint64
    );
}

constexpr bool is_floating_type(std::string_view type)
{
    return (type == builtin_type_id_float32 ||
        type == builtin_type_id_float64);
}

constexpr bool is_char_type(std::string_view type)
{
    return type == builtin_type_id_char;
}

constexpr bool is_string_type(std::string_view type)
{
    return type == builtin_type_id_string;
}

constexpr bool is_bool_type(std::string_view type)
{
    return type == builtin_type_id_bool;
}

constexpr bool is_array_type(std::string_view type)
{
    return type == builtin_type_id_array;
}

constexpr bool is_object_reference_type(std::string_view type)
{
    return type == builtin_type_id_object_reference;
}

constexpr bool is_vec2_type(std::string_view type)
{
    return type == builtin_type_id_vector2;
}

constexpr bool is_vec3_type(std::string_view type)
{
    return type == builtin_type_id_vector3;
}

constexpr bool is_vec4_type(std::string_view type)
{
    return type == builtin_type_id_vector4;
}

constexpr bool is_quat_type(std::string_view type)
{
    return type == builtin_type_id_quaternion;
}

constexpr builtin_type get_builtin_type(std::string_view str)
{
    // NOTE: We compare manually instead of using a for loop to take
    // advantage of compiler optimizations that can be done when we
    // already know the lengths of the strings we're comparing.

    if (str == builtin_type_id_int8)
    {
        return builtin_type::int8;
    }
    else if (str == builtin_type_id_int16)
    {
        return builtin_type::int16;
    }
    else if (str == builtin_type_id_int32)
    {
        return builtin_type::int32;
    }
    else if (str == builtin_type_id_int64)
    {
        return builtin_type::int64;
    }
    else if (str == builtin_type_id_uint8)
    {
        return builtin_type::uint8;
    }
    else if (str == builtin_type_id_uint16)
    {
        return builtin_type::uint16;
    }
    else if (str == builtin_type_id_uint32)
    {
        return builtin_type::uint32;
    }
    else if (str == builtin_type_id_uint64)
    {
        return builtin_type::uint64;
    }
    else if (str == builtin_type_id_float32)
    {
        return builtin_type::float32;
    }
    else if (str == builtin_type_id_float64)
    {
        return builtin_type::float64;
    }
    else if (str == builtin_type_id_char)
    {
        return builtin_type::_char;
    }
    else if (str == builtin_type_id_string)
    {
        return builtin_type::string;
    }
    else if (str == builtin_type_id_bool)
    {
        return builtin_type::_bool;
    }
    else if (str == builtin_type_id_object_reference)
    {
        return builtin_type::object_reference;
    }
    else if (str == builtin_type_id_vector2)
    {
        return builtin_type::vector2;
    }
    else if (str == builtin_type_id_vector3)
    {
        return builtin_type::vector3;
    }
    else if (str == builtin_type_id_vector4)
    {
        return builtin_type::vector4;
    }
    else if (str == builtin_type_id_quaternion)
    {
        return builtin_type::quaternion;
    }
    else
    {
        throw std::runtime_error("Unsupported built-in type");
    }
}

class enum_value_definition
{
public:
    std::uintmax_t value = 0;
    radix_tree<std::string> descriptions;
    std::string defaultDescription;

    enum_value_definition() noexcept = default;

    enum_value_definition(std::uintmax_t val) noexcept :
        value(val) {}

    enum_value_definition(std::uintmax_t val, std::string defaultDesc) :
        value(val),
        defaultDescription(std::move(defaultDesc)) {}
};

class enum_definition
{
public:
    integral_type type = integral_type::int32;
    radix_tree<enum_value_definition> values;

    inline bool is_signed() const noexcept
    {
        return is_signed_type(type);
    }

    HL_API const char* get_name_of_value(std::uintmax_t val) const;

    template<typename T>
    T get_value(const char* name) const
    {
        const auto& enumValDef = values.at(name);
        return (is_signed()) ? // preserve signedness
            static_cast<T>(static_cast<std::intmax_t>(enumValDef.value)) :
            static_cast<T>(enumValDef.value);
    }

    template<typename T>
    inline T get_value(const std::string& name) const
    {
        return get_value<T>(name.c_str());
    }
};

class field_definition
{
    std::string m_type = builtin_type_id_float32;
    std::string m_subtype;
    std::size_t m_arrayCount = 0;

    union
    {
        std::intmax_t m_defaultValSignedInt;
        std::uintmax_t m_defaultValUnsignedInt;
        double m_defaultValFloating;

        char m_defaultValChar;
        std::string m_defaultValString;

        bool m_defaultValBool;
        vec2 m_defaultValVec2;
        vec3 m_defaultValVec3;
        vec4 m_defaultValVec4;
        quat m_defaultValQuat;
    };

    HL_API void in_destruct_default_val() noexcept;

    HL_API void in_default_construct_default_val(const std::string& type) noexcept;

    void in_copy_construct_default_val(const field_definition& other);

    void in_move_construct_default_val(field_definition&& other) noexcept;

public:
    std::string name;
    radix_tree<std::string> descriptions;
    std::string defaultDescription;
    std::size_t alignment = 0;

    inline const std::string& type() const noexcept
    {
        return m_type;
    }

    inline const std::string& subtype() const noexcept
    {
        return m_subtype;
    }

    inline std::size_t array_count() const noexcept
    {
        return m_arrayCount;
    }

    inline std::intmax_t default_val_int() const noexcept
    {
        return m_defaultValSignedInt;
    }

    inline std::uintmax_t default_val_uint() const noexcept
    {
        return m_defaultValUnsignedInt;
    }

    inline double default_val_floating() const noexcept
    {
        return m_defaultValFloating;
    }

    inline char default_val_char() const noexcept
    {
        return m_defaultValChar;
    }

    inline const std::string& default_val_string() const noexcept
    {
        return m_defaultValString;
    }

    inline bool default_val_bool() const noexcept
    {
        return m_defaultValBool;
    }

    inline const vec2& default_val_vec2() const noexcept
    {
        return m_defaultValVec2;
    }

    inline const vec3& default_val_vec3() const noexcept
    {
        return m_defaultValVec3;
    }

    inline const vec4& default_val_vec4() const noexcept
    {
        return m_defaultValVec4;
    }

    inline const quat& default_val_quat() const noexcept
    {
        return m_defaultValQuat;
    }

    inline bool is_int() const
    {
        return is_int_type(m_type);
    }

    inline bool is_uint() const
    {
        return is_uint_type(m_type);
    }

    inline bool is_floating() const
    {
        return is_floating_type(m_type);
    }

    inline bool is_char() const
    {
        return is_char_type(m_type);
    }

    inline bool is_string() const
    {
        return is_string_type(m_type);
    }

    inline bool is_bool() const
    {
        return is_bool_type(m_type);
    }

    inline bool is_array() const
    {
        return is_array_type(m_type);
    }

    inline bool is_object_reference() const
    {
        return is_object_reference_type(m_type);
    }

    inline bool is_vec2() const
    {
        return is_vec2_type(m_type);
    }

    inline bool is_vec3() const
    {
        return is_vec3_type(m_type);
    }

    inline bool is_vec4() const
    {
        return is_vec4_type(m_type);
    }

    inline bool is_quat() const
    {
        return is_quat_type(m_type);
    }

    HL_API std::size_t get_real_alignment(
        const enum_definition_database& enumsDB,
        const struct_definition_database& structsDB,
        const std::size_t* builtinTypeAlignments) const;

    HL_API void set_default_val_int(std::intmax_t val) noexcept;

    HL_API void set_default_val_uint(std::uintmax_t val) noexcept;

    HL_API void set_default_val_floating(double val) noexcept;

    HL_API void set_default_val_char(char val) noexcept;

    HL_API void set_default_val_string(std::string val) noexcept;

    HL_API void set_default_val_bool(bool val) noexcept;

    HL_API void set_default_val_vec2(vec2 val) noexcept;

    HL_API void set_default_val_vec3(vec3 val) noexcept;

    HL_API void set_default_val_vec4(vec4 val) noexcept;

    HL_API void set_default_val_quat(quat val) noexcept;

    HL_API void set_type(std::string type) noexcept;

    // TODO: set_type enum type and custom default value.

    //HL_API void set_type(std::string enumType, std::uintmax_t defaultVal) noexcept
    //{
    //    in_destruct_default_val();
    //    m_type = std::move(enumType);
    //    m_subtype.clear();
    //    m_arrayCount = 0;
    //    m_defaultValIntegral = defaultVal;
    //}

    template<builtin_type Type>
    std::enable_if_t<Type == builtin_type::int8 || Type == builtin_type::int16 ||
        Type == builtin_type::int32 || Type == builtin_type::int64>
        set_type(std::intmax_t defaultVal = 0) noexcept
    {
        in_destruct_default_val();
        m_type = builtin_type_ids[Type];
        m_subtype.clear();
        m_arrayCount = 0;
        m_defaultValSignedInt = defaultVal;
    }

    template<builtin_type Type>
    std::enable_if_t<Type == builtin_type::uint8 || Type == builtin_type::uint16 ||
        Type == builtin_type::uint32 || Type == builtin_type::uint64>
        set_type(std::uintmax_t defaultVal = 0) noexcept
    {
        in_destruct_default_val();
        m_type = builtin_type_ids[Type];
        m_subtype.clear();
        m_arrayCount = 0;
        m_defaultValUnsignedInt = defaultVal;
    }

    template<builtin_type Type>
    std::enable_if_t<Type == builtin_type::float32 || Type == builtin_type::float64>
        set_type(double defaultVal = 0.0) noexcept
    {
        in_destruct_default_val();
        m_type = builtin_type_ids[Type];
        m_subtype.clear();
        m_arrayCount = 0;
        m_defaultValFloating = defaultVal;
    }

    template<builtin_type Type>
    std::enable_if_t<Type == builtin_type::_char>
        set_type(char defaultVal = '\0') noexcept
    {
        in_destruct_default_val();
        m_type = builtin_type_ids[Type];
        m_subtype.clear();
        m_arrayCount = 0;
        m_defaultValChar = defaultVal;
    }

    template<builtin_type Type>
    std::enable_if_t<Type == builtin_type::string>
        set_type(std::string defaultVal = "") noexcept
    {
        in_destruct_default_val();
        m_type = builtin_type_ids[Type];
        m_subtype.clear();
        m_arrayCount = 0;
        m_defaultValString = std::move(defaultVal);
    }

    template<builtin_type Type>
    std::enable_if_t<Type == builtin_type::_bool>
        set_type(bool defaultVal = false) noexcept
    {
        in_destruct_default_val();
        m_type = builtin_type_ids[Type];
        m_subtype.clear();
        m_arrayCount = 0;
        m_defaultValBool = defaultVal;
    }

    template<builtin_type Type>
    std::enable_if_t<Type == builtin_type::object_reference>
        set_type() noexcept
    {
        in_destruct_default_val();
        m_type = builtin_type_ids[Type];
        m_subtype.clear();
        m_arrayCount = 0;
    }

    template<builtin_type Type>
    std::enable_if_t<Type == builtin_type::vector2>
        set_type(vec2 defaultVal = vec2::zero()) noexcept
    {
        in_destruct_default_val();
        m_type = builtin_type_ids[Type];
        m_subtype.clear();
        m_arrayCount = 0;
        m_defaultValVec2 = std::move(defaultVal);
    }

    template<builtin_type Type>
    std::enable_if_t<Type == builtin_type::vector3>
        set_type(vec3 defaultVal = vec3::zero()) noexcept
    {
        in_destruct_default_val();
        m_type = builtin_type_ids[Type];
        m_subtype.clear();
        m_arrayCount = 0;
        m_defaultValVec3 = std::move(defaultVal);
    }

    template<builtin_type Type>
    std::enable_if_t<Type == builtin_type::vector4>
        set_type(vec4 defaultVal = vec4::zero()) noexcept
    {
        in_destruct_default_val();
        m_type = builtin_type_ids[Type];
        m_subtype.clear();
        m_arrayCount = 0;
        m_defaultValVec4 = std::move(defaultVal);
    }

    template<builtin_type Type>
    std::enable_if_t<Type == builtin_type::quaternion>
        set_type(quat defaultVal = quat::identity()) noexcept
    {
        in_destruct_default_val();
        m_type = builtin_type_ids[Type];
        m_subtype.clear();
        m_arrayCount = 0;
        m_defaultValQuat = std::move(defaultVal);
    }

    template<builtin_type Type>
    std::enable_if_t<Type == builtin_type::array>
        set_type(std::string subtype, std::size_t count = 0) noexcept
    {
        in_destruct_default_val();
        m_type = builtin_type_ids[Type];
        m_subtype = std::move(subtype);
        m_arrayCount = count;
        in_default_construct_default_val(m_subtype);
    }

    // TODO: set_type array with enum subtype and custom default value.

    template<builtin_type Type, builtin_type SubType>
    std::enable_if_t<Type == builtin_type::array && (
        SubType == builtin_type::int8 || SubType == builtin_type::int16 ||
        SubType == builtin_type::int32 || SubType == builtin_type::int64)>
        set_type(std::intmax_t defaultVal = 0, std::size_t count = 0) noexcept
    {
        in_destruct_default_val();
        m_type = builtin_type_ids[Type];
        m_subtype = builtin_type_ids[SubType];
        m_arrayCount = count;
        m_defaultValSignedInt = defaultVal;
    }

    template<builtin_type Type, builtin_type SubType>
    std::enable_if_t<Type == builtin_type::array && (
        SubType == builtin_type::uint8 || SubType == builtin_type::uint16 ||
        SubType == builtin_type::uint32 || SubType == builtin_type::uint64)>
        set_type(std::uintmax_t defaultVal = 0, std::size_t count = 0) noexcept
    {
        in_destruct_default_val();
        m_type = builtin_type_ids[Type];
        m_subtype = builtin_type_ids[SubType];
        m_arrayCount = count;
        m_defaultValUnsignedInt = defaultVal;
    }

    template<builtin_type Type, builtin_type SubType>
    std::enable_if_t<Type == builtin_type::array &&
        (SubType == builtin_type::float32 || SubType == builtin_type::float64)>
        set_type(double defaultVal = 0.0, std::size_t count = 0) noexcept
    {
        in_destruct_default_val();
        m_type = builtin_type_ids[Type];
        m_subtype = builtin_type_ids[SubType];
        m_arrayCount = count;
        m_defaultValFloating = defaultVal;
    }

    template<builtin_type Type, builtin_type SubType>
    std::enable_if_t<Type == builtin_type::array &&
        SubType == builtin_type::_char>
        set_type(char defaultVal = '\0', std::size_t count = 0) noexcept
    {
        in_destruct_default_val();
        m_type = builtin_type_ids[Type];
        m_subtype = builtin_type_ids[SubType];
        m_arrayCount = count;
        m_defaultValChar = defaultVal;
    }

    template<builtin_type Type, builtin_type SubType>
    std::enable_if_t<Type == builtin_type::array &&
        SubType == builtin_type::string>
        set_type(std::string defaultVal = "", std::size_t count = 0) noexcept
    {
        in_destruct_default_val();
        m_type = builtin_type_ids[Type];
        m_subtype = builtin_type_ids[SubType];
        m_arrayCount = count;
        m_defaultValString = std::move(defaultVal);
    }

    template<builtin_type Type, builtin_type SubType>
    std::enable_if_t<Type == builtin_type::array &&
        SubType == builtin_type::_bool>
        set_type(bool defaultVal = false, std::size_t count = 0) noexcept
    {
        in_destruct_default_val();
        m_type = builtin_type_ids[Type];
        m_subtype = builtin_type_ids[SubType];
        m_arrayCount = count;
        m_defaultValBool = defaultVal;
    }

    template<builtin_type Type, builtin_type SubType>
    std::enable_if_t<Type == builtin_type::array &&
        SubType == builtin_type::object_reference>
        set_type(std::size_t count = 0) noexcept
    {
        in_destruct_default_val();
        m_type = builtin_type_ids[Type];
        m_subtype = builtin_type_ids[SubType];
        m_arrayCount = count;
    }

    template<builtin_type Type, builtin_type SubType>
    std::enable_if_t<Type == builtin_type::array &&
        SubType == builtin_type::vector2>
        set_type(vec2 defaultVal = vec2::zero(), std::size_t count = 0) noexcept
    {
        in_destruct_default_val();
        m_type = builtin_type_ids[Type];
        m_subtype = builtin_type_ids[SubType];
        m_arrayCount = count;
        m_defaultValVec2 = std::move(defaultVal);
    }

    template<builtin_type Type, builtin_type SubType>
    std::enable_if_t<Type == builtin_type::array &&
        SubType == builtin_type::vector3>
        set_type(vec3 defaultVal = vec3::zero(), std::size_t count = 0) noexcept
    {
        in_destruct_default_val();
        m_type = builtin_type_ids[Type];
        m_subtype = builtin_type_ids[SubType];
        m_arrayCount = count;
        m_defaultValVec3 = std::move(defaultVal);
    }

    template<builtin_type Type, builtin_type SubType>
    std::enable_if_t<Type == builtin_type::array &&
        SubType == builtin_type::vector4>
        set_type(vec4 defaultVal = vec4::zero(), std::size_t count = 0) noexcept
    {
        in_destruct_default_val();
        m_type = builtin_type_ids[Type];
        m_subtype = builtin_type_ids[SubType];
        m_arrayCount = count;
        m_defaultValVec4 = std::move(defaultVal);
    }

    template<builtin_type Type, builtin_type SubType>
    std::enable_if_t<Type == builtin_type::array &&
        SubType == builtin_type::quaternion>
        set_type(quat defaultVal = quat::identity(), std::size_t count = 0) noexcept
    {
        in_destruct_default_val();
        m_type = builtin_type_ids[Type];
        m_subtype = builtin_type_ids[SubType];
        m_arrayCount = count;
        m_defaultValQuat = std::move(defaultVal);
    }

    HL_API void set_subtype(std::string subtype) noexcept;

    inline void set_array_count(std::size_t count) noexcept
    {
        m_arrayCount = count;
    }

    HL_API field_definition& operator=(const field_definition& other);

    HL_API field_definition& operator=(field_definition&& other) noexcept;

    inline field_definition() noexcept :
        m_defaultValFloating(0.0) {}

    HL_API field_definition(std::string name) noexcept;

    HL_API field_definition(std::string name, std::string desc) noexcept;

    HL_API field_definition(const field_definition& other);

    HL_API field_definition(field_definition&& other) noexcept;

    inline ~field_definition()
    {
        in_destruct_default_val();
    }
};

class struct_definition
{
public:
    std::string parent;
    std::size_t alignment = 0;
    std::vector<field_definition> fields;

    HL_API std::size_t get_real_alignment(
        const enum_definition_database& enumsDB,
        const struct_definition_database& structsDB,
        const std::size_t* builtinTypeAlignments) const;
};

class field_reader
{
    const void* m_rawData;
    std::size_t m_totalBytesRead = 0;

public:
    inline void align(std::size_t alignment) noexcept
    {
        m_totalBytesRead = hl::align(m_totalBytesRead, alignment);
    }

    template<typename T = void>
    inline const T* peek() const noexcept
    {
        return ptradd<T>(m_rawData, m_totalBytesRead);
    }

    inline void jump_ahead(std::size_t amount)
    {
        m_totalBytesRead += amount;
    }

    inline void jump_behind(std::size_t amount)
    {
        m_totalBytesRead -= amount;
    }

    template<typename T>
    T read()
    {
        const auto ptr = peek<T>();
        jump_ahead(sizeof(T));
        return *ptr;
    }

    template<typename T>
    T read(std::size_t alignment)
    {
        if constexpr (std::is_void_v<T>)
        {
            align(alignment);
        }
        else
        {
            align((alignment) ? alignment : alignof(T));
        }

        return read<T>();
    }

    HL_API std::intmax_t read_int(std::string_view type, std::size_t alignment = 1);

    HL_API std::uintmax_t read_uint(std::string_view type, std::size_t alignment = 1);

    HL_API std::uintmax_t read_integral(integral_type type, std::size_t alignment = 1);

    HL_API double read_floating(std::string_view type, std::size_t alignment = 1);

    inline field_reader(const void* rawData) noexcept :
        m_rawData(rawData) {}
};

HL_API void write_int(stream& stream, std::string_view type,
    std::intmax_t val, std::size_t alignment = 1);

HL_API void write_uint(stream& stream, std::string_view type,
    std::uintmax_t val, std::size_t alignment = 1);

HL_API void write_integral(stream& stream, integral_type type,
    std::uintmax_t val, std::size_t alignment = 1);

HL_API void write_floating(stream& stream, std::string_view type,
    double val, std::size_t alignment = 1);
} // reflect
} // hl
#endif
