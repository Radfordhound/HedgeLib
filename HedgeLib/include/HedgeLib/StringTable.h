#pragma once
#include <vector>
#include <cstring>

namespace hl
{
    struct StringTableEntry
    {
        /*! @brief The string referenced by the DataOffset. */
        const char* String;
        /*! @brief The position of the DataOffset which points to String within the file. */
        long OffPosition;
        /*! @brief The length of String, not counting its null terminator. */
        std::size_t Length;

        inline StringTableEntry() = default;
        inline StringTableEntry(const char* str, long offPos, std::size_t len = 0) :
            String(str), OffPosition(offPos), Length((len) ? len : std::strlen(str)) {}
    };

    using StringTable = std::vector<StringTableEntry>;
}
