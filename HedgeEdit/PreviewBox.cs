using HedgeLib.Models;
using SharpDX;
using SharpDX.Direct3D11;
using System;
using Buffer = SharpDX.Direct3D11.Buffer;

namespace HedgeEdit
{
    public class PreviewBox : IDisposable
    {
        // Variables/Constants
        protected float[] vertexData;
        protected VPMesh mesh;
        protected Buffer vertexBuffer;
        private const int vertexDataLen = Mesh.StructureLength * 8;

        // Constructors
        public PreviewBox(Device device, bool useLines = true)
        {
            // Send vertex data to the GPU
            vertexData = Primitives.Cube.VertexData;
            vertexBuffer = Buffer.Create(device, BindFlags.VertexBuffer,
                vertexData, 0, ResourceUsage.Dynamic, CpuAccessFlags.Write);

            // Setup our Vertex Buffer Binding
            var binding = new VertexBufferBinding(vertexBuffer,
                Mesh.StructureByteLength, 0);

            // Send index data to the GPU
            var indices = (useLines) ?
                Primitives.Cube.LineIndices :
                Primitives.Cube.TriangleIndices;

            var indexBuffer = Buffer.Create(device, BindFlags.IndexBuffer,
                indices, 0, ResourceUsage.Immutable); // Will be disposed when we dispose mesh

            // Generate a VPMesh
            mesh = new VPMesh(indexBuffer, binding, null,
                indices.Length, Mesh.Slots.Default);
        }

        ~PreviewBox()
        {
            Dispose();
        }

        // Methods
        public void Update(Vector3[] corners)
        {
            int cornerIndex = 0;
            for (int i = Mesh.VertPos; i < vertexDataLen;
                i += Mesh.StructureLength)
            {
                vertexData[i] = corners[cornerIndex].X;
                vertexData[i + 1] = corners[cornerIndex].Y;
                vertexData[i + 2] = corners[cornerIndex].Z;
                ++cornerIndex;
            }

            Viewport.Context.MapSubresource(vertexBuffer, MapMode.WriteDiscard,
                MapFlags.None, out DataStream ds);

            ds.WriteRange(vertexData);
            ds.Dispose();
            Viewport.Context.UnmapSubresource(vertexBuffer, 0);
        }

        public void Draw(VPObjectInstance instance)
        {
            instance.UpdateConstantBuffers();
            mesh.Draw(Mesh.Slots.Default, true);
        }

        public void Dispose()
        {
            Utilities.Dispose(ref vertexBuffer);
            mesh.Dispose();
        }
    }
}