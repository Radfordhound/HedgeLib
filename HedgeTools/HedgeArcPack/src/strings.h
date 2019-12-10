#pragma once
#include "HedgeLib/String.h"
#include <iostream>
#include <cstdlib>

#ifdef _WIN32
#define ncout std::wcout
#define ncerr std::wcerr
#define ncin std::wcin
#define nstring std::wstring
#define nstrtoul std::wcstoul
#else
#define ncout std::cout
#define ncerr std::cerr
#define ncin std::cin
#define nstring std::string
#define nstrtoul std::strtoul
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
    ERROR_INVALID_INPUT,
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

extern const hl::nchar* const EnglishText[STRING_CONSTANT_COUNT];
extern const hl::nchar* const* Languages[LANGUAGE_COUNT];
extern LANGUAGE_TYPE CurrentLanguage;

inline const hl::nchar* const GetText(STRING_ID id)
{
    return Languages[CurrentLanguage][id];
}
