#include "hr_in_resource.h"
#include "hedgerender/gfx/hr_cmd_list.h"
#include "hedgerender/gfx/hr_resource.h"
#include "hedgerender/gfx/hr_pipeline.h"

namespace hr
{
namespace gfx
{
void cmd_list::copy_buffer_to_image(const buffer& src,
    image& dst, VkImageLayout layout)
{
    // Generate Vulkan buffer image copy regions.
    stack_or_heap_buffer<VkBufferImageCopy, 16> vkCopyRegions(
        dst.layer_count() * dst.mip_levels());

    const in_vulkan_format_info fmtInfo(dst.format());
    VkBufferImageCopy* vkCurCopyRegion = vkCopyRegions;
    VkDeviceSize vkCurBufOffset = 0;

    for (unsigned int i = 0; i < dst.layer_count(); ++i)
    {
        VkExtent3D vkExtent = dst.extent();
        for (unsigned int i2 = 0; i2 < dst.mip_levels(); ++i2)
        {
            vkCurCopyRegion->bufferOffset = vkCurBufOffset;
            vkCurCopyRegion->bufferRowLength = 0;
            vkCurCopyRegion->bufferImageHeight = 0;
            vkCurCopyRegion->imageSubresource =
            {
                // TODO: Copy all image aspects, not just color??
                VK_IMAGE_ASPECT_COLOR_BIT,                              // aspectMask
                static_cast<uint32_t>(i2),                              // mipLevel
                static_cast<uint32_t>(i),                               // baseArrayLayer
                1                                                       // layerCount
            };

            vkCurCopyRegion->imageOffset = { 0, 0, 0 };
            vkCurCopyRegion->imageExtent = vkExtent;

            vkCurBufOffset += fmtInfo.get_bytes_per_mip_level(vkExtent);
            in_vulkan_get_next_mip_extent(vkExtent);
            ++vkCurCopyRegion;
        }
    }

    // Schedule a copy from the given buffer to the given image.
    vkCmdCopyBufferToImage(m_vkCmdBuf, src.handle(), dst.handle(), 
        layout, dst.mip_levels(), vkCopyRegions);
}

void cmd_list::transition_image_layout(image& img,
    VkImageLayout oldLayout, VkImageLayout newLayout,
    VkAccessFlags oldAccess, VkAccessFlags newAccess,
    VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages)
{
    const VkImageMemoryBarrier vkImgMemoryBarrier =
    {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,                         // sType
        nullptr,                                                        // pNext
        static_cast<VkAccessFlags>(oldAccess),                          // srcAccessMask
        static_cast<VkAccessFlags>(newAccess),                          // dstAccessMask
        static_cast<VkImageLayout>(oldLayout),                          // oldLayout
        static_cast<VkImageLayout>(newLayout),                          // newLayout
        VK_QUEUE_FAMILY_IGNORED,                                        // srcQueueFamilyIndex
        VK_QUEUE_FAMILY_IGNORED,                                        // dstQueueFamilyIndex
        img.handle(),                                                   // image
        {                                                               // subresourceRange
            VK_IMAGE_ASPECT_COLOR_BIT,                                  //  aspectMask
            0,                                                          //  baseMipLevel
            img.mip_levels(),                                           //  levelCount
            0,                                                          //  baseArrayLayer
            img.layer_count()                                           //  layerCount
        }
    };

    vkCmdPipelineBarrier(m_vkCmdBuf, srcStages, dstStages,
        0, 0, nullptr, 0, nullptr, 1, &vkImgMemoryBarrier);
}

void cmd_list::push_constants(const pipeline_layout& layout,
    VkShaderStageFlags vkShaderStages, unsigned int offset,
    unsigned int count, const void* values)
{
    vkCmdPushConstants(m_vkCmdBuf, layout.handle(), vkShaderStages,
        offset, count * sizeof(uint32_t), values);
}

void cmd_list::set_viewport(float x, float y,
    float width, float height, float minDepth, float maxDepth)
{
    const VkViewport vkViewport =
    {
        x,                                                              // x
        y,                                                              // y
        width,                                                          // width
        height,                                                         // height
        minDepth,                                                       // minDepth
        maxDepth                                                        // maxDepth
    };

    vkCmdSetViewport(m_vkCmdBuf, 0, 1, &vkViewport);
}

void cmd_list::set_scissor(int x, int y, unsigned int width, unsigned int height)
{
    const VkRect2D vkScissor =
    {
        {                                                               // offset
            x,                                                          //  x
            y                                                           //  y
        },

        {                                                               // extent
            width,                                                      //  width
            height                                                      //  height
        }
    };

    vkCmdSetScissor(m_vkCmdBuf, 0, 1, &vkScissor);
}

void cmd_list::bind_pipeline(const pipeline& pipeline)
{
    // TODO: Also allow compute pipelines.
    vkCmdBindPipeline(m_vkCmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle());
}

void cmd_list::bind_vertex_buffers(const buffer* buffers,
    const VkDeviceSize* offsets, unsigned int startSlot,
    unsigned int bufCount)
{
    // Generate Vulkan vertex buffer array.
    stack_or_heap_buffer<VkBuffer, 16> vkBuffers(bufCount);
    for (unsigned int i = 0; i < bufCount; ++i)
    {
        vkBuffers[i] = buffers[i].handle();
    }

    // Bind Vulkan vertex buffers.
    vkCmdBindVertexBuffers(m_vkCmdBuf, startSlot,
        bufCount, vkBuffers, offsets);
}

void cmd_list::bind_vertex_buffer(const buffer& buffer,
    VkDeviceSize offset, unsigned int startSlot)
{
    const VkBuffer vkBuffer = buffer.handle();
    vkCmdBindVertexBuffers(m_vkCmdBuf, startSlot,
        1, &vkBuffer, &offset);
}

void cmd_list::bind_index_buffer(const buffer& buffer,
    VkDeviceSize offset, VkIndexType indexType)
{
    vkCmdBindIndexBuffer(m_vkCmdBuf, buffer.handle(), offset, indexType);
}

void cmd_list::bind_shader_data(const pipeline_layout& layout,
    const shader_data* shaderData, unsigned int shaderDataCount)
{
    vkCmdBindDescriptorSets(m_vkCmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
        layout.handle(), 0, shaderDataCount,
        shaderData, 0, nullptr);
}

void cmd_list::bind_shader_data(const pipeline_layout& layout,
    const shader_data& shaderData)
{
    vkCmdBindDescriptorSets(m_vkCmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
        layout.handle(), 0, 1, &shaderData, 0, nullptr);
}

void cmd_list::draw_indexed(unsigned int firstIndex, unsigned int indexCount,
    unsigned int firstInstance, unsigned int instanceCount, int vertexOffset)
{
    vkCmdDrawIndexed(m_vkCmdBuf, indexCount, instanceCount,
        firstIndex, vertexOffset, firstInstance);
}
} // gfx
} // hr
