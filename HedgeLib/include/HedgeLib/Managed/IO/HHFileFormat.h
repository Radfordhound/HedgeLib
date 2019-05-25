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

        inline HL_RESULT StartWriteStandard(File& file, std::uint32_t version)
        {
            return hl_HHStartWriteStandard(&file, 1);
        }

        inline HL_RESULT FinishWriteStandard(const File& file,
            const OffsetTable& offTable, long headerPos = 0,
            bool writeEOFThing = false)
        {
            return hl_HHFinishWriteStandard(&file, headerPos,
                writeEOFThing, &offTable);
        }

        inline HL_RESULT Write(const File& file, OffsetTable& offTable)
        {
            this->Get()->Write(file, offTable);
            // TODO: Make Write return an HL_RESULT
            return HL_SUCCESS;
        }

        inline HL_RESULT WriteStandard(File& file, std::uint32_t version,
            bool writeEOFThing = false)
        {
            // Start writing
            long headerPos = file.Tell();
            HL_RESULT result = StartWriteStandard(file, version);
            if (HL_FAILED(result)) return result;

            // Write data node
            OffsetTable offTable;
            result = Write(file, offTable);
            if (HL_FAILED(result)) return result;

            // Finish writing
            result = FinishWriteStandard(file, offTable,
                headerPos, writeEOFThing);

            return result;
        }

        inline HL_RESULT Write(File& file) override
        {
            return WriteStandard(file, 5);
        }

        inline HL_RESULT SaveStandard(const std::filesystem::path filePath,
            std::uint32_t version, bool writeEOFThing = false)
        {
            File f = File::OpenWrite(filePath);
            return WriteStandard(f, version, writeEOFThing);
        }
    };
}
