#include "HedgeLib/Managed/IO/IFileFormat.h"

namespace HedgeLib::IO
{
    IFileFormat::~IFileFormat() {}

    HL_RESULT IFileFormat::Load(const std::filesystem::path filePath)
    {
        File f = File::OpenRead(filePath);
        return Read(f);
    }

    HL_RESULT IFileFormat::Save(const std::filesystem::path filePath)
    {
        File f = File::OpenWrite(filePath);
        return Write(f);
    }
}
