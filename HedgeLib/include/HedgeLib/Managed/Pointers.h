#pragma once
#include "HedgeLib/Direct/IO/HedgehogEngine.h"
#include <memory>
#include <cstdlib>
#include <iostream> // TODO: REMOVE ME

namespace HedgeLib
{
    template<typename T>
    struct SPointerGetter
    {
        inline T* operator()(void* ptr)
        {
            return static_cast<T*>(ptr);
        }
    };

    struct HHBlobDeleter
    {
        inline void operator()(void* blob)
        {
            std::cout << "finna free" << std::endl; // TODO: REMOVE ME
            hl_HHFreeBlob(blob);
            std::cout << "freed" << std::endl; // TODO: REMOVE ME
        }
    };

    template<typename T>
    struct HHBlobGetter
    {
        inline T* operator()(void* blob)
        {
            return static_cast<T*>(hl_HHGetData(blob));
        }
    };

    using HHBlobPointer = std::unique_ptr<void, HHBlobDeleter>;
}
