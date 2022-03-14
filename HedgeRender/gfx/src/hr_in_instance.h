#ifndef HR_IN_INSTANCE_H_INCLUDED
#define HR_IN_INSTANCE_H_INCLUDED
namespace hr
{
namespace gfx
{
constexpr uint32_t in_vulkan_engine_version = VK_MAKE_VERSION(1, 0, 0);
constexpr uint32_t in_vulkan_api_version = VK_API_VERSION_1_2;

extern const char* const in_vulkan_validation_layers[];
extern const uint32_t in_vulkan_validation_layer_count;
} // gfx
} // hr
#endif
