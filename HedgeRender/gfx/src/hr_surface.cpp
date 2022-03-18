#include "hr_in_surface.h"
#include "hedgerender/gfx/hr_instance.h"
#include "hedgerender/gfx/hr_surface.h"

namespace hr
{
namespace gfx
{
/**
    @brief This special value indicates that there is no limit on the maximum
    number of images a Vulkan swapchain can create for the given surface.
*/
constexpr uint32_t in_vulkan_image_count_no_limit = 0U;

uint32_t in_vulkan_clamp_image_count(
    const VkSurfaceCapabilitiesKHR& vkSurfaceCapabilities,
    uint32_t imageCount)
{
    // Clamp image count to minimum amount allowed by the surface.
    imageCount = std::max<>(imageCount, vkSurfaceCapabilities.minImageCount);

    // Clamp image count to maximum amount allowed by the surface.
    if (vkSurfaceCapabilities.maxImageCount != in_vulkan_image_count_no_limit &&
        imageCount > vkSurfaceCapabilities.maxImageCount)
    {
        imageCount = vkSurfaceCapabilities.maxImageCount;
    }

    return imageCount;
}

VkExtent2D in_vulkan_clamp_surface_extent(
    const VkSurfaceCapabilitiesKHR& vkSurfaceCapabilities,
    VkExtent2D vkExtent)
{
    vkExtent.width = std::max<>(vkSurfaceCapabilities.minImageExtent.width,
        std::min<>(vkSurfaceCapabilities.maxImageExtent.width, vkExtent.width));

    vkExtent.height = std::max<>(vkSurfaceCapabilities.minImageExtent.height,
        std::min<>(vkSurfaceCapabilities.maxImageExtent.height, vkExtent.height));

    return vkExtent;
}

void surface::destroy() noexcept
{
    vkDestroySurfaceKHR(m_vkInstance, m_vkSurface, nullptr);
}

surface& surface::operator=(surface&& other) noexcept
{
    if (&other != this)
    {
        destroy();

        m_vkInstance = other.m_vkInstance;
        m_vkSurface = other.m_vkSurface;

        other.m_vkSurface = VK_NULL_HANDLE;
    }

    return *this;
}

surface::surface(surface&& other) noexcept :
    m_vkInstance(other.m_vkInstance),
    m_vkSurface(other.m_vkSurface)
{
    other.m_vkSurface = VK_NULL_HANDLE;
}
} // gfx
} // hr
