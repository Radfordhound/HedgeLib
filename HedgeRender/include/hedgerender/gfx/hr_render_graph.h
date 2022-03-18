#ifndef HR_RENDER_GRAPH_H_INCLUDED
#define HR_RENDER_GRAPH_H_INCLUDED
#include "hr_gfx_internal.h"
#include "hr_color.h"
#include <hedgelib/hl_internal.h>
#include <robin_hood.h>
#include <vector>
#include <functional>
#include <memory>

namespace hr
{
namespace gfx
{
class render_graph_builder;
class render_device;
class cmd_list;

namespace internal
{
class in_render_graph;
} // internal

class render_graph : public non_copyable
{
    friend render_graph_builder;

    internal::in_render_graph* m_renderGraph;

public:
    inline internal::in_render_graph* handle() const noexcept
    {
        return m_renderGraph;
    }

    HR_GFX_API void recreate_framebuffers(render_device& device);

    HR_GFX_API void destroy() noexcept;

    HR_GFX_API render_graph& operator=(render_graph&& other) noexcept;

    inline render_graph(internal::in_render_graph* renderGraph) noexcept :
        m_renderGraph(renderGraph) {}

    HR_GFX_API render_graph(render_graph&& other) noexcept;

    inline ~render_graph()
    {
        destroy();
    }
};

namespace internal
{
struct in_render_resource_info
{
    color clearColor = colors::black;

    in_render_resource_info(const color& clearColor) noexcept :
        clearColor(clearColor) {}
};

enum class in_attachment_type
{
    none = 0,
    vertex_shader_input = 1,
    pixel_shader_input = 2,
    color_output = 4
};

HL_ENUM_CLASS_DEF_BITWISE_OPS(in_attachment_type)

constexpr bool in_has_attachment_type(in_attachment_type types,
    in_attachment_type type) noexcept
{
    return ((types & type) != in_attachment_type::none);
}

constexpr bool in_is_input_attachment(in_attachment_type types) noexcept
{
    return in_has_attachment_type(types,
        in_attachment_type::vertex_shader_input |
        in_attachment_type::pixel_shader_input
    );
}

constexpr bool in_is_output_attachment(in_attachment_type types) noexcept
{
    return in_has_attachment_type(types,
        in_attachment_type::color_output
    );
}
} // internal

class render_subpass
{
public:
    virtual ~render_subpass() {}
    virtual void execute(cmd_list& cmdList) {}

    render_subpass& operator=(const render_subpass& other) = default;
    render_subpass& operator=(render_subpass&& other) noexcept = default;

    render_subpass() noexcept = default;
    render_subpass(const render_subpass& other) = default;
    render_subpass(render_subpass&& other) noexcept = default;
};

class render_pass
{
public:
    virtual ~render_pass() {}
    virtual void before_pass(cmd_list& cmdList) {}
    virtual void after_pass(cmd_list& cmdList) {}

    render_pass& operator=(const render_pass& other) = default;
    render_pass& operator=(render_pass&& other) noexcept = default;

    render_pass() noexcept = default;
    render_pass(const render_pass& other) = default;
    render_pass(render_pass&& other) noexcept = default;
};

class render_attachment_info
{
public:
    internal::in_attachment_type type = internal::in_attachment_type::none;
    VkPipelineColorBlendAttachmentState vkColorBlendState =
    {
        VK_FALSE,                                                       // blendEnable
        VK_BLEND_FACTOR_ZERO,                                           // srcColorBlendFactor
        VK_BLEND_FACTOR_ZERO,                                           // dstColorBlendFactor
        VK_BLEND_OP_ADD,                                                // colorBlendOp
        VK_BLEND_FACTOR_ZERO,                                           // srcAlphaBlendFactor
        VK_BLEND_FACTOR_ZERO,                                           // dstAlphaBlendFactor
        VK_BLEND_OP_ADD,                                                // alphaBlendOp
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |           // colorWriteMask
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };
};

using render_resource_id = std::vector<internal::in_render_resource_info>::size_type;
using render_attachment_info_map = robin_hood::unordered_map<
    render_resource_id, render_attachment_info>;

class render_subpass_builder
{
    std::unique_ptr<render_subpass> m_dataBuf;
    render_subpass* m_data;
    render_attachment_info_map m_attachments;

public:
    template<typename T = render_subpass>
    inline const T& data() const noexcept
    {
        return *reinterpret_cast<const T*>(m_data);
    }

