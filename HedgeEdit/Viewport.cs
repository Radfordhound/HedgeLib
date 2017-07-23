using HedgeEdit.Properties;
using HedgeLib.Models;
using OpenTK;
using OpenTK.Graphics.ES30;
using OpenTK.Input;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Windows.Forms;

namespace HedgeEdit
{
    public static class Viewport
    {
        //Variables/Constants
        public static List<HedgeEditModel> Models = new List<HedgeEditModel>();
        public static Model DefaultCube;
        public static Vector3 CameraPos = Vector3.Zero, CameraRot = Vector3.Zero;
        public static float FOV = 40.0f, NearDistance = 0.1f, FarDistance = 1000f;
        public static bool MovingCamera = false;

        private static GLControl vp = null;
        private static Point prevMousePos = Point.Empty;

        //Methods
        public static void Init(GLControl viewport)
        {
            vp = viewport;
            GL.Enable(EnableCap.DepthTest);

            // Load the shaders
            Shaders.LoadAll();

            // TODO: Remove the following debug stuff
            // YES I KNOW THIS IS TRASH LOL
            var watch = System.Diagnostics.Stopwatch.StartNew();
            var mesh = new Mesh();

            using (var reader = new StringReader(Resources.DefaultCube))
            {
                // Vertices
                var vertices = reader.ReadLine().Split(',');
                mesh.Vertices = new float[vertices.Length];

                for (int i = 0; i < vertices.Length; ++i)
                {
                    mesh.Vertices[i] = float.Parse(vertices[i]);
                }

                // Normals
                var normals = reader.ReadLine().Split(',');
                mesh.Normals = new float[normals.Length];

                for (int i = 0; i < normals.Length; ++i)
                {
                    mesh.Normals[i] = float.Parse(normals[i]);
                }

                // Indices
                var indices = reader.ReadLine().Split(',');
                mesh.Triangles = new uint[indices.Length];

                for (int i = 0; i < indices.Length; ++i)
                {
                    mesh.Triangles[i] = uint.Parse(indices[i]) - 1;
                }
            }

            DefaultCube = new Model(mesh);
            watch.Stop();
            Console.WriteLine("Debug model init time: {0}", watch.ElapsedMilliseconds);
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
            int defaultID = Shaders.ShaderPrograms["ColorTest"]; // TODO: Make this more efficient
            GL.UseProgram(defaultID);

            // Update camera transform
            var mouseState = Mouse.GetState();
            if (MovingCamera && mouseState.RightButton == OpenTK.Input.ButtonState.Pressed)
            {
                var vpMousePos = vp.PointToClient(Cursor.Position);
                float screenX = (float)vpMousePos.X / vp.Size.Width;
                float screenY = (float)vpMousePos.Y / vp.Size.Height;

                // Rotation
                var mouseDifference = new Point(
                    Cursor.Position.X - prevMousePos.X,
                    Cursor.Position.Y - prevMousePos.Y);

                CameraRot.X += mouseDifference.X * 0.1f;
                CameraRot.Y -= mouseDifference.Y * 0.1f;

                // Position
                var keyState = Keyboard.GetState();
                if (keyState.IsKeyDown(Key.W))
                {
                    ++CameraPos.Z;
                }
                else if (keyState.IsKeyDown(Key.S))
                {
                    --CameraPos.Z;
                }

                if (keyState.IsKeyDown(Key.A))
                {
                    ++CameraPos.X;
                }
                else if (keyState.IsKeyDown(Key.D))
                {
                    --CameraPos.X;
                }

                // Snap cursor to center of viewport
                Cursor.Position =
                    vp.PointToScreen(new Point(vp.Width / 2, vp.Height / 2));
            }

            // Update Transforms
            var view = Matrix4.CreateFromAxisAngle(new Vector3(0, 1, 0),
                MathHelper.DegreesToRadians(CameraRot.X));
            view *= Matrix4.CreateFromAxisAngle(new Vector3(-1, 0, 0),
                MathHelper.DegreesToRadians(CameraRot.Y));

            view *= Matrix4.CreateTranslation(CameraPos);

            var projection = Matrix4.CreatePerspectiveFieldOfView(
                MathHelper.DegreesToRadians(FOV),
                (float)vp.Width / vp.Height, NearDistance, FarDistance);

            prevMousePos = Cursor.Position;

            // Transform Gizmos
            //float screenX = (float)Math.Min(Math.Max(0,
            //    vpMousePos.X), vp.Size.Width) / vp.Size.Width;

            //float screenY = (float)Math.Min(Math.Max(0,
            //    vpMousePos.Y), vp.Size.Height) / vp.Size.Height;
            // TODO

            // Update shader transform matrices
            int viewLoc = GL.GetUniformLocation(defaultID, "view");
            int projectionLoc = GL.GetUniformLocation(defaultID, "projection");

            GL.UniformMatrix4(viewLoc, false, ref view);
            GL.UniformMatrix4(projectionLoc, false, ref projection);

            // Draw all models in the scene
            foreach (var mdl in Models)
            {
                mdl.Draw(defaultID);
            }

            // Swap our buffers
            vp.SwapBuffers();
        }

        public static void AddModel(Model mdl)
        {
            Models.Add(new HedgeEditModel(mdl));
        }

        public static void AddModel(Model mdl, Vector3 pos, Quaternion rot)
        {
            Models.Add(new HedgeEditModel(mdl, pos, rot));
        }

        public static void AddModel(Model mdl, HedgeLib.Vector3 pos, HedgeLib.Quaternion rot)
        {
            Models.Add(new HedgeEditModel(mdl, 
                Types.ToOpenTK(pos), Types.ToOpenTK(rot)));
        }

        public static void Clear()
        {
            Models.Clear();
        }
    }
}