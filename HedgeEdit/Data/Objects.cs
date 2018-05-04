using HedgeEdit.Lua;
using HedgeEdit.UI;
using HedgeLib;
using HedgeLib.Models;
using HedgeLib.Sets;
using System.Collections.Generic;
using System.IO;

namespace HedgeEdit
{
    public static partial class Data
    {
        // Variables/Constants
        public static Dictionary<string, VPModel> Objects =
            new Dictionary<string, VPModel>();

        public static List<SetData> SetLayers = new List<SetData>();
        public static SetData CurrentSetLayer = null;

        // Methods
        public static void SpawnObject(SetObject obj, VPModel mdl,
            Vector3 posOffset, float unitMultiplier = 1)
        {
            Program.MainUIInvoke(() =>
            {
                var instance = AddObjectInstance(mdl,
                    (obj.Transform.Position * unitMultiplier) + posOffset,
                    obj.Transform.Rotation, obj.Transform.Scale, obj);

                if (obj.Children == null) return;
                foreach (var child in obj.Children)
                {
                    if (child == null) continue;
                    AddTransform(mdl, unitMultiplier,
                        child, posOffset);
                }
            });
        }

        public static void SpawnObject(SetObject obj, string modelName,
            Vector3 posOffset, float unitMultiplier = 1)
        {
            Program.MainUIInvoke(() =>
            {
                var instance = AddObjectInstance(modelName,
                    (obj.Transform.Position * unitMultiplier) + posOffset,
                    obj.Transform.Rotation, obj.Transform.Scale, obj);

                if (obj.Children == null) return;
                foreach (var child in obj.Children)
                {
                    if (child == null) continue;
                    AddTransform(modelName, unitMultiplier,
                        child, posOffset);
                }
            });
        }

        public static VPModel GetObjectModel(string name, string resDir = null)
        {
            if (string.IsNullOrEmpty(name))
                return DefaultCube;

            if (!Objects.ContainsKey(name))
            {
                // Attempt to load the model
                var mdlExt = Types.ModelExtension;
                foreach (var dir in ModelDirectories)
                {
                    if (Directory.Exists(dir.FullPath))
                    {
                        string path = Path.Combine(dir.FullPath, $"{name}{mdlExt}");
                        if (File.Exists(path))
                        {
                            var mdl = LoadModel(path, resDir, false, true, null,
                                path.StartsWith(Program.ResourcesDirectory));

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
                return Objects[name];
            }
        }

        public static VPObjectInstance GetObjectInstance(string modelName, object obj)
        {
            if (!Objects.ContainsKey(modelName))
                return null;

            return GetInstance(Objects[modelName], obj);
        }

        public static VPObjectInstance GetObjectInstance(object obj)
        {
            foreach (var model in Objects)
            {
                var instance = GetInstance(model.Value, obj);
                if (instance != null)
                    return instance;
            }

            return GetInstance(DefaultCube, obj);
        }

        public static VPModel GetObjectModel(object obj)
        {
            foreach (var model in Objects)
            {
                var instance = GetInstance(model.Value, obj);
                if (instance != null)
                    return model.Value;
            }

            return DefaultCube;
        }

        public static void GetObject(object obj,
            out VPModel mdl, out VPObjectInstance inst)
        {
            foreach (var model in Objects)
            {
                var instance = GetInstance(model.Value, obj);
                if (instance != null)
                {
                    mdl = model.Value;
                    inst = instance;
                    return;
                }
            }

            mdl = DefaultCube;
            inst = GetInstance(DefaultCube, obj);
        }

        public static Vector3 GetObjOffsetPos(SetObjectType type)
        {
            var offsetPos = new Vector3(0, 0, 0);
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

            return offsetPos;
        }

        public static Vector3 GetObjScale(SetObjectType type, SetObject obj)
        {
            if (type == null)
                return new Vector3(1, 1, 1);

            var scaleExtra = type.GetExtra("scale");
            if (scaleExtra != null && !string.IsNullOrEmpty(scaleExtra.Value))
            {
                if (float.TryParse(scaleExtra.Value, out float s))
                {
                    return new Vector3(s, s, s);
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
                                return (Vector3)param.Data;
                            }
                            else if (param.DataType == typeof(float))
                            {
                                float f = (float)param.Data;
                                return new Vector3(f, f, f);
                            }
                        }
                    }
                }
            }

            return new Vector3(1, 1, 1);
        }

        public static string GetObjModelExtra(SetObject obj,
            SetObjectType type, string startName = null)
        {
            // Get the correct extra's model name
            string mdlName = startName;
            foreach (var extra in type.Extras)
            {
                if (extra.Type.ToLower() != "model")
                    continue;

                if (string.IsNullOrEmpty(extra.Condition) ||
                    LuaScript.EvaluateObjectCondition(obj, type, extra.Condition))
                {
                    mdlName = extra.Value;
                    break;
                }
            }

            if (string.IsNullOrEmpty(mdlName))
                return mdlName;

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

            return mdlName;
        }

        public static VPModel GetObjectModelInfo(SetObject obj,
            SetObjectType type, out Vector3 offsetPos, bool loadModels = true)
        {
            // Get offset position/scale (if any)
            offsetPos = GetObjOffsetPos(type);
            obj.Transform.Scale = GetObjScale(type, obj);

            // Load Object Model
            string mdlName = null;
            if (loadModels && type != null)
            {
                mdlName = GetObjModelExtra(obj, type);
                mdlName = Path.GetFileNameWithoutExtension(mdlName);
            }

            return GetObjectModel(mdlName);
        }

        public static void LoadSetLayerResources(GameEntry gameType,
            SetData setData, bool loadModels = true)
        {
            foreach (var obj in setData.Objects)
            {
                LoadObjectResources(gameType, obj, loadModels);
            }
        }

