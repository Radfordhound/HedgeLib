#include "HedgeLib/Direct/IO/HedgehogEngine.h"
#include "HedgeLib/Managed/IO/File.h"
#include "HedgeLib/Managed/Offsets.h"
#include <cstdlib>
#include <cstddef>

using namespace HedgeLib;
using namespace HedgeLib::IO;

HL_IMPL_ENDIAN_SWAP_CPP(hl_DHHHeader);

HL_IMPL_ENDIAN_SWAP(hl_DHHHeader, v)
{
    hl_SwapUInt32(&v->FileSize);
    hl_SwapUInt32(&v->Version);
}

HL_IMPL_ENDIAN_SWAP_CPP(hl_DHHStandardHeader);

HL_IMPL_ENDIAN_SWAP(hl_DHHStandardHeader, v)
{
    HL_ENDIAN_SWAP(hl_DHHHeader, &v->Header);
    hl_SwapUInt32(&v->DataSize);
    hl_SwapUInt32(&v->DataOffset);
    hl_SwapUInt32(&v->OffsetTableOffset);
    hl_SwapUInt32(&v->EOFOffset);
}

void hl_HHFixOffsets(uint32_t* offTable,
    uint32_t offCount, void* data)
{
    hl_DataOff32* offPtr = static_cast<hl_DataOff32*>(data);
    for (uint32_t i = 0; i < offCount; ++i)
    {
        // Endian swap offset table entry
        hl_SwapUInt32(&(offTable[i]));

        // Get position of next offset
        // (Data Pointer + Current entry in offset table)
        offPtr = GetAbs<hl_DataOff32>(data, offTable[i]);

        if (offPtr == 0) continue; // null pointers should remain null

        // Endian swap offset
        hl_SwapUInt32(offPtr);

        // Get absolute pointer
        uintptr_t absPtr = (reinterpret_cast
            <uintptr_t>(data) + *offPtr);

        // Fix offset
#ifdef x86
        *offPtr = static_cast<hl_DataOff32>(absPtr);
#elif x64
        *offPtr = hl_x64AddAbsPtr32(absPtr);
#endif
    }
}

void* hl_HHMirageGetDataNode(const void* blob)
{
    // TODO
    return nullptr;
}

void* hl_HHMirageGetData(void* blob)
{
    // TODO
    return nullptr;
}

enum HL_RESULT hl_HHRead(struct hl_File* file, void** blob)
{
    HL_RESULT result;
    file->DoEndianSwap = true;

    // Get file size
    uint32_t fileSize;
    result = file->Read(fileSize);
    if (HL_FAILED(result)) return result;

    // Get header type and go back to beginning of file
    bool isMirage = false; // "Friends are nothing but a fleeting illusion"
    if (fileSize & HL_HHMIRAGE_FLAGS_MASK) // "Your mask can't hide how sad and lonely you are!"
    {
        // Make sure this is really a Mirage header
        uint32_t version;
        result = file->Read(version);
        if (HL_FAILED(result)) return result;

        if (version == HL_HHMIRAGE_SIGNATURE)
        {
            isMirage = true;
            fileSize &= HL_HHMIRAGE_SIZE_MASK;
        }

        hl_FileJumpBehind(file, 8);
    }
    else
    {
        hl_FileJumpBehind(file, 4);
    }

    // Read entire file
    void* fileData = std::malloc(fileSize);
    if (!fileData) return HL_ERROR_OUT_OF_MEMORY;

    result = file->ReadBytes(fileData, fileSize);
    if (HL_FAILED(result)) return result;

    // Header-specific
    uint32_t* offTable;
    uint32_t offCount;
    void* data;

    if (isMirage) // Mirage Header
    {
        // TODO
        offTable = nullptr;
        offCount = 0;
        data = nullptr;
    }
    else // Standard Header
    {
        // Endian-swap standard header
        hl_DHHStandardHeader* header = static_cast
            <hl_DHHStandardHeader*>(fileData);

        HL_ENDIAN_SWAP(hl_DHHStandardHeader, header);

        // Get offset table
        offTable = GetAbs<uint32_t>(
            fileData, header->OffsetTableOffset);

        hl_SwapUInt32(offTable);
        offCount = *offTable++;

        // Get data pointer
        data = GetAbs<void>(fileData, header->DataOffset);
    }

