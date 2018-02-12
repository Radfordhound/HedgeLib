using OpenTK.Graphics.ES30;
using System.Collections.Generic;
using System.IO;

namespace HedgeEdit
{
    public static class Shaders
    {
        // Variables/Constants
        public static Dictionary<string, int> ShaderPrograms =
            new Dictionary<string, int>();

        public const string ShadersDirectory = "Shaders", ShaderList = "ShaderList.txt",
            VertExtension = ".vert", FragExtension = ".frag";

        // Methods
        public static void LoadAll()
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
                pth1 = Path.Combine(shaderDir, $"{name}{VertExtension}");
                pth2 = Path.Combine(shaderDir, $"{name}{FragExtension}");

                if (!File.Exists(pth1) || !File.Exists(pth2))
                    continue;

                // Make a GL Program
                int program = GL.CreateProgram();

                // Load shaders
                int vertShader = LoadShader(pth1, ShaderType.VertexShader);
                int fragShader = LoadShader(pth2, ShaderType.FragmentShader);
                
                // Attach them to our program
                GL.AttachShader(program, vertShader);
                GL.AttachShader(program, fragShader);
                GL.LinkProgram(program);

                // Delete the shader objects (we won't be using them anymore)
                GL.DeleteShader(vertShader);
                GL.DeleteShader(fragShader);

                // Add our program to the dictionary
                ShaderPrograms.Add(name, program);
            }

            // Sub-Methods
            int LoadShader(string path, ShaderType tp)
            {
                int id = GL.CreateShader(tp);
                GL.ShaderSource(id, File.ReadAllText(path));
                GL.CompileShader(id);

                // TODO: Check if the shader compiled correctly
                // TODO: Look into binary shader support

                return id;
            }
        }
    }
}