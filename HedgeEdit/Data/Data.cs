using HedgeLib.Materials;
using HedgeLib.Models;
using HedgeLib.Textures;
using System;
using System.IO;

namespace HedgeEdit
{
    public static partial class Data
    {
        // Methods
        public static void LoadDefaults()
        {
            // Load default model
            var watch = System.Diagnostics.Stopwatch.StartNew();
            string cubePth = Path.Combine(Program.StartupPath,
                Program.ResourcesPath, $"DefaultCube{Model.MDLExtension}");

            var mdl = new Model();
            mdl.Load(cubePth);

            // Load default texture
            string defaultTexPath = Path.Combine(Program.StartupPath,
                Program.ResourcesPath, $"DefaultTexture{DDS.Extension}");

            Texture defaultTex;
            if (File.Exists(defaultTexPath))
            {
                defaultTex = new DDS();
                defaultTex.Load(defaultTexPath);
            }
            else
            {
                defaultTex = new Texture()
                {
                    Width = 1,
                    Height = 1,
                    PixelFormat = Texture.PixelFormats.RGB,
                    MipmapCount = 1,
                    ColorData = new byte[][]
                    {
                        new byte[] { 255, 255, 255 }
                    }
                };
            }

            // Setup default texture/material/model
            DefaultTexture = GenTexture(defaultTex);
            DefaultMaterial = new GensMaterial();
            DefaultCube = new VPModel(mdl, true);

            watch.Stop();
            Console.WriteLine("Default assets init time: {0}",
                watch.ElapsedMilliseconds);
        }

        public static void Clear()
        {
            // Clear Resource Directories
            ModelDirectories.Clear();
            ResourceDirectories.Clear();
            
            // Clear Viewport Models/Instances
            Viewport.SelectedInstances.Clear();
            DefaultCube.Instances.Clear();
            DefaultTerrainGroup.Clear();
            TerrainGroups.Clear();
            Objects.Clear();

            // Clear Resources
            Materials.Clear();
            Textures.Clear();
        }
    }
}