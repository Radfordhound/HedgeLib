using OpenTK;
using OpenTK.Graphics.ES30;
using System;

namespace HedgeEdit
{
    public static class Viewport
    {
        //Variables/Constants
        public static float FOV = 45.0f;
        private static GLControl vp = null;

        //TODO: Remove these
        private static float[] verts = new float[]
        {
            -0.1f,  0.5f, 0.0f,
            -0.1f, -0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            -0.5f,  0.5f, 0.0f,

            0.5f,  0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0f, -0.5f, 0.0f,
            0f,  0.5f, 0.0f
        };

        private static uint[] indices = new uint[]
        {
            0, 1, 3,
            1, 2, 3,

            4, 5, 7,
            5, 6, 7
        };

        private static float thing = 1;
        private static uint vao, vbo, ebo;

        //Methods
        public static void Init(GLControl viewport)
        {
            vp = viewport;

            // Load the shaders
            Shaders.LoadAll();

            // TODO: Put the following stuff somewhere else lol
            // Setup a vertex array object and vertex buffer object
            GL.GenVertexArrays(1, out vao);
            GL.GenBuffers(1, out vbo);

            // Bind the VAO and VBO
            GL.BindVertexArray(vao);
            GL.BindBuffer(BufferTarget.ArrayBuffer, vbo);

            // Send vertex data to the GPU
            GL.BufferData(BufferTarget.ArrayBuffer, verts.Length *
                sizeof(float), verts, BufferUsageHint.StaticDraw);

            // Set our vertex attribute pointers
            GL.VertexAttribPointer(0, 3, VertexAttribPointerType.Float,
                false, 3 * sizeof(float), 0);
            GL.EnableVertexAttribArray(0);

            // Generate an element buffer object
            GL.GenBuffers(1, out ebo);
            GL.BindBuffer(BufferTarget.ElementArrayBuffer, ebo);
            GL.BufferData(BufferTarget.ElementArrayBuffer, indices.Length *
                sizeof(uint), indices, BufferUsageHint.StaticDraw);

            // TODO: Un-bind these objects
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
            GL.Clear(ClearBufferMask.ColorBufferBit);

            // Start using our "Default" program and bind our VAO
            int defaultID = Shaders.ShaderPrograms["Default"]; // TODO: Make this more efficient
            GL.UseProgram(defaultID);
            //GL.BindVertexArray(vao); //TODO: Un-comment this

            // Update Transforms
            var pos = Matrix4.CreateTranslation(new Vector3(0, 0, 0));
            var rot = Matrix4.CreateFromAxisAngle(new Vector3(1.0f, 0.0f, 0.0f),
                thing * MathHelper.DegreesToRadians(-55.0f));

            var model = Matrix4.Add(pos, rot);
            var view = Matrix4.CreateTranslation(new Vector3(0.0f, 0.0f, -3.0f));

            var projection = Matrix4.CreatePerspectiveFieldOfView(
                MathHelper.DegreesToRadians(FOV),
                (float)vp.Width / vp.Height, 0.1f, 100.0f);

            thing += 0.1f;

            // Update shader transform matrices
            int modelLoc = GL.GetUniformLocation(defaultID, "model");
            int viewLoc = GL.GetUniformLocation(defaultID, "view");
            int projectionLoc = GL.GetUniformLocation(defaultID, "projection");

            GL.UniformMatrix4(modelLoc, false, ref model);
            GL.UniformMatrix4(viewLoc, false, ref view);
            GL.UniformMatrix4(projectionLoc, false, ref projection);

            // Draw our rectangle object
            GL.DrawElements(PrimitiveType.Triangles, 12,
                DrawElementsType.UnsignedInt, IntPtr.Zero);

            //TODO

            // Un-bind the VAO and swap our buffers
            //GL.BindVertexArray(0); //TODO: Un-comment this
            vp.SwapBuffers();
        }
    }
}