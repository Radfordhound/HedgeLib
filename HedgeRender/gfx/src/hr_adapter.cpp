#include "hr_in_adapter.h"
#include "hedgerender/gfx/hr_instance.h"

namespace hr
{
namespace gfx
{
const char* const in_vulkan_device_required_extensions[] =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME
};

const uint32_t in_vulkan_device_required_extension_count =
    HL_COUNT_OF(in_vulkan_device_required_extensions);

static bool in_vulkan_has_extension(
    const fixed_array<VkExtensionProperties, uint32_t>& vkExts,
    const char* name)
{
    for (auto& vkExt : vkExts)
    {
        if (std::strcmp(name, vkExt.extensionName) == 0)
        {
            return true;
        }
    }

    return false;
}

static fixed_array<VkExtensionProperties, uint32_t>
    in_vulkan_get_extensions(VkPhysicalDevice vkPhyDev)
{
    // Get extension count.
    uint32_t vkExtCount;
    if (vkEnumerateDeviceExtensionProperties(vkPhyDev, nullptr,
        &vkExtCount, nullptr) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not enumerate Vulkan physical device extensions");
    }

    // Get extension properties.
    fixed_array<VkExtensionProperties, uint32_t> vkExts(vkExtCount, no_default_construct);
    if (vkEnumerateDeviceExtensionProperties(vkPhyDev, nullptr,
        &vkExtCount, vkExts.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not enumerate Vulkan physical device extensions");
    }

    return vkExts;
}

static bool in_vulkan_has_required_extensions(VkPhysicalDevice vkPhyDev)
{
    // Get extensions.
    auto vkExts = in_vulkan_get_extensions(vkPhyDev);

    // Ensure device has all of the required extensions.
    for (uint32_t i = 0; i < in_vulkan_device_required_extension_count; ++i)
    {
        if (!in_vulkan_has_extension(vkExts,
            in_vulkan_device_required_extensions[i]))
        {
            return false;
        }
    }

    return true;
}

static fixed_array<VkQueueFamilyProperties, uint32_t>
    in_vulkan_get_queue_family_properties(VkPhysicalDevice vkPhyDev)
{
    // Get supported queue family count.
    uint32_t vkQueueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhyDev,
        &vkQueueFamilyCount, nullptr);

    // Get supported queue families.
    fixed_array<VkQueueFamilyProperties, uint32_t> vkQueueFamilies(
        vkQueueFamilyCount, no_default_construct);

    vkGetPhysicalDeviceQueueFamilyProperties(vkPhyDev,
        &vkQueueFamilyCount, vkQueueFamilies.data());

    return vkQueueFamilies;
}

static bool in_vulkan_supports_present(VkPhysicalDevice vkPhyDev,
    VkSurfaceKHR vkSurface, uint32_t queueFamilyIndex)
{
    VkBool32 supportsPresent;
    if (vkGetPhysicalDeviceSurfaceSupportKHR(vkPhyDev, queueFamilyIndex,
        vkSurface, &supportsPresent) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not check the Vulkan physical "
            "device for surface present support");
    }

    return supportsPresent;
}