    // Fix offsets
    hl_HHFixOffsets(offTable, offCount, data);

    // Set blob pointer and return
    *blob = fileData;
    return HL_SUCCESS;
}

enum HL_RESULT hl_HHLoad(const char* path, void** blob)
{
    // TODO: Do stuff here instead of just calling hl_HHRead so you
    // can optimize-out the need to read the file size and backtrack.
    File file = File::OpenRead(path, true);
    return hl_HHRead(&file, blob);
}

enum HL_RESULT hl_HHStartWriteStandard(struct hl_File* file, uint32_t version)
{
    // Create "empty" header
    hl_DHHStandardHeader header = {};
    header.Header.Version = version;
    header.DataOffset = sizeof(hl_DHHStandardHeader);

    // Write header
    file->DoEndianSwap = true;
    HL_RESULT result = file->Write(header);

    // Set origin
    if (HL_OK(result))
    {
        file->Origin = file->Tell();
    }

    return result;
}

enum HL_RESULT hl_HHWriteOffsetTableStandard(const struct hl_File* file,
    const struct hl_OffsetTable* offTable)
{
    // Write offset count
    uint32_t offCount = static_cast<uint32_t>(offTable->size());
    HL_RESULT result = file->Write(offCount);
    if (HL_FAILED(result)) return result;

    // Write offsets
    uint32_t off;
    const long* offsets = offTable->data();

    for (size_t i = 0; i < offTable->size(); ++i)
    {
        off = static_cast<uint32_t>(
            offsets[i] - file->Origin);

        result = file->Write(off);
        if (HL_FAILED(result)) return result;
    }

    return result;
}

enum HL_RESULT hl_HHFinishWriteStandard(const struct hl_File* file, long headerPos,
    bool writeEOFThing, const struct hl_OffsetTable* offTable)
{
    // Write offset table
    uint32_t offTablePos = static_cast<uint32_t>(file->Tell());
    if (headerPos >= static_cast<long>(offTablePos))
        return HL_ERROR_UNKNOWN; // TODO: Return a better error

    HL_RESULT result = hl_HHWriteOffsetTableStandard(file, offTable);
    if (HL_FAILED(result)) return result;

    // TODO: Write EOF thing if told to

    // Fill-in file size
    uint32_t fileSize = static_cast<uint32_t>(file->Tell());
    file->JumpTo(headerPos);

    result = file->Write(fileSize);
    if (HL_FAILED(result)) return result;

    if (file->JumpAhead(4)) return HL_ERROR_UNKNOWN; // TODO: Return better error

    // Fill-in data size
    uint32_t dataSize = (offTablePos -
        sizeof(hl_DHHStandardHeader));

    result = file->Write(dataSize);
    if (HL_FAILED(result)) return result;

    file->JumpAhead(4);

    // Fill-in offset table position
    result = file->Write(offTablePos);
    if (HL_FAILED(result)) return result;

    // TODO: Fill-in EOF position if told to
    return result;
}

void hl_HHFreeBlob(void* blob)
{
#ifdef x64
    // Get offset table
    std::uint32_t offCount;
    std::uint32_t* offTable;
    void* data;

    if (!blob) return;

    if (hl_HHDetectHeaderType(static_cast<hl_DHHHeader*>(
        blob)) == HL_HHHEADER_TYPE_MIRAGE)
    {
        // TODO
        offCount = 0;
        offTable = nullptr;
        data = nullptr;
    }
    else
    {
        // Get offset table pointer
        hl_DHHStandardHeader* header = static_cast
            <hl_DHHStandardHeader*>(blob);

        offTable = GetAbs<std::uint32_t>(
            blob, header->OffsetTableOffset);

        // Get offset table count
        offCount = *offTable++;

        // Get data
        data = GetAbs<void>(blob, header->DataOffset);
    }

    // Free all offsets using data in offset table
    for (uint32_t i = 0; i < offCount; ++i)
    {
        hl_x64RemoveAbsPtr32(*GetAbs<hl_DataOff32>(
            data, offTable[i]));
    }
#endif
    
    // Free data
    std::free(blob);
}
