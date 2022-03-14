#include "hr_in_render_graph.h"
#include "hedgerender/gfx/hr_renderer.h"
#include "hedgerender/gfx/hr_render_device.h"
#include "hedgerender/gfx/hr_render_graph.h"

namespace hr
{
namespace gfx
{
static void in_vulkan_tmp_render_test(render_device& device,
    internal::in_render_graph& graph, unsigned int threadIndex)
{
    using namespace internal;

    // Reset Vulkan graphics command pool for this thread/frame.
    const unsigned int curFrameIndex = device.cur_frame_index();
    auto& curFrameData = device.per_frame_data(curFrameIndex);
    auto& curFrameThreadData = device.per_frame_thread_data(curFrameIndex, threadIndex);

    if (vkResetCommandPool(device.handle(),
        curFrameThreadData.vkGraphicsCmdPool, 0) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not reset Vulkan graphics command pool");
    }

    // Begin recording Vulkan command buffer.
    const VkCommandBufferBeginInfo vkCmdBufBeginInfo =
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,                    // sType
        nullptr,                                                        // pNext
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,                    // flags
        nullptr                                                         // pInheritanceInfo
    };

    if (vkBeginCommandBuffer(curFrameThreadData.graphicsCmdList.handle(),
        &vkCmdBufBeginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not begin recording Vulkan command buffer");
    }

    const uint32_t curImageIndex = device.swap_chain().curImageIndex;
    const VkFramebuffer* curVkFramebuffer =
        (graph.vkFramebuffersPerImagePass.data() +
        (graph.passes.size() * curImageIndex));

    for (auto& pass : graph.passes)
    {
        auto subpasses = (graph.subpasses.data() + pass.firstSubpassIndex);
        pass.passData->before_pass(curFrameThreadData.graphicsCmdList);

        // Begin Vulkan render pass.
        const VkRenderPassBeginInfo vkPassBeginInfo =
        {
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,                   // sType
            nullptr,                                                    // pNext
            pass.vkRenderPass,                                          // renderPass
            *curVkFramebuffer,                                          // framebuffer

            {                                                           // renderArea
                {                                                       //  offset
                    0,                                                  //   x
                    0                                                   //   y
                },
                device.swap_chain().vkSurfaceExtent                     //  extent
            },

            pass.attachmentCount,                                       // clearValueCount
            (graph.vkClearValues.data() + pass.firstClearValueIndex)    // pClearValues
        };

        vkCmdBeginRenderPass(curFrameThreadData.graphicsCmdList.handle(),
            &vkPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Set Vulkan dynamic viewport state if requested.
        auto& vkSurfaceExtent = device.swap_chain().vkSurfaceExtent;
        if (pass.doUpdateViewport)
        {
            const VkViewport vkViewports[] =
            {
                0.0f,                                                   // x
                0.0f,                                                   // y
                static_cast<float>(vkSurfaceExtent.width),              // width
                static_cast<float>(vkSurfaceExtent.height),             // height
                0.0f,                                                   // minDepth
                1.0f                                                    // maxDepth
            };

            vkCmdSetViewport(curFrameThreadData.graphicsCmdList.handle(), 0, 1, vkViewports);
        }
        
        // Set Vulkan dynamic scissor state if requested.
        if (pass.doUpdateScissor)
        {
            const VkRect2D vkScissors[] =
            {
                {
                    {                                                   // offset
                        0,                                              //  x
                        0                                               //  y
                    },

                    vkSurfaceExtent                                     // extent
                }
            };

            vkCmdSetScissor(curFrameThreadData.graphicsCmdList.handle(), 0, 1, vkScissors);
        }

        // TODO: Handle 0 subpasses, or prevent that from even being possible in the render graph building?
        /*for (std::size_t subpassIndex = 0;
            subpassIndex < pass.subpassCount;
            ++subpassIndex)*/
        std::size_t subpassIndex = 0;
        while (true)
        {
            // Execute subpass.
            auto& subpass = subpasses[subpassIndex];
            subpass.subpassData->execute(curFrameThreadData.graphicsCmdList);

            // Go to next subpass, if necessary.
            if (++subpassIndex >= pass.subpassCount)
            {
                break;
            }

            vkCmdNextSubpass(curFrameThreadData.graphicsCmdList.handle(),
                VK_SUBPASS_CONTENTS_INLINE);
        }

        // Finish Vulkan render pass.
        vkCmdEndRenderPass(curFrameThreadData.graphicsCmdList.handle());
        pass.passData->after_pass(curFrameThreadData.graphicsCmdList);
        ++curVkFramebuffer;
    }
    
    // Finish recording Vulkan command buffer.
    if (vkEndCommandBuffer(curFrameThreadData.graphicsCmdList.handle()) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not finish recording Vulkan command buffer");
    }

    // Generate Vulkan submit info.
    const VkSemaphore vkWaitSemaphores[] =
    {
        curFrameData.vkImageAcquiredSemaphore
        //curThreadData.vkCopyCompleteSemaphore
    };

    const VkPipelineStageFlags vkWaitStages[] =
    {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    const uint64_t waitSemaphoreValues[] =
    {
        0
        //waitBatchID
    };

    const uint32_t waitSemaphoreCount = /*(waitBatchID) ? 2 :*/ 1;
    const VkTimelineSemaphoreSubmitInfo vkTimelineSemaphoreSubmitInfo =
    {
        VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,               // sType
        nullptr,                                                        // pNext
        waitSemaphoreCount,                                             // waitSemaphoreValueCount
        waitSemaphoreValues,                                            // pWaitSemaphoreValues
        0,                                                              // signalSemaphoreValueCount
        nullptr                                                         // pSignalSemaphoreValues
    };

    const VkCommandBuffer vkGraphicsCmdBuf = curFrameThreadData.graphicsCmdList.handle();
    const VkSubmitInfo vkSubmitInfo =
    {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,                                  // sType
        &vkTimelineSemaphoreSubmitInfo,                                 // pNext
        waitSemaphoreCount,                                             // waitSemaphoreCount
        vkWaitSemaphores,                                               // pWaitSemaphores
        vkWaitStages,                                                   // pWaitDstStageMask
        1,                                                              // commandBufferCount
        &vkGraphicsCmdBuf,                                              // pCommandBuffers
        1,                                                              // signalSemaphoreCount
        &curFrameData.vkDrawCompleteSemaphore                           // pSignalSemaphores
    };

    // Lock graphics queue.
    std::unique_lock<std::mutex> m_gfxQueueLock = device.get_gfx_queue_lock();

    // Submit command buffers to Vulkan graphics queue.
    if (vkQueueSubmit(device.queue(HR_IN_QUEUE_TYPE_GRAPHICS),
        1, &vkSubmitInfo, curFrameData.vkFence) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not submit command buffers to Vulkan graphics queue");
    }
}

void default_renderer::render(render_graph& graph)
{
    // If the swap chain was just recreated, recreate render graph framebuffers.
    if (m_device->swap_chain().needFramebufferResize)
    {
        graph.recreate_framebuffers(*m_device);
        m_device->swap_chain().needFramebufferResize = false;
    }

    // Render everything.
    in_vulkan_tmp_render_test(*m_device, *graph.handle(), 0);

    // Schedule present of current frame and move to next frame.
    m_device->end_frame();

    // Prepare next frame.
    m_device->begin_frame();
}

default_renderer::default_renderer(render_device& device) :
    m_device(&device)
{
    // Prepare first frame.
    device.begin_frame();
}
} // gfx
} // hr
