#include "hr_in_render_graph.h"
#include "hedgerender/gfx/hr_render_device.h"
#include <memory>

namespace hr
{
namespace gfx
{
namespace internal
{
void in_render_pass::destroy(VkDevice vkDevice) noexcept
{
    vkDestroyRenderPass(vkDevice, vkRenderPass, nullptr);
}

in_render_pass::in_render_pass(render_pass* pass, std::size_t firstSubpassIndex,
    std::size_t subpassCount, std::size_t firstClearValueIndex,
    std::size_t screenOutputAttachIndex, uint32_t attachmentCount,
    bool doUpdateViewport, bool doUpdateScissor) noexcept :

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
    vkColorAttachBlendStates(attachmentCount, no_default_construct) {}

void in_render_resource::destroy(VkDevice vkDevice, VmaAllocator vmaAllocator) noexcept
{
    vkDestroyImageView(vkDevice, vkImageView, nullptr);
    vmaDestroyImage(vmaAllocator, vkImage, vmaAlloc);
}

VkFramebuffer in_render_graph::get_framebuffer(
    std::size_t renderPassIndex, std::size_t imageIndex) const
{
    return vkFramebuffersPerImagePass[(passes.size() * imageIndex) + renderPassIndex];
}

in_render_resource& in_render_graph::add_new_resource(
    VkFormat vkFormat, uint32_t width, uint32_t height)
{
    // Create resource.
    resources.emplace_back();
    auto& res = resources.back();

    // Create Vulkan image for resource.
    const VkImageCreateInfo vkImageCreateInfo =
    {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,                            // sType
        nullptr,                                                        // pNext
        0,                                                              // flags
        VK_IMAGE_TYPE_2D,                                               // imageType
        vkFormat,                                                       // format
        
        {                                                               // extent
            width,                                                      //  width
            height,                                                     //  height
            1                                                           //  depth
        },
        
        1,                                                              // mipLevels
        1,                                                              // arrayLayers
        VK_SAMPLE_COUNT_1_BIT,                                          // samples
        VK_IMAGE_TILING_OPTIMAL,                                        // tiling
        
        // TODO: Make sure us using transient attachments + lazily-allocated memory is always ok!
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |                       // usage
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        
        VK_SHARING_MODE_EXCLUSIVE,                                      // sharingMode
        0,                                                              // queueFamilyIndexCount
        nullptr,                                                        // pQueueFamilyIndices
        VK_IMAGE_LAYOUT_UNDEFINED                                       // initialLayout
    };
    
    const VmaAllocationCreateInfo vmaAllocCreateInfo =
    {
        0,                                                              // flags
        VMA_MEMORY_USAGE_GPU_ONLY,                                      // usage
        0,                                                              // requiredFlags
        VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,                        // preferredFlags
        0,                                                              // memoryTypeBits
        VK_NULL_HANDLE,                                                 // pool
        nullptr,                                                        // pUserData
        0                                                               // priority
    };
    
    if (vmaCreateImage(m_vmaAllocator, &vkImageCreateInfo,
        &vmaAllocCreateInfo, &res.vkImage, &res.vmaAlloc,
        nullptr) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create Vulkan image");
    }

    // Create Vulkan image view for resource.
    const VkImageViewCreateInfo vkImageViewCreateInfo =
    {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,                       // sType
        nullptr,                                                        // pNext
        0,                                                              // flags
        res.vkImage,                                                    // image
        VK_IMAGE_VIEW_TYPE_2D,                                          // viewType
        vkImageCreateInfo.format,                                       // format
        {},                                                             // components
        {                                                               // subresourceRange
            VK_IMAGE_ASPECT_COLOR_BIT,                                  //  aspectMask
            0,                                                          //  baseMipLevel
            1,                                                          //  levelCount
            0,                                                          //  baseArrayLayer
            1                                                           //  layerCount
        }
    };

    if (vkCreateImageView(m_vkDevice, &vkImageViewCreateInfo,
        nullptr, &res.vkImageView) != VK_SUCCESS)
    {
        vmaDestroyImage(m_vmaAllocator, res.vkImage, res.vmaAlloc);
        res.vkImage = VK_NULL_HANDLE;
        res.vmaAlloc = VK_NULL_HANDLE;
        throw std::runtime_error("Could not create Vulkan image view");
    }
    
    return res;
}

VkFramebuffer in_render_graph::add_new_framebuffer(
    const VkFramebufferCreateInfo& vkFramebufferCreateInfo)
{
    vkFramebuffers.push_back(VK_NULL_HANDLE);
    auto& vkFramebuffer = vkFramebuffers.back();

    if (vkCreateFramebuffer(m_vkDevice, &vkFramebufferCreateInfo,
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
                vkPassAttachments[pass.screenOutputAttachIndex] =
                    swapChain.vkSwapChainImageViews[imageIndex];

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
    if (!m_vkDevice) return;

    // Wait for Vulkan device to idle so we can safely clean everything up.
    vkDeviceWaitIdle(m_vkDevice);

    // Destroy resources.
    for (auto& resource : resources)
    {
        resource.destroy(m_vkDevice, m_vmaAllocator);
    }

    // Destroy Vulkan framebuffers.
    for (auto vkFramebuffer : vkFramebuffers)
    {
        vkDestroyFramebuffer(m_vkDevice, vkFramebuffer, nullptr);
    }

    // Destroy passes.
    for (auto& pass : passes)
    {
        pass.destroy(m_vkDevice);
    }
}

in_render_graph& in_render_graph::operator=(in_render_graph&& other) noexcept
{
    if (&other != this)
    {
        destroy();

        m_vkDevice = other.m_vkDevice;
        m_vmaAllocator = other.m_vmaAllocator;
        passes = std::move(other.passes);
        subpasses = std::move(other.subpasses);
        vkClearValues = std::move(other.vkClearValues);
        vkFramebuffersPerImagePass = std::move(vkFramebuffersPerImagePass);
        vkFramebuffers = std::move(other.vkFramebuffers);
        resources = std::move(other.resources);

        other.m_vkDevice = VK_NULL_HANDLE;
    }

    return *this;
}

in_render_graph::in_render_graph(VkDevice vkDevice, VmaAllocator vmaAllocator) :
    m_vkDevice(vkDevice),
    m_vmaAllocator(vmaAllocator) {}

in_render_graph::in_render_graph(in_render_graph&& other) noexcept :
    m_vkDevice(other.m_vkDevice),
    m_vmaAllocator(other.m_vmaAllocator),
    passes(std::move(other.passes)),
    subpasses(std::move(other.subpasses)),
    vkClearValues(std::move(other.vkClearValues)),
    vkFramebuffersPerImagePass(std::move(vkFramebuffersPerImagePass)),
    vkFramebuffers(std::move(vkFramebuffers)),
    resources(std::move(resources))
{
    other.m_vkDevice = VK_NULL_HANDLE;
}
} // internal

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
    bool needsDependency = false;
};

static VkPipelineStageFlags in_vulkan_get_stage_mask(
    const internal::in_attachment_type types) noexcept
{
    using namespace internal;

    VkPipelineStageFlags vkFlags = 0;
    if (in_has_attachment_type(types, in_attachment_type::vertex_shader_input))
    {
        vkFlags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }

    if (in_has_attachment_type(types, in_attachment_type::pixel_shader_input))
    {
        vkFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    if (in_has_attachment_type(types, in_attachment_type::color_output))
    {
        vkFlags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
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
        in_attachment_type::pixel_shader_input))
    {
        vkFlags |= VK_ACCESS_SHADER_READ_BIT;
    }

    if (in_has_attachment_type(types, in_attachment_type::color_output))
    {
        vkFlags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
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
    case in_attachment_type::pixel_shader_input:
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    case in_attachment_type::color_output:
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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
            if (subpass->attachments().contains(render_graph_builder::screen_output))
            {
                return &(*pass);
            }
        }
    }

