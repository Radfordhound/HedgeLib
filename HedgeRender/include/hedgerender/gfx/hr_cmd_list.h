#ifndef HR_CMD_LIST_H_INCLUDED
#define HR_CMD_LIST_H_INCLUDED
#include "hr_shader.h"

namespace hr
{
namespace gfx
{
class pipeline_layout;
class pipeline;
class buffer;
class image;

class cmd_list : public non_copyable
{
    VkCommandBuffer m_vkCmdBuf = VK_NULL_HANDLE;

public:
    inline VkCommandBuffer handle() const noexcept
    {
        return m_vkCmdBuf;
    }

    HR_GFX_API void copy_buffer_to_image(const buffer& src,
        image& dst, VkImageLayout layout);

    HR_GFX_API void transition_image_layout(image& img,
        VkImageLayout oldLayout, VkImageLayout newLayout,
        VkAccessFlags oldAccess, VkAccessFlags newAccess,
        VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages);

    HR_GFX_API void push_constants(const pipeline_layout& layout,
        VkShaderStageFlags vkShaderStages, unsigned int offset,
        unsigned int count, const void* values);

    HR_GFX_API void set_viewport(float x, float y,
        float width, float height, float minDepth = 0.0f,
        float maxDepth = 1.0f);

    HR_GFX_API void set_scissor(int x, int y, unsigned int width, unsigned int height);

    HR_GFX_API void bind_pipeline(const pipeline& pipeline);

    HR_GFX_API void bind_vertex_buffers(const buffer* buffers,
        const VkDeviceSize* offsets, unsigned int startSlot,
        unsigned int bufCount);

    HR_GFX_API void bind_vertex_buffer(const buffer& buffer,
        VkDeviceSize offset, unsigned int startSlot);

    HR_GFX_API void bind_index_buffer(const buffer& buffer,
        VkDeviceSize offset, VkIndexType indexType);

    HR_GFX_API void bind_shader_data(const pipeline_layout& layout,
        const shader_data* shaderData, unsigned int shaderDataCount);

    HR_GFX_API void bind_shader_data(const pipeline_layout& layout,
        const shader_data& shaderData);

    HR_GFX_API void draw_indexed(unsigned int firstIndex, unsigned int indexCount,
        unsigned int firstInstance, unsigned int instanceCount, int vertexOffset = 0);

    inline cmd_list() noexcept = default;

    inline cmd_list(VkCommandBuffer vkCmdBuf) noexcept :
        m_vkCmdBuf(vkCmdBuf) {}
};
} // gfx
} // hr
#endif
