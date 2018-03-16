using HedgeLib.IO;
using System.IO;
using HedgeLib.Headers;
using System;
using System.Collections.Generic;
using System.Xml.Linq;
using System.Text;

namespace HedgeLib.Misc
{
    public class ForcesText : FileBase
    {
        // Variables/Constants
        public Dictionary<string, EntryType> Types = new Dictionary<string, EntryType>();
        public List<Layout> Layouts = new List<Layout>();
        public List<Node> Nodes = new List<Node>();
        public BINAHeader Header = new BINAHeader() { Version = 210 };

        public const string Extension = ".cnvrs-text";
        public const char NullReplaceChar = '⭗';

        // Methods
        public override void Load(Stream fileStream)
        {
            // BINA Header
            var reader = new BINAReader(fileStream, BINA.BINATypes.Version2);
            Header = reader.ReadHeader();

            // Header
            byte unknown1 = reader.ReadByte(); // Always 3?
            byte nodeCount = reader.ReadByte(); // ?
            byte unknown2 = reader.ReadByte(); // Always 0?
            byte unknown3 = reader.ReadByte(); // Always 0?
            uint unknown4 = reader.ReadUInt32();
            long nodesOffset = reader.ReadInt64();

            // Nodes
            reader.JumpTo(nodesOffset, false);
            for (uint i = 0; i < nodeCount; ++i)
            {
                Nodes.Add(new Node()
                {
                    NameOffset = reader.ReadInt64(),
                    EntriesCount = reader.ReadUInt64(),
                    EntriesOffset = reader.ReadInt64()
                });
            }

            // Entries
            for (int i = 0; i < nodeCount; ++i)
            {
                var node = Nodes[i];
                reader.JumpTo(node.EntriesOffset, false);

                for (uint i2 = 0; i2 < node.EntriesCount; ++i2)
                {
                    node.Entries.Add(new Entry()
                    {
                        UUID = reader.ReadUInt64(),
                        NameOffset = reader.ReadInt64(),
                        SecondEntryOffset = reader.ReadInt64(),
                        DataOffset = reader.ReadInt64()
                    });
                }
            }

            // Data
            foreach (var node in Nodes)
            {
                for (int i = 0; i < node.Entries.Count; ++i)
                {
                    var entry = node.Entries[i];
                    reader.JumpTo(entry.DataOffset.Value, false);

                    var chars = new List<byte>();
                    bool isReadingButton = false;

                    // Read Unicode strings
                    do
                    {
                        byte b1 = reader.ReadByte();
                        byte b2 = reader.ReadByte();

                        if (b1 != 0 && ((b2 & 0xE0) == 0xE0))
                            isReadingButton = true;

                        if (b1 == 0 && b2 == 0)
                        {
                            if (isReadingButton)
                            {
                                isReadingButton = false;
                                chars.Add(0x57); // NullReplaceChar byte1
                                chars.Add(0x2B); // NullReplaceChar byte2
                                continue;
                            }
                            else break;
                        }

                        chars.Add(b1);
                        chars.Add(b2);
                    }
                    while (fileStream.Position < fileStream.Length);
                    entry.Data = Encoding.Unicode.GetString(chars.ToArray());
                }
            }

            // Second Entries
            var typeOffsets = new Dictionary<long, string>();
            foreach (var node in Nodes)
            {
                for (int i = 0; i < node.Entries.Count; ++i)
                {
                    var entry = node.Entries[i];
                    reader.JumpTo(entry.SecondEntryOffset.Value, false);

                    long nameOffset = reader.ReadInt64();
                    long typeOffset = reader.ReadInt64();
                    entry.LayoutOffset = reader.ReadInt64();

                    if (nameOffset != entry.NameOffset)
                    {
                        Console.WriteLine(
                            "WARNING: Second name offset ({0:X}) != first ({1:X})!",
                            nameOffset, entry.NameOffset);
                    }

                    // Caption Type
                    if (typeOffsets.ContainsKey(typeOffset))
                    {
                        entry.TypeName = typeOffsets[typeOffset];
                        continue;
                    }

                    var type = new EntryType();
                    reader.JumpTo(typeOffset, false);

                    long typeNameOffset = reader.ReadInt64();
                    long typeNamespaceOffset = reader.ReadInt64();
                    long unknownFloat1Offset = reader.ReadInt64();
                    long unknownFloat2Offset = reader.ReadInt64();
                    long unknownFloat3Offset = reader.ReadInt64();
                    long unknownInt1Offset = reader.ReadInt64();
                    long unknownInt2Offset = reader.ReadInt64();
                    long unknownOffset1 = reader.ReadInt64();
                    long unknownULong2Offset = reader.ReadInt64();
                    long unknownOffset2 = reader.ReadInt64();
                    long unknownOffset3 = reader.ReadInt64();
                    long unknownOffset4 = reader.ReadInt64();
                    long unknownULong1Offset = reader.ReadInt64();
                    long unknownOffset5 = reader.ReadInt64();

                    // UnknownFloat1
                    if (unknownFloat1Offset > 0)
                    {
                        reader.JumpTo(unknownFloat1Offset, false);
                        type.UnknownFloat1 = reader.ReadSingle();
                        uint padding1 = reader.ReadUInt32();

                        if (padding1 != 0)
                            Console.WriteLine("WARNING: Type Padding1 != 0 (0x{0:X})", padding1);
                    }

                    // UnknownFloat2
                    if (unknownFloat2Offset > 0)
                    {
                        reader.JumpTo(unknownFloat2Offset, false);
                        type.UnknownFloat2 = reader.ReadSingle();
                        uint padding2 = reader.ReadUInt32();

                        if (padding2 != 0)
                            Console.WriteLine("WARNING: Type Padding2 != 0 (0x{0:X})", padding2);
                    }

                    // UnknownFloat3
                    if (unknownFloat3Offset > 0)
                    {
                        reader.JumpTo(unknownFloat3Offset, false);
                        type.UnknownFloat3 = reader.ReadSingle();
                        uint padding3 = reader.ReadUInt32();

                        if (padding3 != 0)
                            Console.WriteLine("WARNING: Type Padding3 != 0 (0x{0:X})", padding3);
                    }

                    // UnknownInt1
                    if (unknownInt1Offset > 0)
                    {
                        reader.JumpTo(unknownInt1Offset, false);
                        type.UnknownInt1 = reader.ReadInt32();
                        uint padding4 = reader.ReadUInt32();

                        if (padding4 != 0)
                            Console.WriteLine("WARNING: Type Padding4 != 0 (0x{0:X})", padding4);
                    }

                    // UnknownInt2
                    if (unknownInt2Offset > 0)
                    {
                        reader.JumpTo(unknownInt2Offset, false);
                        type.UnknownInt2 = reader.ReadInt32();
                        uint padding5 = reader.ReadUInt32();

                        if (padding5 != 0)
                            Console.WriteLine("WARNING: Type Padding5 != 0 (0x{0:X})", padding5);
                    }

                    if (unknownOffset1 != 0)
                        Console.WriteLine("WARNING: Type ukOff1 != 0 (0x{0:X})", unknownOffset1);

                    // UnknownULong2Offset
                    if (unknownULong2Offset > 0)
                    {
                        reader.JumpTo(unknownULong2Offset, false);
                        type.UnknownULong2 = reader.ReadUInt64();
                    }

                    if (unknownOffset2 != 0)
                        Console.WriteLine("WARNING: Type ukOff2 != 0 (0x{0:X})", unknownOffset2);

                    if (unknownOffset3 != 0)
                        Console.WriteLine("WARNING: Type ukOff3 != 0 (0x{0:X})", unknownOffset3);

                    if (unknownOffset4 != 0)
                        Console.WriteLine("WARNING: Type ukOff4 != 0 (0x{0:X})", unknownOffset4);

                    // UnknownULong1Offset
                    if (unknownULong1Offset > 0)
                    {
                        reader.JumpTo(unknownULong1Offset, false);
                        type.UnknownULong1 = reader.ReadUInt64();
                    }

                    if (unknownOffset5 != 0)
                        Console.WriteLine("WARNING: Type ukOff5 != 0 (0x{0:X})", unknownOffset5);

                    // Strings
                    reader.JumpTo(typeNameOffset, false);
                    string typeName = reader.ReadNullTerminatedString();

                    reader.JumpTo(typeNamespaceOffset, false);
                    type.Namespace = reader.ReadNullTerminatedString();

                    entry.TypeName = typeName;
                    typeOffsets.Add(typeOffset, typeName);
                    Types.Add(typeName, type);
                }
            }

            // Layouts
            var layoutOffsets = new List<long>();
            foreach (var node in Nodes)
            {
                foreach (var entry in node.Entries)
                {
                    if (!layoutOffsets.Contains(entry.LayoutOffset.Value))
                    {
                        var layout = new Layout();
                        reader.JumpTo(entry.LayoutOffset.Value, false);

                        long layoutNameOffset = reader.ReadInt64();
                        long unknownData1Offset = reader.ReadInt64();
                        long unknownData2Offset = reader.ReadInt64();
                        long unknownData3Offset = reader.ReadInt64();
                        long unknownData4Offset = reader.ReadInt64();
                        long unknownData5Offset = reader.ReadInt64();
                        long unknownData6Offset = reader.ReadInt64();
                        long unknownData7Offset = reader.ReadInt64();
                        layout.UnknownData8 = reader.ReadInt64(); // Always 0?

                        if (layout.UnknownData8 != 0)
                        {
                            Console.WriteLine(
                                "WARNING: Layout UnknownData8 != 0! ({0:X})",
                                layout.UnknownData8);
                        }

                        // Unknown Data 1
                        if (unknownData1Offset > 0)
                        {
                            reader.JumpTo(unknownData1Offset, false);
                            layout.UnknownData1 = reader.ReadInt32(); // Always 1?
                        }

                        // Unknown Data 2
                        if (unknownData2Offset > 0)
                        {
                            reader.JumpTo(unknownData2Offset, false);
                            layout.UnknownData2 = reader.ReadSingle();
                            uint padding = reader.ReadUInt32();

                            if (padding != 0)
                            {
                                Console.WriteLine(
                                    "WARNING: Layout UnknownData2 Padding != 0! ({0:X})",
                                    padding);
                            }
                        }

                        // Unknown Data 3
                        if (unknownData3Offset > 0)
                        {
                            reader.JumpTo(unknownData3Offset, false);
                            layout.UnknownData3 = reader.ReadSingle();
                            uint padding = reader.ReadUInt32();

                            if (padding != 0)
                            {
                                Console.WriteLine(
                                    "WARNING: Layout UnknownData3 Padding != 0! ({0:X})",
                                    padding);
                            }
                        }

                        // Unknown Data 4
                        if (unknownData4Offset > 0)
                        {
                            reader.JumpTo(unknownData4Offset, false);
                            layout.UnknownData4 = reader.ReadInt32(); // Always 0?
                        }

                        // Unknown Data 5
                        if (unknownData5Offset > 0)
                        {
                            reader.JumpTo(unknownData5Offset, false);
                            layout.UnknownData5 = reader.ReadInt32();
                        }

                        // Unknown Data 6
                        if (unknownData6Offset > 0)
                        {
                            reader.JumpTo(unknownData6Offset, false);
                            layout.UnknownData6 = reader.ReadInt32(); // Always 1?
                        }

                        // Unknown Data 7
                        if (unknownData7Offset > 0)
                        {
                            reader.JumpTo(unknownData7Offset, false);
                            layout.UnknownData7 = reader.ReadInt32(); // Always 2?
                        }

                        // Layout Name
                        reader.JumpTo(layoutNameOffset, false);
                        layout.Name = reader.ReadNullTerminatedString();

                        layout.Offset = entry.LayoutOffset;
                        entry.LayoutIndex = layoutOffsets.Count;
                        layoutOffsets.Add(entry.LayoutOffset.Value);
                        Layouts.Add(layout);
                    }
                    else
                    {
                        entry.LayoutIndex = layoutOffsets.IndexOf(
                            entry.LayoutOffset.Value);
                    }
                }
            }

            // Names
            foreach (var node in Nodes)
            {
                reader.JumpTo(node.NameOffset, false);
                node.Name = reader.ReadNullTerminatedString();

                for (int i = 0; i < node.Entries.Count; ++i)
                {
                    var entry = node.Entries[i];
                    reader.JumpTo(entry.NameOffset.Value, false);
                    entry.Name = reader.ReadNullTerminatedString();
                }
            }
        }

