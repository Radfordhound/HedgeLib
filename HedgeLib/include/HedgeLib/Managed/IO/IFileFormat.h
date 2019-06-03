#pragma once
#include "File.h"
#include "HedgeLib/Direct/Errors.h"
#include <filesystem>

namespace HedgeLib::IO
{
    class IFileFormat
    {
    public:
        virtual ~IFileFormat() = 0;

        HL_API virtual HL_RESULT Load(const std::filesystem::path filePath);
        virtual HL_RESULT Read(File& file) = 0;
        HL_API virtual HL_RESULT Save(const std::filesystem::path filePath);
        virtual HL_RESULT Write(File& file) = 0;
    };
}
