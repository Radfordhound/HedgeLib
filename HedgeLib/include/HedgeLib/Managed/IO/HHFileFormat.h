#pragma once
#include "HedgeLib/Direct/IO/HedgehogEngine.h"
#include "IFileFormat.h"
#include "../Objects.h"

namespace HedgeLib::IO
{
    template<typename T>
    class HHFileFormat : public IFileFormat, public HHParentObject<T>
    {
    public:
        using HHParentObject<T>::HHParentObject;
        inline HHFileFormat(const std::filesystem::path filePath)
        {
            // TODO: Switch to hl_HHLoad after its optimization
            Load(filePath);
        }

        inline HL_RESULT Read(File& file) override
        {
            void* blob;
            HL_RESULT result = hl_HHRead(file.Get(), &blob);
            ptr = HHPointer<T>(static_cast<T*>(blob));
            Get()->EndianSwapRecursive(true);
            return result;
        }

        inline HL_RESULT Write(File& file) override
        {
            // TODO
            return HL_SUCCESS;
        }
    };
}
