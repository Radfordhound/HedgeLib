using HedgeLib.Models;
using OpenTK;
using OpenTK.Graphics.ES30;
using OpenTK.Input;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;

namespace HedgeEdit
{
    public static class Viewport
    {
        // Variables/Constants
        public static List<VPObjectInstance> SelectedInstances =
            new List<VPObjectInstance>();

        public static Vector3 CameraPos = Vector3.Zero, CameraRot = new Vector3(-90, 0, 0);
        public static Vector3 CameraForward { get; private set; } = new Vector3(0, 0, -1);
        public static float FOV = 40.0f, NearDistance = 0.1f, FarDistance = 1000000f;
        public static bool IsMovingCamera = false;

        private static GLControl vp = null;
        private static Point prevMousePos = Point.Empty;
        private static MouseState prevMouseState;
        private static Vector3 camUp = new Vector3(0, 1, 0);

        private static float camSpeed = normalSpeed;
        private const float normalSpeed = 1, fastSpeed = 8, slowSpeed = 0.25f;

        // Methods
        public static void Init(GLControl viewport)
        {
            vp = viewport;
            GL.Enable(EnableCap.DepthTest);

            // Load the shaders
            Shaders.LoadAll();

            // Set Texture Parameters
            // TODO: Are these necessary?
            GL.TexParameter(TextureTarget.Texture2D,
                TextureParameterName.TextureWrapS, (int)TextureWrapMode.Repeat);

            GL.TexParameter(TextureTarget.Texture2D,
                TextureParameterName.TextureWrapT, (int)TextureWrapMode.Repeat);

            GL.TexParameter(TextureTarget.Texture2D,
                TextureParameterName.TextureMinFilter, (int)TextureMinFilter.LinearMipmapLinear);

            GL.TexParameter(TextureTarget.Texture2D,
                TextureParameterName.TextureMagFilter, (int)TextureMagFilter.Linear);

            // Enable Blending
            GL.Enable(EnableCap.Blend);
            GL.BlendFunc(BlendingFactorSrc.SrcAlpha,
                BlendingFactorDest.OneMinusSrcAlpha);

            vp.MakeCurrent();
            vp.VSync = true;
        }

        public static void Resize(int width, int height)
        {
            GL.Viewport(0, 0, width, height);
        }

