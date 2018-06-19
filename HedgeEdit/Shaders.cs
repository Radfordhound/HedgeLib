using SharpDX;
using SharpDX.D3DCompiler;
using SharpDX.Direct3D11;
using System.Collections.Generic;
using System.IO;

namespace HedgeEdit
{
    public static class Shaders
    {
        // Variables/Constants
        public static Dictionary<string, Shader> ShaderPrograms =
            new Dictionary<string, Shader>();

        public const string ShadersDirectory = "Shaders", ShaderList = "ShaderList.txt",
            VertExtension = ".vs", FragExtension = ".ps", Extension = ".fx";

        // Methods
        public static void LoadAll(Device device)
        {
            // Make some generic variables we can re-use
            string pth1, pth2, name;

            // Load the shader list
            var shaderDir = Path.Combine(Program.StartupPath, ShadersDirectory);
            pth1 = Path.Combine(shaderDir, ShaderList);
            if (!File.Exists(pth1))
            {
                throw new FileNotFoundException(
                    "Cannot load shaders - shader list not found!", pth1);
            }

            var shaderNames = File.ReadAllLines(pth1);

            // Load all vertex & fragment shaders on the list and make GL programs from them
            for (int i = 0; i < shaderNames.Length; ++i)
            {
                // Make sure both shaders exist first
                name = shaderNames[i];
                pth1 = Path.Combine(shaderDir, $"{name}{Extension}");
                pth2 = Path.Combine(shaderDir, $"{name}{Extension}");

                if (!File.Exists(pth1))
                    continue;

                // Make a DX Shader
                var vsByteCode = ShaderBytecode.CompileFromFile(pth1, "VS", "vs_4_0");
                var psByteCode = ShaderBytecode.CompileFromFile(pth2, "PS", "ps_4_0");
                var signature = ShaderSignature.GetInputSignature(vsByteCode);

                ShaderPrograms.Add(name, new Shader()
                {
                    Signature = signature,
                    VertexShader = new VertexShader(device, vsByteCode),
                    PixelShader = new PixelShader(device, psByteCode),
                    ConstantBuffer = new Buffer(device, Utilities.SizeOf<Matrix>(),
                        ResourceUsage.Default, BindFlags.ConstantBuffer,
                        CpuAccessFlags.None, ResourceOptionFlags.None, 0)
                });

                vsByteCode.Dispose();
                psByteCode.Dispose();

                // Make a GL Program
                //int program = GL.CreateProgram();

                //// Load shaders
                //int vertShader = LoadShader(pth1, ShaderType.VertexShader);
                //int fragShader = LoadShader(pth2, ShaderType.FragmentShader);

                //// Attach them to our program
                //GL.AttachShader(program, vertShader);
                //GL.AttachShader(program, fragShader);
                //GL.LinkProgram(program);

                //// Delete the shader objects (we won't be using them anymore)
                //GL.DeleteShader(vertShader);
                //GL.DeleteShader(fragShader);

                //// Add our program to the dictionary
                //ShaderPrograms.Add(name, program);
            }

            // Sub-Methods
            //int LoadShader(string path, ShaderType tp)
            //{
            //    int id = GL.CreateShader(tp);
            //    GL.ShaderSource(id, File.ReadAllText(path));
            //    GL.CompileShader(id);

            //    // TODO: Check if the shader compiled correctly
            //    // TODO: Look into binary shader support

            //    return id;
            //}
        }

        // Other
        public class Shader
        {
            // Variables/Constants
            public VertexShader VertexShader;
            public PixelShader PixelShader;
            public ShaderSignature Signature;
            public Buffer ConstantBuffer;

            // Methods
            public void Use(DeviceContext context)
            {
                context.VertexShader.SetConstantBuffer(0, ConstantBuffer);
                context.VertexShader.Set(VertexShader);
                context.PixelShader.Set(PixelShader);
            }

            public void Dispose()
            {
                Signature.Dispose();
                VertexShader.Dispose();
                PixelShader.Dispose();
                ConstantBuffer.Dispose();
            }
        }
    }
}