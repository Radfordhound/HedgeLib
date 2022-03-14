#ifndef HR_ADAPTER_H_INCLUDED
#define HR_ADAPTER_H_INCLUDED
#include "hr_gfx_internal.h"
#include <array>

namespace hr
{
namespace gfx
{
class instance;

namespace internal
{
enum in_queue_type
{
    // HACK: In in_swap_chain::create, we rely on the first
    // type being graphics and the second being present.
    HR_IN_QUEUE_TYPE_GRAPHICS,
    HR_IN_QUEUE_TYPE_PRESENT,
    HR_IN_QUEUE_TYPE_TRANSFER,
    HR_IN_QUEUE_TYPE_COUNT
};

struct in_queue_families
{
    std::array<uint32_t, HR_IN_QUEUE_TYPE_COUNT> familyIndices;
    unsigned int uniqueFamilies = 0;

    inline uint32_t graphics_family() const noexcept
    {
        return familyIndices[HR_IN_QUEUE_TYPE_GRAPHICS];
    }

    inline uint32_t present_family() const noexcept
    {
        return familyIndices[HR_IN_QUEUE_TYPE_PRESENT];
    }

    inline uint32_t transfer_family() const noexcept
    {
        return familyIndices[HR_IN_QUEUE_TYPE_TRANSFER];
    }

    inline bool has_unique_family(in_queue_type type) const noexcept
    {
        return (uniqueFamilies & (1 << type));
    }

    void set_family(in_queue_type type,
        uint32_t vkQueueFamilyIndex, bool isUnique) noexcept
    {
        familyIndices[type] = vkQueueFamilyIndex;

        if (isUnique)
        {
            uniqueFamilies |= (1 << type);
        }
    }
};
} // internal

class adapter
{
    instance* m_instance;
    VkPhysicalDevice m_vkPhyDev;
    internal::in_queue_families m_queueFamilies;

public:
    inline const instance& parent() const noexcept
    {
        return *m_instance;
    }

    inline instance& parent() noexcept
    {
        return *m_instance;
    }

    inline VkPhysicalDevice handle() const noexcept
    {
        return m_vkPhyDev;
    }

    inline const internal::in_queue_families& queue_families() const noexcept
    {
        return m_queueFamilies;
    }

    inline adapter(instance& instance, VkPhysicalDevice vkPhyDev,
        const internal::in_queue_families& queueFamilies) noexcept :
        m_instance(&instance),
        m_vkPhyDev(vkPhyDev),
        m_queueFamilies(queueFamilies) {}
};
} // gfx
} // hr
#endif
