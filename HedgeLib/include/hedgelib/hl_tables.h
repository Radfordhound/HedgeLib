#ifndef HL_TABLES_H_INCLUDED
#define HL_TABLES_H_INCLUDED
#include "hl_text.h"
#include <vector>
#include <utility>

namespace hl
{
using off_table = std::vector<std::size_t>;

struct str_table_entry
{
    /** @brief The string being referenced. */
    std::string str;
    /** @brief The (absolute) position of the string offset within the file. */
    std::size_t offPos;

    str_table_entry() = default;
    
    str_table_entry(std::string str, std::size_t offPos) noexcept :
        str(std::move(str)),
        offPos(offPos) {}

#ifdef HL_IN_WIN32_UNICODE
    str_table_entry(const nchar* str, std::size_t offPos) :
        str(std::move(text::conv<text::native_to_utf8>(str))),
        offPos(offPos) {}

    str_table_entry(const nstring& str, std::size_t offPos) :
        str(std::move(text::conv<text::native_to_utf8>(str))),
        offPos(offPos) {}
#endif
};

using str_table = std::vector<str_table_entry>;
} // hl
#endif
