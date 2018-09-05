using SharpDX;
using System.Runtime.InteropServices;

namespace HedgeEdit.D3D.BufferLayouts.HE2
{
    [StructLayout(LayoutKind.Explicit, Size = 3744)]
    public struct CBWorld : IBufferLayout
    {
        [FieldOffset(0)]
        public Matrix view_matrix;
        [FieldOffset(64)]
        public Matrix proj_matrix;
        [FieldOffset(128)]
        public Matrix inv_view_matrix;
        [FieldOffset(192)]
        public Matrix inv_proj_matrix;
        [FieldOffset(256)]
        public Matrix culling_proj_matrix;
        [FieldOffset(320)]
        public Matrix view_proj_matrix;
        [FieldOffset(384)]
        public Matrix inv_view_proj_matrix;
        [FieldOffset(448)]
        public Matrix prev_view_proj_matrix;
        [FieldOffset(512)]
        public Vector4 jitter_offset;
        [FieldOffset(528)]
        public Vector4 shadow_camera_view_matrix_third_row;
        [FieldOffset(544)]
        public Matrix shadow_view_matrix;
        [FieldOffset(608)]
        public Matrix shadow_view_proj_matrix;
        [FieldOffset(672), MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public Vector4[] shadow_map_parameter;
        [FieldOffset(704)]
        public Vector4 shadow_map_size;
        [FieldOffset(720), MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public Vector4[] shadow_cascade_offset;
        [FieldOffset(784), MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public Vector4[] shadow_cascade_scale;
        [FieldOffset(848)]
        public Vector4 shadow_cascade_frustums_eye_space_depth;
        [FieldOffset(864)]
        public Vector4 shadow_cascade_transition_scale;
        [FieldOffset(880)]
        public Matrix heightmap_view_matrix;
        [FieldOffset(944)]
        public Matrix heightmap_view_proj_matrix;
        [FieldOffset(1008)]
        public Vector4 heightmap_parameter;
        [FieldOffset(1024)]
        public Vector4 u_lightColor;
        [FieldOffset(1040)]
        public Vector4 u_lightDirection;
        [FieldOffset(1056)]
        public Vector4 u_cameraPosition;
        [FieldOffset(1072), MarshalAs(UnmanagedType.ByValArray, SizeConst = 72)]
        public Vector4[] g_probe_data;
        [FieldOffset(2224), MarshalAs(UnmanagedType.ByValArray, SizeConst = 24)]
        public Vector4[] g_probe_pos;
        [FieldOffset(2608), MarshalAs(UnmanagedType.ByValArray, SizeConst = 24)]
        public Vector4[] g_probe_param;
        [FieldOffset(2992)]
        public Vector4 g_probe_count;
        [FieldOffset(3008)]
        public Vector4 g_LightScattering_Ray_Mie_Ray2_Mie2;
        [FieldOffset(3024)]
        public Vector4 g_LightScattering_ConstG_FogDensity;
        [FieldOffset(3040)]
        public Vector4 g_LightScatteringFarNearScale;
        [FieldOffset(3056)]
        public Vector4 g_LightScatteringColor;
        [FieldOffset(3072)]
        public Vector4 g_alphathreshold;
        [FieldOffset(3088)]
        public Vector4 g_smoothness_param;
        [FieldOffset(3104)]
        public Vector4 g_time_param;
        [FieldOffset(3120)]
        public Vector4 g_billboard_guest_param;
        [FieldOffset(3136), MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public Vector4[] g_billboard_guest_look_position;
        [FieldOffset(3168)]
        public bool g_billboard_guest_enable_multi_direction;
        [FieldOffset(3172)]
        public bool g_billboard_guest_use_look_position;
        [FieldOffset(3184)]
        public Vector4 u_planar_projection_shadow_plane;
        [FieldOffset(3200)]
        public Vector3 u_planar_projection_shadow_light_position;
        [FieldOffset(3216)]
        public Vector4 u_planar_projection_shadow_color;
        [FieldOffset(3232)]
        public Vector4 u_planar_projection_shadow_param;
        [FieldOffset(3248)]
        public Vector4 g_global_user_param_0;
        [FieldOffset(3264)]
        public Vector4 g_global_user_param_1;
        [FieldOffset(3280)]
        public Vector4 g_global_user_param_2;
        [FieldOffset(3296)]
        public Vector4 g_global_user_param_3;
        [FieldOffset(3312)]
        public Vector4 g_tonemap_param;
        [FieldOffset(3328), MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
        public Vector4[] u_contrast_factor;
        [FieldOffset(3376)]
        public Vector4 u_hls_offset;
        [FieldOffset(3392)]
        public Vector4 u_hls_rgb;
        [FieldOffset(3408)]
        public bool enable_hls_correction;
        [FieldOffset(3424)]
        public Vector4 u_color_grading_factor;
        [FieldOffset(3440)]
        public Vector4 u_screen_info;
        [FieldOffset(3456)]
        public Vector4 u_viewport_info;
        [FieldOffset(3472)]
        public Vector4 u_view_param;
        [FieldOffset(3488), MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public Vector4[] u_sggi_param;
        [FieldOffset(3520)]
        public Vector4 u_histogram_param;
        [FieldOffset(3536), MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public Vector4[] u_occlusion_capsule_param;
        [FieldOffset(3568)]
        public Vector4 u_ssao_param;
        [FieldOffset(3584), MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public Vector4[] u_highlight_param;
        [FieldOffset(3616), MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public Vector4[] u_wind_param;
        [FieldOffset(3648)]
        public Vector4 u_wind_frequencies;
        [FieldOffset(3664)]
        public Vector4 u_grass_lod_distance;
        [FieldOffset(3680)]
        public bool enable_ibl_plus_directional_specular;
        [FieldOffset(3696)]
        public Vector4 g_debug_option;
        [FieldOffset(3712)]
        public Vector4 g_debug_param_float;
        [FieldOffset(3568)]
        public Int4 g_debug_param_int;

        public void Init()
        {
            shadow_map_parameter = new Vector4[2];
            shadow_cascade_offset = new Vector4[4];
            shadow_cascade_scale = new Vector4[2];
            g_probe_data = new Vector4[72];
            g_probe_pos = new Vector4[24];
            g_probe_param = new Vector4[24];
            g_billboard_guest_look_position = new Vector4[2];
            u_contrast_factor = new Vector4[3];
            u_sggi_param = new Vector4[2];
            u_occlusion_capsule_param = new Vector4[2];
            u_highlight_param = new Vector4[2];
            u_wind_param = new Vector4[2];
        }
    }

    [StructLayout(LayoutKind.Explicit, Size = 368)]
    public struct CBMaterialDynamic : IBufferLayout
    {
        [FieldOffset(0)]
        public Matrix world_matrix;
        [FieldOffset(64)]
        public Matrix prev_world_matrix;
        [FieldOffset(128), MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
        public Vector4[] light_field_color;
        [FieldOffset(256)]
        public Vector4 u_modulate_color;
        [FieldOffset(272)]
        public Vector4 u_view_offset;
        [FieldOffset(288)]
        public Vector4 u_model_user_param_0;
        [FieldOffset(304)]
        public Vector4 u_model_user_param_1;
        [FieldOffset(320)]
        public Vector4 u_model_user_param_2;
        [FieldOffset(336)]
        public Vector4 u_model_user_param_3;
        [FieldOffset(352)]
        public bool u_disable_max_bone_influences_8;
        [FieldOffset(356)]
        public bool u_enable_highlight_object;
        [FieldOffset(360)]
        public bool u_enable_highlight_chara;

        public void Init()
        {
            light_field_color = new Vector4[8];
        }
    }

    [StructLayout(LayoutKind.Explicit, Size = 96)]
    public struct CBMaterialAnimation : IBufferLayout
    {
        [FieldOffset(0)]
        public Vector4 diffuse_color;

        [FieldOffset(16)]
        public Vector4 ambient_color;

        [FieldOffset(32)]
        public Vector4 specular_color;

        [FieldOffset(48)]
        public Vector4 emissive_color;

        [FieldOffset(64)]
        public Vector4 opacity_color;

        [FieldOffset(80)]
        public Vector4 alpha_threshold;

        public void Init() { }
    }

    [StructLayout(LayoutKind.Explicit, Size = 16)]
    public struct CBMaterialStatic : IBufferLayout
    {
        [FieldOffset(0)]
        public Vector4 Luminance;

        public void Init() { }
    }

    [StructLayout(LayoutKind.Explicit, Size = 4112)]
    public struct CBSHLightFieldProbes : IBufferLayout
    {
        [FieldOffset(0)]
        public Vector4 shlightfield_default;
        [FieldOffset(16)]
        public Vector4 shlightfield_multiply_color_up;
        [FieldOffset(32)]
        public Vector4 shlightfield_multiply_color_down;
        [FieldOffset(48), MarshalAs(UnmanagedType.ByValArray, SizeConst = 18)]
        public Vector4[] shlightfield_probes_SHLightFieldProbe;
        [FieldOffset(4096)]
        public Vector4 shlightfield_probe_SHLightFieldProbe_end;

        public void Init()
        {
            shlightfield_probes_SHLightFieldProbe = new Vector4[18];
        }
    }

    [StructLayout(LayoutKind.Explicit, Size = 65408)]
    public struct CBLocalLightIndexData : IBufferLayout
    {
        // TODO: This is actually a UInt4. Will this be a problem?
        [FieldOffset(0), MarshalAs(UnmanagedType.ByValArray, SizeConst = 4088)]
        public Int4[] g_local_light_index_data;

        public void Init()
        {
            g_local_light_index_data = new Int4[4088];
        }
    }

    [StructLayout(LayoutKind.Explicit, Size = 64016)]
    public struct CBLocalLightContextData : IBufferLayout
    {
        [FieldOffset(0)]
        public Vector4 g_local_light_count;
        [FieldOffset(16), MarshalAs(UnmanagedType.ByValArray, SizeConst = 1000)]
        public Matrix[] g_local_light_data;

        public void Init()
        {
            g_local_light_data = new Matrix[1000];
        }
    }

    [StructLayout(LayoutKind.Explicit, Size = 2048)]
    public struct CBLocalLightTileData : IBufferLayout
    {
        // TODO: This is actually a UInt4. Will this be a problem?
        [FieldOffset(0), MarshalAs(UnmanagedType.ByValArray, SizeConst = 128)]
        public Int4[] g_local_light_tile_data;

        public void Init()
        {
            g_local_light_tile_data = new Int4[128];
        }
    }
}