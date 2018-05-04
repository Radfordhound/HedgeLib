using HedgeEdit.UI;
using HedgeLib.Materials;
using System;
using System.IO;

namespace HedgeEdit.Lua
{
    public partial class LuaScript
    {
        // Methods
        protected void InitMaterialCallbacks()
        {
            script.Globals["GetMaterials"] = (Func<
                AssetCollection<GensMaterial>>)GetMaterials;

            script.Globals["LoadMaterial"] = (Func<string, string,
                bool, GensMaterial>)LoadMaterial;

            script.Globals["SaveMaterial"] = (Action<string, GensMaterial>)SaveMaterial;
            script.Globals["SaveMaterials"] = (Action<string, string, string, bool>)SaveMaterials;

            script.Globals["LoadTexture"] = (Func<string, string, int>)LoadTexture;
            script.Globals["AddResourceDirectory"] = (Action<string>)AddResourceDirectory;
            script.Globals["AddResourceDirectoryFromPath"] =
                (Action<string>)AddResourceDirectoryFromPath;
        }

        // Lua Callbacks
        public AssetCollection<GensMaterial> GetMaterials()
        {
            return Data.Materials;
        }

        public GensMaterial LoadMaterial(string path,
            string name = null, bool nonEditable = true)
        {
            // Format path strings, return if file doesn't exist
            path = FormatCacheDir(path);
            if (!File.Exists(path))
                return null;

            return Data.LoadMaterial(path, name, nonEditable);
        }

        public void SaveMaterial(string path, GensMaterial mat)
        {
            path = FormatCacheDir(path);
            Data.SaveMaterial(path, mat);
        }

        public void SaveMaterials(string dir, string prefix,
            string suffix, bool showProgress = true)
        {
            // Format path strings
            Asset<GensMaterial> mat;
            dir = FormatCacheDir(dir);
            Directory.CreateDirectory(dir);

            // Save Materials
            int count = Data.Materials.Count, i = 0;
            if (showProgress)
                GUI.ShowProgress();

            foreach (var matAsset in Data.Materials)
            {
                mat = matAsset.Value;
                if (mat.NonEditable)
                    continue;

                if (showProgress)
                {
                    GUI.ChangeProgress((int)(((float)i / count) * 100));
                    GUI.ChangeSaveStatus(string.Format(
                        "Material {0:D2}/{1:D2}", ++i, count));
                }

                Data.SaveMaterial(Path.Combine(dir, mat.Directory.Name,
                    $"{prefix}{matAsset.Key}{suffix}"), mat);
            }

            if (showProgress)
                GUI.HideProgress();
        }

        public int LoadTexture(string path, string name = null)
        {
            // Format path strings, return if file doesn't exist
            path = FormatCacheDir(path);
            if (!File.Exists(path))
                return -1;

            return Data.LoadTexture(path, name);
        }

        public void AddResourceDirectory(string dir)
        {
            dir = FormatCacheDir(dir);
            Data.AddModelDirectory(dir);
            Data.AddResourceDirectory(dir);
        }

        public void AddResourceDirectoryFromPath(string path)
        {
            path = FormatCacheDir(path);
            Data.AddModelDirectoryFromPath(path);
            Data.AddResourceDirectoryFromPath(path);
        }
    }
}