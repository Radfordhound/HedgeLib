#pragma once
#include "../Endian.h"
#include "../String.h"

#ifdef __cplusplus
extern "C" {
#endif

// Credit to Skyth for cracking all the "Mirage" stuff
#define HL_HHMIRAGE_FLAGS_MASK 0xE0000000U
#define HL_HHMIRAGE_SIZE_MASK 0x1FFFFFFFU
#define HL_HHMIRAGE_SIGNATURE 0x0133054AU

HL_API extern const char* const hl_HHMirageContextsType;

typedef struct hl_Blob hl_Blob;
typedef struct hl_File hl_File;

typedef enum HL_HHHEADER_TYPE
{
    HL_HHHEADER_TYPE_STANDARD,
    HL_HHHEADER_TYPE_MIRAGE
}
HL_HHHEADER_TYPE;

typedef struct hl_HHStandardHeader
{
    uint32_t FileSize;
    uint32_t Version;
    uint32_t DataSize;
    uint32_t DataOffset;
    uint32_t OffsetTableOffset;
    uint32_t EOFOffset; // Maybe supposed to be "next node offset"?

    HL_DECL_ENDIAN_SWAP_CPP();
}
hl_HHStandardHeader;

HL_STATIC_ASSERT_SIZE(hl_HHStandardHeader, 24);
HL_DECL_ENDIAN_SWAP(hl_HHStandardHeader);

// "Friends are nothing but a fleeting illusion"
// get it because "mirage" and "illusion" ok I'm sorry
typedef enum HL_HHMIRAGE_NODE_FLAGS
{
    HL_HHMIRAGE_NODE_HAS_NO_CHILDREN = 0x20000000U,
    HL_HHMIRAGE_NODE_IS_LAST_CHILD = 0x40000000U,
    HL_HHMIRAGE_NODE_IS_ROOT = 0x80000000U,
}
HL_HHMIRAGE_NODE_FLAGS;

typedef struct hl_HHMirageHeader
{
    uint32_t FileSize;
    uint32_t Signature; // Apparently the game doesn't check this lol
    uint32_t OffsetTableOffset;
    uint32_t OffsetCount;

    HL_DECL_ENDIAN_SWAP_CPP();
}
hl_HHMirageHeader;

HL_STATIC_ASSERT_SIZE(hl_HHMirageHeader, 16);
HL_DECL_ENDIAN_SWAP(hl_HHMirageHeader);

typedef struct hl_HHMirageNode
{
    uint32_t Size;
    uint32_t Value;
    char Name[8];

    HL_DECL_ENDIAN_SWAP_CPP();
}
hl_HHMirageNode;

HL_STATIC_ASSERT_SIZE(hl_HHMirageNode, 16);
HL_DECL_ENDIAN_SWAP(hl_HHMirageNode);

////inline bool hl_HHHeaderIsMirage(const void* header)
////{
////    // "Friends are nothing but a fleeting illusion"
////    // get it because "mirage" and "illusion" and ok I'm sorry
////    return (*((uint32_t*)header) & HL_HHMIRAGE_NODE_IS_ROOT);
////}

inline HL_HHHEADER_TYPE hl_HHDetectHeaderType(const void* header)
{
    return (*((uint32_t*)header) & HL_HHMIRAGE_NODE_IS_ROOT) ?
        HL_HHHEADER_TYPE_MIRAGE : HL_HHHEADER_TYPE_STANDARD;
}

HL_API void hl_HHFixOffsets(uint32_t* offTable,
    uint32_t offCount, void* data);

inline HL_HHMIRAGE_NODE_FLAGS hl_HHMirageNodeGetFlags(
    const hl_HHMirageNode* node)
{
    // "Your mask can't hide how sad and lonely you are!"
    return (HL_HHMIRAGE_NODE_FLAGS)(
        node->Size & HL_HHMIRAGE_FLAGS_MASK);
}

inline uint32_t hl_HHMirageNodeGetSize(const hl_HHMirageNode* node)
{
    // "HA! You think you can intimidate ME with just your size?!"
    return (node->Size & HL_HHMIRAGE_SIZE_MASK);
}

HL_API const void* hl_HHMirageNodeGetData(const hl_HHMirageNode* node);

HL_API const hl_HHMirageNode* hl_HHMirageGetChildNodes(
    const hl_HHMirageNode* node);

HL_API const hl_HHMirageNode* hl_HHMirageGetNextNode(
    const hl_HHMirageNode* node);

HL_API const hl_HHMirageNode* hl_HHMirageGetNode(
    const hl_HHMirageNode* parentNode, const char* name,
    bool HL_DEFARG(recursive, true));

HL_API const hl_HHMirageNode* hl_HHMirageGetDataNode(const hl_Blob* blob);

HL_API const void* hl_HHGetDataStandard(const hl_Blob* blob);
HL_API const void* hl_HHGetDataMirage(const hl_Blob* blob);
HL_API const void* hl_HHGetData(const hl_Blob* blob);
HL_API HL_RESULT hl_HHRead(hl_File* file, hl_Blob** blob);

/*! @brief Direct-loads a given file in the "Standard" or "Mirage" Hedgehog Engine node formats.
 *
 * This function auto-detects the header type of the file at the given path (both
 * "Standard" and "Mirage" formats are supported), loads its contents into a blob, endian-swaps
 * the header and offset table, and finally uses the offset table to fix each offset.
 *
 * It does not do any form of endian-swapping on the data itself as this requires the type of data
 * to be known. You must retrieve the data yourself with hl_HHGetData (or hl_BlobGetData) and
 * endian-swap it using HL_ENDIAN_SWAP_RECURSIVE (or data->EndianSwapRecursive if you're using C++).
 *
 * @remark Does not endian-swap the data itself as this is not possible without knowing its type.
 * You must swap the data yourself using HL_ENDIAN_SWAP_RECURSIVE (or data->EndianSwapRecursive if you're using C++).
 *
 * @param[in] filePath The path of the file to be loaded.
 * @param[out] blob The pointer to the blob which will contain the loaded data.
 * Must be freed when no longer in-use using free, otherwise memory will be leaked.
 *
 * @errors If this function fails, it will return an HL_RESULT specifying the error.
 * TODO: List possible errors
 *
 * @return An HL_RESULT specifying an error code if an error occurred, otherwise `HL_SUCCESS`.
 * You can check if the function failed using HL_FAILED or if the function succeeded using HL_OK.
 *
 * @sa @ref hl_HHLoadNative
 * @sa @ref hl_HHRead
 * @ingroup archives
*/
HL_API HL_RESULT hl_HHLoad(const char* filePath, hl_Blob** blob);
HL_API HL_RESULT hl_HHLoadNative(const hl_NativeChar* filePath, hl_Blob** blob);

HL_API HL_RESULT hl_HHStartWriteStandard(hl_File* file, uint32_t version);

HL_API HL_RESULT hl_HHStartWriteMirage(hl_File* file);

HL_API HL_RESULT hl_HHStartWriteMirageNode(const hl_File* file,
    uint32_t value, const char* name);

HL_API HL_RESULT hl_HHFinishWriteMirageNode(const hl_File* file,
    long nodePos, uint32_t flags);

HL_API HL_RESULT hl_HHFinishWriteMirage(const hl_File* file,
    long headerPos, const hl_OffsetTable* offTable);

HL_API HL_RESULT hl_HHWriteOffsetTable(const hl_File* file,
    const hl_OffsetTable* offTable);

HL_API HL_RESULT hl_HHWriteOffsetTableStandard(const hl_File* file,
    const hl_OffsetTable* offTable);

HL_API HL_RESULT hl_HHFinishWriteStandard(const hl_File* file, long headerPos,
    const hl_OffsetTable* offTable, bool HL_DEFARG(writeEOFPadding, false));

#ifdef __cplusplus
}

