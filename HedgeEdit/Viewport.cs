using HedgeLib.Models;
using System;
using System.Collections.Generic;
using System.Windows.Forms;
using SharpDX;
using SharpDX.Direct3D11;
using SharpDX.DXGI;
using SharpDX.Direct3D;
using HedgeEdit.D3D;
using SharpDX.Windows;
using System.Threading;

using Point = System.Drawing.Point;
using Device = SharpDX.Direct3D11.Device;

namespace HedgeEdit
{
    public static class Viewport
    {
        // Variables/Constants
        public static List<VPObjectInstance> SelectedInstances =
            new List<VPObjectInstance>();

        //public static TransformGizmo Gizmo = new TransformGizmo();
        public static VShader CurrentVShader
        {
            get => currentVShader;
            set
            {
                if (value == currentVShader)
                    return;

                currentVShader = value;
                currentVShader.Use(Context);
            }
        }

        public static PShader CurrentPShader
        {
            get => currentPShader;
            set
            {
                if (value == currentPShader)
                    return;

                currentPShader = value;
                currentPShader.Use(Context);
            }
        }

        public static Vector3 CameraPos
        {
            get => camPos;
            set
            {
                camPos = value;
                UpdateViewMatrix();
            }
        }

        public static Vector3 CameraRot
        {
            get => camRot;
            set
            {
                camRot = value;
                UpdateCameraForward();
                UpdateViewMatrix();
            }
        }

        public static Vector3 CameraForward { get; private set; } = Vector3.ForwardRH;
        public static float FOV = 40.0f, NearDistance = 0.1f, FarDistance = 10000f;
        public static bool IsMovingCamera, RenderOnNextFrame = true;

        public static Device Device => device;
        public static DeviceContext Context { get; private set; }
        public static InputAssemblerStage InputAssembler { get; private set; }
        public static RenderModes RenderMode { get; private set; } = RenderModes.Default;

        public enum RenderModes
        {
            Default, HedgehogEngine2
        }

        private static Device device;
        private static SwapChain swapChain;
        private static Texture2D backBuffer, depthBuffer;
        private static RenderTargetView renderView;
        private static DepthStencilView depthView;
        private static PreviewBox previewBox;

        private static VShader currentVShader;
        private static PShader currentPShader;
        private static Matrix view, proj, viewProj;
        private static SharpDX.Viewport viewport;
        private static Control vp;
        public static Point MouseDifference = Point.Empty, PrevMousePos = Point.Empty;
        //private static MouseState prevMouseState;
        private static Vector3 camPos = new Vector3(0, 0, 5), camRot;
        private static Vector3 camUp = Vector3.Up;

        public const int BufferCount = 2;
        private static float camSpeed = normalSpeed;
        private const float normalSpeed = 1, fastSpeed = 8, slowSpeed = 0.25f;

        // Methods
        public static void Init(Control control)
        {
            // Create a device and swap chain using the given width/height/handle
            vp = control;

            #if DEBUG
                var dcf = DeviceCreationFlags.Debug;
            #else
                var dcf = DeviceCreationFlags.None;
            #endif

            Device.CreateWithSwapChain(DriverType.Hardware,
                dcf, new SwapChainDescription()
                {
                    BufferCount = BufferCount,
                    IsWindowed = true,
                    Flags = SwapChainFlags.None,
                    OutputHandle = vp.Handle,
                    ModeDescription = new ModeDescription(vp.ClientSize.Width,
                        vp.ClientSize.Height, new Rational(60, 1), Format.R8G8B8A8_UNorm),

                    SampleDescription = new SampleDescription(1, 0),
                    SwapEffect = SwapEffect.Discard,
                    Usage = Usage.RenderTargetOutput
                },
                out device, out swapChain);

            // Setup our context reference, back buffer, and RenderTargetView
            Context = device.ImmediateContext;

            // Setup our InputElements
            var elements = InputElements.GetElements(RenderMode);

            // Load the shader list
            Data.LoadShaders(device, Data.ShadersDirectory, elements);

            // TODO: Load these separately instead and ensure they exist
            currentVShader = Data.VertexShaders["Default"];
            currentPShader = Data.PixelShaders["Default"];

            // Setup ConstantBuffers
            if (RenderMode != RenderModes.Default)
                Buffers.Init(device, RenderModes.Default);

            Buffers.Init(device, RenderMode);

            // Setup the Input Assembler
            InputAssembler = Context.InputAssembler;
            InputAssembler.PrimitiveTopology = PrimitiveTopology.TriangleList;

            // Setup the Sampler
            Context.PixelShader.SetSampler(0, new SamplerState(device,
                new SamplerStateDescription()
                {
                    Filter = Filter.MinMagMipLinear,
                    AddressU = TextureAddressMode.Wrap,
                    AddressV = TextureAddressMode.Wrap,
                    AddressW = TextureAddressMode.Wrap,
                    MaximumAnisotropy = 1,
                    MinimumLod = 0,
                    MaximumLod = float.MaxValue,
                }));

            // Setup our Preview Box
            previewBox = new PreviewBox(device);

            // Set out current shaders and call OnResize to finalise viewport
            currentVShader.Use(Context);
            currentPShader.Use(Context);
            OnResize();

            // Start our Update/Render loop
            var renderThread = new Thread(new ThreadStart(() =>
            {
                control.Invoke(new Action(() => {
                    RenderLoop.Run(control, Update, true); }));
            }));

            renderThread.Start();
        }

