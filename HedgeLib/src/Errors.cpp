#include "HedgeLib/Errors.h"
#include "HedgeLib/Result.h"

static const HL_RESULT hl_INResultStringsCount = static_cast<HL_RESULT>(6);

static const char* const hl_INResultStrings[hl_INResultStringsCount] =
{
#include "INErrors.h"
};

#ifdef _WIN32
static const hl_NativeChar* const hl_INResultStringsNative[hl_INResultStringsCount] =
{
#define HL_INRESULT_STRING(str) L##str
#include "INErrors.h"
};
#else
#define hl_INResultStringsNative hl_INResultStrings
#endif

const char* hl_GetResultString(HL_RESULT result)
{
    return hl_INResultStrings[
        (result >= hl_INResultStringsCount) ?
            1 : result];
}

const hl_NativeChar* hl_GetResultStringNative(HL_RESULT result)
{
    return hl_INResultStringsNative[
        (result >= hl_INResultStringsCount) ?
        1 : result];
}
