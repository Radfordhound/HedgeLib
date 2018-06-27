using SharpDX;
using SharpDX.Direct3D11;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;

namespace HedgeEdit
{
    public static partial class Data
    {
        // Variables/Constants
        public static Dictionary<string, Shader> Shaders =
            new Dictionary<string, Shader>();

        public const string ShadersDirectory = "Shaders", ShaderList = "ShaderList.txt";

        // Methods
        public static void LoadShaderList(Device device)
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
                // TODO: Make it so shaders can be in two separate files or in one
                name = shaderNames[i];
                pth1 = Path.Combine(shaderDir, $"{name}{Shader.Extension}");
                pth2 = Path.Combine(shaderDir, $"{name}{Shader.Extension}");

                if (!File.Exists(pth1) || !File.Exists(pth2))
                    continue;

                // Load the Shader
                var shader = new Shader();
                var layout = new HedgeEditConstantBuffer(); // TODO: Have different types
                shader.Load(device, ref layout, pth1, pth2);
                Shaders.Add(name, shader);
            }
        }

        // Other
        [StructLayout(LayoutKind.Sequential)]
        public struct HedgeEditConstantBuffer
        {
            public Matrix WorldViewProj;
        }
    }
}