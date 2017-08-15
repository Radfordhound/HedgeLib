using HedgeLib.Headers;
using System.IO;
using System.Text;

namespace HedgeLib.IO
{
    public class GensReader : ExtendedBinaryReader
    {
        // Constructors
        public GensReader(Stream input, bool isBigEndian = true) :
            base(input, Encoding.ASCII, isBigEndian) { }

        public GensReader(Stream input, Encoding encoding,
            bool isBigEndian = true) : base(input, encoding, isBigEndian) { }

        // Methods
        public GensHeader ReadHeader()
        {
            var header = new GensHeader()
            {
                FileSize = ReadUInt32(),
                RootNodeType = ReadUInt32(),
                OffsetFinalTable = ReadUInt32(),
                RootNodeOffset = ReadUInt32(),
                OffsetFinalTableAbs = ReadUInt32(),
                FileEndOffset = ReadUInt32()
            };

            Offset = header.RootNodeOffset;
            JumpTo(header.RootNodeOffset);
            return header;
        }

        public uint[] ReadFooter()
        {
            uint offsetCount = ReadUInt32();
            var offsets = new uint[offsetCount];

            for (uint i = 0; i < offsetCount; ++i)
            {
                offsets[i] = (ReadUInt32() + Offset);
            }

            return offsets;
        }
    }

    public class GensWriter : ExtendedBinaryWriter
    {
        // Constructors
        public GensWriter(Stream output, bool isBigEndian = true) :
            base(output, Encoding.ASCII, isBigEndian)
        {
            Offset = GensHeader.Length;
            WriteNulls(Offset);
        }

        public GensWriter(Stream output, Encoding encoding,
            bool isBigEndian = true) : base(output, encoding, isBigEndian)
        {
            Offset = GensHeader.Length;
            WriteNulls(Offset);
        }

        public GensWriter(Stream output, GensHeader header,
            bool isBigEndian = true) : base(output, Encoding.ASCII, isBigEndian)
        {
            if (header.RootNodeOffset < GensHeader.Length)
                header.RootNodeOffset = GensHeader.Length;

            Offset = header.RootNodeOffset;
            WriteNulls(Offset);
        }

        public GensWriter(Stream output, Encoding encoding, GensHeader header,
            bool isBigEndian = true) : base(output, encoding, isBigEndian)
        {
            if (header.RootNodeOffset < GensHeader.Length)
                header.RootNodeOffset = GensHeader.Length;

            Offset = header.RootNodeOffset;
            WriteNulls(Offset);
        }

        // Methods
        public void FinishWrite(GensHeader header)
        {
            WriteFooter(header);
            FillInHeader(header);
        }

        public void FillInHeader(GensHeader header)
        {
            BaseStream.Position = 0;

            Write(header.FileSize);
            Write(header.RootNodeType);
            Write(header.OffsetFinalTable);
            Write(Offset);
            Write(header.OffsetFinalTableAbs);
            Write(header.FileEndOffset);
        }

        public void WriteFooter(GensHeader header)
        {
            uint finalTablePos = (uint)BaseStream.Position;
            Write((uint)offsets.Count);

            foreach (var offset in offsets)
            {
                Write(offset.Value - Offset);
            }

            WriteNulls(4);

            // Update Header Values
            header.OffsetFinalTableAbs = finalTablePos;
            header.OffsetFinalTable = finalTablePos - Offset;
            header.FileSize = (uint)BaseStream.Position;
        }
    }
}