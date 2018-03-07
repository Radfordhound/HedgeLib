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
            bool isBigEndian = true) : base(output, Encoding.ASCII, isBigEndian)
        {
            header.PrepareWrite(this);
        }

        public GensWriter(Stream output, Encoding encoding,
            HedgehogEngineHeader header, bool isBigEndian = true) :
            base(output, encoding, isBigEndian)
        {
            header.PrepareWrite(this);
        }

        // Methods
        public void FinishWrite(HedgehogEngineHeader header, string mirageType = null)
        {
            // Write Footer and Update Header Values
            uint footerPos = (uint)BaseStream.Position;
            if (header is GensHeader gensHeader)
            {
                WriteFooter(true);

                header.FooterOffsetAbs = footerPos;
                gensHeader.FooterOffset = (footerPos - Offset);
                gensHeader.FileSize = (uint)BaseStream.Position;
            }
            else if (header is MirageHeader mirageHeader)
            {
                FixPadding(16);

                MirageHeader.Node typeNode, contextsNode;
                uint footerPosPadded = (uint)BaseStream.Position;
                uint len = MirageHeader.Node.Length;

                header.FooterOffsetAbs = footerPosPadded;
                WriteFooter(false);
                
                if (mirageHeader.RootNode.Nodes.Count < 1)
                {
                    // Auto-Generate MirageHeader
                    if (string.IsNullOrEmpty(mirageType))
                        throw new Exception("Could not auto-generate MirageNodes.");

                    typeNode = mirageHeader.AddNode(mirageType, 1);
                    contextsNode = typeNode.AddNode(
                        MirageHeader.Contexts, header.RootNodeType);
                }
                else
                {
                    // Update Type
                    typeNode = mirageHeader.GetNode(mirageType, false);
                    if (typeNode == null)
                    {
                        mirageHeader.RootNode.Nodes.Clear();
                        typeNode = mirageHeader.AddNode(mirageType, 1);
                    }

                    // Update Contexts
                    contextsNode = typeNode.GetNode(
                        MirageHeader.Contexts, false);

                    if (contextsNode == null)
                    {
                        contextsNode = typeNode.AddNode(MirageHeader.Contexts,
                            header.RootNodeType);
                    }
                    else
                    {
                        contextsNode.Value = header.RootNodeType;
                    }
                }

                // Update Sizes
                uint fileSize = (uint)BaseStream.Position;
                mirageHeader.FooterLength = (fileSize - footerPosPadded);

                mirageHeader.RootNode.DataSize = fileSize;
                UpdateSize(typeNode, footerPosPadded);
                UpdateSize(contextsNode, footerPos);
                // TODO: Update the sizes of each node

                // Sub-Methods
                void UpdateSize(MirageHeader.Node node, uint endPos)
                {
                    node.DataSize = (endPos - len);
                    len += MirageHeader.Node.Length;
                }
            }

            // Write Header
            BaseStream.Position = 0;
            header.FinishWrite(this);
        }

        public void WriteFooter(bool writeCount)
        {
            if (writeCount)
                Write((uint)offsets.Count);

            foreach (var offset in offsets)
            {
                Write(offset.Value - Offset);
            }

            WriteNulls(4);
        }
    }
}