using System;
using System.IO;

namespace HedgeEdit.Lua
{
    public partial class LuaScript
    {
        // Methods
        protected void InitArchiveCallbacks()
        {
            script.Globals["Extract"] = (Action<string, string, string>)Extract;
            script.Globals["Repack"] = (Action<string, string,
                string, bool, bool, uint?>)Repack;
        }

        // Lua Callbacks
        public void Extract(string path, string outDir, string hashID = null)
        {
            // Format path strings, return if file doesn't exist
            path = FormatDataDir(path);
            outDir = FormatCacheDir(outDir);

            if (!File.Exists(path) || string.IsNullOrEmpty(outDir))
                return;

            Data.Extract(path, outDir, hashID);
        }

        public void Repack(string path, string inDir, string hashID = null,
            bool includeSubDirs = false, bool generateList = false,
            uint? splitCount = 0xA00000)
        {
            // Format path strings
            path = FormatDataDir(path);
            inDir = FormatCacheDir(inDir);

            if (!Directory.Exists(inDir))
                return;

            Data.Repack(path, inDir, hashID,
                includeSubDirs, generateList, splitCount);
        }
    }
}