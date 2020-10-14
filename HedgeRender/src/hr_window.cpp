#include "hedgerender/hr_window.h"

#ifdef _WIN32
#include "hr_in_window_win32.h"
#else
#error "Unknown or unsupported target platform."
#endif
