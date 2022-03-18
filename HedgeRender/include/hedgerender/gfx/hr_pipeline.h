#ifndef HR_PIPELINE_H_INCLUDED
#define HR_PIPELINE_H_INCLUDED
#include "hr_gfx_internal.h"

namespace hr
{
namespace gfx
{
class render_device;
class render_graph;
class shader_parameter_group;
class shader;

class pipeline_layout : public non_copyable
{
    VkDevice m_vkDevice = VK_NULL_HANDLE;
    VkPipelineLayout m_vkPipelineLayout = VK_NULL_HANDLE;

public:
    inline VkPipelineLayout handle() const noexcept
    {
        return m_vkPipelineLayout;
    }

    HR_GFX_API void destroy() noexcept;

    HR_GFX_API pipeline_layout& operator=(pipeline_layout&& other) noexcept;

    pipeline_layout() noexcept = default;

    HR_GFX_API pipeline_layout(render_device& device,
        const shader_parameter_group* paramGroups,
        std::size_t paramGroupCount);

    HR_GFX_API pipeline_layout(pipeline_layout&& other) noexcept;

    inline ~pipeline_layout()
    {
        destroy();
    }
};

class input_element
{
public:
    VkFormat format;
    unsigned int index;
    unsigned int offset;
};

class input_layout
{
public:
    const input_element* elements;
    std::size_t elementCount;
    unsigned int stride;
    unsigned int inputSlot;
    VkVertexInputRate inputRate;
};

class rasterizer_desc
{
public:
    VkPolygonMode fillMode = VK_POLYGON_MODE_FILL;
    VkCullModeFlags cullMode = VK_CULL_MODE_NONE;
    bool frontCounterClockwise = false;
    bool depthBiasEnable = false;
    float depthBias = 0.0f;
    float depthBiasClamp = 0.0f;
    float depthBiasSlope = 0.0f;
};

class stencil_op_state
{
public:
    VkStencilOp failOp = VK_STENCIL_OP_KEEP;
    VkStencilOp depthFailOp = VK_STENCIL_OP_KEEP;
    VkStencilOp passOp = VK_STENCIL_OP_KEEP;
    VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS;
};

class depth_stencil_desc
{
public:
    bool depthEnable = true;
    bool depthWriteEnable = true;
    bool stencilEnable = true;
    VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS;
    unsigned int stencilReadMask = UINT_MAX;
    unsigned int stencilWriteMask = UINT_MAX;
    stencil_op_state front;
    stencil_op_state back;
};

class blend_desc
{
public:
    bool logicOpEnable = false;
    VkLogicOp logicOp = VK_LOGIC_OP_NO_OP;
};

class pipeline_desc
{
public:
    VkPipelineLayout layout = VK_NULL_HANDLE;
    shader* vertexShader = nullptr;
    shader* pixelShader = nullptr;
    const input_layout* inputLayouts = nullptr;
    std::size_t inputLayoutCount = 0;
    VkPrimitiveTopology topologyType = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    unsigned int viewportCount = 1;
    unsigned int scissorCount = 1;
    rasterizer_desc raster;
    depth_stencil_desc depthStencil;
    blend_desc blend;
    unsigned int passIndex = 0;
    unsigned int subpassIndex = 0;
};

class pipeline : public non_copyable
{
    VkDevice m_vkDevice = VK_NULL_HANDLE;
    VkPipeline m_vkPipeline = VK_NULL_HANDLE;

public:
    inline VkPipeline handle() const noexcept
    {
        return m_vkPipeline;
    }

    HR_GFX_API void destroy() noexcept;

    HR_GFX_API pipeline& operator=(pipeline&& other) noexcept;

    pipeline() noexcept = default;

    HR_GFX_API pipeline(render_device& device,
        const render_graph& graph, const pipeline_desc& desc);

    HR_GFX_API pipeline(pipeline&& other) noexcept;

    inline ~pipeline()
    {
        destroy();
    }
};
} // gfx
} // hr
#endif
