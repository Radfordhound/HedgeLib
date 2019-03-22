#include "Geometry/HHModel.h"
#include "Geometry/HHMesh.h"
#include "IO/HedgehogEngine.h"
#include <memory>
#include <cstdint>
#include <stdexcept>

namespace HedgeLib::Geometry
{
    using namespace IO::HedgehogEngine;

    void HHModelBase::ConvertMeshes(const ArrOffset32
        <DataOffset32<DHHMesh>>& meshArr) noexcept
    {
        for (std::uint32_t i = 0; i < meshArr.Count(); ++i)
        {
            meshes.push_back(std::unique_ptr<IMesh>(
                new HHMesh(meshArr[i].Get())));
        }
    }

    /*HHTerrainModel::HHTerrainModel(
        DHHTerrainModel* terrainModel) :
        terrainModel(terrainModel), HHModel()
    {
        // TODO: Generate HHMeshes from terrain model
    }*/

    #define ReadModel(file, model)switch (GetHeaderType(file)) {\
        case HEADER_TYPE_STANDARD:\
            ReadStandard(file, model);\
            break;\
\
        case HEADER_TYPE_MIRAGE:\
            ReadMirage(file, model);\
            break;\
\
        default:\
            throw std::logic_error("Cannot read model; unknown header!");\
        }\
        ConvertMeshes(model->Meshes);


    void HHTerrainModel::Read(IO::File& file)
    {
        file.BigEndian = true;
        ReadModel(file, terrainModel);
    }

    void HHTerrainModel::Write(IO::File& file)
    {
        // TODO
    }

    //HHSkeletalModel::HHSkeletalModel(
    //	DHHSkeletalModel* skeletalModel) :
    //	skeletalModel(skeletalModel), HHModel()
    //{
    //	// TODO: Generate HHMeshes from skeletal model
    //}

    void HHSkeletalModel::Read(IO::File& file)
    {
        file.BigEndian = true;
        ReadModel(file, skeletalModel);
    }

    void HHSkeletalModel::Write(IO::File& file)
    {
        // TODO
    }
}
