//#include "hr_in_shader.h"
//#include "hr_in_device.h"
//#include "hr_in_format.h"
#include "hr_in_render_graph.h"
#include "hedgerender/gfx/hr_pipeline.h"
#include "hedgerender/gfx/hr_render_device.h"
#include "hedgerender/gfx/hr_shader.h"
//#include <hedgerender/base/hr_array.h>

namespace hr
{
namespace gfx
{
void pipeline_layout::destroy() noexcept
{
    if (!m_vkDevice) return;
    vkDestroyPipelineLayout(m_vkDevice, m_vkPipelineLayout, nullptr);
}

pipeline_layout& pipeline_layout::operator=(pipeline_layout&& other) noexcept
{
    if (&other != this)
    {
        destroy();

        m_vkDevice = other.m_vkDevice;
        m_vkPipelineLayout = other.m_vkPipelineLayout;

        other.m_vkDevice = VK_NULL_HANDLE;
        other.m_vkPipelineLayout = VK_NULL_HANDLE;
    }

    return *this;
}

static VkPipelineLayout in_vulkan_create_pipeline_layout(VkDevice vkDevice,
    const shader_parameter_group* paramGroups, std::size_t paramGroupCount)
{
    // Generate Vulkan descriptor set layout array.
    VkDescriptorSetLayout vkDescSetLayoutsStack[16];
    std::unique_ptr<VkDescriptorSetLayout[]> vkDescSetLayoutsHeap;
    VkDescriptorSetLayout* vkDescSetLayouts;
    uint32_t vkPushConstRangesCount = 0;

    if (paramGroupCount > 16)
    {
        vkDescSetLayoutsHeap = std::unique_ptr<VkDescriptorSetLayout[]>(
            new VkDescriptorSetLayout[paramGroupCount]);

        vkDescSetLayouts = vkDescSetLayoutsHeap.get();
    }
    else
    {
        vkDescSetLayouts = vkDescSetLayoutsStack;
    }

    // Generate Vulkan descriptor set layout array.
    for (std::size_t i = 0; i < paramGroupCount; ++i)
    {
        auto& paramGroup = paramGroups[i];
        vkDescSetLayouts[i] = paramGroup.handle();
        vkPushConstRangesCount += paramGroup.push_const_ranges().size();
    }

    // Generate Vulkan push constant ranges array.
    stack_or_heap_buffer<VkPushConstantRange, 16> vkPushConstRanges(vkPushConstRangesCount);
    vkPushConstRangesCount = 0;

    for (std::size_t i = 0; i < paramGroupCount; ++i)
    {
        auto& paramGroup = paramGroups[i];
        std::memcpy(vkPushConstRanges + vkPushConstRangesCount,
            paramGroup.push_const_ranges().data(),
            paramGroup.push_const_ranges().size() * sizeof(VkPushConstantRange));

        vkPushConstRangesCount += paramGroup.push_const_ranges().size();
    }

    // Generate Vulkan pipeline layout create info.
    const VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,                  // sType
        nullptr,                                                        // pNext
        0,                                                              // flags
        static_cast<uint32_t>(paramGroupCount),                         // setLayoutCount
        vkDescSetLayouts,                                               // pSetLayouts
        vkPushConstRangesCount,                                         // pushConstantRangeCount
        vkPushConstRanges                                               // pPushConstantRanges
    };

