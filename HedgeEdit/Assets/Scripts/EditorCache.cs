using System.Collections.Generic;
using System.IO;
using System.Xml.Linq;

public class EditorCache
{
    //Variables/Constants
    public List<List<string>> ArcHashes = new List<List<string>>();
    public string GameType;

    public const string FileName = "EditorCache.xml";

    //Methods
    public void Load(string filePath)
    {
        using (var fileStream = File.OpenRead(filePath))
        {
            var xml = XDocument.Load(fileStream);

            //Game Type
            var gameTypeElem = xml.Root.Element("GameType");
            if (gameTypeElem != null)
                GameType = gameTypeElem.Value;

            //Hashes
            var arcHashesElem = xml.Root.Element("ArcHashes");
            if (arcHashesElem != null)
            {
                foreach (var archiveElem in arcHashesElem.Elements("Archive"))
                {
                    var arcHashes = new List<string>();
                    foreach (var arcHashElem in archiveElem.Elements("ArcHash"))
                    {
                        arcHashes.Add(arcHashElem.Value);
                    }
                    ArcHashes.Add(arcHashes);
                }
            }

            fileStream.Close();
        }
    }

    public void Save(string filePath)
    {
        var rootElem = new XElement("EditorCache");
        var gameTypeElem = new XElement("GameType", GameType);
        var arcHashesElem = new XElement("ArcHashes");

        foreach (var arcHashes in ArcHashes)
        {
            var arcElem = new XElement("Archive");
            foreach (var arcHash in arcHashes)
            {
                var arcHashElem = new XElement("ArcHash", arcHash);
                arcElem.Add(arcHashElem);
            }
            arcHashesElem.Add(arcElem);
        }

        rootElem.Add(gameTypeElem);
        rootElem.Add(arcHashesElem);

        var xml = new XDocument(rootElem);
        xml.Save(filePath);
    }
}