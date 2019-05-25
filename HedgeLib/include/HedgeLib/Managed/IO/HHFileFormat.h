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
            Load(filePath);
        }

        inline HL_RESULT Load(const std::filesystem::path filePath) override
        {
            // TODO: Optimize this like hl_HHLoad
            File f = File::OpenRead(filePath);
            return Read(f);
        }

        inline HL_RESULT Read(File& file) override
        {
            void* blob;
            HL_RESULT result = hl_HHRead(&file, &blob);
            this->ptr = HHBlobPointer(blob);
            this->Get()->EndianSwapRecursive(true);
            return result;
        }

        inline HL_RESULT Write(const File& file) override
        {
            OffsetTable offTable;
            this->Get()->Write(file, offTable);
            // TODO: Should we even return an HL_RESULT?
            return HL_SUCCESS;
        }
    };
}
