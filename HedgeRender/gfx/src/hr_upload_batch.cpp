#include "hedgerender/gfx/hr_render_device.h"

namespace hr
{
namespace gfx
{
upload_batch::upload_batch(render_device& device,
    internal::in_per_upload_batch_data& batchData,
    VkCommandBuffer vkCmdBuf) noexcept :
    m_device(&device),
    m_batchData(&batchData),
    m_cmdList(vkCmdBuf),
    m_batchID(batchData.curBatchID) {}

void upload_batch::in_ensure_batch_was_submitted() const
{
    assert(m_batchID == 0 &&
        "upload_batch::submit() must be called exactly once "
        "for every upload_batch object!");
}

void upload_batch::add(const void* src, buffer& dst)
{
    assert(m_batchID != 0 &&
        "upload_batch::add() cannot be called on an "
        "upload batch that has already been submitted!");

    // Create upload buffer.
    void* uploadBufMappedData;
    m_batchData->uploadBuffers.emplace_back(*m_device,
        memory_type::upload, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        dst.size(), &uploadBufMappedData);

    auto& uploadBuf = m_batchData->uploadBuffers.back();

    // Copy data into upload buffer.
    std::memcpy(uploadBufMappedData, src, dst.size());

    // TODO: Unmap data?

    // Copy upload buffer data into buffer.
    m_cmdList.copy_buffer(uploadBuf, dst);
}

void upload_batch::add(const void* src, image& dst)
{
    assert(m_batchID != 0 &&
        "upload_batch::add() cannot be called on an "
        "upload batch that has already been submitted!");

    // Create upload buffer.
    void* uploadBufMappedData;
    m_batchData->uploadBuffers.emplace_back(*m_device,
        memory_type::upload, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        dst.size(), &uploadBufMappedData);

    auto& uploadBuf = m_batchData->uploadBuffers.back();

    // Copy data into upload buffer.
    std::memcpy(uploadBufMappedData, src, dst.size());

    // TODO: Unmap data?

    // Transition image into TRANSFER_DST image layout.
    m_cmdList.transition_image_layout(dst,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT);

    // Copy upload buffer data into image.
    m_cmdList.copy_buffer_to_image(uploadBuf, dst,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
}

void upload_batch::submit()
{
    using namespace internal;

    assert(m_batchID != 0 &&
        "upload_batch::submit() must be called exactly once "
        "for every upload_batch object!");

    // Finish recording Vulkan command buffer.
    const auto vkTransferCmdBuf = m_cmdList.handle();
    if (vkEndCommandBuffer(vkTransferCmdBuf) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not finish recording Vulkan command buffer");
    }

    // Generate Vulkan submit info.
    const VkTimelineSemaphoreSubmitInfo vkTimelineSemaphoreSubmitInfo =
    {
        VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,               // sType
        nullptr,                                                        // pNext
        0,                                                              // waitSemaphoreValueCount
        nullptr,                                                        // pWaitSemaphoreValues
        1,                                                              // signalSemaphoreValueCount
        &m_batchID                                                      // pSignalSemaphoreValues
    };

    const VkSubmitInfo vkSubmitInfo =
    {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,                                  // sType
        &vkTimelineSemaphoreSubmitInfo,                                 // pNext
        0,                                                              // waitSemaphoreCount
        nullptr,                                                        // pWaitSemaphores
        nullptr,                                                        // pWaitDstStageMask
        1,                                                              // commandBufferCount
        &vkTransferCmdBuf,												// pCommandBuffers
        1,                                                              // signalSemaphoreCount
        &m_device->m_vkUploadCompleteSemaphore                          // pSignalSemaphores
    };

    // Lock graphics queue if our "transfer queue" is actually just the graphics queue.
    std::unique_lock<std::mutex> m_gfxQueueLock;
    if (!m_device->adapter().queue_families().has_unique_family(in_queue_type::transfer))
    {
        m_gfxQueueLock = m_device->get_gfx_queue_lock();
    }

    // Submit command buffers to Vulkan transfer queue.
    if (vkQueueSubmit(m_device->queue(in_queue_type::transfer),
        1, &vkSubmitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not submit command buffers to Vulkan transfer queue");
    }

    // Set batch ID to 0 to mark this upload batch as having been submitted.
    m_batchID = 0;
}

upload_batch& upload_batch::operator=(upload_batch&& other) noexcept
{
    if (&other != this)
    {
        in_ensure_batch_was_submitted();

        m_device = other.m_device;
        m_batchData = other.m_batchData;
        m_cmdList = std::move(other.m_cmdList);
        m_batchID = other.m_batchID;
        
        other.m_batchID = 0;
    }

    return *this;
}

upload_batch::upload_batch(upload_batch&& other) noexcept :
    m_device(other.m_device),
    m_batchData(other.m_batchData),
    m_cmdList(std::move(other.m_cmdList)),
    m_batchID(other.m_batchID)
{
    other.m_batchID = 0;
}
} // gfx
} // hr
