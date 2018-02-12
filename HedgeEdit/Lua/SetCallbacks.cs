using HedgeEdit.UI;
using HedgeLib;
using HedgeLib.Materials;
using HedgeLib.Models;
using HedgeLib.Sets;
using MoonSharp.Interpreter;
using System;
using System.IO;

namespace HedgeEdit.Lua
{
    public partial class LuaScript
    {
        // Methods
        protected void InitSetCallbacks()
        {
            script.Globals["GetSetLayer"] = (Func<int, SetData>)GetSetLayer;
            script.Globals["GetSetLayerCount"] = (Func<int>)GetSetLayerCount;
            script.Globals["ChangeCurrentSetLayer"] = (Action<SetData>)ChangeCurrentSetLayer;
            script.Globals["GetCurrentSetLayer"] = (Func<SetData>)GetCurrentSetLayer;
            script.Globals["GenSetObjectParam"] = (Func<string,
                object, SetObjectParam>)GenSetObjectParam;

            script.Globals["LoadObjectModel"] = (Func<string,
                string, bool, Vector3?, Model>)LoadObjectModel;

            script.Globals["LoadSetLayer"] = (Func<string, bool,
                string[], SetData>)LoadSetLayer;

            script.Globals["SaveSetLayer"] = (Action<string, SetData>)SaveSetLayer;
            script.Globals["SaveSetLayers"] = (Action<string, string, string, bool>)SaveSetLayers;
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
                return s.DoString(txt).Boolean;
            }
            catch (Exception ex)
            {
                LuaTerminal.LogError($"ERROR: {ex.Message}");
                return false;
            }
        }

        public void ChangeCurrentSetLayer(SetData layer)
        {
            Stage.CurrentSetLayer = layer;
            // TODO: Show current layer on UI
        }

        public SetData GetCurrentSetLayer()
        {
            return Stage.CurrentSetLayer;
        }
        
        public SetObjectParam GenSetObjectParam(string type, object data)
        {
            var t = HedgeLib.Types.GetTypeFromString(type);
            return new SetObjectParam(t, Helpers.ChangeType(data, t));
        }

        public void LoadSetLayerResources(GameEntry gameType, SetData setData,
            bool loadModels = true, string[] altModelDirs = null)
        {
            string resDir = gameType.ResourcesDir;
            bool resDirExists = Directory.Exists(resDir);

            foreach (var obj in setData.Objects)
            {
                LoadObjectResources(gameType, obj, resDir,
                    resDirExists, loadModels, altModelDirs);
            }
        }

        public void LoadSetObjectResources(GameEntry gameType, SetObject obj,
            bool loadModels = true, string[] altModelDirs = null)
        {
            string resDir = gameType.ResourcesDir;
            bool resDirExists = Directory.Exists(resDir);

            LoadObjectResources(gameType, obj, resDir,
                resDirExists, loadModels, altModelDirs);
        }

        protected void LoadObjectResources(GameEntry gameType, SetObject obj,
            string resDir, bool resDirExists, bool loadModels, string[] altModelDirs)
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

        private static void SpawnObject(SetObject obj, string modelName,
            float unitMultiplier, Vector3 posOffset)
        {
            MainUIInvoke(() =>
            {
                var instance = Viewport.AddObjectInstance(modelName,
                    (obj.Transform.Position * unitMultiplier) + posOffset,
                    obj.Transform.Rotation, obj.Transform.Scale,
                    obj);

                if (obj.Children == null) return;
                foreach (var child in obj.Children)
                {
                    if (child == null) continue;

                    Viewport.AddObjectInstance(modelName,
                        (child.Position * unitMultiplier) + posOffset,
                        child.Rotation, child.Scale,
                        child);
                }
            });
        }

        // Lua Callbacks
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
                        "Could not load, Storybook models are not yet supported!");

                // TODO: Add Colors Support
                case Games.Colors:
                    throw new NotImplementedException(
                        "Could not load, Colors models are not yet supported!");

                // TODO: Add 06 Support
                case Games.S06:
                    throw new NotImplementedException(
                        "Could not load, '06 models are not yet supported!");

                // TODO: Add Heroes/Shadow Support
                case Games.Shadow:
                case Games.Heroes:
                    throw new NotImplementedException(
                        "Could not load, Heroes/Shadow models are not yet supported!");
                
                // TODO: Add SA2 Support
                case Games.SA2:
                    throw new NotImplementedException(
                        "Could not load, SA2 models are not yet supported!");

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

                // TODO: Add SA2 Support
                case Games.SA2:
                    throw new NotImplementedException(
                        "Could not load, SA2 set data is not yet supported!");
                //setData = new SA2SetData();
                //break;

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