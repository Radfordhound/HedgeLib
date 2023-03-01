#ifndef HL_HH_GEDIT_H_INCLUDED
#define HL_HH_GEDIT_H_INCLUDED

#include "../sets/hl_hson.h"
#include "../csl/hl_csl_move_array.h"
#include "../io/hl_bina.h"

namespace hl
{
class set_object_type_database;

namespace hh
{
namespace gedit
{
inline constexpr char tag_range_spawning[] = "RangeSpawning";
inline constexpr nchar extension[] = HL_NTEXT(".gedit");

namespace v1
{
struct raw_object_id
{
    /** @brief The ID of the object. */
    u16 objID;
    /** @brief The ID of the group the object is part of. */
    u16 groupID;

    static constexpr raw_object_id zero() noexcept
    {
        return raw_object_id(nullptr);
    }

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(objID);
        hl::endian_swap(groupID);
    }

    [[nodiscard]] inline bool empty() const noexcept
    {
        return (!objID && !groupID);
    }

    HL_API guid as_guid() const noexcept;

    inline raw_object_id() noexcept = default;

    constexpr raw_object_id(std::nullptr_t) noexcept :
        objID(0),
        groupID(0) {}

    constexpr raw_object_id(u16 objID, u16 groupID) noexcept :
        objID(objID),
        groupID(groupID) {}
};

HL_STATIC_ASSERT_SIZE(raw_object_id, 4);

struct raw_transform
{
    /** @brief Position, represented as XYZ coordinates. */
    vec3 pos;
    /** @brief Rotation, represented as Roll/Pitch/Yaw euler angles in radians. */
    vec3 rot;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(pos);
        hl::endian_swap(rot);
    }

    HL_API matrix4x4A as_matrix() const noexcept;

    raw_transform() noexcept = default;

    HL_API raw_transform(const matrix4x4& mtx);

    HL_API raw_transform(const matrix4x4A& mtx);
};

HL_STATIC_ASSERT_SIZE(raw_transform, 24);

struct raw_tag
{
    u32 unknown1;
    off32<char> type;
    u32 size;
    off32<void> dataPtr;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(unknown1);
        hl::endian_swap<swapOffsets>(type);
        hl::endian_swap(size);
        hl::endian_swap<swapOffsets>(dataPtr);
    }

    template<typename T = void>
    inline const T* data() const noexcept
    {
        return static_cast<const T*>(dataPtr.get());
    }

    template<typename T = void>
    inline T* data() noexcept
    {
        return static_cast<T*>(dataPtr.get());
    }
};

HL_STATIC_ASSERT_SIZE(raw_tag, 16);

struct raw_tag_data_range_spawning
{
    /**
       @brief How close the player needs to get to the
       object before the object will spawn.
    */
    float rangeIn;
    /**
       @brief How far the player needs to move away from the
       object once it has spawned before it will de-spawn.
    */
    float rangeOut;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(rangeIn);
        hl::endian_swap(rangeOut);
    }

    HL_API hson::parameter as_hson_parameter() const;
};

HL_STATIC_ASSERT_SIZE(raw_tag_data_range_spawning, 8);

struct raw_object
{
    /** @brief Always 0? */
    u32 unknown1;
    /** @brief Always 0? */
    u32 unknown2;
    off32<char> type;
    off32<char> name;
    raw_object_id id;
    raw_object_id parentID;
    raw_transform transform;
    csl::move_array32<off32<raw_tag>> tags;
    off32<void> paramData;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(unknown1);
        hl::endian_swap(unknown2);
        hl::endian_swap<swapOffsets>(type);
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap<swapOffsets>(id);
        hl::endian_swap<swapOffsets>(parentID);
        hl::endian_swap<swapOffsets>(transform);
        hl::endian_swap<swapOffsets>(tags);
        hl::endian_swap<swapOffsets>(paramData);
    }

    template<typename T = void>
    inline const T* params() const noexcept
    {
        return static_cast<const T*>(paramData.get());
    }

    template<typename T = void>
    inline T* params() noexcept
    {
        return static_cast<T*>(paramData.get());
    }
};

HL_STATIC_ASSERT_SIZE(raw_object, 0x44);

