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

        public static Dictionary<string, Texture2D> Textures =
            new Dictionary<string, Texture2D>();

        public static AssetDirectories ResourceDirectories = new AssetDirectories();
        public static GensMaterial DefaultMaterial;
        public static Texture2D DefaultTexture;

        // Methods
        public static Texture2D GetTexture(string name)
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

        public static Texture2D AddTexture(string name, Texture tex)
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

        public static Texture2D LoadTexture(string path, string name = null)
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
                Texture2D texture = null;
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

        private static Texture2D GenTexture(Texture tex)
        {
            return null;
            // TODO: Multiply tex.Width by something different depending on format
            int len = 0, stride = (int)tex.Width * 4;
            for (int i = 0; i < tex.ColorData.Length; ++i)
            {
                len += tex.ColorData[i].Length;
            }

            var stream = new DataStream((int)tex.Height * stride, true, true);
            //var description = new Texture2DDescription()
            //{
            //    Width = (int)tex.Width,
            //    Height = (int)tex.Height,
            //    ArraySize = 1,
            //    MipLevels = (int)tex.MipmapCount,
            //    Format = (Format)tex.Format,
            //    SampleDescription = new SampleDescription(1, 0),
            //    //BindFlags = BindFlags.ConstantBuffer,
            //    //CpuAccessFlags = CpuAccessFlags.Write,
            //    //OptionFlags = ResourceOptionFlags.None,
            //    //Usage = ResourceUsage.Dynamic
            //};

            var description = new Texture2DDescription()
            {
                Width = (int)tex.Width,
                Height = (int)tex.Height,
                ArraySize = 1,
                BindFlags = SharpDX.Direct3D11.BindFlags.ShaderResource | BindFlags.RenderTarget,
                Usage = SharpDX.Direct3D11.ResourceUsage.Default,
                CpuAccessFlags = SharpDX.Direct3D11.CpuAccessFlags.None,
                Format = (Format)tex.Format,
                MipLevels = 1, // (int)tex.MipmapCount
                OptionFlags = ResourceOptionFlags.GenerateMipMaps, // ResourceOptionFlags.GenerateMipMap
                SampleDescription = new SharpDX.DXGI.SampleDescription(1, 0),
            };

            //System.Windows.Forms.MessageBox.Show(tex.MipmapCount.ToString());

            //using (var texture2D = new Texture2D(Viewport.Device,
            //    description, new[] { new DataBox(stream.DataPointer) }))
            //{

            //}

            foreach (var layer in tex.ColorData)
            {
                if (layer.Length < 1)
                    continue;

                stream.Write(layer, 0, layer.Length);
                break;
            }

            //var samplerState = new SamplerState(Viewport.Device, new SamplerStateDescription()
            //{
            //    AddressU
            //});

            // TODO: Multiply tex.Width by something different depending on format
            return new Texture2D(Viewport.Device, description,
                new DataRectangle(stream.DataPointer, stride));

            //return new Texture2D(Viewport.Device, description,
            //    new[] { new DataBox(stream.DataPointer, (int)tex.Width * 4, 0) });

            //    var texture = new Texture2D(Viewport.Device, new Texture2DDescription()
            //{
            //    CpuAccessFlags = CpuAccessFlags.None,
            //    MipLevels = (int)tex.MipmapCount,
            //    Height = (int)tex.Height,
            //    Width = (int)tex.Width,
            //    Format = SharpDX.DXGI.Format.A8P8, // TODO
            //}, );


            // TODO
            //int texture = GL.GenTexture();
            //GL.BindTexture(TextureTarget.Texture2D, texture);

            //if (tex == null)
            //    throw new ArgumentNullException("tex");

            //// Set Parameters
            //GL.TexParameter(TextureTarget.Texture2D,
            //    TextureParameterName.TextureMinFilter,
            //    (float)TextureMinFilter.LinearMipmapLinear);

            //GL.TexParameter(TextureTarget.Texture2D,
            //    TextureParameterName.TextureMagFilter,
            //    (int)TextureMagFilter.Linear);

            //GL.TexParameter(TextureTarget.Texture2D,
            //    TextureParameterName.TextureWrapS,
            //    (int)TextureWrapMode.Repeat);

            //GL.TexParameter(TextureTarget.Texture2D,
            //    TextureParameterName.TextureWrapT,
            //    (int)TextureWrapMode.Repeat);

            //GL.TexParameter(TextureTarget.Texture2D,
            //    TextureParameterName.TextureBaseLevel,
            //    0);

            //GL.TexParameter(TextureTarget.Texture2D,
            //    TextureParameterName.TextureMaxLevel,
            //    (int)tex.MipmapCount - 1);

            //// Generate textures
            //uint mipmapCount = ((tex.MipmapCount == 0) ? 1 : tex.MipmapCount);
            //int w = (int)tex.Width, h = (int)tex.Height;
            //for (int i = 0; i < mipmapCount; ++i)
            //{
            //    // Un-Compressed
            //    if (tex.CompressionFormat == Texture.CompressionFormats.None)
            //    {
            //        GL.TexImage2D(TextureTarget2d.Texture2D,
            //            i, // level
            //            (TextureComponentCount)tex.PixelFormat,
            //            w,
            //            h,
            //            0, // border
            //            (PixelFormat)tex.PixelFormat,
            //            PixelType.UnsignedByte,
            //            tex.ColorData[i]);
            //    }

            //    // Compressed
            //    else
            //    {
            //        GL.CompressedTexImage2D(TextureTarget2d.Texture2D,
            //            i, // level
            //            (CompressedInternalFormat)tex.CompressionFormat,
            //            w,
            //            h,
            //            0, // border
            //            tex.ColorData[i].Length,
            //            tex.ColorData[i]);
            //    }

            //    w /= 2;
            //    h /= 2;
            //}

            //return texture;
            //return 0;
        }
    }
}