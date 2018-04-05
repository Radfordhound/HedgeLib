using HedgeLib.IO;
using System.IO;
using System.Collections.Generic;
using System;
using System.Linq;
using HedgeLib.Exceptions;

namespace HedgeLib.Archives
{
    // HUGELY based off of Skyth's Forces PAC specification (thanks bb lol)
    public class ForcesArchive : Archive
    {
        // Variables/Constants
        public uint? ID;
        public const string Extension = ".pac", Signature = "PACx";

        // Huge thanks to Skyth for these lists as well!
        protected Dictionary<string, string> Types = new Dictionary<string, string>()
        {
            { ".asm", "ResAnimator" },
            { ".anm.hkx", "ResAnimSkeleton" },
            {".uv-anim", "ResAnimTexSrt"},
            {".material", "ResMirageMaterial"},
            {".model", "ResModel"},
            {".rfl", "ResReflection"},
            {".skl.hkx", "ResSkeleton"},
            {".dds", "ResTexture"},
            {".cemt", "ResCyanEffect"},
            {".cam-anim", "ResAnimCameraContainer"},
            {".effdb", "ResParticleLocation"},
            {".mat-anim", "ResAnimMaterial"},
            {".phy.hkx", "ResHavokMesh"},
            {".vis-anim", "ResAnimVis"},
            {".scfnt", "ResScalableFontSet"},
            {".pt-anim", "ResAnimTexPat"},
            {".scene", "ResScene"},
            {".pso", "ResMiragePixelShader"},
            {".vso", "ResMirageVertexShader"},
            {".shader-list", "ResShaderList"},
            {".vib", "ResVibration"},
            {".bfnt", "ResBitmapFont"},
            {".codetbl", "ResCodeTable"},
            {".cnvrs-text", "ResText"},
            {".cnvrs-meta", "ResTextMeta"},
            {".cnvrs-proj", "ResTextProject"},
            {".shlf", "ResSHLightField"},
            {".swif", "ResSurfRideProject"},
            {".gedit", "ResObjectWorld"},
            {".fxcol.bin", "ResFxColFile"},
            {".path", "ResSplinePath"},
            {".lit-anim", "ResAnimLightContainer"},
            {".gism", "ResGismoConfig"},
            {".light", "ResMirageLight"},
            {".probe", "ResProbe"},
            {".svcol.bin", "ResSvCol"},
            {".terrain-instanceinfo", "ResMirageTerrainInstanceInfo"},
            {".terrain-model", "ResMirageTerrainModel"},
            {".model-instanceinfo", "ResModelInstanceInfo"},
            {".grass.bin", "ResTerrainGrassInfo" }
        };

        protected string[] RootExclusiveTypes = new string[]
        {
            ".anm.hkx",
            ".cemt",
            ".phy.hkx",
            ".skl.hkx",
            ".rfl",
            ".bfnt",
            ".effdb",
            ".vib",
            ".scene",
            ".shlf",
            ".scfnt",
            ".codetbl",
            ".cnvrs-text",
            ".swif",
            ".fxcol.bin",
            ".path",
            ".gism",
            ".light",
            ".probe",
            ".svcol.bin",
            ".terrain-instanceinfo",
            ".model-instanceinfo",
            ".grass.bin",
            ".shader-list",
            ".gedit",
            ".cnvrs-meta",
            ".cnvrs-proj"
        };

        // Constructors
        public ForcesArchive() : base() { }
        public ForcesArchive(Archive arc) : base(arc) { }

        // Methods
        public override void Load(string filePath)
        {
            var splitArchiveList = GetSplitArchivesList(filePath);
            foreach (string splitArchive in splitArchiveList)
            {
                using (var fs = File.OpenRead(splitArchive))
                {
                    Load(fs);
                }
            }
        }

