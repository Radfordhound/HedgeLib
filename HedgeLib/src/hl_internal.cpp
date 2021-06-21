#include "hedgelib/hl_text.h"
#include <stdexcept>
#include <new>

#ifdef _MSC_VER
#include <malloc.h>
#elif defined(__cpp_aligned_new)
#include <cstdlib>
#include <cstring>
#else
#include <stdlib.h>
#include <cstring>
#endif

namespace hl
{
std::exception make_exception(error_type err)
{
    switch (err)
    {
    default:
    case error_type::unknown:
        return std::runtime_error(
            "An unknown error has occurred.");

    case error_type::unsupported:
        return std::runtime_error(
            "Attempted to perform an action which is not possible, or which "
            "is not currently supported.");

    case error_type::out_of_memory:
        return std::bad_alloc();

    case error_type::invalid_args:
        return std::invalid_argument(
            "One or more of the given arguments was invalid.");

    case error_type::out_of_range:
        return std::out_of_range(
            "A number has gone out of its expected or supported range.");

    case error_type::already_exists:
        return std::runtime_error(
            "The given file or directory already exists.");

    case error_type::not_found:
        return std::runtime_error(
            "The requested item could not be found.");

    case error_type::invalid_data:
        return std::runtime_error(
            "The given data was corrupt, or was not valid for use in the "
            "requested operation.");
    
    case error_type::no_more_entries:
        return std::runtime_error(
            "There were no more entries in the given directory.");

    case error_type::sharing_violation:
        return std::runtime_error(
            "The requested file could not be accessed as it is in-use by "
            "another application.");
    }
}

std::exception make_exception(error_type err, const char* what_arg)
{
    switch (err)
    {
    default:
        return make_exception(err);
    
    case error_type::invalid_args:
        return std::invalid_argument(std::string(what_arg) +
            " was invalid.");

    case error_type::out_of_range:
        return std::out_of_range(std::string(what_arg) +
            " has gone out of its expected or supported range.");
    }
}

void* aligned_malloc(std::size_t size, std::size_t alignment)
{
#ifdef _MSC_VER // MSVC
    return _aligned_malloc(size, alignment);
#elif defined(__cpp_aligned_new) // C++17
    return std::aligned_alloc(alignment, size);
#else // POSIX
    void* ptr = nullptr;
    posix_memalign(&ptr, alignment, size);
    return ptr;
#endif
}

void* aligned_realloc(void* ptr, std::size_t size, std::size_t alignment)
{
#ifdef _MSC_VER // MSVC
    return _aligned_realloc(ptr, size, alignment);
#elif defined(__cpp_aligned_new) // C++17
    void* newPtr = std::aligned_alloc(alignment, size);
    if (newPtr)
    {
        std::memcpy(newPtr, ptr, size);
        std::free(ptr);
    }

    return newPtr;
#else // POSIX
    void* newPtr = aligned_malloc(size, alignment);
    if (newPtr)
    {
        std::memcpy(newPtr, ptr, size);
        free(ptr);
    }

    return newPtr;
#endif
}

void aligned_free(void* ptr)
{
#ifdef _MSC_VER // MSVC
    _aligned_free(ptr);
#elif defined(__cpp_aligned_new) // C++17
    std::free(ptr);
#else // POSIX
    free(ptr);
#endif
}
} // hl
