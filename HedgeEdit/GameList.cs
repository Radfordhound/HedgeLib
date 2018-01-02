using HedgeLib;
using HedgeLib.Sets;
using System;
using System.Collections.Generic;
using System.IO;
using System.Xml.Linq;

namespace HedgeEdit
{
    public static class GameList
    {
        // Variables/Constants
        public static List<GameEntry> Games = new List<GameEntry>();
        public const string FilePath = "GameList.xml", TemplatesDir = "Templates";
        public const float HighestSupportedVersion = 1.1f;

        // Methods
        public static void Load(string startupPath)
        {
            string filePath = Helpers.CombinePaths(startupPath, FilePath);
            string templatesDir = Helpers.CombinePaths(startupPath, TemplatesDir);

            using (var fileStream = File.OpenRead(filePath))
            {
                var xml = XDocument.Load(fileStream);
                var versionAttr = xml.Root.Attribute("version");
                if (versionAttr == null)
                    return;

                if (float.TryParse(versionAttr.Value, out float v) && v > HighestSupportedVersion)
                {
                    throw new FileLoadException(string.Format(
                        "Could not load GameList. GameLists of version ({0}) are unsupported.",
                        versionAttr.Value), filePath);
                }

                foreach (var element in xml.Root.Elements("Game"))
                {
                    // Game Name
                    var shortNameAttr = element.Attribute("shortName");
                    var nameAttr = element.Attribute("name");
                    var unitMultiplierAttr = element.Attribute("unitMultiplier");

                    if (shortNameAttr == null) continue;
                    string shortName = shortNameAttr.Value;

                    // Game Entry
                    var templates = SetObjectType.LoadObjectTemplates(templatesDir, shortName);
                    var game = new GameEntry()
                    {
                        Name = (nameAttr == null) ? shortName : nameAttr.Value,
                        ShortName = shortName,
                        ObjectTemplates = templates,
                        UnitMultiplier = (unitMultiplierAttr == null) ? 1 :
                            Convert.ToSingle(unitMultiplierAttr.Value)
                    };

                    Games.Add(game);

                    // TODO: Remove this line
                    Console.WriteLine("Loaded {0} templates for {1}.",
                        game.ObjectTemplates.Count, game.Name);
                }
            }
        }
    }
}