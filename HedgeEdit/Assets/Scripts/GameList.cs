using HedgeLib.Sets;
using System.Collections.Generic;
using System.IO;
using System.Xml.Linq;

public static class GameList
{
    //Variables/Constants
    public static Dictionary<string, GameEntry> Games = new Dictionary<string, GameEntry>();
    public const string FilePath = "GameList.xml";
    public const float HighestSupportedVersion = 1.0f;

    //Methods
    public static void Load()
    {
        string filePath = HedgeLib.Helpers.CombinePaths(Globals.StartupPath, FilePath);
        using (var fileStream = File.OpenRead(filePath))
        {
            var xml = XDocument.Load(fileStream);
            var versionAttr = xml.Root.Attribute("version");
            if (versionAttr != null)
            {
                float v;
                if (float.TryParse(versionAttr.Value, out v) && v > HighestSupportedVersion)
                {
                    throw new FileLoadException("Could not load GameList. The file's version (" +
                        versionAttr.Value + ") is higher than the highest supported version (" +
                        HighestSupportedVersion + ")!", filePath);
                }
            }

            foreach (var element in xml.Root.Elements("Game"))
            {
                //Game Name
                var shortNameAttr = element.Attribute("shortName");
                var nameAttr = element.Attribute("name");
                var dataTypeAttr = element.Attribute("dataType");

                if (shortNameAttr == null || dataTypeAttr == null) continue;
                string shortName = shortNameAttr.Value,
                       dataType = dataTypeAttr.Value;

                //Game Entry
                var game = new GameEntry()
                {
                    Name = (nameAttr == null) ? shortName : nameAttr.Value,
                    DataType = dataType,
                    GameDataType = DataTypes.GetDataType(dataType),
                    ObjectTemplates = LoadObjectTemplates(shortName)
                };

                //Unpack Info
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
                        SearchPattern = (searchPatternAttr != null) ?
                            searchPatternAttr.Value : null
                    };

                    game.UnpackInfo.Add(entry);
                }

                //Load Info
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

                Games.Add(shortName, game);

                UnityEngine.Debug.Log("Loaded " + game.ObjectTemplates.Count +
                    " templates for " + game.Name + "."); //TODO: REMOVE THIS
            }

            fileStream.Close();
        }
    }

    private static Dictionary<string, SetObjectType> LoadObjectTemplates(string shortName)
    {
        var dirPath = HedgeLib.Helpers.CombinePaths(Globals.StartupPath,
            Globals.TemplatesDir, shortName);

        if (!Directory.Exists(dirPath)) return null;
        var objectTemplates = new Dictionary<string, SetObjectType>();

        foreach (var dir in Directory.GetDirectories(dirPath))
        {
            //TODO: Categories.
            foreach (var file in Directory.GetFiles(dir, "*" + SetObjectType.Extension))
            {
                var fileInfo = new FileInfo(file);
                var template = new SetObjectType();
                string objTypeName = fileInfo.Name.Substring(0,
                    fileInfo.Name.Length - SetObjectType.Extension.Length);

                template.Load(file);
                objectTemplates.Add(objTypeName, template);
            }
        }

        return objectTemplates;
    }
}

public class GameEntry
{
    //Variables/Constants
    public Dictionary<string, SetObjectType> ObjectTemplates =
        new Dictionary<string, SetObjectType>();
    public List<UnpackInfoEntry> UnpackInfo = new List<UnpackInfoEntry>();
    public LoadInfo LoadInfo = new LoadInfo();
    public IGameDataType GameDataType;
    public string Name, DataType;
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