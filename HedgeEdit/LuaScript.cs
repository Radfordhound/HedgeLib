using HedgeLib;
using HedgeLib.Archives;
using HedgeLib.Materials;
using HedgeLib.Models;
using HedgeLib.Sets;
using HedgeLib.Terrain;
using HedgeLib.Textures;
using MoonSharp.Interpreter;
using System;
using System.Collections.Generic;
using System.IO;
using System.Windows.Forms;

namespace HedgeEdit
{
    public class LuaScript
    {
        // Variables/Constants
        public Games Game => game;
        protected Script script;
        protected Games game = Games.None;

        public enum Games
        {
            None,
            Heroes,
            Shadow,
            S06,
            Storybook,
            SU,
            Colors,
            Gens,
            LW,
            Forces
        }

        public const string GamesDir = "Games",
            PluginsDir = "Plugins", Extension = ".lua";

        // Constructors
        public LuaScript()
        {
            // TODO: Maybe set more CoreModules?
            script = new Script(CoreModules.Basic | CoreModules.String);

            // General
            script.Globals["Log"] = (Action<object>)LuaTerminal.Log;
            script.Globals["LogWarning"] = (Action<object>)LuaTerminal.LogWarning;
            script.Globals["LogError"] = (Action<object>)LuaTerminal.LogError;
            script.Globals["SetDataType"] = (Action<string>)SetDataType;

            // UI
            script.Globals["UIChangeLoadStatus"] = (Action<string>)UIChangeLoadStatus;
            script.Globals["UIChangeSaveStatus"] = (Action<string>)UIChangeSaveStatus;
            script.Globals["UIChangeStatus"] = (Action<string>)UIChangeStatus;
            script.Globals["UIChangeProgress"] = (Action<int>)UIChangeProgress;
            script.Globals["UIChangeProgressVisible"] = (Action<bool>)UIChangeProgressVisible;
            script.Globals["UIShowProgress"] = (Action)UIShowProgress;
            script.Globals["UIHideProgress"] = (Action)UIHideProgress;
            script.Globals["UIMessageBox"] = (Func<string, string, int, int, int>)UIMessageBox;
            script.Globals["UIErrorBox"] = (Func<string, string, int, int>)UIErrorBox;
            script.Globals["UIWarningBox"] = (Func<string, string, int, int>)UIWarningBox;

            // IO
            script.Globals["IOFileExists"] = (Func<string, bool>)IOFileExists;
            script.Globals["IODirExists"] = (Func<string, bool>)IODirExists;
            script.Globals["IOGetFilesInDir"] = (Func<string,
                string, bool, string[]>)IOGetFilesInDir;

            script.Globals["IOGetExtension"] = (Func<string, string>)IOGetExtension;
            script.Globals["IOGetName"] = (Func<string, string>)IOGetName;

            script.Globals["IOGetNameWithoutExtension"] =
                (Func<string, string>)IOGetNameWithoutExtension;

            script.Globals["IOCopyFile"] = (Action<string, string, bool>)IOCopyFile;

            // Values
            script.Globals["GetSetLayer"] = (Func<int, SetData>)GetSetLayer;
            script.Globals["GetSetLayerCount"] = (Func<int>)GetSetLayerCount;

            // Loading
            script.Globals["Extract"] = (Action<string, string, string>)Extract;
            script.Globals["LoadTerrainList"] = (Func<string, string,
                string, GensTerrainList>)LoadTerrainList;

            script.Globals["LoadTerrain"] = (Func<string, string, bool, Model>)LoadTerrain;
            script.Globals["LoadObjectModel"] = (Func<string,
                string, bool, Vector3?, Model>)LoadObjectModel;

            script.Globals["LoadMaterial"] = (Func<string,
                string, string, GensMaterial>)LoadMaterial;

            script.Globals["LoadTexture"] = (Func<string, string, Texture>)LoadTexture;
            script.Globals["LoadSetLayer"] = (Func<string, bool, string[], SetData>)LoadSetLayer;

            // Saving
            script.Globals["Repack"] = (Action<string, string, string, bool, bool, uint?>)Repack;
            script.Globals["SaveSetLayer"] = (Action<string, SetData>)SaveSetLayer;
            script.Globals["SaveSetLayers"] = (Action<string, string, string, bool>)SaveSetLayers;
        }

