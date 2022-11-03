#include "hedgerender/base/hr_log.h"
#include <hedgelib/hl_tool_helpers.h>

namespace hr
{
static const hl::nchar in_default_log_prefix_info[] = HL_NTEXT("[INFO]: ");
static const hl::nchar in_default_log_prefix_warning[] = HL_NTEXT("[WARN]: ");
static const hl::nchar in_default_log_prefix_error[] = HL_NTEXT("[ERROR]: ");

/** @brief Indexable via a hr::log_level enum value. */
static const hl::nchar* const in_default_log_prefixes[] =
{
    in_default_log_prefix_info,
    in_default_log_prefix_warning,
    in_default_log_prefix_error
};

/** @brief Indexable via a hr::log_level enum value. */
static const std::size_t in_default_log_prefix_lens[] =
{
    (hl::count_of(in_default_log_prefix_info) - 1),
    (hl::count_of(in_default_log_prefix_warning) - 1),
    (hl::count_of(in_default_log_prefix_error) - 1)
};

static const hl::nchar in_default_log_suffix[] = HL_NTEXT("\n");

/** @brief Indexable via a hr::log_level enum value. */
static const hl::nchar* const in_default_log_suffixes[] =
{
    in_default_log_suffix,
    in_default_log_suffix,
    in_default_log_suffix
};

/** @brief Indexable via a hr::log_level enum value. */
static const std::size_t in_default_log_suffix_lens[] =
{
    (hl::count_of(in_default_log_suffix) - 1),
    (hl::count_of(in_default_log_suffix) - 1),
    (hl::count_of(in_default_log_suffix) - 1)
};

static std::size_t in_write_log_str(hl::nchar* buf, std::size_t bufCount,
    const hl::nchar* fmt, std::va_list argList)
{
    const int strLen = hl::nvsnprintf(buf, bufCount, fmt, argList);
    if (strLen < 0)
    {
        throw std::runtime_error("Failed to write log string");
    }

    return static_cast<std::size_t>(strLen);
}

static void in_default_log_callback(log_level level, const hl::nchar* fmt, ...)
{
    // Get variadic arguments.
    std::va_list argList;
    va_start(argList, fmt);

    // Compute lengths.
    const std::size_t prefixLen = in_default_log_prefix_lens[static_cast<int>(level)];
    const std::size_t strLen = in_write_log_str(nullptr, 0, fmt, argList);
    const std::size_t suffixLen = in_default_log_suffix_lens[static_cast<int>(level)];
    const std::size_t bufLen = (prefixLen + strLen + suffixLen + 1); // +1 for null terminator.

    // Utilize stack buffer of 512 nchars, or allocate heap buffer if 512 nchars is not sufficient.
    hl::stack_or_heap_buffer<hl::nchar, 512> buf(bufLen);
    hl::nchar* ptr = buf;

    // Append prefix.
    std::memcpy(ptr, in_default_log_prefixes[static_cast<int>(level)],
        prefixLen * sizeof(hl::nchar));

    ptr += prefixLen;

    // Append log string.
    ptr += in_write_log_str(ptr, bufLen, fmt, argList);
    
    // Append suffix.
    std::memcpy(ptr, in_default_log_suffixes[static_cast<int>(level)],
        suffixLen * sizeof(hl::nchar));

    ptr += suffixLen;

    // Append null terminator.
    *ptr = HL_NTEXT('\0');

    // Print string.
    if (hl::nputs(buf) < 0)
    {
        throw std::runtime_error("Failed to print log string to output");
    }
}

log_callback_func log_callback_ptr = &in_default_log_callback;
} // hr
