#include "hr_in_render_graph.h"
#include "hedgerender/gfx/hr_render_device.h"
#include <memory>

namespace hr
{
namespace gfx
{
namespace internal
{
void in_render_pass::transition_resources(cmd_list& cmdList)
{
    for (auto& trans : initialResTransitions)
    {
        if (trans.vkInitialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
            continue;

        cmdList.transition_image_layout(*trans.image,
            VK_IMAGE_LAYOUT_UNDEFINED, trans.vkInitialLayout,
            0, trans.vkInitialAccess,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, trans.vkInitialStage);
    }
}

void in_render_pass::destroy(VkDevice vkDevice) noexcept
{
    vkDestroyRenderPass(vkDevice, vkRenderPass, nullptr);
}

in_render_pass::in_render_pass(
    std::vector<in_render_resource_transition>&& initialResTransitions,
    render_pass* pass, std::size_t firstSubpassIndex,
    std::size_t subpassCount, std::size_t firstClearValueIndex,
    std::size_t screenOutputAttachIndex, uint32_t attachmentCount,
    bool doUpdateViewport, bool doUpdateScissor) noexcept :

    initialResTransitions(std::move(initialResTransitions)),
    passData(pass),
    firstSubpassIndex(firstSubpassIndex),
    subpassCount(subpassCount),
    firstClearValueIndex(firstClearValueIndex),
    screenOutputAttachIndex(screenOutputAttachIndex),
    attachmentCount(attachmentCount),
    doUpdateViewport(doUpdateViewport),
    doUpdateScissor(doUpdateScissor) {}

in_render_subpass::in_render_subpass(render_subpass* subpass, uint32_t attachmentCount) :
    subpassData(subpass),
    vkColorAttachBlendStates(hl::no_value_init, attachmentCount) {}

in_render_resource::in_render_resource(render_device& device,
    VkImageUsageFlags vkImageUsage, VkFormat vkFormat,
    unsigned int width, unsigned int height, const char* debugName) :
    
    image(device, in_get_memory_type(vkImageUsage), VK_IMAGE_TYPE_2D,
        in_get_image_usage(vkImageUsage), vkFormat, width, height,
        1, 1, 1, debugName),
    
    imageView(device, image, VK_IMAGE_VIEW_TYPE_2D),
    isValid(true) {}

VkFramebuffer in_render_graph::get_framebuffer(
    std::size_t renderPassIndex, std::size_t imageIndex) const
{
    return vkFramebuffersPerImagePass[(passes.size() * imageIndex) + renderPassIndex];
}

VkFramebuffer in_render_graph::add_new_framebuffer(
    const VkFramebufferCreateInfo& vkFramebufferCreateInfo)
{
    vkFramebuffers.push_back(VK_NULL_HANDLE);
    auto& vkFramebuffer = vkFramebuffers.back();

    if (vkCreateFramebuffer(device->handle(), &vkFramebufferCreateInfo,
        nullptr, &vkFramebuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create Vulkan framebuffer");
    }

    return vkFramebuffer;
}

void in_render_graph::create_framebuffers(const in_swap_chain& swapChain)
{
    // Generate placeholder Vulkan framebuffer create info.
    VkFramebufferCreateInfo vkFramebufferCreateInfo =
    {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,                  // sType
        nullptr,                                                    // pNext
        0,                                                          // flags
        VK_NULL_HANDLE,                                             // renderPass
        0,                                                          // attachmentCount
        nullptr,                                                    // pAttachments
        swapChain.vkSurfaceExtent.width,                            // width
        swapChain.vkSurfaceExtent.height,                           // height
        1                                                           // layers
    };

    // Create Vulkan framebuffer for each swapchain image/pass.
    for (uint32_t imageIndex = 0;
        imageIndex < swapChain.vkSwapChainImageCount;
        ++imageIndex)
    {
        VkImageView* vkPassAttachments = vkAttachments.data();
        for (std::size_t passIndex = 0; passIndex < passes.size(); ++passIndex)
        {
            // Set framebuffer create info values for this pass.
            auto& pass = passes[passIndex];
            vkFramebufferCreateInfo.renderPass = pass.vkRenderPass;
            vkFramebufferCreateInfo.attachmentCount = pass.attachmentCount;
            vkFramebufferCreateInfo.pAttachments = vkPassAttachments;

            // Create at least one Vulkan framebuffer for every pass, and also
            // another one for every subsequent pass that uses the screen output.
            if (imageIndex == 0 || pass.uses_screen_output())
            {
                if (pass.uses_screen_output())
                {
                    vkAttachments[pass.screenOutputAttachIndex] =
                        swapChain.vkSwapChainImageViews[imageIndex];
                }

                vkFramebuffersPerImagePass.push_back(
                    add_new_framebuffer(vkFramebufferCreateInfo));
            }

            // Reuse the existing Vulkan framebuffer pointer for
            // subsequent passes that don't use the screen output.
            else
            {
                vkFramebuffersPerImagePass.push_back(
                    vkFramebuffersPerImagePass[passIndex]);
            }

            // Increase per-pass attachments pointer.
            vkPassAttachments += pass.attachmentCount;
        }
    }
}

void in_render_graph::destroy() noexcept
{
    // Return early if this graph is just an empty shell
    // (i.e. one that's been moved from using std::move()).
    if (!device) return;

    // Wait for Vulkan device to idle so we can safely clean everything up.
    device->wait_for_idle();

    // Destroy Vulkan framebuffers.
    for (auto vkFramebuffer : vkFramebuffers)
    {
        vkDestroyFramebuffer(device->handle(), vkFramebuffer, nullptr);
    }

    // Destroy passes.
    for (auto& pass : passes)
    {
        pass.destroy(device->handle());
    }
}

in_render_graph& in_render_graph::operator=(in_render_graph&& other) noexcept
{
    if (&other != this)
    {
        destroy();

        device = other.device;
        passes = std::move(other.passes);
        subpasses = std::move(other.subpasses);
        vkClearValues = std::move(other.vkClearValues);
        vkFramebuffersPerImagePass = std::move(other.vkFramebuffersPerImagePass);
        vkFramebuffers = std::move(other.vkFramebuffers);
        resources = std::move(other.resources);

        other.device = nullptr;
    }

    return *this;
}

in_render_graph::in_render_graph(render_device& device, std::size_t resCount) :
    device(&device),
    resources(resCount) {}

in_render_graph::in_render_graph(in_render_graph&& other) noexcept :
    device(other.device),
    passes(std::move(other.passes)),
    subpasses(std::move(other.subpasses)),
    vkClearValues(std::move(other.vkClearValues)),
    vkFramebuffersPerImagePass(std::move(other.vkFramebuffersPerImagePass)),
    vkFramebuffers(std::move(other.vkFramebuffers)),
    resources(std::move(other.resources))
{
    other.device = nullptr;
}
} // internal

const image_view& render_graph::get_image_view(render_resource_id resID) const
{
    assert(resID != render_graph_builder::screen_output_id &&
        "The screen output is not a normal resource that is available for access.");

    return m_renderGraph->resources[resID].imageView;
}

void render_graph::recreate_framebuffers(render_device& device)
{
    // Destroy Vulkan framebuffers.
    for (auto vkFramebuffer : m_renderGraph->vkFramebuffers)
    {
        vkDestroyFramebuffer(device.handle(), vkFramebuffer, nullptr);
    }

    m_renderGraph->vkFramebuffers.clear();
    m_renderGraph->vkFramebuffersPerImagePass.clear();

    // Re-create Vulkan framebuffers.
    m_renderGraph->create_framebuffers(device.swap_chain());
}

void render_graph::destroy() noexcept
{
    delete m_renderGraph;
}

render_graph& render_graph::operator=(render_graph&& other) noexcept
{
    if (&other != this)
    {
        destroy();

        m_renderGraph = other.m_renderGraph;
        other.m_renderGraph = nullptr;
    }

    return *this;
}

render_graph::render_graph(render_graph&& other) noexcept :
    m_renderGraph(other.m_renderGraph)
{
    other.m_renderGraph = nullptr;
}

struct in_render_resource_state
{
    const render_pass_builder* mostRecentPass = nullptr;
    std::size_t mostRecentSubpassIndex = 0;
    VkPipelineStageFlags mostRecentStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkAccessFlags mostRecentAccess = 0;
    uint32_t vkAttachmentIndex = 0;
    VkImageUsageFlags vkImageUsageFlags = 0;
    VkImageLayout vkCurLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    bool needsDependency = false;
};

static VkPipelineStageFlags in_vulkan_get_stage_mask(
    const internal::in_attachment_type types) noexcept
{
    using namespace internal;

    VkPipelineStageFlags vkFlags = 0;
    if (in_has_attachment_type(types,
        in_attachment_type::vertex_shader_input |
        in_attachment_type::vertex_shader_input_sampled))
    {
        vkFlags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }

    if (in_has_attachment_type(types,
        in_attachment_type::pixel_shader_input |
        in_attachment_type::pixel_shader_input_sampled))
    {
        vkFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    if (in_has_attachment_type(types, in_attachment_type::color_output))
    {
        vkFlags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }

    if (in_has_attachment_type(types, in_attachment_type::depth_stencil_output))
    {
        vkFlags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }

    return vkFlags;
}

static VkAccessFlags in_vulkan_get_access_mask(
    const internal::in_attachment_type types) noexcept
{
    using namespace internal;

    VkAccessFlags vkFlags = 0;
    if (in_has_attachment_type(types,
        in_attachment_type::vertex_shader_input |
        in_attachment_type::vertex_shader_input_sampled |
        in_attachment_type::pixel_shader_input |
        in_attachment_type::pixel_shader_input_sampled))
    {
        vkFlags |= VK_ACCESS_SHADER_READ_BIT;
    }

    if (in_has_attachment_type(types, in_attachment_type::color_output))
    {
        vkFlags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }

    if (in_has_attachment_type(types, in_attachment_type::depth_stencil_output))
    {
        vkFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }

    return vkFlags;
}

static VkImageLayout in_vulkan_get_image_layout(
    internal::in_attachment_type types) noexcept
{
    using namespace internal;

    switch (types)
    {
    case in_attachment_type::vertex_shader_input:
    case in_attachment_type::vertex_shader_input_sampled:
    case in_attachment_type::pixel_shader_input:
    case in_attachment_type::pixel_shader_input_sampled:
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    case in_attachment_type::color_output:
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    case in_attachment_type::depth_stencil_output:
        // TODO: Make it so you can just do depth?
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    default:
        return VK_IMAGE_LAYOUT_GENERAL;
    }
}

static VkAttachmentReference& in_vulkan_add_attachment_ref(
    const in_render_resource_state& res,
    internal::in_attachment_type types,
    std::vector<VkAttachmentReference>& vkAttachmentReferences)
{
    // Add a new Vulkan attachment reference.
    vkAttachmentReferences.emplace_back();
    auto& vkAttachRef = vkAttachmentReferences.back();

    // Setup the new Vulkan attachment reference.
    vkAttachRef.attachment = res.vkAttachmentIndex;
    vkAttachRef.layout = in_vulkan_get_image_layout(types);

    return vkAttachRef;
}

static bool in_vulkan_resource_used_in_later_passes(render_resource_id resID,
    std::vector<render_pass_builder>::const_iterator pass,
    std::vector<render_pass_builder>::const_iterator endOfPasses)
{
    for (++pass; pass != endOfPasses; ++pass)
    {
        for (auto& subpass : pass->subpasses())
        {
            if (subpass.attachments().contains(resID))
            {
                return true;
            }
        }
    }
    
    return false;
}

static VkImageLayout in_vulkan_get_final_layout(render_resource_id resID,
    const render_pass_builder& pass, std::size_t curSubpassIndex)
{
    // Iterate through all the subpasses in the given pass which use the
    // given resID, and determine the resource's final attachment type.
    auto& subpasses = pass.subpasses();
    auto finalAttachType = subpasses[curSubpassIndex].attachments().at(resID);

    for (auto subpass = (subpasses.begin() + curSubpassIndex + 1);
        subpass != subpasses.end();
        ++subpass)
    {
        // Skip this subpass if it doesn't use the given resID.
        auto attachInfo = subpass->attachments().find(resID);
        if (attachInfo == subpass->attachments().end())
        {
            continue;
        }

        finalAttachType = attachInfo->second;
    }

    // Convert the final attachment type to a Vulkan image layout.
    return in_vulkan_get_image_layout(finalAttachType.type);
}

static VkImageLayout in_vulkan_get_next_pass_layout(render_resource_id resID,
    std::vector<render_pass_builder>::const_iterator pass,
    std::vector<render_pass_builder>::const_iterator endOfPasses)
{
    // Find the first usage of the given resID in subsequent subpasses, if any.
    for (++pass; pass != endOfPasses; ++pass)
    {
        for (auto& subpass : pass->subpasses())
        {
            // Skip this subpass if it doesn't use the given resID.
            auto attachInfo = subpass.attachments().find(resID);
            if (attachInfo == subpass.attachments().end())
            {
                continue;
            }

            // We found the first usage of the given resID within the subsequent subpasses!
            // Return an image layout that works for how the attachment is to be used.
            return in_vulkan_get_image_layout(attachInfo->second.type);
        }
    }

    // There were no more uses of the given resID in subsequent subpasses.
    return VK_IMAGE_LAYOUT_UNDEFINED;
}

static const render_pass_builder* in_vulkan_get_present_pass(
    const std::vector<render_pass_builder>& passes) noexcept
{
    // Find the last render pass which has a subpass that
    // uses the screen output resource as an attachment.
    for (auto pass = passes.rbegin(); pass != passes.rend(); ++pass)
    {
        for (auto subpass = pass->subpasses().rbegin();
            subpass != pass->subpasses().rend();
            ++subpass)
        {
            if (subpass->attachments().contains(render_graph_builder::screen_output_id))
            {
                return &(*pass);
            }
        }
    }

    return nullptr;
}

static constexpr VkImageUsageFlags in_get_image_usage(
    internal::in_attachment_type attachTypes) noexcept
{
    VkImageUsageFlags vkImageUsage = 0;
    if (in_is_input_attachment(attachTypes))
    {
        vkImageUsage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }

    if (in_is_input_attachment_sampled(attachTypes))
    {
        vkImageUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    if (in_is_output_attachment(attachTypes))
    {
        vkImageUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    if (internal::in_has_attachment_type(attachTypes,
        internal::in_attachment_type::depth_stencil_output))
    {
        vkImageUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }

    return vkImageUsage;
}

render_graph render_graph_builder::build(render_device& device)
{
    using namespace internal;

    // Allocate render graph and graph creation resources.
    std::unique_ptr<in_render_graph> graph(new in_render_graph(
        device, m_resources.size()));

    std::vector<VkAttachmentDescription> vkAttachmentDescriptions;
    std::vector<VkAttachmentReference> vkAttachmentReferences;
    std::vector<uint32_t> vkPreserveAttachments;
    std::vector<VkSubpassDescription> vkSubpassDescriptions;
    std::vector<VkSubpassDependency> vkSubpassDependencies;

    graph->passes.reserve(m_passes.size());
    graph->vkFramebuffersPerImagePass.reserve(m_passes.size() *
        device.m_swapChain.vkSwapChainImageCount);

    // Allocate temporary resource state array.

    /*
    * HACK: Since screen_output_id is defined as SIZE_MAX, and unsigned integer
    * overflow is (surprisingly) well defined and valid per the C++ standard,
    * we rely on (resID + 1) always being a valid way to index this array, even
    * when resID is SIZE_MAX, in which case, it (SIZE_MAX + 1) will work out to 0.
    */
    hl::fixed_array<in_render_resource_state> resState(m_resources.size() + 1);

    // The default "screen output" resource always requires a subpass dependency.
    resState[screen_output_id + 1].needsDependency = true;

    // Get the final render pass which uses the screen output resource.
    const auto presentPass = in_vulkan_get_present_pass(m_passes);

    // Get all image usage flags for all resources.
    for (auto& pass : m_passes)
    {
        for (auto& subpass : pass.subpasses())
        {
            for (auto attachInfo : subpass.attachments())
            {
                auto& res = resState[attachInfo.first + 1];
                res.vkImageUsageFlags |= in_get_image_usage(attachInfo.second.type);
            }
        }
    }

    // Generate render passes.
    for (auto pass = m_passes.begin(); pass != m_passes.end(); ++pass)
    {
        std::vector<in_render_resource_transition> resTransitions;
        const std::size_t graphFirstSubpassIndex = graph->subpasses.size();
        const std::size_t graphFirstClearValueIndex = graph->vkClearValues.size();
        const std::size_t graphFirstAttachmentIndex = graph->vkAttachments.size();

        // Generate Vulkan attachment descriptions, attachment references, and subpass
        // dependencies, begin generating subpass descriptions/dependencies, and
        // add subpass execution functions for this pass to the graph.
        for (std::size_t curSubpassIndex = 0;
            curSubpassIndex < pass->subpasses().size();
            ++curSubpassIndex)
        {
            auto& subpass = pass->subpasses()[curSubpassIndex];

            // Generate Vulkan attachment descriptions and subpass dependencies.
            for (auto attachInfo : subpass.attachments())
            {
                auto& res = resState[attachInfo.first + 1];
                auto& resInfo = (attachInfo.first == screen_output_id) ?
                    m_screenOutputResInfo : m_resources[attachInfo.first];

                // Generate Vulkan attachment descriptions as necessary.
                if (res.mostRecentPass != &(*pass))
                {
                    // Determine the final image layout for this attachment.
                    VkImageLayout vkInitialLayout = res.vkCurLayout;
                    VkImageLayout vkFinalLayout = in_vulkan_get_next_pass_layout(
                        attachInfo.first, pass, m_passes.end());

                    VkFormat vkFormat = (resInfo.isDepthStencil) ?
                        VK_FORMAT_D32_SFLOAT : // TODO: Check hardware support and use the best format available.
                        device.m_swapChain.vkSurfaceFormat.format;

                    if (vkFinalLayout == VK_IMAGE_LAYOUT_UNDEFINED)
                    {
                        // Fallback to present layout if this is the screen output of the
                        // present pass, or the final image layout used amongst the subpasses
                        // within this pass otherwise.
                        vkFinalLayout = (attachInfo.first == screen_output_id &&
                            &(*pass) == presentPass) ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR :
                            in_vulkan_get_final_layout(attachInfo.first,
                                *pass, curSubpassIndex);
                    }

                    // Generate Vulkan resources and attachments as necessary.
                    if (attachInfo.first != screen_output_id)
                    {
                        // Create new render resource if necessary.
                        auto& renderRes = graph->resources[attachInfo.first];
                        if (!renderRes.isValid)
                        {
                            renderRes = in_render_resource(device,
                                res.vkImageUsageFlags, vkFormat,
                                device.m_swapChain.vkSurfaceExtent.width,
                                device.m_swapChain.vkSurfaceExtent.height,
                                (resInfo.debugName.empty()) ? nullptr : resInfo.debugName.c_str());

                            // TODO: Do we also need to do this with non-sampled input attachments?
                            if (in_is_input_attachment_sampled(attachInfo.second.type))
                            {
                                vkInitialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                                resTransitions.emplace_back(renderRes.image,
                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                    VK_ACCESS_SHADER_READ_BIT,
                                    in_vulkan_get_stage_mask(attachInfo.second.type));
                            }
                        }

                        // Generate Vulkan attachment.
                        graph->vkAttachments.push_back(renderRes.imageView.handle());
                    }
                    else
                    {
                        // NOTE: This value will be filled in later by the call to create_framebuffers().
                        graph->vkAttachments.push_back(nullptr);
                    }

                    // Set Vulkan attachment index for this resource/pass.
                    res.vkAttachmentIndex = static_cast<uint32_t>(
                        vkAttachmentDescriptions.size());

                    // Update current Vulkan image layout for this resource.
                    res.vkCurLayout = vkFinalLayout;

                    // Generate Vulkan attachment description.
                    vkAttachmentDescriptions.emplace_back();
                    auto& vkAttachDesc = vkAttachmentDescriptions.back();

                    vkAttachDesc.flags = 0;
                    vkAttachDesc.format = vkFormat;
                    vkAttachDesc.samples = VK_SAMPLE_COUNT_1_BIT;

                    vkAttachDesc.loadOp = (res.mostRecentPass != nullptr) ?
                        VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;

                    vkAttachDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

                    vkAttachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    vkAttachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

                    vkAttachDesc.initialLayout = vkInitialLayout;
                    vkAttachDesc.finalLayout = vkFinalLayout;

                    // Generate Vulkan clear value and set it as necessary.
                    graph->vkClearValues.emplace_back();
                    auto& vkClearValue = graph->vkClearValues.back();

                    if (vkAttachDesc.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
                    {
                        // TODO: Handle non-float32 formats??
                        if (resInfo.isDepthStencil)
                        {
                            vkClearValue.color.float32[0] = resInfo.clearColor.r;
                            vkClearValue.color.float32[1] = resInfo.clearColor.g;
                            vkClearValue.color.float32[2] = resInfo.clearColor.b;
                            vkClearValue.color.float32[3] = resInfo.clearColor.a;
                        }
                        else
                        {
                            vkClearValue.color.float32[0] = resInfo.clearColor.r;
                            vkClearValue.color.float32[1] = resInfo.clearColor.g;
                            vkClearValue.color.float32[2] = resInfo.clearColor.b;
                            vkClearValue.color.float32[3] = resInfo.clearColor.a;
                        }
                    }
                }

                // Generate subpass dependencies as necessary.
                if (res.needsDependency)
                {
                    vkSubpassDependencies.emplace_back();
                    auto& vkSubpassDep = vkSubpassDependencies.back();

                    vkSubpassDep.srcSubpass = (res.mostRecentPass == &(*pass)) ?
                        static_cast<uint32_t>(res.mostRecentSubpassIndex) :
                        VK_SUBPASS_EXTERNAL;

                    vkSubpassDep.dstSubpass = static_cast<uint32_t>(curSubpassIndex);

                    vkSubpassDep.srcStageMask = res.mostRecentStage;
                    vkSubpassDep.dstStageMask = in_vulkan_get_stage_mask(
                        attachInfo.second.type);

                    vkSubpassDep.srcAccessMask = res.mostRecentAccess;
                    vkSubpassDep.dstAccessMask = in_vulkan_get_access_mask(
                        attachInfo.second.type);

                    vkSubpassDep.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
                }

                // Update resource state for future iterations.
                else if (in_is_output_attachment(attachInfo.second.type))
                {
                    // This attachment is used as an output, so we'll need to
                    // create a subpass dependency to this resource in any later
                    // subpasses/render passes to synchronize everything correctly.
                    res.needsDependency = true;
                }

                res.mostRecentPass = &(*pass);
                res.mostRecentSubpassIndex = curSubpassIndex;
                res.mostRecentStage = in_vulkan_get_stage_mask(attachInfo.second.type);
                res.mostRecentAccess = in_vulkan_get_access_mask(attachInfo.second.type);
            }

            // Begin generating Vulkan subpass description.
            vkSubpassDescriptions.emplace_back();
            auto& vkSubpassDesc = vkSubpassDescriptions.back();

            vkSubpassDesc.flags = 0;
            vkSubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

            // Generate Vulkan input attachment references.
            uint32_t vkCurAttachCount = 0;
            for (auto attachInfo : subpass.attachments())
            {
                auto& res = resState[attachInfo.first + 1];
                if (in_is_input_attachment(attachInfo.second.type))
                {
                    in_vulkan_add_attachment_ref(res,
                        attachInfo.second.type, vkAttachmentReferences);

                    ++vkCurAttachCount;
                }
            }

            vkSubpassDesc.inputAttachmentCount = vkCurAttachCount;

            // Generate Vulkan color output attachment references.
            vkCurAttachCount = 0;
            for (auto attachInfo : subpass.attachments())
            {
                auto& res = resState[attachInfo.first + 1];
                if (in_has_attachment_type(attachInfo.second.type,
                    in_attachment_type::color_output))
                {
                    in_vulkan_add_attachment_ref(res,
                        attachInfo.second.type, vkAttachmentReferences);

                    ++vkCurAttachCount;
                }
            }

            vkSubpassDesc.colorAttachmentCount = vkCurAttachCount;

            // Generate Vulkan depth stencil output attachment references.
            for (auto attachInfo : subpass.attachments())
            {
                auto& res = resState[attachInfo.first + 1];
                if (in_has_attachment_type(attachInfo.second.type,
                    in_attachment_type::depth_stencil_output))
                {
                    in_vulkan_add_attachment_ref(res,
                        attachInfo.second.type, vkAttachmentReferences);

                    // HACK: Set the depth stencil pointer now even though that could likely
                    // result in a dangling pointer and fix it later. We just need it to be
                    // *something* non-null so we can do a check quickly/easily later.
                    vkSubpassDesc.pDepthStencilAttachment = &vkAttachmentReferences.back();
                }
            }

            // Add subpass to graph.
            graph->subpasses.emplace_back(&subpass.data(),
                vkSubpassDesc.colorAttachmentCount);

            auto& graphSubpass = graph->subpasses.back();
            subpass.release_data();

            // Copy over Vulkan color attachment blend states.
            vkCurAttachCount = 0;
            for (auto attachInfo : subpass.attachments())
            {
                auto& res = resState[attachInfo.first + 1];
                if (in_has_attachment_type(attachInfo.second.type,
                    in_attachment_type::color_output))
                {
                    graphSubpass.vkColorAttachBlendStates[vkCurAttachCount++] =
                        attachInfo.second.vkColorBlendState;
                }
            }
        }

        // Generate Vulkan preserve attachments.
        for (std::size_t curSubpassIndex = 0;
            curSubpassIndex < pass->subpasses().size();
            ++curSubpassIndex)
        {
            auto& subpass = pass->subpasses()[curSubpassIndex];
            uint32_t vkCurAttachCount = 0;

            for (std::size_t i = 0; i < resState.size(); ++i)
            {
                auto& res = resState[i];
                if (!subpass.attachments().contains(i - 1) &&
                   (res.mostRecentSubpassIndex > curSubpassIndex ||
                    in_vulkan_resource_used_in_later_passes(
                        i - 1, pass, m_passes.end())))
                {
                    vkPreserveAttachments.push_back(res.vkAttachmentIndex);
                    ++vkCurAttachCount;
                }
            }
            
            // Update Vulkan subpass description preserve attachment count.
            auto& vkSubpassDesc = vkSubpassDescriptions[curSubpassIndex];
            vkSubpassDesc.preserveAttachmentCount = vkCurAttachCount;
        }

        // Finish generating Vulkan subpass descriptions.
        const VkAttachmentReference* vkCurAttachRef = vkAttachmentReferences.data();
        const uint32_t* vkCurPreserveAttach = vkPreserveAttachments.data();

        for (std::size_t curSubpassIndex = 0;
            curSubpassIndex < pass->subpasses().size();
            ++curSubpassIndex)
        {
            // Set attachment reference pointers in Vulkan subpass description.
            // NOTE: We have to do this here to avoid vector reallocation issues!
            auto& vkSubpassDesc = vkSubpassDescriptions[curSubpassIndex];

            vkSubpassDesc.pInputAttachments = vkCurAttachRef;
            vkCurAttachRef += vkSubpassDesc.inputAttachmentCount;

            vkSubpassDesc.pColorAttachments = vkCurAttachRef;
            vkCurAttachRef += vkSubpassDesc.colorAttachmentCount;

            // HACK: Fix the (probably dangling) depth/stencil attachment pointer from earlier.
            if (vkSubpassDesc.pDepthStencilAttachment)
            {
                vkSubpassDesc.pDepthStencilAttachment = vkCurAttachRef++;
            }

            vkSubpassDesc.pPreserveAttachments = vkCurPreserveAttach;
            vkCurPreserveAttach += vkSubpassDesc.preserveAttachmentCount;
        }

        // Get screen output attachment index, or SIZE_MAX if
        // this pass doesn't use the screen output.
        auto& screenOutputRes = resState[screen_output_id + 1];
        const std::size_t screenOutputAttachIndex =
            (screenOutputRes.mostRecentPass != &(*pass)) ? SIZE_MAX :
            (graphFirstAttachmentIndex + screenOutputRes.vkAttachmentIndex);

        // Add pass object to graph.
        graph->passes.emplace_back(std::move(resTransitions), &pass->data(),
            graphFirstSubpassIndex, pass->subpasses().size(), graphFirstClearValueIndex,
            screenOutputAttachIndex, static_cast<uint32_t>(vkAttachmentDescriptions.size()),
            pass->doUpdateViewport, pass->doUpdateScissor);

        auto& graphPass = graph->passes.back();
        pass->release_data();

        // Create Vulkan render pass.
        const VkRenderPassCreateInfo vkRenderPassCreateInfo =
        {
            VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,                  // sType
            nullptr,                                                    // pNext
            0,                                                          // flags
            static_cast<uint32_t>(vkAttachmentDescriptions.size()),     // attachmentCount
            vkAttachmentDescriptions.data(),                            // pAttachments
            static_cast<uint32_t>(vkSubpassDescriptions.size()),        // subpassCount
            vkSubpassDescriptions.data(),                               // pSubpasses
            static_cast<uint32_t>(vkSubpassDependencies.size()),        // dependencyCount
            vkSubpassDependencies.data()                                // pDependencies
        };

        if (vkCreateRenderPass(device.m_vkDevice, &vkRenderPassCreateInfo,
            nullptr, &graphPass.vkRenderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("Could not create Vulkan render pass");
        }

        // Clear vectors for next loop iteration.
        vkAttachmentDescriptions.clear();
        vkAttachmentReferences.clear();
        vkPreserveAttachments.clear();
        vkSubpassDescriptions.clear();
        vkSubpassDependencies.clear();
    }

    // Create Vulkan framebuffers and return graph.
    graph->create_framebuffers(device.m_swapChain);
    return graph.release();
}
} // gfx
} // hr
