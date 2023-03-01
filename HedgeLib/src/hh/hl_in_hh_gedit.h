#ifndef HL_IN_HH_GEDIT_H_INCLUDED
#define HL_IN_HH_GEDIT_H_INCLUDED

#include "hedgelib/hl_math.h"

namespace hl
{
namespace hson
{
class parameter;
} // hson

namespace hh
{
namespace gedit
{
namespace internal
{
template<typename RawObjectIDType,
    template<typename> typename RawOffsetType,
    template<typename> typename RawArrayType>
static constexpr std::size_t in_builtin_type_alignments[] =
{
    alignof(s8),                    // int8
    alignof(s16),                   // int16
    alignof(s32),                   // int32
    alignof(s64),                   // int64

    alignof(u8),                    // uint8
    alignof(u16),                   // uint16
    alignof(u32),                   // uint32
    alignof(u64),                   // uint64

    alignof(float),                 // float32
    alignof(double),                // float64

    alignof(u8),                    // char
    alignof(RawOffsetType<char>),   // string

    alignof(u8),                    // bool
    alignof(RawArrayType<void>),    // array
    alignof(RawObjectIDType),       // object_reference
    
    // NOTE: There are actually two rfl vector3 types Sonic Team uses:
    // "vector3" and "position". "vector3" is 16-byte aligned, but "position"
    // is not (it's default-aligned). To handle this, we treat our vector3
    // type as having default alignment and are careful to generate all
    // "vector3" typed fields in the templates with a custom alignment of
    // 16, and all "position" typed fields with default alignment.

    alignof(vec2),                  // vector2
    alignof(vec3),                  // vector3
    16,                             // vector4
    16                              // quaternion
};

template<typename WriterType>
struct in_tag_info
{
    using read_data_func = hson::parameter (*)(const void* rawTagData);
    using write_data_func = void (*)(const hson::parameter& tag, WriterType& writer);

    const char* name;
    std::size_t size;
    std::size_t align;
    read_data_func readData;
    write_data_func writeData;
};
} // internal
} // gedit
} // hh
} // hl

#endif
