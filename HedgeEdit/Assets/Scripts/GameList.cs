using HedgeLib.Sets;
using System.Collections.Generic;
using System.IO;
using System.Xml.Linq;

public static class GameList
{
    //Variables/Constants
    public static Dictionary<string, GameEntry> Games = new Dictionary<string, GameEntry>();
    public const string FilePath = "GameList.xml";

    //Methods
    public static void Load()
    {
        string filePath = HedgeLib.Helpers.CombinePaths(Globals.StartupPath, FilePath);
        using (var fileStream = File.OpenRead(filePath))
        {
            var xml = XDocument.Load(fileStream);
            foreach (var element in xml.Root.Elements("Game"))
            {
                var shortNameAttr = element.Attribute("shortName");
                var nameAttr = element.Attribute("name");

                if (shortNameAttr == null) continue;
                string shortName = shortNameAttr.Value;

                var game = new GameEntry()
                {
                    Name = (nameAttr == null) ?  shortName : nameAttr.Value,
                    ObjectTemplates = LoadObjectTemplates(shortName)
                };

                if (string.IsNullOrEmpty(Globals.CurrentGame))
                    Globals.CurrentGame = shortName;
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
    public string Name;
}