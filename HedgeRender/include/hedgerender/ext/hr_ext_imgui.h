#ifndef HR_EXT_IMGUI_H_INCLUDED
#define HR_EXT_IMGUI_H_INCLUDED
#include <hedgerender/gfx/hr_pipeline.h>

#ifdef HR_EXT_IMGUI_IMPLEMENTATION
#define ImTextureID ::hr::gfx::shader_data
#include "backends/imgui_impl_glfw.h"
#include <hedgerender/gfx/hr_render_device.h>
#include <hedgerender/gfx/hr_resource.h>
#include <hedgerender/gfx/hr_shader.h>
#include <hedgelib/hl_internal.h>
#endif

struct ImGuiContext;
struct ImDrawData;

namespace hr
{
namespace gfx
{
class render_device;
class cmd_list;
class shader_data_allocator;
class image_view;
} // gfx

namespace ext
{
class imgui_instance : public non_copyable
{
    ImGuiContext* m_ctx = nullptr;

public:
    void destroy() noexcept;

    imgui_instance& operator=(imgui_instance&& other) noexcept;

    imgui_instance(gfx::render_device& device,
        GLFWwindow& window, bool installCallbacks = true);

    inline imgui_instance(imgui_instance&& other) noexcept :
        m_ctx(other.m_ctx)
    {
        other.m_ctx = nullptr;
    }