    // Create Vulkan pipeline layout.
    VkPipelineLayout vkPipelineLayout;
    if (vkCreatePipelineLayout(vkDevice, &vkPipelineLayoutCreateInfo,
        nullptr, &vkPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create Vulkan pipeline layout");
    }
    
    return vkPipelineLayout;
}

pipeline_layout::pipeline_layout(render_device& device,
    const shader_parameter_group* paramGroups,
    std::size_t paramGroupCount) :
    m_vkDevice(device.handle()),
    m_vkPipelineLayout(in_vulkan_create_pipeline_layout(
        m_vkDevice, paramGroups, paramGroupCount)) {}

pipeline_layout::pipeline_layout(pipeline_layout&& other) noexcept :
    m_vkDevice(other.m_vkDevice),
    m_vkPipelineLayout(other.m_vkPipelineLayout)
{
    other.m_vkDevice = VK_NULL_HANDLE;
    other.m_vkPipelineLayout = VK_NULL_HANDLE;
}

void pipeline::destroy() noexcept
{
    if (!m_vkDevice) return;
    vkDestroyPipeline(m_vkDevice, m_vkPipeline, nullptr);
}

pipeline& pipeline::operator=(pipeline&& other) noexcept
{
    if (&other != this)
    {
        destroy();

        m_vkDevice = other.m_vkDevice;
        m_vkPipeline = other.m_vkPipeline;

        other.m_vkDevice = VK_NULL_HANDLE;
        other.m_vkPipeline = VK_NULL_HANDLE;
    }

    return *this;
}

static VkBool32 in_vulkan_needs_primitive_restart(
    const VkPrimitiveTopology topologyType) noexcept
{
    switch (topologyType)
    {
    case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
    case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
        return VK_TRUE;

    default:
        return VK_FALSE;
    }
}

static VkStencilOpState in_vulkan_to_vulkan_stencil_op_state(
    const depth_stencil_desc& desc, const stencil_op_state& stencilState) noexcept
{
    const VkStencilOpState vkStencilOpState =
    {
        static_cast<VkStencilOp>(stencilState.failOp),                  // failOp
        static_cast<VkStencilOp>(stencilState.passOp),                  // passOp
        static_cast<VkStencilOp>(stencilState.depthFailOp),             // depthFailOp
        static_cast<VkCompareOp>(stencilState.compareOp),               // compareOp
        desc.stencilReadMask,                                           // compareMask
        desc.stencilWriteMask,                                          // writeMask
        0                                                               // reference
    };

    return vkStencilOpState;
}

constexpr std::size_t in_vulkan_max_shader_stages = 2;

static VkPipeline in_vulkan_create_pipeline(VkDevice vkDevice,
    const render_graph& graph, const pipeline_desc& desc)
{
    // Generate Vulkan shader stage create infos.
    VkPipelineShaderStageCreateInfo vkShaderStages[in_vulkan_max_shader_stages];
    uint32_t vkShaderStageCount = 0;

    if (desc.vertexShader)
    {
        auto& vkShaderStage = vkShaderStages[vkShaderStageCount];

        vkShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vkShaderStage.pNext = nullptr;
        vkShaderStage.flags = 0;
        vkShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vkShaderStage.module = desc.vertexShader->handle();
        vkShaderStage.pName = desc.vertexShader->entry_point().c_str();
        vkShaderStage.pSpecializationInfo = nullptr;

        ++vkShaderStageCount;
    }

    if (desc.pixelShader)
    {
        auto& vkShaderStage = vkShaderStages[vkShaderStageCount];

        vkShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vkShaderStage.pNext = nullptr;
        vkShaderStage.flags = 0;
        vkShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        vkShaderStage.module = desc.pixelShader->handle();
        vkShaderStage.pName = desc.pixelShader->entry_point().c_str();
        vkShaderStage.pSpecializationInfo = nullptr;

        ++vkShaderStageCount;
    }

    // Generate Vulkan vertex binding descriptions.
    stack_or_heap_buffer<VkVertexInputBindingDescription, 8>
        vkVtxBindingDescs(desc.inputLayoutCount);

    std::size_t curVkAttribDescIndex = 0;

    for (std::size_t i = 0; i < desc.inputLayoutCount; ++i)
    {
        auto& inputLayout = desc.inputLayouts[i];
        auto& vkVtxBindingDesc = vkVtxBindingDescs[i];

        vkVtxBindingDesc.binding = inputLayout.inputSlot;
        vkVtxBindingDesc.stride = inputLayout.stride;
        vkVtxBindingDesc.inputRate = inputLayout.inputRate;

        curVkAttribDescIndex += inputLayout.elementCount;
    }

    // Generate Vulkan vertex attribute descriptions.
    stack_or_heap_buffer<VkVertexInputAttributeDescription, 64>
        vkVtxAttribDescs(curVkAttribDescIndex);

    curVkAttribDescIndex = 0;

    for (std::size_t i = 0; i < desc.inputLayoutCount; ++i)
    {
        auto& inputLayout = desc.inputLayouts[i];
        for (std::size_t i2 = 0; i2 < inputLayout.elementCount; ++i2)
        {
            auto& inputElement = inputLayout.elements[i2];
            auto& vkVtxAttribDesc = vkVtxAttribDescs[curVkAttribDescIndex++];

            vkVtxAttribDesc.location = inputElement.index;
            vkVtxAttribDesc.binding = inputLayout.inputSlot;
            vkVtxAttribDesc.format = inputElement.format;
            vkVtxAttribDesc.offset = inputElement.offset;
        }
    }

    // Generate Vulkan vertex input state create info.
    const VkPipelineVertexInputStateCreateInfo vkVtxInputState =
    {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,      // sType
        nullptr,                                                        // pNext
        0,                                                              // flags
        static_cast<uint32_t>(desc.inputLayoutCount),                   // vertexBindingDescriptionCount
        vkVtxBindingDescs,                                              // pVertexBindingDescriptions
        static_cast<uint32_t>(curVkAttribDescIndex),                    // vertexAttributeDescriptionCount
        vkVtxAttribDescs                                                // pVertexAttributeDescriptions
    };

    // Generate Vulkan input assembly state create info.
    const VkPipelineInputAssemblyStateCreateInfo vkInputAsmStateCreateInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,    // sType
        nullptr,                                                        // pNext
        0,                                                              // flags
        desc.topologyType,                                              // topology
        in_vulkan_needs_primitive_restart(desc.topologyType)            // primitiveRestartEnable
    };

