#ifndef HR_LOG_H_INCLUDED
#define HR_LOG_H_INCLUDED
#include "hr_base_internal.h"
#include <hedgelib/hl_text.h>

namespace hr
{
enum class log_level
{
    info,
    warning,
    error
};

using log_callback_func = void (*)(log_level level, const hl::nchar* fmt, ...);

HR_BASE_API extern log_callback_func log_callback_ptr;
#define HR_SET_LOG_CALLBACK(callback) ::hr::log_callback_ptr = (callback)

#define HR_NLOGF(level, ...)    ::hr::log_callback_ptr(level, __VA_ARGS__)
#define HR_NLOGF_INFO(...)      HR_NLOGF(::hr::log_level::info, __VA_ARGS__)
#define HR_NLOGF_WARN(...)      HR_NLOGF(::hr::log_level::warning, __VA_ARGS__)
#define HR_NLOGF_ERROR(...)     HR_NLOGF(::hr::log_level::error, __VA_ARGS__)

#define HR_NLOG(level, text)    ::hr::log_callback_ptr(level, text)
#define HR_NLOG_INFO(text)      HR_NLOG(::hr::log_level::info, text)
#define HR_NLOG_WARN(text)      HR_NLOG(::hr::log_level::warning, text)
#define HR_NLOG_ERROR(text)     HR_NLOG(::hr::log_level::error, text)

#define HR_LOGF(level, fmt, ...)    ::hr::log_callback_ptr(level, HL_NTEXT(fmt), __VA_ARGS__)
#define HR_LOGF_INFO(fmt, ...)      HR_LOGF(::hr::log_level::info, fmt, __VA_ARGS__)
#define HR_LOGF_WARN(fmt, ...)      HR_LOGF(::hr::log_level::warning, fmt, __VA_ARGS__)
#define HR_LOGF_ERROR(fmt, ...)     HR_LOGF(::hr::log_level::error, fmt, __VA_ARGS__)

#define HR_LOG(level, text)     HR_NLOG(level, HL_NTEXT(text))
#define HR_LOG_INFO(text)       HR_LOG(::hr::log_level::info, text)
#define HR_LOG_WARN(text)       HR_LOG(::hr::log_level::warning, text)
#define HR_LOG_ERROR(text)      HR_LOG(::hr::log_level::error, text)
} // hr
#endif
