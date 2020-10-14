#include "hedgeedit.h"
#include "hedgerender/hr_win32_main.h"
#include "../../hedgetools_helpers.h"

int nmain(int argc, HlNChar* argv[])
{
    HlResult result = hedgeEditRun();
    return (HL_OK(result)) ? EXIT_SUCCESS : EXIT_FAILURE;
}