        public override List<string> GetSplitArchivesList(string filePath)
        {
            var fileInfo = new FileInfo(filePath);
            var splitArchivesList = new List<string>();

            string ext = fileInfo.Extension;
            if (int.TryParse(ext.Substring(1), out var e))
                ext = "";

            // fileInfo.Extension only gets the last extension in the fileName.
            // Therefore if the fileName is something like "Sonic.pac.000", this
            // will only remove the ".000" from the end.
            string shortName = fileInfo.Name.Substring(0,
                fileInfo.Name.Length - ext.Length);

            for (int i = 0; i <= 999; ++i)
            {
                if (i == 0 && !string.IsNullOrEmpty(ext))
                {
                    string fn = Path.Combine(
                        fileInfo.DirectoryName, $"{shortName}{ext}");

                    if (File.Exists(fn))
                        splitArchivesList.Add(fn);
                }

                string fileName = Path.Combine(fileInfo.DirectoryName,
                    $"{shortName}{ext}.{i.ToString("000")}");

                if (!File.Exists(fileName))
                    break;

                splitArchivesList.Add(fileName);
            }

            return splitArchivesList;
        }

        public override void Load(Stream fileStream)
        {
            // BINA Header
            var reader = new BINAReader(fileStream, BINA.BINATypes.Version2);
            string sig = reader.ReadSignature(4);
            if (sig != Signature)
                throw new InvalidSignatureException(Signature, sig);

            // Version String
            string verString = reader.ReadSignature(3);
            if (!ushort.TryParse(verString, out var version))
            {
                Console.WriteLine(
                    "WARNING: PACx header version was invalid {0}",
                    verString);
            }

            reader.Offset = 0;
            reader.IsBigEndian = (reader.ReadChar() == 'B');
            ID = reader.ReadUInt32();
            uint arcSize = reader.ReadUInt32();

            // PAC Header
            uint nodesSize = reader.ReadUInt32();
            uint pacsSize = reader.ReadUInt32();
            uint entriesSize = reader.ReadUInt32();
            uint stringsSize = reader.ReadUInt32();
            uint dataSize = reader.ReadUInt32();
            uint finalTableLength = reader.ReadUInt32();

            // 1 = HasNoSplits, 2 = IsSplit, 5 = HasSplits
            ushort pacSplitType = reader.ReadUInt16(); 

            ushort unknown1 = reader.ReadUInt16(); // Always 0x108?
            uint splitCount = reader.ReadUInt32();

            // Type Names
            var typeTree = ReadNodeTree(reader);
            var names = new string[typeTree.Nodes.Count];

            for (int i = 0; i < typeTree.Nodes.Count; ++i)
            {
                var typeNode = typeTree.Nodes[i];
                if (typeNode.ChildCount < 1)
                    continue;

                int nameIndex = -1;
                if (typeNode.ChildIDTableOffset > 0)
                {
                    reader.JumpTo(typeNode.ChildIDTableOffset);
                    nameIndex = reader.ReadInt32();
                }

                if (typeNode.NameOffset > 0)
                {
                    reader.JumpTo(typeNode.NameOffset);
                    names[i] = reader.ReadNullTerminatedString();
                }
            }

            // Types
            foreach (var type in typeTree.Nodes)
            {
                if (!type.HasData)
                    continue;

                string name = string.Empty;
                var n = type;

                while (n.ParentIndex >= 0)
                {
                    name = $"{names[n.ParentIndex]}{name}";
                    n = typeTree.Nodes[n.ParentIndex];
                }

                reader.JumpTo(type.DataOffset);
                var fileTree = ReadNodeTree(reader);

                // File Names
                var fileNames = new string[fileTree.Nodes.Count];
                for (int i = 0; i < fileTree.Nodes.Count; ++i)
                {
                    var fileNode = fileTree.Nodes[i];
                    if (fileNode.ChildCount < 1)
                        continue;

                    int nameIndex = -1;
                    if (fileNode.ChildIDTableOffset > 0)
                    {
                        reader.JumpTo(fileNode.ChildIDTableOffset);
                        nameIndex = reader.ReadInt32();
                    }

                    if (fileNode.NameOffset > 0)
                    {
                        reader.JumpTo(fileNode.NameOffset);
                        fileNames[i] = reader.ReadNullTerminatedString();
                    }
                }

                // File Nodes
                foreach (var file in fileTree.Nodes)
                {
                    if (!file.HasData)
                        continue;

                    name = string.Empty;
                    n = file;

                    while (n.ParentIndex >= 0)
                    {
                        name = $"{fileNames[n.ParentIndex]}{name}";
                        n = fileTree.Nodes[n.ParentIndex];
                    }

                    // File Entries
                    reader.JumpTo(file.DataOffset);
                    uint pacID = reader.ReadUInt32();
                    if (pacID != ID)
                    {
                        Console.WriteLine(
                            $"WARNING: Skipped file {name} as its pac ID was missing");
                        continue;
                    }

                    ulong fileSize = reader.ReadUInt64();
                    uint padding1 = reader.ReadUInt32();
                    long fileDataOffset = reader.ReadInt64();
                    ulong padding2 = reader.ReadUInt64();
                    long extensionOffset = reader.ReadInt64();
                    uint pacType = reader.ReadUInt32();
                    uint padding3 = reader.ReadUInt32();
                    
                    if (fileDataOffset <= 0 || pacType == 1)
                        continue;

                    // File Extension
                    reader.JumpTo(extensionOffset);
                    name += $".{reader.ReadNullTerminatedString()}";

                    // File Data
                    reader.JumpTo(fileDataOffset);
                    var data = reader.ReadBytes((int)fileSize);

                    // BINA Check
                    // TODO: Remove this check
                    if (data[0] == 0x42 && data[1] == 0x49 && data[2] == 0x4E && data[3] == 0x41)
                    {
                        if (pacType != 2)
                        {
                            Console.WriteLine(
                                $"WARNING: FileType ({pacType}) != 2 when file carries BINA Header!");
                        }
                    }
                    else
                    {
                        if (pacType == 2)
                        {
                            Console.WriteLine(
                                $"WARNING: FileType ({pacType}) == 2 when file has no BINA Header!");
                        }
                    }

                    Data.Add(new ArchiveFile(name, data));
                }
            }
        }

