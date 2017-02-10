using HedgeLib.Sets;
using System.Collections.Generic;
using System.IO;
using System.Reflection; //Let's take some time to reflect...
using UnityEngine;

public static class Globals
{
    //Variables/Constants
    public static List<SetData> Sets = new List<SetData>();
    public static string StartupPath = Path.GetFullPath(HedgeLib.Helpers.CombinePaths(
        Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), "..", "..",
        (Application.isEditor) ? DebugStartupDir : ""));
    public static string CurrentGame;

    public const string DebugStartupDir = "DebugStartup";
    public const string TemplatesDir = "Templates";
}