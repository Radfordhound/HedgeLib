#pragma once
#include <HedgeLib/String.h>
#include <iostream>
#include <cstdlib>

#ifdef _WIN32
#define ncout std::wcout
#define ncin std::wcin
#define nstring std::wstring
#else
#define ncout std::cout
#define ncin std::cin
#define nstring std::string
#endif

enum STRING_ID
{
    VERSION_STRING,
    USAGE_STRING,
    HELP1_STRING,
    HELP2_STRING,

    ERROR_STRING,
    ERROR_INVALID_FLAGS,
    ERROR_TOO_MANY_MODES,
    ERROR_TOO_MANY_ARGUMENTS,
    ERROR_INVALID_TYPE,
    ERROR_INVALID_SPLIT_LIMIT,
    ERROR_NO_INPUT,

    TYPE1_STRING,
    TYPE2_STRING,

    EXTRACTING_STRING,
    PACKING_STRING,
    DONE1_STRING,
    DONE2_STRING,

    FILE_TYPE_HEROES,
    FILE_TYPE_STORYBOOK,
    FILE_TYPE_HEDGEHOG,
    FILE_TYPE_PACV2,
    FILE_TYPE_PACV3,

    STRING_CONSTANT_COUNT
};

enum LANGUAGE_TYPE
{
    ENGLISH,
    LANGUAGE_COUNT
};

extern const hl_NativeStr const EnglishText[STRING_CONSTANT_COUNT];
extern const hl_NativeStr const* Languages[LANGUAGE_COUNT];
extern LANGUAGE_TYPE CurrentLanguage;

inline const hl_NativeStr const GetText(STRING_ID id)
{
    return Languages[CurrentLanguage][id];
}