        // Methods
        public static void Initialize()
        {
            Script.DefaultOptions.DebugPrint = LuaTerminal.Log;
            UserData.RegisterType<Vector3>();
            UserData.RegisterType<Quaternion>();

            UserData.RegisterType<SetObjectParam>();
            UserData.RegisterType<SetObjectTransform>();
            UserData.RegisterType<SetObject>();
            UserData.RegisterType<SetData>();

            UserData.RegisterType<Texture>();
            UserData.RegisterType<GensMaterial>();
            UserData.RegisterType<Model>();
            UserData.RegisterType<GensTerrainList>();
        }

        public static bool EvaluateCondition(string condition)
        {
            try
            {
                var s = new Script();
                string txt = $"return ({condition.Replace("!=", "~=")})";
                var v = s.DoString(txt);
                return v.Boolean;
            }
            catch (Exception ex)
            {
                LuaTerminal.LogError($"ERROR: {ex.Message}");
                return false;
            }
        }

        public static bool EvaluateObjectCondition(SetObject obj,
            SetObjectType template, string condition)
        {
            try
            {
                var s = new Script();
                for (int i = 0; i < template.Parameters.Count; ++i)
                {
                    var param = template.Parameters[i];
                    s.Globals[param.Name] = obj.Parameters[i].Data;
                }

                string txt = $"return ({condition.Replace("!=", "~=")})";
                var v = s.DoString(txt);
                return v.Boolean;
            }
            catch (Exception ex)
            {
                LuaTerminal.LogError($"ERROR: {ex.Message}");
                return false;
            }
        }

        public void DoScript(string filePath)
        {
            script.DoFile(filePath);
        }

        public void DoString(string str)
        {
            script.DoString(str);
        }

        public void Call(string funcName, params object[] args)
        {
            script.Call(script.Globals[funcName], args);
        }

        protected static void MainUIInvoke(Action action)
        {
            if (Program.MainForm == null || Program.MainForm.Disposing ||
                Program.MainForm.IsDisposed)
                return;

            Program.MainForm.Invoke(action);
        }

        private static void SpawnObject(SetObject obj, string modelName,
            float unitMultiplier, Vector3 posOffset)
        {
            MainUIInvoke(() =>
            {
                var instance = Viewport.AddInstance(modelName,
                    (obj.Transform.Position * unitMultiplier) + posOffset,
                    obj.Transform.Rotation, obj.Transform.Scale,
                    true, obj);

                if (obj.Children == null) return;
                foreach (var child in obj.Children)
                {
                    if (child == null) continue;

                    Viewport.AddInstance(modelName,
                        (child.Position * unitMultiplier) + posOffset,
                        child.Rotation, child.Scale,
                        true, child);
                }
            });
        }

        public static Games GetGame(string dataType)
        {
            switch (dataType.ToLower())
            {
                case "forces":
                    return Games.Forces;

                case "lost world":
                case "lw":
                    return Games.LW;

                case "generations":
                case "gens":
                    return Games.Gens;

                case "colors":
                    return Games.Colors;

                case "unleashed":
                case "su":
                    return Games.SU;

                case "black knight":
                case "secret rings":
                case "storybook":
                    return Games.Storybook;

                case "s06":
                case "06":
                    return Games.S06;

                case "shadow":
                    return Games.Shadow;

                case "heroes":
                    return Games.Heroes;

                default:
                    throw new NotImplementedException(
                        $"Unknown game type \"{dataType}\"!");
            }
        }

