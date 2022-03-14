#ifndef HR_EXT_GLFW_H_INCLUDED
#define HR_EXT_GLFW_H_INCLUDED
#include <hedgerender/base/hr_base_internal.h>

#ifdef HR_EXT_GLFW_IMPLEMENTATION
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <hedgerender/gfx/hr_instance.h>
#endif

struct GLFWwindow;

namespace hr
{
namespace ext
{
class glfw_raii_window : public non_copyable
{
    GLFWwindow* m_handle = nullptr;

public:
    inline const GLFWwindow* get() const noexcept
    {
        return m_handle;
    }

    inline GLFWwindow* get() noexcept
    {
        return m_handle;
    }

    glfw_raii_window& operator=(glfw_raii_window&& other) noexcept;

    inline const GLFWwindow& operator*() const noexcept
    {
        return *m_handle;
    }

    inline GLFWwindow& operator*() noexcept
    {
        return *m_handle;
    }

    inline operator const GLFWwindow*() const noexcept
    {
        return m_handle;
    }

    inline operator GLFWwindow*() noexcept
    {
        return m_handle;
    }

    inline glfw_raii_window() noexcept = default;

    inline glfw_raii_window(GLFWwindow* window) noexcept :
        m_handle(window) {}

    inline glfw_raii_window(glfw_raii_window&& other) noexcept :
        m_handle(other.m_handle)
    {
        other.m_handle = nullptr;
    }

    ~glfw_raii_window();
};

void glfw_init();

void glfw_shutdown();

#ifdef HR_EXT_GLFW_IMPLEMENTATION
#undef HR_EXT_GLFW_IMPLEMENTATION
glfw_raii_window& glfw_raii_window::operator=(glfw_raii_window&& other) noexcept
{
    if (&other != this)
    {
        glfwDestroyWindow(m_handle);
        m_handle = other.m_handle;
        other.m_handle = nullptr;
    }

    return *this;
}

glfw_raii_window::~glfw_raii_window()
{
    glfwDestroyWindow(m_handle);
}

namespace internal
{
static void in_glfw_error_callback(int error, const char* msg)
{
    HR_LOGF_ERROR("GLFW error code %d: %hs", msg);
}
} // internal

void glfw_init()
{
    // Set GLFW error callback.
    glfwSetErrorCallback(&internal::in_glfw_error_callback);

    // Initialize GLFW Vulkan loader if necessary.
#if GLFW_VERSION_MAJOR > 3 || GLFW_VERSION_MINOR >= 4
    glfwInitVulkanLoader(&vkGetInstanceProcAddr);
#endif

    // Initialize GLFW.
    if (!glfwInit())
    {
        throw std::runtime_error("Could not initialize GLFW");
    }

    // Get required Vulkan instance extensions.
    uint32_t vkReqInstExtsCount;
    const char** const vkReqInstExts = glfwGetRequiredInstanceExtensions(&vkReqInstExtsCount);

    if (!vkReqInstExts)
    {
        glfwTerminate();
        throw std::runtime_error("Could not get list of required "
            "Vulkan instance extensions from GLFW");
    }

    // Tell HedgeRender_GFX that we additionally need these instance extensions.
    try
    {
        gfx::internal::in_vulkan_additional_instance_extensions.insert(
            gfx::internal::in_vulkan_additional_instance_extensions.end(),
            vkReqInstExts, vkReqInstExts + vkReqInstExtsCount);
    }
    catch (...)
    {
        glfwTerminate();
        throw;
    }
}

void glfw_shutdown()
{
    glfwTerminate();
}
#endif
} // ext
} // hr
#endif