        public static void Render()
        {
            if (vp == null)
                throw new Exception("Cannot render viewport - viewport not yet initialized!");

            // Clear the background color
            GL.ClearColor(0, 0, 0, 1);
            GL.Clear(ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit);

            // Start using our "Default" program and bind our VAO
            int defaultID = Shaders.ShaderPrograms["Default"];
            GL.UseProgram(defaultID);

            // Update camera transform
            var keyState = Keyboard.GetState();
            var mouseState = Mouse.GetState();
            var mousePos = Cursor.Position;
            var vpMousePos = vp.PointToClient(mousePos);

            if (IsMovingCamera && mouseState.RightButton == OpenTK.Input.ButtonState.Pressed)
            {
                float screenX = (float)vpMousePos.X / vp.Size.Width;
                float screenY = (float)vpMousePos.Y / vp.Size.Height;

                // Set Camera Rotation
                var mouseDifference = new Point(
                    mousePos.X - prevMousePos.X,
                    mousePos.Y - prevMousePos.Y);

                CameraRot.X += mouseDifference.X * 0.1f;
                CameraRot.Y -= mouseDifference.Y * 0.1f;

                // Set Camera Movement Speed
                if (keyState.IsKeyDown(Key.ShiftLeft) ||
                    keyState.IsKeyDown(Key.ShiftRight))
                {
                    camSpeed = fastSpeed;
                }
                else if (keyState.IsKeyDown(Key.AltLeft) ||
                    keyState.IsKeyDown(Key.AltRight))
                {
                    camSpeed = slowSpeed;
                }
                else
                {
                    camSpeed = normalSpeed;
                }

                // Set Camera Position
                if (keyState.IsKeyDown(Key.W))
                {
                    CameraPos += camSpeed * CameraForward;
                }
                else if (keyState.IsKeyDown(Key.S))
                {
                    CameraPos -= camSpeed * CameraForward;
                }

                if (keyState.IsKeyDown(Key.A))
                {
                    CameraPos -= Vector3.Normalize(
                        Vector3.Cross(CameraForward, camUp)) * camSpeed;
                }
                else if (keyState.IsKeyDown(Key.D))
                {
                    CameraPos += Vector3.Normalize(
                        Vector3.Cross(CameraForward, camUp)) * camSpeed;
                }

                // Snap cursor to center of viewport
                Cursor.Position =
                    vp.PointToScreen(new Point(vp.Width / 2, vp.Height / 2));
            }

            // Update Transforms
            float x = MathHelper.DegreesToRadians(CameraRot.X);
            float y = MathHelper.DegreesToRadians(CameraRot.Y);
            float yCos = (float)Math.Cos(y);

            var front = new Vector3()
            {
                X = (float)Math.Cos(x) * yCos,
                Y = (float)Math.Sin(y),
                Z = (float)Math.Sin(x) * yCos
            };

            CameraForward = Vector3.Normalize(front);

            var view = Matrix4.LookAt(CameraPos,
                CameraPos + CameraForward, camUp);

            var projection = Matrix4.CreatePerspectiveFieldOfView(
                MathHelper.DegreesToRadians(FOV),
                (float)vp.Width / vp.Height, NearDistance, FarDistance);

            prevMousePos = Cursor.Position;

            // Update shader transform matrices
            int viewLoc = GL.GetUniformLocation(defaultID, "view");
            int projectionLoc = GL.GetUniformLocation(defaultID, "projection");

            GL.UniformMatrix4(viewLoc, false, ref view);
            GL.UniformMatrix4(projectionLoc, false, ref projection);

            // Object Selection
            if (mouseState.LeftButton == OpenTK.Input.ButtonState.Pressed &&
                prevMouseState.LeftButton == OpenTK.Input.ButtonState.Released &&
                vpMousePos.X >= 0 && vpMousePos.Y >= 0 && vpMousePos.X <= vp.Width &&
                vpMousePos.Y <= vp.Height && Program.MainForm.Active)
            {
                // Get mouse world coordinates/direction
                view.Invert();
                projection.Invert();

                var near = UnProject(0);
                var far = UnProject(1);
                var direction = (far - near);
                direction.Normalize(); // TODO: Is NormalizeFast accurate enough?

                // Fire a ray from mouse coordinates in camera direction and
                // select any object that ray comes in contact with.
                if (!SelectObject(Data.DefaultCube))
                {
                    foreach (var obj in Data.Objects)
                    {
                        SelectObject(obj.Value);
                    }
                }

                // Sub-Methods
                bool SelectObject(VPModel mdl)
                {
                    // TODO: Fix farther objects being selected first due to dictionary order
                    var instance = mdl.InstanceIntersects(near, direction);
                    if (instance != null && instance.CustomData != null)
                    {
                        if (!keyState.IsKeyDown(Key.LControl))
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

                    Vector4.Transform(ref vec, ref projection, out vec);
                    Vector4.Transform(ref vec, ref view, out vec);

                    if (vec.W > float.Epsilon || vec.W < float.Epsilon)
                    {
                        vec.X /= vec.W;
                        vec.Y /= vec.W;
                        vec.Z /= vec.W;
                    }

                    return vec.Xyz;
                }
            }

            // Transform Gizmos
            // float screenX = (float)Math.Min(Math.Max(0,
            //    vpMousePos.X), vp.Size.Width) / vp.Size.Width;

            // float screenY = (float)Math.Min(Math.Max(0,
            //    vpMousePos.Y), vp.Size.Height) / vp.Size.Height;
            // TODO

            // Draw all models in the scene
            for (int i = 0; i < 4; ++i)
            {
                var slot = (Mesh.Slots)i;
                Data.DefaultCube.Draw(defaultID, slot);

                foreach (var mdl in Data.DefaultTerrainGroup)
                {
                    mdl.Value.Draw(defaultID, slot);
                }

                foreach (var group in Data.TerrainGroups)
                {
                    foreach (var mdl in group.Value)
                    {
                        mdl.Value.Draw(defaultID, slot);
                    }
                }

                foreach (var mdl in Data.Objects)
                {
                    mdl.Value.Draw(defaultID, slot);
                }
            }

            // Swap our buffers
            //GL.Flush();
            vp.SwapBuffers();
            prevMouseState = mouseState;
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