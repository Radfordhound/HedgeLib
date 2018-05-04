using System;
using System.Collections.Generic;
using HedgeLib.IO;

namespace HedgeLib.Headers
{
    // Full Credit to Skyth for cracking the new header format!
    public class MirageHeader : HedgehogEngineHeader
    {
        // Variables/Constants
        public Node RootNode = new Node();
        public uint FooterOffsetsCount;

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
            RootNode.ReadRoot(reader, out FooterOffset, out FooterOffsetsCount);

            var contexts = RootNode.GetNode(Contexts, true);
            if (contexts != null)
                RootNodeType = contexts.Value;
        }

        public void GenerateNodes(string type)
        {
            if (RootNode == null)
                RootNode = new Node();

            Node typeNode, contextsNode;
            if (RootNode.Nodes.Count < 1)
            {
                // Auto-Generate MirageHeader
                if (string.IsNullOrEmpty(type))
                    throw new Exception("Could not auto-generate MirageNodes.");

                typeNode = AddNode(type, 1);
                contextsNode = typeNode.AddNode(
                    Contexts, RootNodeType);
            }
            else
            {
                // Update Type
                typeNode = GetNode(type, false);
                if (typeNode == null)
                {
                    RootNode.Nodes.Clear();
                    typeNode = AddNode(type, 1);
                }

                // Update Contexts
                contextsNode = typeNode.GetNode(Contexts, false);
                if (contextsNode == null)
                {
                    contextsNode = typeNode.AddNode(
                        Contexts, RootNodeType);
                }
                else
                {
                    contextsNode.Value = RootNodeType;
                }
            }
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
                    if (value == null) return;
                    int len = value.Length;

                    if (len == NameLength)
                    {
                        name = value;
                    }
                    else if (len < NameLength)
                    {
                        // I'm aware this essentially creates two new strings.
                        // However, in this case, I feel it's efficient enough™.
                        name = value + new string(' ', NameLength - len);
                    }
                    else throw new Exception("MirageNode Names must be <= 8 characters!");
                }
            }

            public uint DataSize, Value;
            public NodeFlags Flags => flags;
            protected NodeFlags flags;
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
                Name = name;
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
                out uint footerOffset, out uint footerCount)
            {
                DataSize = reader.ReadUInt32();
                Value = reader.ReadUInt32();
                footerOffset = reader.ReadUInt32();
                footerCount = reader.ReadUInt32();

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
                flags = (NodeFlags)(DataSize >> 29);
                DataSize &= 0x1FFFFFFF;

                // Return if this node has no child nodes
                if ((flags & NodeFlags.HasNoChildren) != 0)
                    return;

                // Read Child Nodes
                Node child;
                do
                {
                    child = new Node(reader);
                    Nodes.Add(child);
                }
                while ((child.flags & NodeFlags.IsLastChildNode) == 0);
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
                Value = Signature;
                flags |= NodeFlags.IsRootNode;
                Write(writer);

                writer.Write(header.FooterOffset);
                writer.Write(header.FooterOffsetsCount);

                WriteChildren(writer);
            }

            public void FinishWrite(ExtendedBinaryWriter writer)
            {
                Write(writer);
                writer.Write(name.ToCharArray());

                WriteChildren(writer);
            }

            protected void WriteChildren(ExtendedBinaryWriter writer)
            {
                for (int i = 0; i < Nodes.Count; ++i)
                {
                    var node = Nodes[i];
                    if (i == (Nodes.Count - 1))
                        node.flags |= NodeFlags.IsLastChildNode;

                    node.FinishWrite(writer);
                }
            }

            protected void Write(ExtendedBinaryWriter writer)
            {
                if (Nodes.Count < 1)
                    flags |= NodeFlags.HasNoChildren;

                writer.Write(((uint)flags << 29) | DataSize);
                writer.Write(Value);
            }
        }
    }
}