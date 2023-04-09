#include "hl_in_hh_gedit_field_reader.h"
#include "hl_in_hh_gedit_field_writer.h"
#include "hedgelib/hh/hl_hh_gedit.h"
#include "hedgelib/io/hl_file.h"
#include <DirectXMath.h>
#include <unordered_set>

namespace hl
{
namespace hh
{
namespace gedit
{
namespace internal
{
template<>
inline std::string in_as_guid_string<v3::raw_object_id>(const v3::raw_object_id& objRef)
{
    return objRef.as_string();
}

template<>
inline v3::raw_object_id in_as_obj_ref<v3::raw_object_id>(const std::string& guidStr)
{
    return v3::raw_object_id(guidStr);
}
} // internal

namespace v3
{
using in_field_reader = internal::in_field_reader<
    raw_object_id, off64, csl::move_array64>;

using in_field_writer = internal::in_field_writer<
    bina::v2::writer64, raw_object_id, off64, csl::move_array64>;

using in_tag_info = internal::in_tag_info<bina::v2::writer64>;

static const in_tag_info in_tag_info_range_spawning =
{
    tag_range_spawning,                                                 // name
    sizeof(raw_tag_data_range_spawning),                                // size
    alignof(raw_tag_data_range_spawning),                               // align
    [](const void* rawTagData) -> hson::parameter                       // readData
    {
        // Convert RangeSpawning data to a HSON parameter and return it.
        const auto& rawRangeSpawningData = *static_cast<
            const raw_tag_data_range_spawning*>(rawTagData);

        return rawRangeSpawningData.as_hson_parameter();
    },
    [](const hson::parameter& hsonTag, bina::v2::writer64& writer)      // writeData
    {
        // Get properties from HSON tag.
        const auto rangeIn = hsonTag.value_object().get("rangeIn");
        const auto rangeOut = hsonTag.value_object().get("rangeOut");

        // Generate RangeSpawning tag data.
        raw_tag_data_range_spawning rawTagDataRangeSpawning = {};
        rawTagDataRangeSpawning.rangeIn = (rangeIn) ?
            rangeIn->get_value_as_floating() : FLT_MAX;

        rawTagDataRangeSpawning.rangeOut = (rangeOut) ?
            rangeOut->get_value_as_floating() : FLT_MAX;

        // Write RangeSpawning tag data.
        writer.swap_and_write_obj(rawTagDataRangeSpawning);
    }
};

static const in_tag_info* in_get_tag_info(const char* tagName)
{
    // RangeSpawning
    if (text::equal(tagName, tag_range_spawning))
    {
        return &in_tag_info_range_spawning;
    }

    return nullptr;
}

static u64 in_get_supported_tag_count(
    const radix_tree<hson::parameter>& hsonParams)
{
    u64 supportedTagCount = 0;
    for (const auto it : hsonParams)
    {
        if (in_get_tag_info(it.first))
        {
            ++supportedTagCount;
        }
    }

    return supportedTagCount;
}

const raw_transform& raw_object::get_local_transform() const noexcept
{
    return (parentID.empty()) ?
        transformBase : transformOffset;
}

static DirectX::XMMATRIX in_get_matrix(const raw_transform& rawTransform) noexcept
{
    // Compute matrix with the given translation and rotation.
    return DirectX::XMMatrixMultiply(
        DirectX::XMMatrixRotationRollPitchYaw(
            rawTransform.rot.x, rawTransform.rot.y, rawTransform.rot.z),
        DirectX::XMMatrixTranslation(
            rawTransform.pos.x, rawTransform.pos.y, rawTransform.pos.z));
}

matrix4x4A raw_object::get_local_transform_matrix() const noexcept
{
    return get_local_transform().as_matrix();
}

static DirectX::XMMATRIX in_get_global_transform_matrix(
    const raw_object& rawObj, const raw_world& rawWorld)
{
    // If the object has a parent, its transform is local to the parent.
    if (!rawObj.parentID.empty())
    {
        const auto parentObj = rawWorld.get_object(rawObj.parentID);
        if (parentObj)
        {
            return DirectX::XMMatrixMultiply(
                in_get_matrix(rawObj.transformOffset),
                in_get_global_transform_matrix(*parentObj, rawWorld));
        }
    }

    // Otherwise, its transform is global.
    return in_get_matrix(rawObj.transformBase);
}

matrix4x4A raw_object::get_global_transform_matrix(const raw_world& world) const
{
    HL_STATIC_ASSERT_SIZE(DirectX::XMFLOAT4X4A, sizeof(matrix4x4A));

    matrix4x4A result;
    DirectX::XMStoreFloat4x4A(
        reinterpret_cast<DirectX::XMFLOAT4X4A*>(&result),
        in_get_global_transform_matrix(*this, world));

    return result;
}

void raw_object::add_to_hson(
    ordered_map<guid, hson::object>& hsonObjects,
    const set_object_type_database* objTypeDB,
    bool tailEndAlignParentStructs) const
{
    // Convert object to HSON.
    auto& hsonObj = hsonObjects.emplace(
        id, hson::object()).first->second;

    hsonObj.type = type.get();

    if (name)
    {
        hsonObj.name = name.get();
    }

    hsonObj.parentID = parentID;

    const auto& localTransform = get_local_transform();
    hsonObj.position = localTransform.pos;
    hsonObj.rotation = quat(localTransform.rot);

    // Convert tag data to HSON.
    hson::parameter hsonTags(hson::parameter_type::object);
    for (const auto& rawTagOff : tags)
    {
        const auto& rawTag = *rawTagOff;
        const auto rawTagType = rawTag.type.get();

        // Get info for the current tag.
        const auto tagInfo = in_get_tag_info(rawTagType);
        if (!tagInfo) continue;

        // Add all supported tags to HSON tags.
        hsonTags.value_object().insert(rawTagType, tagInfo->readData(rawTag.data()));
    }

    if (!hsonTags.value_object().empty())
    {
        hsonObj.parameters.insert("tags", std::move(hsonTags));
    }

    // Return if we don't have an object type database.
    if (!objTypeDB) return;

    // Get object definition from object type database.
    const auto objType = objTypeDB->get(type.get());
    if (!objType || objType->structType.empty()) return;

    // Get struct definition from object type database.
    const auto objStructType = objTypeDB->structs.get(objType->structType);
    if (!objStructType) return;

    // Convert parameter data to HSON.
    in_field_reader fieldReader(paramData.get(), *objTypeDB, tailEndAlignParentStructs);
    fieldReader.read_struct_fields(*objStructType, hsonObj.parameters);
}

const raw_object* raw_world::get_object(const raw_object_id& id) const
{
    for (const auto& rawObjPtr : objects)
    {
        if (rawObjPtr->id == id)
        {
            return rawObjPtr.get();
        }
    }

    return nullptr;
}

static void in_fix(raw_world& rawHeader)
{
    // Endian-swap header.
    rawHeader.endian_swap<false>();

    // Endian-swap objects.
    for (auto& rawObjOff : rawHeader.objects)
    {
        // Skip this object if its offset is null.
        if (!rawObjOff) continue;

        // Endian-swap this object.
        auto& rawObj = *rawObjOff;
        rawObj.endian_swap<false>();

        // Endian-swap tags.
        for (auto& rawTagOff : rawObj.tags)
        {
            // Skip this tag if its offset is null.
            if (!rawTagOff) continue;

            // Endian-swap this tag.
            auto& rawTag = *rawTagOff;
            rawTag.endian_swap<false>();

            // Endian-swap tag data.
            const auto rawTagType = rawTag.type.get();
            if (text::equal(rawTagType, tag_range_spawning))
            {
                auto& rawTagData = *rawTag.data<raw_tag_data_range_spawning>();
                rawTagData.endian_swap<false>();
            }
        }

        // TODO: Endian-swap parameter data.
    }
}

void raw_world::fix(bina::endian_flag endianFlag)
{
    if (bina::needs_swap(endianFlag))
    {
        in_fix(*this);
    }
}

void raw_world::add_to_hson(ordered_map<guid, hson::object>& hsonObjects,
    const set_object_type_database* objTypeDB,
    bool tailEndAlignParentStructs) const
{
    // Add all objects in the gedit world to the HSON.
    hsonObjects.reserve(hsonObjects.size() + objects.count);

    for (const auto& obj : objects)
    {
        // Skip object if its type is not in the database.
        if (objTypeDB && !objTypeDB->contains(obj->type.get()))
        {
            // TODO: make this optional?
            continue;
        }

        // Add object to HSON.
        obj->add_to_hson(hsonObjects, objTypeDB, tailEndAlignParentStructs);
    }
}

template<typename T>
static bool in_is_valid_vec(const hson::parameter& param)
{
    if (param.type() == hson::parameter_type::array &&
        param.value_array().size() == T::size())
    {
        for (std::size_t i = 0; i < T::size(); ++i)
        {
            if (param.value_array()[i].type() !=
                hson::parameter_type::floating)
            {
                return false;
            }
        }

        return true;
    }

    return false;
}

template<typename T>
static T in_get_vec(const hson::parameter& param)
{
    T vec;
    const auto& arr = param.value_array();

    for (std::size_t i = 0; i < T::size(); ++i)
    {
        vec[i] = arr[i].value_floating();
    }

    return vec;
}

static const radix_tree<hson::parameter>* in_get_tags(
    const hson::object& obj, const hson::project& project,
    radix_tree<hson::parameter>& tagsBuf)
{
    if (obj.has_inherited_parameters(project))
    {
        tagsBuf.clear();
        obj.get_flattened_parameters(project, tagsBuf, "tags");
        return (tagsBuf.empty()) ? nullptr : &tagsBuf;
    }
    else
    {
        const auto tagsParam = obj.parameters.get("tags");
        return (tagsParam && tagsParam->type() == hson::parameter_type::object) ?
            &tagsParam->value_object() : nullptr;
    }
}

void write(const hson::project& project,
    const set_object_type_database& objTypeDB,
    bina::v2::writer64& writer, bool tailEndAlignParentStructs)
{
    // Start writing BINA data block.
    writer.start_data_block();

    // Determine the number of objects we have sufficient type info for.
    std::unordered_set<std::string> usedObjNames;
    u64 objCount = 0;

    usedObjNames.reserve(project.objects.size());

    for (auto it = project.objects.begin(); it != project.objects.end(); ++it)
    {
        const auto& obj = it->second;
        const auto hsonObjInheritedType = obj.get_inherited_type(project);

        if (hsonObjInheritedType && objTypeDB.contains(*hsonObjInheritedType))
        {
            if (obj.name.has_value())
            {
                usedObjNames.insert(obj.name.value());
            }

            ++objCount;
        }
    }

    // Write world header.
    auto curOffPos = writer.tell();

    {
        // Generate world.
        raw_world rawWorld = {};
        //rawWorld.objects.dataPtr = sizeof(raw_world);
        rawWorld.objects.count = objCount;
        rawWorld.objects.capacity = objCount;

        // Write world.
        writer.swap_and_write_obj(rawWorld);
    }

    // Write placeholder object offsets.
    if (objCount)
    {
        writer.fix_offset(curOffPos + offsetof(raw_world, objects));
        curOffPos = writer.tell();
        writer.write_nulls(sizeof(off64<raw_object>) * objCount);
        writer.pad(16);
    }

    // Write objects.
    radix_tree<hson::parameter> tagsBuf;
    radix_tree<std::size_t> numObjsOfTypes;
    const auto objsPos = writer.tell();

    for (auto it = project.objects.begin(); it != project.objects.end(); ++it)
    {
        // Skip this object if it's not in the type database.
        const auto& obj = it->second;
        const auto hsonObjInheritedType = obj.get_inherited_type(project);

        if (!hsonObjInheritedType || !objTypeDB.contains(*hsonObjInheritedType))
            continue;

        // Fix this object's offset.
        const auto objPos = writer.tell();
        writer.fix_offset(curOffPos);

        // Get tags.
        const auto curObjTags = in_get_tags(obj, project, tagsBuf);

        // Generate object.
        raw_object rawObject = {};
        rawObject.id = it->first;

        const auto hsonObjParentId = obj.get_inherited_parent_id(project);
        rawObject.parentID = (hsonObjParentId) ?
            *hsonObjParentId : hson::object::default_parent_id;

        rawObject.transformBase = obj.get_global_transform(project);
        rawObject.transformOffset.pos = obj.get_local_position(project);
        rawObject.transformOffset.rot = obj.get_local_rotation(project).as_euler();

        const auto tagCount = (!curObjTags) ? 0 :
            in_get_supported_tag_count(*curObjTags);

        rawObject.tags.count = tagCount;
        rawObject.tags.capacity = rawObject.tags.count;

        // Write object.
        writer.swap_and_write_obj(rawObject);

        // Add type string.
        writer.add_string(*hsonObjInheritedType, objPos + offsetof(raw_object, type));

        // Add name string, if any.
        const auto* hsonObjInheritedName = obj.get_inherited_name(project);
        if (hsonObjInheritedName) // TODO: Should we disallow empty names?
        {
            writer.add_string(*hsonObjInheritedName, objPos + offsetof(raw_object, name));
        }

        // Otherwise, fallback to default name.
        else
        {
            // Get the number of objects of this type.
            auto& numObjsOfType = numObjsOfTypes.insert(
                *hsonObjInheritedType, 0).first->second;

            // Create a default name based on the object's type, e.g. "Ring".
            std::string defaultName(*hsonObjInheritedType);
            const auto defaultNameNumPos = defaultName.size();

            while (true)
            {
                // Append the number of objects of this type to the name, e.g. "Ring0"
                defaultName.append(std::to_string(numObjsOfType));

                // Increment the number of objects of this type.
                ++numObjsOfType;

                // Break if the name we just generated is not already in-use.
                const auto usedObjNameIt = usedObjNames.find(defaultName);
                if (usedObjNameIt == usedObjNames.end())
                {
                    break;
                }

                // Otherwise, we need to generate another name!
                // Remove the number we appended to the end of the name, and try again.
                defaultName.erase(defaultNameNumPos);
            }

            // Add the generated default name to the object.
            writer.add_string(std::move(defaultName), objPos + offsetof(raw_object, name));
        }

        // Write tags if necessary.
        if (tagCount)
        {
            // Write placeholder tag offsets.
            auto curTagOffPos = writer.tell();
            writer.fix_offset(objPos + offsetof(raw_object, tags));
            writer.write_nulls(sizeof(off64<raw_tag>) * tagCount);
            writer.pad(16);

            // Write tags.
            for (const auto it : *curObjTags)
            {
                // Get info for the current tag.
                const auto tagInfo = in_get_tag_info(it.first);
                if (!tagInfo) continue;

                // Generate tag.
                raw_tag rawTag = {};
                rawTag.size = tagInfo->size;

                // Write tag.
                const auto tagPos = writer.tell();
                writer.fix_offset(curTagOffPos);
                writer.write_nulls(sizeof(raw_tag));
                writer.add_string(it.first, tagPos + offsetof(raw_tag, type));

                // Update current tag offset position.
                curTagOffPos += sizeof(off64<raw_tag>);
            }
        }

        // Update current offset position.
        curOffPos += sizeof(off64<raw_object>);
    }

    // Write object parameters and tag data.
    in_field_writer fieldWriter(writer, objTypeDB, tailEndAlignParentStructs);
    curOffPos = objsPos;

    for (auto it = project.objects.begin(); it != project.objects.end(); ++it)
    {
        // Write parameters, and skip this object if it's not in the type database.
        const auto& obj = it->second;
        if (!fieldWriter.write_parameters(obj, project,
            curOffPos + offsetof(raw_object, paramData)))
        {
            continue;
        }

        // Update current offset position.
        curOffPos += sizeof(raw_object);

        // Write tag data as necessary.
        const auto curObjTags = in_get_tags(obj, project, tagsBuf);
        if (curObjTags)
        {
            const auto tagCount = in_get_supported_tag_count(*curObjTags);
            if (!tagCount) continue;

            // Increase offset position.
            curOffPos += (sizeof(off64<raw_tag>) * tagCount);
            curOffPos = align(curOffPos, 16);

            // Write tag data for supported tags.
            for (const auto it : *curObjTags)
            {
                // Get info for the current tag.
                const auto tagInfo = in_get_tag_info(it.first);
                if (!tagInfo) continue;

                // Pad stream and fix tag data offset.
                writer.pad(tagInfo->align);
                writer.fix_offset(curOffPos + offsetof(raw_tag, dataPtr));

                // Write tag data.
                tagInfo->writeData(it.second, writer);

                // Increase offset position.
                curOffPos += sizeof(raw_tag);
            }
        }
    }

    // Finish writing BINA data block.
    writer.finish_data_block();
}

void save(const hson::project& project,
    const set_object_type_database& objTypeDB,
    bina::endian_flag endianFlag, stream& stream,
    bool tailEndAlignParentStructs)
{
    bina::v2::writer64 writer(stream);
    writer.start(endianFlag);
    write(project, objTypeDB, writer, tailEndAlignParentStructs);
    writer.finish();
}

void save(const hson::project& project,
    const set_object_type_database& objTypeDB,
    bina::endian_flag endianFlag, const nchar* filePath,
    bool tailEndAlignParentStructs)
{
    file_stream stream(filePath, file::mode::write);
    save(project, objTypeDB, endianFlag,
        stream, tailEndAlignParentStructs);
}
} // v3
} // gedit
} // hh
} // hl
