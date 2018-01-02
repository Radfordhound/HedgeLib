using HedgeLib.Sets;
using System;
using System.Collections.Generic;
using System.IO;

namespace HedgeEdit
{
    public class Stage
    {
        // Variables/Constants
        public static List<SetData> Sets = new List<SetData>();
        public static SetData CurrentSetLayer = null;
        public static GameEntry GameType;
        public static EditorCache EditorCache;
        public static string ID, DataDir, CacheDir;

        private static LuaScript script;
        private static string scriptPath;

        // Methods
        public static void Load(string dataDir, string stageID, GameEntry game)
        {
            Viewport.Clear();

            // Throw exceptions if necessary
            if (!Directory.Exists(dataDir))
            {
                throw new DirectoryNotFoundException(
                    $"Cannot load stage from \"{dataDir}\". Data Directory not found!");
            }

            if (string.IsNullOrEmpty(stageID))
            {
                throw new ArgumentNullException("stageID",
                    "Cannot load stage. Invalid Stage ID!");
            }

            GameType = game;
            ID = stageID;
            DataDir = dataDir;

            // Make cache directory
            CacheDir = Path.Combine(Program.StartupPath,
                Program.CachePath, stageID);

            string editorCachePath = Path.Combine(
                CacheDir, EditorCache.FileName);
            Directory.CreateDirectory(CacheDir);

            // Load Editor Cache
            bool cacheExists = File.Exists(editorCachePath);
            EditorCache = new EditorCache()
            {
                GameType = game.Name,
            };

            if (cacheExists)
            {
                var editorCache = new EditorCache();
                editorCache.Load(editorCachePath);

                if (editorCache.GameType == game.Name)
                    EditorCache = editorCache;
            }

            // Lua Script
            string pth = Path.Combine(Program.StartupPath,
                Program.ScriptsPath, LuaScript.GamesDir,
                $"{game.ShortName}{LuaScript.Extension}");

            if (script == null || scriptPath != pth)
            {
                script = new LuaScript();
                scriptPath = pth;

                try
                {
                    script.DoScript(scriptPath);
                }
                catch (Exception ex)
                {
                    LuaTerminal.LogError($"ERROR: {ex.Message}");
                }
            }

            // Unpack/Load
            var unpackStopWatch = System.Diagnostics.Stopwatch.StartNew();

            try
            {
                script.Call("Load", dataDir, CacheDir, stageID);
            }
            catch (Exception ex)
            {
                LuaTerminal.LogError($"ERROR: {ex.Message}");
            }

            unpackStopWatch.Stop();
            Console.WriteLine("Done unpacking! Time: {0}(ms).",
                unpackStopWatch.ElapsedMilliseconds);

            // Generate new Editor Cache
            if (cacheExists)
                File.Delete(editorCachePath);

            EditorCache.Save(editorCachePath, true);

            // TODO: Remove this line
            //CurrentSetLayer = Sets[0];
        }

        public static void SaveSets()
        {
            SaveSets(Sets, DataDir, CacheDir);
        }

        public static void SaveSets(List<SetData> sets, string dataDir, string cacheDir)
        {
            // Argument Checks
            if (sets == null)
                throw new ArgumentNullException("sets");

            if (string.IsNullOrEmpty(dataDir))
                throw new ArgumentNullException("dataDir");

            if (string.IsNullOrEmpty(cacheDir))
                throw new ArgumentNullException("cacheDir");

            // Save Sets
            try
            {
                script.Call("SaveSets", dataDir, CacheDir, ID);
            }
            catch (Exception ex)
            {
                LuaTerminal.LogError($"ERROR: {ex.Message}");
            }

            // Generate new Editor Cache
            string editorCachePath = Path.Combine(
                CacheDir, EditorCache.FileName);

            if (File.Exists(editorCachePath))
                File.Delete(editorCachePath);

            EditorCache.Save(editorCachePath, true);
        }
    }
}