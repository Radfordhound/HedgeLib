#ifndef HL_IN_HH_GEDIT_FIELD_READER_H_INCLUDED
#define HL_IN_HH_GEDIT_FIELD_READER_H_INCLUDED

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
std::string in_as_guid_string(const T& objRef);

template<typename RawObjectIDType,
    template<typename> typename RawOffsetType,
    template<typename> typename RawArrayType>
class in_field_reader : public reflect::field_reader
{
    static inline constexpr const std::size_t* in_builtin_type_alignments =
        internal::in_builtin_type_alignments<
            RawObjectIDType, RawOffsetType, RawArrayType>;

    const set_object_type_database* m_objTypeDB;
    bool m_tailEndAlignParentStructs;

    /**
     * @brief Reads a vector of the given type (i.e. vec2, vec3, vec4, quat).
     * 
     * @tparam T The type of vector to read.
     * @param fieldAlignment The custom alignment of this vector, or 0 to use default alignment.
     * @return hson::parameter A HSON parameter representing the vector.
     */
    template<typename T>
    hson::parameter in_read_vector(std::size_t fieldAlignment = 0)
    {
        // Read the vector into a HSON parameter.
        hson::parameter hsonParam(hson::parameter_type::array);
        const auto vec = read<T>(fieldAlignment);
        hsonParam.value_array().assign(T::size(), hson::parameter_type::floating);

        for (std::size_t i = 0; i < T::size(); ++i)
        {
            hsonParam.value_array()[i].value_floating() = vec[i];
        }
        
        // Do tail-end alignment for vectors, since they are structs.
        if (fieldAlignment)
        {
            align(fieldAlignment);
        }

        return hsonParam;
    }

