using HedgeLib.Headers;
using System;
using System.IO;

namespace HedgeLib.Bases
{
    public class LWFileBase : FileBase
    {
        //Variables/Constants
        public LWHeader Header;

        //Methods
        public override sealed void Load(Stream fileStream)
        {
            var reader = new ExtendedBinaryReader(fileStream);
            Header = ReadHeader(reader);

            Read(reader);
            //We don't really need to read the footer for our purposes.
        }

        protected virtual void Read(ExtendedBinaryReader reader)
        {
            throw new NotImplementedException();
        }

        private LWHeader ReadHeader(ExtendedBinaryReader reader)
        {
            //BINA Header
            var header = new LWHeader();

            var sig = reader.ReadSignature();
            if (sig != LWHeader.Signature)
                throw new InvalidDataException("The given file's signature was incorrect!" +
                    " (Expected " + LWHeader.Signature + " got " + sig + ".)");

            header.VersionString = reader.ReadSignature(3);
            reader.IsBigEndian = (reader.ReadChar() == 'B');
            header.FileSize = reader.ReadUInt32();

            reader.JumpAhead(4); //TODO: Figure out what this value is.

            //DATA Header
            var dataSig = reader.ReadSignature();
            if (dataSig != LWHeader.DataSignature)
                throw new InvalidDataException("The given file's signature was incorrect!" +
                    " (Expected " + LWHeader.DataSignature + " got " + dataSig + ".)");

            header.DataLength = reader.ReadUInt32();
            header.CustomDataLength = reader.ReadUInt32();
            header.StringDataLength = reader.ReadUInt32();
            header.FinalTableLength = reader.ReadUInt32();

            uint padding = reader.ReadUInt32(); //TODO: Make sure this is correct.
            reader.JumpAhead(padding);

            return header;
        }

        public override sealed void Save(Stream fileStream)
        {
            var writer = new ExtendedBinaryWriter(fileStream);

            writer.WriteNulls(LWHeader.Length);
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
            //TODO
            throw new NotImplementedException();
        }

        private void WriteFooter(ExtendedBinaryWriter writer)
        {
            //TODO
            throw new NotImplementedException();
        }
    }
}