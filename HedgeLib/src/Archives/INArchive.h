#pragma once

namespace hl
{
    template<typename char_t>
    bool INArchiveNextSplit(char_t* splitCharPtr);

    template<typename char_t>
    bool INArchiveNextSplitPACxV3(char_t* splitCharPtr);

    template<typename char_t>
    bool INArchiveNextSplit(char_t* splitCharPtr, bool pacv3);
}