        public override void Save(Stream fileStream)
        {
            // BINA Header
            var writer = new BINAWriter(fileStream, BINA.BINATypes.Version2, false);
            var rand = new Random();
            Header.Version = 210;

            // Header
            writer.Write((byte)3); // TODO: Figure out what this is lol
            writer.Write((byte)Nodes.Count);
            writer.Write((byte)0);
            writer.Write((byte)0);

            writer.Write(0);
            writer.AddOffset("nodesOffset", 8);

            // Nodes
            writer.FillInOffsetLong("nodesOffset", false, false);
            for (int i = 0; i < Nodes.Count; ++i)
            {
                var node = Nodes[i];
                writer.AddString($"nodeNameOffset{i}", node.Name, 8);
                writer.Write((ulong)node.Entries.Count);
                writer.AddOffset($"nodeEntriesOffset{i}", 8);
            }

            // Entries
            for (int i = 0; i < Nodes.Count; ++i)
            {
                var node = Nodes[i];
                writer.FillInOffsetLong($"nodeEntriesOffset{i}", false, false);

                for (int i2 = 0; i2 < node.Entries.Count; ++i2)
                {
                    var entry = node.Entries[i2];
                    if (!entry.UUID.HasValue)
                        entry.UUID = (ulong)rand.Next();

                    writer.Write(entry.UUID.Value);
                    writer.AddString($"entryNameOffset{i}{i2}", entry.Name, 8);
                    writer.AddOffset($"secondEntryOffset{i}{i2}", 8);
                    writer.AddOffset($"dataOffset{i}{i2}", 8);
                }
            }

            // Data
            for (int i = 0; i < Nodes.Count; ++i)
            {
                var node = Nodes[i];
                for (int i2 = 0; i2 < node.Entries.Count; ++i2)
                {
                    var entry = node.Entries[i2];
                    writer.FillInOffsetLong($"dataOffset{i}{i2}", false, false);

                    //var chArr = entry.Data.ToCharArray();
                    var sb = new StringBuilder(entry.Data);
                    sb.Replace(NullReplaceChar, '\0');
                    sb.Replace("\r\n", "\n");
                    var bytes = Encoding.Unicode.GetBytes(sb.ToString());

                    writer.Write(bytes);
                    writer.Write((ushort)0);

                    writer.FixPadding(8);
                }
            }

            // Second Entries
            for (int i = 0; i < Nodes.Count; ++i)
            {
                var node = Nodes[i];
                for (int i2 = 0; i2 < node.Entries.Count; ++i2)
                {
                    var entry = node.Entries[i2];
                    writer.FillInOffsetLong($"secondEntryOffset{i}{i2}", false, false);
                    writer.AddString($"secondEntryNameOffset{i}{i2}", entry.Name, 8);

                    if (!string.IsNullOrEmpty(entry.TypeName))
                        writer.AddOffset($"typeOffset{i}{i2}", 8);
                    else
                        writer.Write(0UL);

                    writer.AddOffset($"layoutOffset{i}{i2}", 8);
                }
            }

            // Types
            foreach (var type in Types)
            {
                // Fill-in Second Entry Type Offset
                var t = type.Value;
                for (int i = 0; i < Nodes.Count; ++i)
                {
                    var node = Nodes[i];
                    for (int i2 = 0; i2 < node.Entries.Count; ++i2)
                    {
                        var entry = node.Entries[i2];
                        if (string.IsNullOrEmpty(entry.TypeName) || entry.TypeName != type.Key)
                            continue;

                        writer.FillInOffsetLong($"typeOffset{i}{i2}", false, false);
                    }
                }

                // Offsets
                if (!string.IsNullOrEmpty(type.Key))
                    writer.AddString($"typeName{type.Key}", type.Key, 8);
                else
                    writer.Write(0UL);

                if (!string.IsNullOrEmpty(t.Namespace))
                    writer.AddString($"typeNamespace{type.Key}", t.Namespace, 8);
                else
                    writer.Write(0UL);

                WriteOptOffset($"typeUnknownFloat1Offset{type.Key}", t.UnknownFloat1);
                WriteOptOffset($"typeUnknownFloat2Offset{type.Key}", t.UnknownFloat2);
                WriteOptOffset($"typeUnknownFloat3Offset{type.Key}", t.UnknownFloat3);
                WriteOptOffset($"typeUnknownInt1Offset{type.Key}", t.UnknownInt1);
                WriteOptOffset($"typeUnknownInt2Offset{type.Key}", t.UnknownInt2);
                writer.Write(0UL); // unknownOffset1

                WriteOptOffset($"typeUnknownULong2Offset{type.Key}", t.UnknownULong2);
                writer.Write(0UL); // unknownOffset2
                writer.Write(0UL); // unknownOffset3
                writer.Write(0UL); // unknownOffset4

                WriteOptOffset($"typeUnknownULong1Offset{type.Key}", t.UnknownULong1);
                writer.Write(0UL); // unknownOffset5

                // UnknownFloat1
                if (t.UnknownFloat1.HasValue)
                {
                    writer.FillInOffsetLong($"typeUnknownFloat1Offset{type.Key}", false, false);
                    writer.Write(t.UnknownFloat1.Value);
                    writer.Write(0);
                }

                // UnknownFloat2
                if (t.UnknownFloat2.HasValue)
                {
                    writer.FillInOffsetLong($"typeUnknownFloat2Offset{type.Key}", false, false);
                    writer.Write(t.UnknownFloat2.Value);
                    writer.Write(0);
                }

                // UnknownFloat3
                if (t.UnknownFloat3.HasValue)
                {
                    writer.FillInOffsetLong($"typeUnknownFloat3Offset{type.Key}", false, false);
                    writer.Write(t.UnknownFloat3.Value);
                    writer.Write(0);
                }

                // UnknownInt1
                if (t.UnknownInt1.HasValue)
                {
                    writer.FillInOffsetLong($"typeUnknownInt1Offset{type.Key}", false, false);
                    writer.Write(t.UnknownInt1.Value);
                    writer.Write(0);
                }

                // UnknownInt2
                if (t.UnknownInt2.HasValue)
                {
                    writer.FillInOffsetLong($"typeUnknownInt2Offset{type.Key}", false, false);
                    writer.Write(t.UnknownInt2.Value);
                    writer.Write(0);
                }

                // UnknownULong1
                if (t.UnknownULong1.HasValue)
                {
                    writer.FillInOffsetLong($"typeUnknownULong1Offset{type.Key}", false, false);
                    writer.Write(t.UnknownULong1.Value);
                }

                // UnknownULong2
                if (t.UnknownULong2.HasValue)
                {
                    writer.FillInOffsetLong($"typeUnknownULong2Offset{type.Key}", false, false);
                    writer.Write(t.UnknownULong2.Value);
                }
            }

            // Layouts
            for (int i = 0; i < Layouts.Count; ++i)
            {
                writer.FixPadding(8);

                // Fill-In Entry Offsets
                var cat = Layouts[i];
                for (int nodeIndex = 0; nodeIndex < Nodes.Count; ++nodeIndex)
                {
                    var node = Nodes[nodeIndex];
                    for (int i2 = 0; i2 < node.Entries.Count; ++i2)
                    {
                        var entry = node.Entries[i2];
                        if (entry.LayoutIndex != i)
                            continue;

                        writer.FillInOffsetLong($"layoutOffset{nodeIndex}{i2}", false, false);
                    }
                }

                // Write Layout
                if (!string.IsNullOrEmpty(cat.Name))
                    writer.AddString($"layoutName{i}", cat.Name, 8);
                else
                    writer.Write(0UL);

                WriteOptOffset($"catUnknownData1{i}", cat.UnknownData1);
                WriteOptOffset($"catUnknownData2{i}", cat.UnknownData2);
                WriteOptOffset($"catUnknownData3{i}", cat.UnknownData3);
                WriteOptOffset($"catUnknownData4{i}", cat.UnknownData4);
                WriteOptOffset($"catUnknownData5{i}", cat.UnknownData5);
                WriteOptOffset($"catUnknownData6{i}", cat.UnknownData6);
                WriteOptOffset($"catUnknownData7{i}", cat.UnknownData7);
                writer.Write(cat.UnknownData8);

                // UnknownData1
                if (cat.UnknownData1.HasValue)
                {
                    writer.FixPadding(8);
                    writer.FillInOffsetLong($"catUnknownData1{i}", false, false);
                    writer.Write(cat.UnknownData1.Value);
                }

                // UnknownData2
                if (cat.UnknownData2.HasValue)
                {
                    writer.FixPadding(8);
                    writer.FillInOffsetLong($"catUnknownData2{i}", false, false);
                    writer.Write(cat.UnknownData2.Value);
                }

                // UnknownData3
                if (cat.UnknownData3.HasValue)
                {
                    writer.FixPadding(8);
                    writer.FillInOffsetLong($"catUnknownData3{i}", false, false);
                    writer.Write(cat.UnknownData3.Value);
                }

                // UnknownData4
                if (cat.UnknownData4.HasValue)
                {
                    writer.FixPadding(8);
                    writer.FillInOffsetLong($"catUnknownData4{i}", false, false);
                    writer.Write(cat.UnknownData4.Value);
                }

                // UnknownData5
                if (cat.UnknownData5.HasValue)
                {
                    writer.FixPadding(8);
                    writer.FillInOffsetLong($"catUnknownData5{i}", false, false);
                    writer.Write(cat.UnknownData5.Value);
                }

                // UnknownData6
                if (cat.UnknownData6.HasValue)
                {
                    writer.FixPadding(8);
                    writer.FillInOffsetLong($"catUnknownData6{i}", false, false);
                    writer.Write(cat.UnknownData6.Value);
                }

                // UnknownData7
                if (cat.UnknownData7.HasValue)
                {
                    writer.FixPadding(8);
                    writer.FillInOffsetLong($"catUnknownData7{i}", false, false);
                    writer.Write(cat.UnknownData7.Value);
                }
            }

            // Footer
            writer.FinishWrite(Header);

            // Sub-Methods
            void WriteOptOffset<T>(string offsetName, T? value)
                where T: struct
            {
                if (value.HasValue)
                    writer.AddOffset(offsetName, 8);
                else
                    writer.Write(0UL);
            }
        }

