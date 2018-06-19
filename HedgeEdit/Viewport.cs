using HedgeLib.Models;
using System;
using System.Collections.Generic;
using System.Windows.Forms;
using SharpDX;
using SharpDX.Direct3D11;
using SharpDX.DXGI;
using SharpDX.Direct3D;
using SharpDX.Mathematics.Interop;

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
        public static Shaders.Shader CurrentShader;
        public static Vector3 CameraPos = new Vector3(0, 0, -5), CameraRot = new Vector3(0, 0, 0);//new Vector3(-90, 0, 0);
        public static Vector3 CameraForward { get; private set; } = new Vector3(0, 0, 1);//new Vector3(0, 0, -1);
        public static float FOV = 40.0f, NearDistance = 0.1f, FarDistance = 1000000f;
        public static bool IsMovingCamera = false;

        private static Device device;
        private static SwapChain swapChain;
        private static DeviceContext context;
        private static Texture2D backBuffer, depthBuffer;
        private static RenderTargetView renderView;
        private static DepthStencilView depthView;

        private static Control vp;
        private static Point prevMousePos = Point.Empty;
        //private static MouseState prevMouseState;
        private static Vector3 camUp = new Vector3(0, 1, 0);

        public const int BufferCount = 2;
        private static float camSpeed = normalSpeed;
        private const float normalSpeed = 1, fastSpeed = 8, slowSpeed = 0.25f;

        // Methods
        public static void Init(Control control)
        {
            // Create a device and swap chain using the given width/height/handle
            vp = control;
            Device.CreateWithSwapChain(DriverType.Hardware,
                DeviceCreationFlags.None, new SwapChainDescription()
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
            context = device.ImmediateContext;

            // Load the shaders
            Shaders.LoadAll(device);

            // Vertices
            var vertices = SharpDX.Direct3D11.Buffer.Create(device, BindFlags.VertexBuffer, new[]
            {
                -1.0f, -1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f, // Front
                -1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,
                 1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,
                 1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,
                 1.0f, -1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,

                -1.0f, -1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f, // BACK
                 1.0f,  1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,
                -1.0f,  1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,
                -1.0f, -1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,
                 1.0f, -1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,
                 1.0f,  1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,

                -1.0f, 1.0f, -1.0f,     0.0f, 0.0f, 1.0f, 1.0f, // Top
                -1.0f, 1.0f,  1.0f,     0.0f, 0.0f, 1.0f, 1.0f,
                 1.0f, 1.0f,  1.0f,     0.0f, 0.0f, 1.0f, 1.0f,
                -1.0f, 1.0f, -1.0f,     0.0f, 0.0f, 1.0f, 1.0f,
                 1.0f, 1.0f,  1.0f,     0.0f, 0.0f, 1.0f, 1.0f,
                 1.0f, 1.0f, -1.0f,     0.0f, 0.0f, 1.0f, 1.0f,

                -1.0f,-1.0f, -1.0f,     1.0f, 1.0f, 0.0f, 1.0f, // Bottom
                 1.0f,-1.0f,  1.0f,     1.0f, 1.0f, 0.0f, 1.0f,
                -1.0f,-1.0f,  1.0f,     1.0f, 1.0f, 0.0f, 1.0f,
                -1.0f,-1.0f, -1.0f,     1.0f, 1.0f, 0.0f, 1.0f,
                 1.0f,-1.0f, -1.0f,     1.0f, 1.0f, 0.0f, 1.0f,
                 1.0f,-1.0f,  1.0f,     1.0f, 1.0f, 0.0f, 1.0f,

                -1.0f, -1.0f, -1.0f,    1.0f, 0.0f, 1.0f, 1.0f, // Left
                -1.0f, -1.0f,  1.0f,    1.0f, 0.0f, 1.0f, 1.0f,
                -1.0f,  1.0f,  1.0f,    1.0f, 0.0f, 1.0f, 1.0f,
                -1.0f, -1.0f, -1.0f,    1.0f, 0.0f, 1.0f, 1.0f,
                -1.0f,  1.0f,  1.0f,    1.0f, 0.0f, 1.0f, 1.0f,
                -1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 1.0f, 1.0f,

                 1.0f, -1.0f, -1.0f,    0.0f, 1.0f, 1.0f, 1.0f, // Right
                 1.0f,  1.0f,  1.0f,    0.0f, 1.0f, 1.0f, 1.0f,
                 1.0f, -1.0f,  1.0f,    0.0f, 1.0f, 1.0f, 1.0f,
                 1.0f, -1.0f, -1.0f,    0.0f, 1.0f, 1.0f, 1.0f,
                 1.0f,  1.0f, -1.0f,    0.0f, 1.0f, 1.0f, 1.0f,
                 1.0f,  1.0f,  1.0f,    0.0f, 1.0f, 1.0f, 1.0f,
            });

            // Setup a layout
            CurrentShader = Shaders.ShaderPrograms["Default"];
            var layout = new InputLayout(device, CurrentShader.Signature, new[]
            {
                new InputElement("POSITION", 0, Format.R32G32B32_Float, 0, 0),
                new InputElement("COLOR", 0, Format.R32G32B32A32_Float, 12, 0)
            });

            // Setup our context
            context.InputAssembler.InputLayout = layout;
            context.InputAssembler.PrimitiveTopology = PrimitiveTopology.TriangleList;
            context.InputAssembler.SetVertexBuffers(0, new VertexBufferBinding(
                vertices, Utilities.SizeOf<Vector3>() + Utilities.SizeOf<Vector4>(), 0));

            vertices.Dispose();
            layout.Dispose(); // TODO: Is this ok??

            // Set out Current Shader and call OnResize to finalise viewport
            CurrentShader.Use(context);
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
                Format = Format.D32_Float_S8X24_UInt,
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
            context.Rasterizer.SetViewport(new SharpDX.Viewport(0, 0,
                vp.ClientSize.Width, vp.ClientSize.Height, 0.0f, 1.0f));
            context.OutputMerger.SetTargets(depthView, renderView);
        }

        public static void Dispose()
        {
            if (device == null)
                return;

            foreach (var shader in Shaders.ShaderPrograms)
            {
                shader.Value.Dispose();
            }

            depthBuffer.Dispose();
            depthView.Dispose();
            renderView.Dispose();
            backBuffer.Dispose();
            context.ClearState();
            context.Flush();
            device.Dispose();
            context.Dispose();
            swapChain.Dispose();
            //factory.Dispose(); // TODO
        }

        public static void Update()
        {
            if (IsMovingCamera)
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

                CameraRot.X += mouseDifference.X * 0.1f;
                CameraRot.Y -= mouseDifference.Y * 0.1f;

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
                    CameraPos += camSpeed * CameraForward;
                }
                else if (Input.IsInputDown(Inputs.Down))
                {
                    CameraPos -= camSpeed * CameraForward;
                }

                if (Input.IsInputDown(Inputs.Left))
                {
                    CameraPos += Vector3.Normalize(
                        Vector3.Cross(CameraForward, camUp)) * camSpeed;
                }
                else if (Input.IsInputDown(Inputs.Right))
                {
                    CameraPos -= Vector3.Normalize(
                        Vector3.Cross(CameraForward, camUp)) * camSpeed;
                }

                // Snap cursor to center of viewport
                Cursor.Position =
                    vp.PointToScreen(new Point(vp.Width / 2, vp.Height / 2));

                // Update Transforms
                float x = MathUtil.DegreesToRadians(CameraRot.X);
                float y = MathUtil.DegreesToRadians(CameraRot.Y);
                float yCos = (float)Math.Cos(y);

                var front = new Vector3()
                {
                    X = (float)Math.Sin(x) * yCos,
                    Y = (float)Math.Sin(y),
                    Z = (float)Math.Cos(x) * yCos
                };

                CameraForward = Vector3.Normalize(front);
                Render();
            }

            prevMousePos = Cursor.Position;
        }

        public static void Render()
        {
            if (device == null)
                throw new Exception("Cannot render viewport - viewport not yet initialized!");

            // Clear the background color
            context.ClearDepthStencilView(depthView, DepthStencilClearFlags.Depth, 1, 0);
            context.ClearRenderTargetView(renderView, Color.Black);
            // TODO

            //GL.Clear(ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit);

            //// Start using our "Default" program and bind our VAO
            //int defaultID = Shaders.ShaderPrograms["Default"];
            //GL.UseProgram(defaultID);

            var view = Matrix.LookAtLH(CameraPos, CameraPos + CameraForward, camUp);
            var proj = Matrix.PerspectiveFovLH((float)Math.PI / 4.0f,
                vp.Width / (float)vp.Height, NearDistance, FarDistance);
            //var view = Matrix4.LookAt(CameraPos,
            //    CameraPos + CameraForward, camUp);

            //var projection = Matrix4.CreatePerspectiveFieldOfView(
            //    MathHelper.DegreesToRadians(FOV),
            //    (float)vp.Width / vp.Height, NearDistance, FarDistance);

            //// Update shader transform matrices
            var worldViewProj = Matrix.Multiply(view, proj);
            worldViewProj.Transpose();
            context.UpdateSubresource(ref worldViewProj, CurrentShader.ConstantBuffer);
            //int viewLoc = GL.GetUniformLocation(defaultID, "view");
            //int projectionLoc = GL.GetUniformLocation(defaultID, "projection");

            //GL.UniformMatrix4(viewLoc, false, ref view);
            //GL.UniformMatrix4(projectionLoc, false, ref projection);

            //// Get mouse world coordinates/direction
            //view.Invert();
            //projection.Invert();
            //var near = UnProject(0);

            //// Transform Gizmos
            //if (mouseState.LeftButton == OpenTK.Input.ButtonState.Released &&
            //    prevMouseState.LeftButton == OpenTK.Input.ButtonState.Pressed)
            //    Gizmo.IsMoving = false;

            //if (Gizmo.IsMoving)
            //    Gizmo.Update(near);

            //// Object Selection
            //if (mouseState.LeftButton == OpenTK.Input.ButtonState.Pressed &&
            //    prevMouseState.LeftButton == OpenTK.Input.ButtonState.Released &&
            //    vpMousePos.X >= 0 && vpMousePos.Y >= 0 && vpMousePos.X <= vp.Width &&
            //    vpMousePos.Y <= vp.Height && Program.MainForm.Active)
            //{
            //    var far = UnProject(1);
            //    var direction = (far - near);
            //    direction.Normalize(); // TODO: Is NormalizeFast accurate enough?

            //    // Check for Transform Gizmo clicks first, then object clicks
            //    if (!Gizmo.Click(near, direction))
            //    {
            //        // Fire a ray from mouse coordinates in camera direction and
            //        // select any object that ray comes in contact with.
            //        if (!SelectObject(Data.DefaultCube))
            //        {
            //            foreach (var obj in Data.Objects)
            //            {
            //                SelectObject(obj.Value);
            //            }
            //        }
            //    }

            //    // Sub-Methods
            //    bool SelectObject(VPModel mdl)
            //    {
            //        // TODO: Fix farther objects being selected first due to dictionary order
            //        var instance = mdl.InstanceIntersects(near, direction);
            //        if (instance != null && instance.CustomData != null)
            //        {
            //            if (!keyState.IsKeyDown(Key.LControl))
            //                SelectedInstances.Clear();

            //            if (!SelectedInstances.Contains(instance))
            //            {
            //                SelectedInstances.Add(instance);
            //                Program.MainForm.RefreshGUI();
            //                return true;
            //            }
            //        }

            //        if (Program.MainForm.Focused)
            //            SelectedInstances.Clear();

            //        return false;
            //    }
            //}

            //// Draw all models in the scene
            //for (int i = 0; i < 4; ++i)
            //{
            //    var slot = (Mesh.Slots)i;
            //    Data.DefaultCube.Draw(defaultID, slot);

            //    foreach (var mdl in Data.DefaultTerrainGroup)
            //    {
            //        mdl.Value.Draw(defaultID, slot);
            //    }

            //    foreach (var group in Data.TerrainGroups)
            //    {
            //        foreach (var mdl in group.Value)
            //        {
            //            mdl.Value.Draw(defaultID, slot);
            //        }
            //    }

            //    foreach (var mdl in Data.Objects)
            //    {
            //        mdl.Value.Draw(defaultID, slot);
            //    }
            //}

            ////int prevID = Shaders.ShaderPrograms["Preview"];
            ////GL.UseProgram(prevID);
            //Data.PreviewBox.Draw(defaultID, Mesh.Slots.Default, true);
            context.Draw(36, 0);

            //// Draw Transform Gizmos
            //Gizmo.Render(defaultID);

            //// Swap our buffers
            ////GL.Flush();
            //vp.SwapBuffers();
            //prevMouseState = mouseState;
            swapChain.Present(0, PresentFlags.None);

            //// Sub-Methods
            //Vector3 UnProject(float z)
            //{
            //    // This method was hacked together from
            //    // a bunch of StackOverflow posts lol
            //    var vec = new Vector4()
            //    {
            //        X = 2.0f * vpMousePos.X / vp.Width - 1,
            //        Y = -(2.0f * vpMousePos.Y / vp.Height - 1),
            //        Z = z,
            //        W = 1.0f
            //    };

            //    Vector4.Transform(ref vec, ref projection, out vec);
            //    Vector4.Transform(ref vec, ref view, out vec);

            //    if (vec.W > float.Epsilon || vec.W < float.Epsilon)
            //    {
            //        vec.X /= vec.W;
            //        vec.Y /= vec.W;
            //        vec.Z /= vec.W;
            //    }

            //    return vec.Xyz;
            //}
        }

        public static VPObjectInstance SelectObject(object obj)
        {
            var instance = Data.GetObjectInstance(obj);
            if (instance == null)
                return null;

            SelectedInstances.Add(instance);
            return instance;
        }
    }
}