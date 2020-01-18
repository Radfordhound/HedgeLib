#pragma once
#include "../Offsets.h"
#include "../String.h"
#include "../HedgeLib.h"
#include <vector>

namespace hl
{
    struct DS06FileEntry
    {
        /* @brief Name used by the game/lua files/etc. to refer to this file. */
        StringOffset32 FriendlyName;

        /* @brief Relative to either the win32 or the xenon folders within this arc
           based on the file's type. "ragdoll" files for example seem to be in xenon
           whereas "model" files are in win32. */
        StringOffset32 FilePath;
    };

    struct DS06TypeEntry
    {
        /*! @brief What type of data this is. "model" means xno, for example.
            I assume the game uses this to determine how to parse the data. */
        StringOffset32 TypeName;

        /*! @brief The count of file entries which are of this type. */
        std::uint32_t FileCount;

        /*! @brief Pointer to a sub-section of the Files array
            listed earlier that contains files of this type. */
        DataOffset32<DS06FileEntry> Files;

        inline void EndianSwap()
        {
            Swap(FileCount);
        }
    };

    struct DS06Package
    {
        ArrayOffset32<DS06FileEntry> Files;
        ArrayOffset32<DS06TypeEntry> Types;

        inline void EndianSwap()
        {
            Files.EndianSwap();
            Types.EndianSwap();
        }

        inline void EndianSwapRecursive(bool isBigEndian)
        {
            Files.EndianSwapRecursive(isBigEndian);
            Types.EndianSwapRecursive(isBigEndian);
        }
    };

    struct S06FileEntry
    {
        std::string FriendlyName;
        std::string FilePath;

        inline S06FileEntry() = default;
        inline S06FileEntry(std::string name) :
            FriendlyName(name) {}

        inline S06FileEntry(std::string name, std::string filePath) :
            FriendlyName(name), FilePath(filePath) {}
    };

    struct S06TypeEntry
    {
        std::string Name;
        std::vector<S06FileEntry> Files;

        inline S06TypeEntry() = default;
        inline S06TypeEntry(std::string name) : Name(name) {}
    };

    using S06Package = std::vector<S06TypeEntry>;
    
    HL_API void SaveS06Package(const char* filePath, const S06Package& pkg);

#ifdef _WIN32
    HL_API void SaveS06Package(const nchar* filePath, const S06Package& pkg);
#endif
}
