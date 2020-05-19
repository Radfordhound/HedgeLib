using System.IO;
using HedgeLib.IO;
using HedgeLib.Headers;
using System.Collections.Generic;

namespace HedgeLib.Archives
{
    public class S06TypeEntry
    {
        public string TypeName;
        public uint FileStartIndex;
        public uint FileCount;

        public S06TypeEntry() {}
        public S06TypeEntry(BINAReader reader, uint fileEntriesPos)
        {
            // Read entry
            uint namePos = reader.ReadUInt32();
            FileCount = reader.ReadUInt32();
            uint filesPos = reader.ReadUInt32();

            // Read name
            long curPos = reader.BaseStream.Position;
            reader.JumpTo(namePos, false);
            TypeName = reader.ReadNullTerminatedString();

            // Get file start index
            FileStartIndex = ((filesPos - fileEntriesPos) / 8);

            // Jump back
            reader.JumpTo(curPos);
        }
    }

    public class S06FileEntry
    {
        public string FriendlyName;
        public string FilePath;

        public S06FileEntry() {}
        public S06FileEntry(BINAReader reader)
        {
            // Read entry
            uint friendlyNamePos = reader.ReadUInt32();
            uint filePathPos = reader.ReadUInt32();

            // Read strings
            long curPos = reader.BaseStream.Position;
            reader.JumpTo(friendlyNamePos, false);
            FriendlyName = reader.ReadNullTerminatedString();

            reader.JumpTo(filePathPos, false);
            FilePath = reader.ReadNullTerminatedString();

            // Jump back
            reader.JumpTo(curPos);
        }
    }

    public class S06Package : FileBase
    {
        public List<S06FileEntry> Files = new List<S06FileEntry>();
        public List<S06TypeEntry> Types = new List<S06TypeEntry>();

        public override void Load(Stream fileStream)
        {
            // Header
            var reader = new BINAReader(fileStream);
            reader.ReadHeader();

            uint fileCount = reader.ReadUInt32();
            uint fileEntriesPos = reader.ReadUInt32();

            uint typeCount = reader.ReadUInt32();
            uint typeEntriesPos = reader.ReadUInt32();

            // Files
            Files.Capacity = (int)fileCount;
            reader.JumpTo(fileEntriesPos, false);

            for (uint i = 0; i < fileCount; ++i)
            {
                Files.Add(new S06FileEntry(reader));
            }

            // Types
            Types.Capacity = (int)typeCount;
            reader.JumpTo(typeEntriesPos, false);

            for (uint i = 0; i < typeCount; ++i)
            {
                Types.Add(new S06TypeEntry(reader, fileEntriesPos));
            }
        }

        public override void Save(Stream fileStream)
        {
            // Header
            var header = new BINAv1Header();
            var writer = new BINAWriter(fileStream, header);

            writer.Write((uint)Files.Count);
            writer.AddOffset("fileEntriesPos");
            writer.Write((uint)Types.Count);
            writer.AddOffset("typeEntriesPos");

            // Types
            writer.FillInOffset("typeEntriesPos", false);
            for (int i = 0; i < Types.Count; i++)
            {
                writer.AddString($"typeName{i}", Types[i].TypeName);
                writer.Write(Types[i].FileCount);
                writer.AddOffset($"typeFilesOffset{i}");
            }

            // Files
            uint fileEntriesPos = (uint)writer.BaseStream.Position;

			writer.FillInOffset("fileEntriesPos", false);
            for (int i = 0; i < Files.Count; i++)
            {
                writer.AddString($"friendlyName{i}", Files[i].FriendlyName);
                writer.AddString($"filePath{i}", Files[i].FilePath);
            }

            // Fill-in type file offsets
            for (int i = 0; i < Types.Count; i++)
            {
                writer.FillInOffset($"typeFilesOffset{i}",
                    (fileEntriesPos + (Types[i].FileStartIndex * 8U)),
                    false, false);
            }

            writer.FinishWrite(header);
        }
    }
}
