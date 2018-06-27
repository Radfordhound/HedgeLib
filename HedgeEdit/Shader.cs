using SharpDX.D3DCompiler;
using SharpDX.Direct3D11;
using System;
using System.Runtime.InteropServices;
using Buffer = SharpDX.Direct3D11.Buffer;

namespace HedgeEdit
{
    public class Shader : IDisposable
    {
        // Variables/Constants
        public VertexShader VertexShader;
        public PixelShader PixelShader;
        public ShaderSignature VertexSignature;
        public Buffer ConstantBuffer;

        public const string VSExtension = ".vs",
            PSExtension = ".ps", Extension = ".fx";

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
            var bufferDesc = new BufferDescription()
            {
                SizeInBytes = Marshal.SizeOf<T>(),
                Usage = ResourceUsage.Default,
                BindFlags = BindFlags.ConstantBuffer,
                CpuAccessFlags = CpuAccessFlags.None,
                OptionFlags = ResourceOptionFlags.None,
                StructureByteStride = 0
            };

            ConstantBuffer = Buffer.Create(device,
                ref constBufferLayout, bufferDesc);

            // Dispose of Shader Byte Code
            vsByteCode.Dispose();
            psByteCode.Dispose();
        }

        public void Use(DeviceContext context)
        {
            context.VertexShader.SetConstantBuffer(0, ConstantBuffer);
            context.VertexShader.Set(VertexShader);
            context.PixelShader.Set(PixelShader);
        }

        public void Dispose()
        {
            if (ConstantBuffer == null)
                return;

            VertexSignature.Dispose();
            VertexShader.Dispose();
            PixelShader.Dispose();
            ConstantBuffer.Dispose();
            ConstantBuffer = null;
        }
    }
}