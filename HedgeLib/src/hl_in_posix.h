#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#ifndef HL_IN_POSIX_H_INCLUDED
#define HL_IN_POSIX_H_INCLUDED
#include <string>
#include <system_error>
#include <cerrno>

namespace hl
{
inline std::system_error in_posix_get_exception(int err)
{
    return std::system_error(err, std::generic_category());
}

inline std::system_error in_posix_get_exception(int err, const char* whatArg)
{
    return std::system_error(err, std::generic_category(), whatArg);
}

inline std::system_error in_posix_get_exception(int err, const std::string& whatArg)
{
    return std::system_error(err, std::generic_category(), whatArg);
}

inline std::system_error in_posix_get_last_exception()
{
    return in_posix_get_exception(errno);
}

inline std::system_error in_posix_get_last_exception(const char* whatArg)
{
    return in_posix_get_exception(errno, whatArg);
}

inline std::system_error in_posix_get_last_exception(const std::string& whatArg)
{
    return in_posix_get_exception(errno, whatArg);
}
} // hl
#endif
#endif