        public static void OnResize()
        {
            if (device == null)
                return;

            Utilities.Dispose(ref backBuffer);
            Utilities.Dispose(ref renderView);
            Utilities.Dispose(ref depthBuffer);
            Utilities.Dispose(ref depthView);

            swapChain.ResizeBuffers(BufferCount, vp.ClientSize.Width,
                vp.ClientSize.Height, Format.Unknown, SwapChainFlags.None);

            backBuffer = Texture2D.FromSwapChain<Texture2D>(swapChain, 0);
            renderView = new RenderTargetView(device, backBuffer);

            // Create the depth buffer
            depthBuffer = new Texture2D(device, new Texture2DDescription()
            {
                Format = Format.D24_UNorm_S8_UInt,
                ArraySize = 1,
                MipLevels = 1,
                Width = vp.ClientSize.Width,
                Height = vp.ClientSize.Height,
                SampleDescription = new SampleDescription(1, 0),
                Usage = ResourceUsage.Default,
                BindFlags = BindFlags.DepthStencil,
                CpuAccessFlags = CpuAccessFlags.None,
                OptionFlags = ResourceOptionFlags.None
            });

            // Create the depth buffer view
            depthView = new DepthStencilView(device, depthBuffer);

            // Setup targets and viewport for rendering
            viewport = new SharpDX.Viewport(0, 0,
                vp.ClientSize.Width, vp.ClientSize.Height, 0, 1.0f);

            Context.Rasterizer.SetViewport(viewport);
            Context.OutputMerger.SetTargets(depthView, renderView);

            var rasterizerDesc = RasterizerStateDescription.Default();
            rasterizerDesc.IsFrontCounterClockwise = true;
            Context.Rasterizer.State = new RasterizerState(device, rasterizerDesc);

            // Setup Matrices
            UpdateViewMatrix();
            proj = Matrix.PerspectiveFovRH(MathUtil.DegreesToRadians(FOV),
                vp.Width / (float)vp.Height, NearDistance, FarDistance);

            UpdateViewProjMatrix();
            RenderOnNextFrame = true;
        }

        public static VPObjectInstance SelectObject(object obj)
        {
            var instance = Data.GetObjectInstance(obj);
            if (instance == null)
                return null;

            SelectedInstances.Add(instance);
            return instance;
        }

        public static void Dispose()
        {
            if (device == null)
                return;

            // Dispose Shaders
            foreach (var shader in Data.VertexShaders)
            {
                shader.Value.Dispose();
            }

            foreach (var shader in Data.PixelShaders)
            {
                shader.Value.Dispose();
            }

            Data.VertexShaders.Clear();
            Data.PixelShaders.Clear();

            // Dispose Buffers
            Buffers.DisposeAll();

            // Dispose Models
            Data.DefaultCube.Dispose();
            foreach (var mdl in Data.DefaultTerrainGroup)
            {
                mdl.Value.Dispose();
            }

            foreach (var group in Data.TerrainGroups)
            {
                foreach (var mdl in group.Value)
                {
                    mdl.Value.Dispose();
                }
            }

            foreach (var mdl in Data.Objects)
            {
                mdl.Value.Dispose();
            }

            Data.DefaultTerrainGroup.Clear();
            Data.TerrainGroups.Clear();
            Data.Objects.Clear();

            // Dispose Textures
            foreach (var tex in Data.Textures)
            {
                tex.Value.Dispose();
            }

            Data.Textures.Clear();

            // Dispose Preview Box
            Utilities.Dispose(ref previewBox);

            // Dispose D3D stuff
            Utilities.Dispose(ref renderView);
            Utilities.Dispose(ref backBuffer);
            Utilities.Dispose(ref depthView);
            Utilities.Dispose(ref depthBuffer);

            Context.ClearState();
            Context.Flush();

            Utilities.Dispose(ref swapChain);
            Utilities.Dispose(ref device);
        }

