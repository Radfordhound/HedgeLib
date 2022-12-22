#ifndef HR_GFX_INTERNAL_H_INCLUDED
#define HR_GFX_INTERNAL_H_INCLUDED

#include <hedgerender/base/hr_base_internal.h>

#define VULKAN_HPP_TYPESAFE_CONVERSION 1
#include <vulkan/vulkan.hpp>

/*
   Shared library macros
   (Adapted from https://gcc.gnu.org/wiki/Visibility)
*/
#ifndef HR_GFX_API
#ifdef HR_GFX_IS_DLL
#ifdef _WIN32
#ifdef HR_GFX_IS_BUILDING_DLL
#define HR_GFX_API __declspec(dllexport)                        /* We're building a Windows DLL; export the given symbol. */
#else
#define HR_GFX_API __declspec(dllimport)                        /* We're using a pre-built Windows DLL; import the given symbol. */
#endif
#elif defined(__GNUC__) && __GNUC__ >= 4 
#define HR_GFX_API __attribute__ ((visibility ("default")))     /* We're using GCC (or compatible); use __attribute__ */
#else
#define HR_GFX_API                                              /* We don't know the target; just assume it requires no keywords. */
#endif
#else
#define HR_GFX_API                                              /* This is a static library; no keyword(s) are needed. */
#endif
#endif

#endif
