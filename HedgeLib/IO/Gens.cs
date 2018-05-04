using HedgeLib.Headers;
using System;
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
        public HedgehogEngineHeader ReadHeader()
        {
            uint fileSize = ReadUInt32();
            uint rootNodeType = ReadUInt32();
            JumpBehind(8);

            // Mirage Header
            byte mirageMarker = (byte)(fileSize >> 31);
            if (mirageMarker == 1 && rootNodeType == MirageHeader.Signature)
                return new MirageHeader(this);

            // Generations Header
            return new GensHeader(this);
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
            base(output, Encoding.ASCII, isBigEndian) { }

        public GensWriter(Stream output, Encoding encoding,
            bool isBigEndian = true) : base(output, encoding, isBigEndian) {}

        public GensWriter(Stream output, HedgehogEngineHeader header,
            string mirageType = null, bool isBigEndian = true) :
            base(output, Encoding.ASCII, isBigEndian)
        {
            if (!string.IsNullOrEmpty(mirageType) && header is MirageHeader mirageHeader)
            {
                mirageHeader.GenerateNodes(mirageType);
            }

            header.PrepareWrite(this);
        }

        public GensWriter(Stream output, Encoding encoding,
            HedgehogEngineHeader header, string mirageType = null,
            bool isBigEndian = true) : base(output, encoding, isBigEndian)
        {
            if (!string.IsNullOrEmpty(mirageType) && header is MirageHeader mirageHeader)
            {
                mirageHeader.GenerateNodes(mirageType);
            }

            header.PrepareWrite(this);
        }

        // Methods
        public void FinishWrite(HedgehogEngineHeader header,
            string mirageType = null, bool writeEOFNull = true)
        {
            // Write Footer and Update Header Values
            uint footerPos = (uint)BaseStream.Position;
            if (header is GensHeader gensHeader)
            {
                WriteFooter(true, writeEOFNull);

                header.FooterOffset = footerPos;
                gensHeader.RootNodeSize = (footerPos - Offset);
                gensHeader.FileSize = (uint)BaseStream.Position;
            }
            else if (header is MirageHeader mirageHeader)
            {
                FixPadding(16);

                uint footerPosPadded = (uint)BaseStream.Position;
                uint len = MirageHeader.Node.Length;

                header.FooterOffset = footerPosPadded;
                WriteFooter(false, writeEOFNull);
                
                // Update Sizes
                uint fileSize = (uint)BaseStream.Position;
                mirageHeader.FooterOffsetsCount = (uint)offsets.Count;
                mirageHeader.RootNode.DataSize = fileSize;

                if (!string.IsNullOrEmpty(mirageType))
                {
                    MirageHeader.Node typeNode, contextsNode;
                    typeNode = mirageHeader.GetNode(mirageType, false);

                    if (typeNode != null)
                    {
                        UpdateSize(typeNode, footerPosPadded);
                        contextsNode = typeNode.GetNode(MirageHeader.Contexts, false);

                        if (contextsNode != null)
                            UpdateSize(contextsNode, footerPos);
                    }
                }

                // Sub-Methods
                void UpdateSize(MirageHeader.Node node, uint endPos)
                {
                    if (node.DataSize == 0)
                        node.DataSize = (endPos - len);

                    len += MirageHeader.Node.Length;
                }
            }

            // Write Header
            BaseStream.Position = 0;
            header.FinishWrite(this);
        }

        public void WriteFooter(bool writeCount, bool writeEOFNull = true)
        {
            if (writeCount)
                Write((uint)offsets.Count);

            foreach (var offset in offsets)
            {
                Write(offset.Value - Offset);
            }

            if (writeEOFNull)
                WriteNulls(4);
        }
    }
}