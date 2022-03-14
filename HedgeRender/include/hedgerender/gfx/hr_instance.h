#ifndef HR_INSTANCE_H_INCLUDED
#define HR_INSTANCE_H_INCLUDED
#include "hr_adapter.h"
#include <string>
#include <vector>

namespace hr
{
namespace gfx
{
class surface;
class render_device;

namespace internal
{
HR_GFX_API extern std::vector<const char*> in_vulkan_additional_instance_extensions;
} // internal

class instance : public non_copyable
{
    friend render_device;

    VkInstance m_vkInstance = nullptr;
    bool m_isDebug;

public:
    inline VkInstance handle() const noexcept
    {
        return m_vkInstance;
    }

    HR_GFX_API std::vector<adapter> adapters(surface& surface);
    HR_GFX_API adapter get_default_adapter(surface& surface);

    HR_GFX_API void destroy() noexcept;

    HR_GFX_API instance& operator=(instance&& other) noexcept;

    instance(VkInstance vkInstance = nullptr, bool isDebug = is_debug_build) noexcept :
        m_vkInstance(vkInstance),
        m_isDebug(isDebug) {}

    HR_GFX_API instance(const char* appName, version_t appVersion,
        bool useDebugLayers = is_debug_build);

    inline instance(const std::string& appName, version_t appVersion,
        bool useDebugLayers = is_debug_build) :
        instance(appName.c_str(), appVersion, useDebugLayers) {}

    HR_GFX_API instance(instance&& other) noexcept;

    inline ~instance()
    {
        destroy();
    }
};
} // gfx
} // hr
#endif
