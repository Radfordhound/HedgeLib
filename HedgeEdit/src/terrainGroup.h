#pragma once
#include "gameObject.h"
#include <string>

namespace HedgeEdit
{
    struct TerrainGroup
    {
        std::string Name;
        std::vector<GameObject> Terrain;

        inline TerrainGroup() = default;
        inline TerrainGroup(const char* name) : Name(name) {}

        GameObject* GetTerrainInstance(const char* mdlName);
    };
}
