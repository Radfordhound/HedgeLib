using HedgeEdit.UI;
using HedgeLib;
using HedgeLib.Materials;
using HedgeLib.Models;
using HedgeLib.Sets;
using HedgeLib.Terrain;
using HedgeLib.Textures;
using MoonSharp.Interpreter;
using System;
using System.Collections;

namespace HedgeEdit.Lua
{
    public partial class LuaScript
    {
        // Variables/Constants
        public Games Game => game;
        protected Script script;
        protected Games game = Games.None;

        public enum Games
        {
            None,
            SA2,
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
            script = new Script(CoreModules.Basic | CoreModules.String |
                CoreModules.TableIterators);

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

        public void Call(string funcName, params object[] args)
        {
            if (script.Globals[funcName] != null)
                script.Call(script.Globals[funcName], args);
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

                case "sa2":
                    return Games.SA2;

                default:
                    throw new NotImplementedException(
                        $"Unknown game type \"{dataType}\"!");
            }
        }

        // Lua Callbacks
        public void SetDataType(string dataType)
        {
            game = GetGame(dataType);
        }
    }
}