        public void ImportXML(string filePath)
        {
            using (var fs = File.OpenRead(filePath))
            {
                ImportXML(fs);
            }
        }

        public void ExportXML(string filePath)
        {
            using (var fs = File.Create(filePath))
            {
                ExportXML(fs);
            }
        }

        public void ImportXML(Stream fileStream)
        {
            // Layouts
            var xml = XDocument.Load(fileStream);
            var layoutElems = xml.Root.Element("Layouts");

            var cats = new Dictionary<string, int>();
            foreach (var elem in layoutElems.Elements())
            {
                var ukData8Attr = elem.Attribute("unknownData8");
                long.TryParse(ukData8Attr?.Value, out var ukData8);

                cats.Add(elem.Name.LocalName, Layouts.Count);
                Layouts.Add(new Layout()
                {
                    Name = elem.Name.LocalName,
                    UnknownData1 = GetOptValue<int>(elem, "UnknownData1"),
                    UnknownData2 = GetOptValue<float>(elem, "UnknownData2"),
                    UnknownData3 = GetOptValue<float>(elem, "UnknownData3"),
                    UnknownData4 = GetOptValue<int>(elem, "UnknownData4"),
                    UnknownData5 = GetOptValue<int>(elem, "UnknownData5"),
                    UnknownData6 = GetOptValue<int>(elem, "UnknownData6"),
                    UnknownData7 = GetOptValue<int>(elem, "UnknownData7"),
                    UnknownData8 = ukData8
                });
            }

            // Types
            var typeElems = xml.Root.Element("Types");
            foreach (var elem in typeElems.Elements())
            {
                var nmSpace = elem.Attribute("namespace");
                Types.Add(elem.Name.LocalName, new EntryType()
                {
                    Namespace = nmSpace?.Value,
                    UnknownFloat1 = GetOptValue<float>(elem, "UnknownFloat1"),
                    UnknownFloat2 = GetOptValue<float>(elem, "UnknownFloat2"),
                    UnknownFloat3 = GetOptValue<float>(elem, "UnknownFloat3"),
                    UnknownInt1 = GetOptValue<int>(elem, "UnknownInt1"),
                    UnknownInt2 = GetOptValue<int>(elem, "UnknownInt2"),
                    UnknownULong1 = GetOptValue<ulong>(elem, "UnknownULong1"),
                    UnknownULong2 = GetOptValue<ulong>(elem, "UnknownULong2")
                });
            }

            // Nodes
            var nodeElems = xml.Root.Elements("Nodes");
            foreach (var nodeElem in nodeElems.Elements())
            {
                var node = new Node()
                {
                    Name = nodeElem.Name.LocalName,
                };

                foreach (var elem in nodeElem.Elements())
                {
                    var uuidAttr = elem.Attribute("uuid");
                    var typeAttr = elem.Attribute("type");
                    var layoutAttr = elem.Attribute("layout");

                    if (typeAttr == null || layoutAttr == null)
                    {
                        Console.WriteLine(
                            "WARNING: Skipped Node Entry because it has no type/layout!");

                        continue;
                    }

                    // Get UUID
                    bool hasUUID = true;
                    if (!ulong.TryParse(uuidAttr?.Value, out var uuid))
                        hasUUID = false;

                    // Get Data
                    var sb = new StringBuilder();
                    foreach (var line in elem.Elements())
                    {
                        sb.Append(line.Value.Replace(NullReplaceChar, '\0'));
                        sb.Append('\n');
                    }

                    // Generate Entry
                    node.Entries.Add(new Entry()
                    {
                        Name = elem.Name.LocalName,
                        Data = sb.ToString(),
                        TypeName = typeAttr.Value,
                        LayoutIndex = cats[layoutAttr.Value],
                        UUID = (hasUUID) ? (ulong?)uuid : null
                    });
                }

                Nodes.Add(node);
            }

            // Sub-Methods
            T? GetOptValue<T>(XElement e, string n) where T: struct
            {
                var elem = e.Element(n);
                if (elem == null)
                    return null;

                return (T?)Helpers.ChangeType(elem.Value, typeof(T));
            }
        }

