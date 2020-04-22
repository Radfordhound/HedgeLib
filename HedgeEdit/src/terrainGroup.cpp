#include "terrainGroup.h"

namespace HedgeEdit
{
    GameObject* TerrainGroup::GetTerrainInstance(const char* mdlName)
    {
        for (auto& inst : Terrain)
        {
            if (inst.ModelName == mdlName)
            {
                return &inst;
            }
        }

        return nullptr;
    }
}
