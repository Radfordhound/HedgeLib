using HedgeEdit.UI;
using HedgeLib.Materials;
using HedgeLib.Textures;
using System;
using System.IO;

namespace HedgeEdit.Lua
{
    public partial class LuaScript
    {
        // Methods
        protected void InitMaterialCallbacks()
        {
            script.Globals["LoadMaterial"] = (Func<string,
                string, string, GensMaterial>)LoadMaterial;

            script.Globals["LoadTexture"] = (Func<string, string, Texture>)LoadTexture;
        }

        // Lua Callbacks
        public GensMaterial LoadMaterial(string path, string name, string texDir)
        {
            // Don't bother loading this material again if we've already loaded it
            if (string.IsNullOrEmpty(name))
                name = Path.GetFileNameWithoutExtension(path);

            if (Viewport.Materials.ContainsKey(name))
                return null;

            // Format path strings, return if file doesn't exist
            path = string.Format(path, Stage.CacheDir, Stage.ID);
            if (!File.Exists(path))
                return null;

            // Figure out texture directory
            if (string.IsNullOrEmpty(texDir))
                texDir = Path.GetDirectoryName(path);

            // Figure out what type of material to use
            GensMaterial mat; // TODO: Set to generic material type once one is made
            switch (game)
            {
                case Games.Forces:
                case Games.LW:
                case Games.Gens:
                case Games.SU:
                    mat = new GensMaterial();
                    break;

                // TODO: Add Storybook Support
                case Games.Storybook:
                    throw new NotImplementedException(
                        "Could not load, Storybook materials are not yet supported!");

                // TODO: Add Colors Support
                case Games.Colors:
                    throw new NotImplementedException(
                        "Could not load, Colors materials are not yet supported!");

                // TODO: Add 06 Support
                case Games.S06:
                    throw new NotImplementedException(
                        "Could not load, '06 materials are not yet supported!");

                // TODO: Add Heroes/Shadow Support
                case Games.Shadow:
                case Games.Heroes:
                    throw new NotImplementedException(
                        "Could not load, Heroes/Shadow materials are not yet supported!");

                // TODO: Add SA2 Support
                case Games.SA2:
                    throw new NotImplementedException(
                        "Could not load, SA2 materials are not yet supported!");

                default:
                    throw new Exception(
                        "Could not load, game type has not been set!");
            }

            // Material
            mat.Load(path);
            Viewport.Materials.Add(name, mat);

            // Textures
            foreach (var tex in mat.Texset.Textures)
            {
                // TODO: Make extension type-specific
                LoadTexture(Path.Combine(texDir,
                    $"{tex.TextureName}{DDS.Extension}"),
                    tex.TextureName);
            }

            return mat;
        }

        public Texture LoadTexture(string path, string name)
        {
            // Don't bother loading this texture again if we've already loaded it
            name = (string.IsNullOrEmpty(name)) ?
                Path.GetFileNameWithoutExtension(path) : name;

            if (Viewport.Textures.ContainsKey(name))
                return null;

            // Format path strings, return if file doesn't exist
            path = string.Format(path, Stage.CacheDir, Stage.ID);
            if (!File.Exists(path))
                return null;

            // Figure out what type of texture to use
            Texture tex;
            switch (game)
            {
                case Games.Forces:
                case Games.LW:
                case Games.Gens:
                case Games.SU:
                case Games.S06:
                    tex = new DDS();
                    break;

                // TODO: Add Storybook Support
                case Games.Storybook:
                    throw new NotImplementedException(
                        "Could not load, Storybook textures are not yet supported!");

                // TODO: Add Colors Support
                case Games.Colors:
                    throw new NotImplementedException(
                        "Could not load, Colors textures are not yet supported!");

                // TODO: Add Heroes/Shadow Support
                case Games.Shadow:
                case Games.Heroes:
                    throw new NotImplementedException(
                        "Could not load, Heroes/Shadow textures are not yet supported!");

                // TODO: Add SA2 Support
                case Games.SA2:
                    throw new NotImplementedException(
                        "Could not load, SA2 textures are not yet supported!");

                default:
                    throw new Exception(
                        "Could not load, game type has not been set!");
            }

            // Load Texture
            try
            {
                tex.Load(path);
                MainUIInvoke(() =>
                {
                    Viewport.AddTexture(name, tex);
                });
            }
            catch (Exception ex)
            {
                LuaTerminal.LogError($"ERROR: {ex.Message}");
                return null;
            }

            return tex;
        }
    }
}