        public void ExportXML(Stream fileStream)
        {
            var root = new XElement("Text");
            var layoutNames = new List<string>();

            // Layouts
            var layoutElems = new XElement("Layouts");
            foreach (var layout in Layouts)
            {
                var elem = new XElement(layout.Name);
                AddOptElem("UnknownData1", elem, layout.UnknownData1);
                AddOptElem("UnknownData2", elem, layout.UnknownData2);
                AddOptElem("UnknownData3", elem, layout.UnknownData3);
                AddOptElem("UnknownData4", elem, layout.UnknownData4);
                AddOptElem("UnknownData5", elem, layout.UnknownData5);
                AddOptElem("UnknownData6", elem, layout.UnknownData6);
                AddOptElem("UnknownData7", elem, layout.UnknownData7);

                elem.Add(new XAttribute("unknownData8", layout.UnknownData8));
                layoutNames.Add(layout.Name);
                layoutElems.Add(elem);
            }

            // Types
            var typeElems = new XElement("Types");
            foreach (var type in Types)
            {
                var typeElem = new XElement(type.Key, new XAttribute(
                    "namespace", type.Value.Namespace));

                AddOptElem("UnknownFloat1", typeElem, type.Value.UnknownFloat1);
                AddOptElem("UnknownFloat2", typeElem, type.Value.UnknownFloat2);
                AddOptElem("UnknownFloat3", typeElem, type.Value.UnknownFloat3);
                AddOptElem("UnknownInt1", typeElem, type.Value.UnknownInt1);
                AddOptElem("UnknownInt2", typeElem, type.Value.UnknownInt2);
                AddOptElem("UnknownULong1", typeElem, type.Value.UnknownULong1);
                AddOptElem("UnknownULong2", typeElem, type.Value.UnknownULong2);

                typeElems.Add(typeElem);
            }

            // Nodes
            var nodeElems = new XElement("Nodes");
            foreach (var node in Nodes)
            {
                var nodeElem = new XElement(node.Name);
                foreach (var entry in node.Entries)
                {
                    // Separate the data into different elements per-line.
                    var elem = new XElement(entry.Name);
                    string data = entry.Data.Replace('\0', NullReplaceChar);
                    using (var reader = new StringReader(data))
                    {
                        string line = string.Empty;
                        do
                        {
                            line = reader.ReadLine();
                            if (line != null)
                            {
                                elem.Add(new XElement("Line", line));
                            }
                        }
                        while (line != null);
                    }

                    // Write Element
                    elem.Add(new XAttribute("uuid", entry.UUID));
                    elem.Add(new XAttribute("type", entry.TypeName));
                    elem.Add(new XAttribute("layout",
                        layoutNames[entry.LayoutIndex]));

                    nodeElem.Add(elem);
                }

                nodeElems.Add(nodeElem);
            }

            root.Add(layoutElems);
            root.Add(typeElems);
            root.Add(nodeElems);

            // Write the generated XML File
            var xml = new XDocument(root);
            xml.Save(fileStream);

            // Sub-Methods
            void AddOptElem<T>(string n, XElement e, T? v)
                where T : struct
            {
                if (v.HasValue)
                    e.Add(new XElement(n, v.Value));
            }
        }

        // Other
        public class Node
        {
            public List<Entry> Entries = new List<Entry>();
            public string Name;

            public long NameOffset, EntriesOffset;
            public ulong EntriesCount;
        }

        public class Entry
        {
            public string Name, Data, TypeName;
            public ulong? UUID;
            public long? NameOffset, SecondEntryOffset,
                DataOffset, LayoutOffset;
            public int LayoutIndex;
        }

        public class EntryType
        {
            public string Namespace;
            public ulong? UnknownULong1, UnknownULong2;
            public float? UnknownFloat1, UnknownFloat2, UnknownFloat3;
            public int? UnknownInt1, UnknownInt2;
        }

        public class Layout
        {
            public string Name;
            public long UnknownData8;
            public long? Offset;
            public float? UnknownData2, UnknownData3;
            public int? UnknownData1, UnknownData4,
                UnknownData5, UnknownData6, UnknownData7;
        }
    }
}