        public static void LoadObjectResources(GameEntry gameType,
            SetObject obj, bool loadModels = true)
        {
            // Get Object Template (if any)
            SetObjectType type = null;
            if (gameType.ObjectTemplates.ContainsKey(obj.ObjectType))
                type = gameType.ObjectTemplates[obj.ObjectType];

            // Update Object Info
            var mdl = GetObjectModelInfo(obj, type,
                out Vector3 offsetPos, loadModels);

            // Spawn Object in World
            SpawnObject(obj, mdl, offsetPos,
                gameType.UnitMultiplier);
        }

        public static SetData LoadSetLayer(string path, bool loadModels = true)
        {
            // Load the sets
            // TODO: Handle the object templates cleaner here
            var setData = Types.SetDataType;
            var gameType = Stage.GameType;
            setData.Load(path, gameType.ObjectTemplates);

            // Spawn Objects in World
            LoadSetLayerResources(gameType, setData, loadModels);

            // Add Set Layer to list
            setData.Name = Path.GetFileNameWithoutExtension(path);
            SetLayers.Add(setData);

            // Refresh UI Scene View
            GUI.RefreshSceneView();
            return setData;
        }

        public static void ImportSetLayerXML(string path)
        {
            // Import Set Layer from XML file
            var setData = Types.SetDataType;
            setData.Name = Path.GetFileNameWithoutExtension(path);
            setData.ImportXML(path);
            LoadSetLayerResources(Stage.GameType, setData);

            // If a layer of the same name already exists, replace it.
            int setIndex = -1;
            for (int i = 0; i < SetLayers.Count; ++i)
            {
                if (SetLayers[i].Name == setData.Name)
                {
                    setIndex = i;
                    break;
                }
            }

            if (setIndex == -1)
            {
                SetLayers.Add(setData);
            }
            else
            {
                var layer = SetLayers[setIndex];
                Viewport.SelectedInstances.Clear();

                foreach (var obj in layer.Objects)
                {
                    GetObject(obj, out VPModel mdl, out VPObjectInstance instance);
                    if (mdl == null || instance == null) continue;
                    mdl.Instances.Remove(instance);
                }

                if (CurrentSetLayer == layer)
                    CurrentSetLayer = setData;
                
                SetLayers[setIndex] = setData;
            }

            // Refresh the UI
            GUI.RefreshSceneView();
            GUI.RefreshGUI();
        }

        public static VPModel AddObjectModel(Model mdl)
        {
            return AddObjectModel(mdl, mdl.Name);
        }

        public static VPModel AddObjectModel(Model mdl, string name)
        {
            // Add/Replace Model
            var obj = new VPModel(mdl, true);
            if (!Objects.ContainsKey(name))
            {
                Objects.Add(name, obj);
            }
            else
            {
                Objects[name] = obj;
            }

            return obj;
        }

        public static VPObjectInstance AddObjectInstance(VPModel obj,
            VPObjectInstance instance)
        {
            obj.Instances.Add(instance);
            return instance;
        }

        public static VPObjectInstance AddObjectInstance(string modelName,
            VPObjectInstance instance)
        {
            var obj = GetObjectModel(modelName);
            obj.Instances.Add(instance);
            return instance;
        }

        public static VPObjectInstance AddObjectInstance(VPModel obj,
            object customData = null)
        {
            return AddObjectInstance(obj,
                new VPObjectInstance(customData));
        }

        public static VPObjectInstance AddObjectInstance(string modelName,
            object customData = null)
        {
            return AddObjectInstance(modelName,
                new VPObjectInstance(customData));
        }

        public static VPObjectInstance AddObjectInstance(VPModel obj,
            OpenTK.Vector3 pos, OpenTK.Quaternion rot, OpenTK.Vector3 scale,
            object customData = null)
        {
            return AddObjectInstance(obj, new VPObjectInstance(
                pos, rot, scale, customData));
        }

        public static VPObjectInstance AddObjectInstance(string modelName,
            OpenTK.Vector3 pos, OpenTK.Quaternion rot, OpenTK.Vector3 scale,
            object customData = null)
        {
            return AddObjectInstance(modelName, new VPObjectInstance(
                pos, rot, scale, customData));
        }

        public static VPObjectInstance AddObjectInstance(VPModel obj,
            Vector3 pos, Quaternion rot,
            Vector3 scale, object customData = null)
        {
            return AddObjectInstance(obj, new VPObjectInstance(
                Types.ToOpenTK(pos), Types.ToOpenTK(rot),
                Types.ToOpenTK(scale), customData));
        }

        public static VPObjectInstance AddObjectInstance(string modelName,
            Vector3 pos, Quaternion rot,
            Vector3 scale, object customData = null)
        {
            return AddObjectInstance(modelName, new VPObjectInstance(
                Types.ToOpenTK(pos), Types.ToOpenTK(rot),
                Types.ToOpenTK(scale), customData));
        }

        public static VPObjectInstance AddTransform(VPModel obj,
            float unitMultiplier, SetObjectTransform child,
            Vector3 posOffset)
        {
            return AddObjectInstance(obj,
                (child.Position * unitMultiplier) + posOffset,
                child.Rotation, child.Scale, child);
        }

        public static VPObjectInstance AddTransform(string modelName,
            float unitMultiplier, SetObjectTransform child,
            Vector3 posOffset)
        {
            return AddObjectInstance(modelName,
                (child.Position * unitMultiplier) + posOffset,
                child.Rotation, child.Scale, child);
        }

        public static void RemoveObjectInstance(VPObjectInstance instance)
        {
            foreach (var model in Objects)
            {
                if (model.Value.Instances.Remove(instance))
                    return;
            }

            DefaultCube.Instances.Remove(instance);
        }
    }
}