        public void LoadSetLayerResources(GameEntry gameType, SetData setData,
            bool loadModels = true, string[] altModelDirs = null)
        {
            string resDir = gameType.ResourcesDir;
            bool resDirExists = Directory.Exists(resDir);

            foreach (var obj in setData.Objects)
            {
                var offsetPos = new Vector3(0, 0, 0);
                SetObjectType type = null;

                if (gameType.ObjectTemplates.ContainsKey(obj.ObjectType))
                    type = gameType.ObjectTemplates[obj.ObjectType];

                if (type != null)
                {
                    var offsetPosExtra = type.GetExtra("OffsetPosition");
                    if (offsetPosExtra == null)
                    {
                        offsetPos.X = GetFloatExtra("OffsetPositionX", "Offset_Position_X");
                        offsetPos.Y = GetFloatExtra("OffsetPositionY", "Offset_Position_Y");
                        offsetPos.Z = GetFloatExtra("OffsetPositionZ", "Offset_Position_Z");
                    }
                    else if (!string.IsNullOrEmpty(offsetPosExtra.Value))
                    {
                        offsetPos = (Vector3)Helpers.ChangeType(
                            offsetPosExtra.Value, typeof(Vector3));
                    }

                    // Sub-Methods
                    float GetFloatExtra(string name, string altName)
                    {
                        var extra = type.GetExtra(name);
                        if (extra == null)
                            extra = type.GetExtra(altName);

                        float.TryParse((string.IsNullOrEmpty(extra?.Value)) ?
                            "0" : extra.Value, out float f);
                        return f;
                    }
                }

                // Load Object Model
                string mdlName = null;
                if (loadModels && resDirExists && type != null)
                {
                    foreach (var extra in type.Extras)
                    {
                        if (extra.Type.ToLower() != "model")
                            continue;

                        if (string.IsNullOrEmpty(extra.Condition) ||
                            EvaluateObjectCondition(obj, type, extra.Condition))
                        {
                            mdlName = extra.Value;
                            break;
                        }
                    }
                }

                if (!string.IsNullOrEmpty(mdlName) &&
                    !Viewport.Objects.ContainsKey(mdlName))
                {
                    // If the model name is actually supposed to be the value of
                    // another parameter (e.g. in Gismos), get the name from that instead.
                    if (mdlName.IndexOf('.') == -1)
                    {
                        int mdlNameParamIndex = type.GetParameterIndex(mdlName);
                        if (mdlNameParamIndex != -1)
                        {
                            mdlName = (obj.Parameters[
                                mdlNameParamIndex].Data as string);
                        }
                    }
                    else
                    {
                        int openIndex = mdlName.IndexOf('{');
                        int closeIndex = mdlName.IndexOf('}');

                        if (openIndex != -1 && closeIndex > openIndex)
                        {
                            ++openIndex;
                            if (int.TryParse(mdlName.Substring(openIndex,
                                closeIndex - openIndex), out int index) &&
                                index >= 0 && index < type.Parameters.Count)
                            {
                                mdlName = mdlName.Replace($"{{{index}}}",
                                    (obj.Parameters[index].Data as string));
                            }
                        }
                    }

                    // Get scale if any
                    Vector3? scale = null;
                    var scaleExtra = type.GetExtra("scale");

                    if (scaleExtra != null && !string.IsNullOrEmpty(scaleExtra.Value))
                    {
                        if (float.TryParse(scaleExtra.Value, out float s))
                        {
                            scale = new Vector3(s, s, s);
                        }

                        // TODO: Maybe try to parse it as a Vector3 as well?

                        else
                        {
                            int scaleParamIndex = type.GetParameterIndex(scaleExtra.Value);
                            if (scaleParamIndex != -1)
                            {
                                var param = obj.Parameters[scaleParamIndex];
                                if (param != null)
                                {
                                    if (param.DataType == typeof(Vector3))
                                    {
                                        scale = (Vector3)param.Data;
                                    }
                                    else if (param.DataType == typeof(float))
                                    {
                                        float f = (float)param.Data;
                                        scale = new Vector3(f, f, f);
                                    }
                                }
                            }
                        }
                    }

                    // Load the model from the standard resources directory
                    var mdl = LoadObjectModel(
                        Path.Combine(resDir, mdlName),
                        resDir, true, scale);

                    // If the model could not be found, attempt to load from
                    // any given alternative resources directories.
                    if (altModelDirs != null && mdl == null)
                    {
                        foreach (string dir in altModelDirs)
                        {
                            mdl = LoadObjectModel(
                                Path.Combine(dir, mdlName),
                                dir, true, scale);
                        }
                    }
                }

                // Spawn Object in World
                SpawnObject(obj, mdlName,
                    gameType.UnitMultiplier, offsetPos);
            }
        }

        // General Callbacks
        public void SetDataType(string dataType)
        {
            game = GetGame(dataType);
        }

        // UI Callbacks
        public static void UIChangeLoadStatus(string status)
        {
            UIChangeStatus($"Loading {status}...");
        }

        public static void UIChangeSaveStatus(string status)
        {
            UIChangeStatus($"Saving {status}...");
        }

        public static void UIChangeStatus(string status)
        {
            MainUIInvoke(() =>
            {
                Program.MainForm.UpdateStatus(status);
            });
        }

        public static void UIChangeProgress(int progress)
        {
            MainUIInvoke(() =>
            {
                Program.MainForm.UpdateProgress(progress);
            });
        }

