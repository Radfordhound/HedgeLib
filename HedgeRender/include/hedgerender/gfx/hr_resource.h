#ifndef HR_RESOURCE_H_INCLUDED
#define HR_RESOURCE_H_INCLUDED
#include "hr_gfx_internal.h"
#include <hedgelib/hl_internal.h>

struct VmaAllocator_T;
struct VmaAllocation_T;

namespace hr
{
namespace gfx
{
class res_allocator;
class render_device;

enum class memory_type
{
    /**
        @brief Memory that is only mappable on the GPU.

        Great for "immutable" resources that are written to just once
        via a copy from another resource, and for resources that are only
        ever read/written by the device (such as render graph resources).
    */
    gpu_only = 1,

    /**
        @brief Memory that is only mappable on the CPU.

        Great for "upload" resources that are uploaded from
        the CPU to a gpu_only resource on the GPU.
    */
    upload = 2,

    /**
        @brief Memory that is mappable on the CPU and the GPU.

        Great for "dynamic" resources that are written often
        by the CPU and read on the GPU.
    */
    dynamic = 3,

    /**
        @brief Memory that is cached and is mappable on the CPU.

        Great for "readback" resources that are written by the
        GPU and read by the CPU.
    */
    readback = 4,

    // TODO: Document this.
    cpu_copy = 5,

    // TODO: Document this.
    transient = 6
};

class buffer : public non_copyable
{
    VmaAllocator_T* m_vmaAllocator = nullptr;
    VkBuffer m_vkBuffer = VK_NULL_HANDLE;
    VmaAllocation_T* m_vmaAlloc = nullptr;
    std::size_t m_size = 0;

public:
    inline VkBuffer handle() const noexcept
    {
        return m_vkBuffer;
    }

    inline VmaAllocation_T* allocation() const noexcept
    {
        return m_vmaAlloc;
    }

    inline std::size_t size() const noexcept
    {
        return m_size;
    }

    HR_GFX_API void* map();

    template<typename T>
    inline T* map()
    {
        return static_cast<T*>(map());
    }

    HR_GFX_API void unmap();

    HR_GFX_API void destroy() noexcept;

    HR_GFX_API buffer& operator=(buffer&& other) noexcept;

    buffer() noexcept = default;

    HR_GFX_API buffer(res_allocator& allocator, memory_type memType,
        VkBufferUsageFlags vkBufUsage, std::size_t size,
        void** mappedData = nullptr);

    HR_GFX_API buffer(render_device& device, memory_type memType,
        VkBufferUsageFlags vkBufUsage, std::size_t size,
        void** mappedData = nullptr);

    template<typename T>
    buffer(res_allocator& allocator, memory_type memType,
        VkBufferUsageFlags vkBufUsage, std::size_t size,
        T** mappedData) :
        buffer(allocator, memType, vkBufUsage, size,
            reinterpret_cast<void**>(mappedData)) {}

    template<typename T>
    buffer(render_device& device, memory_type memType,
        VkBufferUsageFlags vkBufUsage, std::size_t size,
        T** mappedData) :
        buffer(device, memType, vkBufUsage, size,
            reinterpret_cast<void**>(mappedData)) {}

    HR_GFX_API buffer(buffer&& other) noexcept;

    inline ~buffer()
    {
        destroy();
    }
};

class image : public non_copyable
{
    VmaAllocator_T* m_vmaAllocator = nullptr;
    VkImage m_vkImage = VK_NULL_HANDLE;
    VmaAllocation_T* m_vmaAlloc = nullptr;
    std::size_t m_size = 0;
    VkImageType m_vkImageType = VK_IMAGE_TYPE_1D;
    VkFormat m_vkFormat = VK_FORMAT_UNDEFINED;
    VkExtent3D m_vkExtent = { 0, 0, 0 };
    unsigned int m_mipLevels = 0;
    unsigned int m_layerCount = 0;

public:
    inline VkImage handle() const noexcept
    {
        return m_vkImage;
    }

    inline VmaAllocation_T* allocation() const noexcept
    {
        return m_vmaAlloc;
    }

    inline std::size_t size() const noexcept
    {
        return m_size;
    }

    inline VkImageType type() const noexcept
    {
        return m_vkImageType;
    }

    inline VkFormat format() const noexcept
    {
        return m_vkFormat;
    }

    inline const VkExtent3D& extent() const noexcept
    {
        return m_vkExtent;
    }

    inline unsigned int mip_levels() const noexcept
    {
        return m_mipLevels;
    }

    inline unsigned int layer_count() const noexcept
    {
        return m_layerCount;
    }

    HR_GFX_API void destroy() noexcept;

    HR_GFX_API image& operator=(image&& other) noexcept;

    image() noexcept = default;

    HR_GFX_API image(res_allocator& allocator, memory_type memType, VkImageType vkImageType,
        VkImageUsageFlags vkImageUsage, VkFormat vkFormat, unsigned int width,
        unsigned int height, unsigned int depth, unsigned int mipLevels,
        unsigned int layerCount);

    HR_GFX_API image(render_device& device, memory_type memType, VkImageType vkImageType,
        VkImageUsageFlags vkImageUsage, VkFormat vkFormat, unsigned int width,
        unsigned int height, unsigned int depth, unsigned int mipLevels,
        unsigned int layerCount);

    HR_GFX_API image(image&& other) noexcept;

    inline ~image()
    {
        destroy();
    }
};

class image_view : public non_copyable
{
    VkDevice m_vkDevice = VK_NULL_HANDLE;
    VkImageView m_vkImageView = VK_NULL_HANDLE;

public:
    inline VkImageView handle() const noexcept
    {
        return m_vkImageView;
    }

    HR_GFX_API void destroy() noexcept;

    HR_GFX_API image_view& operator=(image_view&& other) noexcept;

    image_view() noexcept = default;

    HR_GFX_API image_view(render_device& device, image& image,
        VkImageViewType viewType);

    HR_GFX_API image_view(render_device& device, image& image);

    HR_GFX_API image_view(image_view&& other) noexcept;

    inline ~image_view()
    {
        destroy();
    }
};

class sampler_desc
{
public:
    VkFilter magFilter = VK_FILTER_LINEAR;
    VkFilter minFilter = VK_FILTER_LINEAR;
    VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    float mipLodBias = 0.0f;
    float maxAnisotropy = 1.0f;
    VkCompareOp compareOp = VK_COMPARE_OP_NEVER;
    float minLod = 0.0f;
    float maxLod = 1.0f;
    VkBorderColor borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    bool anisotropyEnable = false;
    bool compareEnable = false;
    bool unnormalizedCoordinates = false;
};

class sampler : public non_copyable
{
    VkDevice m_vkDevice = VK_NULL_HANDLE;
    VkSampler m_vkSampler = VK_NULL_HANDLE;

public:
    inline VkSampler handle() const noexcept
    {
        return m_vkSampler;
    }

    HR_GFX_API void destroy() noexcept;

    HR_GFX_API sampler& operator=(sampler&& other) noexcept;

    sampler() noexcept = default;

    HR_GFX_API sampler(render_device& device, const sampler_desc& desc);

    HR_GFX_API sampler(sampler&& other) noexcept;

    inline ~sampler()
    {
        destroy();
    }
};
} // gfx
} // hr
#endif
