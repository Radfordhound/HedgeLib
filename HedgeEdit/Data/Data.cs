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
            var mdl = new Model(Primitives.Cube.Mesh);


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
                    Format = Texture.DXGI_FORMATS.R32G32B32_FLOAT,
                    MipmapCount = 1,
                    ColorData = new byte[][]
                    {
                        new byte[]
                        {
                            0, 0, 0x80, 0x3F,
                            0, 0, 0x80, 0x3F,
                            0, 0, 0x80, 0x3F,
                        }
                    }
                };
            }

            // Setup default texture/material/model
            DefaultTexture = GenTexture(defaultTex);
            DefaultMaterial = new GensMaterial();
            DefaultCube = new VPModel(mdl, true);
            //PreviewBox = new VPModel(mdl, true);

            // TODO: REMOVE THIS
            DefaultCube.Instances.Add(new VPObjectInstance(new SharpDX.Vector3(0, 0, 0),
                new SharpDX.Quaternion(0, 0.17365f, 0, 0.98481f)));

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