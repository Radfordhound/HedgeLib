#pragma once

// Define shared library macros
// Adapted from https://gcc.gnu.org/wiki/Visibility
#if defined _WIN32 || defined __CYGWIN__
#define HL_HELPER_DLL_IMPORT __declspec(dllimport)
#define HL_HELPER_DLL_EXPORT __declspec(dllexport)
#define HL_HELPER_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define HL_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
#define HL_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
#define HL_HELPER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define HL_HELPER_DLL_IMPORT
#define HL_HELPER_DLL_EXPORT
#define HL_HELPER_DLL_LOCAL
#endif
#endif

#ifdef HL_DLL // HedgeLib is a DLL
#ifdef HL_DLL_EXPORTS
#define HL_API HL_HELPER_DLL_EXPORT // For building HedgeLib DLLs
#else
#define HL_API HL_HELPER_DLL_IMPORT // For using pre-built HedgeLib DLLs
#endif
#define HL_LOCAL HL_HELPER_DLL_LOCAL
#else // HedgeLib is a static library
#define HL_API
#define HL_LOCAL
#endif
