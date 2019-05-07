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

enum HL_RESULT hl_HHRead(FILE* file, void** blob)
{
    // Get file size
    uint32_t fileSize = hl_FileReadUInt32(file, true);
    bool isMirage = false; // "Friends are nothing but a fleeting illusion"

    if (fileSize & HL_HHMIRAGE_FLAGS_MASK) // "Your mask can't hide how sad and lonely you are!"
    {
        // Make sure this is really a Mirage header
        uint32_t version = hl_FileReadUInt32(file, true);
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
    fread(fileData, fileSize, 1, file);
    // TODO: Error checking

    // Header-specific
    uint32_t* offTable;
    uint32_t offCount;
    void* data;

    if (isMirage)
    {
        // TODO
        offTable = nullptr;
        offCount = 0;
        data = nullptr;
    }
    else
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
    File file = File::OpenRead(path);
    return hl_HHRead(file.Get(), blob);
}

void hl_HHStartWriteStandard(FILE* file, uint32_t version)
{
    hl_DHHStandardHeader header = {};
    header.Header.Version = version;
    header.DataOffset = sizeof(hl_DHHStandardHeader);

    fwrite(&header, sizeof(header), 1, file);
}

void hl_HHWriteOffsetTableStandard(FILE* file,
    const struct hl_OffsetTable* offTable)
{
    File f = File(file, true);

    // Write offset count
    size_t offCount = offTable->size();
    f.Write(&offCount);

    // Write offsets
    uint32_t off;
    for (size_t i = 0; i < offTable->size(); ++i)
    {
        off = static_cast<long>(offTable->data()[i]);
        f.Write(&off);
    }
}

void hl_HHFinishWriteStandard(FILE* file, long headerPos,
    bool writeEOFThing, const struct hl_OffsetTable* offTable)
{
    File f = File(file, true);

    // Write offset table
    long offTablePos = f.Tell();
    hl_HHWriteOffsetTableStandard(file, offTable);

    // TODO: Write EOF thing if told to

    // Fill-in file size
    long fileSize = f.Tell();
    f.JumpTo(headerPos);

    f.Write(&fileSize);
    f.JumpAhead(4);

    // Fill-in data size
    long dataSize = (fileSize - offTablePos -
        sizeof(hl_DHHStandardHeader));

    f.Write(&dataSize);
    f.JumpAhead(4);

    // Fill-in offset table position
    f.Write(&offTablePos);

    // TODO: Fill-in EOF position if told to
}

void hl_HHFree(void* blob)
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
