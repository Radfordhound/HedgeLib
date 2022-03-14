#ifndef HR_IN_SURFACE_H_INCLUDED
#define HR_IN_SURFACE_H_INCLUDED
namespace hr
{
namespace gfx
{
uint32_t in_vulkan_clamp_image_count(
    const VkSurfaceCapabilitiesKHR& vkSurfaceCapabilities,
    uint32_t imageCount);

VkExtent2D in_vulkan_clamp_surface_extent(
    const VkSurfaceCapabilitiesKHR& vkSurfaceCapabilities,
    VkExtent2D vkExtent);
} // gfx
} // hr
#endif