        public static void UIChangeProgressVisible(bool visible)
        {
            MainUIInvoke(() =>
            {
                Program.MainForm.UpdateProgressVisible(visible);
            });
        }

        public static void UIShowProgress()
        {
            UIChangeProgressVisible(true);
        }

        public static void UIHideProgress()
        {
            UIChangeProgressVisible(false);
        }

        public static int UIMessageBox(string txt, string caption = "",
            int buttons = 0, int icon = 0)
        {
            return (int)MessageBox.Show(txt, caption,
                (MessageBoxButtons)buttons, (MessageBoxIcon)icon);
        }

        public static int UIErrorBox(string txt, string caption = "", int buttons = 0)
        {
            return (int)MessageBox.Show(txt, caption,
                (MessageBoxButtons)buttons, MessageBoxIcon.Error);
        }

        public static int UIWarningBox(string txt, string caption = "", int buttons = 0)
        {
            return (int)MessageBox.Show(txt, caption,
                (MessageBoxButtons)buttons, MessageBoxIcon.Warning);
        }

        // IO Callbacks
        public bool IOFileExists(string path)
        {
            path = string.Format(path, Stage.CacheDir, Stage.ID);
            return File.Exists(path);
        }

        public bool IODirExists(string path)
        {
            path = string.Format(path, Stage.CacheDir, Stage.ID);
            return Directory.Exists(path);
        }

        public string[] IOGetFilesInDir(string path,
            string filter, bool includeSubDirs)
        {
            path = string.Format(path, Stage.CacheDir, Stage.ID);

            if (string.IsNullOrEmpty(filter))
                return Directory.GetFiles(path);

            return Directory.GetFiles(path, filter,
                (includeSubDirs) ? SearchOption.AllDirectories :
                SearchOption.TopDirectoryOnly);
        }

        public string IOGetExtension(string path)
        {
            path = string.Format(path, Stage.CacheDir, Stage.ID);
            return Path.GetExtension(path);
        }

        public string IOGetName(string path)
        {
            path = string.Format(path, Stage.CacheDir, Stage.ID);
            return Path.GetFileName(path);
        }

        public string IOGetNameWithoutExtension(string path)
        {
            path = string.Format(path, Stage.CacheDir, Stage.ID);
            return Path.GetFileNameWithoutExtension(path);
        }

        public void IOCopyFile(string source,
            string dest, bool overwrite = true)
        {
            source = string.Format(source, Stage.CacheDir, Stage.ID);
            dest = string.Format(dest, Stage.CacheDir, Stage.ID);
            File.Copy(source, dest, overwrite);
        }

        // Value Callbacks
        public int GetSetLayerCount()
        {
            if (Stage.Sets == null)
                return -1;

            return Stage.Sets.Count;
        }

        public SetData GetSetLayer(int index)
        {
            if (Stage.Sets == null || index >= Stage.Sets.Count)
                return null;

            return Stage.Sets[index];
        }

