using HedgeEdit.UI;
using HedgeLib;
using HedgeLib.IO;
using HedgeLib.Materials;
using HedgeLib.Models;
using HedgeLib.Sets;
using HedgeLib.Terrain;
using HedgeLib.Textures;
using MoonSharp.Interpreter;
using System;
using System.Collections;
using System.Collections.Generic;

namespace HedgeEdit.Lua
{
    public partial class LuaScript
    {
        // Variables/Constants
        protected Script script;

        public const string GamesDir = "Games",
            PluginsDir = "Plugins", Extension = ".lua";

        // Constructors
        public LuaScript()
        {
            // TODO: Maybe set more CoreModules?
            script = new Script(CoreModules.Basic | CoreModules.String |
                CoreModules.TableIterators | CoreModules.LoadMethods);

            // General Callbacks
            script.Globals["Log"] = (Action<object>)LuaTerminal.Log;
            script.Globals["LogWarning"] = (Action<object>)LuaTerminal.LogWarning;
            script.Globals["LogError"] = (Action<object>)LuaTerminal.LogError;
            script.Globals["SetDataType"] = (Action<string>)SetDataType;

            // Other Callbacks
            InitIOCallbacks();
            InitArchiveCallbacks();
            InitMaterialCallbacks();
            InitTerrainCallbacks();
            InitSetCallbacks();
            InitUICallbacks();
        }

        // Methods
        public static void Initialize()
        {
            Script.DefaultOptions.DebugPrint = LuaTerminal.Log;
            UserData.RegisterType<IDictionary>();
            UserData.RegisterType<Vector3>();
            UserData.RegisterType<Quaternion>();

            UserData.RegisterType<FileBase>();
            UserData.RegisterType<SetObjectParam>();
            UserData.RegisterType<SetObjectTransform>();
            UserData.RegisterType<SetObject>();
            UserData.RegisterType<SetObjectTypeParamExtra>();
            UserData.RegisterType<SetObjectTypeParamEnum>();
            UserData.RegisterType<SetObjectTypeParam>();
            UserData.RegisterType<SetObjectType>();
            UserData.RegisterType<SetData>();

            UserData.RegisterType<VPModel>();
            UserData.RegisterType<GensTerrainList>();
            UserData.RegisterType<GensMaterial>();

            UserData.RegisterType<AssetDirectory>();
            UserData.RegisterType<AssetDirectories>();
            UserData.RegisterType<Asset<GensMaterial>>();
            UserData.RegisterType<KeyValuePair<string, Asset<GensMaterial>>>();
            UserData.RegisterType<AssetCollection<GensMaterial>>();
        }

        public static bool EvaluateCondition(string condition)
        {
            try
            {
                var s = new Script();
                string txt = $"return ({condition.Replace("!=", "~=")})";
                return s.DoString(txt).Boolean;
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

        public DynValue Call(string funcName, params object[] args)
        {
            if (script.Globals[funcName] != null)
                return script.Call(script.Globals[funcName], args);

            return null;
        }

        public object GetValue(string name)
        {
            return script.Globals[name];
        }

        public string FormatCacheDir(string path)
        {
            return string.Format(path, Stage.CacheDir, Stage.ID);
        }

        public string FormatDataDir(string path)
        {
            return string.Format(path, Stage.DataDir, Stage.ID);
        }

        // Lua Callbacks
        public void SetDataType(string dataType)
        {
            Types.CurrentDataType = Types.GetDataType(dataType);
        }
    }
}