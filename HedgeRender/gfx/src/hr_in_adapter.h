#ifndef HR_IN_ADAPTER_H_INCLUDED
#define HR_IN_ADAPTER_H_INCLUDED
#include "hedgerender/gfx/hr_adapter.h"
#include <vector>

namespace hr
{
namespace gfx
{
extern const char* const in_vulkan_device_required_extensions[];
extern const uint32_t in_vulkan_device_required_extension_count;

adapter in_vulkan_get_default_adapter(
    instance& instance, VkSurfaceKHR vkSurface);

std::vector<adapter> in_vulkan_get_adapters(
    instance& instance, VkSurfaceKHR vkSurface);

VkSurfaceCapabilitiesKHR in_vulkan_get_surface_capabilities(
    VkSurfaceKHR vkSurface, VkPhysicalDevice vkPhyDev);

VkSurfaceFormatKHR in_vulkan_get_best_surface_format(
    VkSurfaceKHR vkSurface, VkPhysicalDevice vkPhyDev);

VkPresentModeKHR in_vulkan_get_best_present_mode(
    VkSurfaceKHR vkSurface, VkPhysicalDevice vkPhyDev, bool vsync);
} // gfx
} // hr
#endif