struct raw_world
{
    /** @brief Always 0? */
    u32 unknown1;
    /** @brief Always 0? */
    u32 unknown2;
    /** @brief Always 0? */
    u32 unknown3;
    /** @brief All objects contained within this gedit. */
    csl::move_array32<off32<raw_object>> objects;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(unknown1);
        hl::endian_swap(unknown2);
        hl::endian_swap(unknown3);
        hl::endian_swap<swapOffsets>(objects);
    }

    HL_API void fix(bina::endian_flag endianFlag);

    HL_API void add_to_hson(hson::project& proj,
        const set_object_type_database* objTypeDB = nullptr,
        bool tailEndAlignParentStructs = true) const;
};

HL_STATIC_ASSERT_SIZE(raw_world, 0x1C);
} // v1

namespace v2
{
using raw_object_id = v1::raw_object_id;
using raw_transform = v1::raw_transform;

struct raw_tag
{
    u64 unknown1;
    off64<char> type;
    u64 size;
    off64<void> dataPtr;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(unknown1);
        hl::endian_swap<swapOffsets>(type);
        hl::endian_swap(size);
        hl::endian_swap<swapOffsets>(dataPtr);
    }

    template<typename T = void>
    inline const T* data() const noexcept
    {
        return static_cast<const T*>(dataPtr.get());
    }

    template<typename T = void>
    inline T* data() noexcept
    {
        return static_cast<T*>(dataPtr.get());
    }
};

HL_STATIC_ASSERT_SIZE(raw_tag, 32);

using raw_tag_data_range_spawning = v1::raw_tag_data_range_spawning;

struct raw_object
{
    /** @brief Always 0? */
    u32 unknown1;
    /** @brief Always 0? */
    u32 unknown2;
    off64<char> type;
    off64<char> name;
    raw_object_id id;
    raw_object_id parentID;
    raw_transform transform;
    raw_transform childrenBaseTransform;
    csl::move_array64<off64<raw_tag>> tags;
    off64<void> paramData;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(unknown1);
        hl::endian_swap(unknown2);
        hl::endian_swap<swapOffsets>(type);
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap<swapOffsets>(id);
        hl::endian_swap<swapOffsets>(parentID);
        hl::endian_swap<swapOffsets>(transform);
        hl::endian_swap<swapOffsets>(childrenBaseTransform);
        hl::endian_swap<swapOffsets>(tags);
        hl::endian_swap<swapOffsets>(paramData);
    }

    template<typename T = void>
    inline const T* params() const noexcept
    {
        return static_cast<const T*>(paramData.get());
    }

    template<typename T = void>
    inline T* params() noexcept
    {
        return static_cast<T*>(paramData.get());
    }

    HL_API void add_to_hson(hson::project& proj,
        const set_object_type_database* objTypeDB = nullptr,
        bool tailEndAlignParentStructs = true) const;
};

HL_STATIC_ASSERT_SIZE(raw_object, 0x78);

struct raw_world
{
    /** @brief Always 0? */
    u32 unknown1;
    /** @brief Always 0? */
    u32 unknown2;
    /** @brief Always 0? Could be a pointer or a u32 with padding? */
    u64 unknown3;
    /** @brief All objects contained within this gedit. */
    csl::move_array64<off64<raw_object>> objects;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(unknown1);
        hl::endian_swap(unknown2);
        hl::endian_swap(unknown3);
        hl::endian_swap<swapOffsets>(objects);
    }

    HL_API void fix(bina::endian_flag endianFlag);

    HL_API void add_to_hson(hson::project& proj,
        const set_object_type_database* objTypeDB = nullptr,
        bool tailEndAlignParentStructs = true) const;
};

HL_STATIC_ASSERT_SIZE(raw_world, 0x30);
} // v2

