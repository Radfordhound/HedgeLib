#include "hr_in_adapter.h"
#include "hr_in_surface.h"
#include "hr_in_instance.h"
#include "hedgerender/gfx/hr_render_device.h"
#include "hedgerender/gfx/hr_shader.h"
#include "hedgerender/gfx/hr_surface.h"
#include "hedgerender/gfx/hr_instance.h"

namespace hr
{
namespace gfx
{
namespace internal
{
std::array<VkDescriptorPoolSize, in_desc_pool_types_count>
    in_desc_pool_allocator::get_desc_pool_sizes() const noexcept
{
    return
    {{
        {
            VK_DESCRIPTOR_TYPE_SAMPLER,                                 // type
            desc_count(in_desc_pool_types::sampler)                     // descriptorCount
        },
        {
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,                  // type
            desc_count(in_desc_pool_types::combined_image_sampler)      // descriptorCount
        },
        {
            VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,                           // type
            desc_count(in_desc_pool_types::sampled_image)               // descriptorCount
        },
        {
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,                           // type
            desc_count(in_desc_pool_types::storage_image)               // descriptorCount
        },
        {
            VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,                    // type
            desc_count(in_desc_pool_types::uniform_texel_buffer)        // descriptorCount
        },
        {
            VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,                    // type
            desc_count(in_desc_pool_types::storage_texel_buffer)        // descriptorCount
        },
        {
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,                          // type
            desc_count(in_desc_pool_types::uniform_buffer)              // descriptorCount
        },
        {
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,                          // type
            desc_count(in_desc_pool_types::storage_buffer)              // descriptorCount
        },
        {
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,                  // type
            desc_count(in_desc_pool_types::uniform_buffer_dynamic)      // descriptorCount
        },
        {
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,                  // type
            desc_count(in_desc_pool_types::storage_buffer_dynamic)      // descriptorCount
        },
        {
            VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,                        // type
            desc_count(in_desc_pool_types::input_attachment)            // descriptorCount
        }
    }};
}

void in_desc_pool_allocator::destroy(render_device& device) noexcept
{
    for (auto vkDescPool : unusedPools)
    {
        vkDestroyDescriptorPool(device.handle(), vkDescPool, nullptr);
    }
}

void in_desc_pools::reset(VkDevice vkDevice, in_desc_pool_allocator& descPoolAllocator)
{
    // Reset all full Vulkan descriptor pools and move them into unused pools list.
    for (auto vkDescPool : fullPools)
    {
        vkResetDescriptorPool(vkDevice, vkDescPool, 0);
        descPoolAllocator.unusedPools.push_back(vkDescPool);
    }

    fullPools.clear();

    // Reset all usable Vulkan descriptor pools and move them into unused pools list.
    for (auto vkDescPool : usablePools)
    {
        vkResetDescriptorPool(vkDevice, vkDescPool, 0);
        descPoolAllocator.unusedPools.push_back(vkDescPool);
    }
    
    usablePools.clear();
}

void in_desc_pools::destroy(render_device& device) noexcept
{
    for (auto vkDescPool : fullPools)
    {
        vkDestroyDescriptorPool(device.handle(), vkDescPool, nullptr);
    }

    for (auto vkDescPool : usablePools)
    {
        vkDestroyDescriptorPool(device.handle(), vkDescPool, nullptr);
    }
}

void in_swap_chain::create(VkPhysicalDevice vkPhyDev,
    VkDevice vkDevice, const in_queue_families& queueFamilies)
{
    // Get clamped Vulkan swap chain minimum image count.
    const uint32_t minImageCount = in_vulkan_clamp_image_count(
        vkSurfaceCapabilities, requestedImageCount);

    // Clamp Vulkan surface extent.
    vkSurfaceExtent = in_vulkan_clamp_surface_extent(
        vkSurfaceCapabilities, vkSurfaceExtent);

    // Generate Vulkan swap chain create info.
    VkSwapchainKHR vkOldSwapChain = vkSwapChain;
    VkSwapchainCreateInfoKHR vkSwapChainCreateInfo =
    {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,                    // sType
        nullptr,                                                        // pNext
        0U,                                                             // flags
        vkSurface,                                                      // surface
        minImageCount,                                                  // minImageCount
        vkSurfaceFormat.format,                                         // imageFormat
        vkSurfaceFormat.colorSpace,                                     // imageColorSpace
        vkSurfaceExtent,                                                // imageExtent
        1U,                                                             // imageArrayLayers
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,                            // imageUsage
        VK_SHARING_MODE_EXCLUSIVE,                                      // imageSharingMode
        0U,                                                             // queueFamilyIndexCount
        nullptr,                                                        // pQueueFamilyIndices
        vkSurfaceCapabilities.currentTransform,                         // preTransform
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,                              // compositeAlpha
        in_vulkan_get_best_present_mode(vkSurface, vkPhyDev, vsync),    // presentMode
        VK_TRUE,                                                        // clipped
        vkOldSwapChain                                                  // oldSwapchain
    };

    // Share swap chain images between graphics queue family and
    // present queue family if we have a unique present family.
    if (queueFamilies.has_unique_family(in_queue_type::present))
    {
        vkSwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        vkSwapChainCreateInfo.queueFamilyIndexCount = 2;
        vkSwapChainCreateInfo.pQueueFamilyIndices = queueFamilies.familyIndices.data();
    }

    // Create Vulkan swap chain, and destroy the old swap chain, if any.
    const VkResult vkResult = vkCreateSwapchainKHR(vkDevice,
        &vkSwapChainCreateInfo, nullptr, &vkSwapChain);

    // NOTE: All Vulkan destroy functions perform their own null checks, so we don't have to.
    vkDestroySwapchainKHR(vkDevice, vkOldSwapChain, nullptr);

    // Destroy old swap chain image views.
    for (uint32_t i = 0; i < vkSwapChainImageCount; ++i)
    {
        vkDestroyImageView(vkDevice, vkSwapChainImageViews[i], nullptr);
    }

    // Check if swap chain creation succeeded.
    if (vkResult != VK_SUCCESS)
    {
        vkSwapChainImageViews.reset();
        vkSwapChain = VK_NULL_HANDLE;
        throw std::runtime_error("Could not create Vulkan swap chain");
    }

    // Get Vulkan swap chain image count.
    if (vkGetSwapchainImagesKHR(vkDevice, vkSwapChain,
        &vkSwapChainImageCount, nullptr) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not get Vulkan swap chain image count");
    }

    // Get Vulkan swap chain images.
    vkSwapChainImages.reset(new VkImage[vkSwapChainImageCount]);
    if (vkGetSwapchainImagesKHR(vkDevice, vkSwapChain,
        &vkSwapChainImageCount, vkSwapChainImages.get()) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not get Vulkan swap chain images");
    }

    // Create Vulkan swap chain image views.
    vkSwapChainImageViews.reset(new VkImageView[vkSwapChainImageCount]);
    for (uint32_t i = 0; i < vkSwapChainImageCount; ++i)
    {
        const VkImageViewCreateInfo vkImageViewCreateInfo =
        {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,                   // sType
            nullptr,                                                    // pNext
            0,                                                          // flags
            vkSwapChainImages[i],                                       // image
            VK_IMAGE_VIEW_TYPE_2D,                                      // viewType
            vkSurfaceFormat.format,                                     // format

            {                                                           // components
                VK_COMPONENT_SWIZZLE_IDENTITY,                          //  r
                VK_COMPONENT_SWIZZLE_IDENTITY,                          //  g
                VK_COMPONENT_SWIZZLE_IDENTITY,                          //  b
                VK_COMPONENT_SWIZZLE_IDENTITY                           //  a
            },

            {                                                           // subresourceRange
                VK_IMAGE_ASPECT_COLOR_BIT,                              //  aspectMask
                0,                                                      //  baseMipLevel
                1,                                                      //  levelCount
                0,                                                      //  baseArrayLayer
                1                                                       //  layerCount
            }
        };

        if (vkCreateImageView(vkDevice, &vkImageViewCreateInfo,
            nullptr, &vkSwapChainImageViews[i]) != VK_SUCCESS)
        {
            while (i > 0)
            {
                vkDestroyImageView(vkDevice, vkSwapChainImageViews[--i], nullptr);
            }

            throw std::runtime_error("Could not create Vulkan swap chain image views");
        }
    }
}

void in_swap_chain::recreate(VkPhysicalDevice vkPhyDev,
    VkDevice vkDevice, const in_queue_families& queueFamilies)
{
    // Determine surface format and capabilities again.
    vkSurfaceFormat = in_vulkan_get_best_surface_format(vkSurface, vkPhyDev);
    vkSurfaceCapabilities = in_vulkan_get_surface_capabilities(vkSurface, vkPhyDev);

    // Re-create the Vulkan swap chain.
    create(vkPhyDev, vkDevice, queueFamilies);

    // Mark this swap chain as requiring a framebuffer resize.
    needFramebufferResize = true;
}

void in_swap_chain::destroy(VkDevice vkDevice) noexcept
{
    // NOTE: The swap chain images are automatically destroyed with the swap chain.

    // Destroy image views.
    for (uint32_t i = 0; i < vkSwapChainImageCount; ++i)
    {
        vkDestroyImageView(vkDevice, vkSwapChainImageViews[i], nullptr);
    }

    // Destroy swap chain.
    vkDestroySwapchainKHR(vkDevice, vkSwapChain, nullptr);
}

in_swap_chain::in_swap_chain(
    VkSurfaceKHR vkSurface, VkPhysicalDevice vkPhyDev, VkDevice vkDevice,
    const in_queue_families& queueFamilies, VkExtent2D vkExtent,
    uint32_t imageCount, bool vsync) :

