#ifndef HR_BASE_INTERNAL_H_INCLUDED
#define HR_BASE_INTERNAL_H_INCLUDED
#include <cstdint>
#include <cstddef>
#include <utility>
#include <new>

namespace hr
{
/*
   Shared library macros
   (Adapted from https://gcc.gnu.org/wiki/Visibility)
*/
#ifndef HR_BASE_API
#ifdef HR_BASE_IS_DLL
// HedgeRender_Base is a DLL.
#ifdef _WIN32
// We're targetting Windows; use __declspec
#ifdef HR_BASE_IS_BUILDING_DLL
// We're building a DLL; export the given symbol.
#define HR_BASE_API __declspec(dllexport)
#else
// We're using a pre-built DLL; import the given symbol.
#define HR_BASE_API __declspec(dllimport)
#endif
#elif defined(__GNUC__) && __GNUC__ >= 4
// We're not targetting Windows and we're using gcc; use __attribute__ 
#define HR_BASE_API __attribute__ ((visibility ("default")))
#else
// We don't know the target platform/compiler; assume it doesn't require any keywords.
#define HR_BASE_API
#endif
#else
// HedgeRender_Base is a static library; no keyword(s) are needed. 
#define HR_BASE_API
#endif
#endif

/* Helper constants */
constexpr bool is_debug_build =
#ifndef NDEBUG
    true;
#else
    false;
#endif

/* Version numbers */
using version_t = std::uint32_t;

#define HR_MAKE_VERSION(major, minor, revision) \
    (((static_cast<::hr::version_t>(major)) << 22) |\
    ((static_cast<::hr::version_t>(minor)) << 12) |\
    (static_cast<::hr::version_t>(revision)))

/* Helper objects */
class non_copyable
{
public:
    non_copyable& operator=(const non_copyable& other) = delete;
    non_copyable& operator=(non_copyable&& other) noexcept = default;

    non_copyable() = default;
    non_copyable(const non_copyable& other) = delete;
    non_copyable(non_copyable&& other) noexcept = default;
};

class non_moveable
{
public:
    non_moveable& operator=(const non_moveable& other) = default;
    non_moveable& operator=(non_moveable&& other) noexcept = delete;

    non_moveable() = default;
    non_moveable(const non_moveable& other) = default;
    non_moveable(non_moveable&& other) noexcept = delete;
};
} // hr
#endif
