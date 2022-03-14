#ifndef HR_GFX_INTERNAL_H_INCLUDED
#define HR_GFX_INTERNAL_H_INCLUDED
#include <hedgerender/base/hr_base_internal.h>
#include <vulkan/vulkan.h>

/*
   Shared library macros
   (Adapted from https://gcc.gnu.org/wiki/Visibility)
*/
#ifndef HR_GFX_API
#ifdef HR_GFX_IS_DLL
// HedgeRender_GFX is a DLL.
#ifdef _WIN32
// We're targetting Windows; use __declspec
#ifdef HR_GFX_IS_BUILDING_DLL
// We're building a DLL; export the given symbol.
#define HR_GFX_API __declspec(dllexport)
#else
// We're using a pre-built DLL; import the given symbol.
#define HR_GFX_API __declspec(dllimport)
#endif
#elif defined(__GNUC__) && __GNUC__ >= 4
// We're not targetting Windows and we're using gcc; use __attribute__ 
#define HR_GFX_API __attribute__ ((visibility ("default")))
#else
// We don't know the target platform/compiler; assume it doesn't require any keywords.
#define HR_GFX_API
#endif
#else
// HedgeRender_GFX is a static library; no keyword(s) are needed. 
#define HR_GFX_API
#endif
#endif
#endif
