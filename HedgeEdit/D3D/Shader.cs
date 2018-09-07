using SharpDX.D3DCompiler;
using SharpDX.Direct3D11;
using System;

namespace HedgeEdit.D3D
{
    public abstract class Shader : IDisposable
    {
        // Variables/Constants
        public const string EntryPoint = "main",
            VSEntryPoint = "VS", PSEntryPoint = "PS";

        public const string VSExtension = ".vs",
            PSExtension = ".ps", Extension = ".fx";

        // Constructors
        ~Shader()
        {
            Dispose();
        }

        // Methods
        public abstract void Use(DeviceContext context);
        public abstract void Dispose();
    }

    public class VShader : Shader
    {
        // Variables/Constants
        public VertexShader VertexShader { get; protected set; }
        public InputLayout InputLayout { get; protected set; }

        // Constructors
        public VShader(Device device, string pth, InputElement[] elements,
            string entryPoint = EntryPoint)
        {
            Load(device, pth, elements, entryPoint);
        }

        public VShader(Device device, byte[] byteCode, InputElement[] elements)
        {
            Load(device, byteCode, elements);
        }

        public VShader(VertexShader vertexShader, InputLayout layout)
        {
            VertexShader = vertexShader ?? throw new ArgumentNullException("vertexShader");
            InputLayout = layout;
        }

        ~VShader()
        {
            Dispose();
        }

        // Methods
        public void Load(Device device, string pth, InputElement[] elements,
            string entryPoint = EntryPoint)
        {
            // Load HLSL code and compile it
            var vsByteCode = ShaderBytecode.CompileFromFile(pth, entryPoint, "vs_4_0");
            var signature = ShaderSignature.GetInputSignature(vsByteCode);

            // Create InputLayout
            if (elements != null && elements.Length > 0)
                InputLayout = new InputLayout(device, signature, elements);

            // Dispose Input Signature
            signature.Dispose();

            // Make D3D Shaders from compiled HLSL
            VertexShader = new VertexShader(device, vsByteCode);

            // Dispose of Shader Byte Code
            vsByteCode.Dispose();
        }

        public void Load(Device device, byte[] byteCode, InputElement[] elements)
        {
            // Create Input Signature
            var signature = ShaderSignature.GetInputSignature(byteCode);

            // Create InputLayout
            if (elements != null && elements.Length > 0)
                InputLayout = new InputLayout(device, signature, elements);

            // Dispose Input Signature
            signature.Dispose();

            // Make D3D Shaders from compiled HLSL
            VertexShader = new VertexShader(device, byteCode);
        }

        public override void Use(DeviceContext context)
        {
            context.InputAssembler.InputLayout = InputLayout;
            context.VertexShader.Set(VertexShader);
        }

        public override void Dispose()
        {
            VertexShader.Dispose();

            if (InputLayout != null)
                InputLayout.Dispose();
        }
    }

    public class PShader : Shader
    {
        // Variables/Constants
        public PixelShader PixelShader { get; protected set; }

        // Constructors
        public PShader(Device device, byte[] byteCode)
        {
            Load(device, byteCode);
        }

        public PShader(Device device, string pth, string entryPoint = EntryPoint)
        {
            Load(device, pth, entryPoint);
        }

        public PShader(PixelShader pixelShader)
        {
            PixelShader = pixelShader ?? throw new ArgumentNullException("pixelShader");
        }

        ~PShader()
        {
            Dispose();
        }

        // Methods
        public void Load(Device device, string pth, string entryPoint = EntryPoint)
        {
            // Load HLSL code and compile it
            var psByteCode = ShaderBytecode.CompileFromFile(pth, entryPoint, "ps_4_0");

            // Make D3D Shaders from compiled HLSL
            PixelShader = new PixelShader(device, psByteCode);

            // Dispose of Shader Byte Code
            psByteCode.Dispose();
        }

        public void Load(Device device, byte[] byteCode)
        {
            PixelShader = new PixelShader(device, byteCode);
        }

        public override void Use(DeviceContext context)
        {
            context.PixelShader.Set(PixelShader);
        }

        public override void Dispose()
        {
            PixelShader.Dispose();
        }
    }
}