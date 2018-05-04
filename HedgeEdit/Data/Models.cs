using HedgeEdit.UI;
using HedgeLib.Models;
using HedgeLib.Terrain;
using System;
using System.Collections.Generic;
using System.IO;

namespace HedgeEdit
{
    public static partial class Data
    {
        // Variables/Constants
        public static Dictionary<string, Dictionary<string, VPModel>> TerrainGroups =
            new Dictionary<string, Dictionary<string, VPModel>>();

        public static Dictionary<string, VPModel> DefaultTerrainGroup =
            new Dictionary<string, VPModel>();

        public static VPModel DefaultCube;
        public static AssetDirectories ModelDirectories = new AssetDirectories();

        // Methods
        public static VPObjectInstance GetInstance(VPModel model, object obj)
        {
            foreach (var instance in model.Instances)
            {
                if (instance.CustomData == obj)
                    return instance;
            }

            return null;
        }

        public static Dictionary<string, VPModel> GetTerrainGroup(
            string name, bool createIfNotExists = false)
        {
            if (string.IsNullOrEmpty(name) || !TerrainGroups.ContainsKey(name))
            {
                if (createIfNotExists)
                {
                    var group = new Dictionary<string, VPModel>();
                    TerrainGroups.Add(name, group);
                    return group;
                }

                return DefaultTerrainGroup;
            }

            return TerrainGroups[name];
        }

        public static VPModel GetModel(string name, string resDir = null,
            bool isTerrain = false, bool loadMats = false, string group = null,
            bool nonEditable = true, GensTerrainInstanceInfo instInfo = null,
            bool spawnInstance = true)
        {
            if (string.IsNullOrEmpty(name))
                return DefaultCube;

            var g = GetTerrainGroup(group);
            if (!g.ContainsKey(name))
            {
                // Attempt to load the model
                var mdlExt = (isTerrain) ? Types.TerrainModelExtension : Types.ModelExtension;
                foreach (var dir in ResourceDirectories)
                {
                    if (Directory.Exists(dir.FullPath))
                    {
                        string path = Path.Combine(dir.FullPath, $"{name}{mdlExt}");
                        if (File.Exists(path))
                        {
                            var mdl = LoadModel(path, resDir, isTerrain, loadMats,
                                group, nonEditable, instInfo, spawnInstance);

                            if (mdl != null)
                                return mdl;

                            // TODO: Maybe remove this line so it keeps trying other dirs?
                            return DefaultCube;
                        }
                    }
                }

                // Return the default cube if that failed
                return DefaultCube;
            }
            else
            {
                if (spawnInstance && instInfo != null)
                {
                    Program.MainUIInvoke(() =>
                    {
                        g[name].Instances.Add(new VPObjectInstance(
                            instInfo.TransformMatrix, instInfo.FileName));
                    });
                }

                return g[name];
            }
        }

        public static GensTerrainList LoadTerrainList(string path,
            string groupsDir, string resDir, bool nonEditable = true)
        {
            // Terrain List
            GUI.ChangeLoadStatus("Terrain List");

            string dir = Path.GetDirectoryName(path);
            var terrainList = new GensTerrainList();
            terrainList.Load(path);

            // Terrain Groups
            int groupCount = terrainList.GroupEntries.Length;
            GUI.ShowProgress();

            for (int i = 0; i < groupCount; ++i)
            {
                // Update UI
                GUI.ChangeProgress((int)((i / (float)groupCount) * 100));
                GUI.ChangeLoadStatus($"Terrain Group {i + 1}/{groupCount}");

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
                        "WARNING: Terrain group \"{0}\" was skipped {1}",
                        groupEntry.FileName, "because it was not found!"));

                    continue;
                }

                // Load the group
                var group = new GensTerrainGroup();
                group.Load(groupPath);

                Dictionary<string, VPModel> terrainGroup = null;
                Program.MainUIInvoke(() =>
                {
                    terrainGroup = GetTerrainGroup(groupEntry.FileName, true);
                });

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

                        var instance = new VPObjectInstance(info.TransformMatrix,
                            info.FileName);

