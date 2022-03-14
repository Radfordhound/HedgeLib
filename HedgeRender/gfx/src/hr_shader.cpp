#include "hedgerender/gfx/hr_shader.h"
#include "hedgerender/gfx/hr_render_device.h"
//#include "hr_in_shader.h"

namespace hr
{
namespace gfx
{
static const uint32_t in_vulkan_default_vertex_shader[] =
{
#include "default_vs.h"
};

const std::size_t default_vertex_shader_size = sizeof(in_vulkan_default_vertex_shader);
const void* const default_vertex_shader_code = in_vulkan_default_vertex_shader;

static const uint32_t in_vulkan_default_pixel_shader[] =
{
#include "default_ps.h"
};

const std::size_t default_pixel_shader_size = sizeof(in_vulkan_default_pixel_shader);
const void* const default_pixel_shader_code = in_vulkan_default_pixel_shader;

void shader_parameter_group::destroy() noexcept
{
    if (!m_vkDevice) return;
    vkDestroyDescriptorSetLayout(m_vkDevice, m_vkDescSetLayout, nullptr);
}

shader_parameter_group& shader_parameter_group::operator=(
    shader_parameter_group&& other) noexcept
{
    if (&other != this)
    {
        destroy();

        m_vkDevice = other.m_vkDevice;
        m_vkDescSetLayout = other.m_vkDescSetLayout;
        m_vkPushConstantRanges = std::move(other.m_vkPushConstantRanges);

        other.m_vkDevice = nullptr;
    }

    return *this;
}

static VkDescriptorSetLayout in_vulkan_create_desc_set_layout(VkDevice vkDevice,
    const shader_parameter* params, unsigned int paramCount)
{
    // Compute total immutable samplers count.
    std::size_t totalImmutableSamplerCount = 0;
    for (unsigned int i = 0; i < paramCount; ++i)
    {
        totalImmutableSamplerCount += params[i].immutableSamplerCount;
    }

    // Generate Vulkan descriptor set layout bindings.
    stack_or_heap_buffer<VkDescriptorSetLayoutBinding, 32> vkDescSetLayoutBindings(paramCount);
    stack_or_heap_buffer<VkSampler, 16> vkImmutableSamplers(totalImmutableSamplerCount);
    totalImmutableSamplerCount = 0;

    for (unsigned int i = 0; i < paramCount; ++i)
    {
        auto& vkDescSetLayoutBinding = vkDescSetLayoutBindings[i];
        auto& param = params[i];

        vkDescSetLayoutBinding.binding = param.firstRegisterIndex;
        // TODO: Ensure we have inline uniform buffer extension or Vulkan 1.3 if necessary.
        vkDescSetLayoutBinding.descriptorType = param.type;
        vkDescSetLayoutBinding.descriptorCount = param.registerCount;
        vkDescSetLayoutBinding.stageFlags = param.shaderStages;

        VkSampler* vkCurImmutableSampler = (vkImmutableSamplers + totalImmutableSamplerCount);
        vkDescSetLayoutBinding.pImmutableSamplers = vkCurImmutableSampler;

        for (unsigned int i2 = 0; i2 < param.immutableSamplerCount; ++i2)
        {
            vkCurImmutableSampler[i2] = param.immutableSamplers[i2].handle();
        }

        totalImmutableSamplerCount += param.immutableSamplerCount;
    }

    // Generate Vulkan descriptor set layout create info.
    const VkDescriptorSetLayoutCreateInfo vkDescSetLayoutCreateInfo =
    {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,            // sType
        nullptr,                                                        // pNext
        0,                                                              // flags
        paramCount,                                                     // bindingCount
        vkDescSetLayoutBindings                                         // pBindings
    };

    // Create Vulkan descriptor set layout.
    VkDescriptorSetLayout vkDescSetLayout;
    if (vkCreateDescriptorSetLayout(vkDevice, &vkDescSetLayoutCreateInfo,
        nullptr, &vkDescSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create Vulkan descriptor set layout");
    }

    return vkDescSetLayout;
}

shader_parameter_group::shader_parameter_group(render_device& device,
    const shader_parameter* params, unsigned int paramCount,
    const push_constant_range* constRanges, unsigned int constRangeCount) :

    m_vkDevice(device.handle()),
    m_vkDescSetLayout(in_vulkan_create_desc_set_layout(
        m_vkDevice, params, paramCount)),

    m_vkPushConstantRanges(constRangeCount, no_default_construct)
{
    for (uint32_t i = 0; i < m_vkPushConstantRanges.size(); ++i)
    {
        auto& vkPushConstRange = m_vkPushConstantRanges[i];
        auto& constRange = constRanges[i];

        vkPushConstRange.stageFlags = constRange.shaderStages;
        vkPushConstRange.offset = constRange.offset;
        vkPushConstRange.size = (constRange.count * sizeof(uint32_t));
    }
}

shader_parameter_group::shader_parameter_group(shader_parameter_group&& other) noexcept :
    m_vkDevice(other.m_vkDevice),
    m_vkDescSetLayout(other.m_vkDescSetLayout),
    m_vkPushConstantRanges(std::move(m_vkPushConstantRanges))
{
    other.m_vkDevice = nullptr;
}

void shader::destroy() noexcept
{
    if (!m_vkDevice) return;
    vkDestroyShaderModule(m_vkDevice, m_vkShaderModule, nullptr);
}

shader& shader::operator=(shader&& other) noexcept
{
    if (&other != this)
    {
        destroy();

        m_vkDevice = other.m_vkDevice;
        m_entryPoint = std::move(other.m_entryPoint);
        m_vkShaderModule = other.m_vkShaderModule;

        other.m_vkDevice = nullptr;
    }

    return *this;
}

static VkShaderModule in_vulkan_create_shader_module(VkDevice vkDevice,
    const void* code, std::size_t codeSize)
{
    const VkShaderModuleCreateInfo vkShaderModuleCreateInfo =
    {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,                    // sType
        nullptr,                                                        // pNext
        0,                                                              // flags
        codeSize,                                                       // codeSize
        static_cast<const uint32_t*>(code)                              // code
    };

    VkShaderModule vkShaderModule;
    if (vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo,
        nullptr, &vkShaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create Vulkan shader module");
    }

    return vkShaderModule;
}

shader::shader(render_device& device, std::string entryPoint,
    const void* code, std::size_t codeSize) :
    m_vkDevice(device.handle()),
    m_entryPoint(entryPoint),
    m_vkShaderModule(in_vulkan_create_shader_module(
        m_vkDevice, code, codeSize)) {}

shader::shader(shader&& other) noexcept :
    m_vkDevice(other.m_vkDevice),
    m_entryPoint(std::move(other.m_entryPoint)),
    m_vkShaderModule(other.m_vkShaderModule)
{
    other.m_vkDevice = nullptr;
}

void shader_data_allocator::in_steal(shader_data_allocator&& other) noexcept
{
    m_device = other.m_device;
    m_descPoolAllocator = other.m_descPoolAllocator;
    m_descPools = other.m_descPools;
    m_vkDescPool = other.m_vkDescPool;

    other.m_descPoolAllocator = nullptr;
}

void shader_data_allocator::allocate(const shader_parameter_group* paramGroups,
    std::size_t paramGroupCount, shader_data* shaderDataHandles)
{
    assert(paramGroups && shaderDataHandles && "Invalid arguments");

    // Generate Vulkan descriptor set layouts array.
    stack_or_heap_buffer<VkDescriptorSetLayout, 32>
        vkDescSetLayouts(paramGroupCount);

    for (std::size_t i = 0; i < paramGroupCount; ++i)
    {
        vkDescSetLayouts[i] = paramGroups[i].handle();
    }

    // Generate Vulkan descriptor set allocate info.
    VkDescriptorSetAllocateInfo vkDescSetAllocInfo =
    {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,                 // sType
        nullptr,                                                        // pNext
        m_vkDescPool,                                                   // descriptorPool
        static_cast<uint32_t>(paramGroupCount),                         // descriptorSetCount
        vkDescSetLayouts                                                // pSetLayouts
    };

    // Allocate Vulkan descriptor sets.
    VkResult vkResult = vkAllocateDescriptorSets(m_device->handle(),
        &vkDescSetAllocInfo, shaderDataHandles);

    // Attempt to re-allocate if the allocation failed due to running out of memory.
    while (vkResult != VK_SUCCESS)
    {
        switch (vkResult)
        {
        case VK_ERROR_FRAGMENTED_POOL:
        case VK_ERROR_OUT_OF_POOL_MEMORY:
        {
            // Add descriptor pool to full pools list.
            std::lock_guard<std::mutex> lock(m_descPoolAllocator->mutex);
            m_descPools->fullPools.push_back(m_vkDescPool);

            // Create a new allocator and replace this allocator with it.
            in_steal(std::move(m_device->get_shader_data_allocator(
                m_descPoolAllocator != &m_device->m_globalDescPoolAllocator)));

            // Attempt to re-allocate Vulkan descriptor sets.
            vkDescSetAllocInfo.descriptorPool = m_vkDescPool;
            vkResult = vkAllocateDescriptorSets(m_device->handle(),
                &vkDescSetAllocInfo, shaderDataHandles);

            break;
        }

        default:
            throw std::runtime_error("Could not allocate Vulkan descriptor sets.");
        }
    }
}

fixed_array<shader_data> shader_data_allocator::allocate(
    const shader_parameter_group* paramGroups,
    std::size_t paramGroupCount)
{
    fixed_array<shader_data> shaderDataHandles(paramGroupCount, no_default_construct);
    allocate(paramGroups, paramGroupCount, shaderDataHandles.data());
    return shaderDataHandles;
}

shader_data shader_data_allocator::allocate(const shader_parameter_group& paramGroup)
{
    shader_data shaderDataHandle;
    allocate(&paramGroup, 1, &shaderDataHandle);
    return shaderDataHandle;
}

void shader_data_allocator::destroy()
{
    if (!m_descPoolAllocator) return;
    
    // Add descriptor pool to usable pools list.
    std::lock_guard<std::mutex> lock(m_descPoolAllocator->mutex);
    m_descPools->usablePools.push_back(m_vkDescPool);
}

shader_data_allocator& shader_data_allocator::operator=(
    shader_data_allocator&& other) noexcept
{
    if (&other != this)
    {
        destroy();
        in_steal(std::move(other));
    }

    return *this;
}

shader_data_allocator::shader_data_allocator(render_device& device,
    internal::in_desc_pool_allocator& descPoolAllocator,
    internal::in_desc_pools& descPools, VkDescriptorPool vkDescPool) noexcept :
    m_device(&device),
    m_descPoolAllocator(&descPoolAllocator),
    m_descPools(&descPools),
    m_vkDescPool(vkDescPool) {}

shader_data_allocator::shader_data_allocator(shader_data_allocator&& other) noexcept :
    m_device(other.m_device),
    m_descPoolAllocator(other.m_descPoolAllocator),
    m_descPools(other.m_descPools),
    m_vkDescPool(other.m_vkDescPool)
{
    other.m_descPoolAllocator = nullptr;
}
} // gfx
} // hr
