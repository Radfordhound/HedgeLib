#ifndef HR_RENDER_DEVICE_H_INCLUDED
#define HR_RENDER_DEVICE_H_INCLUDED
#include "hr_adapter.h"
#include "hr_resource.h"
#include "hr_upload_batch.h"
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>

struct VmaAllocator_T;

namespace hr
{
namespace gfx
{
class surface;
class render_graph_builder;
class shader_data_write_desc;

namespace internal
{
enum class in_desc_pool_types
{
    sampler,
    combined_image_sampler,
    sampled_image,
    storage_image,
    uniform_texel_buffer,
    storage_texel_buffer,
    uniform_buffer,
    storage_buffer,
    uniform_buffer_dynamic,
    storage_buffer_dynamic,
    input_attachment,
    count
};

constexpr std::size_t in_desc_pool_types_count =
    static_cast<std::size_t>(in_desc_pool_types::count);

constexpr uint32_t in_default_desc_pool_size = 2000;

struct in_desc_pool_allocator
{
    std::array<float, in_desc_pool_types_count> poolSizeMultipliers =
    {
        4.0f,                                                           // sampler
        4.0f,                                                           // combined_image_sampler
        4.0f,                                                           // sampled_image
        1.0f,                                                           // storage_image
        1.0f,                                                           // uniform_texel_buffer
        1.0f,                                                           // storage_texel_buffer
        2.0f,                                                           // uniform_buffer
        2.0f,                                                           // storage_buffer
        1.0f,                                                           // uniform_buffer_dynamic
        1.0f,                                                           // storage_buffer_dynamic
        1.0f                                                            // input_attachment
    };

    std::vector<VkDescriptorPool> unusedPools;
    std::mutex mutex;

    inline uint32_t desc_count(in_desc_pool_types type) const noexcept
    {
        return static_cast<uint32_t>(poolSizeMultipliers[
            static_cast<std::size_t>(type)] * in_default_desc_pool_size);
    }

    HR_GFX_API std::array<VkDescriptorPoolSize, in_desc_pool_types_count>
        get_desc_pool_sizes() const noexcept;

    HR_GFX_API void destroy(render_device& device) noexcept;
};

struct in_desc_pools
{
    std::vector<VkDescriptorPool> usablePools;
    std::vector<VkDescriptorPool> fullPools;

    HR_GFX_API void reset(VkDevice vkDevice, in_desc_pool_allocator& descPoolAllocator);

    HR_GFX_API void destroy(render_device& device) noexcept;
};

struct in_swap_chain
{
    VkSwapchainKHR vkSwapChain = nullptr;
    VkSurfaceKHR vkSurface;
    VkSurfaceFormatKHR vkSurfaceFormat;
    VkSurfaceCapabilitiesKHR vkSurfaceCapabilities;
    VkExtent2D vkSurfaceExtent;
    std::unique_ptr<VkImage[]> vkSwapChainImages = nullptr;
    std::unique_ptr<VkImageView[]> vkSwapChainImageViews = nullptr;
    uint32_t vkSwapChainImageCount;
    uint32_t requestedImageCount;
    uint32_t curImageIndex = 0;
    bool vsync;
    bool needFramebufferResize = false;

    HR_GFX_API void create(VkPhysicalDevice vkPhyDev, VkDevice vkDevice,
        const in_queue_families& queueFamilies);

    HR_GFX_API void recreate(VkPhysicalDevice vkPhyDev, VkDevice vkDevice,
        const in_queue_families& queueFamilies);

    HR_GFX_API void destroy(VkDevice vkDevice) noexcept;

    HR_GFX_API in_swap_chain(VkSurfaceKHR vkSurface,
        VkPhysicalDevice vkPhyDev, VkDevice vkDevice,
        const in_queue_families& queueFamilies,
        VkExtent2D vkExtent, uint32_t imageCount, bool vsync);
};

struct in_per_frame_data
{
    VkSemaphore vkImageAcquiredSemaphore;
    VkSemaphore vkDrawCompleteSemaphore;
    VkFence vkFence;
    in_desc_pools descPools;

