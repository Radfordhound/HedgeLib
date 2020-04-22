#pragma once
#include "transform.h"
#include <string>
#include <vector>

namespace HedgeEdit
{
    class GameObject
    {
    public:
        std::string ModelName;
        std::vector<Transform> Instances;
        
        inline GameObject() = default;
        inline GameObject(std::string mdlName) : ModelName(mdlName) {}
    };
}