                        // Terrain Model
                        if (terrainGroup.ContainsKey(info.ModelFileName))
                        {
                            bool success = false;
                            Program.MainUIInvoke(new Action(() =>
                            {
                                success = AddInstance(info.ModelFileName,
                                    instance, terrainGroup);
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
                                "WARNING: Terrain model \"{0}\" was skipped {1}",
                                info.ModelFileName, "because it was not found!"));

                            continue;
                        }

                        var mdl = new GensModel();
                        mdl.Load(modelPath);
                        mdl.Name = info.ModelFileName;

                        // Materials
                        var matExt = Types.MaterialExtension;
                        foreach (var mesh in mdl.Meshes)
                        {
                            LoadMaterial(Path.Combine(resDir,
                                $"{mesh.MaterialName}{matExt}"),
                                mesh.MaterialName, nonEditable);
                        }

                        Program.MainUIInvoke(() =>
                        {
                            AddModelInstance(mdl, instance, terrainGroup);
                        });
                    }
                }
            }

            GUI.HideProgress();
            return terrainList;
        }

        public static VPModel LoadModel(string path, string resDir = null,
            bool isTerrain = false, bool loadMats = false, string group = null,
            bool nonEditable = true, GensTerrainInstanceInfo instInfo = null,
            bool spawnInstance = true)
        {
            // Figure out what type of model to use
            Model mdl;
            switch (Types.CurrentDataType)
            {
                case Types.DataTypes.Forces:
                case Types.DataTypes.LW:
                case Types.DataTypes.Gens:
                case Types.DataTypes.SU:
                    mdl = new GensModel();
                    break;

                // TODO: Add Storybook Support
                case Types.DataTypes.Storybook:
                    throw new NotImplementedException(
                        "Could not load, Storybook terrain is not yet supported!");

                // TODO: Add Colors Support
                case Types.DataTypes.Colors:
                    throw new NotImplementedException(
                        "Could not load, Colors terrain is not yet supported!");

                // TODO: Add 06 Support
                case Types.DataTypes.S06:
                    throw new NotImplementedException(
                        "Could not load, '06 terrain is not yet supported!");

                // TODO: Add Heroes/Shadow Support
                case Types.DataTypes.Shadow:
                case Types.DataTypes.Heroes:
                    throw new NotImplementedException(
                        "Could not load, Heroes/Shadow terrain is not yet supported!");

                // TODO: Add SA2 Support
                case Types.DataTypes.SA2:
                    throw new NotImplementedException(
                        "Could not load, SA2 terrain is not yet supported!");

                default:
                    throw new Exception(
                        "Could not load, game type has not been set!");
            }

            // Get some information about the model
            string dir = Path.GetDirectoryName(path);
            string shortName = Path.GetFileNameWithoutExtension(path);
            Dictionary<string, VPModel> dict = null;
            VPObjectInstance instance = null;

            // Load the model
            if (mdl.GetType() == typeof(GensModel))
            {
                if (isTerrain)
                {
                    // Terrain Instance Info
                    Program.MainUIInvoke(() =>
                    {
                        dict = GetTerrainGroup(group, true);
                    });

                    if (spawnInstance)
                    {
                        instance = (instInfo == null) ? new VPObjectInstance(shortName) :
                            new VPObjectInstance(instInfo.TransformMatrix, instInfo.FileName);
                    }

                    // Don't bother loading the model again if we've
                    // already loaded a model with the same name.
                    if (dict.ContainsKey(shortName))
                    {
                        if (spawnInstance)
                        {
                            Program.MainUIInvoke(() =>
                            {
                                AddTerrainInstance(shortName, instance, group);
                            });
                        }

                        return dict[shortName];
                    }
                }
                else
                {
                    dict = Objects;
                }
            }

            // Model
            if (dict == null) return null;
            if (dict.ContainsKey(shortName))
            {
                LuaTerminal.LogWarning(string.Format(
                    "WARNING: Skipped model \"{0}\" as a model with that {1}",
                    shortName, "name has already been loaded!"));
                return dict[shortName];
            }
            else
            {
                mdl.Load(path);
                mdl.Name = shortName;
            }

            // Materials
            if (loadMats)
            {
                bool useResDirs = string.IsNullOrEmpty(resDir);
                var matExt = Types.MaterialExtension;

                foreach (var mesh in mdl.Meshes)
                {
                    string pth = (useResDirs) ? null : Path.Combine(resDir,
                        $"{mesh.MaterialName}{matExt}");

                    if (!useResDirs && File.Exists(pth))
                    {
                        var mat = LoadMaterial(pth, mesh.MaterialName, nonEditable);
                        if (mat != null)
                            continue;
                    }

                    GetMaterial(mesh.MaterialName, nonEditable);
                }
            }

            VPModel vpMdl = null;
            Program.MainUIInvoke(() =>
            {
                if (isTerrain)
                {
                    vpMdl = (spawnInstance) ? AddModelInstance(mdl,
                        instance, dict) : AddModel(mdl, dict);
                }
                else
                {
                    vpMdl = AddObjectModel(mdl, shortName);
                }
            });

            return vpMdl;
        }

        public static AssetDirectory AddModelDirectoryFromPath(string path)
        {
            return AddModelDirectory(Path.GetDirectoryName(path));
        }

        public static AssetDirectory AddModelDirectory(string dir)
        {
            return ModelDirectories.AddDirectory(dir);
        }

        public static bool AddTerrainInstance(string modelName,
            VPObjectInstance instance, string group = null)
        {
            var terrainGroup = GetTerrainGroup(group, true);
            return AddInstance(modelName, instance, terrainGroup);
        }

        public static void AddTerrainInstance(Model mdl,
            VPObjectInstance instance, string group = null)
        {
            var trr = AddTerrainModel(mdl, group);
            trr.Instances.Add(instance);
        }

        public static VPModel AddModelInstance(Model mdl, VPObjectInstance instance,
            Dictionary<string, VPModel> dict)
        {
            var m = AddModel(mdl, dict);
            m.Instances.Add(instance);
            return m;
        }

        public static bool AddInstance(string modelName,
            VPObjectInstance instance, Dictionary<string, VPModel> dict)
        {
            // Add Instance
            if (string.IsNullOrEmpty(modelName) || !dict.ContainsKey(modelName))
                return false;

            dict[modelName].Instances.Add(instance);
            return true;
        }

        public static VPModel AddTerrainModel(Model mdl, string group = null)
        {
            var terrainGroup = GetTerrainGroup(group, true);
            return AddModel(mdl, terrainGroup);
        }

        public static VPModel AddModel(Model mdl,
            Dictionary<string, VPModel> dict)
        {
            // Add/Replace Model
            var m = new VPModel(mdl);
            if (!dict.ContainsKey(mdl.Name))
            {
                dict.Add(mdl.Name, m);
            }
            else
            {
                dict[mdl.Name] = m;
            }

            return m;
        }
    }
}