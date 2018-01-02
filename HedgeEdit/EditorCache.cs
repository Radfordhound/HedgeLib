using HedgeLib.IO;
using System.Collections.Generic;
using System.IO;
using System.Xml.Linq;

namespace HedgeEdit
{
    public class EditorCache : FileBase
    {
        // Variables/Constants
        public Dictionary<string, List<string>> ArcHashes =
            new Dictionary<string, List<string>>();

        public string GameType;
        public const string FileName = "EditorCache.xml";
        public const float Version = 1.0f;

        // Methods
        public override void Load(Stream fileStream)
        {
            var xml = XDocument.Load(fileStream);

            // Version Check
            var versionAttr = xml.Root.Attribute("version");
            if (versionAttr == null || !float.TryParse(versionAttr.Value,
                out float v) || v > Version)
                return;

            // Game Type
            var gameTypeElem = xml.Root.Element("GameType");
            if (gameTypeElem != null)
                GameType = gameTypeElem.Value;

            // Hashes
            var arcHashesElem = xml.Root.Element("ArcHashes");
            if (arcHashesElem != null)
            {
                foreach (var archiveElem in arcHashesElem.Elements("Archive"))
                {
                    var idAttr = archiveElem.Attribute("id");
                    if (idAttr == null || string.IsNullOrEmpty(idAttr.Value))
                        continue;

                    var arcHashes = new List<string>();
                    foreach (var arcHashElem in archiveElem.Elements("ArcHash"))
                    {
                        arcHashes.Add(arcHashElem.Value);
                    }

                    ArcHashes.Add(idAttr.Value, arcHashes);
                }
            }
        }

        public override void Save(Stream fileStream)
        {
            var versionAttr = new XAttribute("version", Version);
            var rootElem = new XElement("EditorCache", versionAttr);
            var gameTypeElem = new XElement("GameType", GameType);
            var arcHashesElem = new XElement("ArcHashes");

            foreach (var arc in ArcHashes)
            {
                var idAttr = new XAttribute("id", arc.Key);
                var arcElem = new XElement("Archive", idAttr);
                var arcHashes = arc.Value;

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
            xml.Save(fileStream);
        }
    }
}