    /**
     * @brief Reads one element of the given type.
     * 
     * @param fieldType The type of element to read.
     * @param fieldAlignment The custom alignment of this element, or 0 to use default alignment.
     * @return hson::parameter A HSON parameter representing the element.
     */
    hson::parameter in_read_element(
        std::string_view fieldType, std::size_t fieldAlignment = 0)
    {
        // NOTE: Conditions are sorted based on assumed frequency.

        if (reflect::is_bool_type(fieldType))
        {
            hson::parameter hsonParam(hson::parameter_type::boolean);
            hsonParam.value_bool() = read<u8>(fieldAlignment);
            return hsonParam;
        }
        else if (reflect::is_floating_type(fieldType))
        {
            hson::parameter hsonParam(hson::parameter_type::floating);
            hsonParam.value_floating() = read_floating(fieldType, fieldAlignment);
            return hsonParam;
        }
        else if (reflect::is_int_type(fieldType))
        {
            hson::parameter hsonParam(hson::parameter_type::signed_integer);
            hsonParam.value_int() = read_int(fieldType, fieldAlignment);
            return hsonParam;
        }
        else if (reflect::is_uint_type(fieldType))
        {
            hson::parameter hsonParam(hson::parameter_type::unsigned_integer);
            hsonParam.value_uint() = read_uint(fieldType, fieldAlignment);
            return hsonParam;
        }
        else if (reflect::is_string_type(fieldType))
        {
            hson::parameter hsonParam(hson::parameter_type::string);
            align((fieldAlignment) ? fieldAlignment : alignof(RawOffsetType<char>));

            // Read past string.
            const auto& rawStrOff = *peek<RawOffsetType<char>>();
            jump_ahead(sizeof(RawOffsetType<char>) * 2);

            // Initialize HSON parameter with string value.
            const auto rawStr = rawStrOff.get();
            if (rawStr) // nullptr check
            {
                hsonParam.value_string() = rawStr;
            }

            // Do tail-end alignment for strings, since they are structs.
            if (fieldAlignment)
            {
                align(fieldAlignment);
            }

            return hsonParam;
        }
        else if (reflect::is_vec3_type(fieldType))
        {
            return in_read_vector<vec3>(fieldAlignment);
        }
        else if (reflect::is_object_reference_type(fieldType))
        {
            const auto rawObjRef = read<RawObjectIDType>(fieldAlignment);
            hson::parameter hsonParam(hson::parameter_type::string);
            hsonParam.value_string() = in_as_guid_string<RawObjectIDType>(rawObjRef);

            // Do tail-end alignment for object IDs, since they are structs.
            if (fieldAlignment)
            {
                align(fieldAlignment);
            }

            return hsonParam;
        }
        else if (reflect::is_vec2_type(fieldType))
        {
            return in_read_vector<vec2>(fieldAlignment);
        }
        else if (reflect::is_vec4_type(fieldType) || reflect::is_quat_type(fieldType))
        {
            return in_read_vector<vec4>((fieldAlignment) ?
                fieldAlignment : 16);
        }
        else if (reflect::is_char_type(fieldType))
        {
            hson::parameter hsonParam(hson::parameter_type::string);
            hsonParam.value_string() = static_cast<char>(read<u8>(fieldAlignment));
            return hsonParam;
        }
        else
        {
            // Read enum type.
            const auto enumDef = m_objTypeDB->enums.get(fieldType.data());
            if (enumDef)
            {
                const auto enumVal = read_integral(
                    enumDef->type, fieldAlignment);

                // If the value we just read is present within the enum, store
                // the name of that enum value within the HSON data.
                const auto enumValName = enumDef->get_name_of_value(enumVal);
                if (enumValName)
                {
                    hson::parameter hsonParam(hson::parameter_type::string);
                    hsonParam.value_string() = enumValName;
                    return hsonParam;
                }

                // Otherwise, fallback to storing the numerical value directly.
                else if (enumDef->is_signed())
                {
                    hson::parameter hsonParam(hson::parameter_type::signed_integer);
                    hsonParam.value_int() = static_cast<std::intmax_t>(enumVal);
                    return hsonParam;
                }
                else
                {
                    hson::parameter hsonParam(hson::parameter_type::unsigned_integer);
                    hsonParam.value_uint() = enumVal;
                    return hsonParam;
                }
            }

            // Read struct type.
            const auto structDef = m_objTypeDB->structs.get(fieldType.data());
            if (structDef)
            {
                hson::parameter hsonParam(hson::parameter_type::object);
                if (!fieldAlignment)
                {
                    fieldAlignment = structDef->get_real_alignment(
                        m_objTypeDB->enums, m_objTypeDB->structs,
                        in_builtin_type_alignments);
                }

                align(fieldAlignment);
                read_struct_fields(*structDef, hsonParam.value_object());

                // NOTE: We have to do tail-end alignment for structs.
                align(fieldAlignment);
                return hsonParam;
            }

            throw std::runtime_error("Unknown or unsupported field type");
        }
    }

public:
    /**
     * @brief Reads a field using the given field info into a HSON parameter.
     * 
     * @param fieldInfo Information describing the field to read.
     * @return hson::parameter A HSON parameter representing the field.
     */
    hson::parameter read_field(const reflect::field_definition& fieldInfo)
    {
        /* arrays */
        if (fieldInfo.is_array())
        {
            hson::parameter hsonParam(hson::parameter_type::array);

            /* dynamic-sized array */
            if (!fieldInfo.array_count())
            {
                align((fieldInfo.alignment) ?
                    fieldInfo.alignment : alignof(RawArrayType<void>));

                // Read past array.
                const auto& rawArr = *peek<RawArrayType<void>>();
                jump_ahead(sizeof(RawArrayType<void>));

                // Read array elements.
                in_field_reader arrayFieldReader(rawArr.data(),
                    *m_objTypeDB, m_tailEndAlignParentStructs);

                auto& hsonParamValArr = hsonParam.value_array();
                hsonParamValArr.reserve(rawArr.size());

                for (typename RawArrayType<void>::size_type i = 0; i < rawArr.size(); ++i)
                {
                    hsonParamValArr.emplace_back(
                        arrayFieldReader.in_read_element(fieldInfo.subtype()));
                }

                // Do tail-end alignment for dynamic arrays, since they are structs.
                if (fieldInfo.alignment)
                {
                    align(fieldInfo.alignment);
                }
            }

            /* fixed-sized array */
            else
            {
                // Align stream if necessary for fixed array.
                std::size_t fieldAlignment;
                if (fieldInfo.alignment)
                {
                    align(fieldInfo.alignment);
                    fieldAlignment = 1;
                }
                else
                {
                    fieldAlignment = 0;
                }

                // Read array elements.
                auto& hsonParamValArr = hsonParam.value_array();
                hsonParamValArr.reserve(fieldInfo.array_count());

                for (std::size_t i = 0; i < fieldInfo.array_count(); ++i)
                {
                    hsonParamValArr.emplace_back(
                        in_read_element(fieldInfo.subtype(),
                        fieldAlignment));
                }
            }

            return hsonParam;
        }

        /* non-arrays */
        else
        {
            return in_read_element(
                fieldInfo.type(), fieldInfo.alignment);
        }
    }

    /**
     * @brief Reads all of the fields in the given struct into
     * the given HSON parameters radix tree.
     * 
     * @param structDef Information describing the struct to read.
     * @param hsonParams The radix tree to store the resulting HSON parameters into.
     */
    void read_struct_fields(const reflect::struct_definition& structDef,
        radix_tree<hson::parameter>& hsonParams)
    {
        // Recursively read struct parents.
        if (!structDef.parent.empty())
        {
            const auto parentStructDef = m_objTypeDB->structs.get(structDef.parent);
            if (!parentStructDef)
            {
                throw std::runtime_error("Could not find parent struct type in database");
            }

            read_struct_fields(*parentStructDef, hsonParams);

            if (m_tailEndAlignParentStructs)
            {
                align(parentStructDef->get_real_alignment(m_objTypeDB->enums,
                    m_objTypeDB->structs, in_builtin_type_alignments));
            }
        }

        // Read struct fields.
        for (const auto& structFieldInfo : structDef.fields)
        {
            hsonParams.insert(structFieldInfo.name,
                read_field(structFieldInfo));
        }
    }

    in_field_reader(const void* rawData,
        const set_object_type_database& objTypeDB,
        bool tailEndAlignParentStructs) noexcept :
        field_reader(rawData),
        m_objTypeDB(&objTypeDB),
        m_tailEndAlignParentStructs(tailEndAlignParentStructs) {}
};
} // internal
} // gedit
} // hh
} // hl
#endif
