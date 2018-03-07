using System;
using System.Collections.Generic;
using HedgeLib.IO;

namespace HedgeLib.Headers
{
    // Full Credit to Skyth for cracking the new header format!
    public class MirageHeader : HedgehogEngineHeader
    {
        // Variables/Constants
        public Node RootNode;
        public uint FooterLength;

        public const string NodesExt = "NodesExt", NodePrms = "NodePrms",
            SCAParam = "SCAParam", Contexts = "Contexts";
        public const uint Signature = 0x133054A;

        // Constructors
        public MirageHeader() { }
        public MirageHeader(ExtendedBinaryReader reader)
        {
            Read(reader);
        }

        public MirageHeader(ExtendedBinaryWriter writer)
        {
            PrepareWrite(writer);
        }

        // Methods
        public Node AddNode(string name, uint value)
        {
            return RootNode.AddNode(name, value);
        }

        public Node GetNode(string name, bool searchSubNodes)
        {
            return RootNode.GetNode(name, searchSubNodes);
        }

        public override void Read(ExtendedBinaryReader reader)
        {
            reader.Offset = Node.Length;
            RootNode = new Node();
            RootNode.ReadRoot(reader, out FooterOffsetAbs, out FooterLength);

            var contexts = RootNode.GetNode(Contexts, true);
            if (contexts != null)
                RootNodeType = contexts.Value;
        }

        public override void PrepareWrite(ExtendedBinaryWriter writer)
        {
            writer.Offset = Node.Length;
            RootNode.PrepareWrite(writer);
        }

        public override void FinishWrite(ExtendedBinaryWriter writer)
        {
            RootNode.FinishWriteRoot(writer, this);
        }

        // Other
        public class Node
        {
            // Variables/Constants
            public List<Node> Nodes = new List<Node>();
            public string Name
            {
                get => name;
                set
                {
                    int len = value.Length;
                    if (len == NameLength)
                    {
                        name = value;
                    }
                    else if (len < NameLength)
                    {
                        // I'm aware this essentially creates two new strings.
                        // However, in this case, I feel it's efficient enough™.
                        name += new string(' ', NameLength - len);
                    }
                    else throw new Exception("MirageNode Names must be <= 8 characters!");
                }
            }

            public uint DataSize, Value;
            public NodeFlags Flags;
            protected string name;

            public const uint Length = 0x10;
            public const int NameLength = 8;

            [Flags]
            public enum NodeFlags
            {
                HasNoChildren = 1,
                IsLastChildNode = 2,
                IsRootNode = 4
            }

            // Constructors
            public Node() { }
            public Node(ExtendedBinaryReader reader)
            {
                Read(reader);
            }

            public Node(string name, uint value)
            {
                this.name = name;
                Value = value;
            }

            // Methods
            public Node AddNode(string name, uint value)
            {
                var node = new Node(name, value);
                Nodes.Add(node);
                return node;
            }

            public Node GetNode(string name, bool searchSubNodes)
            {
                foreach (var node in Nodes)
                {
                    if (node.Name == name)
                        return node;

                    if (searchSubNodes)
                    {
                        var n = node.GetNode(name, searchSubNodes);
                        if (n != null)
                            return n;
                    }
                }

                return null;
            }

            public void ReadRoot(ExtendedBinaryReader reader,
                out uint footerOffset, out uint footerLength)
            {
                DataSize = reader.ReadUInt32();
                Value = reader.ReadUInt32();
                footerOffset = reader.ReadUInt32();
                footerLength = reader.ReadUInt32();

                FinishRead(reader);
            }

            public void Read(ExtendedBinaryReader reader)
            {
                DataSize = reader.ReadUInt32();
                Value = reader.ReadUInt32();
                name = new string(reader.ReadChars(8));

                // Remove spaces from name
                for (int i = NameLength; i > 0;)
                {
                    if (name[--i] != ' ')
                    {
                        name = name.Substring(0, ++i);
                        break;
                    }
                }

                FinishRead(reader);
            }

            protected void FinishRead(ExtendedBinaryReader reader)
            {
                // Separate Flags and Size
                Flags = (NodeFlags)(DataSize >> 29);
                DataSize &= 0x1FFFFFFF;

                // Return if this node has no child nodes
                if ((Flags & NodeFlags.HasNoChildren) != 0)
                    return;

                // Read Child Nodes
                Node child;
                do
                {
                    child = new Node(reader);
                    Nodes.Add(child);
                }
                while ((child.Flags & NodeFlags.IsLastChildNode) == 0);
            }

            public void PrepareWrite(ExtendedBinaryWriter writer)
            {
                writer.WriteNulls(Length);
                foreach (var child in Nodes)
                {
                    child.PrepareWrite(writer);
                }
            }

            public void FinishWriteRoot(ExtendedBinaryWriter writer,
                MirageHeader header)
            {
                Value = header.RootNodeType;
                Write(writer);

                writer.Write(header.FooterOffsetAbs);
                writer.Write(header.FooterLength);

                foreach (var node in Nodes)
                {
                    node.FinishWrite(writer);
                }
            }

            public void FinishWrite(ExtendedBinaryWriter writer)
            {
                Write(writer);
                writer.Write(name.ToCharArray());

                foreach (var node in Nodes)
                {
                    node.FinishWrite(writer);
                }
            }

            protected void Write(ExtendedBinaryWriter writer)
            {
                writer.Write(((uint)Flags << 29) | DataSize);
                writer.Write(Value);
            }
        }
    }
}