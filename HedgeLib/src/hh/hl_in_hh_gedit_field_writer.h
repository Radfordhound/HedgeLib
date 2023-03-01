#ifndef HL_IN_HH_GEDIT_FIELD_WRITER_H_INCLUDED
#define HL_IN_HH_GEDIT_FIELD_WRITER_H_INCLUDED

#include "hl_in_hh_gedit.h"
#include "hedgelib/sets/hl_set_obj_type.h"
#include "hedgelib/sets/hl_hson.h"

namespace hl
{
namespace hh
{
namespace gedit
{
namespace internal
{
template<typename T>
T in_as_obj_ref(const std::string& guidStr);

template<typename WriterType, typename RawObjectIDType,
    template<typename> typename RawOffsetType,
    template<typename> typename RawArrayType>
class in_field_writer
{
    inline static constexpr const std::size_t* in_builtin_type_alignments =
        internal::in_builtin_type_alignments<
            RawObjectIDType, RawOffsetType, RawArrayType>;

    WriterType* m_writer;
    const set_object_type_database* m_objTypeDB;
    bool m_tailEndAlignParentStructs;

    /**
     * @brief Ensures that the given HSON parameter is of the given type.
     * 
     * @param hsonParam The HSON parameter to validate.
     * @param hsonParamType The type the HSON parameter must be of.
     */
    static inline void in_validate_param_type(
        const hson::parameter& hsonParam,
        hson::parameter_type hsonParamType)
    {
        if (hsonParam.type() != hsonParamType)
        {
            throw std::runtime_error("Invalid HSON parameter type");
        }
    }

    template<typename T>
    auto in_write_primitive(const hson::parameter& hsonParam, std::size_t fieldAlignment = 0) ->
        std::enable_if_t<std::is_same_v<T, bool>>
    {
        // Write bool.
        in_validate_param_type(hsonParam, hson::parameter_type::boolean);
        const auto val = static_cast<u8>(hsonParam.value_bool());
        m_writer->write_obj(val, fieldAlignment);
    }

    template<typename T>
    auto in_write_primitive(const hson::parameter& hsonParam, std::size_t fieldAlignment = 0) ->
        std::enable_if_t<std::is_same_v<T, char>>
    {
        // Write char.
        in_validate_param_type(hsonParam, hson::parameter_type::string);
        const auto val = static_cast<u8>((hsonParam.value_string().empty()) ?
            char() : hsonParam.value_string()[0]);

        m_writer->write_obj(val, fieldAlignment);
    }

    template<typename T>
    auto in_write_primitive(const hson::parameter& hsonParam, std::size_t fieldAlignment = 0) ->
        std::enable_if_t<std::is_same_v<T, s8> || std::is_same_v<T, s16> ||
            std::is_same_v<T, s32> || std::is_same_v<T, s64>>
    {
        // Write signed integer.
        auto val = static_cast<T>(hsonParam.get_value_as_int());
        m_writer->swap_and_write_obj(val, fieldAlignment);
    }

    template<typename T>
    auto in_write_primitive(const hson::parameter& hsonParam, std::size_t fieldAlignment = 0) ->
        std::enable_if_t<std::is_same_v<T, u8> || std::is_same_v<T, u16> ||
            std::is_same_v<T, u32> || std::is_same_v<T, u64>>
    {
        // Write unsigned integer.
        auto val = static_cast<T>(hsonParam.get_value_as_uint());
        m_writer->swap_and_write_obj(val, fieldAlignment);
    }

    template<typename T>
    auto in_write_primitive(const hson::parameter& hsonParam, std::size_t fieldAlignment = 0) ->
        std::enable_if_t<std::is_same_v<T, float> || std::is_same_v<T, double>>
    {
        // Write floating point.
        auto val = static_cast<T>(hsonParam.get_value_as_floating());
        m_writer->swap_and_write_obj(val, fieldAlignment);
    }

