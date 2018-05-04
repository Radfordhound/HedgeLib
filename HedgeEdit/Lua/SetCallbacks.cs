using HedgeEdit.UI;
using HedgeLib;
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
            script.Globals["GetCurrentSetLayer"] = (Func<SetData>)GetCurrentSetLayer;
            script.Globals["ChangeCurrentSetLayer"] = (Action<SetData>)ChangeCurrentSetLayer;
            
            script.Globals["GenSetObjectParam"] = (Func<string,
                object, SetObjectParam>)GenSetObjectParam;

            script.Globals["AddCustomData"] = (Action<SetObject,
                string, string, object>)AddCustomData;

            script.Globals["LoadObjectModel"] = (Func<string,
                string, bool, Vector3?, VPModel>)LoadObjectModel;

            script.Globals["AddSetLayer"] = (Func<string, SetData>)AddSetLayer;
            script.Globals["LoadSetLayer"] = (Func<string, bool, SetData>)LoadSetLayer;
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

        // Lua Callbacks
        public SetData GetSetLayer(int index)
        {
            if (Data.SetLayers == null || index >= Data.SetLayers.Count)
                return null;

            return Data.SetLayers[index];
        }

        public int GetSetLayerCount()
        {
            if (Data.SetLayers == null)
                return -1;

            return Data.SetLayers.Count;
        }

        public SetData GetCurrentSetLayer()
        {
            return Data.CurrentSetLayer;
        }

        public void ChangeCurrentSetLayer(SetData layer)
        {
            Data.CurrentSetLayer = layer;
            Program.MainUIInvoke(() =>
            {
                Program.MainForm.UpdateTitle(Stage.ID);
            });
        }

        public SetObjectParam GenSetObjectParam(string type, object data)
        {
            var t = HedgeLib.Types.GetTypeFromString(type);
            return new SetObjectParam(t, Helpers.ChangeType(data, t));
        }

        public void AddCustomData(SetObject obj,
            string name, string type, object data)
        {
            if (obj.CustomData.ContainsKey(name))
                return;

            var objParam = GenSetObjectParam(type, data);
            obj.CustomData.Add(name, objParam);
        }

        public VPModel LoadObjectModel(string path, string resDir = null,
            bool loadMats = true, Vector3? scale = null)
        {
            // Format path strings, return if file doesn't exist
            path = FormatCacheDir(path);

            if (!string.IsNullOrEmpty(resDir))
                resDir = FormatCacheDir(resDir);

            if (!File.Exists(path))
                return null;

            return Data.LoadModel(path, resDir, false, loadMats, null,
                path.StartsWith(Program.ResourcesDirectory));
        }

        public SetData AddSetLayer(string name)
        {
            // Add Set Layer to list
            var setData = Types.SetDataType;
            setData.Name = name;
            Data.SetLayers.Add(setData);

            // Refresh UI Scene View
            GUI.RefreshSceneView();
            return setData;
        }

        public SetData LoadSetLayer(string path, bool loadModels = true)
        {
            // Format path strings, return if file doesn't exist
            path = FormatCacheDir(path);
            if (!File.Exists(path))
                return null;

            return Data.LoadSetLayer(path, loadModels);
        }

        public void SaveSetLayer(string path, SetData layer)
        {
            path = FormatCacheDir(path);
            layer.Save(path, true);
        }

        public void SaveSetLayers(string dir, string prefix,
            string suffix, bool showProgress = true)
        {
            // Format path strings
            dir = FormatCacheDir(dir);
            Directory.CreateDirectory(dir);

            // Save sets
            if (showProgress)
                GUI.ShowProgress();

            int layerCount = Data.SetLayers.Count;
            for (int i = 0; i < layerCount; ++i)
            {
                if (showProgress)
                {
                    GUI.ChangeProgress((int)(((float)i / layerCount) * 100));
                    GUI.ChangeSaveStatus(string.Format(
                        "Set Data {0:D2}/{1:D2}", i + 1, layerCount));
                }

                var layer = Data.SetLayers[i];
                layer.Save(Path.Combine(dir,
                    $"{prefix}{layer.Name}{suffix}"), true);
            }

            if (showProgress)
                GUI.HideProgress();
        }
    }
}