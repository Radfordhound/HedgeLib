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
            script.Globals["LoadMaterial"] = (Func<string,
                string, GensMaterial>)LoadMaterial;

            script.Globals["LoadTexture"] = (Func<string, string, int>)LoadTexture;
            script.Globals["AddResourceDirectory"] = (Action<string>)AddResourceDirectory;
            script.Globals["AddResourceDirectoryFromPath"] =
                (Action<string>)AddResourceDirectoryFromPath;
        }

        // Lua Callbacks
        public GensMaterial LoadMaterial(string path,
            string name = null)
        {
            // Format path strings, return if file doesn't exist
            path = FormatCacheDir(path);
            if (!File.Exists(path))
                return null;

            return Data.LoadMaterial(path, name);
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