        protected NodeTree ReadNodeTree(BINAReader reader)
        {
            var nodeTree = new NodeTree();
            uint nodesCount = reader.ReadUInt32();
            nodeTree.DataNodeCount = reader.ReadUInt32();
            long nodesOffset = reader.ReadInt64();
            long dataNodeIndicesOffset = reader.ReadInt64();

            // Nodes
            reader.JumpTo(nodesOffset);
            for (uint i = 0; i < nodesCount; ++i)
            {
                nodeTree.Nodes.Add(new Node(reader));
            }

            return nodeTree;
        }

        public override void Save(string filePath, bool overwrite = false)
        {
            Save(filePath, 0xA00000);
        }

        public void Save(string filePath, uint? splitCount = 0xA00000)
        {
            var fileInfo = new FileInfo(filePath);
            string shortName = fileInfo.Name.Substring(0,
                fileInfo.Name.IndexOf('.'));

            // Generate Split Archives
            if (splitCount.HasValue)
            {
                // Save Split PACs
                var pacList = new List<string>();
                int startIndex = 0, arcIndex = 0;
                while (startIndex != -1)
                {
                    string fileName = Path.Combine(fileInfo.DirectoryName,
                        $"{shortName}{Extension}.{arcIndex.ToString("000")}");

                    using (var fileStream = File.Create(fileName))
                    {
                        startIndex = Save(fileStream, splitCount, startIndex);
                        pacList.Add($"{shortName}{Extension}.{arcIndex.ToString("000")}");
                        ++arcIndex;
                    }
                }

                // Save Root PAC
                using (var fileStream = File.Create(filePath))
                {
                    Save(fileStream, null, 0, pacList);
                }
            }

            // Generate archive without splits
            else
            {
                using (var fileStream = File.Create(filePath))
                {
                    Save(fileStream, null, 0);
                }
            }
        }

        public override void Save(Stream fileStream)
        {
            Save(fileStream, null, 0);
        }

