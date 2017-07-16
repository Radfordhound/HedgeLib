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
        //Variables/Constants
        public static List<GameEntry> Games = new List<GameEntry>();
        public const string FilePath = "GameList.xml", TemplatesDir = "Templates";
        public const float HighestSupportedVersion = 1.0f;

        //Methods
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
                    var dataTypeAttr = element.Attribute("dataType");
                    var unitMultiplierAttr = element.Attribute("unitMultiplier");

                    if (shortNameAttr == null || dataTypeAttr == null) continue;
                    string shortName = shortNameAttr.Value,
                           dataType = dataTypeAttr.Value;

                    // Game Entry
                    var templates = SetObjectType.LoadObjectTemplates(templatesDir, shortName);
                    var game = new GameEntry()
                    {
                        Name = (nameAttr == null) ? shortName : nameAttr.Value,
                        ShortName = shortName,
                        DataType = Types.GetDataType(dataType),
                        ObjectTemplates = templates,
                        UnitMultiplier = (unitMultiplierAttr == null) ? 1 :
                            Convert.ToSingle(unitMultiplierAttr.Value)
                    };

                    // Unpack Info
                    var unpackInfoElem = element.Element("UnpackInfo");
                    if (unpackInfoElem == null) continue;

                    foreach (var subElem in unpackInfoElem.Elements())
                    {
                        var pathAttr = subElem.Attribute("path");
                        var cachePathAttr = subElem.Attribute("cachePath");
                        var searchPatternAttr = subElem.Attribute("searchPattern");
                        if (pathAttr == null || cachePathAttr == null) continue;

                        var entry = new UnpackInfoEntry()
                        {
                            Type = subElem.Name.LocalName,
                            Path = pathAttr.Value,
                            CachePath = cachePathAttr.Value,
                            SearchPattern = searchPatternAttr?.Value
                        };

                        game.UnpackInfo.Add(entry);
                    }

                    // Load Info
                    var loadInfoElem = element.Element("LoadInfo");
                    if (loadInfoElem == null) continue;

                    foreach (var subElem in loadInfoElem.Elements())
                    {
                        var typeAttr = subElem.Attribute("type");
                        if (typeAttr == null) continue;

                        switch (subElem.Name.LocalName.ToLower())
                        {
                            case "directory":
                                {
                                    var filterAttr = subElem.Attribute("filter");
                                    if (filterAttr == null ||
                                        game.LoadInfo.Directories.ContainsKey(typeAttr.Value))
                                        continue;

                                    var dirEntry = new LoadInfoDirectory()
                                    {
                                        Directory = subElem.Value,
                                        Filter = filterAttr.Value
                                    };
                                    game.LoadInfo.Directories.Add(typeAttr.Value, dirEntry);
                                    break;
                                }

                            case "file":
                                if (game.LoadInfo.Files.ContainsKey(typeAttr.Value)) continue;
                                game.LoadInfo.Files.Add(typeAttr.Value, subElem.Value);
                                break;
                        }
                    }

                    Games.Add(game);

                    // TODO: Remove this line
                    Console.WriteLine("Loaded {0} templates for {1}.",
                        game.ObjectTemplates.Count, game.Name);
                }
            }
        }
    }

    public class GameEntry
    {
        //Variables/Constants
        public Dictionary<string, SetObjectType> ObjectTemplates =
            new Dictionary<string, SetObjectType>();

        public List<UnpackInfoEntry> UnpackInfo = new List<UnpackInfoEntry>();
        public LoadInfo LoadInfo = new LoadInfo();

        public Types.DataTypes DataType;
        public string Name, ShortName;
        public float UnitMultiplier = 1;
    }

    public class UnpackInfoEntry
    {
        //Variables/Constants
        public string Type;
        public string Path, CachePath, SearchPattern;
    }

    public class LoadInfo
    {
        //Variables/Constants
        public Dictionary<string, LoadInfoDirectory> Directories =
            new Dictionary<string, LoadInfoDirectory>();
        public Dictionary<string, string> Files = new Dictionary<string, string>();
    }

    public class LoadInfoDirectory
    {
        //Variables/Constants
        public string Directory, Filter;
    }
}