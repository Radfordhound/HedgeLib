#pragma once
#include "IO/IFileFormat.h"
#include <vector>
#include <filesystem>

namespace HedgeLib::Archives
{
    class IArchive : public IO::IFileFormat
    {
    public:
        virtual std::vector<std::filesystem::path> GetSplitList(
            std::filesystem::path filePath) = 0;

        virtual void Extract(const std::filesystem::path dir) = 0;
    };
}