    vkSurface(vkSurface),
    vkSurfaceFormat(in_vulkan_get_best_surface_format(vkSurface, vkPhyDev)),
    vkSurfaceCapabilities(in_vulkan_get_surface_capabilities(vkSurface, vkPhyDev)),
    vkSurfaceExtent(vkExtent),
    vkSwapChainImageCount(0),
    requestedImageCount(imageCount),
    vsync(vsync)
{
    create(vkPhyDev, vkDevice, queueFamilies);
}

void in_per_frame_data::destroy(render_device& device) noexcept
{
    descPools.destroy(device);
    vkDestroyFence(device.handle(), vkFence, nullptr);
    vkDestroySemaphore(device.handle(), vkDrawCompleteSemaphore, nullptr);
    vkDestroySemaphore(device.handle(), vkImageAcquiredSemaphore, nullptr);
}

in_per_frame_data::in_per_frame_data(render_device& device)
{
    // Create Vulkan semaphores.
    const VkSemaphoreCreateInfo vkSemaphoreCreateInfo =
    {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,                        // sType
        nullptr,                                                        // pNext
        0                                                               // flags
    };

    if (vkCreateSemaphore(device.handle(), &vkSemaphoreCreateInfo,
        nullptr, &vkImageAcquiredSemaphore) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create Vulkan semaphore");
    }

    if (vkCreateSemaphore(device.handle(), &vkSemaphoreCreateInfo,
        nullptr, &vkDrawCompleteSemaphore) != VK_SUCCESS)
    {
        vkDestroySemaphore(device.handle(), vkImageAcquiredSemaphore, nullptr);
        throw std::runtime_error("Could not create Vulkan semaphore");
    }

    // Create Vulkan fence.
    const VkFenceCreateInfo vkFenceCreateInfo =
    {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,                            // sType
        nullptr,                                                        // pNext
        VK_FENCE_CREATE_SIGNALED_BIT                                    // flags
    };

    if (vkCreateFence(device.handle(), &vkFenceCreateInfo, nullptr, &vkFence) != VK_SUCCESS)
    {
        vkDestroySemaphore(device.handle(), vkDrawCompleteSemaphore, nullptr);
        vkDestroySemaphore(device.handle(), vkImageAcquiredSemaphore, nullptr);
        throw std::runtime_error("Could not create Vulkan fence");
    }
}

void in_per_thread_data::destroy(render_device& device) noexcept {}

in_per_thread_data::in_per_thread_data(render_device& device) {}

void in_per_frame_thread_data::destroy(render_device& device) noexcept
{
    // NOTE: Vulkan command buffers are freed when their command pools are destroyed.

    // Destroy Vulkan transfer command pool.
    vkDestroyCommandPool(device.handle(), vkTransferCmdPool, nullptr);

    // Destroy Vulkan graphics command pool.
    vkDestroyCommandPool(device.handle(), vkGraphicsCmdPool, nullptr);
}

in_per_frame_thread_data::in_per_frame_thread_data(
    render_device& device, const in_queue_families& queueFamilies)
{
    // Create Vulkan graphics command pool.
    VkCommandPoolCreateInfo vkCmdPoolCreateInfo =
    {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,                     // sType
        nullptr,                                                        // pNext
        0,                                                              // flags
        queueFamilies.graphics_family()                                 // queueFamilyIndex
    };

    if (vkCreateCommandPool(device.handle(), &vkCmdPoolCreateInfo,
        nullptr, &vkGraphicsCmdPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create Vulkan graphics command pool");
    }

    // Allocate Vulkan graphics command buffers.
    VkCommandBufferAllocateInfo vkCmdBufAllocInfo =
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,                 // sType
        nullptr,                                                        // pNext
        vkGraphicsCmdPool,                                              // commandPool
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                                // level
        1                                                               // commandBufferCount
    };

    VkCommandBuffer vkGraphicsCmdBuf;
    if (vkAllocateCommandBuffers(device.handle(),
        &vkCmdBufAllocInfo, &vkGraphicsCmdBuf) != VK_SUCCESS)
    {
        vkDestroyCommandPool(device.handle(), vkGraphicsCmdPool, nullptr);
        throw std::runtime_error("Could not allocate Vulkan graphics command buffers");
    }

    // Create Vulkan transfer command pool.
    vkCmdPoolCreateInfo.queueFamilyIndex = queueFamilies.transfer_family();
    vkCmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(device.handle(), &vkCmdPoolCreateInfo,
        nullptr, &vkTransferCmdPool) != VK_SUCCESS)
    {
        vkDestroyCommandPool(device.handle(), vkGraphicsCmdPool, nullptr);
        throw std::runtime_error("Could not create Vulkan transfer command pool");
    }

