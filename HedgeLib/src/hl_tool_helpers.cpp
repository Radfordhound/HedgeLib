#include "hedgelib/hl_tool_helpers.h"

#ifdef _WIN32
// NOTE: We define _WIN32_WINNT so we can use some additional functions we require.
#define _WIN32_WINNT 0x0500
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace hl
{
enum class in_tool_text_id
{
    press_enter_prompt
};

// Auto-generate tool common localized text arrays.
#define LOCALIZED_TEXT(languageID)\
    static const nchar* const in_tool_localized_##languageID##_text[] =

#include "hl_in_tool_common_text.h"
#undef LOCALIZED_TEXT

const nchar* const* const in_tool_localized_text[] =
{
// Auto-generate this array.
#define HL_LANGUAGE_AUTOGEN(languageID) in_tool_localized_##languageID##_text,
#include "hedgelib/hl_languages_autogen.h"
};

static const nchar* in_tool_get_text(language lang, in_tool_text_id id) noexcept
{
    return in_tool_localized_text[static_cast<int>(lang)]
        [static_cast<int>(id)];
}

namespace console
{
nstring read_line(std::FILE* stream)
{
    nstring str;
    int c;

    while ((c = nfgetc(stream)) != EOF && c != '\n')
    {
        str += static_cast<nchar>(c);
    }

    return str;
}

void write(nchar ch, std::FILE* stream)
{
    const int r = nfputc(ch, stream);
    if (r == EOF)
    {
        throw unknown_exception();
    }
}

void write(const nchar* str, std::FILE* stream)
{
    const int r = nfputs(str, stream);
    if (r == EOF)
    {
        throw unknown_exception();
    }
}

void write_line(const nchar* str, std::FILE* stream)
{
    write(str, stream);
    write(HL_NTEXT('\n'), stream);
}

void write_line(const nchar* str)
{
    const int r = nputs(str);
    if (r == EOF)
    {
        throw unknown_exception();
    }
}

#ifdef _WIN32
bool should_pause()
{
    HWND consoleWindow = GetConsoleWindow();
    DWORD processID;

    // If console window handle is null, just return false.
    if (!consoleWindow) return false;

    // Get the process ID of the console window.
    GetWindowThreadProcessId(consoleWindow, &processID);

    // If the current process ID matches the console window's
    // process ID, we should prompt the user to press enter.
    return (GetCurrentProcessId() == processID);
}
#endif

void pause(language lang)
{
    // Print press enter prompt.
    write_line(in_tool_get_text(lang, in_tool_text_id::press_enter_prompt));
    
    // Wait for user to press enter.
    ngetchar();
}
} // console
} // hl
