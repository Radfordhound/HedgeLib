using System.IO;
using HedgeLib.IO;
using System.Xml.Linq;
using HedgeLib.Headers;
using System.Collections.Generic;

namespace HedgeLib.Misc
{
    public class S06TypeEntry
    {
        public string TypeName;
        public List<S06FileEntry> Files = new List<S06FileEntry>();
    }

    public class S06FileEntry
    {
        public string FriendlyName;
        public string FilePath;
    }

    public class S06Package : FileBase
    {
        public List<S06TypeEntry> Types = new List<S06TypeEntry>();

        public override void Load(Stream fileStream)
        {
            // Header
            var reader = new BINAReader(fileStream);
            reader.ReadHeader();

            //File related header stuff
            uint fileCount = reader.ReadUInt32();
            uint fileEntriesPos = reader.ReadUInt32();

            //Type related header stuff
            uint typeCount = reader.ReadUInt32();
            uint typeEntriesPos = reader.ReadUInt32();

            //Read Types
            reader.JumpTo(typeEntriesPos, false);
            for (uint i = 0; i < typeCount; ++i)
            {
                S06TypeEntry type = new S06TypeEntry();
                uint namePos = reader.ReadUInt32();
                uint typeFileCount = reader.ReadUInt32();
                uint filesPos = reader.ReadUInt32();

                long pos = reader.BaseStream.Position;

                reader.JumpTo(namePos, false);
                type.TypeName = reader.ReadNullTerminatedString();

                reader.JumpTo(filesPos, false);

                //Read Objects
                for (uint f = 0; f < typeFileCount; ++f)
                {
                    S06FileEntry file = new S06FileEntry();
                    uint friendlyNamePos = reader.ReadUInt32();
                    uint filePathPos = reader.ReadUInt32();

                    long curPos = reader.BaseStream.Position;

                    reader.JumpTo(friendlyNamePos, false);
                    file.FriendlyName = reader.ReadNullTerminatedString();

                    reader.JumpTo(filePathPos, false);
                    file.FilePath = reader.ReadNullTerminatedString();

                    reader.JumpTo(curPos);
                    type.Files.Add(file);
                }
                Types.Add(type);

                reader.JumpTo(pos, true);
            }
        }

        public override void Save(Stream fileStream)
        {
            // Header
            var header = new BINAv1Header();
            var writer = new BINAWriter(fileStream, header);

            uint filesCount = 0;

            for (int i = 0; i < Types.Count; i++)
            {
                for (int c = 0; c < Types[i].Files.Count; c++)
                {
                    filesCount++;
                }
            }

            writer.Write(filesCount);
            writer.AddOffset("fileEntriesPos");
            writer.Write(Types.Count);
            writer.AddOffset("typeEntriesPos");

            writer.FillInOffset("typeEntriesPos", false);
            for (int i = 0; i < Types.Count; i++)
            {
                writer.AddString($"typeName{i}", Types[i].TypeName);
                writer.Write(Types[i].Files.Count);
                writer.AddOffset($"typeFilesOffset{i}");
            }

            writer.FillInOffset("fileEntriesPos", false);
            int objectNum = 0;
            for (int i = 0; i < Types.Count; i++)
            {
                writer.FillInOffset($"typeFilesOffset{i}", false);
                for (int f = 0; f < Types[i].Files.Count; f++)
                {
                    writer.AddString($"friendlyName{objectNum}", Types[i].Files[f].FriendlyName);
                    writer.AddString($"filePath{objectNum}", Types[i].Files[f].FilePath);
                    objectNum++;
                }
            }

            writer.FinishWrite(header);
        }

        public void ExportXML(string filePath)
        {
            var rootElem = new XElement("PKG");

            for (int i = 0; i < Types.Count; i++)
            {
                var typeElem = new XElement("Type");
                var typeAttr = new XAttribute("Name", Types[i].TypeName);
                typeElem.Add(typeAttr);

                for (int f = 0; f < Types[i].Files.Count; f++)
                {
                    var fileElem = new XElement("File", Types[i].Files[f].FilePath);
                    var fileTypeAttr = new XAttribute("FriendlyName", Types[i].Files[f].FriendlyName);
                    fileElem.Add(fileTypeAttr);
                    typeElem.Add(fileElem);
                }

                rootElem.Add(typeElem);
            }

            var xml = new XDocument(rootElem);
            xml.Save(filePath);
        }

        public void ImportXML(string filepath)
        {
            var xml = XDocument.Load(filepath);
            foreach (var typeElem in xml.Root.Elements("Type"))
            {
                S06TypeEntry typeEntry = new S06TypeEntry();
                typeEntry.TypeName = typeElem.Attribute("Name").Value;
                foreach (var fileElem in typeElem.Elements("File"))
                {
                    S06FileEntry fileEntry = new S06FileEntry();
                    fileEntry.FriendlyName = fileElem.Attribute("FriendlyName").Value;
                    fileEntry.FilePath = fileElem.Value;
                    typeEntry.Files.Add(fileEntry);
                }
                Types.Add(typeEntry);
            }
        }
    }
}