    template<typename T = render_subpass>
    inline T& data() noexcept
    {
        return *reinterpret_cast<T*>(m_data);
    }

    inline const render_attachment_info_map& attachments() const noexcept
    {
        return m_attachments;
    }

    template<typename T = render_subpass>
    inline T* release_data() noexcept
    {
        return reinterpret_cast<T*>(m_dataBuf.release());
    }

    inline void add_vertex_shader_input(render_resource_id resID)
    {
        m_attachments[resID].type |= internal::in_attachment_type::vertex_shader_input;
    }

    inline void add_pixel_shader_input(render_resource_id resID)
    {
        m_attachments[resID].type |= internal::in_attachment_type::pixel_shader_input;
    }

    inline void add_color_output(render_resource_id resID)
    {
        m_attachments[resID].type |= internal::in_attachment_type::color_output;
    }

    void add_color_output(render_resource_id resID,
        const VkPipelineColorBlendAttachmentState& vkBlendState)
    {
        auto& attachment = m_attachments[resID];
        attachment.type |= internal::in_attachment_type::color_output;
        attachment.vkColorBlendState = vkBlendState;
    }

    template<typename T>
    render_subpass_builder(std::unique_ptr<T>&& subpassData) :
        m_dataBuf(std::move(subpassData)),
        m_data(m_dataBuf.get()) {}
};

class render_pass_builder
{
    std::unique_ptr<render_pass> m_dataBuf;
    render_pass* m_data;
    std::vector<render_subpass_builder> m_subpasses;

public:
    bool doUpdateViewport = true;
    bool doUpdateScissor = true;

    template<typename T = render_pass>
    inline const T& data() const noexcept
    {
        return *reinterpret_cast<const T*>(m_data);
    }

    template<typename T = render_pass>
    inline T& data() noexcept
    {
        return *reinterpret_cast<T*>(m_data);
    }

    inline const std::vector<render_subpass_builder>& subpasses() const noexcept
    {
        return m_subpasses;
    }

    inline std::vector<render_subpass_builder>& subpasses() noexcept
    {
        return m_subpasses;
    }

    template<typename T = render_subpass>
    inline T* release_data() noexcept
    {
        return reinterpret_cast<T*>(m_dataBuf.release());
    }

    template<typename T, typename... args_t>
    render_subpass_builder& add_subpass(args_t&&... args)
    {
        std::unique_ptr<T> subpassData(new T(std::forward<args_t>(args)...));
        m_subpasses.emplace_back(std::move(subpassData));
        return m_subpasses.back();
    }

    template<typename T>
    render_pass_builder(std::unique_ptr<T>&& passData) :
        m_dataBuf(std::move(passData)),
        m_data(m_dataBuf.get()) {}
};

class render_graph_builder
{
    std::vector<internal::in_render_resource_info> m_resources;
    std::vector<render_pass_builder> m_passes;

public:
    constexpr static render_resource_id screen_output() noexcept
    {
        return 0;
    }

    inline const std::vector<render_pass_builder>& passes() const noexcept
    {
        return m_passes;
    }

    inline std::vector<render_pass_builder>& passes() noexcept
    {
        return m_passes;
    }

    template <typename... args_t>
    render_resource_id add_resource(args_t&&... args)
    {
        const render_resource_id curResID = m_resources.size();
        m_resources.emplace_back(std::forward<args_t>(args)...);
        return curResID;
    }

    template <typename T, typename... args_t>
    render_pass_builder& add_pass(args_t&&... args)
    {
        std::unique_ptr<T> passData(new T(std::forward<args_t>(args)...));
        m_passes.emplace_back(std::move(passData));
        return m_passes.back();
    }

    HR_GFX_API render_graph build(render_device& device);

    render_graph_builder(const color& screenClearColor)
    {
        m_resources.emplace_back(screenClearColor);
    }
};
} // gfx
} // hr
#endif