    return nullptr;
}

render_graph render_graph_builder::build(render_device& device)
{
    using namespace internal;

    // Allocate render graph and graph creation resources.
    std::unique_ptr<in_render_graph> graph(new in_render_graph(
        device.m_vkDevice, device.allocator().handle()));

    std::vector<VkAttachmentDescription> vkAttachmentDescriptions;
    std::vector<VkAttachmentReference> vkAttachmentReferences;
    std::vector<uint32_t> vkPreserveAttachments;
    std::vector<VkSubpassDescription> vkSubpassDescriptions;
    std::vector<VkSubpassDependency> vkSubpassDependencies;

    graph->passes.reserve(m_passes.size());
    graph->vkFramebuffersPerImagePass.reserve(m_passes.size() *
        device.m_swapChain.vkSwapChainImageCount);

    // Allocate temporary resource state array.
    fixed_array<in_render_resource_state, render_resource_id> resState(
        static_cast<render_resource_id>(m_resources.size()));

    // The default "screen output" resource always requires a subpass dependency.
    resState[screen_output].needsDependency = true;

    // Get the final render pass which uses the screen output resource.
    auto presentPass = in_vulkan_get_present_pass(m_passes);

    // Generate render passes.
    for (auto pass = m_passes.begin(); pass != m_passes.end(); ++pass)
    {
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
                auto& res = resState[attachInfo.first];
                auto& resInfo = m_resources[attachInfo.first];

                // Generate Vulkan attachment descriptions as necessary.
                if (res.mostRecentPass != &(*pass))
                {
                    // Determine the final image layout for this attachment.
                    VkImageLayout vkFinalLayout = in_vulkan_get_next_pass_layout(
                        attachInfo.first, pass, m_passes.end());

                    if (vkFinalLayout == VK_IMAGE_LAYOUT_UNDEFINED)
                    {
                        // Fallback to present layout if this is the present pass, or
                        // the final image layout used amongst the subpasses within
                        // this pass otherwise.
                        vkFinalLayout = (&(*pass) == presentPass) ?
                            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR :
                            in_vulkan_get_final_layout(attachInfo.first,
                                *pass, curSubpassIndex);
                    }

                    // Generate Vulkan attachment.
                    // (NOTE: We don't need to create a resource for the screen output; just
                    // set these to nullptr for now and fill the pointer in properly later.)
                    graph->vkAttachments.push_back((attachInfo.first == screen_output) ?
                        VK_NULL_HANDLE : graph->add_new_resource(
                            device.m_swapChain.vkSurfaceFormat.format,
                            device.m_swapChain.vkSurfaceExtent.width,
                            device.m_swapChain.vkSurfaceExtent.height).vkImageView);

                    // Set Vulkan attachment index for this resource/pass.
                    res.vkAttachmentIndex = static_cast<uint32_t>(
                        vkAttachmentDescriptions.size());

                    // Generate Vulkan attachment description.
                    vkAttachmentDescriptions.emplace_back();
                    auto& vkAttachDesc = vkAttachmentDescriptions.back();

                    vkAttachDesc.flags = 0;
                    vkAttachDesc.format = device.m_swapChain.vkSurfaceFormat.format;
                    vkAttachDesc.samples = VK_SAMPLE_COUNT_1_BIT;

                    vkAttachDesc.loadOp = (res.mostRecentPass != nullptr) ?
                        VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;

                    vkAttachDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

                    vkAttachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    vkAttachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

                    vkAttachDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    vkAttachDesc.finalLayout = vkFinalLayout;

                    // Generate Vulkan clear value and set it as necessary.
                    graph->vkClearValues.emplace_back();
                    auto& vkClearValue = graph->vkClearValues.back();

                    if (vkAttachDesc.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
                    {
                        // TODO: Handle non-float32 formats??
                        vkClearValue.color.float32[0] = resInfo.clearColor.r;
                        vkClearValue.color.float32[1] = resInfo.clearColor.g;
                        vkClearValue.color.float32[2] = resInfo.clearColor.b;
                        vkClearValue.color.float32[3] = resInfo.clearColor.a;
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
                auto& res = resState[attachInfo.first];
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
                auto& res = resState[attachInfo.first];
                if (in_has_attachment_type(attachInfo.second.type,
                    in_attachment_type::color_output))
                {
                    in_vulkan_add_attachment_ref(res,
                        attachInfo.second.type, vkAttachmentReferences);

                    ++vkCurAttachCount;
                }
            }

            vkSubpassDesc.colorAttachmentCount = vkCurAttachCount;

            // Add subpass to graph.
            graph->subpasses.emplace_back(&subpass.data(),
                vkSubpassDesc.colorAttachmentCount);

            auto& graphSubpass = graph->subpasses.back();
            subpass.release_data();

            // Copy over Vulkan color attachment blend states.
            vkCurAttachCount = 0;
            for (auto attachInfo : subpass.attachments())
            {
                auto& res = resState[attachInfo.first];
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

            for (render_resource_id resID = 0; resID < resState.size(); ++resID)
            {
                auto& res = resState[resID];
                if (!subpass.attachments().contains(resID) &&
                   (res.mostRecentSubpassIndex > curSubpassIndex ||
                    in_vulkan_resource_used_in_later_passes(
                        resID, pass, m_passes.end())))
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

            vkSubpassDesc.pPreserveAttachments = vkCurPreserveAttach;
            vkCurPreserveAttach += vkSubpassDesc.preserveAttachmentCount;
        }

        // Get screen output attachment index, or SIZE_MAX if
        // this pass doesn't use the screen output.
        auto& screenOutputRes = resState[screen_output];
        const std::size_t screenOutputAttachIndex =
            (screenOutputRes.mostRecentPass != &(*pass)) ? SIZE_MAX :
            (graphFirstAttachmentIndex + screenOutputRes.vkAttachmentIndex);

        // Add pass object to graph.
        graph->passes.emplace_back(&pass->data(), graphFirstSubpassIndex,
            pass->subpasses().size(), graphFirstClearValueIndex, screenOutputAttachIndex,
            static_cast<uint32_t>(vkAttachmentDescriptions.size()),
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

constexpr render_resource_id render_graph_builder::screen_output;
} // gfx
} // hr
