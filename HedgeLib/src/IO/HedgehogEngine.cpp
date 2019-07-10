#include "HedgeLib/IO/HedgehogEngine.h"
#include "HedgeLib/IO/File.h"
#include "HedgeLib/Offsets.h"
#include "../INBlob.h"

const char* const hl_HHMirageTypeContexts = "Contexts";

HL_IMPL_ENDIAN_SWAP_CPP(hl_DHHHeader);
HL_IMPL_ENDIAN_SWAP(hl_DHHHeader)
{
    hl_SwapUInt32(&v->FileSize);
    hl_SwapUInt32(&v->Version);
}

HL_IMPL_ENDIAN_SWAP_CPP(hl_DHHStandardHeader);
HL_IMPL_ENDIAN_SWAP(hl_DHHStandardHeader)
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
        offPtr = hl_GetAbs<hl_DataOff32>(data, offTable[i]);

        // Fix offset
        hl_FixOffset(offPtr, data, true);
    }
}

const void* hl_HHMirageGetDataNode(const struct hl_Blob* blob)
{
    // TODO
    return nullptr;
}

const void* hl_HHStandardGetData(const struct hl_Blob* blob)
{
    const hl_DHHStandardHeader* header =
        blob->GetData<hl_DHHStandardHeader>();

    return HL_GETABSV(header, header->DataOffset);
}

const void* hl_HHMirageGetData(const struct hl_Blob* blob)
{
    // TODO
    return nullptr;
}

const void* hl_HHGetData(const struct hl_Blob* blob)
{
    // Mirage Header
    if (hl_HHDetectHeaderType(blob->GetData<
        hl_DHHHeader>()) == HL_HHHEADER_TYPE_MIRAGE)
    {
        return hl_HHMirageGetData(blob);
    }

    // Standard Header
    return hl_HHStandardGetData(blob);
}

enum HL_RESULT hl_HHRead(struct hl_File* file, struct hl_Blob** blob)
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

        file->JumpBehind(8);
    }
    else
    {
        file->JumpBehind(4);
    }

    // Read entire file
    *blob = hl_INCreateBlob(HL_BLOB_TYPE_HEDGEHOG_ENGINE, fileSize);
    if (!(*blob)) return HL_ERROR_OUT_OF_MEMORY;

    void* fileData = (*blob)->GetData<void>();
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
        offTable = hl_GetAbs<uint32_t>(
            fileData, header->OffsetTableOffset);

        hl_SwapUInt32(offTable);
        offCount = *offTable++;

        // Get data pointer
        data = hl_GetAbs<void>(fileData, header->DataOffset);
    }

    // Fix offsets
    hl_HHFixOffsets(offTable, offCount, data);
    return HL_SUCCESS;
}

enum HL_RESULT hl_HHLoad(const char* filePath, struct hl_Blob** blob)
{
    // TODO: Do stuff here instead of just calling hl_HHRead so you
    // can optimize-out the need to read the file size and backtrack.
    hl_File file = hl_File::OpenRead(std::filesystem::u8path(filePath), true);
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
    const hl_OffsetTable* offTable)
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
    bool writeEOFThing, const hl_OffsetTable* offTable)
{
    // Write offset table
    uint32_t offTablePos = static_cast<uint32_t>(file->Tell());
    if (headerPos >= static_cast<long>(offTablePos))
        return HL_ERROR_UNKNOWN; // TODO: Return a better error

    HL_RESULT result = hl_HHWriteOffsetTableStandard(file, offTable);
    if (HL_FAILED(result)) return result;

    // TODO: Write EOF thing if told to

    // Fill-in file size
    uint32_t fileSize = static_cast<uint32_t>(file->Tell()) - headerPos;
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

void hl_HHFreeBlob(struct hl_Blob* blob)
{
#ifdef x64
    // Get offset table
    std::uint32_t offCount;
    std::uint32_t* offTable;
    void* data;

    if (!blob) return;

    if (hl_HHDetectHeaderType(blob->GetData<
        hl_DHHHeader>()) == HL_HHHEADER_TYPE_MIRAGE)
    {
        // TODO
        offCount = 0;
        offTable = nullptr;
        data = nullptr;
    }
    else
    {
        // Get offset table pointer
        hl_DHHStandardHeader* header = blob->GetData
            <hl_DHHStandardHeader>();

        offTable = hl_GetAbs<std::uint32_t>(
            header, header->OffsetTableOffset);

        // Get offset table count
        offCount = *offTable++;

        // Get data
        data = hl_GetAbs<void>(header, header->DataOffset);
    }

    // Free all offsets using data in offset table
    for (uint32_t i = 0; i < offCount; ++i)
    {
        hl_x64RemoveAbsPtr32(*hl_GetAbs<hl_DataOff32>(
            data, offTable[i]));
    }
#endif
    
    // Free data
    std::free(blob);
}