    // Generate Vulkan viewport state create info.
    const VkPipelineViewportStateCreateInfo vkViewportStateCreateInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,          // sType
        nullptr,                                                        // pNext
        0,                                                              // flags
        desc.viewportCount,                                             // viewportCount
        nullptr,                                                        // pViewports
        desc.scissorCount,                                              // scissorCount
        nullptr                                                         // pScissors
    };

    // Generate Vulkan rasterization state create info.
    const VkPipelineRasterizationStateCreateInfo vkRasterStateCreateInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,     // sType
        nullptr,                                                        // pNext
        0,                                                              // flags
        VK_FALSE,                                                       // depthClampEnable
        VK_FALSE,                                                       // rasterizerDiscardEnable
        desc.raster.fillMode,                                           // polygonMode
        desc.raster.cullMode,                                           // cullMode

        (desc.raster.frontCounterClockwise) ?                           // frontFace
            VK_FRONT_FACE_COUNTER_CLOCKWISE :
            VK_FRONT_FACE_CLOCKWISE,

        desc.raster.depthBiasEnable,                                    // depthBiasEnable
        desc.raster.depthBias,                                          // depthBiasConstantFactor
        desc.raster.depthBiasClamp,                                     // depthBiasClamp
        desc.raster.depthBiasSlope,                                     // depthBiasSlopeFactor
        1.0f                                                            // lineWidth
    };

    // Generate Vulkan multisample state create info.
    const VkPipelineMultisampleStateCreateInfo vkMultisampleStateCreateInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,       // sType
        nullptr,                                                        // pNext
        0,                                                              // flags
        VK_SAMPLE_COUNT_1_BIT,                                          // rasterizationSamples // TODO: Let user change this
        VK_FALSE,                                                       // sampleShadingEnable
        0.0f,                                                           // minSampleShading
        nullptr,                                                        // pSampleMask
        VK_FALSE,                                                       // alphaToCoverageEnable
        VK_FALSE                                                        // alphaToOneEnable
    };

    // Generate Vulkan depth stencil state create info.
    const VkPipelineDepthStencilStateCreateInfo vkDepthStencilStateCreateInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,     // sType
        nullptr,                                                        // pNext
        0,                                                              // flags
        desc.depthStencil.depthEnable,                                  // depthTestEnable
        desc.depthStencil.depthWriteEnable,                             // depthWriteEnable
        static_cast<VkCompareOp>(desc.depthStencil.depthCompareOp),     // depthCompareOp
        VK_FALSE,                                                       // depthBoundsTestEnable
        desc.depthStencil.stencilEnable,                                // stencilTestEnable

        in_vulkan_to_vulkan_stencil_op_state(desc.depthStencil,         // front
            desc.depthStencil.front),

        in_vulkan_to_vulkan_stencil_op_state(desc.depthStencil,         // back
            desc.depthStencil.back),

        0.0f,                                                           // minDepthBounds
        1.0f                                                            // maxDepthBounds
    };

    // Generate Vulkan color blend state create info.
    auto& pass = graph.handle()->passes[desc.passIndex];
    auto& subpass = graph.handle()->subpasses[pass.firstSubpassIndex + desc.subpassIndex];

    const VkPipelineColorBlendStateCreateInfo vkColorBlendStateCreateInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,       // sType
        nullptr,                                                        // pNext
        0,                                                              // flags
        desc.blend.logicOpEnable,                                       // logicOpEnable
        static_cast<VkLogicOp>(desc.blend.logicOp),                     // logicOp
        subpass.vkColorAttachBlendStates.size(),                        // attachmentCount
        subpass.vkColorAttachBlendStates.data(),                        // pAttachments
        { 0.0f, 0.0f, 0.0f, 0.0f }                                      // blendConstants // TODO: Do we need to set this?
    };

    // Generate Vulkan dynamic state create info.
    static const VkDynamicState vkDynamicStates[] =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    static const uint32_t vkDynamicStateCount = static_cast<uint32_t>(
        sizeof(vkDynamicStates) / sizeof(*vkDynamicStates));

    static const VkPipelineDynamicStateCreateInfo vkDynamicStateCreateInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,           // sType
        nullptr,                                                        // pNext
        0,                                                              // flags
        vkDynamicStateCount,                                            // dynamicStateCount
        vkDynamicStates                                                 // pDynamicStates
    };

    // Generate Vulkan graphics pipeline create info.
    const VkGraphicsPipelineCreateInfo vkPipelineCreateInfo =
    {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,                // sType
        nullptr,                                                        // pNext
        0,                                                              // flags
        vkShaderStageCount,                                             // stageCount
        vkShaderStages,                                                 // pStages
        &vkVtxInputState,                                               // pVertexInputState
        &vkInputAsmStateCreateInfo,                                     // pInputAssemblyState
        nullptr,                                                        // pTessellationState
        &vkViewportStateCreateInfo,                                     // pViewportState
        &vkRasterStateCreateInfo,                                       // pRasterizationState
        &vkMultisampleStateCreateInfo,                                  // pMultisampleState
        &vkDepthStencilStateCreateInfo,                                 // pDepthStencilState
        &vkColorBlendStateCreateInfo,                                   // pColorBlendState
        &vkDynamicStateCreateInfo,                                      // pDynamicState
        desc.layout,                                                    // layout
        pass.vkRenderPass,                                              // renderPass
        desc.subpassIndex,                                              // subpass
        VK_NULL_HANDLE,                                                 // basePipelineHandle
        -1                                                              // basePipelineIndex
    };

    // Create Vulkan graphics pipeline.
    VkPipeline vkPipeline;
    if (vkCreateGraphicsPipelines(vkDevice, VK_NULL_HANDLE, 1,
        &vkPipelineCreateInfo, nullptr, &vkPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create Vulkan graphics pipeline");
    }

    return vkPipeline;
}

pipeline::pipeline(render_device& device,
    const render_graph& graph, const pipeline_desc& desc) :
    m_vkDevice(device.handle()),
    m_vkPipeline(in_vulkan_create_pipeline(m_vkDevice, graph, desc)) {}

pipeline::pipeline(pipeline&& other) noexcept :
    m_vkDevice(other.m_vkDevice),
    m_vkPipeline(other.m_vkPipeline)
{
    other.m_vkDevice = VK_NULL_HANDLE;
    other.m_vkPipeline = VK_NULL_HANDLE;
}
} // gfx
} // hr
