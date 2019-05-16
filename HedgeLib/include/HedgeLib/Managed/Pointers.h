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

    template<typename T>
    struct HHPointerDeleter
    {
        inline void operator()(T* ptr)
        {
            std::cout << "finna free" << std::endl; // TODO: REMOVE ME
            hl_HHFreeBlob(ptr);
            std::cout << "freed" << std::endl; // TODO: REMOVE ME
        }
    };

    template<typename T>
    struct HHPointerGetter
    {
        inline T* operator()(void* ptr)
        {
            return static_cast<T*>(hl_HHGetData(ptr));
        }
    };

    template<typename T>
    using HHPointer = std::unique_ptr<T, HHPointerDeleter<T>>;
}
