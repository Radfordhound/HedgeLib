#ifndef HR_IN_RENDER_GRAPH_H_INCLUDED
#define HR_IN_RENDER_GRAPH_H_INCLUDED
#include "hedgerender/gfx/hr_render_graph.h"
#include "hedgerender/gfx/hr_resource.h"

namespace hr
{
namespace gfx
{
namespace internal
{
struct in_swap_chain;

struct in_render_resource_transition
{
    gfx::image* image;
    VkImageLayout vkInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkAccessFlags vkInitialAccess = 0;
    VkPipelineStageFlags vkInitialStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    in_render_resource_transition(gfx::image& image, VkImageLayout vkInitialLayout,
        VkAccessFlags vkInitialAccess, VkPipelineStageFlags vkInitialStage) noexcept :
        image(&image),
        vkInitialLayout(vkInitialLayout),
        vkInitialAccess(vkInitialAccess),
        vkInitialStage(vkInitialStage) {}
};

struct in_render_pass
{
    std::vector<in_render_resource_transition> initialResTransitions;
    std::unique_ptr<render_pass> passData;
    VkRenderPass vkRenderPass = VK_NULL_HANDLE;
    std::size_t firstSubpassIndex;
    std::size_t subpassCount;
    std::size_t firstClearValueIndex;
    std::size_t screenOutputAttachIndex;
    uint32_t attachmentCount;
    bool doUpdateViewport;
    bool doUpdateScissor;

    inline bool uses_screen_output() const noexcept
    {
        return (screenOutputAttachIndex != SIZE_MAX);
    }

    void transition_resources(cmd_list& cmdList);

    void destroy(VkDevice vkDevice) noexcept;

    in_render_pass(std::vector<in_render_resource_transition>&& initialResTransitions,
        render_pass* pass, std::size_t firstSubpassIndex,
        std::size_t subpassCount, std::size_t firstClearValueIndex,
        std::size_t screenOutputAttachIndex, uint32_t attachmentCount,
        bool doUpdateViewport, bool doUpdateScissor) noexcept;
};

struct in_render_subpass
{
    std::unique_ptr<render_subpass> subpassData;
    hl::fixed_array<VkPipelineColorBlendAttachmentState, uint32_t> vkColorAttachBlendStates;

    in_render_subpass(render_subpass* subpass, uint32_t colorAttachmentCount);
};

struct in_render_resource
{
    gfx::image image;
    image_view imageView;
    bool isValid = false;

    static constexpr memory_type in_get_memory_type(VkImageUsageFlags vkImageUsage) noexcept
    {
        return (vkImageUsage == VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ||
            vkImageUsage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) ?
            memory_type::transient : memory_type::gpu_only;
    }

    static constexpr VkImageUsageFlags in_get_image_usage(VkImageUsageFlags vkImageUsage) noexcept
    {
        if (vkImageUsage == VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ||
            vkImageUsage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            vkImageUsage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
        }

        return vkImageUsage;
    }

    in_render_resource() noexcept = default;

    in_render_resource(render_device& device, VkImageUsageFlags vkImageUsage,
        VkFormat vkFormat, unsigned int width, unsigned int height,
        const char* debugName = "");
};

struct in_render_graph : public non_copyable
{
    render_device* device;
    //VmaAllocator vmaAllocator;
    std::vector<in_render_pass> passes;

    /** @brief One subpass for every subpass in every pass. */
    std::vector<in_render_subpass> subpasses;
    std::vector<VkClearValue> vkClearValues;

    /**
        @brief One VkFramebuffer per-renderpass per-swapchain-image.
        

        Laid out in memory like this (IM = swapchain IMage, RP = Render Pass):

              0    1    2           3    4    5
        IM1: [RP1, RP2, RP3], IM2: [RP1, RP2, RP3], etc.

        You can get the framebuffer for a given swapchain image + render pass like this:
        ((renderPassCount * imageIndex) + renderPassIndex)
        
        Or, use the helper function get_framebuffer(renderPassIndex, imageIndex);
    */
    std::vector<VkFramebuffer> vkFramebuffersPerImagePass;

    /**
        @brief All of the VkFramebuffer objects used in this render graph.

        When destroying the render graph, simply destroy all
        of the framebuffers in this vector.
    */
    std::vector<VkFramebuffer> vkFramebuffers;
    std::vector<in_render_resource> resources;
    std::vector<VkImageView> vkAttachments;

    VkFramebuffer get_framebuffer(std::size_t renderPassIndex,
        std::size_t imageIndex) const;

    VkFramebuffer add_new_framebuffer(
        const VkFramebufferCreateInfo& vkFramebufferCreateInfo);

    void create_framebuffers(const in_swap_chain& swapChain);

    void destroy() noexcept;

    in_render_graph& operator=(in_render_graph&& other) noexcept;

    in_render_graph(render_device& device, std::size_t resCount);

    in_render_graph(in_render_graph&& other) noexcept;

    inline ~in_render_graph()
    {
        destroy();
    }
};
} // internal
} // gfx
} // hr
#endif
