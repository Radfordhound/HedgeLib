#include "hr_in_instance.h"
#include "hr_in_adapter.h"
#include "hedgerender/gfx/hr_instance.h"
#include "hedgerender/gfx/hr_surface.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace hr
{
namespace gfx
{
const char* const in_vulkan_validation_layers[] =
{
    "VK_LAYER_KHRONOS_validation"
};

const uint32_t in_vulkan_validation_layer_count =
hl::count_of(in_vulkan_validation_layers);

namespace internal
{
std::vector<const char*> in_vulkan_additional_instance_extensions;
} // internal

std::vector<adapter> instance::adapters(surface& surface)
{
    return in_vulkan_get_adapters(*this, surface.handle());
}

adapter instance::get_default_adapter(surface& surface)
{
    return in_vulkan_get_default_adapter(*this, surface.handle());
}

void instance::destroy() noexcept
{
    vkDestroyInstance(m_vkInstance, nullptr);
}

instance& instance::operator=(instance&& other) noexcept
{
    if (&other != this)
    {
        destroy();

        m_vkInstance = other.m_vkInstance;
        m_isDebug = other.m_isDebug;

        other.m_vkInstance = VK_NULL_HANDLE;
    }

    return *this;
}

static bool in_vulkan_supports_layer(uint32_t vkLayerPropCount,
    const VkLayerProperties* vkLayerProps, const char* name)
{
    for (uint32_t i = 0; i < vkLayerPropCount; ++i)
    {
        if (std::strcmp(name, vkLayerProps[i].layerName) == 0)
        {
            return true;
        }
    }

    return false;
}

static bool in_vulkan_supports_validation_layers()
{
    // Get instance layer property count.
    uint32_t vkLayerPropCount;
    if (vkEnumerateInstanceLayerProperties(&vkLayerPropCount, nullptr) != VK_SUCCESS)
    {
        return false;
    }

    // Get instance layer properties.
    std::unique_ptr<VkLayerProperties[]> vkLayerProps(new VkLayerProperties[vkLayerPropCount]);
    if (vkEnumerateInstanceLayerProperties(&vkLayerPropCount, vkLayerProps.get()) != VK_SUCCESS)
    {
        return false;
    }

    // Return whether we have support for all of our validation layers.
    for (std::size_t i = 0; i < in_vulkan_validation_layer_count; ++i)
    {
        if (!in_vulkan_supports_layer(vkLayerPropCount,
            vkLayerProps.get(), in_vulkan_validation_layers[i]))
        {
            return false;
        }
    }

    return true;
}

instance::instance(const char* appName,
    version_t appVersion, bool useDebugLayers) :
    m_isDebug(useDebugLayers)
{
    using namespace internal;

    // Generate Vulkan application info.
    const VkApplicationInfo vkAppInfo =
    {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,                             // sType
        nullptr,                                                        // pNext
        appName,                                                        // pApplicationName
        appVersion,                                                     // applicationVersion
        "HedgeRender",                                                  // pEngineName
        in_vulkan_engine_version,                                       // engineVersion
        in_vulkan_api_version                                           // apiVersion
    };

    // Get all normal required Vulkan instance extensions.
    hl::stack_or_heap_buffer<const char*, 8> requiredExts(
        1 + in_vulkan_additional_instance_extensions.size());

    uint32_t requiredExtCount = 0;

    if (useDebugLayers)
    {
        requiredExts[requiredExtCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }

    // Add additional requested Vulkan instance extensions.
    std::memcpy(&requiredExts[requiredExtCount], in_vulkan_additional_instance_extensions.data(),
        sizeof(const char*) * in_vulkan_additional_instance_extensions.size());

    requiredExtCount += in_vulkan_additional_instance_extensions.size();

    // Generate Vulkan instance create info.
    VkInstanceCreateInfo vkInstCreateInfo =
    {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,                         // sType
        nullptr,                                                        // pNext
        0,                                                              // flags
        &vkAppInfo,                                                     // pApplicationInfo
        0,                                                              // enabledLayerCount
        nullptr,                                                        // ppEnabledLayerNames
        requiredExtCount,                                               // enabledExtensionCount
        requiredExts                                                    // ppEnabledExtensionNames
    };

    // Enable validation layers if requested and supported by the system.
    if (useDebugLayers)
    {
        if (in_vulkan_supports_validation_layers())
        {
            vkInstCreateInfo.enabledLayerCount = in_vulkan_validation_layer_count;
            vkInstCreateInfo.ppEnabledLayerNames = in_vulkan_validation_layers;
        }
        else
        {
            m_isDebug = false;
            HR_LOG_WARN("Vulkan validation layers were requested but are not supported");
        }
    }

    // Create Vulkan instance.
    if (vkCreateInstance(&vkInstCreateInfo, nullptr, &m_vkInstance) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create Vulkan instance");
    }
}

instance::instance(instance&& other) noexcept :
    m_vkInstance(other.m_vkInstance),
    m_isDebug(other.m_isDebug)
{
    other.m_vkInstance = VK_NULL_HANDLE;
}
} // gfx
} // hr
