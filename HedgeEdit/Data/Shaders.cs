using HedgeEdit.D3D;
using SharpDX.Direct3D11;
using System.Collections.Generic;
using System.IO;

namespace HedgeEdit
{
    public static partial class Data
    {
        // Variables/Constants
        public static Dictionary<string, VShader> VertexShaders =
            new Dictionary<string, VShader>();

        public static Dictionary<string, PShader> PixelShaders =
            new Dictionary<string, PShader>();

        public static string ShadersDirectory => Path.Combine(Program.StartupPath, ShadersPath);
        public const string ShadersPath = "Shaders", ShaderList = "ShaderList.txt";

        // Methods
        public static void LoadShaders(Device device, string dir, InputElement[] elements,
            string vsEntryPoint = Shader.VSEntryPoint, string psEntryPoint = Shader.PSEntryPoint)
        {
            // Load all shaders in the given directory
            foreach (var pth in Directory.GetFiles(dir, $"*{Shader.Extension}"))
            {
                var vshader = new VShader(device, pth, elements, vsEntryPoint);
                var pshader = new PShader(device, pth, psEntryPoint);
                string name = Path.GetFileNameWithoutExtension(pth);

                VertexShaders.Add(name, vshader);
                PixelShaders.Add(name, pshader);
            }

            foreach (var pth in Directory.GetFiles(dir, $"*{Shader.VSExtension}"))
            {
                var shader = new VShader(device, pth, elements, Shader.EntryPoint);
                VertexShaders.Add(Path.GetFileNameWithoutExtension(pth), shader);
            }

            foreach (var pth in Directory.GetFiles(dir, $"*{Shader.PSExtension}"))
            {
                var shader = new PShader(device, pth, Shader.EntryPoint);
                PixelShaders.Add(Path.GetFileNameWithoutExtension(pth), shader);
            }
        }
    }
}