        public int Save(Stream fileStream, uint? sizeLimit,
            int startIndex = 0, List<string> splitList = null)
        {
            uint size = 0;
            int endIndex = -1;
            bool isRootPAC = (!sizeLimit.HasValue && splitList != null);

            // BINA Header
            var writer = new BINAWriter(fileStream,
                BINA.BINATypes.Version2, false, false);

            writer.WriteNulls(0x30);
            writer.Offset = 0;

            if (!ID.HasValue)
            {
                var rand = new Random();
                ID = (uint)rand.Next(1, int.MaxValue);
            }

            // Generate Node Trees
            var typeTree = new NodeTree();
            typeTree.Nodes.Add(new Node()
            {
                ChildCount = 1,
            });

            typeTree.Nodes.Add(new Node()
            {
                Name = "Res",
                ParentIndex = 0,
            });

            var fileTrees = new Dictionary<string, NodeTree>();
            for (int i = startIndex; i < Data.Count; ++i)
            {
                var file = (Data[i] as ArchiveFile);
                if (file == null)
                    continue;

                string ext = file.Name.Substring(file.Name.IndexOf('.'));
                string shortName = file.Name.Substring(0,
                    file.Name.Length - ext.Length);

                if (!Types.ContainsKey(ext))
                {
                    Console.WriteLine(
                        "WARNING: Skipped {0} as its extension ({1}) is unsupported!",
                        file.Name, ext);
                    continue;
                }

                // Root-Exclusive Type Check
                ulong fileDataType = 0;
                if (isRootPAC)
                {
                    fileDataType = (RootExclusiveTypes.Contains(ext)) ?
                        0UL : 1UL;
                }

                // BINA Header Check
                if (file.Data[0] == 0x42 && file.Data[1] == 0x49 &&
                    file.Data[2] == 0x4E && file.Data[3] == 0x41)
                {
                    fileDataType = 2;
                }

                if (!isRootPAC && RootExclusiveTypes.Contains(ext))
                    continue;

                NodeTree fileTree;
                string pacType = Types[ext];

                // TODO: Do node name splitting more like the game does it
                if (!fileTrees.ContainsKey(ext))
                {
                    ++typeTree.Nodes[1].ChildCount;
                    ++typeTree.DataNodeCount;
                    typeTree.Nodes.Add(new Node()
                    {
                        FullPathSize = 3,
                        Name = pacType.Substring(3),
                        ParentIndex = 1,
                        ChildCount = 1
                    });

                    typeTree.Nodes.Add(new Node()
                    {
                        FullPathSize = (byte)pacType.Length,
                        ParentIndex = typeTree.Nodes.Count - 1,
                        DataOffset = -1,
                        HasData = true
                    });

                    fileTree = new NodeTree();
                    fileTree.Nodes.Add(new Node());

                    fileTrees.Add(ext, fileTree);
                }
                else
                {
                    fileTree = fileTrees[ext];
                }

                ++fileTree.Nodes[0].ChildCount;
                ++fileTree.DataNodeCount;
                fileTree.Nodes.Add(new Node()
                {
                    Name = shortName,
                    ParentIndex = 0,
                    ChildCount = 1
                });

                fileTree.Nodes.Add(new Node()
                {
                    ArchiveFileIndex = i,
                    DataOffset = -1,
                    ParentIndex = fileTree.Nodes.Count - 1,
                    FullPathSize = (byte)shortName.Length,
                    HasData = true,
                    DataType = fileDataType
                });

                // Split if you exceed the sizeLimit
                if (sizeLimit.HasValue && i < Data.Count - 1)
                {
                    // Not very accurate but close enough™
                    size += (uint)(0x40 + file.Data.Length);

                    if (size >= sizeLimit.Value)
                    {
                        endIndex = i + 1;
                        break;
                    }
                }
            }

            // Write Node Trees
            WriteNodeTree(writer, typeTree, "typeTree");

            int nodeTreeIndex = -1;
            foreach (var fileTree in fileTrees)
            {
                writer.FillInOffsetLong(
                    $"typeTreenodeDataOffset{++nodeTreeIndex}", true, false);

                WriteNodeTree(writer, fileTree.Value, Types[fileTree.Key]);
            }

            // Write Data Node Indices
            WriteTreeDataIndices(writer, typeTree, "typeTree");
            foreach (var fileTree in fileTrees)
            {
                WriteTreeDataIndices(writer, fileTree.Value, Types[fileTree.Key]);
            }

            // Write Child Node Indices
            for (int i = 0; i < typeTree.Nodes.Count; ++i)
            {
                var node = typeTree.Nodes[i];
                if (node.ChildCount < 1)
                    continue;

                writer.FillInOffsetLong(
                    $"typeTreenodeChildIDTableOffset{i}", true, false);

                for (int i2 = 0; i2 < node.ChildCount; ++i2)
                {
                    writer.Write(node.ChildIDs[i2]);
                }

                writer.FixPadding(8);
            }

            foreach (var fileTree in fileTrees)
            {
                var tree = fileTree.Value;
                for (int i = 0; i < tree.Nodes.Count; ++i)
                {
                    var node = tree.Nodes[i];
                    if (node.ChildCount < 1)
                        continue;

                    writer.FillInOffsetLong(
                        $"{Types[fileTree.Key]}nodeChildIDTableOffset{i}", true, false);

                    for (int i2 = 0; i2 < node.ChildCount; ++i2)
                    {
                        writer.Write(node.ChildIDs[i2]);
                    }

                    writer.FixPadding(8);
                }
            }

            // Write PACs section
            uint pacsSize = 0;
            if (isRootPAC)
            {
                writer.Write((ulong)splitList.Count);
                pacsSize += 8;

                for (int i = 0; i < splitList.Count; ++i)
                {
                    writer.AddOffset($"splitPACOffset{i}", 8);
                    writer.FillInOffsetLong($"splitPACOffset{i}", true, false);
                    writer.AddString($"splitPACName{i}", splitList[i], 8);
                    pacsSize += 16;
                }
            }

            // Write File Entries
            long fileEntriesOffset = fileStream.Position;
            foreach (var fileTree in fileTrees)
            {
                var tree = fileTree.Value;
                for (int i = 0; i < tree.DataNodeCount; ++i)
                {
                    var node = tree.Nodes[tree.DataNodeIndices[i]];
                    writer.FillInOffsetLong(
                        $"{Types[fileTree.Key]}nodeDataOffset{i}", true, false);

                    var file = Data[node.ArchiveFileIndex] as ArchiveFile;
                    writer.Write(ID.Value);
                    writer.Write((ulong)file.Data.Length);
                    writer.Write(0U);

                    if (node.DataType == 1)
                    {
                        writer.Write(0UL);
                    }
                    else
                    {
                        writer.AddOffset($"{Types[fileTree.Key]}fileDataOffset{i}", 8);
                    }

                    writer.Write(0UL);
                    writer.AddString($"{Types[fileTree.Key]}extDataOffset{i}",
                        fileTree.Key.Substring(1), 8);
                    writer.Write(node.DataType);
                }
            }

            // Write String Table
            uint stringTablePos = writer.WriteStringTable();
            writer.FixPadding(8);

            // Write File Data
            long fileDataOffset = fileStream.Position;
            foreach (var fileTree in fileTrees)
            {
                var tree = fileTree.Value;
                for (int i = 0; i < tree.DataNodeCount; ++i)
                {
                    var node = tree.Nodes[tree.DataNodeIndices[i]];
                    if (node.DataType == 1)
                        continue;

                    writer.FixPadding(16);
                    writer.FillInOffsetLong(
                        $"{Types[fileTree.Key]}fileDataOffset{i}", true, false);

                    var file = Data[node.ArchiveFileIndex] as ArchiveFile;
                    writer.Write(file.Data);
                }
            }

            // Write Offset Table
            writer.FixPadding(8);
            uint footerPos = writer.WriteFooter();
            writer.FixPadding(8);

            // Fill-In Header
            uint fileSize = (uint)fileStream.Position;
            writer.BaseStream.Position = 0;

            writer.WriteSignature(Signature);
            writer.WriteSignature("301");
            writer.Write((writer.IsBigEndian) ? 'B' : 'L');
            writer.Write(fileSize);

            writer.Write((uint)(fileEntriesOffset - 0x30) - pacsSize);
            writer.Write(pacsSize);
            writer.Write(stringTablePos - (uint)fileEntriesOffset);
            writer.Write((uint)fileDataOffset - stringTablePos);
            writer.Write(footerPos - (uint)fileDataOffset);
            writer.Write(fileSize - footerPos);

            // 5 if there are splits but this is the root, 2 if this is a split, 1 if no splits
            writer.Write((sizeLimit.HasValue) ? (ushort)2 :
                (splitList != null) ? (ushort)5 : (ushort)1);

            writer.Write((ushort)0x108);
            writer.Write((splitList == null) ? 0U : (uint)splitList.Count);

            return endIndex;
        }