    template<typename T>
    void in_write_vector(const hson::parameter& hsonParam,
        std::size_t fieldAlignment = 0, T defaultVal = T::zero())
    {
        in_validate_param_type(hsonParam, hson::parameter_type::array);

        // Copy over HSON array values.
        const auto& hsonParamValArr = hsonParam.value_array();
        const auto minCount = std::min<std::size_t>(
            hsonParamValArr.size(), T::size());

        for (std::size_t i = 0; i < minCount; ++i)
        {
            const auto& hsonArrElem = hsonParamValArr[i];
            defaultVal[i] = static_cast<float>(hsonArrElem.get_value_as_floating());
        }

        // Write vector to stream.
        m_writer->swap_and_write_obj(defaultVal, fieldAlignment);

        // Do tail-end alignment for vectors, since they are structs.
        if (fieldAlignment)
        {
            m_writer->pad(fieldAlignment);
        }
    }

    template<typename T>
    void in_write_enum(const hson::parameter& hsonParam,
        const reflect::enum_definition& enumDef, std::size_t fieldAlignment = 0)
    {
        // Get enum value.
        T val;
        if (hsonParam.type() == hson::parameter_type::string)
        {
            val = enumDef.get_value<T>(hsonParam.value_string());
        }
        else if (hsonParam.type() == hson::parameter_type::unsigned_integer)
        {
            val = static_cast<T>(hsonParam.value_uint());
        }
        else if (hsonParam.type() == hson::parameter_type::signed_integer)
        {
            val = static_cast<T>(hsonParam.value_int());
        }
        else
        {
            throw std::runtime_error("Invalid HSON parameter type");
        }

        // Write enum value.
        m_writer->swap_and_write_obj(val, fieldAlignment);
    }

    void in_write_enum(const hson::parameter& hsonParam,
        const reflect::enum_definition& enumDef, std::size_t fieldAlignment = 0)
    {
        switch (enumDef.type)
        {
        case reflect::integral_type::int8:
            in_write_enum<s8>(hsonParam, enumDef, fieldAlignment);
            return;

        case reflect::integral_type::int16:
            in_write_enum<s16>(hsonParam, enumDef, fieldAlignment);
            return;

        case reflect::integral_type::int32:
            in_write_enum<s32>(hsonParam, enumDef, fieldAlignment);
            return;

        case reflect::integral_type::int64:
            in_write_enum<s64>(hsonParam, enumDef, fieldAlignment);
            return;

        case reflect::integral_type::uint8:
            in_write_enum<u8>(hsonParam, enumDef, fieldAlignment);
            return;

        case reflect::integral_type::uint16:
            in_write_enum<u16>(hsonParam, enumDef, fieldAlignment);
            return;

        case reflect::integral_type::uint32:
            in_write_enum<u32>(hsonParam, enumDef, fieldAlignment);
            return;

        case reflect::integral_type::uint64:
            in_write_enum<u64>(hsonParam, enumDef, fieldAlignment);
            return;

        default:
            throw std::runtime_error("Invalid enum integral type");
        }
    }