    inline ~imgui_instance()
    {
        destroy();
    }
};

gfx::shader_data imgui_create_texture(gfx::shader_data_allocator& allocator,
    gfx::image_view& imageView);

void imgui_create_resources();

gfx::pipeline imgui_create_pipeline(const gfx::render_graph& graph,
    std::uint32_t passID, std::uint32_t subpassID);

void imgui_new_frame();

void imgui_prepare_frame_resources(gfx::cmd_list& cmdList);

void imgui_draw(ImDrawData& drawData,
    const gfx::pipeline& pipeline, gfx::cmd_list& cmdList);

#ifdef HR_EXT_IMGUI_IMPLEMENTATION
#undef HR_EXT_IMGUI_IMPLEMENTATION

namespace internal
{
constexpr std::size_t in_imgui_max_buf_size = 1000000; // Give each buffer up to 1 MB each.
constexpr std::size_t in_imgui_max_vertices_count = (in_imgui_max_buf_size / sizeof(ImDrawVert));
constexpr std::size_t in_imgui_max_indices_count = (in_imgui_max_buf_size / sizeof(ImDrawIdx));

static const uint32_t in_imgui_vertex_shader_code[] =
{
    0x07230203,0x00010000,0x00080001,0x0000002e,0x00000000,0x00020011,0x00000001,0x0006000b,
    0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
    0x000a000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x0000000b,0x0000000f,0x00000015,
    0x0000001b,0x0000001c,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
    0x00000000,0x00030005,0x00000009,0x00000000,0x00050006,0x00000009,0x00000000,0x6f6c6f43,
    0x00000072,0x00040006,0x00000009,0x00000001,0x00005655,0x00030005,0x0000000b,0x0074754f,
    0x00040005,0x0000000f,0x6c6f4361,0x0000726f,0x00030005,0x00000015,0x00565561,0x00060005,
    0x00000019,0x505f6c67,0x65567265,0x78657472,0x00000000,0x00060006,0x00000019,0x00000000,
    0x505f6c67,0x7469736f,0x006e6f69,0x00030005,0x0000001b,0x00000000,0x00040005,0x0000001c,
    0x736f5061,0x00000000,0x00060005,0x0000001e,0x73755075,0x6e6f4368,0x6e617473,0x00000074,
    0x00050006,0x0000001e,0x00000000,0x61635375,0x0000656c,0x00060006,0x0000001e,0x00000001,
    0x61725475,0x616c736e,0x00006574,0x00030005,0x00000020,0x00006370,0x00040047,0x0000000b,
    0x0000001e,0x00000000,0x00040047,0x0000000f,0x0000001e,0x00000002,0x00040047,0x00000015,
    0x0000001e,0x00000001,0x00050048,0x00000019,0x00000000,0x0000000b,0x00000000,0x00030047,
    0x00000019,0x00000002,0x00040047,0x0000001c,0x0000001e,0x00000000,0x00050048,0x0000001e,
    0x00000000,0x00000023,0x00000000,0x00050048,0x0000001e,0x00000001,0x00000023,0x00000008,
    0x00030047,0x0000001e,0x00000002,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,
    0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040017,
    0x00000008,0x00000006,0x00000002,0x0004001e,0x00000009,0x00000007,0x00000008,0x00040020,
    0x0000000a,0x00000003,0x00000009,0x0004003b,0x0000000a,0x0000000b,0x00000003,0x00040015,
    0x0000000c,0x00000020,0x00000001,0x0004002b,0x0000000c,0x0000000d,0x00000000,0x00040020,
    0x0000000e,0x00000001,0x00000007,0x0004003b,0x0000000e,0x0000000f,0x00000001,0x00040020,
    0x00000011,0x00000003,0x00000007,0x0004002b,0x0000000c,0x00000013,0x00000001,0x00040020,
    0x00000014,0x00000001,0x00000008,0x0004003b,0x00000014,0x00000015,0x00000001,0x00040020,
    0x00000017,0x00000003,0x00000008,0x0003001e,0x00000019,0x00000007,0x00040020,0x0000001a,
    0x00000003,0x00000019,0x0004003b,0x0000001a,0x0000001b,0x00000003,0x0004003b,0x00000014,
    0x0000001c,0x00000001,0x0004001e,0x0000001e,0x00000008,0x00000008,0x00040020,0x0000001f,
    0x00000009,0x0000001e,0x0004003b,0x0000001f,0x00000020,0x00000009,0x00040020,0x00000021,
    0x00000009,0x00000008,0x0004002b,0x00000006,0x00000028,0x00000000,0x0004002b,0x00000006,
    0x00000029,0x3f800000,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,
    0x00000005,0x0004003d,0x00000007,0x00000010,0x0000000f,0x00050041,0x00000011,0x00000012,
    0x0000000b,0x0000000d,0x0003003e,0x00000012,0x00000010,0x0004003d,0x00000008,0x00000016,
    0x00000015,0x00050041,0x00000017,0x00000018,0x0000000b,0x00000013,0x0003003e,0x00000018,
    0x00000016,0x0004003d,0x00000008,0x0000001d,0x0000001c,0x00050041,0x00000021,0x00000022,
    0x00000020,0x0000000d,0x0004003d,0x00000008,0x00000023,0x00000022,0x00050085,0x00000008,
    0x00000024,0x0000001d,0x00000023,0x00050041,0x00000021,0x00000025,0x00000020,0x00000013,
    0x0004003d,0x00000008,0x00000026,0x00000025,0x00050081,0x00000008,0x00000027,0x00000024,
    0x00000026,0x00050051,0x00000006,0x0000002a,0x00000027,0x00000000,0x00050051,0x00000006,
    0x0000002b,0x00000027,0x00000001,0x00070050,0x00000007,0x0000002c,0x0000002a,0x0000002b,
    0x00000028,0x00000029,0x00050041,0x00000011,0x0000002d,0x0000001b,0x0000000d,0x0003003e,
    0x0000002d,0x0000002c,0x000100fd,0x00010038
};

static const uint32_t in_imgui_pixel_shader_code[] =
{
    0x07230203,0x00010000,0x00080001,0x0000001e,0x00000000,0x00020011,0x00000001,0x0006000b,
    0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
    0x0007000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x00000009,0x0000000d,0x00030010,
    0x00000004,0x00000007,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
    0x00000000,0x00040005,0x00000009,0x6c6f4366,0x0000726f,0x00030005,0x0000000b,0x00000000,
    0x00050006,0x0000000b,0x00000000,0x6f6c6f43,0x00000072,0x00040006,0x0000000b,0x00000001,
    0x00005655,0x00030005,0x0000000d,0x00006e49,0x00050005,0x00000016,0x78655473,0x65727574,
    0x00000000,0x00040047,0x00000009,0x0000001e,0x00000000,0x00040047,0x0000000d,0x0000001e,
    0x00000000,0x00040047,0x00000016,0x00000022,0x00000000,0x00040047,0x00000016,0x00000021,
    0x00000000,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,
    0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040020,0x00000008,0x00000003,
    0x00000007,0x0004003b,0x00000008,0x00000009,0x00000003,0x00040017,0x0000000a,0x00000006,
    0x00000002,0x0004001e,0x0000000b,0x00000007,0x0000000a,0x00040020,0x0000000c,0x00000001,
    0x0000000b,0x0004003b,0x0000000c,0x0000000d,0x00000001,0x00040015,0x0000000e,0x00000020,
    0x00000001,0x0004002b,0x0000000e,0x0000000f,0x00000000,0x00040020,0x00000010,0x00000001,
    0x00000007,0x00090019,0x00000013,0x00000006,0x00000001,0x00000000,0x00000000,0x00000000,
    0x00000001,0x00000000,0x0003001b,0x00000014,0x00000013,0x00040020,0x00000015,0x00000000,
    0x00000014,0x0004003b,0x00000015,0x00000016,0x00000000,0x0004002b,0x0000000e,0x00000018,
    0x00000001,0x00040020,0x00000019,0x00000001,0x0000000a,0x00050036,0x00000002,0x00000004,
    0x00000000,0x00000003,0x000200f8,0x00000005,0x00050041,0x00000010,0x00000011,0x0000000d,
    0x0000000f,0x0004003d,0x00000007,0x00000012,0x00000011,0x0004003d,0x00000014,0x00000017,
    0x00000016,0x00050041,0x00000019,0x0000001a,0x0000000d,0x00000018,0x0004003d,0x0000000a,
    0x0000001b,0x0000001a,0x00050057,0x00000007,0x0000001c,0x00000017,0x0000001b,0x00050085,
    0x00000007,0x0000001d,0x00000012,0x0000001c,0x0003003e,0x00000009,0x0000001d,0x000100fd,
    0x00010038
};

static gfx::sampler in_imgui_create_fonts_sampler(gfx::render_device& device)
{
    const gfx::sampler_desc samplerDesc =
    {
        VK_FILTER_LINEAR,                                               // magFilter
        VK_FILTER_LINEAR,                                               // minFilter
        VK_SAMPLER_MIPMAP_MODE_LINEAR,                                  // mipmapMode
        VK_SAMPLER_ADDRESS_MODE_REPEAT,                                 // addressModeU
        VK_SAMPLER_ADDRESS_MODE_REPEAT,                                 // addressModeV
        VK_SAMPLER_ADDRESS_MODE_REPEAT,                                 // addressModeW
        0.0f,                                                           // mipLodBias
        1.0f,                                                           // maxAnisotropy
        VK_COMPARE_OP_NEVER,                                            // compareOp
        -1000.0f,                                                       // minLod
        1000.0f,                                                        // maxLod
        VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,                        // borderColor
        false,                                                          // anisotropyEnable
        false,                                                          // compareEnable
        false                                                           // unnormalizedCoordinates
    };

    return gfx::sampler(device, samplerDesc);
}

static gfx::shader_parameter_group in_imgui_create_shader_param_group(
    gfx::render_device& device, const gfx::sampler& fontsSampler)
{
    const gfx::shader_parameter shaderParams[] =
    {
        {
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,                  // type
            VK_SHADER_STAGE_FRAGMENT_BIT,                               // shaderStages
            0,                                                          // firstRegisterIndex
            1,                                                          // registerCount
            &fontsSampler,                                              // immutableSamplers
            1                                                           // immutableSamplerCount
        }
    };

    const gfx::push_constant_range pushConstRanges[] =
    {
        {
            VK_SHADER_STAGE_VERTEX_BIT,                                 // shaderStages
            0,                                                          // offset
            4                                                           // count
        }
    };

    return gfx::shader_parameter_group(device,
        shaderParams, hl::count_of(shaderParams),
        pushConstRanges, hl::count_of(pushConstRanges));
}

struct in_imgui_renderer_data
{
    gfx::render_device* device;
    vk::ShaderModule vertexShader;
    vk::ShaderModule pixelShader;
    gfx::sampler fontsSampler;
    gfx::shader_parameter_group shaderParamGroup;
    gfx::pipeline_layout pipelineLayout;
    gfx::image fontsImage;
    std::atomic_uint64_t fontsImageUploadBatchID = {0};
    gfx::image_view fontsImageView;
    gfx::buffer vtxBuffer;
    gfx::buffer idxBuffer;
    ImDrawVert* mappedVtxBufData = nullptr;
    ImDrawIdx* mappedIdxBufData = nullptr;
    gfx::shader_data fontsShaderData = VK_NULL_HANDLE;

