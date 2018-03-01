using HedgeLib.Terrain;
using System;
using System.IO;

namespace HedgeEdit.Lua
{
    public partial class LuaScript
    {
        // Methods
        protected void InitTerrainCallbacks()
        {
            script.Globals["LoadTerrainList"] = (Func<string, string,
                string, GensTerrainList>)LoadTerrainList;

            script.Globals["LoadTerrain"] = (Func<string, string,
                string, bool, VPModel>)LoadTerrain;
        }

        // Lua Callbacks
        public GensTerrainList LoadTerrainList(string path, string groupsDir, string resDir)
        {
            // Format path strings, return if the given files/directories don't exist
            path = FormatCacheDir(path);
            groupsDir = FormatCacheDir(groupsDir);
            resDir = FormatCacheDir(resDir);

            if (!File.Exists(path) || !Directory.Exists(groupsDir) ||
                !Directory.Exists(resDir))
                return null;

            return Data.LoadTerrainList(path, groupsDir, resDir);
        }

        public VPModel LoadTerrain(string path, string resDir,
            string group = null, bool loadMats = true)
        {
            // Format path strings, return if the given files/directories don't exist
            path = FormatCacheDir(path);
            resDir = FormatCacheDir(resDir);

            if (!File.Exists(path) || !Directory.Exists(resDir))
                return null;

            return Data.LoadModel(path, resDir,
                true, loadMats, group);
        }
    }
}