        protected void WriteNodeTree(BINAWriter writer,
            NodeTree tree, string prefix)
        {
            writer.Write(tree.Nodes.Count);
            writer.Write(tree.DataNodeCount);
            writer.AddOffset($"{prefix}nodesOffset", 8);
            writer.AddOffset($"{prefix}dataNodeIndicesOffset", 8);

            // Nodes
            int dataIndex = -1;
            tree.DataNodeIndices = new List<int>();
            writer.FillInOffsetLong($"{prefix}nodesOffset", true, false);

            for (int i = 0; i < tree.Nodes.Count; ++i)
            {
                // Name
                var node = tree.Nodes[i];
                if (!string.IsNullOrEmpty(node.Name))
                {
                    writer.AddString($"{prefix}nodeNameOffset{i}", node.Name, 8);
                }
                else
                {
                    writer.Write(0UL);
                }

                // Data
                if (node.HasData)
                {
                    node.DataIndex = ++dataIndex;
                    writer.AddOffset($"{prefix}nodeDataOffset{dataIndex}", 8);
                    tree.DataNodeIndices.Add(i);
                }
                else
                {
                    writer.Write(0UL);
                }

                // Child Index Table Offset
                if (node.ChildCount > 0)
                {
                    writer.AddOffset($"{prefix}nodeChildIDTableOffset{i}", 8);
                }
                else
                {
                    writer.Write(0UL);
                }

                // Populate Child IDs
                if (node.ParentIndex >= 0)
                {
                    tree.Nodes[node.ParentIndex].ChildIDs.Add(i);
                }

                // Indices
                writer.Write(node.ParentIndex);
                writer.Write(i); // Global Index
                writer.Write(node.DataIndex);

                // Other
                writer.Write(node.ChildCount);
                writer.Write((byte)0);
                writer.Write(node.HasData);
                writer.Write(node.FullPathSize);
            }
        }