        // Loading Callbacks
        public void Extract(string path, string outDir, string hashID = null)
        {
            // Format path strings, return if file doesn't exist
            path = string.Format(path, Stage.DataDir, Stage.ID);
            outDir = string.Format(outDir, Stage.CacheDir, Stage.ID);

            if (!File.Exists(path) || string.IsNullOrEmpty(outDir))
                return;

            // Figure out what type of archive to use
            Archive arc = null;
            switch (game)
            {
                case Games.Forces:
                    arc = new ForcesArchive();
                    break;

                // TODO: Add LW Support
                case Games.LW:
                    throw new NotImplementedException(
                        "Could not unpack, LW archives are not yet supported!");

                case Games.Gens:
                case Games.SU:
                    arc = new GensArchive();
                    break;

                case Games.Storybook:
                    arc = new SBArchive();
                    break;

                // TODO: Add Colors Support
                case Games.Colors:
                    throw new NotImplementedException(
                        "Could not unpack, Colors archives are not yet supported!");

                case Games.S06:
                    arc = new S06Archive();
                    break;

                case Games.Shadow:
                case Games.Heroes:
                    arc = new ONEArchive();
                    break;
                
                default:
                    throw new Exception(
                        "Could not unpack, game type has not been set!");
            }

            // Hashes
            bool hashesMatch = false;
            if (!string.IsNullOrEmpty(hashID) && Stage.EditorCache != null)
            {
                var editorCache = Stage.EditorCache;

                // Get splits list
                var arcSplits = new List<string>();
                hashesMatch = true;

                try
                {
                    var arcFileList = arc.GetSplitArchivesList(path);
                    arcSplits.AddRange(arcFileList);
                }
                catch
                {
                    arcSplits.Add(path);
                }

                // Check hashes
                var splitHashes = new List<string>();
                var splitCacheHashes = (editorCache.ArcHashes.ContainsKey(
                    hashID)) ? editorCache.ArcHashes[hashID] : null;

                if (splitCacheHashes != null &&
                    splitCacheHashes.Count != arcSplits.Count)
                {
                    hashesMatch = false;
                }

                for (int i = 0; i < arcSplits.Count; ++i)
                {
                    string file = arcSplits[i];
                    string arcHash = Helpers.GetFileHash(file);
                    splitHashes.Add(arcHash);

                    if (splitCacheHashes == null || splitCacheHashes[i] != arcHash)
                    {
                        hashesMatch = false;
                    }
                }

                // Update editor cache hashes
                if (splitCacheHashes == null)
                {
                    Stage.EditorCache.ArcHashes.Add(
                        hashID, splitHashes);
                }
                else if (!hashesMatch)
                {
                    Stage.EditorCache.ArcHashes[
                        hashID] = splitHashes;
                }

                if (hashesMatch)
                {
                    LuaTerminal.LogWarning(string.Format(
                        "{0} \"{1}\", as it hasn't changed since it was last unpacked.",
                        "WARNING: Skipped", hashID));
                }
                else
                {
                    LuaTerminal.Log($"Extracting \"{hashID}\"...");
                }
            }

            // Extract the archive
            if (!hashesMatch)
            {
                if (Directory.Exists(outDir))
                    Directory.Delete(outDir, true);

                arc.Load(path);
                arc.Extract(outDir);

                LuaTerminal.Log((string.IsNullOrEmpty(hashID)) ?
                    "Successfully unpacked!" :
                    $"Successfully unpacked \"{hashID}\"!");
            }
        }

        public GensTerrainList LoadTerrainList(string path, string groupsDir, string resDir)
        {
            // Format path strings, return if file doesn't exist
            path = string.Format(path, Stage.CacheDir, Stage.ID);
            groupsDir = string.Format(groupsDir, Stage.CacheDir, Stage.ID);
            resDir = string.Format(resDir, Stage.CacheDir, Stage.ID);

            if (!File.Exists(path) || !Directory.Exists(groupsDir) ||
                !Directory.Exists(resDir))
                return null;

            // Terrain List
            UIChangeLoadStatus("Terrain List");

            string dir = Path.GetDirectoryName(path);
            var terrainList = new GensTerrainList();
            terrainList.Load(path);

            // Terrain Groups
            int groupCount = terrainList.GroupEntries.Length;
            UIShowProgress();

            for (int i = 0; i < groupCount; ++i)
            {
                // Update UI
                UIChangeProgress((int)((i / (float)groupCount) * 100));
                UIChangeLoadStatus($"Terrain Group {i + 1}/{groupCount}");

                // Get the path to the terrain group
                var groupEntry = terrainList.GroupEntries[i];
                string groupDir = Path.Combine(groupsDir,
                    groupEntry.FileName);

                string groupPath = Path.Combine(dir,
                    $"{groupEntry.FileName}{GensTerrainGroup.Extension}");

                // Ensure the group exists
                if (!File.Exists(groupPath))
                {
                    LuaTerminal.LogWarning(string.Format(
                        "WARNING: Terrain group {0} was skipped {1}",
                        groupEntry.FileName, "because it was not found!"));

                    continue;
                }

                // Load the group
                var group = new GensTerrainGroup();
                group.Load(groupPath);

                // Terrain Instances
                for (int i2 = 0; i2 < group.InstanceInfos.Length; ++i2)
                {
                    var infoEntry = group.InstanceInfos[i2];
                    for (int i3 = 0; i3 < infoEntry.FileNames.Length; ++i3)
                    {
                        string infoPath = Path.Combine(groupDir,
                            string.Format("{0}{1}",
                            infoEntry.FileNames[i3],
                            GensTerrainInstanceInfo.Extension));

                        if (!File.Exists(infoPath))
                        {
                            LuaTerminal.LogWarning(string.Format(
                                "WARNING: Terrain instance {0} was skipped {1}",
                                infoEntry.FileNames[i3], "because it was not found!"));

                            continue;
                        }

                        var info = new GensTerrainInstanceInfo();
                        info.Load(infoPath);

                        var instance = new VPObjectInstance(info.TransformMatrix);

                        // Terrain Model
                        if (Viewport.Terrain.ContainsKey(info.ModelFileName))
                        {
                            // Don't bother loading the model again if we've
                            // already loaded a model with the same name.
                            MainUIInvoke(new Action(() =>
                            {
                                Viewport.AddInstance(info.ModelFileName,
                                    instance, false);
                            }));
                            continue;
                        }

                        string modelPath = Path.Combine(groupDir,
                            string.Format("{0}{1}",
                            info.ModelFileName,
                            GensModel.TerrainExtension));

                        if (!File.Exists(modelPath))
                        {
                            LuaTerminal.LogWarning(string.Format(
                                "WARNING: Terrain model {0} was skipped {1}",
                                info.ModelFileName, "because it was not found!"));

                            continue;
                        }

                        var mdl = new GensModel();
                        mdl.Load(modelPath);
                        mdl.Name = info.ModelFileName;

                        // Materials
                        foreach (var mesh in mdl.Meshes)
                        {
                            LoadMaterial(Path.Combine(resDir,
                                $"{mesh.MaterialName}{GensMaterial.Extension}"),
                                mesh.MaterialName, resDir);
                        }

                        MainUIInvoke(() =>
                        {
                            Viewport.AddTerrainModel(mdl);
                            Viewport.AddInstance(mdl.Name, instance, false);
                        });
                    }
                }
            }

            UIHideProgress();
            return terrainList;
        }

