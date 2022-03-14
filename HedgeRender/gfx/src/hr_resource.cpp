#include "hr_in_resource.h"
#include "hedgerender/gfx/hr_resource.h"
#include "hedgerender/gfx/hr_render_device.h"

namespace hr
{
namespace gfx
{
in_vulkan_format_info::in_vulkan_format_info(VkFormat vkFormat)
{
    switch (vkFormat)
    {
    case VK_FORMAT_R8_UNORM:
    case VK_FORMAT_R8_SNORM:
    case VK_FORMAT_R8_UINT:
    case VK_FORMAT_R8_SINT:
        type = in_vulkan_format_type::standard;
        bitsPerPixel = 8;
        break;

    case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
        type = in_vulkan_format_type::standard;
        bitsPerPixel = 12;
        break;

    case VK_FORMAT_R5G6B5_UNORM_PACK16:
    case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
    case VK_FORMAT_R8G8_UNORM:
    case VK_FORMAT_R8G8_SNORM:
    case VK_FORMAT_R8G8_UINT:
    case VK_FORMAT_R8G8_SINT:
    case VK_FORMAT_R16_UNORM:
    case VK_FORMAT_R16_SNORM:
    case VK_FORMAT_R16_UINT:
    case VK_FORMAT_R16_SINT:
    case VK_FORMAT_R16_SFLOAT:
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
        type = in_vulkan_format_type::standard;
        bitsPerPixel = 16;
        break;

    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
    case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
        type = in_vulkan_format_type::standard;
        bitsPerPixel = 24;
        break;

    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_SNORM:
    case VK_FORMAT_R8G8B8A8_UINT:
    case VK_FORMAT_R8G8B8A8_SINT:
    case VK_FORMAT_R8G8B8A8_SRGB:
    case VK_FORMAT_B8G8R8A8_UNORM:
    case VK_FORMAT_B8G8R8A8_SRGB:
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_UINT_PACK32:
    case VK_FORMAT_R16G16_UNORM:
    case VK_FORMAT_R16G16_SNORM:
    case VK_FORMAT_R16G16_UINT:
    case VK_FORMAT_R16G16_SINT:
    case VK_FORMAT_R16G16_SFLOAT:
    case VK_FORMAT_R32_UINT:
    case VK_FORMAT_R32_SINT:
    case VK_FORMAT_R32_SFLOAT:
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
        type = in_vulkan_format_type::standard;
        bitsPerPixel = 32;
        break;

    case VK_FORMAT_R16G16B16A16_UNORM:
    case VK_FORMAT_R16G16B16A16_SNORM:
    case VK_FORMAT_R16G16B16A16_UINT:
    case VK_FORMAT_R16G16B16A16_SINT:
    case VK_FORMAT_R16G16B16A16_SFLOAT:
    case VK_FORMAT_R32G32_UINT:
    case VK_FORMAT_R32G32_SINT:
    case VK_FORMAT_R32G32_SFLOAT:
        type = in_vulkan_format_type::standard;
        bitsPerPixel = 64;
        break;

    case VK_FORMAT_R32G32B32_UINT:
    case VK_FORMAT_R32G32B32_SINT:
    case VK_FORMAT_R32G32B32_SFLOAT:
        type = in_vulkan_format_type::standard;
        bitsPerPixel = 96;
        break;

    case VK_FORMAT_R32G32B32A32_UINT:
    case VK_FORMAT_R32G32B32A32_SINT:
    case VK_FORMAT_R32G32B32A32_SFLOAT:
        type = in_vulkan_format_type::standard;
        bitsPerPixel = 128;
        break;

    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
    case VK_FORMAT_BC4_UNORM_BLOCK:
    case VK_FORMAT_BC4_SNORM_BLOCK:
        type = in_vulkan_format_type::block_compressed;
        blockSize = 8;
        break;

    case VK_FORMAT_BC2_UNORM_BLOCK:
    case VK_FORMAT_BC2_SRGB_BLOCK:
    case VK_FORMAT_BC3_UNORM_BLOCK:
    case VK_FORMAT_BC3_SRGB_BLOCK:
    case VK_FORMAT_BC5_UNORM_BLOCK:
    case VK_FORMAT_BC5_SNORM_BLOCK:
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:
    case VK_FORMAT_BC6H_SFLOAT_BLOCK:
    case VK_FORMAT_BC7_UNORM_BLOCK:
    case VK_FORMAT_BC7_SRGB_BLOCK:
        type = in_vulkan_format_type::block_compressed;
        blockSize = 16;
        break;

    case VK_FORMAT_G8B8G8R8_422_UNORM:
    case VK_FORMAT_B8G8R8G8_422_UNORM:
        type = in_vulkan_format_type::packed;
        elemSize = 4;
        break;

    case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
    case VK_FORMAT_G16B16G16R16_422_UNORM:
        type = in_vulkan_format_type::packed;
        elemSize = 8;
        break;

    default:
        throw std::runtime_error("Unknown or unsupported image format");
    }
}

std::size_t in_vulkan_format_info::get_bytes_per_mip_level(
    const VkExtent3D& vkExtent) const noexcept
{
    // Compute size for block compressed data.
    if (type == in_vulkan_format_type::block_compressed)
    {
        const std::size_t numBlocksWide = std::max<std::size_t>(
            1, ((static_cast<std::size_t>(vkExtent.width) + 3) / 4));

        const std::size_t numBlocksHigh = std::max<std::size_t>(
            1, ((static_cast<std::size_t>(vkExtent.height) + 3) / 4));

        return (((numBlocksWide * numBlocksHigh) * blockSize) * vkExtent.depth);
    }

    // Compute size for packed data.
    else if (type == in_vulkan_format_type::packed)
    {
        const std::size_t bytesPerRow = ((static_cast<std::size_t>(
            vkExtent.width) + 1) >> 1) * elemSize;

        return ((bytesPerRow * vkExtent.height) * vkExtent.depth);
    }

    // Compute size for standard data.
    else
    {
        const std::size_t bytesPerRow = ((vkExtent.width *
            bitsPerPixel) + 7) / 8;

        return ((bytesPerRow * vkExtent.height) * vkExtent.depth);
    }
}

void in_vulkan_get_next_mip_extent(VkExtent3D& vkExtent) noexcept
{
    vkExtent.width /= 2;
    vkExtent.height /= 2;
    vkExtent.depth /= 2;

    if (vkExtent.width == 0) vkExtent.width = 1;
    if (vkExtent.height == 0) vkExtent.height = 1;
    if (vkExtent.depth == 0) vkExtent.depth = 1;
}

static VmaMemoryUsage in_vulkan_to_vma_memory_usage(
    const memory_type memType) noexcept
{
    return (memType == memory_type::transient) ?
        VMA_MEMORY_USAGE_GPU_ONLY :
        static_cast<VmaMemoryUsage>(memType);
}

static VkMemoryPropertyFlags in_vulkan_get_memory_prop_flags(
    const memory_type memType) noexcept
{
    return (memType == memory_type::transient) ?
        VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT : 0;
}

void* buffer::map()
{
    void* mappedData;
    if (vmaMapMemory(m_vmaAllocator, m_vmaAlloc, &mappedData) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to map Vulkan buffer");
    }

    return mappedData;
}

void buffer::unmap()
{
    vmaUnmapMemory(m_vmaAllocator, m_vmaAlloc);
}

void buffer::destroy() noexcept
{
    if (!m_vmaAllocator) return;
    vmaDestroyBuffer(m_vmaAllocator, m_vkBuffer, m_vmaAlloc);
}

buffer& buffer::operator=(buffer&& other) noexcept
{
    if (&other != this)
    {
        destroy();

        m_vmaAllocator = other.m_vmaAllocator;
        m_vkBuffer = other.m_vkBuffer;
        m_vmaAlloc = other.m_vmaAlloc;

        other.m_vmaAllocator = nullptr;
    }

    return *this;
}

buffer::buffer(res_allocator& allocator, memory_type memType,
    VkBufferUsageFlags vkBufUsage, std::size_t size,
    void** mappedData) :
    m_vmaAllocator(allocator.handle()),
    m_size(size)
{
    // Generate Vulkan buffer create info.
    const VkBufferCreateInfo vkBufCreateInfo =
    {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,                           // sType
        nullptr,                                                        // pNext
        0,                                                              // flags
        size,                                                           // size
        vkBufUsage,                                                     // usage
        VK_SHARING_MODE_EXCLUSIVE,                                      // sharingMode
        0,                                                              // queueFamilyIndexCount
        nullptr                                                         // pQueueFamilyIndices
    };
    
    // Generate VMA allocation create info.
    const VmaAllocationCreateInfo vmaAllocCreateInfo =
    {
        static_cast<VmaAllocationCreateFlags>((mappedData) ?            // flags
            VMA_ALLOCATION_CREATE_MAPPED_BIT : 0),
    
        in_vulkan_to_vma_memory_usage(memType),                         // usage
        0,                                                              // requiredFlags
        in_vulkan_get_memory_prop_flags(memType),                       // preferredFlags
        0,                                                              // memoryTypeBits
        nullptr,                                                        // pool
        nullptr,                                                        // pUserData
        0                                                               // priority
    };
    
    // Create Vulkan buffer using VMA.
    VmaAllocationInfo vmaAllocInfo;
    if (vmaCreateBuffer(m_vmaAllocator, &vkBufCreateInfo,
        &vmaAllocCreateInfo, &m_vkBuffer, &m_vmaAlloc,
        &vmaAllocInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create Vulkan buffer");
    }
    
    // Return mapped data pointer if requested.
    if (mappedData)
    {
        *mappedData = vmaAllocInfo.pMappedData;
    }
}

buffer::buffer(render_device& device, memory_type memType,
    VkBufferUsageFlags vkBufUsage, std::size_t size,
    void** mappedData) :
    buffer(device.allocator(), memType, vkBufUsage, size, mappedData) {}

buffer::buffer(buffer&& other) noexcept :
    m_vmaAllocator(other.m_vmaAllocator),
    m_vkBuffer(other.m_vkBuffer),
    m_vmaAlloc(other.m_vmaAlloc)
{
    other.m_vmaAllocator = nullptr;
}

void image::destroy() noexcept
{
    if (!m_vmaAllocator) return;
    vmaDestroyImage(m_vmaAllocator, m_vkImage, m_vmaAlloc);
}

image& image::operator=(image&& other) noexcept
{
    if (&other != this)
    {
        destroy();

        m_vmaAllocator = other.m_vmaAllocator;
        m_vkImage = other.m_vkImage;
        m_vmaAlloc = other.m_vmaAlloc;
        m_size = other.m_size;
        m_vkImageType = other.m_vkImageType;
        m_vkFormat = other.m_vkFormat;
        m_vkExtent = std::move(other.m_vkExtent);
        m_mipLevels = other.m_mipLevels;
        m_layerCount = other.m_layerCount;

        other.m_vmaAllocator = nullptr;
    }

    return *this;
}

static std::size_t in_vulkan_compute_image_size(VkFormat vkFormat,
    unsigned int width, unsigned int height, unsigned int depth,
    unsigned int mipLevels, unsigned int layerCount)
{
    // Validate arguments.
    assert(width > 0 && "Image width must be greater than 0");
    assert(height > 0 && "Image height must be greater than 0");
    assert(depth > 0 && "Image depth must be greater than 0");
    assert(mipLevels > 0 && "Image mip levels must be greater than 0");
    assert(layerCount > 0 && "Image layer count must be greater than 0");

    // Get format info.
    const in_vulkan_format_info fmtInfo(vkFormat);

    // Compute image size.
    std::size_t imageSize = 0;
    while (layerCount--)
    {
        unsigned int layerMipLevels = mipLevels;
        VkExtent3D vkExtent = { width, height, depth };

        while (layerMipLevels--)
        {
            imageSize += fmtInfo.get_bytes_per_mip_level(vkExtent);
            in_vulkan_get_next_mip_extent(vkExtent);
        }
    }
    
    return imageSize;
}

static VkImageCreateFlags in_vulkan_get_image_create_flags(
    VkImageType vkImageType, unsigned int layerCount) noexcept
{
    VkImageCreateFlags vkImageCreateFlags = 0;
    if (vkImageType == VK_IMAGE_TYPE_2D)
    {
        /*if (layerCount > 1)
        {
            vkImageCreateFlags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
        }*/

        if (layerCount >= 6)
        {
            vkImageCreateFlags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        }
    }

    return vkImageCreateFlags;
}

image::image(res_allocator& allocator, memory_type memType, VkImageType vkImageType,
    VkImageUsageFlags vkImageUsage, VkFormat vkFormat, unsigned int width,
    unsigned int height, unsigned int depth, unsigned int mipLevels,
    unsigned int layerCount) :

    m_vmaAllocator(allocator.handle()),
    m_size(in_vulkan_compute_image_size(vkFormat, width,
        height, depth, mipLevels, layerCount)),

    m_vkImageType(vkImageType),
    m_vkFormat(vkFormat),
    m_vkExtent({ width, height, depth }),
    m_mipLevels(mipLevels),
    m_layerCount(layerCount)
{
    /*const uint32_t vkQueueFamilyIndices[] =
    {
        inDevice.vkQueueFamilies.graphics_family(),
        inDevice.vkQueueFamilies.copy_family()
    };*/

    // Generate Vulkan image create info.
    const VkImageCreateInfo vkImageCreateInfo =
    {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,                            // sType
        nullptr,                                                        // pNext
        in_vulkan_get_image_create_flags(vkImageType, layerCount),      // flags
        vkImageType,                                                    // imageType
        vkFormat,                                                       // format
        
        {                                                               // extent
            static_cast<uint32_t>(width),                               //  width
            static_cast<uint32_t>(height),                              //  height
            static_cast<uint32_t>(depth)                                //  depth
        },
        
        static_cast<uint32_t>(mipLevels),                               // mipLevels
        static_cast<uint32_t>(layerCount),                              // arrayLayers
        VK_SAMPLE_COUNT_1_BIT,                                          // samples
        VK_IMAGE_TILING_OPTIMAL,                                        // tiling
        vkImageUsage,                                                   // usage
        VK_SHARING_MODE_EXCLUSIVE,                                      // sharingMode
        0,                                                              // queueFamilyIndexCount
        nullptr,                                                        // pQueueFamilyIndices
        VK_IMAGE_LAYOUT_UNDEFINED                                       // initialLayout
    };

    // Generate VMA allocation create info.
    const VmaAllocationCreateInfo vmaAllocCreateInfo =
    {
        0,                                                              // flags
        in_vulkan_to_vma_memory_usage(memType),                         // usage
        0,                                                              // requiredFlags
        in_vulkan_get_memory_prop_flags(memType),                       // preferredFlags
        0,                                                              // memoryTypeBits
        nullptr,                                                        // pool
        nullptr,                                                        // pUserData
        0                                                               // priority
    };

    // Create Vulkan image using VMA.
    if (vmaCreateImage(m_vmaAllocator, &vkImageCreateInfo,
        &vmaAllocCreateInfo, &m_vkImage, &m_vmaAlloc,
        nullptr) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create Vulkan image");
    }
}

image::image(render_device& device, memory_type memType, VkImageType vkImageType,
    VkImageUsageFlags vkImageUsage, VkFormat vkFormat, unsigned int width,
    unsigned int height, unsigned int depth, unsigned int mipLevels,
    unsigned int layerCount) :
    image(device.allocator(), memType, vkImageType, vkImageUsage,
        vkFormat, width, height, depth, mipLevels, layerCount) {}

image::image(image&& other) noexcept :
    m_vmaAllocator(other.m_vmaAllocator),
    m_vkImage(other.m_vkImage),
    m_vmaAlloc(other.m_vmaAlloc),
    m_size(other.m_size),
    m_vkImageType(other.m_vkImageType),
    m_vkFormat(other.m_vkFormat),
    m_vkExtent(std::move(other.m_vkExtent)),
    m_mipLevels(other.m_mipLevels),
    m_layerCount(other.m_layerCount)
{
    other.m_vmaAllocator = nullptr;
}

void image_view::destroy() noexcept
{
    if (!m_vkDevice) return;
    vkDestroyImageView(m_vkDevice, m_vkImageView, nullptr);
}

image_view& image_view::operator=(image_view&& other) noexcept
{
    if (&other != this)
    {
        destroy();

        m_vkDevice = other.m_vkDevice;
        m_vkImageView = other.m_vkImageView;

        other.m_vkDevice = nullptr;
    }
    
    return *this;
}

static VkImageView in_vulkan_create_image_view(VkDevice vkDevice,
    image& image, VkImageViewType viewType)
{
    const VkImageViewCreateInfo vkImageViewCreateInfo =
    {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,                       // sType
        nullptr,                                                        // pNext
        0,                                                              // flags
        image.handle(),                                                 // image
        viewType,                                                       // viewType
        image.format(),                                                 // format

        {                                                               // components
            VK_COMPONENT_SWIZZLE_IDENTITY,                              //  r
            VK_COMPONENT_SWIZZLE_IDENTITY,                              //  g
            VK_COMPONENT_SWIZZLE_IDENTITY,                              //  b
            VK_COMPONENT_SWIZZLE_IDENTITY                               //  a
        },

        {                                                               // subresourceRange
            VK_IMAGE_ASPECT_COLOR_BIT,                                  //  aspectMask
            0,                                                          //  baseMipLevel
            image.mip_levels(),                                         //  levelCount
            0,                                                          //  baseArrayLayer
            image.layer_count()                                         //  layerCount
        }
    };

    VkImageView vkImageView;
    if (vkCreateImageView(vkDevice, &vkImageViewCreateInfo,
        nullptr, &vkImageView) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create Vulkan image view");
    }

    return vkImageView;
}

image_view::image_view(render_device& device, image& image, VkImageViewType viewType) :
    m_vkDevice(device.handle()),
    m_vkImageView(in_vulkan_create_image_view(m_vkDevice, image, viewType)) {}

static VkImageViewType in_vulkan_get_image_view_type(const image& image) noexcept
{
    // TODO: Is all of this correct?
    // TODO: What about VK_IMAGE_VIEW_TYPE_CUBE_ARRAY?

    switch (image.type())
    {
    case VK_IMAGE_TYPE_1D:
        return (image.layer_count() > 1) ?
            VK_IMAGE_VIEW_TYPE_1D_ARRAY :
            VK_IMAGE_VIEW_TYPE_1D;

    default:
    case VK_IMAGE_TYPE_2D:
        if (image.layer_count() >= 6)
        {
            return VK_IMAGE_VIEW_TYPE_CUBE;
        }
        else if (image.layer_count() > 1)
        {
            return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        }

        return VK_IMAGE_VIEW_TYPE_2D;

    case VK_IMAGE_TYPE_3D:
        return VK_IMAGE_VIEW_TYPE_3D;
    }
}

image_view::image_view(render_device& device, image& image) :
    m_vkDevice(device.handle()),
    m_vkImageView(in_vulkan_create_image_view(m_vkDevice,
        image, in_vulkan_get_image_view_type(image))) {}

image_view::image_view(image_view&& other) noexcept :
    m_vkDevice(other.m_vkDevice),
    m_vkImageView(other.m_vkImageView)
{
    other.m_vkDevice = nullptr;
}

void sampler::destroy() noexcept
{
    if (!m_vkDevice) return;
    vkDestroySampler(m_vkDevice, m_vkSampler, nullptr);
}

sampler& sampler::operator=(sampler&& other) noexcept
{
    if (&other != this)
    {
        destroy();

        m_vkDevice = other.m_vkDevice;
        m_vkSampler = other.m_vkSampler;

        other.m_vkDevice = nullptr;
    }

    return *this;
}

static VkSampler in_vulkan_create_sampler(VkDevice vkDevice, const sampler_desc& desc)
{
    // Generate Vulkan sampler create info.
    const VkSamplerCreateInfo vkSamplerCreateInfo =
    {
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,                          // sType
        nullptr,                                                        // pNext
        0,                                                              // flags
        desc.magFilter,                                                 // magFilter
        desc.minFilter,                                                 // minFilter
        desc.mipmapMode,                                                // mipmapMode
        desc.addressModeU,                                              // addressModeU
        desc.addressModeV,                                              // addressModeV
        desc.addressModeW,                                              // addressModeW
        desc.mipLodBias,                                                // mipLodBias
        (desc.anisotropyEnable) ? VK_TRUE : VK_FALSE,                   // anisotropyEnable
        desc.maxAnisotropy,                                             // maxAnisotropy
        (desc.compareEnable) ? VK_TRUE : VK_FALSE,                      // compareEnable
        desc.compareOp,                                                 // compareOp
        desc.minLod,                                                    // minLod
        desc.maxLod,                                                    // maxLod
        desc.borderColor,                                               // borderColor
        (desc.unnormalizedCoordinates) ? VK_TRUE : VK_FALSE             // unnormalizedCoordinates
    };

    // Create Vulkan sampler.
    VkSampler vkSampler;
    if (vkCreateSampler(vkDevice, &vkSamplerCreateInfo, nullptr, &vkSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create Vulkan sampler");
    }

    return vkSampler;
}

sampler::sampler(render_device& device, const sampler_desc& desc) :
    m_vkDevice(device.handle()),
    m_vkSampler(in_vulkan_create_sampler(m_vkDevice, desc)) {}

sampler::sampler(sampler&& other) noexcept :
    m_vkDevice(other.m_vkDevice),
    m_vkSampler(other.m_vkSampler)
{
    other.m_vkDevice = nullptr;
}
} // gfx
} // hr