    void destroy() noexcept
    {
        if (device)
        {
            device->handle().destroy(vertexShader);
            device->handle().destroy(pixelShader);
        }
    }

    in_imgui_renderer_data(gfx::render_device& device) :
        device(&device),
        vertexShader(device.handle().createShaderModule(vk::ShaderModuleCreateInfo(
            vk::ShaderModuleCreateFlags(), sizeof(in_imgui_vertex_shader_code),
            in_imgui_vertex_shader_code))),

        pixelShader(device.handle().createShaderModule(vk::ShaderModuleCreateInfo(
            vk::ShaderModuleCreateFlags(), sizeof(in_imgui_pixel_shader_code),
            in_imgui_pixel_shader_code))),

        fontsSampler(in_imgui_create_fonts_sampler(device)),
        shaderParamGroup(in_imgui_create_shader_param_group(device, fontsSampler)),
        pipelineLayout(device, &shaderParamGroup, 1) {}

    ~in_imgui_renderer_data()
    {
        destroy();
    }
};

static in_imgui_renderer_data& in_imgui_get_renderer_data()
{
    IM_ASSERT(ImGui::GetCurrentContext());

    return *static_cast<in_imgui_renderer_data*>(
        ImGui::GetIO().BackendRendererUserData);
}

static void in_imgui_init_renderer(gfx::render_device& device)
{
    // Setup renderer backend.
    auto& io = ImGui::GetIO();
    IM_ASSERT(!io.BackendRendererUserData);

    auto backData = IM_NEW(in_imgui_renderer_data)(device);
    io.BackendRendererUserData = backData;
    io.BackendRendererName = "hedgerender";

    // NOTE: We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
}

static void in_imgui_shutdown_renderer() noexcept
{
    // Shutdown renderer backend.
    auto& io = ImGui::GetIO();
    auto backData = static_cast<in_imgui_renderer_data*>(
        io.BackendRendererUserData);

    io.BackendRendererName = nullptr;
    io.BackendRendererUserData = nullptr;

    IM_DELETE(backData);
}

static ImGuiContext& in_imgui_create_context()
{
    IMGUI_CHECKVERSION();
    return *ImGui::CreateContext();
}

static void in_imgui_setup_render_state(ImDrawData& drawData,
    const gfx::pipeline& pipeline, float viewportWidth,
    float viewportHeight, gfx::cmd_list& cmdList)
{
    // Bind pipeline.
    auto& backData = in_imgui_get_renderer_data();
    cmdList.bind_pipeline(pipeline);

    // Bind vertex/index buffers if necessary.
    if (drawData.TotalVtxCount > 0)
    {
        cmdList.bind_vertex_buffer(backData.vtxBuffer, 0, 0);
        cmdList.bind_index_buffer(backData.idxBuffer, 0, (sizeof(ImDrawIdx) == 2) ?
            VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
    }

    // Setup viewport.
    cmdList.set_viewport(0.0f, 0.0f, viewportWidth, viewportHeight);

    // Setup scale and translation.
    const float constants[4] =
    {
        2.0f / drawData.DisplaySize.x,                                  // scaleX
        2.0f / drawData.DisplaySize.y,                                  // scaleY
        (-1.0f - (drawData.DisplayPos.x * constants[0])),               // translateX
        (-1.0f - (drawData.DisplayPos.y * constants[1]))                // translateY
    };

    cmdList.push_constants(backData.pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT, 0, 4, constants);
}
} // internal

void imgui_instance::destroy() noexcept
{
    if (!m_ctx) return;

    // Shutdown ImGui renderer backend.
    internal::in_imgui_shutdown_renderer();

    // Shutdown ImGui platform backend.
    ImGui_ImplGlfw_Shutdown();

    // Destroy ImGui context.
    ImGui::DestroyContext(m_ctx);
}

imgui_instance& imgui_instance::operator=(imgui_instance&& other) noexcept
{
    if (&other != this)
    {
        destroy();

        m_ctx = other.m_ctx;
        other.m_ctx = nullptr;
    }

    return *this;
}

imgui_instance::imgui_instance(gfx::render_device& device,
    GLFWwindow& window, bool installCallbacks) :
    m_ctx(&internal::in_imgui_create_context())
{
    ImGui::SetCurrentContext(m_ctx);

    // Initialize platform backend.
    try
    {
        if (!ImGui_ImplGlfw_InitForVulkan(&window, installCallbacks))
        {
            throw std::runtime_error("Could not initialize ImGui GLFW platform backend");
        }
    }
    catch (const std::exception& ex)
    {
        ImGui::DestroyContext(m_ctx);
        throw ex;
    }

    // Initialize renderer backend.
    try
    {
        internal::in_imgui_init_renderer(device);
    }
    catch (const std::exception& ex)
    {
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(m_ctx);
        throw ex;
    }
}

gfx::shader_data imgui_create_texture(gfx::shader_data_allocator& allocator,
    gfx::image_view& imageView)
{
    auto& io = ImGui::GetIO();
    auto& backData = internal::in_imgui_get_renderer_data();

    // Allocate shader data.
    auto shaderData = allocator.allocate(backData.shaderParamGroup);

    // Update shader data.
    const gfx::image_write_desc imageWrite =
    {
        VK_NULL_HANDLE,                                                 // vkSampler
        imageView.handle(),                                             // vkImageView
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL                        // imageLayout
    };

    const gfx::shader_data_write_desc shaderDataWrite =
    {
        shaderData,                                                     // shaderData
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,                      // type
        0,                                                              // firstRegisterIndex
        0,                                                              // arrayElementIndex
        1,                                                              // registerCount
        &imageWrite,                                                    // imageWrites
        nullptr                                                         // bufferWrites
    };

    backData.device->update_shader_data(&shaderDataWrite, 1);
    return shaderData;
}

void imgui_create_resources()
{
    using namespace internal;

    auto& io = ImGui::GetIO();
    auto& backData = in_imgui_get_renderer_data();

    // Get ImGui fonts texture data.
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    // Create fonts image.
    backData.fontsImage = gfx::image(*backData.device,
        gfx::memory_type::gpu_only, VK_IMAGE_TYPE_2D,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VK_FORMAT_R8G8B8A8_UNORM, static_cast<unsigned int>(width),
        static_cast<unsigned int>(height), 1, 1, 1);

    // Create upload batch for fonts image data.
    auto uploadBatch = backData.device->start_upload_batch(0);
    backData.fontsImageUploadBatchID = uploadBatch.id();
    uploadBatch.add(pixels, backData.fontsImage);

    // Start uploading fonts image data to GPU.
    uploadBatch.submit();

    // Create fonts image view.
    backData.fontsImageView = gfx::image_view(*backData.device, backData.fontsImage);

    // Create vertex buffer.
    backData.vtxBuffer = gfx::buffer(*backData.device,
        gfx::memory_type::dynamic, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        in_imgui_max_vertices_count * sizeof(ImDrawVert),
        &backData.mappedVtxBufData);

    // Create index buffer.
    backData.idxBuffer = gfx::buffer(*backData.device,
        gfx::memory_type::dynamic, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        in_imgui_max_indices_count * sizeof(ImDrawIdx),
        &backData.mappedIdxBufData);

    // Allocate fonts shader data.
    auto allocator = backData.device->get_shader_data_allocator(false);
    backData.fontsShaderData = imgui_create_texture(allocator, backData.fontsImageView);

    // Wait for fonts image data to finish uploading, and cleanup the upload batch resources.
    backData.device->wait_for_upload_batch(0, backData.fontsImageUploadBatchID);

    // Set ImGui fonts texture ID.
    io.Fonts->SetTexID(backData.fontsShaderData);
}

gfx::pipeline imgui_create_pipeline(const gfx::render_graph& graph,
    std::uint32_t passID, std::uint32_t subpassID)
{
    using namespace internal;

    auto& backData = in_imgui_get_renderer_data();
    const gfx::input_element inputElements[] =
    {
        {
            VK_FORMAT_R32G32_SFLOAT,                                    // format
            0,                                                          // index
            static_cast<unsigned int>(IM_OFFSETOF(ImDrawVert, pos))     // offset
        },
        {
            VK_FORMAT_R32G32_SFLOAT,                                    // format
            1,                                                          // index
            static_cast<unsigned int>(IM_OFFSETOF(ImDrawVert, uv))      // offset
        },
        {
            VK_FORMAT_R8G8B8A8_UNORM,                                   // format
            2,                                                          // index
            static_cast<unsigned int>(IM_OFFSETOF(ImDrawVert, col))     // offset
        }
    };

    const gfx::input_layout inputLayouts[] =
    {
        {
            inputElements,                                              // elements
            hl::count_of(inputElements),                                // elementCount
            sizeof(ImDrawVert),                                         // stride
            0,                                                          // inputSlot
            VK_VERTEX_INPUT_RATE_VERTEX                                 // inputRate
        }
    };

    const gfx::pipeline_desc pipelineDesc =
    {
        backData.pipelineLayout.handle(),                               // layout
        backData.vertexShader,                                          // vertexShader
        gfx::default_shader_entry_point,                                // vertexShaderEntryPoint
        nullptr,                                                        // vsSpecializationInfo
        backData.pixelShader,                                           // pixelShader
        gfx::default_shader_entry_point,                                // pixelShaderEntryPoint
        nullptr,                                                        // psSpecializationInfo
        inputLayouts,                                                   // inputLayouts
        hl::count_of(inputLayouts),                                     // inputLayoutCount
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,                            // topologyType
        1,                                                              // viewportCount
        1,                                                              // scissorCount

        {                                                               // raster
            VK_POLYGON_MODE_FILL,                                       //  fillMode
            VK_CULL_MODE_NONE,                                          //  cullMode
            true,                                                       //  frontCounterClockwise
            false,                                                      //  depthBiasEnable
            0.0f,                                                       //  depthBias
            0.0f,                                                       //  depthBiasClamp
            0.0f,                                                       //  depthBiasSlope
        },

        {                                                               // depthStencil
            false,                                                      //  depthEnable
            false,                                                      //  depthWriteEnable
            false,                                                      //  stencilEnable
            VK_COMPARE_OP_NEVER,                                        //  depthCompareOp
            0,                                                          //  stencilReadMask
            0,                                                          //  stencilWriteMask
            gfx::stencil_op_state(),                                    //  front
            gfx::stencil_op_state()                                     //  back
        },

        {                                                               // blend
            false,                                                      //  logicOpEnable
            VK_LOGIC_OP_NO_OP                                           //  logicOp
        },

        passID,                                                         // passID
        subpassID                                                       // subpassID
    };

    return gfx::pipeline(graph, pipelineDesc);
}

void imgui_new_frame()
{
    // Update platform backend.
    ImGui_ImplGlfw_NewFrame();

    // Begin a new ImGui frame.
    ImGui::NewFrame();
}

void imgui_prepare_frame_resources(gfx::cmd_list& cmdList)
{
    using namespace internal;

    auto& backData = in_imgui_get_renderer_data();

    // Transition fonts image if necessary.
    const uint64_t fontsImageUploadBatchID = backData.fontsImageUploadBatchID.exchange(0);
    if (fontsImageUploadBatchID != 0)
    {
        cmdList.transition_image_layout(backData.fontsImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    }
}

void imgui_draw(ImDrawData& drawData, const gfx::pipeline& pipeline, gfx::cmd_list& cmdList)
{
    using namespace internal;

    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    const float fbWidth = static_cast<int>(drawData.DisplaySize.x *
        drawData.FramebufferScale.x);

    const float fbHeight = static_cast<int>(drawData.DisplaySize.y *
        drawData.FramebufferScale.y);

    if (fbWidth <= 0 || fbHeight <= 0) return;

    // Update vertex/index buffers if necessary.
    auto& backData = in_imgui_get_renderer_data();
    if (drawData.TotalVtxCount > 0)
    {
        // Ensure the total vertex/index count does not exceed the maximum.
        assert(drawData.TotalVtxCount < in_imgui_max_vertices_count &&
            "ImGui total vertices count exceeded the maximum allowed count");

        assert(drawData.TotalIdxCount < in_imgui_max_indices_count &&
            "ImGui total indices count exceeded the maximum allowed count");

        // Copy data to vertex/index buffers.
        ImDrawVert* vtxDst = backData.mappedVtxBufData;
        ImDrawIdx* idxDst = backData.mappedIdxBufData;

        for (int i = 0; i < drawData.CmdListsCount; ++i)
        {
            const ImDrawList* drawList = drawData.CmdLists[i];
            memcpy(vtxDst, drawList->VtxBuffer.Data, drawList->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idxDst, drawList->IdxBuffer.Data, drawList->IdxBuffer.Size * sizeof(ImDrawIdx));

            vtxDst += drawList->VtxBuffer.Size;
            idxDst += drawList->IdxBuffer.Size;
        }

        // Flush vertex/index buffers.
        VmaAllocation_T* const vmaAllocs[] =
        {
            backData.vtxBuffer.allocation(),
            backData.idxBuffer.allocation()
        };

        backData.device->allocator().flush(hl::count_of(vmaAllocs), vmaAllocs);
    }

    // Setup render state.
    in_imgui_setup_render_state(drawData,
        pipeline, fbWidth, fbHeight, cmdList);

    // Will project scissor/clipping rectangles into framebuffer space
    ImVec2 clipOff = drawData.DisplayPos; // (0,0) unless using multi-viewports
    ImVec2 clipScale = drawData.FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    int globalVtxOffset = 0, globalIdxOffset = 0;
    for (int i = 0; i < drawData.CmdListsCount; ++i)
    {
        const ImDrawList* drawList = drawData.CmdLists[i];
        for (int i2 = 0; i2 < drawList->CmdBuffer.Size; ++i2)
        {
            auto& draw = drawList->CmdBuffer[i2];
            if (draw.UserCallback)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                if (draw.UserCallback == ImDrawCallback_ResetRenderState)
                {
                    in_imgui_setup_render_state(drawData,
                        pipeline, fbWidth, fbHeight, cmdList);
                }
                else
                {
                    draw.UserCallback(drawList, &draw);
                }
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec2 clipMin((draw.ClipRect.x - clipOff.x) * clipScale.x,
                    (draw.ClipRect.y - clipOff.y) * clipScale.y);

                ImVec2 clipMax((draw.ClipRect.z - clipOff.x) * clipScale.x,
                    (draw.ClipRect.w - clipOff.y) * clipScale.y);

                // Clamp to viewport as Vulkan won't accept values that are out of bounds.
                clipMin.x = std::max<float>(clipMin.x, 0.0f);
                clipMin.y = std::max<float>(clipMin.y, 0.0f);
                clipMax.x = std::min<float>(clipMax.x, fbWidth);
                clipMax.y = std::min<float>(clipMax.y, fbHeight);

                if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
                    continue;

                // Apply scissor/clipping rectangle.
                cmdList.set_scissor(
                    static_cast<int>(clipMin.x), static_cast<int>(clipMin.y),
                    static_cast<unsigned int>(clipMax.x - clipMin.x),
                    static_cast<unsigned int>(clipMax.y - clipMin.y));

                // Bind shader data.
                cmdList.bind_shader_data(backData.pipelineLayout, draw.TextureId);

                // Draw vertices.
                cmdList.draw_indexed(draw.IdxOffset + globalIdxOffset,
                    draw.ElemCount, 0, 1, draw.VtxOffset + globalVtxOffset);
            }
        }

        globalIdxOffset += drawList->IdxBuffer.Size;
        globalVtxOffset += drawList->VtxBuffer.Size;
    }
}
#endif
} // ext
} // hr
#endif
