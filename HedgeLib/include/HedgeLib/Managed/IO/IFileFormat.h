#pragma once
#include "File.h"
#include "HedgeLib/Direct/Errors.h"
#include <filesystem>

namespace HedgeLib::IO
{
    class HL_API IFileFormat
    {
    public:
        virtual ~IFileFormat() = 0;

        virtual HL_RESULT Load(const std::filesystem::path filePath);
        virtual HL_RESULT Read(File& file) = 0;
        virtual HL_RESULT Save(const std::filesystem::path filePath);
        virtual HL_RESULT Write(const File& file) = 0;
    };
}