        protected void WriteTreeDataIndices(
            BINAWriter writer, NodeTree tree, string prefix)
        {
            // Data Node Indices
            writer.FillInOffsetLong($"{prefix}dataNodeIndicesOffset", true, false);
            for (int i = 0; i < tree.DataNodeIndices.Count; ++i)
            {
                writer.Write(tree.DataNodeIndices[i]);
            }

            writer.FixPadding(8);
        }

        // Other
        protected class NodeTree
        {
            public List<Node> Nodes = new List<Node>();
            public List<int> DataNodeIndices = new List<int>();
            public uint DataNodeCount = 0;
        }

        protected class Node
        {
            // Variables/Constants
            public List<int> ChildIDs = new List<int>();
            public string Name = string.Empty;

            public ulong DataType = 0;
            public long NameOffset = 0, DataOffset = 0, ChildIDTableOffset = 0;
            public int ParentIndex = -1, GlobalIndex, DataIndex = -1, ArchiveFileIndex = -1;
            public byte ChildCount = 0, FullPathSize = 0;
            public bool HasData = false;

            // Constructors
            public Node() { }
            public Node(ExtendedBinaryReader reader)
            {
                Read(reader);
            }

            // Methods
            public void Read(ExtendedBinaryReader reader)
            {
                NameOffset = reader.ReadInt64();
                DataOffset = reader.ReadInt64();
                ChildIDTableOffset = reader.ReadInt64();

                ParentIndex = reader.ReadInt32();
                GlobalIndex = reader.ReadInt32();
                DataIndex = reader.ReadInt32();

                ChildCount = reader.ReadByte();
                byte padding1 = reader.ReadByte();
                HasData = reader.ReadBoolean();
                FullPathSize = reader.ReadByte();  // Not counting this node in.

                // Padding Checks
                if (padding1 != 0)
                    Console.WriteLine($"WARNING: Padding1 != 0 ({padding1})");
            }
        }
    }
}