// Non-const overloads
inline void* hl_HHMirageNodeGetData(hl_HHMirageNode* node)
{
    return const_cast<void*>(hl_HHMirageNodeGetData(
        const_cast<const hl_HHMirageNode*>(node)));
}

inline hl_HHMirageNode* hl_HHMirageGetChildNodes(
    hl_HHMirageNode* node)
{
    return const_cast<hl_HHMirageNode*>(hl_HHMirageGetChildNodes(
        const_cast<const hl_HHMirageNode*>(node)));
}

inline hl_HHMirageNode* hl_HHMirageGetNextNode(
    hl_HHMirageNode* node)
{
    return const_cast<hl_HHMirageNode*>(hl_HHMirageGetNextNode(
        const_cast<const hl_HHMirageNode*>(node)));
}

inline hl_HHMirageNode* hl_HHMirageGetNode(
    hl_HHMirageNode* parentNode, const char* name,
    bool recursive = true)
{
    return const_cast<hl_HHMirageNode*>(hl_HHMirageGetNode(
        const_cast<const hl_HHMirageNode*>(parentNode),
        name, recursive));
}

inline hl_HHMirageNode* hl_HHMirageGetDataNode(hl_Blob* blob)
{
    return const_cast<hl_HHMirageNode*>(hl_HHMirageGetDataNode(
        const_cast<const hl_Blob*>(blob)));
}

inline const void* hl_HHGetDataStandard(hl_Blob* blob)
{
    return const_cast<void*>(hl_HHGetDataStandard(
        const_cast<const hl_Blob*>(blob)));
}

inline const void* hl_HHGetDataMirage(hl_Blob* blob)
{
    return const_cast<void*>(hl_HHGetDataMirage(
        const_cast<const hl_Blob*>(blob)));
}

inline void* hl_HHGetData(hl_Blob* blob)
{
    return const_cast<void*>(hl_HHGetData(
        const_cast<const hl_Blob*>(blob)));
}

// Helper functions
template<typename T>
inline T* hl_HHMirageNodeGetData(hl_HHMirageNode* node)
{
    return static_cast<T*>(hl_HHMirageNodeGetData(node));
}

template<typename T>
inline const T* hl_HHMirageNodeGetData(const hl_HHMirageNode* node)
{
    return static_cast<const T*>(hl_HHMirageNodeGetData(node));
}

template<typename T>
inline T* hl_HHGetDataStandard(hl_Blob* blob)
{
    return static_cast<T*>(hl_HHGetDataStandard(blob));
}

template<typename T>
inline const T* hl_HHGetDataStandard(const hl_Blob* blob)
{
    return static_cast<const T*>(hl_HHGetDataStandard(blob));
}

template<typename T>
inline T* hl_HHGetDataMirage(hl_Blob* blob)
{
    return static_cast<T*>(hl_HHGetDataMirage(blob));
}

template<typename T>
inline const T* hl_HHGetDataMirage(const hl_Blob* blob)
{
    return static_cast<const T*>(hl_HHGetDataMirage(blob));
}

template<typename T>
inline T* hl_HHGetData(hl_Blob* blob)
{
    return static_cast<T*>(hl_HHGetData(blob));
}

template<typename T>
inline const T* hl_HHGetData(const hl_Blob* blob)
{
    return static_cast<const T*>(hl_HHGetData(blob));
}

// Windows-specific overloads
#ifdef _WIN32
inline HL_RESULT hl_HHLoad(const hl_NativeChar* filePath, hl_Blob** blob)
{
    return hl_HHLoadNative(filePath, blob);
}
#endif
#endif
