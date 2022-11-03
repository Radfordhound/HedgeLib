#ifndef HR_SHADER_H_INCLUDED
#define HR_SHADER_H_INCLUDED
#include "hr_gfx_internal.h"
#include <hedgelib/hl_array.h>
#include <hedgelib/hl_blob.h>

namespace hr
{
namespace gfx
{
namespace internal
{
struct in_desc_pool_allocator;
struct in_desc_pools;
} // internal

class render_device;
class sampler;

HR_GFX_API extern const std::size_t default_vertex_shader_size;
HR_GFX_API extern const void* const default_vertex_shader_code;

HR_GFX_API extern const std::size_t default_pixel_shader_size;
HR_GFX_API extern const void* const default_pixel_shader_code;

class shader_parameter
{
public:
    VkDescriptorType type;
    VkShaderStageFlags shaderStages = 0;
    unsigned int firstRegisterIndex = 0;
    unsigned int registerCount = 1;
    const sampler* immutableSamplers = nullptr;
    unsigned int immutableSamplerCount = 0;
};

class push_constant_range
{
public:
    VkShaderStageFlags shaderStages = 0;
    unsigned int offset = 0;
    unsigned int count = 0;
};

class shader_parameter_group : public non_copyable
{
    VkDevice m_vkDevice = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_vkDescSetLayout = VK_NULL_HANDLE;
    hl::fixed_array<VkPushConstantRange, uint32_t> m_vkPushConstantRanges;

public:
    inline VkDescriptorSetLayout handle() const noexcept
    {
        return m_vkDescSetLayout;
    }

    inline const hl::fixed_array<VkPushConstantRange, uint32_t>&
        push_const_ranges() const noexcept
    {
        return m_vkPushConstantRanges;
    }

    HR_GFX_API void destroy() noexcept;

    HR_GFX_API shader_parameter_group& operator=(shader_parameter_group&& other) noexcept;

    shader_parameter_group() noexcept = default;

    HR_GFX_API shader_parameter_group(render_device& device,
        const shader_parameter* params, unsigned int paramCount,
        const push_constant_range* constRanges, unsigned int constRangeCount);

    inline shader_parameter_group(render_device& device,
        const shader_parameter* params, unsigned int paramCount) :
        shader_parameter_group(device, params, paramCount, nullptr, 0) {}

    HR_GFX_API shader_parameter_group(shader_parameter_group&& other) noexcept;

    inline ~shader_parameter_group()
    {
        destroy();
    }
};

class shader : public non_copyable
{
    VkDevice m_vkDevice = VK_NULL_HANDLE;
    std::string m_entryPoint;
    VkShaderModule m_vkShaderModule = VK_NULL_HANDLE;

public:
    inline VkShaderModule handle() const noexcept
    {
        return m_vkShaderModule;
    }

    inline const std::string& entry_point() const noexcept
    {
        return m_entryPoint;
    }

    HR_GFX_API void destroy() noexcept;

    HR_GFX_API shader& operator=(shader&& other) noexcept;

    shader() noexcept = default;

    shader(VkDevice vkDevice, std::string entryPoint,
        VkShaderModule vkShaderModule) :
        m_vkDevice(vkDevice),
        m_entryPoint(entryPoint),
        m_vkShaderModule(vkShaderModule) {}

    HR_GFX_API shader(render_device& device, std::string entryPoint,
        const void* code, std::size_t codeSize, const char* debugName = nullptr);

    inline shader(render_device& device, std::string entryPoint,
        const void* code, std::size_t codeSize, const std::string& debugName) :
        shader(device, std::move(entryPoint), code, codeSize, debugName.c_str()) {}

    shader(render_device& device, std::string entryPoint,
        const hl::blob& code, const char* debugName = nullptr) :
        shader(device, std::move(entryPoint), code.data(),
            code.size(), debugName) {}

    shader(render_device& device, std::string entryPoint,
        const hl::blob& code, const std::string& debugName) :
        shader(device, std::move(entryPoint), code.data(),
            code.size(), debugName.c_str()) {}

    HR_GFX_API shader(shader&& other) noexcept;

    inline ~shader()
    {
        destroy();
    }
};

using shader_data = VkDescriptorSet;

class shader_data_allocator : public non_copyable
{
    render_device* m_device = nullptr;
    internal::in_desc_pool_allocator* m_descPoolAllocator = nullptr;
    internal::in_desc_pools* m_descPools = nullptr;
    VkDescriptorPool m_vkDescPool = VK_NULL_HANDLE;

    HR_GFX_API void in_steal(shader_data_allocator&& other) noexcept;

public:
    inline render_device& device() const noexcept
    {
        return *m_device;
    }

    inline VkDescriptorPool handle() const noexcept
    {
        return m_vkDescPool;
    }

    HR_GFX_API void allocate(const shader_parameter_group* paramGroups,
        std::size_t paramGroupCount, shader_data* shaderDataHandles);

    HR_GFX_API hl::fixed_array<shader_data> allocate(
        const shader_parameter_group* paramGroups,
        std::size_t paramGroupCount);

    HR_GFX_API shader_data allocate(const shader_parameter_group& paramGroup);

    HR_GFX_API void destroy();

    HR_GFX_API shader_data_allocator& operator=(shader_data_allocator&& other) noexcept;

    shader_data_allocator() noexcept = default;

    HR_GFX_API shader_data_allocator(render_device& device,
        internal::in_desc_pool_allocator& descPoolAllocator,
        internal::in_desc_pools& descPools, VkDescriptorPool vkDescPool) noexcept;

    HR_GFX_API shader_data_allocator(shader_data_allocator&& other) noexcept;

    inline ~shader_data_allocator()
    {
        destroy();
    }
};

class image_write_desc
{
public:
    VkSampler vkSampler;
    VkImageView vkImageView;
    VkImageLayout imageLayout;
};

class buffer_write_desc
{
public:
    VkBuffer vkBuffer;
    std::size_t offset;
    std::size_t size;
};

class shader_data_write_desc
{
public:
    shader_data shaderData;
    VkDescriptorType type;
    unsigned int firstRegisterIndex = 0;
    unsigned int arrayElementIndex = 0;
    unsigned int registerCount = 1;
    const image_write_desc* imageWrites = nullptr;
    const buffer_write_desc* bufferWrites = nullptr;
};
} // gfx
} // hr
#endif
