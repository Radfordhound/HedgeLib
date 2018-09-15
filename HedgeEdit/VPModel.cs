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
        public VPModel(Model mdl, bool generateAABB = true)
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
                    BindFlags.VertexBuffer, mesh.VertexData, 0,
                    ResourceUsage.Immutable);

                // Setup our Vertex Buffer Binding
                var binding = new VertexBufferBinding(vertices,
                    Mesh.StructureByteLength, 0);

                // Send index data to the GPU
                var indices = Buffer.Create(Viewport.Device,
                    BindFlags.IndexBuffer, mesh.Triangles, 0,
                    ResourceUsage.Immutable);

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
        public VPObjectInstance InstanceIntersects(ref Ray ray, out float distance)
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
                if (bb.Intersects(ref ray, out distance))
                {
                    return instance;
                }
            }

            distance = 0;
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

            VPObjectInstance instance;

            for (int i = 0; i < meshes.Length; ++i)
            {
                if (meshes[i].Slot != slot)
                    continue;

                meshes[i].Bind(skipMaterial);
                for (int i2 = 0; i2 < Instances.Count; ++i2)
                {
                    instance = Instances[i2];

                    // Update Constant Buffers and Draw Mesh
                    instance.UpdateConstantBuffers();
                    meshes[i].Draw();
                }

            }
        }
    }
}