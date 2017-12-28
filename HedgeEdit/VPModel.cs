using HedgeLib.Models;
using OpenTK.Graphics.ES30;
using System;
using System.Collections.Generic;

namespace HedgeEdit
{
    public class VPModel
    {
        // Variables/Constants
        public List<VPObjectInstance> Instances = new List<VPObjectInstance>();

        private string[] matNames;
        private int[] triLengths;
        private uint[] vaos;

        // Constructors
        public VPModel(Model mdl)
        {
            if (mdl == null)
                throw new ArgumentNullException("mdl");

            int meshCount = mdl.Meshes.Count;
            Mesh mesh;

            vaos = new uint[meshCount];
            triLengths = new int[meshCount];
            matNames = new string[meshCount];

            for (int i = 0; i < meshCount; ++i)
            {
                mesh = mdl.Meshes[i];
                triLengths[i] = mesh.Triangles.Length;

                // Setup a vertex array object and vertex buffer object
                GL.GenVertexArrays(1, out uint vao);
                GL.GenBuffers(1, out uint vbo);

                // Bind the VAO and VBO
                GL.BindVertexArray(vao);
                GL.BindBuffer(BufferTarget.ArrayBuffer, vbo);

                // Send vertex data to the GPU
                GL.BufferData(BufferTarget.ArrayBuffer, mesh.VertexData.Length *
                    sizeof(float), mesh.VertexData, BufferUsageHint.StaticDraw);

                // Vertex Positions
                GL.VertexAttribPointer(0, 3, VertexAttribPointerType.Float,
                    false, Mesh.StructureByteLength, Mesh.VertPos);
                GL.EnableVertexAttribArray(0);

                // Normals
                GL.VertexAttribPointer(1, 3, VertexAttribPointerType.Float,
                    false, Mesh.StructureByteLength, Mesh.NormPos * sizeof(float));
                GL.EnableVertexAttribArray(1);

                // Colors
                GL.VertexAttribPointer(2, 4, VertexAttribPointerType.Float,
                    false, Mesh.StructureByteLength, Mesh.ColorPos * sizeof(float));
                GL.EnableVertexAttribArray(2);

                // UV Coordinates
                GL.VertexAttribPointer(3, 2, VertexAttribPointerType.Float,
                    false, Mesh.StructureByteLength, Mesh.UVPos * sizeof(float));
                GL.EnableVertexAttribArray(3);

                // Generate an element buffer object
                GL.GenBuffers(1, out uint ebo);
                GL.BindBuffer(BufferTarget.ElementArrayBuffer, ebo);
                GL.BufferData(BufferTarget.ElementArrayBuffer, mesh.Triangles.Length *
                    sizeof(uint), mesh.Triangles, BufferUsageHint.StaticDraw);

                // Assign the mesh's material name
                matNames[i] = mesh.MaterialName;

                // Un-bind the VAO (and, by extension, the VBO/EBO)
                GL.BindVertexArray(0);

                // Assign the VAO/VBO/EBO to their corresponding arrays
                vaos[i] = vao;
            }
        }

        // Methods
        public void Draw(int shaderID)
        {
            if (vaos == null)
                throw new Exception("Cannot draw model - model not initialized!");

            int modelLoc = GL.GetUniformLocation(shaderID, "model");
            foreach (var transform in Instances)
            {
                // Update Transforms
                var modelTransform = transform.Matrix;

                // Update shader transform matrices
                GL.UniformMatrix4(modelLoc, false, ref modelTransform);

                // Draw the meshes
                for (int i = 0; i < vaos.Length; ++i)
                {
                    // Get the material
                    string matName = matNames[i];
                    var mat = (string.IsNullOrEmpty(matName) ||
                        !Viewport.Materials.ContainsKey(matName)) ?
                        Viewport.DefaultMaterial : Viewport.Materials[matName];

                    // Get the texture
                    string texName = (mat.Textures.Count > 0) ?
                        mat.Textures[0].TextureName : null;

                    int tex = (string.IsNullOrEmpty(texName) ||
                        !Viewport.Textures.ContainsKey(texName)) ?
                        Viewport.DefaultTexture : Viewport.Textures[texName];

                    // Bind the texture and the mesh's VAO
                    GL.BindTexture(TextureTarget.Texture2D, tex);
                    GL.BindVertexArray(vaos[i]);

                    // Draw the mesh
                    GL.DrawElements(PrimitiveType.Triangles, triLengths[i],
                        DrawElementsType.UnsignedInt, IntPtr.Zero);

                    // Un-bind the mesh's VAO
                    GL.BindVertexArray(0);
                }
            }
        }
    }
}