    void in_write_element(const hson::parameter& hsonParam,
        const reflect::field_definition& defaultFieldInfo,
        std::string_view fieldType, std::size_t fieldAlignment = 0)
    {
        if (reflect::is_bool_type(fieldType))
        {
            in_write_primitive<bool>(hsonParam, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_float32)
        {
            in_write_primitive<float>(hsonParam, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_float64)
        {
            in_write_primitive<double>(hsonParam, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_int8)
        {
            in_write_primitive<s8>(hsonParam, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_int16)
        {
            in_write_primitive<s16>(hsonParam, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_int32)
        {
            in_write_primitive<s32>(hsonParam, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_int64)
        {
            in_write_primitive<s64>(hsonParam, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_uint8)
        {
            in_write_primitive<u8>(hsonParam, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_uint16)
        {
            in_write_primitive<u16>(hsonParam, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_uint32)
        {
            in_write_primitive<u32>(hsonParam, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_uint64)
        {
            in_write_primitive<u64>(hsonParam, fieldAlignment);
        }
        else if (reflect::is_string_type(fieldType))
        {
            in_validate_param_type(hsonParam, hson::parameter_type::string);
            m_writer->pad((fieldAlignment) ? fieldAlignment : alignof(RawOffsetType<char>));

            // Write placeholder string.
            const auto strOffPos = m_writer->tell();
            m_writer->write_nulls(sizeof(RawOffsetType<char>) * 2);

            // Add string to string table if necessary.
            if (!hsonParam.value_string().empty())
            {
                m_writer->add_string(hsonParam.value_string(), strOffPos);
            }

            // Do tail-end alignment for strings, since they are structs.
            if (fieldAlignment)
            {
                m_writer->pad(fieldAlignment);
            }
        }
        else if (reflect::is_vec3_type(fieldType))
        {
            in_write_vector<vec3>(hsonParam, fieldAlignment,
                defaultFieldInfo.default_val_vec3());
        }
        else if (reflect::is_object_reference_type(fieldType))
        {
            in_validate_param_type(hsonParam, hson::parameter_type::string);
            RawObjectIDType rawObjRef = in_as_obj_ref<RawObjectIDType>(
                hsonParam.value_string());

            // Write object reference.
            m_writer->swap_and_write_obj(rawObjRef, fieldAlignment);

            // Do tail-end alignment for object references, since they are structs.
            if (fieldAlignment)
            {
                m_writer->pad(fieldAlignment);
            }
        }
        else if (reflect::is_vec2_type(fieldType))
        {
            in_write_vector<vec2>(hsonParam, fieldAlignment,
                defaultFieldInfo.default_val_vec2());
        }
        else if (reflect::is_vec4_type(fieldType))
        {
            in_write_vector<vec4>(hsonParam,
                (fieldAlignment) ? fieldAlignment : 16,
                defaultFieldInfo.default_val_vec4());
        }
        else if (reflect::is_quat_type(fieldType))
        {
            in_write_vector<quat>(hsonParam,
                (fieldAlignment) ? fieldAlignment : 16,
                defaultFieldInfo.default_val_quat());
        }
        else if (reflect::is_char_type(fieldType))
        {
            in_write_primitive<char>(hsonParam, fieldAlignment);
        }
        else
        {
            // Write enum type.
            const auto enumDef = m_objTypeDB->enums.get(fieldType.data());
            if (enumDef)
            {
                in_write_enum(hsonParam, *enumDef, fieldAlignment);
                return;
            }

            // Write struct type.
            const auto structDef = m_objTypeDB->structs.get(fieldType.data());
            if (structDef)
            {
                in_validate_param_type(hsonParam, hson::parameter_type::object);

                if (!fieldAlignment)
                {
                    fieldAlignment = structDef->get_real_alignment(
                        m_objTypeDB->enums, m_objTypeDB->structs,
                        in_builtin_type_alignments);
                }

                m_writer->pad(fieldAlignment);
                write_struct_fields(*structDef, &hsonParam.value_object());

                // NOTE: We have to do tail-end alignment for structs.
                m_writer->pad(fieldAlignment);
                return;
            }

            throw std::runtime_error("Unknown or unsupported field type");
        }
    }

    template<typename T>
    inline void in_write_enum_value(std::uintmax_t value, std::size_t fieldAlignment = 0)
    {
        // Cast value to the requested type, preserving signedness as necessary.
        T val;
        if constexpr (std::is_signed_v<T>)
        {
            val = static_cast<T>(static_cast<std::intmax_t>(value));
        }
        else
        {
            val = static_cast<T>(value);
        }

        // Write the value as the requested type.
        m_writer->swap_and_write_obj(val, fieldAlignment);
    }

    template<typename T>
    inline void in_write_enum_value(const reflect::field_definition& defaultFieldInfo,
        std::size_t fieldAlignment = 0)
    {
        in_write_enum_value<T>(defaultFieldInfo.default_val_uint(), fieldAlignment);
    }

    void in_write_default_enum(const reflect::field_definition& defaultFieldInfo,
        const reflect::enum_definition& enumDef, std::size_t fieldAlignment = 0)
    {
        switch (enumDef.type)
        {
        case reflect::integral_type::int8:
            in_write_enum_value<s8>(defaultFieldInfo, fieldAlignment);
            return;

        case reflect::integral_type::int16:
            in_write_enum_value<s16>(defaultFieldInfo, fieldAlignment);
            return;

        case reflect::integral_type::int32:
            in_write_enum_value<s32>(defaultFieldInfo, fieldAlignment);
            return;

        case reflect::integral_type::int64:
            in_write_enum_value<s64>(defaultFieldInfo, fieldAlignment);
            return;

        case reflect::integral_type::uint8:
            in_write_enum_value<u8>(defaultFieldInfo, fieldAlignment);
            return;

        case reflect::integral_type::uint16:
            in_write_enum_value<u16>(defaultFieldInfo, fieldAlignment);
            return;

        case reflect::integral_type::uint32:
            in_write_enum_value<u32>(defaultFieldInfo, fieldAlignment);
            return;

        case reflect::integral_type::uint64:
            in_write_enum_value<u64>(defaultFieldInfo, fieldAlignment);
            return;

        default:
            throw std::runtime_error("Invalid enum integral type");
        }
    }

    void in_write_default_element(
        const reflect::field_definition& defaultFieldInfo,
        std::string_view fieldType, std::size_t fieldAlignment = 0)
    {
        if (reflect::is_bool_type(fieldType))
        {
            const auto val = static_cast<u8>(defaultFieldInfo.default_val_bool());
            m_writer->write_obj(val, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_float32)
        {
            auto val = static_cast<float>(defaultFieldInfo.default_val_floating());
            m_writer->swap_and_write_obj(val, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_float64)
        {
            auto val = defaultFieldInfo.default_val_floating();
            m_writer->swap_and_write_obj(val, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_int8)
        {
            const auto val = static_cast<s8>(defaultFieldInfo.default_val_int());
            m_writer->write_obj(val, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_int16)
        {
            auto val = static_cast<s16>(defaultFieldInfo.default_val_int());
            m_writer->swap_and_write_obj(val, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_int32)
        {
            auto val = static_cast<s32>(defaultFieldInfo.default_val_int());
            m_writer->swap_and_write_obj(val, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_int64)
        {
            auto val = static_cast<s64>(defaultFieldInfo.default_val_int());
            m_writer->swap_and_write_obj(val, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_uint8)
        {
            const auto val = static_cast<u8>(defaultFieldInfo.default_val_uint());
            m_writer->write_obj(val, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_uint16)
        {
            auto val = static_cast<u16>(defaultFieldInfo.default_val_uint());
            m_writer->swap_and_write_obj(val, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_uint32)
        {
            auto val = static_cast<u32>(defaultFieldInfo.default_val_uint());
            m_writer->swap_and_write_obj(val, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_uint64)
        {
            auto val = static_cast<u64>(defaultFieldInfo.default_val_uint());
            m_writer->swap_and_write_obj(val, fieldAlignment);
        }
        else if (reflect::is_string_type(fieldType))
        {
            m_writer->pad((fieldAlignment) ? fieldAlignment : alignof(RawOffsetType<char>));

            // Write placeholder string.
            const auto strOffPos = m_writer->tell();
            m_writer->write_nulls(sizeof(RawOffsetType<char>) * 2);

            // Add string to string table if necessary.
            if (!defaultFieldInfo.default_val_string().empty())
            {
                m_writer->add_string(defaultFieldInfo.default_val_string(), strOffPos);
            }

            // Do tail-end alignment for strings, since they are structs.
            if (fieldAlignment)
            {
                m_writer->pad(fieldAlignment);
            }
        }
        else if (reflect::is_vec3_type(fieldType))
        {
            // Write default vector value.
            m_writer->write_obj(defaultFieldInfo.default_val_vec3(), fieldAlignment);

            // Do tail-end alignment for vectors, since they are structs.
            if (fieldAlignment)
            {
                m_writer->pad(fieldAlignment);
            }
        }
        else if (reflect::is_object_reference_type(fieldType))
        {
            RawObjectIDType rawObjRef(nullptr);

            // Write object reference.
            m_writer->swap_and_write_obj(rawObjRef, fieldAlignment);

            // Do tail-end alignment for object references, since they are structs.
            if (fieldAlignment)
            {
                m_writer->pad(fieldAlignment);
            }
        }
        else if (reflect::is_vec2_type(fieldType))
        {
            // Write default vector value.
            m_writer->write_obj(defaultFieldInfo.default_val_vec2(), fieldAlignment);

            // Do tail-end alignment for vectors, since they are structs.
            if (fieldAlignment)
            {
                m_writer->pad(fieldAlignment);
            }
        }
        else if (reflect::is_vec4_type(fieldType))
        {
            // Write default vector value.
            m_writer->write_obj(defaultFieldInfo.default_val_vec4(),
                (fieldAlignment) ? fieldAlignment : 16);

            // Do tail-end alignment for vectors, since they are structs.
            if (fieldAlignment)
            {
                m_writer->pad(fieldAlignment);
            }
        }
        else if (reflect::is_quat_type(fieldType))
        {
            // Write default quaternion value.
            m_writer->write_obj(defaultFieldInfo.default_val_quat(),
                (fieldAlignment) ? fieldAlignment : 16);

            // Do tail-end alignment for quaternions, since they are structs.
            if (fieldAlignment)
            {
                m_writer->pad(fieldAlignment);
            }
        }
        else if (reflect::is_char_type(fieldType))
        {
            const auto val = static_cast<u8>(defaultFieldInfo.default_val_char());
            m_writer->write_obj(val, fieldAlignment);
        }
        else
        {
            // Write enum type.
            const auto enumDef = m_objTypeDB->enums.get(fieldType.data());
            if (enumDef)
            {
                in_write_default_enum(defaultFieldInfo, *enumDef, fieldAlignment);
                return;
            }

            // Write struct type.
            const auto structDef = m_objTypeDB->structs.get(fieldType.data());
            if (structDef)
            {
                if (!fieldAlignment)
                {
                    fieldAlignment = structDef->get_real_alignment(
                        m_objTypeDB->enums, m_objTypeDB->structs,
                        in_builtin_type_alignments);
                }

                m_writer->pad(fieldAlignment);
                write_struct_fields(*structDef);

                // NOTE: We have to do tail-end alignment for structs.
                m_writer->pad(fieldAlignment);
                return;
            }

            throw std::runtime_error("Unknown or unsupported field type");
        }
    }

    template<typename T>
    static std::size_t in_jump_past_primitive(
        std::size_t pos, std::size_t fieldAlignment = 0) noexcept
    {
        pos = align(pos, (fieldAlignment) ? fieldAlignment : alignof(T));
        pos += sizeof(T);
        return pos;
    }

    static std::size_t in_jump_past_object(std::size_t pos,
        std::size_t fieldAlignment, std::size_t size,
        std::size_t defaultAlignment) noexcept
    {
        if (!fieldAlignment)
        {
            fieldAlignment = defaultAlignment;
        }

        pos = align(pos, fieldAlignment);
        pos += size;
        pos = align(pos, fieldAlignment);
        return pos;
    }

    template<typename T>
    static inline std::size_t in_jump_past_object(
        std::size_t pos, std::size_t fieldAlignment) noexcept
    {
        return in_jump_past_object(pos, fieldAlignment,
            sizeof(T), alignof(T));
    }

    std::size_t in_jump_past_enum(std::size_t pos,
        reflect::integral_type type, std::size_t fieldAlignment = 0)
    {
        switch (type)
        {
        case reflect::integral_type::int8:
            return in_jump_past_primitive<s8>(pos, fieldAlignment);

        case reflect::integral_type::int16:
            return in_jump_past_primitive<s16>(pos, fieldAlignment);

        case reflect::integral_type::int32:
            return in_jump_past_primitive<s32>(pos, fieldAlignment);

        case reflect::integral_type::int64:
            return in_jump_past_primitive<s64>(pos, fieldAlignment);

        case reflect::integral_type::uint8:
            return in_jump_past_primitive<u8>(pos, fieldAlignment);

        case reflect::integral_type::uint16:
            return in_jump_past_primitive<u16>(pos, fieldAlignment);

        case reflect::integral_type::uint32:
            return in_jump_past_primitive<u32>(pos, fieldAlignment);

        case reflect::integral_type::uint64:
            return in_jump_past_primitive<u64>(pos, fieldAlignment);

        default:
            throw std::runtime_error("Invalid enum integral type");
        }
    }

    std::size_t in_write_array_data_for_element(std::size_t pos,
        std::string_view fieldType, const hson::parameter* hsonParam = nullptr,
        std::size_t fieldAlignment = 0)
    {
        if (reflect::is_bool_type(fieldType))
        {
            return in_jump_past_primitive<u8>(pos, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_float32)
        {
            return in_jump_past_primitive<float>(pos, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_float64)
        {
            return in_jump_past_primitive<double>(pos, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_int8)
        {
            return in_jump_past_primitive<s8>(pos, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_int16)
        {
            return in_jump_past_primitive<s16>(pos, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_int32)
        {
            return in_jump_past_primitive<s32>(pos, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_int64)
        {
            return in_jump_past_primitive<s64>(pos, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_uint8)
        {
            return in_jump_past_primitive<u8>(pos, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_uint16)
        {
            return in_jump_past_primitive<u16>(pos, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_uint32)
        {
            return in_jump_past_primitive<u32>(pos, fieldAlignment);
        }
        else if (fieldType == reflect::builtin_type_id_uint64)
        {
            return in_jump_past_primitive<u64>(pos, fieldAlignment);
        }
        else if (reflect::is_string_type(fieldType))
        {
            return in_jump_past_object(pos, fieldAlignment,
                sizeof(RawOffsetType<char>) * 2,
                alignof(RawOffsetType<char>));
        }
        else if (reflect::is_vec3_type(fieldType))
        {
            return in_jump_past_object<vec3>(pos, fieldAlignment);
        }
        else if (reflect::is_object_reference_type(fieldType))
        {
            return in_jump_past_object<RawObjectIDType>(pos, fieldAlignment);
        }
        else if (reflect::is_vec2_type(fieldType))
        {
            return in_jump_past_object<vec2>(pos, fieldAlignment);
        }
        else if (reflect::is_vec4_type(fieldType))
        {
            return in_jump_past_object<vec4>(pos,
                (fieldAlignment) ? fieldAlignment : 16);
        }
        else if (reflect::is_quat_type(fieldType))
        {
            return in_jump_past_object<quat>(pos,
                (fieldAlignment) ? fieldAlignment : 16);
        }
        else if (reflect::is_char_type(fieldType))
        {
            return in_jump_past_primitive<u8>(pos, fieldAlignment);
        }
        else
        {
            // Write enum type.
            const auto enumDef = m_objTypeDB->enums.get(fieldType.data());
            if (enumDef)
            {
                return in_jump_past_enum(pos, enumDef->type, fieldAlignment);
            }

            // Write struct type.
            const auto structDef = m_objTypeDB->structs.get(fieldType.data());
            if (structDef)
            {
                if (!fieldAlignment)
                {
                    fieldAlignment = structDef->get_real_alignment(
                        m_objTypeDB->enums, m_objTypeDB->structs,
                        in_builtin_type_alignments);
                }

                pos = align(pos, fieldAlignment);
                pos = write_array_data(pos, *structDef, (hsonParam) ?
                    &hsonParam->value_object() : nullptr);

                // NOTE: We have to do tail-end alignment for structs.
                return align(pos, fieldAlignment);
            }

            throw std::runtime_error("Unknown or unsupported field type");
        }
    }

    std::size_t in_write_array_data(std::size_t arrDataOffPos,
        const reflect::field_definition& fieldInfo,
        const hson::parameter* hsonParam)
    {
        if (fieldInfo.is_array())
        {
            // If a HSON parameter is given, ensure it is an array.
            if (hsonParam)
            {
                in_validate_param_type(*hsonParam, hson::parameter_type::array);
            }

            /* dynamic-sized array */
            if (!fieldInfo.array_count())
            {
                // Align as necessary for dynamic array.
                arrDataOffPos = align(arrDataOffPos, (fieldInfo.alignment) ?
                    fieldInfo.alignment : alignof(RawArrayType<void>));

                // Fix array data offset.
                // NOTE: We fix the offset even if the array count is 0, like Sonic Team.
                m_writer->pad(16); // TODO: Is this always correct?
                m_writer->fix_offset(arrDataOffPos +
                    offsetof(RawArrayType<void>, dataPtr));

                // Write dynamic array data if necessary.
                if (hsonParam)
                {
                    const auto& hsonParamValArr = hsonParam->value_array();
                    for (std::size_t i = 0; i < hsonParamValArr.size(); ++i)
                    {
                        in_write_element(hsonParamValArr[i],
                            fieldInfo, fieldInfo.subtype());
                    }

                    // TODO: Do we have to pad the stream itself after writing array data?
                }

                // Account for dynamic array size.
                arrDataOffPos += sizeof(RawArrayType<void>);

                // Do tail-end alignment for dynamic arrays, since they are structs.
                if (fieldInfo.alignment)
                {
                    arrDataOffPos = align(arrDataOffPos, fieldInfo.alignment);
                }
            }

            /* fixed-sized array */
            else
            {
                // Align if necessary for fixed array.
                std::size_t fieldAlignment;
                if (fieldInfo.alignment)
                {
                    arrDataOffPos = align(arrDataOffPos, fieldInfo.alignment);
                    fieldAlignment = 1; // Align of 1; disables alignment.
                }
                else
                {
                    fieldAlignment = 0; // Align of 0; use default alignment.
                }

                // Account for all fixed array elements which are present.
                std::size_t minCount;
                if (hsonParam)
                {
                    const auto& hsonParamValArr = hsonParam->value_array();
                    minCount = std::min<std::size_t>(fieldInfo.array_count(),
                        hsonParam->value_array().size());

                    for (std::size_t i = 0; i < minCount; ++i)
                    {
                        arrDataOffPos = in_write_array_data_for_element(
                            arrDataOffPos, fieldInfo.subtype(),
                            &hsonParamValArr[i], fieldAlignment);
                    }
                }
                else
                {
                    minCount = 0;
                }

                // Account for any remaining array elements which are not present.
                for (std::size_t i = minCount; i < fieldInfo.array_count(); ++i)
                {
                    arrDataOffPos = in_write_array_data_for_element(
                        arrDataOffPos, fieldInfo.subtype(),
                        nullptr, fieldAlignment);
                }
            }

            return arrDataOffPos;
        }

        /* non-arrays */
        else
        {
            return in_write_array_data_for_element(
                arrDataOffPos, fieldInfo.type(),
                hsonParam, fieldInfo.alignment);
        }
    }

    void in_write_parameters(const reflect::struct_definition& objStructType,
        std::size_t objStructAlignment, const radix_tree<hson::parameter>& hsonParams)
    {
        // Write struct fields.
        const auto arrDataOffPos = m_writer->tell();
        write_struct_fields(objStructType, &hsonParams);

        // Do tail-end alignment for structs.
        m_writer->pad(objStructAlignment);

        // Write array data as necessary.
        write_array_data(arrDataOffPos, objStructType, &hsonParams);
    }

public:
    void write_field(
        const reflect::field_definition& fieldInfo,
        const hson::parameter* hsonParam = nullptr)
    {
        /* arrays */
        if (fieldInfo.is_array())
        {
            // If a HSON parameter is given, ensure it is an array.
            if (hsonParam)
            {
                in_validate_param_type(*hsonParam, hson::parameter_type::array);
            }

            /* dynamic-sized array */
            if (!fieldInfo.array_count())
            {
                // Pad stream as necessary for dynamic array.
                m_writer->pad((fieldInfo.alignment) ?
                    fieldInfo.alignment : alignof(RawArrayType<void>));

                // Generate dynamic array.
                RawArrayType<void> rawDynArray = {};
                rawDynArray.count = static_cast<RawArrayType<void>::size_type>(
                    (hsonParam) ? hsonParam->value_array().size() : 0);

                rawDynArray.capacity = rawDynArray.count;

                // Write dynamic array.
                m_writer->swap_and_write_obj(rawDynArray);

                // Do tail-end alignment for dynamic arrays, since they are structs.
                if (fieldInfo.alignment)
                {
                    m_writer->pad(fieldInfo.alignment);
                }

                // NOTE: For dynamic arrays, we don't write the actual array data until later.
            }

            /* fixed-sized array */
            else
            {
                // Pad stream if necessary for fixed array.
                std::size_t fieldAlignment;
                if (fieldInfo.alignment)
                {
                    m_writer->pad(fieldInfo.alignment);
                    fieldAlignment = 1; // Align of 1; disables alignment.
                }
                else
                {
                    fieldAlignment = 0; // Align of 0; use default alignment.
                }

                // Write all fixed array elements which are present.
                std::size_t minCount;
                if (hsonParam)
                {
                    const auto& hsonParamValArr = hsonParam->value_array();
                    minCount = std::min<std::size_t>(fieldInfo.array_count(),
                        hsonParam->value_array().size());

                    for (std::size_t i = 0; i < minCount; ++i)
                    {
                        in_write_element(hsonParamValArr[i], fieldInfo,
                            fieldInfo.subtype(), fieldAlignment);
                    }
                }
                else
                {
                    minCount = 0;
                }

                // Write default values for any remaining array elements which are not present.
                for (std::size_t i = minCount; i < fieldInfo.array_count(); ++i)
                {
                    in_write_default_element(fieldInfo,
                        fieldInfo.subtype(), fieldAlignment);
                }
            }
        }

        /* non-arrays */
        else if (hsonParam)
        {
            in_write_element(*hsonParam, fieldInfo,
                fieldInfo.type(), fieldInfo.alignment);
        }
        else
        {
            in_write_default_element(fieldInfo,
                fieldInfo.type(), fieldInfo.alignment);
        }
    }

    void write_struct_fields(const reflect::struct_definition& structDef,
        const radix_tree<hson::parameter>* hsonParams = nullptr)
    {
        // Recursively write struct parents.
        if (!structDef.parent.empty())
        {
            const auto parentStructDef = m_objTypeDB->structs.get(structDef.parent);
            if (!parentStructDef)
            {
                throw std::runtime_error("Could not find parent struct type in database");
            }

            write_struct_fields(*parentStructDef, hsonParams);

            if (m_tailEndAlignParentStructs)
            {
                m_writer->pad(parentStructDef->get_real_alignment(m_objTypeDB->enums,
                    m_objTypeDB->structs, in_builtin_type_alignments));
            }
        }

        // Write struct fields.
        for (const auto& structFieldInfo : structDef.fields)
        {
            write_field(structFieldInfo, (hsonParams) ?
                hsonParams->get(structFieldInfo.name) :
                nullptr);
        }
    }

    std::size_t write_array_data(std::size_t arrDataOffPos,
        const reflect::struct_definition& structDef,
        const radix_tree<hson::parameter>* hsonParams = nullptr)
    {
        // Recursively write array data for struct parents.
        if (!structDef.parent.empty())
        {
            const auto parentStructDef = m_objTypeDB->structs.get(structDef.parent);
            if (!parentStructDef)
            {
                throw std::runtime_error("Could not find parent struct type in database");
            }

            arrDataOffPos = write_array_data(arrDataOffPos,
                *parentStructDef, hsonParams);

            if (m_tailEndAlignParentStructs)
            {
                arrDataOffPos = align(arrDataOffPos,
                    parentStructDef->get_real_alignment(m_objTypeDB->enums,
                    m_objTypeDB->structs, in_builtin_type_alignments));
            }
        }

        // Write struct fields.
        for (const auto& structFieldInfo : structDef.fields)
        {
            arrDataOffPos = in_write_array_data(
                arrDataOffPos, structFieldInfo, (hsonParams) ?
                hsonParams->get(structFieldInfo.name) : nullptr);
        }

        return arrDataOffPos;
    }

    bool write_parameters(const hson::object& hsonObj,
        const hson::project& hsonProject, std::size_t objParamDataOffPos)
    {
        // Get inherited type from HSON object.
        const auto hsonObjInheritedType = hsonObj.get_inherited_type(hsonProject);
        if (!hsonObjInheritedType) return false;

        // Get object definition from object type database.
        const auto objType = m_objTypeDB->get(*hsonObjInheritedType);
        if (!objType) return false;

        // Get struct definition from object type database.
        if (objType->structType.empty()) return true;
        const auto& objStructType = m_objTypeDB->structs.at(objType->structType);

        // Align struct as necessary.
        const auto objStructAlignment = objStructType.get_real_alignment(
            m_objTypeDB->enums, m_objTypeDB->structs,
            in_builtin_type_alignments);

        m_writer->pad(std::max<std::size_t>(objStructAlignment, 16));

        // Fix parameters offset.
        m_writer->fix_offset(objParamDataOffPos);

        // Write parameters.
        in_write_parameters(objStructType, objStructAlignment,
            (hsonObj.has_inherited_parameters(hsonProject)) ?
                hsonObj.get_flattened_parameters(hsonProject) :
                hsonObj.parameters);

        return true;
    }

    in_field_writer(WriterType& writer,
        const set_object_type_database& objTypeDB,
        bool tailEndAlignParentStructs) noexcept :
        m_writer(&writer),
        m_objTypeDB(&objTypeDB),
        m_tailEndAlignParentStructs(tailEndAlignParentStructs) {}
};
} // internal
} // gedit
} // hh
} // hl

#endif