        public Model LoadTerrain(string path, string resDir, bool loadMats = true)
        {
            // Format path strings, return if file doesn't exist
            path = string.Format(path, Stage.CacheDir, Stage.ID);
            resDir = string.Format(resDir, Stage.CacheDir, Stage.ID);

            if (!File.Exists(path) || !Directory.Exists(resDir))
                return null;

            // Figure out what type of terrain to use
            Model mdl;
            switch (game)
            {
                case Games.Forces:
                case Games.LW:
                case Games.Gens:
                case Games.SU:
                    mdl = new GensModel();
                    break;

                // TODO: Add Storybook Support
                case Games.Storybook:
                    throw new NotImplementedException(
                        "Could not load, Storybook terrain is not yet supported!");

                // TODO: Add Colors Support
                case Games.Colors:
                    throw new NotImplementedException(
                        "Could not load, Colors terrain is not yet supported!");

                // TODO: Add 06 Support
                case Games.S06:
                    throw new NotImplementedException(
                        "Could not load, '06 terrain is not yet supported!");

                // TODO: Add Heroes/Shadow Support
                case Games.Shadow:
                case Games.Heroes:
                    throw new NotImplementedException(
                        "Could not load, Heroes/Shadow terrain is not yet supported!");

                default:
                    throw new Exception(
                        "Could not load, game type has not been set!");
            }

            string shortName = Path.GetFileNameWithoutExtension(path);

            // Terrain Instance Info
            VPObjectInstance instance = null;

            if (mdl.GetType() == typeof(GensModel))
            {
                string dir = Path.GetDirectoryName(path);
                string instancePath = Path.Combine(dir,
                    string.Format("{0}{1}",
                    shortName, GensTerrainInstanceInfo.Extension));

                if (File.Exists(instancePath))
                {
                    var info = new GensTerrainInstanceInfo();
                    info.Load(instancePath);
                    instance = new VPObjectInstance(info.TransformMatrix);
                }
                else
                {
                    instance = new VPObjectInstance();
                }

                // Don't bother loading the model again if we've
                // already loaded a model with the same name.
                if (Viewport.Terrain.ContainsKey(shortName))
                {
                    MainUIInvoke(() =>
                    {
                        Viewport.AddInstance(shortName, instance, false);
                    });
                    return null;
                }
            }

            // Terrain
            if (Viewport.Terrain.ContainsKey(shortName))
            {
                LuaTerminal.LogWarning(string.Format(
                    "WARNING: Skipped model {0} as a model with that {1}",
                    shortName, "name has already been loaded!"));
                return null;
            }
            else
            {
                mdl.Load(path);
                mdl.Name = shortName;
            }

            // Materials
            if (loadMats)
            {
                foreach (var mesh in mdl.Meshes)
                {
                    LoadMaterial(Path.Combine(resDir,
                        $"{mesh.MaterialName}{GensMaterial.Extension}"),
                        mesh.MaterialName, resDir);
                }
            }

            MainUIInvoke(() =>
            {
                Viewport.AddTerrainModel(mdl);
                Viewport.AddInstance(mdl.Name, instance, false);
            });

            return mdl;
        }

