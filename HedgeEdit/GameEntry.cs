using HedgeLib.Sets;
using System.Collections.Generic;
using System.IO;

namespace HedgeEdit
{
    public class GameEntry
    {
        // Variables/Constants
        public Dictionary<string, SetObjectType> ObjectTemplates =
            new Dictionary<string, SetObjectType>();

        public string ResourcesDir
        {
            get => Path.Combine(Program.StartupPath,
                Program.ResourcesPath, ShortName);
        }

        public string Name, ShortName;
        public float UnitMultiplier = 1;
    }
}