        public static void Click()
        {
            var mousePos = Cursor.Position;
            var vpMousePos = vp.PointToClient(mousePos);

            //// Transform Gizmos
            //if (mouseState.LeftButton == OpenTK.Input.ButtonState.Released &&
            //    prevMouseState.LeftButton == OpenTK.Input.ButtonState.Pressed)
            //    Gizmo.IsMoving = false;

            //if (Gizmo.IsMoving)
            //    Gizmo.Update(near);

            // Object Selection
            if (vpMousePos.X >= 0 && vpMousePos.Y >= 0 && vpMousePos.X <= vp.Width &&
                vpMousePos.Y <= vp.Height && Program.MainForm.Active)
            {
                var ray = Ray.GetPickRay(vpMousePos.X,
                    vpMousePos.Y, viewport, viewProj);

                // Check for Transform Gizmo clicks first, then object clicks
                //if (!Gizmo.Click(near, direction))
                //{
                    // Fire a ray from mouse coordinates in camera direction and
                    // select any object that ray comes in contact with.
                    VPObjectInstance instance = null;
                    float instanceDistance = float.MaxValue;

                    GetClickedInstances(Data.DefaultCube);
                    foreach (var obj in Data.Objects)
                    {
                        GetClickedInstances(obj.Value);
                    }

                    if (instance != null)
                    {
                        if (!SelectedInstances.Contains(instance))
                        {
                            if (!Input.IsInputDown(Inputs.MultiSelect))
                                SelectedInstances.Clear();

                            SelectedInstances.Add(instance);
                            Program.MainForm.RefreshGUI();
                            Render();
                        }
                    }
                    else if (SelectedInstances.Count > 0)
                    {
                        SelectedInstances.Clear();
                        Program.MainForm.RefreshGUI();
                        Render();
                    }

                    // Sub-Methods
                    void GetClickedInstances(VPModel mdl)
                    {
                        var inst = mdl.InstanceIntersects(ref ray, out float distance);
                        if (inst != null && inst.CustomData != null &&
                            distance <= instanceDistance)
                        {
                            instance = inst;
                            instanceDistance = distance;
                        }
                    }
                //}
            }
        }

        public static void Update()
        {
            if (IsMovingCamera)
            {
                // Set Camera Movement Speed
                if (Input.IsInputDown(Inputs.Fast))
                {
                    camSpeed = fastSpeed;
                }
                else if (Input.IsInputDown(Inputs.Slow))
                {
                    camSpeed = slowSpeed;
                }
                else
                {
                    camSpeed = normalSpeed;
                }

                // Set Camera Position
                if (Input.IsInputDown(Inputs.Up))
                {
                    camPos += camSpeed * CameraForward;
                }
                else if (Input.IsInputDown(Inputs.Down))
                {
                    camPos -= camSpeed * CameraForward;
                }

                if (Input.IsInputDown(Inputs.Left))
                {
                    camPos -= Vector3.Normalize(
                        Vector3.Cross(CameraForward, camUp)) * camSpeed;
                }
                else if (Input.IsInputDown(Inputs.Right))
                {
                    camPos += Vector3.Normalize(
                        Vector3.Cross(CameraForward, camUp)) * camSpeed;
                }

                // Update Transforms
                UpdateCameraForward();
                UpdateViewMatrix();
                Render();
            }
            else if (RenderOnNextFrame)
            {
                Render();
            }

            RenderOnNextFrame = false;
        }

