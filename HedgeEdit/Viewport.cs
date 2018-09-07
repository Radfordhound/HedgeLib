using HedgeLib.Models;
using System;
using System.Collections.Generic;
using System.Windows.Forms;
using SharpDX;
using SharpDX.Direct3D11;
using SharpDX.DXGI;
using SharpDX.Direct3D;
using HedgeEdit.D3D;

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

        public static bool IsMovingCamera
        {
            get => (isMovingCamera && vp.Focused);
            set => isMovingCamera = value;
        }

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

        private static VShader currentVShader;
        private static PShader currentPShader;
        private static Matrix view, proj, viewProj;
        private static SharpDX.Viewport viewport;
        private static Control vp;
        private static Point prevMousePos = Point.Empty;
        //private static MouseState prevMouseState;
        private static Vector3 camPos = new Vector3(0, 0, 5), camRot;
        private static Vector3 camUp = Vector3.Up;
        private static bool isMovingCamera = false;

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

            // Set out current shaders and call OnResize to finalise viewport
            currentVShader.Use(Context);
            currentPShader.Use(Context);
            OnResize();
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

            // Setup Projection Matrix
            proj = Matrix.PerspectiveFovRH(MathUtil.DegreesToRadians(FOV),
                vp.Width / (float)vp.Height, NearDistance, FarDistance);
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

            // Get mouse world coordinates/direction
            //ViewProjection.Invert();
            Matrix.Invert(ref viewProj, out Matrix vpInv);
            var near = UnProject(0);

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
                var far = UnProject(1);
                var direction = (far - near);
                direction.Normalize(); // TODO: Is NormalizeFast accurate enough?
                //var mp = new Vector3(vpMousePos.X, vpMousePos.Y, 0f);
                //viewport.Unproject(ref mp, ref vpInv, out Vector3 near);

                //mp.Z = 0.5f;
                //viewport.Unproject(ref mp, ref vpInv, out Vector3 far);

                //var direction = (far - near);
                //Console.WriteLine(p);
                //Console.WriteLine($"far: {far}");
                //Console.WriteLine($"near: {near}");
                //direction.Normalize();
                //Console.WriteLine($"dir: {direction}");

                // Check for Transform Gizmo clicks first, then object clicks
                //if (!Gizmo.Click(near, direction))
                //{
                    // Fire a ray from mouse coordinates in camera direction and
                    // select any object that ray comes in contact with.
                    if (!SelectObject(Data.DefaultCube))
                    {
                        foreach (var obj in Data.Objects)
                        {
                            SelectObject(obj.Value);
                        }
                    }
                //}

                // Sub-Methods
                bool SelectObject(VPModel mdl)
                {
                    // TODO: Fix farther objects being selected first due to dictionary order
                    var instance = mdl.InstanceIntersects(near, direction);
                    if (instance != null && instance.CustomData != null)
                    {
                        if (!Input.IsInputDown(Inputs.MultiSelect))
                            SelectedInstances.Clear();

                        if (!SelectedInstances.Contains(instance))
                        {
                            SelectedInstances.Add(instance);
                            Program.MainForm.RefreshGUI();
                            return true;
                        }
                    }

                    if (Program.MainForm.Focused)
                        SelectedInstances.Clear();

                    return false;
                }
            }

            // Sub-Methods
            Vector3 UnProject(float z)
            {
                // This method was hacked together from
                // a bunch of StackOverflow posts lol
                var vec = new Vector4()
                {
                    X = 2.0f * vpMousePos.X / vp.Width - 1,
                    Y = -(2.0f * vpMousePos.Y / vp.Height - 1),
                    Z = z,
                    W = 1.0f
                };

                Vector4.Transform(ref vec, ref vpInv, out vec);

                if (vec.W > float.Epsilon || vec.W < float.Epsilon)
                {
                    vec.X /= vec.W;
                    vec.Y /= vec.W;
                    vec.Z /= vec.W;
                }

                return new Vector3(vec.X, vec.Y, vec.Z);
            }
        }

        public static void Update()
        {
            if (isMovingCamera)
            {
                // Update camera transform
                var mousePos = Cursor.Position;
                var vpMousePos = vp.PointToClient(mousePos);

                float screenX = (float)vpMousePos.X / vp.Size.Width;
                float screenY = (float)vpMousePos.Y / vp.Size.Height;

                // Set Camera Rotation
                var mouseDifference = new Point(
                    mousePos.X - prevMousePos.X,
                    mousePos.Y - prevMousePos.Y);

                camRot.X += mouseDifference.X * 0.1f;
                camRot.Y -= mouseDifference.Y * 0.1f;

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

                // Snap cursor to center of viewport
                Cursor.Position =
                    vp.PointToScreen(new Point(vp.Width / 2, vp.Height / 2));

                // Update Transforms
                UpdateCameraForward();
                UpdateViewMatrix();
                Render();
            }

            prevMousePos = Cursor.Position;
        }

        public static void Render()
        {
            if (device == null)
                throw new Exception("Cannot render viewport - viewport not yet initialized!");

            if (RenderMode == RenderModes.Default)
            {
                // Update Constant Buffer Data
                viewProj = Matrix.Multiply(view, proj);
                Buffers.Default.CBDefault.Data.ViewProj = viewProj;

                // Send the new Constant Buffer to the GPU
                Buffers.Default.CBDefault.Update();
                Buffers.Default.CBDefault.VSSetConstantBuffer(0);
                Buffers.Default.CBDefault.PSSetConstantBuffer(0);

                // Clear the background color
                Context.ClearRenderTargetView(renderView, Color.Black);
                Context.ClearDepthStencilView(depthView, DepthStencilClearFlags.Depth, 1, 0);
            }
            else if (RenderMode == RenderModes.HedgehogEngine2)
            {
                // Update Constant Buffer Data
                Buffers.HE2.CBWorld.Data.g_LightScatteringColor = new Vector4(
                    0.039f, 0.274f, 0.549f, 1);

                Buffers.HE2.CBWorld.Data.g_tonemap_param = new Vector4(
                    1, 1, 1, 1);

                Buffers.HE2.CBWorld.Data.g_debug_option = Vector4.One;
                Buffers.HE2.CBWorld.Data.prev_view_proj_matrix = ViewProjection;
                Buffers.HE2.CBWorld.Data.proj_matrix = proj;
                Buffers.HE2.CBWorld.Data.view_matrix = view;
                Buffers.HE2.CBWorld.Data.u_cameraPosition = new Vector4(CameraPos, 1);

                // TODO: FIGURE THIS CRAP OUT
                Buffers.HE2.CBWorld.Data.g_LightScatteringFarNearScale = new Vector4(1, 1, 1, 1);
                Buffers.HE2.CBWorld.Data.g_LightScattering_Ray_Mie_Ray2_Mie2 = new Vector4(
                    0.58f, 1, 0.58f, 1);

                Buffers.HE2.CBWorld.Data.u_lightDirection = new Vector4(
                    -0.2991572f, -0.8605858f, 0.4121857f, 1);

                Buffers.HE2.CBWorld.Data.g_LightScattering_ConstG_FogDensity =
                    new Vector4(1, 1, 1, 1);

                // Send the new Constant Buffer to the GPU
                Buffers.HE2.CBWorld.Update();
                Buffers.HE2.CBWorld.VSSetConstantBuffer(0);
                Buffers.HE2.CBWorld.PSSetConstantBuffer(0);

                ViewProjection = Matrix.Multiply(view, proj);

                // Set OutputMerger Targets and Clear Depth Buffer
                // TODO: Deferred Rendering
                //Context.OutputMerger.SetTargets(depthView, renderViews);
                Context.ClearDepthStencilView(depthView, DepthStencilClearFlags.Depth, 1, 0);
            }

            // Draw all models in the scene
            Mesh.Slots slot;
            for (int i = 0; i < 4; ++i)
            {
                slot = (Mesh.Slots)i;
                Data.DefaultCube.Draw(slot);

                foreach (var mdl in Data.DefaultTerrainGroup)
                {
                    mdl.Value.Draw(slot);
                }

                foreach (var group in Data.TerrainGroups)
                {
                    foreach (var mdl in group.Value)
                    {
                        mdl.Value.Draw(slot);
                    }
                }

                foreach (var mdl in Data.Objects)
                {
                    mdl.Value.Draw(slot);
                }
            }

            ////int prevID = Shaders.ShaderPrograms["Preview"];
            ////GL.UseProgram(prevID);
            //Data.PreviewBox.Draw(defaultID, Mesh.Slots.Default, true);

            //// Draw Transform Gizmos
            //Gizmo.Render(defaultID);

            //// Swap our buffers
            ////GL.Flush();
            //vp.SwapBuffers();
            //prevMouseState = mouseState;
            swapChain.Present(0, PresentFlags.None);
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
    }
}