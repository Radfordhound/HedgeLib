using HedgeLib;
using HedgeLib.Archives;
using HedgeLib.Misc;
using HedgeLib.Sets;
using System;
using System.Collections.Generic;
using System.IO;

namespace HedgeEdit
{
    public class Stage
    {
        //Variables/Constants
        public static List<SetData> Sets = new List<SetData>();
        public static Types.DataTypes DataType;

        //Methods
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
                throw new Exception("Cannot load stage. Invalid Stage ID!");
            }

            DataType = game.DataType;

            // Make cache directory
            string cacheDir = Helpers.CombinePaths(Program.StartupPath,
                Program.CachePath, stageID);

            string editorCachePath = Helpers.CombinePaths(cacheDir, EditorCache.FileName);
            Directory.CreateDirectory(cacheDir);

            // Load Editor Cache
            EditorCache editorCache = null;

            if (File.Exists(editorCachePath))
            {
                editorCache = new EditorCache();
                editorCache.Load(editorCachePath);

                if (editorCache.GameType.ToLower() != game.Name.ToLower())
                    editorCache = null;
            }

            // Unpack Data
            var unpackStopWatch = System.Diagnostics.Stopwatch.StartNew();
            var arcHashes = Unpack(cacheDir, editorCache,
                dataDir, stageID, game);
            unpackStopWatch.Stop();
            Console.WriteLine("Done unpacking! Time: {0}(ms).",
                unpackStopWatch.ElapsedMilliseconds);

            // Generate new Editor Cache
            if (editorCache != null) File.Delete(editorCachePath);
            editorCache = new EditorCache()
            {
                GameType = game.Name,
                ArcHashes = arcHashes
            };

            editorCache.Save(editorCachePath);

