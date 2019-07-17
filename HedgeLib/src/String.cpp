#include "HedgeLib/String.h"

const char* const hl_EmptyString = "";

bool hl_StringsEqualASCII(const char* str1, const char* str2)
{
    if (str1 == str2) return true;
    if (!str1 || !*str1) return false;

    while (*str1 || *str2)
    {
        if (*str1 != *str2 && HL_TOLOWERASCII(*str1) !=
            HL_TOLOWERASCII(*str2))
        {
            return false;
        }

        ++str1;
        ++str2;
    }
    
    return true;
}
