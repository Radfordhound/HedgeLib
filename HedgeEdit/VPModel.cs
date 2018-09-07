using HedgeLib.Models;
using SharpDX;
using SharpDX.Direct3D11;
using System;
using System.Collections.Generic;
using Buffer = SharpDX.Direct3D11.Buffer;

namespace HedgeEdit
{
    public class VPModel : IDisposable
    {
        // Variables/Constants
        public List<VPObjectInstance> Instances = new List<VPObjectInstance>();
        public VPMesh[] Meshes => meshes;
        public BoundingBox BoundingBox;
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
                    int pointLen = (mesh.VertexData.Length / Mesh.StructureLength);
                    var points = new Vector3[pointLen];
                    int dataIndex = Mesh.VertPos;

                    for (uint i2 = 0; i2 < pointLen; ++i2)
                    {
                        points[i2] = new Vector3(
                            mesh.VertexData[dataIndex],
                            mesh.VertexData[dataIndex + 1],
                            mesh.VertexData[dataIndex + 2]);
                        dataIndex += Mesh.StructureLength;
                    }

                    var meshBox = BoundingBox.FromPoints(points);
                    BoundingBox.Merge(ref BoundingBox, ref meshBox, out BoundingBox);
                }

                // Send vertex data to the GPU
                var vertices = Buffer.Create(Viewport.Device,
                    BindFlags.VertexBuffer, mesh.VertexData);

                // Setup our Vertex Buffer Binding
                var binding = new VertexBufferBinding(vertices,
                    Mesh.StructureByteLength, 0);

                // Send index data to the GPU
                var indices = Buffer.Create(Viewport.Device,
                    BindFlags.IndexBuffer, mesh.Triangles);

                // Generate a VPMesh
                meshes[i] = new VPMesh(indices, binding, mesh);
            }
        }

        ~VPModel()
        {
            Dispose();
        }

        public void Dispose()
        {
            foreach (var mesh in meshes)
            {
                mesh.Dispose();
            }
        }

        // Methods
        public VPObjectInstance InstanceIntersects(ref Ray ray)
        {
            BoundingBox bb;
            foreach (var instance in Instances)
            {
                // Transform the bounding box by the instance's matrix
                bb.Maximum = (Vector3)Vector3.Transform(
                    BoundingBox.Maximum, instance.Matrix);
                bb.Minimum = (Vector3)Vector3.Transform(
                    BoundingBox.Minimum, instance.Matrix);

                // Check if the ray intersects the transformed bounding box
                if (bb.Intersects(ref ray))
                {
                    // TODO: Return distance here and do distance check
                    return instance;
                }
            }

            return null;
        }

        public void Draw(Mesh.Slots slot, bool skipMaterial = false)
        {
            if (meshes == null)
                throw new Exception("Cannot draw model - model not initialized!");

            //int modelLoc = GL.GetUniformLocation(shaderID, "model");
            //int highlightLoc = GL.GetUniformLocation(shaderID, "highlight");

            //OpenTK.Vector4 col;
            //if (isPreview)
            //{
            //    // TODO: Use transparency setting
            //    var color = (Vector3)instance.CustomData;
            //    col = new OpenTK.Vector4(color.X, color.Y, color.Z, 0.27451f);
            //}
            //else
            //    col = new OpenTK.Vector4(1, 1, 1, 1);

            //if (!isPreview)
            //    GL.Uniform4(highlightLoc, new OpenTK.Vector4(1, 1, 1, 1));

            foreach (var instance in Instances)
            {
                // Update Constant Buffers
                instance.UpdateConstantBuffers();

                // Update Highlight Color
                // TODO
                //bool selected = Viewport.SelectedInstances.Contains(instance);
                //if (selected)
                //{
                //    GL.Uniform4(highlightLoc, new OpenTK.Vector4(1, 0, 0,
                //        (isPreview) ? 0.27451f * 2 : 1)); // TODO: Use transparency setting
                //}
                //else if (isPreview)
                //{
                //    // TODO: Use transparency setting
                //    var color = (Vector3)instance.CustomData;
                //    GL.Uniform4(highlightLoc, new OpenTK.Vector4(
                //        color.X, color.Y, color.Z, 0.27451f));
                //}

                // Draw the meshes
                for (int i = 0; i < meshes.Length; ++i)
                {
                    meshes[i].Draw(slot, skipMaterial);
                }

                // Set Highlight Color back to default
                // TODO
                //if (selected && !isPreview)
                //{
                //    GL.Uniform4(highlightLoc, new OpenTK.Vector4(1, 1, 1, 1));
                //}
            }
        }
    }
}