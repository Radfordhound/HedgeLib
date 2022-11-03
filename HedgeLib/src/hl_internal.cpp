#include "hedgelib/hl_internal.h"
#include <string>
#include <new>

namespace hl
{
std::invalid_argument invalid_arg_exception(const char* argName)
{
    return std::invalid_argument(
        "The following argument was invalid: " +
        std::string(argName));
}

std::out_of_range out_of_range_exception(const char* argName)
{
    return std::out_of_range(
        "The following argument was out of the expected or supported range: " +
        std::string(argName));
}
} // hl
