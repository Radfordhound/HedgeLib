#pragma once
#include <vector>

namespace hl
{
    struct StringTableEntry
    {
        const char* String; // The string referenced by the DataOffset.
        long OffPosition;   // The position of the DataOffset which points to String within the file.

        inline StringTableEntry() = default;
        inline StringTableEntry(const char* str, long offPos) :
            String(str), OffPosition(offPos) {}
    };

    using StringTable = std::vector<StringTableEntry>;
}