namespace v3
{
struct raw_world;

struct alignas(8) raw_object_id : public guid
{
    using guid::operator=;
    using guid::guid;
};

HL_STATIC_ASSERT_SIZE(raw_object_id, 16);

using raw_transform = v2::raw_transform;
using raw_tag = v2::raw_tag;
using raw_tag_data_range_spawning = v2::raw_tag_data_range_spawning;

struct raw_object
{
    /** @brief Always 0? */
    u32 unknown1;
    /** @brief Always 0? */
    u32 unknown2;
    off64<char> type;
    off64<char> name;
    alignas(16) raw_object_id id;
    alignas(16) raw_object_id parentID;
    raw_transform transformBase;
    raw_transform transformOffset;
    csl::move_array64<off64<raw_tag>> tags;
    off64<void> paramData;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(unknown1);
        hl::endian_swap(unknown2);
        hl::endian_swap<swapOffsets>(type);
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap<swapOffsets>(id);
        hl::endian_swap<swapOffsets>(parentID);
        hl::endian_swap<swapOffsets>(transformBase);
        hl::endian_swap<swapOffsets>(transformOffset);
        hl::endian_swap<swapOffsets>(tags);
        hl::endian_swap<swapOffsets>(paramData);
    }

    template<typename T = void>
    inline const T* params() const noexcept
    {
        return static_cast<const T*>(paramData.get());
    }

    template<typename T = void>
    inline T* params() noexcept
    {
        return static_cast<T*>(paramData.get());
    }

    HL_API const raw_transform& get_local_transform() const noexcept;

    HL_API matrix4x4A get_local_transform_matrix() const noexcept;

    HL_API matrix4x4A get_global_transform_matrix(const raw_world& world) const;

    HL_API void add_to_hson(ordered_map<guid, hson::object>& hsonObjects,
        const set_object_type_database* objTypeDB = nullptr,
        bool tailEndAlignParentStructs = true) const;

    inline void add_to_hson(hson::project& hsonProject,
        const set_object_type_database* objTypeDB = nullptr,
        bool tailEndAlignParentStructs = true) const
    {
        add_to_hson(hsonProject.objects, objTypeDB, tailEndAlignParentStructs);
    }
};

HL_STATIC_ASSERT_SIZE(raw_object, 0xA0);

struct raw_world
{
    /** @brief Always 0? */
    u32 unknown1;
    /** @brief Always 0? */
    u32 unknown2;
    /** @brief Always 0? Could be a pointer or a u32 with padding? */
    u64 unknown3;
    /** @brief All objects contained within this gedit. */
    csl::move_array64<off64<raw_object>> objects;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(unknown1);
        hl::endian_swap(unknown2);
        hl::endian_swap(unknown3);
        hl::endian_swap<swapOffsets>(objects);
    }

    HL_API const raw_object* get_object(const raw_object_id& id) const;

    inline raw_object* get_object(const raw_object_id& id)
    {
        return const_cast<raw_object*>(const_cast<
            const raw_world*>(this)->get_object(id));
    }

    HL_API void fix(bina::endian_flag endianFlag);

    HL_API void add_to_hson(ordered_map<guid, hson::object>& hsonObjects,
        const set_object_type_database* objTypeDB = nullptr,
        bool tailEndAlignParentStructs = true) const;

    inline void add_to_hson(hson::project& hsonProject,
        const set_object_type_database* objTypeDB = nullptr,
        bool tailEndAlignParentStructs = true) const
    {
        add_to_hson(hsonProject.objects, objTypeDB, tailEndAlignParentStructs);
    }
};

HL_STATIC_ASSERT_SIZE(raw_world, 0x30);

HL_API void write(const hson::project& project,
    const set_object_type_database& objTypeDB,
    bina::v2::writer64& writer,
    bool tailEndAlignParentStructs = true);

HL_API void save(const hson::project& project,
    const set_object_type_database& objTypeDB,
    bina::endian_flag endianFlag, stream& stream,
    bool tailEndAlignParentStructs = true);

HL_API void save(const hson::project& project,
    const set_object_type_database& objTypeDB,
    bina::endian_flag endianFlag, const nchar* filePath,
    bool tailEndAlignParentStructs = true);

inline void save(const hson::project& project,
    const set_object_type_database& objTypeDB,
    bina::endian_flag endianFlag, const nstring& filePath,
    bool tailEndAlignParentStructs = true)
{
    save(project, objTypeDB, endianFlag,
        filePath.c_str(), tailEndAlignParentStructs);
}
} // v3
} // gedit
} // hh
} // hl
#endif