            // Load Data
            var loadStopWatch = System.Diagnostics.Stopwatch.StartNew();
            LoadFromCache(cacheDir, stageID, game);
            loadStopWatch.Stop();
            Console.WriteLine("Done loading! Time: {0}(ms).",
                loadStopWatch.ElapsedMilliseconds);
        }

        private static List<List<string>> Unpack(string cacheDir, EditorCache editorCache,
            string dataDir, string stgID, GameEntry game)
        {
            var arcHashes = new List<List<string>>();
            int arcIndex = 0;

            foreach (var entry in game.UnpackInfo)
            {
                string path = string.Format(entry.Path, stgID);
                string cachePath = string.Format(entry.CachePath, stgID, "{1}"); // Hacky but idc
                string fullPath = Helpers.CombinePaths(dataDir, path);
                string fullCachePath = Helpers.CombinePaths(cacheDir, cachePath);
                string dataType = entry.Type.ToLower();

                if (!File.Exists(fullPath) && !Directory.Exists(fullPath))
                {
                    Console.WriteLine(
                        "WARNING: Skipping \"{0}\", as it could not be found!", path);

                    if (dataType == "archive") ++arcIndex;
                    continue;
                }

                switch (dataType)
                {
                    case "archive":
                        {
                            // Get file hashes
                            var arc = Types.GetArchiveOfType(game.DataType);
                            if (arc == null)
                            {
                                ++arcIndex;
                                continue;
                            }

                            bool hashesMatch = true;

                            var arcFileList = arc.GetSplitArchivesList(fullPath);
                            var arcHashesSub = new List<string>();
                            var editorCacheHashes = (editorCache == null ||
                                arcIndex >= editorCache.ArcHashes.Count) ?
                                    null : editorCache.ArcHashes[arcIndex];

                            for (int i = 0; i < arcFileList.Count; ++i)
                            {
                                string file = arcFileList[i];
                                string arcHash = Helpers.GetFileHash(file);
                                arcHashesSub.Add(arcHash);

                                if (editorCacheHashes == null || i >= editorCacheHashes.Count ||
                                    editorCacheHashes[i] != arcHash)
                                {
                                    hashesMatch = false;
                                }
                            }
                            arcHashes.Add(arcHashesSub);

                            // Unpack Archive if hash changed (or was not present)
                            if (!hashesMatch)
                            {
                                if (Directory.Exists(fullCachePath))
                                    Directory.Delete(fullCachePath, true);

                                UnpackArchive(arc, fullPath, fullCachePath);
                            }

                            ++arcIndex;
                            break;
                        }

                    case "file":
                        {
                            CopyData(fullPath, fullCachePath);
                            break;
                        }

                    case "files":
                        {
                            string searchPattern = string.Format(entry.SearchPattern, stgID);
                            foreach (var file in Directory.GetFiles(fullPath, searchPattern))
                            {
                                var fileInfo = new FileInfo(file);
                                string dest = string.Format(fullCachePath, stgID, fileInfo.Name);
                                CopyData(file, dest);
                            }
                            break;
                        }
                }
            }

            return arcHashes;

            //Sub-Methods
            void CopyData(string path, string destPath)
            {
                Directory.CreateDirectory(Path.GetDirectoryName(destPath));
                File.Copy(path, destPath, true);
            }

            void UnpackArchive(Archive arc, string arcPath, string dir)
            {
                if (arc == null) return;
                if (!File.Exists(arcPath))
                {
                    throw new FileNotFoundException(
                        "The given archive could not be found!", arcPath);
                }

                Directory.CreateDirectory(dir);

                arc.Load(arcPath);
                arc.Extract(dir);
            }
        }

        private static void LoadFromCache(string cacheDir,
            string stageID, GameEntry game)
        {
            // Load Directories
            foreach (var entry in game.LoadInfo.Directories)
            {
                var dirEntry = entry.Value;
                var dirs = string.Format(dirEntry.Directory, stageID).Split('|');

                foreach (string dir in dirs)
                {
                    string fullDir = Helpers.CombinePaths(cacheDir, dir);
                    if (!Directory.Exists(fullDir))
                    {
                        Console.WriteLine(
                            "WARNING: Could not load directory \"{0}\" as it does not exist!",
                            fullDir);
                        continue;
                    }

                    foreach (string file in Directory.GetFiles(fullDir, dirEntry.Filter))
                    {
                        LoadFile(entry.Key, file, game);
                    }
                }
            }

            // Load Files
            foreach (var entry in game.LoadInfo.Files)
            {
                string fileName = string.Format(entry.Value, stageID);
                string file = Helpers.CombinePaths(cacheDir, fileName);
                LoadFile(entry.Key, file, game);
            }
        }

        private static void LoadFile(string type, string filePath, GameEntry game)
        {
            if (!File.Exists(filePath))
            {
                Console.WriteLine(
                    "WARNING: Could not load file \"{0}\" as it does not exist!",
                    filePath);
                return;
            }

            var fileInfo = new FileInfo(filePath);
            switch (type.ToLower())
            {
                case "lightlist":
                    {
                        // Load Light-List
                        var lightList = new HedgeLib.Lights.GensLightList();
                        lightList.Load(filePath);

                        // Load all lights in list
                        foreach (var lightName in lightList.LightNames)
                        {
                            string lightPath = Helpers.CombinePaths(fileInfo.DirectoryName,
                                lightName + HedgeLib.Lights.Light.Extension);
                            LoadFile("light", lightPath, game);
                        }

                        return;
                    }

                case "gensstagexml":
                    {
                        var stageXML = new GensStageXML();
                        stageXML.Load(filePath);

                        //TODO: Load sonic spawn data.
                        //TODO: Load path data.

                        return;
                    }

                case "setdata":
                    {
                        var setData = Types.GetSetDataOfType(game.DataType);
                        Console.WriteLine("Loading sets " + filePath);
                        setData.Load(filePath, game.ObjectTemplates);

                        // Spawn Objects in World
                        for (int i = 0; i < setData.Objects.Count; ++i)
                        {
                            // TODO: Load actual models.
                            var obj = setData.Objects[i];
                            Viewport.AddModel(Viewport.DefaultCube,
                                obj.Transform.Position * game.UnitMultiplier,
                                obj.Transform.Rotation);

                            // Spawn Child Objects
                            // TODO: Move this to it's own SpawnObject method
                            if (obj.Children == null) continue;
                            foreach (var transform in obj.Children)
                            {
                                if (transform == null) continue;

                                Viewport.AddModel(Viewport.DefaultCube,
                                    transform.Position * game.UnitMultiplier,
                                    transform.Rotation);
                            }
                        }

                        setData.Name = Path.GetFileNameWithoutExtension(filePath);
                        Sets.Add(setData);
                        return;
                    }

                // TODO: Add more types.
            }
        }
    }
}