#ifndef HR_IN_RESOURCE_H_INCLUDED
#define HR_IN_RESOURCE_H_INCLUDED

namespace hr
{
namespace gfx
{
enum class in_vulkan_format_type
{
    standard = 0,
    block_compressed,
    packed
};

struct in_vulkan_format_info
{
    in_vulkan_format_type type;
    union
    {
        std::size_t elemSize;
        std::size_t blockSize;
        std::size_t bitsPerPixel;
    };

    in_vulkan_format_info(VkFormat vkFormat);

    std::size_t get_bytes_per_mip_level(const VkExtent3D& vkExtent) const noexcept;
};

void in_vulkan_get_next_mip_extent(VkExtent3D& vkExtent) noexcept;
} // gfx
} // hr
#endif
