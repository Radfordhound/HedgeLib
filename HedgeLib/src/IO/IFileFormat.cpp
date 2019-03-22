#include "IO/IFileFormat.h"
#include "IO/File.h"
#include <filesystem>
#include <stdexcept>

namespace HedgeLib::IO
{
    IFileFormat::~IFileFormat() {}

    void IFileFormat::Load(const std::filesystem::path filePath)
    {
        File f = File::OpenRead(filePath);
        Read(f);
    }

    void IFileFormat::Read(File& file)
    {
        throw std::logic_error(
            "Cannot call Read function; function not yet implemented!");
    }

    void IFileFormat::Save(const std::filesystem::path filePath)
    {
        File f = File::OpenWrite(filePath);
        Write(f);
    }

    void IFileFormat::Write(File& file)
    {
        throw std::logic_error(
            "Cannot call Write function; function not yet implemented!");
    }
}