        public static void Render()
        {
            if (device == null)
                throw new Exception("Cannot render viewport - viewport not yet initialized!");

            // Clear the background color
            Context.ClearRenderTargetView(renderView, Color.Black);
            Context.ClearDepthStencilView(depthView, DepthStencilClearFlags.Depth, 1, 0);

            // Update Constant Buffer data
            UpdateBuffersFirstPass();

            // Render the Scene
            if (RenderMode == RenderModes.Default)
            {
                // Set Vertex Shader and Render Scene
                CurrentVShader = Data.VertexShaders["Default"];
                RenderScene();
            }
            else if (RenderMode == RenderModes.HedgehogEngine2)
            {
                // TODO: Finish Deferred stuff and commit it lol

                // Setup First Pass
                CurrentVShader = Data.VertexShaders["common_vs"];
                //deferredBuffers.PreRender(Context);

                // Render First Pass
                RenderScene();

                // Setup Second Pass
                Context.OutputMerger.SetTargets(depthView, renderView);
                //for (int i = 0; i < 13; ++i)
                //{
                //    Context.PixelShader.SetSampler(i, sampler);
                //}

                //CurrentVShader = Data.VertexShaders["test"];
                //CurrentPShader = Data.PixelShaders["test"];
                //Context.PixelShader.SetShaderResources(0, deferredBuffers.ShaderResourceViews);
                UpdateBuffersSecondPass();
                
                // Render Second Pass
                //Data.SecondPassQuad.Draw(Mesh.Slots.Default, true); // TODO

                // Unbind PixelShader resources
                //for (int i = 0; i < deferredBuffers.ShaderResourceViews.Length; ++i)
                //{
                //    Context.PixelShader.SetShaderResource(i, null);
                //}
            }

            ////int prevID = Shaders.ShaderPrograms["Preview"];
            ////GL.UseProgram(prevID);
            //Data.PreviewBox.Draw(defaultID, Mesh.Slots.Default, true);

            // Draw Transform Gizmos
            //Gizmo.Render(defaultID);
            // TODO

            // Present our finalized image
            swapChain.Present(1, PresentFlags.UseDuration);
        }

        private static void UpdateCameraForward()
        {
            float x = MathUtil.DegreesToRadians(camRot.X);
            float y = MathUtil.DegreesToRadians(camRot.Y);
            float yCos = (float)Math.Cos(y);

            CameraForward = Vector3.Normalize(new Vector3()
            {
                X = (float)Math.Sin(x) * yCos,
                Y = (float)Math.Sin(y),
                Z = -(float)Math.Cos(x) * yCos
            });
        }

        private static void UpdateViewMatrix()
        {
            view = Matrix.LookAtRH(camPos,
                camPos + CameraForward, camUp);
        }

        private static void UpdateViewProjMatrix()
        {
            viewProj = Matrix.Multiply(view, proj);
        }

