#ifdef _WIN32
#ifndef HL_IN_WIN32_H_INCLUDED
#define HL_IN_WIN32_H_INCLUDED
#include <string>
#include <system_error>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace hl
{
inline std::system_error in_win32_get_exception(DWORD err)
{
    return std::system_error(err, std::generic_category());
}

inline std::system_error in_win32_get_exception(DWORD err, const char* whatArg)
{
    return std::system_error(err, std::generic_category(), whatArg);
}

inline std::system_error in_win32_get_exception(DWORD err, const std::string& whatArg)
{
    return std::system_error(err, std::generic_category(), whatArg);
}

inline std::system_error in_win32_get_last_exception()
{
    return in_win32_get_exception(GetLastError());
}

inline std::system_error in_win32_get_last_exception(const char* whatArg)
{
    return in_win32_get_exception(GetLastError(), whatArg);
}

inline std::system_error in_win32_get_last_exception(const std::string& whatArg)
{
    return in_win32_get_exception(GetLastError(), whatArg);
}
} // hl
#endif
#endif