        public Model LoadObjectModel(string path, string resDir,
            bool loadMats = true, Vector3? scale = null)
        {
            // Format path strings, return if file doesn't exist
            path = string.Format(path, Stage.CacheDir, Stage.ID);
            resDir = string.Format(resDir, Stage.CacheDir, Stage.ID);

            if (!File.Exists(path) || !Directory.Exists(resDir))
                return null;

            // Figure out what type of model to use
            Model mdl;
            switch (game)
            {
                case Games.Forces:
                case Games.LW:
                case Games.Gens:
                case Games.SU:
                    mdl = new GensModel();
                    break;

                // TODO: Add Storybook Support
                case Games.Storybook:
                    throw new NotImplementedException(
                        "Could not load, Storybook terrain is not yet supported!");

                // TODO: Add Colors Support
                case Games.Colors:
                    throw new NotImplementedException(
                        "Could not load, Colors terrain is not yet supported!");

                // TODO: Add 06 Support
                case Games.S06:
                    throw new NotImplementedException(
                        "Could not load, '06 terrain is not yet supported!");

                // TODO: Add Heroes/Shadow Support
                case Games.Shadow:
                case Games.Heroes:
                    throw new NotImplementedException(
                        "Could not load, Heroes/Shadow terrain is not yet supported!");

                default:
                    throw new Exception(
                        "Could not load, game type has not been set!");
            }

            string shortName = Path.GetFileName(path);

            // Model
            if (Viewport.Objects.ContainsKey(shortName))
                return null;

            mdl.Load(path);
            mdl.Name = shortName;

            // Scale
            if (scale.HasValue)
            {
                foreach (var mesh in mdl.Meshes)
                {
                    for (uint i = Mesh.VertPos; i < mesh.VertexData.Length;)
                    {
                        mesh.VertexData[i] *= scale.Value.X;
                        mesh.VertexData[i + 1] *= scale.Value.Y;
                        mesh.VertexData[i + 2] *= scale.Value.Z;
                        i += Mesh.StructureLength;
                    }
                }
            }

            // Materials
            if (loadMats)
            {
                foreach (var mesh in mdl.Meshes)
                {
                    // TODO: Make extension type-specific
                    LoadMaterial(Path.Combine(resDir,
                        $"{mesh.MaterialName}{GensMaterial.Extension}"),
                        mesh.MaterialName, resDir);
                }
            }

            MainUIInvoke(() =>
            {
                Viewport.AddObjectModel(shortName, mdl);
            });

            return mdl;
        }

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
                
                default:
                    throw new Exception(
                        "Could not load, game type has not been set!");
            }

            // Material
            mat.Load(path);
            Viewport.Materials.Add(name, mat);

            // Textures
            foreach (var tex in mat.Textures)
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

        public SetData LoadSetLayer(string path, bool loadModels = true,
            string[] altModelDirs = null)
        {
            // Format path strings, return if file doesn't exist
            path = string.Format(path, Stage.CacheDir, Stage.ID);
            if (!File.Exists(path))
                return null;

            // Figure out what type of set data to use
            SetData setData;
            switch (game)
            {
                case Games.Forces:
                    setData = new ForcesSetData();
                    break;
                
                case Games.LW:
                    setData = new LWSetData();
                    break;
                
                case Games.Gens:
                case Games.SU:
                    setData = new GensSetData();
                    break;
                
                // TODO: Add Storybook Support
                case Games.Storybook:
                    throw new NotImplementedException(
                        "Could not load, Storybook set data is not yet supported!");
                
                case Games.Colors:
                    setData = new ColorsSetData();
                    break;
                
                case Games.S06:
                    setData = new S06SetData();
                    break;
                
                // TODO: Add Shadow Support
                case Games.Shadow:
                    throw new NotImplementedException(
                        "Could not load, Shadow set data is not yet supported!");
                
                case Games.Heroes:
                    setData = new HeroesSetData();
                    break;
                
                default:
                    throw new Exception(
                        "Could not load, game type has not been set!");
            }

            // Load the sets
            // TODO: Handle the object templates cleaner here
            var gameType = Stage.GameType;
            setData.Load(path, gameType.ObjectTemplates);

            // Spawn Objects in World
            LoadSetLayerResources(gameType, setData, loadModels, altModelDirs);

            setData.Name = Path.GetFileNameWithoutExtension(path);
            Stage.Sets.Add(setData);

            // Refresh UI Scene View
            MainUIInvoke(() =>
            {
                Program.MainForm.RefreshSceneView();
            });

            return setData;
        }

