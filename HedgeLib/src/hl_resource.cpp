#include "hedgelib/hl_resource.h"
#include "hedgelib/io/hl_path.h"

namespace hl
{
std::string get_res_name(const nchar* filePath)
{
#ifdef HL_IN_WIN32_UNICODE
    return text::conv<text::native_to_utf8>(
        path::remove_ext(path::get_name(filePath)));
#else
    return path::remove_ext(path::get_name(filePath));
#endif
}

std::string get_res_name(const nstring& filePath)
{
#ifdef HL_IN_WIN32_UNICODE
    return text::conv<text::native_to_utf8>(
        path::remove_ext(path::get_name(filePath)));
#else
    return path::remove_ext(path::get_name(filePath));
#endif
}
} // hl
