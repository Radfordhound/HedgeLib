#pragma once
#include <cstddef>

#define PARSE_FUNC Parse(GFX::Instance& inst,\
    const char* name, void* data, std::size_t size,\
    const char* groupName)

#define DECLARE_PARSER(type) struct type { static void PARSE_FUNC; }
#define DEFINE_PARSER(type) void type::PARSE_FUNC

namespace HedgeEdit
{
    namespace GFX
    {
        class Instance;
    }

    namespace Data
    {
        using ParseFuncPtr = void (*)(GFX::Instance& inst,
            const char* name, void* data, std::size_t size,\
            const char* groupName);
    }
}