static bool in_vulkan_get_required_queue_families(VkPhysicalDevice vkPhyDev,
    VkSurfaceKHR vkSurface, internal::in_queue_families& vkQueueFamilyInfo)
{
    using namespace internal;

    // Get supported queue families.
    auto vkQueueFamilies = in_vulkan_get_queue_family_properties(vkPhyDev);

    // Get the indices of the queue families we require.
    bool hasGraphicsFamily = false, hasPresentFamily = false, hasTransferFamily = false;
    for (uint32_t i = 0; i < vkQueueFamilies.size(); ++i)
    {
        // Check this queue family for graphics support.
        if (!hasGraphicsFamily && vkQueueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            vkQueueFamilyInfo.set_family(HR_IN_QUEUE_TYPE_GRAPHICS, i, true);

            if (!hasPresentFamily && in_vulkan_supports_present(vkPhyDev, vkSurface, i))
            {
                vkQueueFamilyInfo.set_family(HR_IN_QUEUE_TYPE_PRESENT, i, false);

                // Prefer to NOT look for a unique present family.
                hasPresentFamily = true;
            }

            if (!hasTransferFamily)
            {
                vkQueueFamilyInfo.set_family(HR_IN_QUEUE_TYPE_TRANSFER, i, false);
                // Prefer to look for a unique transfer family.
            }

            hasGraphicsFamily = true;
        }

        // Check this queue family for present support.
        else if (!hasPresentFamily && in_vulkan_supports_present(vkPhyDev, vkSurface, i))
        {
            vkQueueFamilyInfo.set_family(HR_IN_QUEUE_TYPE_PRESENT, i, true);

            if (!hasTransferFamily && vkQueueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                vkQueueFamilyInfo.set_family(HR_IN_QUEUE_TYPE_TRANSFER, i, false);
                // Prefer to look for a unique transfer family.
            }

            hasPresentFamily = true;
        }

        // Check this queue family for copy support.
        else if (!hasTransferFamily && vkQueueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            vkQueueFamilyInfo.set_family(HR_IN_QUEUE_TYPE_TRANSFER, i, true);
            hasTransferFamily = true;
        }

        // If we've already found all of the required families, break out of the loop.
        if (hasGraphicsFamily && hasPresentFamily && hasTransferFamily)
        {
            break;
        }
    }

    // NOTE: We DON'T require hasTransferFamily to be true, as we can always just
    // fallback to the graphics family. We can't do that with presentation.
    return hasGraphicsFamily && hasPresentFamily;
}

static fixed_array<VkPhysicalDevice, uint32_t>
    in_vulkan_get_physical_devices(VkInstance vkInstance)
{
    // Get physical device count.
    uint32_t vkPhyDevCount;
    if (vkEnumeratePhysicalDevices(vkInstance,
        &vkPhyDevCount, nullptr) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not enumerate Vulkan physical devices");
    }

    // Get physical devices.
    fixed_array<VkPhysicalDevice, uint32_t> vkPhyDevs(vkPhyDevCount, no_default_construct);
    if (vkEnumeratePhysicalDevices(vkInstance,
        &vkPhyDevCount, vkPhyDevs.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not enumerate Vulkan physical devices");
    }

    return vkPhyDevs;
}

adapter in_vulkan_get_default_adapter(
    instance& instance, VkSurfaceKHR vkSurface)
{
    using namespace internal;

    // Get Vulkan physical devices.
    auto vkPhyDevs = in_vulkan_get_physical_devices(instance.handle());

    // Pick "best" Vulkan physical device for our purposes.
    VkPhysicalDevice bestVkPhyDev = VK_NULL_HANDLE;
    in_queue_families bestVkQueueFamilies;
    int bestScore = -1;

    for (auto vkPhyDev : vkPhyDevs)
    {
        // Get Vulkan physical device properties.
        VkPhysicalDeviceProperties vkPhyDevProps;
        vkGetPhysicalDeviceProperties(vkPhyDev, &vkPhyDevProps);

        // Ensure we support all required Vulkan extensions.
        if (!in_vulkan_has_required_extensions(vkPhyDev))
            continue;

        // Get all required Vulkan queue families.
        in_queue_families vkQueueFamilies;
        if (!in_vulkan_get_required_queue_families(vkPhyDev,
            vkSurface, vkQueueFamilies))
        {
            continue;
        }

        // Prefer discrete GPUs over other device types.
        int curScore = 0;
        if (vkPhyDevProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            curScore += 100;
        }

        // TODO: Check other features of the physical device to determine which device is best.

        // Replace the current "best device" data if this device is better.
        if (curScore > bestScore)
        {
            bestVkPhyDev = vkPhyDev;
            bestVkQueueFamilies = vkQueueFamilies;
            bestScore = curScore;
        }
    }

    // Ensure we found at least one physical device which meets our requirements.
    if (!bestVkPhyDev)
    {
        throw std::runtime_error("Could not find a GPU with Vulkan support");
    }

    return adapter(instance, bestVkPhyDev, bestVkQueueFamilies);
}

std::vector<adapter> in_vulkan_get_adapters(
    instance& instance, VkSurfaceKHR vkSurface)
{
    using namespace internal;

    // Get Vulkan physical devices.
    auto vkPhyDevs = in_vulkan_get_physical_devices(instance.handle());

    // Generate adapters vector.
    std::vector<adapter> adapters;
    adapters.reserve(vkPhyDevs.size());

    for (auto vkPhyDev : vkPhyDevs)
    {
        // Ensure we support all required Vulkan extensions.
        if (!in_vulkan_has_required_extensions(vkPhyDev))
            continue;

        // Get all required Vulkan queue families and add the adapter.
        in_queue_families vkQueueFamilies;
        if (in_vulkan_get_required_queue_families(vkPhyDev,
            vkSurface, vkQueueFamilies))
        {
            adapters.emplace_back(instance, vkPhyDev, vkQueueFamilies);
        }
    }

    return adapters;
}

VkSurfaceCapabilitiesKHR in_vulkan_get_surface_capabilities(
    VkSurfaceKHR vkSurface, VkPhysicalDevice vkPhyDev)
{
    VkSurfaceCapabilitiesKHR vkSurfaceCapabilities;
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhyDev,
        vkSurface, &vkSurfaceCapabilities) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not get capabilities of Vulkan surface");
    }

    return vkSurfaceCapabilities;
}

