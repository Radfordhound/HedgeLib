using HedgeEdit.UI;
using HedgeLib.Headers;
using HedgeLib.Materials;
using HedgeLib.Textures;
using SharpDX;
using SharpDX.Direct3D11;
using SharpDX.DXGI;
using System;
using System.Collections.Generic;
using System.IO;

namespace HedgeEdit
{
    public static partial class Data
    {
        // Variables/Constants
        public static AssetCollection<GensMaterial> Materials =
            new AssetCollection<GensMaterial>();

        public static Dictionary<string, ShaderResourceView> Textures =
            new Dictionary<string, ShaderResourceView>();

        public static AssetDirectories ResourceDirectories = new AssetDirectories();
        public static GensMaterial DefaultMaterial;
        public static ShaderResourceView DefaultTexture;

        // Methods
        public static ShaderResourceView GetTexture(string name)
        {
            if (string.IsNullOrEmpty(name))
                return DefaultTexture;

            if (!Textures.ContainsKey(name))
            {
                // Attempt to load the texture
                var texExt = Types.TextureExtension;
                foreach (var dir in ResourceDirectories)
                {
                    if (Directory.Exists(dir.FullPath))
                    {
                        string path = Path.Combine(dir.FullPath, $"{name}{texExt}");
                        if (File.Exists(path))
                        {
                            var tex = LoadTexture(path, name);
                            if (tex != null)
                                return tex;

                            // TODO: Maybe remove this line so it keeps trying other dirs?
                            return DefaultTexture;
                        }
                    }
                }

                // Return the default texture if that failed
                return DefaultTexture;
            }
            else
            {
                return Textures[name];
            }
        }

        public static GensMaterial GetMaterial(string name, bool nonEditable = true)
        {
            if (string.IsNullOrEmpty(name))
                return DefaultMaterial;

            if (!Materials.ContainsKey(name))
            {
                // Attempt to load the material
                var matExt = Types.MaterialExtension;
                foreach (var dir in ResourceDirectories)
                {
                    if (Directory.Exists(dir.FullPath))
                    {
                        string path = Path.Combine(dir.FullPath, $"{name}{matExt}");
                        if (File.Exists(path))
                        {
                            var mat = LoadMaterial(path, name, nonEditable);
                            if (mat != null)
                                return mat;

                            // TODO: Maybe remove this line so it keeps trying other dirs?
                            return DefaultMaterial;
                        }
                    }
                }

                // Return the default material if that failed
                return DefaultMaterial;
            }
            else
            {
                return Materials[name];
            }
        }

        public static AssetDirectory AddResourceDirectoryFromPath(string path)
        {
            return AddResourceDirectory(Path.GetDirectoryName(path));
        }

        public static AssetDirectory AddResourceDirectory(string dir)
        {
            return ResourceDirectories.AddDirectory(dir);
        }

        public static ShaderResourceView AddTexture(string name, Texture tex)
        {
            // Add/Replace Texture
            var texture = GenTexture(tex);
            if (!Textures.ContainsKey(name))
            {
                Textures.Add(name, texture);
            }
            else
            {
                Textures[name] = texture;
            }

            return texture;
        }

        public static ShaderResourceView LoadTexture(string path, string name = null)
        {
            // Don't bother loading this texture again if we've already loaded it
            name = (string.IsNullOrEmpty(name)) ?
                Path.GetFileNameWithoutExtension(path) : name;

            if (Textures.ContainsKey(name))
                return Textures[name];

            // Figure out what type of texture to use
            Texture tex;
            switch (Types.CurrentDataType)
            {
                case Types.DataTypes.Forces:
                case Types.DataTypes.LW:
                case Types.DataTypes.Gens:
                case Types.DataTypes.SU:
                case Types.DataTypes.S06:
                    tex = new DDS();
                    break;

                // TODO: Add Storybook Support
                case Types.DataTypes.Storybook:
                    throw new NotImplementedException(
                        "Could not load, Storybook textures are not yet supported!");

                // TODO: Add Colors Support
                case Types.DataTypes.Colors:
                    throw new NotImplementedException(
                        "Could not load, Colors textures are not yet supported!");

                // TODO: Add Heroes/Shadow Support
                case Types.DataTypes.Shadow:
                case Types.DataTypes.Heroes:
                    throw new NotImplementedException(
                        "Could not load, Heroes/Shadow textures are not yet supported!");

                // TODO: Add SA2 Support
                case Types.DataTypes.SA2:
                    throw new NotImplementedException(
                        "Could not load, SA2 textures are not yet supported!");

                default:
                    throw new Exception(
                        "Could not load, game type has not been set!");
            }

            // Load Texture
            try
            {
                ShaderResourceView texture = null;
                tex.Load(path);

                Program.MainUIInvoke(() =>
                {
                    texture = AddTexture(name, tex);
                });

                return texture;
            }
            catch (Exception ex)
            {
                LuaTerminal.LogError($"ERROR: {ex.Message}");
                return null;
            }
        }

