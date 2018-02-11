using HedgeEdit.UI;
using HedgeLib.Materials;
using HedgeLib.Models;
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
                string, bool, Model>)LoadTerrain;
        }

        // Lua Callbacks
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

                        var instance = new VPObjectInstance(info.TransformMatrix,
                            info.FileName);

                        // Terrain Model
                        if (Viewport.TerrainGroups.ContainsKey(info.ModelFileName))
                        {
                            // Don't bother loading the model again if we've
                            // already loaded a model with the same name.
                            MainUIInvoke(new Action(() =>
                            {
                                Viewport.AddTerrainInstance(info.ModelFileName,
                                    instance, groupEntry.FileName);
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
                            Viewport.AddTerrainInstance(mdl,
                                instance, groupEntry.FileName);
                        });
                    }
                }
            }

            UIHideProgress();
            return terrainList;
        }

        public Model LoadTerrain(string path, string resDir,
            string group = null, bool loadMats = true)
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
                    instance = new VPObjectInstance(info.TransformMatrix, shortName);
                }
                else
                {
                    instance = new VPObjectInstance(shortName);
                }

                // Don't bother loading the model again if we've
                // already loaded a model with the same name.
                if (Viewport.TerrainGroups.ContainsKey(shortName))
                {
                    MainUIInvoke(() =>
                    {
                        Viewport.AddTerrainInstance(shortName, instance, group);
                    });
                    return null;
                }
            }

            // Terrain
            if (Viewport.TerrainGroups.ContainsKey(shortName))
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
                Viewport.AddTerrainInstance(mdl, instance, group);
            });

            return mdl;
        }
    }
}