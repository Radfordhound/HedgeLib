/*
   hr_win32_main.h

   Provides a WinMain wrapper so the nmain entry point used by other
   HedgeTools can be used even when compiling a Win32 program.

   NOTE: This header is only meant to be included ONCE PER APPLICATION!
*/
#ifndef HR_WIN32_MAIN_H_INCLUDED
#define HR_WIN32_MAIN_H_INCLUDED
#else
#error "This header is only meant to be included once!"
#endif

#ifdef _WIN32
#include "hr_internal.h"
#include <windows.h>

#ifdef HL_IN_WIN32_UNICODE
#define HR_IN_NWINMAIN  wWinMain
#define HR_IN_NMAIN     wmain
#define HR_IN_NARGV     __wargv
#else
#define HR_IN_NWINMAIN  WinMain
#define HR_IN_NMAIN     main
#define HR_IN_NARGV     __argv
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
   Forward-declaration for HrINWin32NCmdShow.
   (Defined in hr_in_window_win32.h)
*/
#ifdef HR_IS_DLL
__declspec(dllimport)
#endif
extern int HrINWin32NCmdShow;

/* Forward-declaration for nmain. */
int HR_IN_NMAIN(int argc, HlNChar* argv[]);

/* WinMain definition. */
int WINAPI HR_IN_NWINMAIN(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPTSTR pCmdLine, int nCmdShow)
{
    /* Set HrINWin32NCmdShow. */
    HrINWin32NCmdShow = nCmdShow;

    /* Call nmain. */
    return HR_IN_NMAIN(__argc, HR_IN_NARGV);
}

#ifdef __cplusplus
}
#endif
#endif
