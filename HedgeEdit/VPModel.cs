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
        public AABB BoundingBox = new AABB();
        protected VPMesh[] meshes;

        // Constructors
        public VPModel(Model mdl, bool generateAABB = false)
        {
            if (mdl == null)
                throw new ArgumentNullException("mdl");

            int meshCount = mdl.Meshes.Count;
            Mesh mesh;

            meshes = new VPMesh[meshCount];
            for (int i = 0; i < meshCount; ++i)
            {
                mesh = mdl.Meshes[i];

                // Generate an AABB
                if (generateAABB)
                {
                    for (uint i2 = Mesh.VertPos; i2 < mesh.VertexData.Length;
                        i2 += Mesh.StructureLength)
                    {
                        var wp = OpenTK.Vector3.TransformPosition(
                            new OpenTK.Vector3(
                                mesh.VertexData[i2],
                                mesh.VertexData[i2 + 1],
                                mesh.VertexData[i2 + 2]),
                            OpenTK.Matrix4.Identity);

                        BoundingBox.AddPoint(wp.X, wp.Y, wp.Z);
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

                // Un-bind the VAO (and, by extension, the VBO/EBO)
                GL.BindVertexArray(0);

                // Generate a VPMesh
                meshes[i] = new VPMesh()
                {
                    VAO = vao,
                    MaterialName = mesh.MaterialName,
                    TriangleCount = mesh.Triangles.Length,
                    Slot = mesh.Slot
                };
            }
        }

        // Methods
        public VPObjectInstance InstanceIntersects(OpenTK.Vector3 origin,
            OpenTK.Vector3 direction, uint distance = 100)
        {
            var o = Types.ToHedgeLib(origin);
            var d = Types.ToHedgeLib(direction);

            foreach (var instance in Instances)
            {
                // TODO: Try to make this more efficient if possible
                if (BoundingBox.Intersects(o, d, Types.ToHedgeLib(
                    instance.Position), distance))
                {
                    return instance;
                }
            }

            return null;
        }

        public void Draw(int shaderID, Mesh.Slots slot)
        {
            if (meshes == null)
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
                for (int i = 0; i < meshes.Length; ++i)
                {
                    meshes[i].Draw(slot);
                }

                // Set Highlight Color back to default
                if (selected)
                    GL.Uniform4(highlightLoc, new OpenTK.Vector4(1, 1, 1, 1));
            }
        }

        // Other
        protected struct VPMesh
        {
            // Variables/Constants
            public string MaterialName;
            public uint VAO;
            public int TriangleCount;
            public Mesh.Slots Slot;

            // Methods
            public void Draw(Mesh.Slots slot)
            {
                if (Slot != slot)
                    return;

                // Get the material
                var mat = (string.IsNullOrEmpty(MaterialName) ||
                    !Data.Materials.ContainsKey(MaterialName)) ?
                    Data.DefaultMaterial : Data.Materials[MaterialName];

                // Get the texture
                string texName = (mat.Texset.Textures.Count > 0) ?
                    mat.Texset.Textures[0].TextureName : null;

                int tex = (string.IsNullOrEmpty(texName) ||
                    !Data.Textures.ContainsKey(texName)) ?
                    Data.DefaultTexture : Data.Textures[texName];

                // Bind the texture and the mesh's VAO
                GL.BindTexture(TextureTarget.Texture2D, tex);
                GL.BindVertexArray(VAO);

                // Draw the mesh
                GL.DrawElements(PrimitiveType.Triangles, TriangleCount,
                    DrawElementsType.UnsignedInt, IntPtr.Zero);

                // Un-bind the VAO
                GL.BindVertexArray(0);
            }
        }
    }
}