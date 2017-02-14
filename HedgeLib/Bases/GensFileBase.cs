using HedgeLib.Headers;
using System;
using System.IO;

namespace HedgeLib.Bases
{
    public class GensFileBase : FileBase
    {
        //Variables/Constants
        public GensHeader Header;

        //Methods
        public override sealed void Load(Stream fileStream)
        {
            var reader = new ExtendedBinaryReader(fileStream, true);
            Header = ReadHeader(reader);

            Read(reader);
            //We don't really need to read the footer for our purposes.
        }

        protected virtual void Read(ExtendedBinaryReader reader)
        {
            throw new NotImplementedException();
        }

        private GensHeader ReadHeader(ExtendedBinaryReader reader)
        {
            var gensHeader = new GensHeader()
            {
                FileSize = reader.ReadUInt32(),
                RootNodeType = reader.ReadUInt32(),
                OffsetFinalTable = reader.ReadUInt32(),
                RootNodeOffset = reader.ReadUInt32(),
                OffsetFinalTableAbs = reader.ReadUInt32(),
                Padding = reader.ReadUInt32()
            };

            return gensHeader;
        }

        public override sealed void Save(Stream fileStream)
        {
            var writer = new ExtendedBinaryWriter(fileStream);

            writer.WriteNulls(GensHeader.Length);
            Write(writer);
            WriteFooter(writer);

            //We write the header last since there's no way we'll know the fileSize until here.
            writer.BaseStream.Position = 0;
            WriteHeader(writer);
        }

        protected virtual void Write(ExtendedBinaryWriter writer)
        {
            throw new NotImplementedException();
        }

        private void WriteHeader(ExtendedBinaryWriter writer)
        {
            writer.IsBigEndian = true;

            writer.Write(Header.FileSize);
            writer.Write(Header.RootNodeType);
            writer.Write(Header.OffsetFinalTable);
            writer.Write(Header.RootNodeOffset);
            writer.Write(Header.OffsetFinalTableAbs);
            writer.Write(Header.Padding);
        }

        private void WriteFooter(ExtendedBinaryWriter writer)
        {
            //TODO
            throw new NotImplementedException();
        }
    }
}