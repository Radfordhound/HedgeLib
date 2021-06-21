#ifndef HL_IN_WIN32_H_INCLUDED
#define HL_IN_WIN32_H_INCLUDED
#ifdef _WIN32
#include "hedgelib/hl_internal.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace hl
{
inline error_type in_win32_get_error(HRESULT hresult) noexcept
{
    switch (hresult)
    {
    case E_INVALIDARG: return error_type::invalid_args;
    case E_OUTOFMEMORY: return error_type::out_of_memory;

    case __HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
    case __HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND):
        return error_type::not_found;

    case __HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES):
        return error_type::no_more_entries;

    case __HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION):
        return error_type::sharing_violation;

    default: return error_type::unknown;
    }
}

inline error_type in_win32_get_last_error()
{
    return in_win32_get_error(HRESULT_FROM_WIN32(GetLastError()));
}

#define HL_IN_WIN32_ERROR()\
    HL_ERROR(in_win32_get_last_error())
} // hl
#endif
#endif