VkSurfaceFormatKHR in_vulkan_get_best_surface_format(
    VkSurfaceKHR vkSurface, VkPhysicalDevice vkPhyDev)
{
    // Get the amount of supported surface formats.
    uint32_t vkSurfaceFormatCount;
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhyDev, vkSurface,
        &vkSurfaceFormatCount, nullptr) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not get supported Vulkan surface formats");
    }

    // Get the supported surface formats.
    std::unique_ptr<VkSurfaceFormatKHR[]> vkSurfaceFormats(
        new VkSurfaceFormatKHR[vkSurfaceFormatCount]);

    if (vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhyDev, vkSurface,
        &vkSurfaceFormatCount, vkSurfaceFormats.get()) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not get supported Vulkan surface formats");
    }

    // Pick the best surface format available (prefer non-linear
    // B8G8R8A8_SRGB, but fallback to whatever we can support).
    for (uint32_t i = 0; i < vkSurfaceFormatCount; ++i)
    {
        if (vkSurfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            vkSurfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return vkSurfaceFormats[i];
        }
    }

    return vkSurfaceFormats[0];
}

VkPresentModeKHR in_vulkan_get_best_present_mode(
    VkSurfaceKHR vkSurface, VkPhysicalDevice vkPhyDev, bool vsync)
{
    // Get the amount of supported present modes.
    uint32_t vkPresentModeCount;
    if (vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhyDev, vkSurface,
        &vkPresentModeCount, nullptr) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not get supported Vulkan present modes");
    }

    // Get the supported present modes.
    std::unique_ptr<VkPresentModeKHR[]> vkPresentModes(
        new VkPresentModeKHR[vkPresentModeCount]);

    if (vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhyDev, vkSurface,
        &vkPresentModeCount, vkPresentModes.get()) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not get supported Vulkan present modes");
    }

    // Pick the best present mode available.
    // TODO: Prefer FIFO on mobile as MAILBOX is very energy-hungry!
    const VkPresentModeKHR vkBestPresentMode = (vsync) ?
        VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;

    for (uint32_t i = 0; i < vkPresentModeCount; ++i)
    {
        if (vkPresentModes[i] == vkBestPresentMode)
        {
            return vkBestPresentMode;
        }
    }

    // Fallback to FIFO, as the Vulkan spec states that
    // this is the only mode required to be supported.
    return VK_PRESENT_MODE_FIFO_KHR;
}
} // gfx
} // hr