    HR_GFX_API void destroy(render_device& device) noexcept;

    HR_GFX_API in_per_frame_data(render_device& device);
};

struct in_per_upload_batch_data
{
    uint64_t curBatchID = 0;
    std::vector<buffer> uploadBuffers;
};

constexpr unsigned int in_max_upload_batches_per_thread = 4;

struct in_per_thread_data
{
    /** @brief The value the next upload batch is going to signal when complete. */
    uint64_t nextBatchID = 0;

    /** @brief Data for every upload batch we might possibly use. */
    std::array<in_per_upload_batch_data, in_max_upload_batches_per_thread> batchData;

    /**
        @brief Vulkan timeline semaphore set to the ID
        of the latest batch that has finished uploading.
    */
    VkSemaphore vkUploadCompleteSemaphore;

    HR_GFX_API unsigned int get_next_upload_batch_index() const;

    HR_GFX_API void destroy(render_device& device) noexcept;

    HR_GFX_API in_per_thread_data(render_device& device);
};

struct in_per_frame_thread_data
{
    VkCommandPool vkGraphicsCmdPool;
    VkCommandPool vkTransferCmdPool;
    cmd_list graphicsCmdList; // TODO: Will we need several of these?
    std::array<cmd_list, in_max_upload_batches_per_thread> transferCmdLists;

    HR_GFX_API void destroy(render_device& device) noexcept;

    HR_GFX_API in_per_frame_thread_data(render_device& device,
        const in_queue_families& queueFamilies);
};
} // internal

class res_allocator : public non_copyable
{
    VmaAllocator_T* m_vmaAllocator;

public:
    inline VmaAllocator_T* handle() const noexcept
    {
        return m_vmaAllocator;
    }

    HR_GFX_API void flush(uint32_t vmaAllocCount,
        VmaAllocation_T* const* vmaAllocs,
        const VkDeviceSize* offsets = nullptr,
        const VkDeviceSize* sizes = nullptr);

    HR_GFX_API void destroy() noexcept;

    HR_GFX_API res_allocator& operator=(res_allocator&& other) noexcept;

    HR_GFX_API res_allocator(render_device& device);

    HR_GFX_API res_allocator(res_allocator&& other) noexcept;

    inline ~res_allocator()
    {
        destroy();
    }
};

class render_device : public non_copyable, public non_moveable
{
    friend shader_data_allocator;
    friend render_graph_builder;
    friend buffer;
    friend image;

    gfx::adapter m_adapter;
    VkDevice m_vkDevice;
    std::array<VkQueue, internal::HR_IN_QUEUE_TYPE_COUNT> m_vkQueues;
    res_allocator m_allocator;
    internal::in_desc_pool_allocator m_globalDescPoolAllocator;
    internal::in_desc_pools m_globalDescPools;
    internal::in_swap_chain m_swapChain;
    internal::in_desc_pool_allocator m_perFrameDescPoolAllocator;
    internal::in_per_frame_data* m_frameData;
    internal::in_per_thread_data* m_threadData;
    internal::in_per_frame_thread_data* m_frameThreadData;
    unsigned int m_frameCount;
    unsigned int m_threadCount;
    std::atomic_uint64_t m_curTotalFrameIndex = {0};
    std::mutex m_gfxQueueMutex;

public:
    inline const gfx::adapter& adapter() const noexcept
    {
        return m_adapter;
    }

    inline gfx::adapter& adapter() noexcept
    {
        return m_adapter;
    }

    inline VkDevice handle() const noexcept
    {
        return m_vkDevice;
    }

    inline VkQueue queue(internal::in_queue_type type) const noexcept
    {
        return m_vkQueues[type];
    }

