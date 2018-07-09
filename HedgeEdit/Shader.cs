using SharpDX;
using SharpDX.D3DCompiler;
using SharpDX.Direct3D11;
using System;
using Buffer = SharpDX.Direct3D11.Buffer;

namespace HedgeEdit
{
    public class Shader : IDisposable
    {
        // Variables/Constants
        public VertexShader VertexShader;
        public PixelShader PixelShader;
        public ShaderSignature VertexSignature;
        public ConstantBuffer ConstantBuffer;

        public const string VSExtension = ".vs",
            PSExtension = ".ps", Extension = ".fx";

        // Constructors
        public Shader(ConstantBuffer constantBuffer)
        {
            ConstantBuffer = constantBuffer ??
                throw new ArgumentNullException("constantBuffer");
        }

        // Methods
        public void Load<T>(Device device, ref T constBufferLayout,
            string vsPath, string psPath) where T : struct
        {
            // Load HLSL code and compile it
            var vsByteCode = ShaderBytecode.CompileFromFile(vsPath, "VS", "vs_4_0");
            var psByteCode = ShaderBytecode.CompileFromFile(psPath, "PS", "ps_4_0");
            VertexSignature = ShaderSignature.GetInputSignature(vsByteCode);

            // Make D3D Shaders from compiled HLSL
            VertexShader = new VertexShader(device, vsByteCode);
            PixelShader = new PixelShader(device, psByteCode);

            // Setup our Constant Buffer
            //ConstantBuffer = new ConstantBuffer();
            ConstantBuffer.CreateBuffer(device, constBufferLayout);

            // Dispose of Shader Byte Code
            vsByteCode.Dispose();
            psByteCode.Dispose();
        }

        public void Use(DeviceContext context)
        {
            context.VertexShader.SetConstantBuffer(0, ConstantBuffer.Buffer);
            context.VertexShader.Set(VertexShader);
            context.PixelShader.Set(PixelShader);
        }

        public void Dispose()
        {
            if (VertexSignature == null)
                return;

            VertexSignature.Dispose();
            VertexShader.Dispose();
            PixelShader.Dispose();
            ConstantBuffer.Dispose();

            VertexSignature = null;
            VertexShader = null;
            PixelShader = null;
        }
    }

    public abstract class ConstantBuffer : IDisposable
    {
        // Variables/Constants
        public Buffer Buffer;

        // Methods
        public void CreateBuffer<T>(Device device, T layout)
            where T : struct
        {
            var bufferDesc = new BufferDescription()
            {
                SizeInBytes = Utilities.SizeOf<T>(),
                Usage = ResourceUsage.Dynamic,
                BindFlags = BindFlags.ConstantBuffer,
                CpuAccessFlags = CpuAccessFlags.Write,
                OptionFlags = ResourceOptionFlags.None,
                StructureByteStride = 0
            };

            //Layout = layout;
            Buffer = Buffer.Create(device, ref layout, bufferDesc);
        }

        public void Update()
        {
            Viewport.Context.MapSubresource(Buffer, MapMode.WriteDiscard,
                MapFlags.None, out DataStream ds);

            Write(ds);

            ds.Dispose();
            Viewport.Context.UnmapSubresource(Buffer, 0);
        }

        protected abstract void Write(DataStream ds);

        public void Dispose()
        {
            if (Buffer == null)
                return;

            Buffer.Dispose();
            Buffer = null;
        }
    }
}