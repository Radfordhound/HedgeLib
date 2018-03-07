using HedgeLib.Archives;
using HedgeLib.Headers;
using HedgeLib.IO;
using System.Collections.Generic;
using System.IO;

namespace HedgeLib.Misc
{
    // Based off of the wonderful SCHG page on Sonic Generations over at Sonic Retro
    public class GensPFI : FileBase
    {
        // Variables/Constants
        public List<AREntry> Entries = new List<AREntry>();
        public GensHeader Header = new GensHeader();
        public const string Extension = ".pfi";

        // Constructors
        public GensPFI() { }
        public GensPFI(GensArchive pfd)
        {
            // Generate a PFI from a PFD archive
            var files = pfd.GetFiles(false);
            uint offset = 0x10;

            for (int i = 0; i < files.Count; ++i)
            {
                var file = files[i];
                offset += (GensArchive.MinFileEntrySize + (uint)file.Name.Length);

                while (offset % pfd.Padding != 0)
                    ++offset;

                uint dataLen = (uint)file.Data.LongLength;
                Entries.Add(new AREntry()
                {
                    Name = file.Name,
                    PFDOffset = offset,
                    Size = dataLen
                });

                offset += dataLen;
            }
        }

        // Methods
        /// <summary>
        /// Packs a PFD and generates a PFI.
        /// </summary>
        /// <param name="input">The directory to pack into a PFD</param>
        /// <param name="output">The directory to output PFD and PFI files.
        /// Auto-detected if none is given.</param>
        /// <param name="pfdPadding">How much padding the generated PFD should have.</param>
        public static void PackPFD(string input, string output = null,
            uint pfdPadding = 2048)
        {
            // Get output Directory
            var info = new DirectoryInfo(input);
            if (string.IsNullOrEmpty(output))
                output = info.Parent.FullName;

            Directory.CreateDirectory(output);

            // Pack PFD
            var pfd = new GensArchive();
            string pfdPath = Path.Combine(output,
                info.Name + GensArchive.PFDExtension);

            pfd.Padding = pfdPadding;
            pfd.AddDirectory(input);
            pfd.Save(pfdPath, overwrite: true);

            // Generate PFI
            var pfi = new GensPFI(pfd);
            string pfiPath = Path.Combine(output,
                info.Name + Extension);

            pfi.Save(pfiPath, true);
        }

        public override void Load(Stream fileStream)
        {
            var reader = new GensReader(fileStream);
            Header = new GensHeader(reader);

            // Root Node
            uint entryCount = reader.ReadUInt32();
            uint entriesOffset = reader.ReadUInt32();

            // AR Entry Offsets
            var entryOffsets = new uint[entryCount];
            reader.JumpTo(entriesOffset, false);

            for (uint i = 0; i < entryCount; ++i)
            {
                entryOffsets[i] = reader.ReadUInt32();
            }

            // AR Entries
            for (uint i = 0; i < entryCount; ++i)
            {
                reader.JumpTo(entryOffsets[i], false);

                uint nameOffset = reader.ReadUInt32();
                uint offset = reader.ReadUInt32();
                uint size = reader.ReadUInt32();

                reader.JumpTo(nameOffset, false);
                Entries.Add(new AREntry()
                {
                    PFDOffset = offset,
                    Size = size,
                    Name = reader.ReadNullTerminatedString()
                });
            }
        }

        public override void Save(Stream fileStream)
        {
            var writer = new GensWriter(fileStream, Header);

            // Root Node
            writer.Write(Entries.Count);
            writer.AddOffset("entriesOffset");

            // AR Entry Offsets
            writer.FillInOffset("entriesOffset", false, false);
            writer.AddOffsetTable("entry", (uint)Entries.Count);

            // AR Entries
            for (int i = 0; i < Entries.Count; ++i)
            {
                var entry = Entries[i];
                writer.FillInOffset($"entry_{i}", false, false);

                writer.AddOffset($"nameOffset{i}");
                writer.Write(entry.PFDOffset);
                writer.Write(entry.Size);

                writer.FillInOffset($"nameOffset{i}", false, false);
                writer.WriteNullTerminatedString(entry.Name);
                writer.FixPadding(4);
            }

            writer.FinishWrite(Header);
        }

        // Other
        public class AREntry
        {
            // Variables/Constants
            public uint PFDOffset, Size;
            public string Name;
        }
    }
}