    inline const res_allocator& allocator() const noexcept
    {
        return m_allocator;
    }

    inline res_allocator& allocator() noexcept
    {
        return m_allocator;
    }

    inline const internal::in_swap_chain& swap_chain() const noexcept
    {
        return m_swapChain;
    }

    inline internal::in_swap_chain& swap_chain() noexcept
    {
        return m_swapChain;
    }

    inline const internal::in_per_frame_data* per_frame_data() const noexcept
    {
        return m_frameData;
    }

    inline internal::in_per_frame_data* per_frame_data() noexcept
    {
        return m_frameData;
    }

    inline const internal::in_per_frame_data& per_frame_data(
        unsigned int index) const noexcept
    {
        return m_frameData[index];
    }

    inline internal::in_per_frame_data& per_frame_data(
        unsigned int index) noexcept
    {
        return m_frameData[index];
    }

    inline unsigned int frame_count() const noexcept
    {
        return m_frameCount;
    }

    inline std::uint64_t cur_total_frame_index() const
    {
        return m_curTotalFrameIndex;
    }

    inline unsigned int cur_frame_index() const
    {
        return static_cast<unsigned int>(m_curTotalFrameIndex % m_frameCount);
    }

    inline const internal::in_per_thread_data* per_thread_data() const noexcept
    {
        return m_threadData;
    }

    inline internal::in_per_thread_data* per_thread_data() noexcept
    {
        return m_threadData;
    }

    inline const internal::in_per_thread_data& per_thread_data(
        unsigned int index) const noexcept
    {
        return m_threadData[index];
    }

    inline internal::in_per_thread_data& per_thread_data(
        unsigned int index) noexcept
    {
        return m_threadData[index];
    }

    inline unsigned int thread_count() const noexcept
    {
        return m_threadCount;
    }

    inline const internal::in_per_frame_thread_data*
        per_frame_thread_data() const noexcept
    {
        return m_frameThreadData;
    }

    inline internal::in_per_frame_thread_data*
        per_frame_thread_data() noexcept
    {
        return m_frameThreadData;
    }

    inline const internal::in_per_frame_thread_data& per_frame_thread_data(
        unsigned int frameIndex, unsigned int threadIndex) const noexcept
    {
        return m_frameThreadData[(m_threadCount * frameIndex) + threadIndex];
    }

    inline internal::in_per_frame_thread_data& per_frame_thread_data(
        unsigned int frameIndex, unsigned int threadIndex) noexcept
    {
        return m_frameThreadData[(m_threadCount * frameIndex) + threadIndex];
    }

    HR_GFX_API std::unique_lock<std::mutex> get_gfx_queue_lock();

    HR_GFX_API void wait_for_upload_batch(unsigned int threadIndex,
        uint64_t batchID, uint64_t timeout = UINT64_MAX) const;

    inline void wait_for_upload_batch(upload_batch_uid batchUID,
        uint64_t timeout = UINT64_MAX) const
    {
        wait_for_upload_batch(batchUID.threadIndex,
            batchUID.batchID, timeout);
    }

    HR_GFX_API upload_batch start_upload_batch(unsigned int threadIndex);

    HR_GFX_API void set_pool_size_multiplier(VkDescriptorType type,
        float multiplier, bool perFrame);

    HR_GFX_API shader_data_allocator get_shader_data_allocator(bool perFrame);

    HR_GFX_API void update_shader_data(const shader_data_write_desc* shaderDataWrites,
        std::size_t shaderDataWriteCount);

    HR_GFX_API void begin_frame();
    HR_GFX_API void end_frame();

    HR_GFX_API void destroy() noexcept;

    HR_GFX_API render_device(const gfx::adapter& adapter,
        surface& surface, unsigned int width, unsigned int height,
        unsigned int prefFrameBufCount = 3, bool vsync = true);
    
    inline ~render_device()
    {
        destroy();
    }
};
} // gfx
} // hr
#endif