        private static void UpdateBuffersFirstPass(bool updateMatrices = true)
        {
            if (RenderMode == RenderModes.HedgehogEngine2)
            {
                // Update CBWorld Constant Buffer
                if (updateMatrices)
                {
                    Buffers.HE2.CBWorld.Data.view_matrix = view;
                    Buffers.HE2.CBWorld.Data.proj_matrix = proj;
                    Buffers.HE2.CBWorld.Data.prev_view_proj_matrix = viewProj;
                    UpdateViewProjMatrix();
                    Buffers.HE2.CBWorld.Data.inv_view_matrix = Matrix.Invert(view);
                    Buffers.HE2.CBWorld.Data.inv_proj_matrix = Matrix.Invert(proj);
                    // TODO: culling_proj_matrix
                    Buffers.HE2.CBWorld.Data.view_proj_matrix = viewProj;
                    Buffers.HE2.CBWorld.Data.inv_view_proj_matrix = Matrix.Invert(viewProj);
                }

                Buffers.HE2.CBWorld.Data.u_cameraPosition = camPos;

                // TODO: Finish proper NeedleFxSceneData RFL loading and commit it
                //if (Stage.NeedleFxSceneData != null)
                //{
                //    var item = Stage.NeedleFxSceneData.Items[0];

                //    // TODO: jitter_offset
                //    // TODO: shadow_camera_view_matrix_third_row
                //    // TODO: shadow_view_matrix
                //    // TODO: shadow_view_proj_matrix
                //    // TODO: shadow_map_parameter[2]

                //    // TODO: Is this correct?
                //    var renderTarget = Stage.NeedleFxSceneData.Config.rendertarget;
                //    Buffers.HE2.CBWorld.Data.shadow_map_size = new Vector4(
                //        renderTarget.shadowMapWidth, renderTarget.shadowMapHeight,
                //        1, 1);

                //    // TODO: shadow_cascade_offset[4]
                //    // TODO: shadow_cascade_scale[4]
                //    // TODO: shadow_cascade_frustums_eye_space_depth
                //    // TODO: shadow_cascade_transition_scale
                //    // TODO: heightmap_view_matrix
                //    // TODO: heightmap_view_proj_matrix
                //    // TODO: heightmap_parameter
                //    // TODO: u_lightColor

                //    Buffers.HE2.CBWorld.Data.u_lightDirection = new Vector4(
                //        -0.2991572f, -0.8605858f, 0.4121857f, 1); // TODO

                //    // TODO: g_probe_data[72]
                //    // TODO: g_probe_pos[24]
                //    // TODO: g_probe_param[24]
                //    // TODO: g_probe_count

                //    // TODO: Is this correct?
                //    Buffers.HE2.CBWorld.Data.g_LightScattering_Ray_Mie_Ray2_Mie2 = new Vector4(
                //        item.lightscattering.rayleigh, item.lightscattering.mie,
                //        item.lightscattering.rayleigh, item.lightscattering.mie);

                //    Buffers.HE2.CBWorld.Data.g_LightScattering_ConstG_FogDensity =
                //        new Vector4(0.1f, 0.1f, 0.1f, 0.1f);

                //    Buffers.HE2.CBWorld.Data.g_LightScatteringFarNearScale = new Vector4(
                //        item.lightscattering.zfar, item.lightscattering.znear, 1, 1);

                //    var lsc = item.lightscattering.color;
                //    Buffers.HE2.CBWorld.Data.g_LightScatteringColor = new Vector4(
                //        lsc.X / 255f, lsc.Y / 255f, lsc.Z / 255f, 1);

                //    // TODO: g_alphathreshold
                //    // TODO: g_smoothness_param
                //    // TODO: g_time_param
                //    // TODO: g_billboard_guest_param
                //    // TODO: g_billboard_guest_look_position[2]
                //    // TODO: g_billboard_guest_enable_multi_direction
                //    // TODO: g_billboard_guest_use_look_position
                //    // TODO: u_planar_projection_shadow_plane
                //    // TODO: u_planar_projection_shadow_light_position
                //    // TODO: u_planar_projection_shadow_color
                //    // TODO: u_planar_projection_shadow_param
                //    // TODO: g_global_user_param_0
                //    // TODO: g_global_user_param_1
                //    // TODO: g_global_user_param_2
                //    // TODO: g_global_user_param_3

                //    Buffers.HE2.CBWorld.Data.g_tonemap_param = new Vector4(
                //        item.tonemap.middleGray, item.tonemap.lumMax,
                //        item.tonemap.lumMin, item.tonemap.adaptedRatio);

                //    // TODO: u_contrast_factor[3]
                //    Buffers.HE2.CBWorld.Data.u_hls_offset = new Vector4(
                //        item.colorContrast.hlsHueOffset,
                //        item.colorContrast.hlsLightnessOffset,
                //        item.colorContrast.hlsSaturationOffset, 1);

                //    // TODO: u_hls_rgb
                //    Buffers.HE2.CBWorld.Data.enable_hls_correction =
                //        item.colorContrast.useHlsCorrection;

                //    // TODO: u_color_grading_factor
                //    // TODO: u_screen_info
                //    // TODO: u_viewport_info
                //    // TODO: u_view_param

                //    Buffers.HE2.CBWorld.Data.u_sggi_param[0] = new Vector4(
                //        item.sggi.sgStartSmoothness, item.sggi.sgEndSmoothness, 0, 0);

                //    // TODO: The other u_sggi_param lol

                //    // TODO: u_histogram_param
                //    // TODO: u_occlusion_capsule_param[2]
                //    // TODO: u_ssao_param
                //    // TODO: u_highlight_param[2]
                //    // TODO: u_wind_param[2]

                //    Buffers.HE2.CBWorld.Data.u_wind_frequencies = new Vector4(
                //        item.sceneEnv.wind_frequencies);

                //    Buffers.HE2.CBWorld.Data.u_grass_lod_distance = Types.ToSharpDX(
                //        item.sceneEnv.grass_lod_distance);

                //    Buffers.HE2.CBWorld.Data.enable_ibl_plus_directional_specular =
                //        item.renderOption.debugIBLPlusDirectionalSpecular;
                //    Buffers.HE2.CBWorld.Data.g_debug_option = Vector4.Zero;

                //    // TODO: g_debug_param_float
                //    // TODO: g_debug_param_int
                //}

                // Send the new Constant Buffer to the GPU
                Buffers.HE2.CBWorld.Update();
                Buffers.HE2.CBWorld.VSSetConstantBuffer(0);
                Buffers.HE2.CBWorld.PSSetConstantBuffer(0);
            }
            else
            {
                // Update CBDefault Constant Buffer
                if (updateMatrices)
                {
                    UpdateViewProjMatrix();
                    Buffers.Default.CBDefault.Data.ViewProj = viewProj;

                    // Don't need to update the CB unless we changed it!
                    Buffers.Default.CBDefault.Update();
                }

                // Send the new Constant Buffer to the GPU
                Buffers.Default.CBDefault.VSSetConstantBuffer(0);
                Buffers.Default.CBDefault.PSSetConstantBuffer(0);
            }
        }