        // Saving Callbacks
        public void Repack(string path, string inDir, string hashID,
            bool includeSubDirs = false, bool generateList = false,
            uint? splitCount = 0xA00000)
        {
            // Format path strings
            path = string.Format(path, Stage.DataDir, Stage.ID);
            inDir = string.Format(inDir, Stage.CacheDir, Stage.ID);

            if (!Directory.Exists(inDir))
                return;

            // Figure out what type of archive to use
            Archive arc = null;
            switch (game)
            {
                case Games.Forces:
                    arc = new ForcesArchive();
                    break;

                // TODO: Add LW Support
                case Games.LW:
                    throw new NotImplementedException(
                        "Could not unpack, LW archives are not yet supported!");

                case Games.Gens:
                case Games.SU:
                    arc = new GensArchive();
                    break;

                case Games.Storybook:
                    arc = new SBArchive();
                    break;

                // TODO: Add Colors Support
                case Games.Colors:
                    throw new NotImplementedException(
                        "Could not unpack, Colors archives are not yet supported!");

                // TODO: Add 06 Support
                case Games.S06:
                    throw new NotImplementedException(
                        "Could not unpack, '06 archives are not yet supported!");

                case Games.Shadow:
                case Games.Heroes:
                    arc = new ONEArchive();
                    break;

                default:
                    throw new Exception(
                        "Could not unpack, game type has not been set!");
            }

            // Generate the archive
            arc.AddDirectory(inDir, includeSubDirs);

            var type = arc.GetType();
            if (type == typeof(GensArchive))
            {
                var gensArc = (arc as GensArchive);
                gensArc.Save(path, generateList, splitCount);
            }
            else if (type == typeof(ForcesArchive))
            {
                var forcesArc = (arc as ForcesArchive);
                forcesArc.Save(path, splitCount);
            }

            // Hashes
            if (!string.IsNullOrEmpty(hashID) && Stage.EditorCache != null)
            {
                // Get splits list
                var arcSplits = new List<string>();
                try
                {
                    var arcFileList = arc.GetSplitArchivesList(path);
                    arcSplits.AddRange(arcFileList);
                }
                catch
                {
                    arcSplits.Add(path);
                }

                // Get new hashes
                for (int i = 0; i < arcSplits.Count; ++i)
                {
                    arcSplits[i] = Helpers.GetFileHash(arcSplits[i]);
                }

                // Update editor cache hashes
                if (Stage.EditorCache.ArcHashes.ContainsKey(hashID))
                {
                    Stage.EditorCache.ArcHashes[hashID] = arcSplits;
                }
                else
                {
                    Stage.EditorCache.ArcHashes.Add(hashID, arcSplits);
                }
            }

            LuaTerminal.Log((string.IsNullOrEmpty(hashID)) ?
                "Successfully repacked!" :
                $"Successfully repacked \"{hashID}\"!");
        }

        public void SaveSetLayer(string path, SetData layer)
        {
            // Format path strings
            path = string.Format(path, Stage.CacheDir, Stage.ID);

            // Save sets
            layer.Save(path, true);
        }

        public void SaveSetLayers(string dir, string prefix,
            string suffix, bool showProgress = false)
        {
            // Format path strings
            dir = string.Format(dir, Stage.CacheDir, Stage.ID);

            // Save sets
            if (showProgress)
                UIShowProgress();

            int layerCount = Stage.Sets.Count;
            for (int i = 0; i < layerCount; ++i)
            {
                if (showProgress)
                {
                    UIChangeProgress((int)(((float)i / layerCount) * 100));
                    UIChangeSaveStatus(string.Format(
                        "Set Data {0:D2}/{1:D2}", i + 1, layerCount));
                }

                var layer = Stage.Sets[i];
                layer.Save(Path.Combine(dir,
                    $"{prefix}{layer.Name}{suffix}"), true);
            }

            if (showProgress)
                UIHideProgress();
        }
    }
}