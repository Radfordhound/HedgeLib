#ifndef HR_SURFACE_H_INCLUDED
#define HR_SURFACE_H_INCLUDED
#include "hr_gfx_internal.h"

namespace hr
{
namespace gfx
{
class instance;
class render_device;

class surface : public non_copyable
{
    friend render_device;

    VkInstance m_vkInstance;
    VkSurfaceKHR m_vkSurface;

public:
    inline VkSurfaceKHR handle() const noexcept
    {
        return m_vkSurface;
    }

    HR_GFX_API void destroy() noexcept;

    HR_GFX_API surface& operator=(surface&& other) noexcept;

    inline surface(VkInstance vkInstance, VkSurfaceKHR vkSurface) noexcept :
        m_vkInstance(vkInstance),
        m_vkSurface(vkSurface) {}

    HR_GFX_API surface(surface&& other) noexcept;

    inline ~surface()
    {
        destroy();
    }
};
} // gfx
} // hr
#endif
