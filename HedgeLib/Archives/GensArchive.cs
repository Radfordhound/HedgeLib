using HedgeLib.IO;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace HedgeLib.Archives
{
    public class GensArchive : Archive
    {
        // Variables/Constants
        public uint Padding = 0x40;

        public const string ARLSignature = "ARL2";
        public const string ListExtension = ".arl", Extension = ".ar",
            PFDExtension = ".pfd", SplitExtension = ".00";
        public const uint Sig1 = 0, Sig2 = 0x10, Sig3 = 0x14;
        public const uint MinFileEntrySize = 21, PFDPadding = 2048;

        public override bool HasSplitArchives => true;

        // Constructors
        public GensArchive() : base() { }
        public GensArchive(Archive arc) : base(arc) { }

        // Methods
        public override void Load(string filePath)
        {
            var fileInfo = new FileInfo(filePath);
            if (fileInfo.Extension == SplitExtension || fileInfo.Extension == ListExtension)
            {
                var splitArchives = GetSplitArchivesList(filePath);
                foreach (string arc in splitArchives)
                    LoadFile(arc);
            }
            else
                LoadFile(filePath);
        }

        public override List<string> GetSplitArchivesList(string filePath)
        {
            var fileInfo = new FileInfo(filePath);
            var splitArchivesList = new List<string>();

            // PFD
            if (fileInfo.Extension == PFDExtension)
            {
                splitArchivesList.Add(filePath);
            }

            // ARL, AR.00, AR, and everything else
            else
            {
                string ext = (fileInfo.Extension == ListExtension) ?
                    ".ar" : fileInfo.Extension;

                if (int.TryParse(ext.Substring(1), out var e))
                    ext = "";

                // fileInfo.Extension only gets the last extension in the fileName.
                // Therefore if the fileName is something like "ghz200.ar.00", this
                // will only remove the ".00" from the end.
                string shortName = fileInfo.Name.Substring(0,
                    fileInfo.Name.Length - fileInfo.Extension.Length);

                for (int i = 0; i <= 99; ++i)
                {
                    string fileName = Path.Combine(fileInfo.DirectoryName,
                        $"{shortName}{ext}.{i.ToString("00")}");

                    if (!File.Exists(fileName))
                        break;

                    splitArchivesList.Add(fileName);
                }
            }

            return splitArchivesList;
        }

        public override void Load(Stream fileStream)
        {
            // Header
            var reader = new ExtendedBinaryReader(fileStream);

            // Apparently SEGA doesn't even do signature checking (try loading an AR in-game
            // with the first 0xC bytes set to 0, it works just fine), so why should we?
            reader.JumpAhead(0xC);
            Padding = reader.ReadUInt32();

            // Data
            while (fileStream.Position < fileStream.Length)
            {
                reader.Offset = (uint)fileStream.Position;

                uint dataEndOffset = reader.ReadUInt32();
                uint dataLength = reader.ReadUInt32();
                uint dataStartOffset = reader.ReadUInt32();
                uint unknown1 = reader.ReadUInt32();
                uint unknown2 = reader.ReadUInt32();
                string name = reader.ReadNullTerminatedString();

                reader.JumpTo(dataStartOffset, false);
                var data = reader.ReadBytes((int)dataLength);
                reader.JumpTo(dataEndOffset, false);

                var file = new ArchiveFile()
                {
                    Name = name,
                    Data = data
                };
                Data.Add(file);
            }
        }

        public override void Save(string filePath, bool overwrite = false)
        {
            Save(filePath, false, null);
        }

        public void Save(string filePath, bool generateARL = true, uint? splitCount = 0xA00000)
        {
            var fileInfo = new FileInfo(filePath);
            var archiveSizes = new List<uint>();
            string shortName = fileInfo.Name.Substring(0,
                fileInfo.Name.IndexOf('.'));

            if (splitCount.HasValue)
            {
                // Generate split Archives
                int startIndex = 0, arcIndex = 0;

                while (startIndex != -1)
                {
                    string fileName = Path.Combine(fileInfo.DirectoryName,
                        $"{shortName}{Extension}.{arcIndex.ToString("00")}");

                    using (var fileStream = File.Create(fileName))
                    {
                        startIndex = Save(fileStream, splitCount, startIndex);

                        archiveSizes.Add((uint)fileStream.Length);
                        ++arcIndex;
                    }
                }
            }
            else
            {
                // Generate archive
                using (var fileStream = File.Create(filePath))
                {
                    Save(fileStream, splitCount, 0);
                    archiveSizes.Add((uint)fileStream.Length);
                }
            }

            // Generate ARL
            if (!generateARL) return;
            string arlPath = Path.Combine(fileInfo.DirectoryName,
                $"{shortName}{ListExtension}");

            using (var fileStream = File.Create(arlPath))
            {
                GenerateARL(fileStream, archiveSizes);
            }
        }

        public override void Save(Stream fileStream)
        {
            Save(fileStream, null, 0);
        }

        public int Save(Stream fileStream, uint? sizeLimit, int startIndex = 0)
        {
            // Header
            var files = GetFiles(false);
            var writer = new ExtendedBinaryWriter(fileStream, Encoding.ASCII, false);

            writer.Write(Sig1);
            writer.Write(Sig2);
            writer.Write(Sig3);
            writer.Write(Padding);

            // The absolute smallest an archive with a header and one
            // valid file entry can be is 37 bytes.
            if (sizeLimit.HasValue && sizeLimit < 37)
            {
                throw new ArgumentOutOfRangeException("sizeLimit", sizeLimit,
                    "The sizeLimit argument must at least be greater than 36.");
            }

            // Data
            for (int i = startIndex; i < files.Count; ++i)
            {
                var file = files[i];
                writer.Offset = (uint)fileStream.Position;

                if (sizeLimit.HasValue && i > startIndex && fileStream.Position +
                    MinFileEntrySize + file.Data.Length > sizeLimit)
                    return i;

                writer.AddOffset("dataEndOffset");
                writer.Write((uint)file.Data.LongLength);
                writer.AddOffset("dataStartOffset");
                writer.WriteNulls(8); // TODO: Figure out what unknown1 and unknown2 are.
                writer.WriteNullTerminatedString(file.Name);
                writer.FixPadding(Padding);
                
                writer.FillInOffset("dataStartOffset", false);
                writer.Write(file.Data);
                writer.FillInOffset("dataEndOffset", false);
            }

            return -1;
        }

        public void GenerateARL(Stream fileStream, List<uint> archiveSizes)
        {
            // Header
            var writer = new ExtendedBinaryWriter(fileStream, Encoding.ASCII, false);
            writer.WriteSignature(ARLSignature);

            writer.Write((uint)archiveSizes.Count);
            foreach (uint arcSize in archiveSizes)
                writer.Write(arcSize);

            // Data
            foreach (var file in Data)
                writer.Write(file.Name);
        }

        private void LoadFile(string filePath)
        {
            using (var fileStream = File.OpenRead(filePath))
            {
                Load(fileStream);
            }
        }
    }
}