#ifndef HL_TOOL_HELPERS_H_INCLUDED
#define HL_TOOL_HELPERS_H_INCLUDED
#include "hl_text.h"
#include <cstdlib>
#include <cstdarg>

#ifdef HL_IN_WIN32_UNICODE
#include <cwchar>
#else
#include <cstdio>
#endif

namespace hl
{
#ifdef HL_IN_WIN32_UNICODE
/**
    @brief Helper macro for program entry point.

    Expands to "wmain" on Windows (unless HL_WIN32_FORCE_ANSI is defined),
    and "main" on everything else.
*/
#define HL_NMAIN wmain
#else
/**
    @brief Helper macro for program entry point.

    Expands to "wmain" on Windows (unless HL_WIN32_FORCE_ANSI is defined),
    and "main" on everything else.
*/
#define HL_NMAIN main
#endif

#ifdef NDEBUG
#define HL_RELEASE_TRY try
#define HL_RELEASE_CATCH(catchExpression, code) catch (catchExpression) { code }
#else
#define HL_RELEASE_TRY
#define HL_RELEASE_CATCH(catchExpression, code)
#endif

/**
    @brief Native-character version of std::fgetc.
    @param stream   The stream to get a native character from.
    
    @return On success, the native character returned from the
            stream, casted to an int. On failure, EOF.
*/
inline int nfgetc(std::FILE* stream)
{
#ifdef HL_IN_WIN32_UNICODE
    const std::wint_t r = std::fgetwc(stream);
    return (r == WEOF) ? EOF : static_cast<int>(r);
#else
    return std::fgetc(stream);
#endif
}

/**
    @brief Native-character version of std::getc.
    @param stream   The stream to get a native character from.

    @return On success, the native character returned from the
            stream, casted to an int. On failure, EOF.
*/
inline int ngetc(std::FILE* stream)
{
    return nfgetc(stream);
}

/**
    @brief Native-character version of std::getchar.

    @return On success, the native character returned from
            stdout, casted to an int. On failure, EOF.
*/
inline int ngetchar()
{
    return nfgetc(stdin);
}

inline nchar* nfgets(nchar* buf, int maxCount, std::FILE* stream)
{
#ifdef HL_IN_WIN32_UNICODE
    return std::fgetws(buf, maxCount, stream);
#else
    return std::fgets(buf, maxCount, stream);
#endif
}

inline int nvsscanf(const nchar* str, const nchar* fmt, std::va_list argList)
{
#ifdef HL_IN_WIN32_UNICODE
    return std::vswscanf(str, fmt, argList);
#else
    return std::vsscanf(str, fmt, argList);
#endif
}

inline int nsscanf(const nchar* str, const nchar* fmt, ...)
{
    std::va_list argList;
    int r;

    va_start(argList, fmt);
    r = nvsscanf(str, fmt, argList);
    va_end(argList);

    return r;
}

inline int nvfscanf(std::FILE* stream, const nchar* fmt, std::va_list argList)
{
#ifdef HL_IN_WIN32_UNICODE
    return std::vfwscanf(stream, fmt, argList);
#else
    return std::vfscanf(stream, fmt, argList);
#endif
}

inline int nfscanf(std::FILE* stream, const nchar* fmt, ...)
{
    std::va_list argList;
    int r;

    va_start(argList, fmt);
    r = nvfscanf(stream, fmt, argList);
    va_end(argList);

    return r;
}

inline int nvscanf(const nchar* fmt, std::va_list argList)
{
#ifdef HL_IN_WIN32_UNICODE
    return std::vwscanf(fmt, argList);
#else
    return std::vscanf(fmt, argList);
#endif
}

inline int nscanf(const nchar* fmt, ...)
{
    std::va_list argList;
    int r;

    va_start(argList, fmt);
    r = nvscanf(fmt, argList);
    va_end(argList);

    return r;
}

inline int nfputc(nchar ch, std::FILE* stream)
{
#ifdef HL_IN_WIN32_UNICODE
    const std::wint_t r = std::fputwc(ch, stream);
    return (r == WEOF) ? EOF : static_cast<int>(r);
#else
    return std::fputc(ch, stream);
#endif
}

inline int nputc(nchar ch, std::FILE* stream)
{
    return nfputc(ch, stream);
}

inline int nputchar(nchar ch)
{
    return nfputc(ch, stdout);
}

inline int nfputs(const nchar* str, std::FILE* stream)
{
#ifdef HL_IN_WIN32_UNICODE
    return std::fputws(str, stream);
#else
    return std::fputs(str, stream);
#endif
}

inline int nputs(const nchar* str)
{
#ifdef HL_IN_WIN32_UNICODE
    // NOTE: For some reason, _putws returns WEOF
    // on failure despite WEOF being an std::wint_t
    // (unsigned short in this case) and _putws 
    // returning an int.
    const int r = _putws(str);
    return (r == WEOF) ? EOF : r;
#else
    return std::puts(str);
#endif
}

inline int nvsnprintf(nchar* buf, std::size_t bufCount,
    const nchar* fmt, std::va_list argList)
{
#ifdef HL_IN_WIN32_UNICODE
    return _vsnwprintf(buf, bufCount, fmt, argList);
#else
    return std::vsnprintf(buf, bufCount, fmt, argList);
#endif
}

inline int nsnprintf(nchar* buf, std::size_t bufCount,
    const nchar* fmt, ...)
{
    std::va_list argList;
    int r;

    va_start(argList, fmt);
    r = nvsnprintf(buf, bufCount, fmt, argList);
    va_end(argList);

    return r;
}

inline int nvsprintf(nchar* buf, const nchar* fmt, std::va_list argList)
{
#ifdef HL_IN_WIN32_UNICODE
    return _vswprintf(buf, fmt, argList);
#else
    return std::vsprintf(buf, fmt, argList);
#endif
}

inline int nsprintf(nchar* buf, const nchar* fmt, ...)
{
    std::va_list argList;
    int r;

    va_start(argList, fmt);
    r = nvsprintf(buf, fmt, argList);
    va_end(argList);

    return r;
}

inline int nvfprintf(std::FILE* stream, const nchar* fmt, std::va_list argList)
{
#ifdef HL_IN_WIN32_UNICODE
    return std::vfwprintf(stream, fmt, argList);
#else
    return std::vfprintf(stream, fmt, argList);
#endif
}

inline int nfprintf(std::FILE* stream, const nchar* fmt, ...)
{
    std::va_list argList;
    int r;

    va_start(argList, fmt);
    r = nvfprintf(stream, fmt, argList);
    va_end(argList);

    return r;
}

inline int nvprintf(const nchar* fmt, std::va_list argList)
{
#ifdef HL_IN_WIN32_UNICODE
    return std::vwprintf(fmt, argList);
#else
    return std::vprintf(fmt, argList);
#endif
}

inline int nprintf(const nchar* fmt, ...)
{
    std::va_list argList;
    int r;

    va_start(argList, fmt);
    r = nvprintf(fmt, argList);
    va_end(argList);

    return r;
}

namespace console
{
HL_API nstring read_line(std::FILE* stream);

inline nstring read_line()
{
    return read_line(stdin);
}

HL_API void write(nchar ch, std::FILE* stream);

inline void write(nchar ch)
{
    write(ch, stdout);
}

HL_API void write(const nchar* str, std::FILE* stream);

inline void write(const nstring& str, std::FILE* stream)
{
    write(str.c_str(), stream);
}

inline void write(const nchar* str)
{
    write(str, stdout);
}

inline void write(const nstring& str)
{
    write(str, stdout);
}

HL_API void write_line(const nchar* str, std::FILE* stream);

inline void write_line(const nstring& str, std::FILE* stream)
{
    write_line(str.c_str(), stream);
}

HL_API void write_line(const nchar* str);

inline void write_line(const nstring& str)
{
    write_line(str.c_str());
}

inline void write_line(std::FILE* stream)
{
    write(HL_NTEXT('\n'), stream);
}

inline void write_line()
{
    write_line(stdout);
}

#ifdef _WIN32
HL_API bool should_pause();
#else
constexpr bool should_pause()
{
    return false;
}
#endif

HL_API void pause(language lang);

inline void pause_if_necessary(language lang)
{
    if (should_pause())
    {
        pause(lang);
    }
}
} // console
} // hl
#endif
