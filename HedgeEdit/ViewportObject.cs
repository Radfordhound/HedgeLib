using HedgeLib.Models;
using OpenTK;
using OpenTK.Graphics.ES30;
using System;

namespace HedgeEdit
{
    public class ViewportObject
    {
        //Variables/Constants
        public Quaternion Rotation = Quaternion.Identity;
        public Vector3 Position = Vector3.Zero;
        public object CustomData = null;

        private Model model = null;
        private uint[] vaos;

        //Constructors
        public ViewportObject(Model mdl) : this(mdl, Vector3.Zero, Quaternion.Identity) { }
        public ViewportObject(Model mdl, Vector3 pos,
            Quaternion rot, object customData = null)
        {
            Mesh mesh;

            Position = pos;
            Rotation = rot;
            CustomData = customData;
            model = mdl ?? throw new ArgumentNullException("mdl");
            vaos = new uint[mdl.Meshes.Count];

            for (int i = 0; i < mdl.Meshes.Count; ++i)
            {
                mesh = mdl.Meshes[i];

                // Setup a vertex array object and vertex buffer object
                GL.GenVertexArrays(1, out uint vao);
                GL.GenBuffers(1, out uint vbo);

                // Bind the VAO and VBO
                GL.BindVertexArray(vao);
                GL.BindBuffer(BufferTarget.ArrayBuffer, vbo);

                // Send vertex data to the GPU
                GL.BufferData(BufferTarget.ArrayBuffer, mesh.Vertices.Length *
                    sizeof(float), mesh.Vertices, BufferUsageHint.StaticDraw);

                // Set our vertex attribute pointers
                GL.VertexAttribPointer(0, 3, VertexAttribPointerType.Float,
                    false, 3 * sizeof(float), 0);
                GL.EnableVertexAttribArray(0);

                // Generate an element buffer object
                GL.GenBuffers(1, out uint ebo);
                GL.BindBuffer(BufferTarget.ElementArrayBuffer, ebo);
                GL.BufferData(BufferTarget.ElementArrayBuffer, mesh.Triangles.Length *
                    sizeof(uint), mesh.Triangles, BufferUsageHint.StaticDraw);

                // Un-bind the VAO (and, by extension, the VBO/EBO)
                GL.BindVertexArray(0);

                // Assign the VAO/VBO/EBO to their corresponding arrays
                vaos[i] = vao;
            }
        }

        //Methods
        public void Draw(int shaderID)
        {
            if (model == null)
                throw new Exception("Cannot draw model - model not initialized!");

            // Update Transforms
            var modelTransform = Matrix4.CreateTranslation(Position) *
                Matrix4.CreateFromQuaternion(Rotation);

            // Update shader transform matrices
            int modelLoc = GL.GetUniformLocation(shaderID, "model");
            GL.UniformMatrix4(modelLoc, false, ref modelTransform);

            // Draw the meshes
            for (int i = 0; i < model.Meshes.Count; ++i)
            {
                // Bind the mesh's VAO
                GL.BindVertexArray(vaos[i]);

                // Draw the mesh
                GL.DrawElements(PrimitiveType.Triangles, model.Meshes[i].Triangles.Length,
                    DrawElementsType.UnsignedInt, IntPtr.Zero);

                // Un-bind the mesh's VAO
                GL.BindVertexArray(0);
            }
        }
    }
}