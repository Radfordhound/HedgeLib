#ifndef HL_SET_OBJ_TYPE_H_INCLUDED
#define HL_SET_OBJ_TYPE_H_INCLUDED
#include "../hl_reflect.h"

namespace hl
{
class set_object_type
{
public:
    std::string structType;
    std::string category;

    set_object_type() noexcept(noexcept(std::string::allocator_type())) = default;

    set_object_type(std::string structType, std::string category = std::string()) noexcept :
        structType(std::move(structType)),
        category(std::move(category)) {}
};

enum class set_object_format
{
    unknown = 0,
    gedit_v3,
};

class set_object_type_database : public radix_tree<set_object_type>
{
    void in_parse(const void* rawData, std::size_t rawDataSize);

    void in_read(stream& stream);

    void in_load(const nchar* filePath);

public:
    set_object_format                       format = set_object_format::unknown;
    reflect::enum_definition_database       enums;
    reflect::struct_definition_database     structs;

    HL_API void clear() noexcept;

    HL_API void parse(const void* rawData, std::size_t rawDataSize);

    HL_API void read(stream& stream);

    HL_API void load(const nchar* filePath);

    inline void load(const nstring& filePath)
    {
        load(filePath.c_str());
    }

    set_object_type_database() noexcept = default;

    HL_API set_object_type_database(const void* rawData, std::size_t rawDataSize);

    HL_API set_object_type_database(stream& stream);

    HL_API set_object_type_database(const nchar* filePath);

    inline set_object_type_database(const nstring& filePath) :
        set_object_type_database(filePath.c_str()) {}
};
} // hl
#endif
