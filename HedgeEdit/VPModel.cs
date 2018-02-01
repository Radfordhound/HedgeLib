using HedgeLib;
using HedgeLib.Math;
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
        public Vector3 BoundingSize;

        private string[] matNames;
        private int[] triLengths;
        private uint[] vaos;

        // Constructors
        public VPModel(Model mdl, bool generateAABB = false)
        {
            if (mdl == null)
                throw new ArgumentNullException("mdl");

            int meshCount = mdl.Meshes.Count;
            Mesh mesh;

            var aabb = new AABB();
            vaos = new uint[meshCount];
            triLengths = new int[meshCount];
            matNames = new string[meshCount];

            for (int i = 0; i < meshCount; ++i)
            {
                mesh = mdl.Meshes[i];
                triLengths[i] = mesh.Triangles.Length;

                // Generate an AABB
                if (generateAABB)
                {
                    for (uint i2 = Mesh.VertPos; i2 < mesh.VertexData.Length;
                        i2 += Mesh.StructureLength)
                    {
                        aabb.AddPoint(
                            mesh.VertexData[i2],
                            mesh.VertexData[i2 + 1],
                            mesh.VertexData[i2 + 2]);
                    }
                }

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

            if (generateAABB)
                BoundingSize = aabb.Size;
        }

        // Methods
        public void Draw(int shaderID)
        {
            if (vaos == null)
                throw new Exception("Cannot draw model - model not initialized!");

            int modelLoc = GL.GetUniformLocation(shaderID, "model");
            int highlightLoc = GL.GetUniformLocation(shaderID, "highlight");
            GL.Uniform4(highlightLoc, new OpenTK.Vector4(1, 1, 1, 1));

            foreach (var transform in Instances)
            {
                // Update Transforms
                var modelTransform = transform.Matrix;

                // Update shader transform matrices
                GL.UniformMatrix4(modelLoc, false, ref modelTransform);

                // Update Highlight Color
                bool selected = Viewport.SelectedInstances.Contains(transform);
                if (selected)
                    GL.Uniform4(highlightLoc, new OpenTK.Vector4(1, 0, 0, 1));

                // Draw the meshes
                for (int i = 0; i < vaos.Length; ++i)
                {
                    // Get the material
                    string matName = matNames[i];
                    var mat = (string.IsNullOrEmpty(matName) ||
                        !Viewport.Materials.ContainsKey(matName)) ?
                        Viewport.DefaultMaterial : Viewport.Materials[matName];

                    // Get the texture
                    string texName = (mat.Texset.Textures.Count > 0) ?
                        mat.Texset.Textures[0].TextureName : null;

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

                // Set Highlight Color back to default
                if (selected)
                    GL.Uniform4(highlightLoc, new OpenTK.Vector4(1, 1, 1, 1));
            }
        }

        public VPObjectInstance InstanceIntersects(OpenTK.Vector3 origin,
            OpenTK.Vector3 direction, uint distance = 100)
        {
            //var dest = (origin + (direction * distance));
            var o = Types.ToHedgeLib(origin);
            var d = Types.ToHedgeLib(direction);

            foreach (var instance in Instances)
            {
                // TODO: Try to make this more efficient if possible
                if (AABB.Intersects(o, d, Types.ToHedgeLib(
                    instance.Position), BoundingSize, distance))
                {
                    return instance;
                }
            }

            return null;
        }
    }
}