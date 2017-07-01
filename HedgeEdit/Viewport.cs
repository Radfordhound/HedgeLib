using HedgeLib.Models;
using OpenTK;
using OpenTK.Graphics.ES30;
using System;
using System.Collections.Generic;

namespace HedgeEdit
{
    public static class Viewport
    {
        //Variables/Constants
        public static List<HedgeEditModel> Models = new List<HedgeEditModel>();
        public static float FOV = 45.0f;

        private static GLControl vp = null;

        //TODO: Remove these
        private static float thing = 0;

        //Methods
        public static void Init(GLControl viewport)
        {
            vp = viewport;
            GL.Enable(EnableCap.DepthTest);

            // Load the shaders
            Shaders.LoadAll();

            // TODO: Remove the following debug stuff
            var watch = System.Diagnostics.Stopwatch.StartNew();
            var mesh = new Mesh()
            {
                Vertices = new float[]
                {
                    -0.1f,  0.5f, 0.0f,
                    -0.1f, -0.5f, 0.0f,
                    -0.5f, -0.5f, 0.0f,
                    -0.5f,  0.5f, 0.0f,
                    0.5f,  0.5f, 0.0f,
                    0.5f, -0.5f, 0.0f,
                    0f, -0.5f, 0.0f,
                    0f,  0.5f, 0.0f
                },

                Triangles = new uint[]
                {
                    0, 1, 3,
                    1, 2, 3,

                    4, 5, 7,
                    5, 6, 7
                }
            };

            Models.Add(new HedgeEditModel(new Model(mesh)));
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
            int defaultID = Shaders.ShaderPrograms["Default"]; // TODO: Make this more efficient
            GL.UseProgram(defaultID);

            // Update Transforms
            var view = Matrix4.CreateTranslation(new Vector3(0.0f, 0.0f, -3));
            view *= Matrix4.CreateFromAxisAngle(new Vector3(0, 0, 1), thing);

            var projection = Matrix4.CreatePerspectiveFieldOfView(
                MathHelper.DegreesToRadians(FOV),
                (float)vp.Width / vp.Height, 0.1f, 100.0f);

            thing += 0.01f;

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
    }
}