using HedgeLib.IO;
using System;
using System.Collections.Generic;
using System.IO;

namespace HedgeLib.Havok
{
    public class HavokFile : FileBase
    {
        // Variables/Constants
        public Dictionary<string, HavokSection> Sections =
            new Dictionary<string, HavokSection>();

        public string ContentsVersion, TopLevelObject;
        public uint UserTag = 0;
        public int ClassVersion;

        // Methods
        public void ImportXML(string filePath)
        {
            using (var fs = OpenStream(filePath))
                ImportXML(fs);
        }

        public void ExportXML(string filePath)
        {
            using (var fs = OpenStream(filePath))
                ExportXML(fs);
        }

        public void ImportXML(Stream fileStream)
        {
            HavokXML.Read(this, fileStream);
        }

        public void ExportXML(Stream fileStream)
        {
            HavokXML.Write(this, fileStream);
        }

        public void LoadPackFile(string filePath)
        {
            using (var fs = OpenStream(filePath))
                LoadPackFile(fs);
        }

        public void LoadPackFile(Stream fileStream)
        {
            var reader = new ExtendedBinaryReader(fileStream, false);
            HavokPackFile.Read(this, reader);
        }

        public void LoadTagFile(string filePath)
        {
            using (var fs = OpenStream(filePath))
                LoadPackFile(fs);
        }

        public void LoadTagFile(Stream fileStream)
        {
            var reader = new ExtendedBinaryReader(fileStream, false);
            HavokTagFile.Read(this, reader);
        }

        public override void Load(Stream fileStream)
        {
            var reader = new ExtendedBinaryReader(fileStream, false);

            // Determine what type of file this is
            uint magic1 = reader.ReadUInt32();
            uint magic2 = reader.ReadUInt32();
            reader.JumpTo(0);

            // Pack File
            if (magic1 == HavokPackFile.Magic1 && magic2 == HavokPackFile.Magic2)
            {
                HavokPackFile.Read(this, reader);
                return;
            }

            // Tag File
            if (magic2 == HavokTagFile.TAGMagic)
            {
                HavokTagFile.Read(this, reader);
                return;
            }

            // XML
            if (magic1 == 0x6D783F3C && magic2 == 0x6576206C)
            {
                HavokXML.Read(this, fileStream);
                return;
            }

            throw new NotSupportedException("The given file's format is unsupported!");
        }

        protected Stream OpenStream(string filePath)
        {
            // Throw exceptions if necessary
            if (string.IsNullOrEmpty(filePath))
                throw new ArgumentNullException("filePath");

            if (!File.Exists(filePath))
            {
                throw new FileNotFoundException(
                    "The given file could not be loaded, as it cannot be found!", filePath);
            }

            // Open a stream to the file
            return File.OpenRead(filePath);
        }
    }
}