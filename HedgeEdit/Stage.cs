using HedgeLib;
using HedgeLib.Materials;
using HedgeLib.Misc;
using HedgeLib.Models;
using HedgeLib.Sets;
using HedgeLib.Terrain;
using HedgeLib.Textures;
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
        public static string ID, CacheDir;

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
                throw new Exception("Cannot load stage. Invalid Stage ID!");
            }

            GameType = game;
            ID = stageID;

            // Make cache directory
            string cacheDir = Helpers.CombinePaths(Program.StartupPath,
                Program.CachePath, stageID);

            CacheDir = cacheDir;

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
            CurrentSetLayer = Sets[0];

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
                            if (!string.IsNullOrEmpty(entry.SearchPattern))
                            {
                                string searchPattern = string.Format(entry.SearchPattern, stgID);
                                foreach (var file in Directory.GetFiles(fullPath, searchPattern))
                                {
                                    var fileInfo = new FileInfo(file);
                                    string shortName = fileInfo.Name.Substring(0,
                                        fileInfo.Name.Length - fileInfo.Extension.Length);

                                    string dest = string.Format(
                                        fullCachePath, stgID, shortName);

                                    var hashes = UnpackArchive(game, file,
                                        dest, editorCache, arcIndex);

                                    if (hashes != null)
                                        arcHashes.Add(hashes);

                                    ++arcIndex;
                                }
                            }
                            else
                            {
                                var hashes = UnpackArchive(game, fullPath,
                                    fullCachePath, editorCache, arcIndex);

                                if (hashes != null)
                                    arcHashes.Add(hashes);

                                ++arcIndex;
                            }
                            
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

            // Sub-Methods
            void CopyData(string path, string destPath)
            {
                Directory.CreateDirectory(Path.GetDirectoryName(destPath));
                File.Copy(path, destPath, true);
            }
        }

        private static List<string> UnpackArchive(GameEntry game, string fullPath,
            string fullCachePath, EditorCache editorCache, int arcIndex)
        {
            // Get file hashes
            var arc = Types.GetArchiveOfType(game.DataType);
            if (arc == null)
                return null;

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

            // Unpack Archive if hash changed (or was not present)
            if (!hashesMatch)
            {
                if (Directory.Exists(fullCachePath))
                    Directory.Delete(fullCachePath, true);

                if (arc == null) return null;
                if (!File.Exists(fullPath))
                {
                    throw new FileNotFoundException(
                        "The given archive could not be found!", fullPath);
                }

                Directory.CreateDirectory(fullCachePath);

                arc.Load(fullPath);
                arc.Extract(fullCachePath);
            }

            return arcHashesSub;
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
                        LoadFile(entry.Key, file, game, dirEntry.Arguments);
                    }
                }
            }

            // Load Files
            foreach (var entry in game.LoadInfo.Files)
            {
                var fileEntry = entry.Value;
                string fileName = string.Format(fileEntry.FilePath, stageID);
                string file = Helpers.CombinePaths(cacheDir, fileName);
                LoadFile(entry.Key, file, game, fileEntry.Arguments);
            }
        }

        private static void LoadFile(string type, string filePath,
            GameEntry game, List<string> args)
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
                        ChangeUILoadStatus("Lights");

                        // Load Light-List
                        var lightList = new HedgeLib.Lights.GensLightList();
                        lightList.Load(filePath);

                        // Load all lights in list
                        foreach (var lightName in lightList.LightNames)
                        {
                            string lightPath = Helpers.CombinePaths(fileInfo.DirectoryName,
                                lightName + HedgeLib.Lights.Light.Extension);
                            LoadFile("light", lightPath, game, null);
                        }

                        // TODO: Spawn lights in viewport

                        return;
                    }

                case "gensstagexml":
                    {
                        var stageXML = new GensStageXML();
                        stageXML.Load(filePath);

                        // TODO: Load sonic spawn data.
                        // TODO: Load path data.

                        return;
                    }

                case "setdata":
                    {
                        ChangeUILoadStatus("Set Data");

                        var setData = Types.GetSetDataOfType(game.DataType);
                        setData.Load(filePath, game.ObjectTemplates);

                        // Spawn Objects in World
                        for (int i = 0; i < setData.Objects.Count; ++i)
                        {
                            // TODO: Load actual models.
                            SpawnObject(setData.Objects[i], game.UnitMultiplier);
                        }

                        setData.Name = Path.GetFileNameWithoutExtension(filePath);
                        Sets.Add(setData);

                        // Refresh UI Scene View
                        GUIInvoke(new Action(() =>
                        {
                            Program.MainForm.RefreshSceneView();
                        }));

                        return;
                    }

                case "terrain":
                    {
                        ChangeUILoadStatus("Terrain");
                        string shortName = fileInfo.Name.Substring(0,
                            fileInfo.Name.Length - fileInfo.Extension.Length);

                        // Terrain Instance Info
                        string instancePath = Path.Combine(fileInfo.DirectoryName,
                            string.Format("{0}{1}",
                            shortName, GensTerrainInstanceInfo.Extension));

                        var info = new GensTerrainInstanceInfo();
                        if (File.Exists(instancePath))
                            info.Load(instancePath);

                        // Terrain Model
                        if (Viewport.Terrain.ContainsKey(shortName))
                        {
                            // Don't bother loading the model again if we've
                            // already loaded a model with the same name.
                            GUIInvoke(new Action(() =>
                            {
                                Viewport.AddInstance(shortName,
                                    info.Position, info.Rotation,
                                    info.Scale, false);
                            }));
                            break;
                        }

                        // TODO
                        if (shortName.Contains("_noGI"))
                        {
                            break;
                        }

                        var model = new GensModel();
                        model.Load(filePath);
                        model.Name = shortName;

                        // Materials
                        string matDir = (args == null || args.Count < 1) ?
                            fileInfo.DirectoryName : Path.Combine(CacheDir,
                            string.Format(args[0], ID));

                        foreach (var mesh in model.Meshes)
                        {
                            if (Viewport.Materials.ContainsKey(mesh.MaterialName))
                                continue;

                            var mat = new GensMaterial();
                            mat.Load(Path.Combine(matDir,
                                $"{mesh.MaterialName}{GensMaterial.Extension}"));

                            Viewport.Materials.Add(mesh.MaterialName, mat);

                            // Textures
                            foreach (var tex in mat.Textures)
                            {
                                if (Viewport.Textures.ContainsKey(tex.TextureName))
                                    continue;

                                var dds = new DDS();
                                try
                                {
                                    dds.Load(Path.Combine(matDir,
                                        $"{tex.TextureName}{DDS.Extension}"));
                                }
                                catch (Exception ex)
                                {
                                    Console.WriteLine(ex.Message);
                                    continue;
                                }

                                GUIInvoke(new Action(() =>
                                {
                                    Viewport.AddTexture(tex.TextureName, dds);
                                }));
                            }
                        }

                        GUIInvoke(new Action(() =>
                        {
                            Viewport.AddTerrainModel(model);
                            Viewport.AddInstance(model.Name, info.Position,
                                info.Rotation, info.Scale, false);
                        }));
                        return;
                    }

                case "terrainlist":
                    {
                        if (args == null || args.Count < 1)
                        {
                            Console.WriteLine(
                                "Skipped loading terrain, no extracted PFD path given.");
                            return;
                        }

                        // Terrain List
                        ChangeUILoadStatus("Terrain List");
                        string dir = fileInfo.DirectoryName;

                        var terrainList = new GensTerrainList();
                        terrainList.Load(filePath);

                        // Terrain Groups
                        int groupCount = terrainList.GroupEntries.Length;
                        for (int i = 0; i < groupCount; ++i)
                        {
                            // Update UI
                            ChangeUILoadStatus($"Terrain Group {i}/{groupCount}");
                            ChangeUIProgress((int)((i / (float)groupCount) * 100), true);

                            // Get the path to the terrain group
                            var groupEntry = terrainList.GroupEntries[i];
                            string groupDir = Path.Combine(
                                fileInfo.Directory.Parent.FullName, args[0],
                                groupEntry.FileName);

                            string path = Path.Combine(dir,
                                $"{groupEntry.FileName}{GensTerrainGroup.Extension}");

                            // Ensure the group exists
                            if (!File.Exists(path))
                            {
                                Console.WriteLine(
                                    "Terrain group {0} was skipped because it was not found!",
                                    groupEntry.FileName);

                                continue;
                            }

                            // Load the group
                            var group = new GensTerrainGroup();
                            group.Load(path);

                            // Terrain Instance Infos
                            for (int i2 = 0; i2 < group.InstanceInfos.Length; ++i2)
                            {
                                var instanceInfo = group.InstanceInfos[i2];
                                for (int i3 = 0; i3 < instanceInfo.FileNames.Length; ++i3)
                                {
                                    string instancePath = Path.Combine(groupDir,
                                        string.Format("{0}{1}",
                                        instanceInfo.FileNames[i3],
                                        GensTerrainInstanceInfo.Extension));

                                    if (!File.Exists(instancePath))
                                        continue;

                                    var info = new GensTerrainInstanceInfo();
                                    info.Load(instancePath);

                                    // Terrain Models
                                    if (Viewport.Terrain.ContainsKey(info.ModelFileName))
                                    {
                                        // Don't bother loading the model again if we've
                                        // already loaded a model with the same name.
                                        GUIInvoke(new Action(() =>
                                        {
                                            Viewport.AddInstance(info.ModelFileName,
                                                info.Position, info.Rotation,
                                                info.Scale, false);
                                        }));
                                        continue;
                                    }

                                    string modelPth = Path.Combine(groupDir,
                                        string.Format("{0}{1}",
                                        info.ModelFileName,
                                        GensModel.TerrainExtension));

                                    var model = new GensModel();
                                    model.Load(modelPth);
                                    model.Name = info.ModelFileName;

                                    // Materials
                                    foreach (var mesh in model.Meshes)
                                    {
                                        if (Viewport.Materials.ContainsKey(mesh.MaterialName))
                                            continue;

                                        var mat = new GensMaterial();
                                        mat.Load(Path.Combine(fileInfo.DirectoryName,
                                            $"{mesh.MaterialName}{GensMaterial.Extension}"));

                                        Viewport.Materials.Add(mesh.MaterialName, mat);

                                        // Textures
                                        foreach (var tex in mat.Textures)
                                        {
                                            if (Viewport.Textures.ContainsKey(tex.TextureName))
                                                continue;

                                            var dds = new DDS();
                                            try
                                            {
                                                dds.Load(Path.Combine(fileInfo.DirectoryName,
                                                    $"{tex.TextureName}{DDS.Extension}"));
                                            }
                                            catch (Exception ex)
                                            {
                                                Console.WriteLine(ex.Message);
                                                continue;
                                            }

                                            GUIInvoke(new Action(() =>
                                            {
                                                Viewport.AddTexture(tex.TextureName, dds);
                                            }));
                                        }
                                    }

                                    GUIInvoke(new Action(() =>
                                    {
                                        Viewport.AddTerrainModel(model);
                                        Viewport.AddInstance(model.Name, info.Position,
                                            info.Rotation, info.Scale, false);
                                    }));
                                }
                            }
                        }

                        ChangeUIProgress(100, false);
                        return;
                    }

                // TODO: Add more types.
            }
        }

        private static void GUIInvoke(Action action)
        {
            if (Program.MainForm == null || Program.MainForm.Disposing ||
                Program.MainForm.IsDisposed)
                return;

            Program.MainForm.Invoke(action);
        }

        private static void SpawnObject(
            SetObject obj, float unitMultiplier)
        {
            GUIInvoke(new Action(() =>
            {
                Viewport.AddInstance(obj.ObjectType,
                    obj.Transform.Position * unitMultiplier,
                    obj.Transform.Rotation, obj.Transform.Scale,
                    true, obj);

                if (obj.Children == null) return;
                foreach (var child in obj.Children)
                {
                    if (child == null) continue;

                    Viewport.AddInstance(obj.ObjectType,
                        child.Position * unitMultiplier,
                        child.Rotation, child.Scale,
                        true, child);
                }
            }));
        }

        private static void ChangeUILoadStatus(string status)
        {
            GUIInvoke(new Action(() =>
            {
                Program.MainForm.UpdateStatus($"Loading {status}...");
            }));
        }

        private static void ChangeUIStatus(string status)
        {
            GUIInvoke(new Action(() =>
            {
                Program.MainForm.UpdateStatus(status);
            }));
        }

        private static void ChangeUIProgress(int progress, bool visible)
        {
            GUIInvoke(new Action(() =>
            {
                Program.MainForm.UpdateProgress(progress, visible);
            }));
        }
    }
}