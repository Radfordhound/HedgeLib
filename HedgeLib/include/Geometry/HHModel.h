#pragma once
#include "HHMesh.h"
#include "IMesh.h"
#include "IModel.h"
#include "Offsets.h"
#include "Reflect.h"
#include "IO/Endian.h"
#include "IO/Nodes.h"
#include "IO/File.h"
#include "Math/Matrix.h"
#include "Math/AABB.h"
#include <cstdint>
#include <cstddef>
#include <vector>
#include <memory>

namespace HedgeLib::Geometry
{
    struct DHHBone
    {
        std::int32_t Index = 0;
        StringOffset32 Name = nullptr;

        ENDIAN_SWAP(Index);
    };

    struct DHHSkeleton
    {
        // TODO: Convert this to an array if you find out what it is
        std::uint32_t UnknownCount = 0;
        DataOffset32<char> UnknownOffset = nullptr;

        std::uint32_t BoneCount;
        DataOffset32<DataOffset32<DHHBone>> Bones;
        DataOffset32<Math::Matrix4x4> BoneMatrices;
        DataOffset32<Math::AABB> GlobalAABB;

        ENDIAN_SWAP_OBJECT(UnknownCount, BoneCount);
        CUSTOM_ENDIAN_SWAP_RECURSIVE_TWOWAY
        {
            if (isBigEndian)
                IO::Endian::Swap(UnknownCount, BoneCount);

            // Swap bones/bone matrices
            for (std::uint32_t i = 0; i < BoneCount; ++i)
            {
                Bones[i]->EndianSwap(isBigEndian);
                BoneMatrices[i].EndianSwap(isBigEndian);
            }

            // Swap AABB
            GlobalAABB->EndianSwap(isBigEndian);

            if (!isBigEndian)
                IO::Endian::Swap(UnknownCount, BoneCount);
        }
    };

    struct DHHTerrainModel
    {
        ArrOffset32<DataOffset32<DHHMesh>> Meshes;
        StringOffset32 Name;
        std::uint32_t Flags; // TODO: Make this an enum

        ENDIAN_SWAP(Meshes, Flags);
    };

    struct DHHSkeletalModel
    {
        ArrOffset32<DataOffset32<DHHMesh>> Meshes;
        DHHSkeleton Skeleton;

        ENDIAN_SWAP(Meshes, Skeleton);
    };

    class HHModelBase : public IModel
    {
    protected:
        std::vector<std::unique_ptr<IMesh>> meshes;

        void ConvertMeshes(const ArrOffset32
            <DataOffset32<DHHMesh>>& meshes) noexcept;

    public:
        inline std::size_t MeshCount() const noexcept override
        {
            return meshes.size();
        }

        inline const std::unique_ptr<IMesh>* Meshes() const noexcept override
        {
            return meshes.data();
        }

        // TODO: Setters and such
    };

    class HHTerrainModel : public HHModelBase
    {
        IO::NodePointer<DHHTerrainModel> terrainModel;

        inline bool Direct() const noexcept
        {
            return (terrainModel != nullptr);
        }

    public:
        inline HHTerrainModel() = default;
        //HHTerrainModel(const DHHTerrainModel* terrainModel);

        inline ~HHTerrainModel() override = default;
        // TODO: Setters and such

        void Read(IO::File& file) override;
        void Write(IO::File& file) override;
    };

    class HHSkeletalModel : public HHModelBase
    {
        IO::NodePointer<DHHSkeletalModel> skeletalModel;

        inline bool Direct() const noexcept
        {
            return (skeletalModel != nullptr);
        }

    public:
        inline HHSkeletalModel() = default;
        //HHSkeletalModel(const DHHSkeletalModel* skeletalModel);

        inline ~HHSkeletalModel() override = default;
        // TODO: Setters and such

        void Read(IO::File& file) override;
        void Write(IO::File& file) override;
    };
}