        private static void UpdateBuffersSecondPass()
        {
            // Update Constant Buffers for Second Pass
            if (RenderMode == RenderModes.HedgehogEngine2)
            {
                // TODO: shlightfield_default
                // TODO: shlightfield_multiply_color_up
                // TODO: shlightfield_multiply_color_down
                // TODO: shlightfield_probes_SHLightFieldProbe
                // TODO: shlightfield_probe_SHLightFieldProbe_end
                // TODO: g_local_light_index_data
                // TODO: g_local_light_count
                // TODO: g_local_light_data
                // TODO: g_local_light_tile_data

                // Send the new Constant Buffers to the GPU
                Buffers.HE2.CBSHLightFieldProbes.Update();
                Buffers.HE2.CBLocalLightIndexData.Update();
                Buffers.HE2.CBLocalLightContextData.Update();
                Buffers.HE2.CBLocalLightTileData.Update();

                Buffers.HE2.CBSHLightFieldProbes.PSSetConstantBuffer(6);
                Buffers.HE2.CBLocalLightIndexData.PSSetConstantBuffer(7);
                Buffers.HE2.CBLocalLightContextData.PSSetConstantBuffer(8);
                Buffers.HE2.CBLocalLightTileData.PSSetConstantBuffer(9);

                // Also set CBWorld again since we unset it to render the preview boxes
                Buffers.HE2.CBWorld.VSSetConstantBuffer(0);
                Buffers.HE2.CBWorld.PSSetConstantBuffer(0);
            }
        }

        private static void RenderScene(bool skipMaterials = false)
        {
            // Draw all models in the scene
            Mesh.Slots slot;
            for (int i = 0; i < 4; ++i)
            {
                slot = (Mesh.Slots)i;
                Data.DefaultCube.Draw(slot, skipMaterials);

                foreach (var mdl in Data.DefaultTerrainGroup)
                {
                    mdl.Value.Draw(slot, skipMaterials);
                }

                foreach (var group in Data.TerrainGroups)
                {
                    foreach (var mdl in group.Value)
                    {
                        mdl.Value.Draw(slot, skipMaterials);
                    }
                }

                foreach (var mdl in Data.Objects)
                {
                    mdl.Value.Draw(slot, skipMaterials);
                }
            }

            // Render selected instance boundries
            if (SelectedInstances.Count > 0)
            {
                // Change RenderMode and PrimitiveTopology
                var prevRenderMode = RenderMode;
                RenderMode = RenderModes.Default;
                InputAssembler.PrimitiveTopology = PrimitiveTopology.LineList;

                // Set Vertex/Pixel Shaders
                CurrentVShader = Data.VertexShaders["DefaultUntextured"];
                CurrentPShader = Data.PixelShaders["DefaultUntextured"];

                // Update Constant Buffers
                UpdateBuffersFirstPass((prevRenderMode != RenderModes.Default));

                // Render each preview box
                bool hasUpdated;
                DrawModelPreviewBoxes(Data.DefaultCube);

                foreach (var mdl in Data.DefaultTerrainGroup)
                {
                    DrawModelPreviewBoxes(mdl.Value);
                }

                foreach (var group in Data.TerrainGroups)
                {
                    foreach (var mdl in group.Value)
                    {
                        DrawModelPreviewBoxes(mdl.Value);
                    }
                }

                foreach (var mdl in Data.Objects)
                {
                    DrawModelPreviewBoxes(mdl.Value);
                }

                // Reset RenderMode and PrimitiveTopology
                RenderMode = prevRenderMode;
                InputAssembler.PrimitiveTopology = PrimitiveTopology.TriangleList;

                // Sub-Methods
                void DrawModelPreviewBoxes(VPModel mdl)
                {
                    hasUpdated = false;
                    foreach (var instance in mdl.Instances)
                    {
                        if (!SelectedInstances.Contains(instance))
                            continue;

                        if (!hasUpdated)
                        {
                            previewBox.Update(mdl.BoundingBox.GetCorners());
                            hasUpdated = true;
                        }

                        previewBox.Draw(instance);
                    }
                }
            }
        }
    }
}