/*
    Pre-compiled header file for HedgeRender_GFX.

    This file is automatically included in every source file
    in the HedgeRender_GFX project thanks to CMake.
*/
#ifndef HR_IN_GFX_PCH_H_INCLUDED
#define HR_IN_GFX_PCH_H_INCLUDED
#include <hedgerender/base/hr_log.h>
#include <hedgerender/base/hr_array.h>
#include <cstring>
#include <cassert>

// Windows
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// VulkanMemoryAllocator (also includes Vulkan)
// HACK: Temporary work-around for arch linux, which currently
// has Vulkan 1.3 headers but only has Vulkan 1.2 libraries!
#define VMA_VULKAN_VERSION 1002000
#include <vk_mem_alloc.h>
#endif