        public static GensMaterial LoadMaterial(string path,
            string name = null, bool nonEditable = true)
        {
            // Don't bother loading this material again if we've already loaded it
            if (string.IsNullOrEmpty(name))
                name = Path.GetFileNameWithoutExtension(path);

            if (Materials.ContainsKey(name))
                return Materials[name];

            // Figure out what type of material to use
            GensMaterial mat; // TODO: Set to generic material type once one is made
            switch (Types.CurrentDataType)
            {
                case Types.DataTypes.Forces:
                case Types.DataTypes.LW:
                case Types.DataTypes.Gens:
                case Types.DataTypes.SU:
                    mat = new GensMaterial();
                    break;

                // TODO: Add Storybook Support
                case Types.DataTypes.Storybook:
                    throw new NotImplementedException(
                        "Could not load, Storybook materials are not yet supported!");

                // TODO: Add Colors Support
                case Types.DataTypes.Colors:
                    throw new NotImplementedException(
                        "Could not load, Colors materials are not yet supported!");

                // TODO: Add 06 Support
                case Types.DataTypes.S06:
                    throw new NotImplementedException(
                        "Could not load, '06 materials are not yet supported!");

                // TODO: Add Heroes/Shadow Support
                case Types.DataTypes.Shadow:
                case Types.DataTypes.Heroes:
                    throw new NotImplementedException(
                        "Could not load, Heroes/Shadow materials are not yet supported!");

                // TODO: Add SA2 Support
                case Types.DataTypes.SA2:
                    throw new NotImplementedException(
                        "Could not load, SA2 materials are not yet supported!");

                default:
                    throw new Exception(
                        "Could not load, game type has not been set!");
            }

            // Material
            mat.Load(path);

            string dir = Path.GetDirectoryName(path);
            var resDir = ResourceDirectories.AddDirectory(dir);
            Materials.Add(name, new Asset<GensMaterial>(resDir, mat, nonEditable));

            // Textures
            foreach (var tex in mat.Texset.Textures)
            {
                GetTexture(tex.TextureName);
            }

            return mat;
        }

        public static void SaveMaterial(string path, GensMaterial mat)
        {
            uint rootNodeType = mat.Header.RootNodeType;
            switch (Types.CurrentDataType)
            {
                case Types.DataTypes.Forces:
                case Types.DataTypes.LW:
                    mat.Header = new MirageHeader();
                    mat.Header.RootNodeType = rootNodeType;
                    break;

                case Types.DataTypes.Gens:
                case Types.DataTypes.SU:
                    mat.Header = new GensHeader();
                    mat.Header.RootNodeType = rootNodeType;
                    break;

                // TODO: Add Storybook Support
                case Types.DataTypes.Storybook:
                    throw new NotImplementedException(
                        "Could not save, Storybook materials are not yet supported!");

                // TODO: Add Colors Support
                case Types.DataTypes.Colors:
                    throw new NotImplementedException(
                        "Could not save, Colors materials are not yet supported!");

                // TODO: Add 06 Support
                case Types.DataTypes.S06:
                    throw new NotImplementedException(
                        "Could not save, '06 materials are not yet supported!");

                // TODO: Add Heroes/Shadow Support
                case Types.DataTypes.Shadow:
                case Types.DataTypes.Heroes:
                    throw new NotImplementedException(
                        "Could not save, Heroes/Shadow materials are not yet supported!");

                // TODO: Add SA2 Support
                case Types.DataTypes.SA2:
                    throw new NotImplementedException(
                        "Could not save, SA2 materials are not yet supported!");

                default:
                    throw new Exception(
                        "Could not save, game type has not been set!");
            }

            mat.Save(path, true);
        }

        private static ShaderResourceView GenTexture(Texture tex)
        {
            // Ensure the texture is in a float4 format for our shaders
            tex.ConvertToFloat();

            // Compute the total size, in bytes, of the texture
            int len = 0;
            for (int i = 0; i < tex.ColorData.Length; ++i)
            {
                len += tex.ColorData[i].Length;
            }

            // Setup a DataStream and Texture Description
            var stream = new DataStream(len, true, true);
            var description = new Texture2DDescription()
            {
                Width = (int)tex.Width,
                Height = (int)tex.Height,
                ArraySize = (int)tex.Depth,
                BindFlags = BindFlags.ShaderResource,
                Usage = ResourceUsage.Default,
                CpuAccessFlags = CpuAccessFlags.None,
                Format = (Format)tex.Format,
                MipLevels = (int)tex.MipmapCount,
                OptionFlags = ResourceOptionFlags.None,
                SampleDescription = new SampleDescription(1, 0),
            };

            // Write the texture's data to the stream and
            // generate DataRectangles for each mipmap
            byte[] layer;
            var rectangles = new DataRectangle[tex.ColorData.Length];
            var p = stream.DataPointer;
            int index = 0, pitch = (int)tex.Pitch;

            for (uint slice = 0; slice < tex.Depth; ++slice)
            {
                for (uint level = 0; level < tex.MipmapCount; ++level)
                {
                    layer = tex.ColorData[index];
                    if (layer.Length > 0)
                    {
                        stream.Write(layer, 0, layer.Length);
                    }

                    rectangles[index] = new DataRectangle(p, pitch);
                    p += layer.Length;

                    pitch /= 2;
                    ++index;
                }
            }

            // Create a Texture2D and ShaderResourceView
            var tex2D = new Texture2D(Viewport.Device, description, rectangles);
            var srv = new ShaderResourceView(Viewport.Device, tex2D);
            tex2D.Dispose();

            return srv;
        }
    }
}