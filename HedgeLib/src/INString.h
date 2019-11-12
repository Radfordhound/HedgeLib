#pragma once
#include "HedgeLib/String.h"

namespace hl
{
    void INStringConvertUTF8ToUTF16NoAlloc(const char* u8str,
        char16_t* u16str, std::size_t u16bufLen, std::size_t u8bufLen);

    void INStringConvertUTF8ToUTF16NoAlloc(const char* u8str,
        char16_t* u16str, std::size_t u8bufLen);

    template<typename char_t>
    void INStringConvertUnicodeToCP932NoAlloc(
        const char_t* ustr, char* cp932str, std::size_t ubufLen);

    void INStringConvertUTF16ToUTF8NoAlloc(const char16_t* u16str,
        char* u8str, std::size_t u8bufLen, std::size_t u16bufLen);

    void INStringConvertUTF16ToUTF8NoAlloc(const char16_t* u16str,
        char* u8str, std::size_t u16bufLen);
}
