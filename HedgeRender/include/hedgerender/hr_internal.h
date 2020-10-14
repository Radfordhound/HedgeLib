#ifndef HR_INTERNAL_H_INCLUDED
#define HR_INTERNAL_H_INCLUDED
#include "hedgelib/hl_internal.h"

/*
   Shared library macros
   (Adapted from https://gcc.gnu.org/wiki/Visibility)
*/
#ifdef _WIN32
/* We're targetting Windows; use __declspec */
#define HR_IN_DLL_EXPORT __declspec(dllexport)
#define HR_IN_DLL_IMPORT __declspec(dllimport)
#elif defined(__GNUC__) && __GNUC__ >= 4
/* We're not targetting Windows and we're using gcc; use __attribute__ */
#define HR_IN_DLL_EXPORT __attribute__ ((visibility ("default")))
#define HR_IN_DLL_IMPORT __attribute__ ((visibility ("default")))
#else
/* We don't know the target platform/compiler; assume it doesn't require any keywords. */
#define HR_IN_DLL_EXPORT
#define HR_IN_DLL_IMPORT
#endif

#ifndef HR_API
#ifdef HR_IS_DLL
/* HedgeRender is a DLL. */
#ifdef HR_IS_BUILDING_DLL
/* We're building a DLL; export the given symbol. */
#define HR_API HR_IN_DLL_EXPORT
#else
/* We're using a pre-built DLL; import the given symbol. */
#define HR_API HR_IN_DLL_IMPORT
#endif
#else
/* HedgeRender is a static library; no keyword(s) are needed. */
#define HR_API
#endif
#endif

#ifndef HR_BACK_FUNC
#ifdef HR_BACKEND_IS_DLL
/* The HedgeRender backend is dynamically linked. */
#ifdef HR_IS_BUILDING_BACKEND_DLL
/* We're building a DLL; define the function normally, but make sure to export it! */
#define HR_BACK_FUNC(returnType, name) HR_IN_DLL_EXPORT returnType name
#else
/* We're using a pre-built DLL; define a pointer we can use to reference the imported function. */
#define HR_BACK_FUNC(returnType, name) extern returnType (*name)
#endif
#else
/* This HedgeRender backend is a static library; just define the function normally. */
#define HR_BACK_FUNC(returnType, name) returnType name
#endif
#endif
#endif