    // Allocate Vulkan transfer command buffers.
    std::array<VkCommandBuffer, in_max_upload_batches_per_thread> vkTransferCmdBufs;
    vkCmdBufAllocInfo.commandPool = vkTransferCmdPool;
    vkCmdBufAllocInfo.commandBufferCount = in_max_upload_batches_per_thread;

    if (vkAllocateCommandBuffers(device.handle(),
        &vkCmdBufAllocInfo, vkTransferCmdBufs.data()) != VK_SUCCESS)
    {
        vkDestroyCommandPool(device.handle(), vkTransferCmdPool, nullptr);
        vkDestroyCommandPool(device.handle(), vkGraphicsCmdPool, nullptr);
        throw std::runtime_error("Could not allocate Vulkan transfer command buffers");
    }

    // Setup command lists.
    graphicsCmdList = vkGraphicsCmdBuf;
    for (unsigned int i = 0; i < in_max_upload_batches_per_thread; ++i)
    {
        transferCmdLists[i] = vkTransferCmdBufs[i];
    }
}
} // internal

void res_allocator::flush(uint32_t vmaAllocCount,
    VmaAllocation_T* const* vmaAllocs, const VkDeviceSize* offsets,
    const VkDeviceSize* sizes)
{
    if (vmaFlushAllocations(m_vmaAllocator, vmaAllocCount,
        vmaAllocs, offsets, sizes) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not flush Vulkan allocator");
    }
}

void res_allocator::destroy() noexcept
{
    vmaDestroyAllocator(m_vmaAllocator);
    m_vmaAllocator = nullptr;
}

res_allocator& res_allocator::operator=(res_allocator&& other) noexcept
{
    if (&other != this)
    {
        destroy();

        m_vmaAllocator = other.m_vmaAllocator;
        other.m_vmaAllocator = nullptr;
    }

    return *this;
}

static VmaAllocator in_vma_create_allocator(VkInstance vkInstance,
    VkPhysicalDevice vkPhyDev, VkDevice vkDevice)
{
    const VmaAllocatorCreateInfo vmaAllocatorCreateInfo =
    {
        0,                                                              // flags
        vkPhyDev,                                                       // physicalDevice
        vkDevice,                                                       // device
        0,                                                              // preferredLargeHeapBlockSize
        nullptr,                                                        // pAllocationCallbacks
        nullptr,                                                        // pDeviceMemoryCallbacks
        nullptr,                                                        // pHeapSizeLimit
        nullptr,                                                        // pVulkanFunctions
        vkInstance,                                                     // instance
        in_vulkan_api_version                                           // vulkanApiVersion
    };

    VmaAllocator vmaAllocator;
    if (vmaCreateAllocator(&vmaAllocatorCreateInfo, &vmaAllocator) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create VMA allocator");
    }

    return vmaAllocator;
}

res_allocator::res_allocator(render_device& device) :
    m_vmaAllocator(in_vma_create_allocator(device.adapter().parent().handle(),
        device.adapter().handle(), device.handle())) {}

res_allocator::res_allocator(res_allocator&& other) noexcept :
    m_vmaAllocator(other.m_vmaAllocator)
{
    other.m_vmaAllocator = nullptr;
}

std::unique_lock<std::mutex> render_device::get_gfx_queue_lock()
{
    return std::unique_lock<std::mutex>(m_gfxQueueMutex);
}

bool render_device::is_frame_render_done(unsigned int frameIndex) const
{
    switch (vkGetFenceStatus(m_vkDevice, m_frameData[frameIndex].vkFence))
    {
    case VK_SUCCESS:
        return true;

    case VK_NOT_READY:
        return false;
        
    default:
        throw std::runtime_error("Could not check status of Vulkan fence");
    }
}

void render_device::wait_for_frame_render(unsigned int frameIndex, std::uint64_t timeout) const
{
    if (vkWaitForFences(m_vkDevice, 1, &m_frameData[frameIndex].vkFence,
        VK_TRUE, timeout) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not wait for Vulkan fence");
    }
}

bool render_device::is_upload_batch_done(std::uint64_t batchID) const
{
    uint64_t vkSemaphoreVal;
    if (vkGetSemaphoreCounterValue(m_vkDevice, m_vkUploadCompleteSemaphore,
        &vkSemaphoreVal) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not query status of upload batch");
    }

    return (vkSemaphoreVal >= batchID);
}

void render_device::wait_for_upload_batch(std::uint64_t batchID, std::uint64_t timeout) const
{
    const VkSemaphoreWaitInfo vkSemaphoreWaitInfo =
    {
        VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,                          // sType
        nullptr,                                                        // pNext
        0,                                                              // flags
        1,                                                              // semaphoreCount
        &m_vkUploadCompleteSemaphore,                                   // pSemaphores
        &batchID                                                        // pValues
    };

    if (vkWaitSemaphores(m_vkDevice, &vkSemaphoreWaitInfo, timeout) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not wait for upload batch");
    }
}

void render_device::wait_for_idle() const
{
    if (vkDeviceWaitIdle(m_vkDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not wait for Vulkan device to idle");
    }
}

upload_batch render_device::start_upload_batch(unsigned int threadIndex)
{
    // Get reference to data for the current frame/thread.
    const unsigned int curFrameIndex = cur_frame_index();
    auto& curThreadData = m_threadData[threadIndex];
    auto& curFrameThreadData = per_frame_thread_data(
        curFrameIndex, threadIndex);

    // Wait for the current upload batch to complete.
    // NOTE: If this upload batch hasn't started yet, this doesn't stall.
    const auto curBatchIndex = curThreadData.get_next_upload_batch_index();
    auto& curBatchData = curThreadData.batchData[curBatchIndex];
    
    wait_for_upload_batch(curBatchData.curBatchID);

    // Destroy all temporary upload buffers from the previous upload batch, if any.
    curBatchData.uploadBuffers.clear();

    // Reset Vulkan command buffer for this upload batch.
    VkCommandBuffer vkCmdBuf = curFrameThreadData.transferCmdLists[curBatchIndex].handle();
    if (vkResetCommandBuffer(vkCmdBuf, 0) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not reset Vulkan command buffer");
    }

    // Begin recording Vulkan command buffer for this upload batch.
    const VkCommandBufferBeginInfo vkCmdBufBeginInfo =
    {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,                    // sType
        nullptr,                                                        // pNext
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,                    // flags
        nullptr                                                         // pInheritanceInfo
    };

    if (vkBeginCommandBuffer(vkCmdBuf, &vkCmdBufBeginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not begin recording Vulkan command buffer");
    }

    // Set new upload batch ID and return new upload batch object.
    curBatchData.curBatchID = ++m_curUploadBatchID;
    return upload_batch(*this, curBatchData, vkCmdBuf);
}

static internal::in_desc_pool_types in_get_desc_pool_type(VkDescriptorType type)
{
    using namespace internal;

    switch (type)
    {
    case VK_DESCRIPTOR_TYPE_SAMPLER:
        return in_desc_pool_types::sampler;

    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        return in_desc_pool_types::combined_image_sampler;

    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        return in_desc_pool_types::sampled_image;

    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        return in_desc_pool_types::storage_image;

    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        return in_desc_pool_types::uniform_texel_buffer;

    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        return in_desc_pool_types::storage_texel_buffer;

    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        return in_desc_pool_types::uniform_buffer;

    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        return in_desc_pool_types::storage_buffer;

    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        return in_desc_pool_types::uniform_buffer_dynamic;

    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        return in_desc_pool_types::storage_buffer_dynamic;

    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        return in_desc_pool_types::input_attachment;

    default:
        throw std::runtime_error("Unsupported Vulkan descriptor type");
    }
}

void render_device::set_pool_size_multiplier(VkDescriptorType type,
    float multiplier, bool perFrame)
{
    using namespace internal;

    // Get the requested descriptor pool allocator.
    auto& descPoolAllocator = (perFrame) ?
        m_perFrameDescPoolAllocator :
        m_globalDescPoolAllocator;

    // Determine descriptor pool size multiplier index.
    const std::size_t poolSizeMultiplierIndex = static_cast<std::size_t>(
        in_get_desc_pool_type(type));

    // Set new descriptor pool size multiplier.
    descPoolAllocator.poolSizeMultipliers[poolSizeMultiplierIndex] = multiplier;
}

shader_data_allocator render_device::get_shader_data_allocator(bool perFrame)
{
    using namespace internal;

    // Get the requested descriptor pool allocator and descriptor pools.
    auto& descPoolAllocator = (perFrame) ?
        m_perFrameDescPoolAllocator :
        m_globalDescPoolAllocator;

    auto& descPools = (perFrame) ?
        m_frameData[cur_frame_index()].descPools :
        m_globalDescPools;

    // Lock the descriptor pool allocator mutex.
    std::lock_guard<std::mutex> lock(descPoolAllocator.mutex);

    // Attempt to reuse a previously-created pool.
    VkDescriptorPool vkDescPool;
    if (!descPoolAllocator.unusedPools.empty())
    {
        vkDescPool = descPoolAllocator.unusedPools.back();
        descPoolAllocator.unusedPools.pop_back();
    }
    else if (!descPools.usablePools.empty())
    {
        vkDescPool = descPools.usablePools.back();
        descPools.usablePools.pop_back();
    }

    // We need a new pool; create a new one.
    else
    {
        const auto vkDescPoolSizes = descPoolAllocator.get_desc_pool_sizes();
        const VkDescriptorPoolCreateInfo vkDescPoolCreateInfo =
        {
            VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,              // sType
            nullptr,                                                    // pNext
            0,                                                          // flags
            in_default_desc_pool_size,                                  // maxSets
            static_cast<uint32_t>(vkDescPoolSizes.size()),              // poolSizeCount
            vkDescPoolSizes.data()                                      // pPoolSizes
        };

        if (vkCreateDescriptorPool(m_vkDevice, &vkDescPoolCreateInfo,
            nullptr, &vkDescPool) != VK_SUCCESS)
        {
            throw std::runtime_error("Could not create Vulkan descriptor pool");
        }
    }

    return shader_data_allocator(*this, descPoolAllocator, descPools, vkDescPool);
}

void render_device::update_shader_data(const shader_data_write_desc* shaderDataWrites,
    std::size_t shaderDataWriteCount)
{
    // Generate Vulkan write descriptor sets.
    hl::stack_or_heap_buffer<VkWriteDescriptorSet, 4> vkWriteDescSets(shaderDataWriteCount);
    std::size_t vkDescImageInfoCount = 0, vkDescBufferInfoCount = 0;

    for (std::size_t i = 0; i < shaderDataWriteCount; ++i)
    {
        auto& vkWriteDescSet = vkWriteDescSets[i];
        auto& shaderDataWrite = shaderDataWrites[i];

        vkWriteDescSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vkWriteDescSet.pNext = nullptr;
        vkWriteDescSet.dstSet = shaderDataWrite.shaderData;
        vkWriteDescSet.dstBinding = shaderDataWrite.firstRegisterIndex;
        vkWriteDescSet.dstArrayElement = shaderDataWrite.arrayElementIndex;
        vkWriteDescSet.descriptorCount = shaderDataWrite.registerCount;
        vkWriteDescSet.descriptorType = shaderDataWrite.type;
        
        switch (shaderDataWrite.type)
        {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            vkDescImageInfoCount += shaderDataWrite.registerCount;
            break;

        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            vkDescBufferInfoCount += shaderDataWrite.registerCount;
            break;

        default:
            throw std::runtime_error("Unknown or unsupported Vulkan descriptor type");
        }
    }

    // Generate Vulkan descriptor image/buffer infos.
    hl::stack_or_heap_buffer<VkDescriptorImageInfo, 8> vkDescImageInfos(vkDescImageInfoCount);
    hl::stack_or_heap_buffer<VkDescriptorBufferInfo, 8> vkDescBufferInfos(vkDescBufferInfoCount);
    vkDescImageInfoCount = vkDescBufferInfoCount = 0;

    for (std::size_t i = 0; i < shaderDataWriteCount; ++i)
    {
        auto& vkWriteDescSet = vkWriteDescSets[i];
        auto& shaderDataWrite = shaderDataWrites[i];

        switch (shaderDataWrite.type)
        {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        {
            vkWriteDescSet.pImageInfo = &vkDescImageInfos[vkDescImageInfoCount];

            for (std::size_t i2 = 0; i2 < shaderDataWrite.registerCount; ++i2)
            {
                auto& vkDescImageInfo = vkDescImageInfos[vkDescImageInfoCount++];
                auto& imageWrite = shaderDataWrite.imageWrites[i2];

                vkDescImageInfo.sampler = imageWrite.vkSampler;
                vkDescImageInfo.imageView = imageWrite.vkImageView;
                vkDescImageInfo.imageLayout = imageWrite.imageLayout;
            }
            break;
        }

        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        {
            vkWriteDescSet.pBufferInfo = &vkDescBufferInfos[vkDescBufferInfoCount];

            for (std::size_t i2 = 0; i2 < shaderDataWrite.registerCount; ++i2)
            {
                auto& vkDescBufferInfo = vkDescBufferInfos[vkDescBufferInfoCount++];
                auto& bufferWrite = shaderDataWrite.bufferWrites[i2];

                vkDescBufferInfo.buffer = bufferWrite.vkBuffer;
                vkDescBufferInfo.offset = bufferWrite.offset;
                vkDescBufferInfo.range = bufferWrite.size;
            }
            break;
        }

        default:
            throw std::runtime_error("Unknown or unsupported Vulkan descriptor type");
        }
    }

    // Update Vulkan descriptor sets.
    vkUpdateDescriptorSets(m_vkDevice, static_cast<uint32_t>(shaderDataWriteCount),
        vkWriteDescSets, 0, nullptr);
}

void render_device::begin_frame()
{
    // Wait for the current frame's fence to be signaled from the GPU.
    auto& curFrameData = m_frameData[cur_frame_index()];
    if (vkWaitForFences(m_vkDevice, 1, &curFrameData.vkFence,
        VK_TRUE, UINT64_MAX) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not wait for Vulkan fence");
    }

    // Reset the current frame's fence back to the unsignaled state.
    if (vkResetFences(m_vkDevice, 1, &curFrameData.vkFence) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not reset Vulkan fence");
    }

    // Acquire next image from Vulkan swap chain.
    VkResult vkResult;
    while ((vkResult = vkAcquireNextImageKHR(m_vkDevice, m_swapChain.vkSwapChain,
        UINT64_MAX, curFrameData.vkImageAcquiredSemaphore, VK_NULL_HANDLE,
        &m_swapChain.curImageIndex)) != VK_SUCCESS)
    {
        switch (vkResult)
        {
        case VK_SUBOPTIMAL_KHR:
            // We can still use this swap chain for this frame; it's just not optimal.
            // That's alright, though, as we re-create the swap chain if needed in
            // render_device::end_frame() anyway.
            break;

        case VK_ERROR_OUT_OF_DATE_KHR:
            m_swapChain.recreate(m_adapter.handle(), m_vkDevice, m_adapter.queue_families());
            break;

        default:
            throw std::runtime_error("Could not acquire next image from Vulkan swapchain");
        }
    }
}

void render_device::end_frame()
{
    using namespace internal;

    // Generate Vulkan present info.
    auto& curFrameData = m_frameData[cur_frame_index()];
    const VkPresentInfoKHR vkPresentInfo =
    {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,                             // sType
        nullptr,                                                        // pNext
        1,                                                              // waitSemaphoreCount
        &curFrameData.vkDrawCompleteSemaphore,                          // pWaitSemaphores
        1,                                                              // swapchainCount
        &m_swapChain.vkSwapChain,                                       // pSwapchains
        &m_swapChain.curImageIndex,                                     // pImageIndices
        nullptr                                                         // pResults
    };

    // Present Vulkan swap chain image.
    {
        // Lock graphics queue if our "present queue" is actually just the graphics queue.
        std::unique_lock<std::mutex> m_gfxQueueLock;
        if (!m_adapter.queue_families().has_unique_family(in_queue_type::present))
        {
            m_gfxQueueLock = get_gfx_queue_lock();
        }

        // Present Vulkan swap chain image.
        switch (vkQueuePresentKHR(m_vkQueues[in_queue_type::present], &vkPresentInfo))
        {
        case VK_SUCCESS:
            break;

        case VK_SUBOPTIMAL_KHR:
        case VK_ERROR_OUT_OF_DATE_KHR:
            m_swapChain.recreate(m_adapter.handle(), m_vkDevice, m_adapter.queue_families());
            break;

        default:
            throw std::runtime_error("Could not present Vulkan swap chain image");
        }
    }

    // Increase current frame index, and get next frame index.
    const unsigned int nextFrameIndex = static_cast<unsigned int>(
        ++m_curTotalFrameIndex % m_frameCount);

    // Reset descriptor pools for next frame.
    m_frameData[nextFrameIndex].descPools.reset(m_vkDevice, m_perFrameDescPoolAllocator);
}

template<typename T, typename... args_t>
static T* in_create_per_t_array(std::size_t count,
    render_device& device, args_t&&... args)
{
    T* arr = static_cast<T*>(::operator new[](sizeof(T)* count));
    std::size_t i = 0;

    try
    {
        for (; i < count; ++i)
        {
            new (arr + i) T(device, std::forward<args_t>(args)...);
        }
    }
    catch (const std::exception& ex)
    {
        while (i > 0)
        {
            T& obj = arr[--i];
            obj.destroy(device);
            obj.~T();
        }

        ::operator delete[](arr);
        throw ex;
    }

    return arr;
}

template<typename T>
static void in_destroy_per_t_array(render_device& device,
    T* arr, std::size_t count)
{
    for (std::size_t i = 0; i < count; ++i)
    {
        T& obj = arr[i];
        obj.destroy(device);
        obj.~T();
    }

    ::operator delete[](arr);
}

void render_device::destroy() noexcept
{
    // Return early if this device is just an empty shell
    // (i.e. one that's been moved from using std::move()).
    if (!m_vkDevice) return;

    // Wait for Vulkan device to idle so we can safely clean everything up.
    vkDeviceWaitIdle(m_vkDevice);

    // Destroy upload complete timeline semaphore.
    vkDestroySemaphore(m_vkDevice, m_vkUploadCompleteSemaphore, nullptr);

    // Destroy per-frame-per-thread data.
    in_destroy_per_t_array(*this, m_frameThreadData,
        m_frameCount * m_threadCount);

    // Destroy per-thread data.
    in_destroy_per_t_array(*this, m_threadData, m_threadCount);

    // Destroy per-frame data.
    in_destroy_per_t_array(*this, m_frameData, m_frameCount);

    // Destroy per-frame descriptor pool allocator.
    m_perFrameDescPoolAllocator.destroy(*this);

    // Destroy swap chain data.
    m_swapChain.destroy(m_vkDevice);

    // Destroy global descriptor pools.
    m_globalDescPools.destroy(*this);

    // Destroy global descriptor pool allocator.
    m_globalDescPoolAllocator.destroy(*this);

    // Destroy pipeline cache.
    vkDestroyPipelineCache(m_vkDevice, m_vkPipelineCache, nullptr);

    // Destroy allocator.
    m_allocator.destroy();

    // Destroy device.
    vkDestroyDevice(m_vkDevice, nullptr);
}

#ifdef VK_EXT_debug_utils
// Use this function as a fallback in the event that the pointer has not been set.
// Doing it this way allows us to avoid doing a nullptr check every time.
static VkResult VKAPI_PTR in_vulkan_fallback_set_debug_name_utils(
    VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo)
{
    return VK_SUCCESS;
}

static PFN_vkSetDebugUtilsObjectNameEXT in_vulkan_set_debug_utils_name_ptr =
    &in_vulkan_fallback_set_debug_name_utils;
#endif

void in_vulkan_set_debug_name(VkDevice vkDevice,
    VkObjectType vkObjectType, uint64_t vkObjectHandle,
    const char* name)
{
#ifdef VK_EXT_debug_utils
    const VkDebugUtilsObjectNameInfoEXT vkDebugObjectNameInfo =
    {
        VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,             // sType
        nullptr,                                                        // pNext
        vkObjectType,                                                   // objectType
        vkObjectHandle,                                                 // objectHandle
        name                                                            // pObjectName
    };

    if (in_vulkan_set_debug_utils_name_ptr(vkDevice, &vkDebugObjectNameInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not set debug name for Vulkan object");
    }
#endif
}

void render_device::set_debug_name(VkObjectType vkObjectType,
    void* vkObjectHandle, const char* name)
{
    in_vulkan_set_debug_name(m_vkDevice, vkObjectType,
        (uint64_t)vkObjectHandle, name);
}

static bool in_vulkan_load_debug_utils_ext(VkInstance vkInstance)
{
#ifdef VK_EXT_debug_utils
    const auto setDebugUtilsObjectNamePtr = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
        vkGetInstanceProcAddr(vkInstance, "vkSetDebugUtilsObjectNameEXT"));

    if (setDebugUtilsObjectNamePtr)
    {
        in_vulkan_set_debug_utils_name_ptr = setDebugUtilsObjectNamePtr;
        return true;
    }
#endif

    HR_LOG_WARN("Vulkan debug utilities were requested but are not supported");
    return false;
}

constexpr float in_vulkan_queue_family_default_priority = 1.0f;

constexpr VkPhysicalDeviceFeatures in_vulkan_get_phy_dev_features() noexcept
{
    VkPhysicalDeviceFeatures vkPhyDevFeatures = { VK_FALSE };
    vkPhyDevFeatures.shaderClipDistance = VK_TRUE;
    return vkPhyDevFeatures;
}

static VkDevice in_vulkan_create_device(
    VkInstance vkInstance, VkPhysicalDevice vkPhyDev,
    const internal::in_queue_families& queueFamilies,
    bool isDebug, const char* debugName)
{
    using namespace internal;

    // Setup queue create infos.
    VkDeviceQueueCreateInfo vkQueueCreateInfos[in_queue_type::count] =
    {
        {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,                 // sType
            nullptr,                                                    // pNext
            0,                                                          // flags
            queueFamilies.graphics_family(),                            // queueFamilyIndex
            1,                                                          // queueCount
            &in_vulkan_queue_family_default_priority                    // pQueuePriorities
        }
    };

    uint32_t vkQueueCreateInfoCount = 1;
    if (queueFamilies.has_unique_family(in_queue_type::transfer))
    {
        vkQueueCreateInfos[vkQueueCreateInfoCount++] =
        {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,                 // sType
            nullptr,                                                    // pNext
            0,                                                          // flags
            queueFamilies.transfer_family(),                            // queueFamilyIndex
            1,                                                          // queueCount
            &in_vulkan_queue_family_default_priority                    // pQueuePriorities
        };
    }

    if (queueFamilies.has_unique_family(in_queue_type::present))
    {
        vkQueueCreateInfos[vkQueueCreateInfoCount++] =
        {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,                 // sType
            nullptr,                                                    // pNext
            0,                                                          // flags
            queueFamilies.present_family(),                             // queueFamilyIndex
            1,                                                          // queueCount
            &in_vulkan_queue_family_default_priority                    // pQueuePriorities
        };
    }

    // Generate Vulkan device create info.
    VkPhysicalDeviceTimelineSemaphoreFeatures vkPhyDevTimelineSemaphoreFeatures =
    {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES,  // sType
        nullptr,                                                        // pNext
        VK_TRUE                                                         // timelineSemaphores
    };

    const VkPhysicalDeviceFeatures2 vkPhyDevFeatures2 =
    {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,                   // sType
        &vkPhyDevTimelineSemaphoreFeatures,                             // pNext
        in_vulkan_get_phy_dev_features()                                // features
    };

    VkDeviceCreateInfo vkDeviceCreateInfo =
    {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,                           // sType
        &vkPhyDevFeatures2,                                             // pNext
        0U,                                                             // flags
        vkQueueCreateInfoCount,                                         // queueCreateInfoCount
        vkQueueCreateInfos,                                             // pQueueCreateInfos
        0U,                                                             // enabledLayerCount
        nullptr,                                                        // ppEnabledLayerNames
        in_vulkan_device_required_extension_count,                      // enabledExtensionCount
        in_vulkan_device_required_extensions,                           // ppEnabledExtensionNames
        nullptr                                                         // pEnabledFeatures
    };

    // Enable validation layers and debug utilities if requested and supported by the system.
    if (isDebug)
    {
        vkDeviceCreateInfo.enabledLayerCount = in_vulkan_validation_layer_count;
        vkDeviceCreateInfo.ppEnabledLayerNames = in_vulkan_validation_layers;
        in_vulkan_load_debug_utils_ext(vkInstance);
    }

    // Create Vulkan device.
    VkDevice vkDevice;
    if (vkCreateDevice(vkPhyDev, &vkDeviceCreateInfo, nullptr, &vkDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create Vulkan device");
    }

    // Set debug name if one was provided.
    if (debugName)
    {
        try
        {
            in_vulkan_set_debug_name(vkDevice, VK_OBJECT_TYPE_DEVICE,
                (uint64_t)vkDevice, debugName);
        }
        catch (...)
        {
            vkDestroyDevice(vkDevice, nullptr);
            throw;
        }
    }

    return vkDevice;
}

static VkPipelineCache in_vulkan_create_pipeline_cache(VkDevice vkDevice)
{
    const VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,                   // sType
        nullptr,                                                        // pNext
        0,                                                              // flags
        0,                                                              // initialDataSize
        nullptr                                                         // pInitialData
    };

    VkPipelineCache vkPipelineCache;
    if (vkCreatePipelineCache(vkDevice, &vkPipelineCacheCreateInfo,
        nullptr, &vkPipelineCache) != VK_SUCCESS)
    {
        throw std::runtime_error("Could not create Vulkan pipeline cache");
    }

    return vkPipelineCache;
}

static internal::in_swap_chain in_create_swap_chain_with_device_cleanup(
    VkSurfaceKHR vkSurface, VkPhysicalDevice vkPhyDev, VkDevice vkDevice,
    const internal::in_queue_families& queueFamilies, VkExtent2D vkExtent,
    uint32_t imageCount, bool vsync)
{
    try
    {
        return internal::in_swap_chain(vkSurface,
            vkPhyDev, vkDevice, queueFamilies,
            vkExtent, imageCount, vsync);
    }
    catch (const std::exception& ex)
    {
        vkDestroyDevice(vkDevice, nullptr);
        throw ex;
    }
}

render_device::render_device(const gfx::adapter& adapter,
    surface& surface, unsigned int width, unsigned int height,
    unsigned int prefFrameBufCount, bool vsync, const char* debugName) :

    m_vkDevice(in_vulkan_create_device(adapter.parent().handle(),
        adapter.handle(), adapter.queue_families(),
        adapter.parent().m_isDebug, debugName)),

    m_adapter(adapter),
    m_allocator(*this),
    m_vkPipelineCache(in_vulkan_create_pipeline_cache(m_vkDevice)),

    m_swapChain(in_create_swap_chain_with_device_cleanup(
        surface.m_vkSurface, adapter.handle(), m_vkDevice,
        adapter.queue_families(), { width, height },
        prefFrameBufCount, vsync))
{
    using namespace internal;

    // Get required queues.
    vkGetDeviceQueue(m_vkDevice, m_adapter.queue_families().graphics_family(),
        0U, &m_vkQueues[in_queue_type::graphics]);

    if (m_adapter.queue_families().has_unique_family(in_queue_type::transfer))
    {
        vkGetDeviceQueue(m_vkDevice, m_adapter.queue_families().transfer_family(),
            0U, &m_vkQueues[in_queue_type::transfer]);
    }
    else
    {
        m_vkQueues[in_queue_type::transfer] = m_vkQueues[in_queue_type::graphics];
    }

    if (m_adapter.queue_families().has_unique_family(in_queue_type::present))
    {
        vkGetDeviceQueue(m_vkDevice, m_adapter.queue_families().present_family(),
            0U, &m_vkQueues[in_queue_type::present]);
    }
    else
    {
        m_vkQueues[in_queue_type::present] = m_vkQueues[in_queue_type::graphics];
    }

    // Create per-frame data.
    m_frameCount = m_swapChain.vkSwapChainImageCount;

    try
    {
        m_frameData = in_create_per_t_array<in_per_frame_data>(
            m_frameCount, *this);
    }
    catch (const std::exception& ex)
    {
        m_allocator.destroy();
        m_swapChain.destroy(m_vkDevice);
        vkDestroyDevice(m_vkDevice, nullptr);
        throw ex;
    }

    // Create per-thread data.
    m_threadCount = 1; // TODO: Set to actual thread count!

    try
    {
        m_threadData = in_create_per_t_array<in_per_thread_data>(
            m_threadCount, *this);
    }
    catch (const std::exception& ex)
    {
        in_destroy_per_t_array(*this, m_frameData, m_frameCount);
        m_allocator.destroy();
        m_swapChain.destroy(m_vkDevice);
        vkDestroyDevice(m_vkDevice, nullptr);
        throw ex;
    }

    // Create per-thread-per-frame data.
    try
    {
        m_frameThreadData = in_create_per_t_array<in_per_frame_thread_data>(
            m_frameCount * m_threadCount, *this, m_adapter.queue_families());
    }
    catch (const std::exception& ex)
    {
        in_destroy_per_t_array(*this, m_threadData, m_threadCount);
        in_destroy_per_t_array(*this, m_frameData, m_frameCount);
        m_allocator.destroy();
        m_swapChain.destroy(m_vkDevice);
        vkDestroyDevice(m_vkDevice, nullptr);
        throw ex;
    }

    // Create Vulkan timeline semaphore for keeping track of completed upload batches.
    const VkSemaphoreTypeCreateInfo vkSemaphoreTypeCreateInfo =
    {
        VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,                   // sType
        nullptr,                                                        // pNext
        VK_SEMAPHORE_TYPE_TIMELINE,                                     // semaphoreType
        0                                                               // initialValue
    };

    const VkSemaphoreCreateInfo vkSemaphoreCreateInfo =
    {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,                        // sType
        &vkSemaphoreTypeCreateInfo,                                     // pNext
        0                                                               // flags
    };

    if (vkCreateSemaphore(m_vkDevice, &vkSemaphoreCreateInfo,
        nullptr, &m_vkUploadCompleteSemaphore) != VK_SUCCESS)
    {
        in_destroy_per_t_array(*this, m_frameThreadData, m_frameCount * m_threadCount);
        in_destroy_per_t_array(*this, m_threadData, m_threadCount);
        in_destroy_per_t_array(*this, m_frameData, m_frameCount);
        m_allocator.destroy();
        m_swapChain.destroy(m_vkDevice);
        vkDestroyDevice(m_vkDevice, nullptr);

        throw std::runtime_error("Could not create Vulkan timeline semaphore");
    }
}
} // gfx
} // hr
