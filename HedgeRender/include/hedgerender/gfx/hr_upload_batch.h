#ifndef HR_UPLOAD_BATCH_H_INCLUDED
#define HR_UPLOAD_BATCH_H_INCLUDED
#include "hr_cmd_list.h"

namespace hr
{
namespace gfx
{
namespace internal
{
struct in_per_upload_batch_data;
} // internal

class upload_batch : public non_copyable
{
    friend render_device;

    render_device* m_device = nullptr;
    internal::in_per_upload_batch_data* m_batchData = nullptr;
    cmd_list m_cmdList;
    std::uint64_t m_batchID;

    HR_GFX_API upload_batch(render_device& device,
        internal::in_per_upload_batch_data& batchData,
        VkCommandBuffer vkCmdBuf) noexcept;

    HR_GFX_API void in_ensure_batch_was_submitted() const;

public:
    inline render_device& device() const noexcept
    {
        return *m_device;
    }

    inline std::uint64_t id() const noexcept
    {
        return m_batchID;
    }

    HR_GFX_API void add(const void* src, buffer& dst);

    HR_GFX_API void add(const void* src, image& dst);

    HR_GFX_API void submit();

    HR_GFX_API upload_batch& operator=(upload_batch&& other) noexcept;

    upload_batch() noexcept = default;

    HR_GFX_API upload_batch(upload_batch&& other) noexcept;

    inline ~upload_batch()
    {
        in_ensure_batch_was_submitted();
    }
};
} // gfx
} // hr
#endif
