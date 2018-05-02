using HedgeLib.Headers;
using HedgeLib.IO;
using System;
using System.Collections.Generic;
using System.IO;

namespace HedgeLib.Models
{
    public class GensModel : Model
    {
        // Variables/Constants
        public HedgehogEngineHeader Header = new GensHeader();
        public const string Extension = ".model", TerrainExtension = ".terrain-model",
            ModelMirageType = "Model";

        public const int SubMeshSlotCount = 4;
        public const uint NextGenSignature = 0x133054A, PS3RootType = 0x3ED;

        // Methods
        public override void Load(string filePath)
        {
            Load(filePath, 1);
        }

        public virtual void Load(string filePath, float scale)
        {
            string ext = Path.GetExtension(filePath);
            Load(filePath, (ext == TerrainExtension), scale);
        }

        public virtual void Load(string filePath, bool isTerrain, float scale = 1)
        {
            // Throw exceptions if necessary
            if (string.IsNullOrEmpty(filePath))
                throw new ArgumentNullException("filePath");

            if (!File.Exists(filePath))
            {
                throw new FileNotFoundException(
                    "The given file could not be loaded, as it cannot be found!", filePath);
            }

            // Load the file
            using (var fileStream = File.OpenRead(filePath))
            {
                Load(fileStream, isTerrain);
            }
        }

        public override void Load(Stream fileStream)
        {
            Load(fileStream, false);
        }

        public virtual void Load(Stream fileStream, bool isTerrain, float scale = 1)
        {
            // Header
            var reader = new GensReader(fileStream);
            Header = reader.ReadHeader();

            // Data
            uint meshCount = reader.ReadUInt32();
            uint meshOffsetsOffset = reader.ReadUInt32();

            if (isTerrain)
            {
                uint modelNameOffset = reader.ReadUInt32(); // TODO: Use this
                uint modelFlag = reader.ReadUInt32(); // TODO: Use this
            }
            else
            {
                // TODO: Read the skeleton
            }

            // Mesh Offsets
            var meshOffsets = new uint[meshCount];
            reader.JumpTo(meshOffsetsOffset, false);

            for (uint i = 0; i < meshCount; ++i)
            {
                meshOffsets[i] = reader.ReadUInt32();
            }

            // Meshes
            for (uint i = 0; i < meshCount; ++i)
            {
                reader.JumpTo(meshOffsets[i], false);
                ReadMesh(reader, scale);
            }
        }

        protected void ReadMesh(GensReader reader, float scale = 1)
        {
            // SubMesh Slots
            uint curPos = (uint)reader.BaseStream.Position;
            for (int i = 0; i < SubMeshSlotCount;)
            {
                uint subMeshCount = reader.ReadUInt32();
                uint subMeshOffsetsOffset = reader.ReadUInt32();

                // TODO: Add support for water SubMesh
                if (i == SubMeshSlotCount - 1) break;

                // SubMesh Offsets
                var subMeshOffsets = new uint[subMeshCount];
                reader.JumpTo(subMeshOffsetsOffset, false);

                for (uint i2 = 0; i2 < subMeshCount; ++i2)
                {
                    subMeshOffsets[i2] = reader.ReadUInt32();
                }

                // SubMeshes
                for (uint i2 = 0; i2 < subMeshCount; ++i2)
                {
                    reader.JumpTo(subMeshOffsets[i2], false);
                    var m = ReadSubMesh(reader, scale);
                    m.Slot = (Mesh.Slots)i;
                }

                reader.JumpTo(curPos + (++i * 8));
            }
        }

        protected unsafe Mesh ReadSubMesh(GensReader reader, float scale = 1)
        {
            uint offset; // Generic uint reused for different data

            // Offsets
            uint materialNameOffset, faceCount, faceOffset, vertexCount,
                vertexSize, vertexOffset, vertexFormatOffset, boneCount,
                boneOffset, textureUnitCount, textureUnitOffsetsOffset;

            if (Header.RootNodeType == PS3RootType)
            {
                uint unknownOffset1 = reader.ReadUInt32(); // VertexFormat?
                materialNameOffset = reader.ReadUInt32();
                boneCount = reader.ReadUInt32();
                boneOffset = reader.ReadUInt32(); // ?
                textureUnitCount = reader.ReadUInt32();
                textureUnitOffsetsOffset = reader.ReadUInt32();

                // Possibly vertex format?
                reader.JumpTo(unknownOffset1, false);
                int unknown1 = reader.ReadInt32();
                int unknown2 = reader.ReadInt32();
                int unknown3 = reader.ReadInt32();
                int unknown4 = reader.ReadInt32();

                uint unknownOffset2 = reader.ReadUInt32();
                uint unknownCount1 = reader.ReadUInt32();
                vertexOffset = reader.ReadUInt32();

                // TODO
                throw new NotImplementedException(
                    "ERROR: Cannot yet read PS3 terrain-models");
            }
            else
            {
                materialNameOffset = reader.ReadUInt32();
                faceCount = reader.ReadUInt32();
                faceOffset = reader.ReadUInt32();
                vertexCount = reader.ReadUInt32();
                vertexSize = reader.ReadUInt32();
                vertexOffset = reader.ReadUInt32();
                vertexFormatOffset = reader.ReadUInt32();
                boneCount = reader.ReadUInt32();
                boneOffset = reader.ReadUInt32();
                textureUnitCount = reader.ReadUInt32();
                textureUnitOffsetsOffset = reader.ReadUInt32();
            }

            // Faces
            var faces = new ushort[faceCount];
            reader.JumpTo(faceOffset, false);

            // Convert faces from triangle strips
            // Basically taken from LibGens because I honestly
            // don't think there's another way to do it lol
            int newStrip = 3, newIndex = 0;
            ushort face1 = 0, face2 = 0, face3 = 0, t;
            uint count = 0, faceIndex = 0;

            // HACK: We go through the whole loop twice, the first time just
            // to get the new faceCount, second time to actually get the faces.
            // This way we don't have to do any copying (yes, lists do it too)
            // which is a huge performance improvement.
            for (uint i = 0; i < faceCount; ++i)
            {
                t = reader.ReadUInt16();
                faces[i] = t;

                if (t == 0xFFFF)
                {
                    newStrip = 3;
                    newIndex = 0;
                }
                else
                {
                    newStrip -= 1;
                    face3 = face2;
                    face2 = face1;
                    face1 = t;

                    if (newStrip == 0)
                    {
                        if ((face1 != face2) && (face2 != face3) && (face1 != face3))
                            count += 3;

                        newStrip = 1;
                        ++newIndex;
                    }
                }
            }

            // Alright, we've got the new count!
            // Time to actually get the faces.
            var newFaces = new uint[count];
            newStrip = 3; newIndex = 0;
            face1 = face2 = face3 = 0;

            for (uint i = 0; i < faceCount; ++i)
            {
                t = faces[i];
                if (t == 0xFFFF)
                {
                    newStrip = 3;
                    newIndex = 0;
                }
                else
                {
                    newStrip -= 1;
                    face3 = face2;
                    face2 = face1;
                    face1 = t;

                    if (newStrip == 0)
                    {
                        if ((face1 != face2) && (face2 != face3) && (face1 != face3))
                        {
                            if ((newIndex % 2) == 0)
                            {
                                newFaces[faceIndex] = face1;
                                newFaces[++faceIndex] = face2;
                                newFaces[++faceIndex] = face3;
                            }
                            else
                            {
                                newFaces[faceIndex] = face3;
                                newFaces[++faceIndex] = face2;
                                newFaces[++faceIndex] = face1;
                            }

                            ++faceIndex;
                        }

                        newStrip = 1;
                        ++newIndex;
                    }
                }
            }

            // Vertex Format
            var vertexFormat = new List<VertexFormatElement>();
            var fs = reader.BaseStream;
            reader.JumpTo(vertexFormatOffset, false);

            for (byte i = 0; i < 0xFF; ++i)
            {
                offset = reader.ReadUInt32();
                if (offset > 1000) break;

                var element = new VertexFormatElement()
                {
                    Offset = offset,
                    Type = reader.ReadUInt32(),
                    ID = reader.ReadUInt16(),
                    Index = reader.ReadByte()
                };

                vertexFormat.Add(element);
                fs.ReadByte();
            }

            // Vertices
            var data = new float[vertexCount * Mesh.StructureLength];
            var idc = new float[vertexCount * 4]; // TODO: Care

            // HACK: Read this stuff using pointers to avoid boundry checks.
            fixed (float* dataStart = data)
            fixed (float* idcp = idc)
            {
                float* dp; // Pointer to data
                uint structStart;

                for (uint i = 0; i < vertexCount; ++i)
                {
                    // Set default vertex coloring
                    structStart = (i * Mesh.StructureLength);
                    dp = dataStart + structStart + Mesh.ColorPos;

                    // RGBA = 1, 1, 1, 1
                    *dp = 1; *++dp = 1;
                    *++dp = 1; *++dp = 1;

                    // Just to be accurate
                    offset = (vertexOffset + (i * vertexSize));
                    foreach (var element in vertexFormat)
                    {
                        reader.JumpTo(offset + element.Offset, false);
                        switch (element.ID)
                        {
                            // Vertex Positions
                            case 0:
                                element.Read(reader, dataStart +
                                    structStart + Mesh.VertPos, scale);
                                break;

                            // Bone Weights
                            case 1:
                                // TODO
                                element.Read(reader, idcp);
                                break;

                            // Bone Indices
                            case 2:
                                // TODO
                                element.Read(reader, idcp);
                                break;

                            // Normals
                            case 3:
                                // TODO: Do I need to scale normals?
                                element.Read(reader, dataStart +
                                    structStart + Mesh.NormPos);
                                break;

                            // UV Coordinates
                            case 5:
                                if (element.Index < 1)
                                {
                                    element.Read(reader, dataStart +
                                        structStart + Mesh.UVPos);
                                }
                                else
                                {
                                    // TODO: Read multi-UV Channels
                                    element.Read(reader, idcp);
                                }
                                break;

                            // Tangents
                            case 6:
                                // TODO
                                element.Read(reader, idcp);
                                break;

                            // Binormals
                            case 7:
                                // TODO
                                element.Read(reader, idcp);
                                break;

                            // Vertex Colors
                            case 10:
                                element.Read(reader, dataStart +
                                    structStart + Mesh.ColorPos);
                                break;
                        }
                    }

                    ++dp;
                }
            }

            // Bones
            reader.JumpTo(boneOffset, false);
            var bones = reader.ReadBytes((int)boneCount);

            // Texture Unit Offsets
            var textureUnitOffsets = new uint[textureUnitCount];
            reader.JumpTo(textureUnitOffsetsOffset, false);

            for (uint i = 0; i < textureUnitCount; ++i)
            {
                textureUnitOffsets[i] = reader.ReadUInt32();
            }

            // Texture Units
            for (uint i = 0; i < textureUnitCount; ++i)
            {
                // TODO: Actually use this data
                reader.JumpTo(textureUnitOffsets[i], false);
                uint textureUnitNameOffset = reader.ReadUInt32();
                uint textureUnitID = reader.ReadUInt32();

                // Texture Unit Name
                reader.JumpTo(textureUnitNameOffset, false);
                string textureUnitName = reader.ReadNullTerminatedString();
            }

            // Material Name
            reader.JumpTo(materialNameOffset, false);
            string materialName = reader.ReadNullTerminatedString();

            // Generate a HedgeLib mesh and add it to the array
            var mesh = new Mesh()
            {
                VertexData = data,
                Triangles = newFaces,
                MaterialName = materialName
            };

            Meshes.Add(mesh);
            return mesh;
        }

        // TODO: Make a Save method

        // Other
        protected class VertexFormatElement
        {
            // Variables/Constants
            public uint Offset, Type;
            public ushort ID;
            public byte Index;

            protected enum DataTypes
            {
                Indices_Byte = 0x1A2286,
                Indices = 0x1A2386,
                Vector2 = 0x2C23A5,
                Vector2_Half = 0x2C235F,
                Vector3 = 0x2A23B9,
                Vector3_360 = 0x2A2190,
                Vector3_Forces = 0x2761095,
                Vector4 = 0x1A23A6,
                Vector4_Byte = 0x1A2086
            }

            // Methods
            public unsafe void Read(GensReader reader, float* data, float scale = 1)
            {
                switch ((DataTypes)Type)
                {
                    case DataTypes.Vector2:
                        *data = reader.ReadSingle() * scale;
                        *++data = reader.ReadSingle() * scale;
                        break;

                    case DataTypes.Vector2_Half:
                        *data = reader.ReadHalf();
                        *++data = reader.ReadHalf();
                        break;

                    case DataTypes.Vector3:
                        *data = reader.ReadSingle() * scale;
                        *++data = reader.ReadSingle() * scale;
                        *++data = reader.ReadSingle() * scale;
                        break;

                    case DataTypes.Vector3_360:
                        // TODO: Read this properly
                        reader.ReadUInt32();
                        break;

                    case DataTypes.Vector3_Forces:
                        // TODO: Read this properly
                        reader.ReadUInt32();
                        break;

                    case DataTypes.Vector4:
                        *data = reader.ReadSingle() * scale;
                        *++data = reader.ReadSingle() * scale;
                        *++data = reader.ReadSingle() * scale;
                        *++data = reader.ReadSingle() * scale;
                        break;

                    case DataTypes.Vector4_Byte:
                    case DataTypes.Indices:
                        // TODO: Read this properly
                        reader.ReadUInt32();
                        break;

                    case DataTypes.Indices_Byte:
                        // TODO: Read this properly
                        reader.ReadUInt32();
                        break;
                }
            }

            public void Write(GensWriter writer, float[] data, uint i, float scale = 1)
            {
                switch ((DataTypes)Type)
                {
                    case DataTypes.Vector2:
                        writer.Write(data[i] * scale);
                        writer.Write(data[++i] * scale);
                        break;

                    case DataTypes.Vector2_Half:
                        // TODO: Is this correct?
                        writer.WriteHalf((Half)(data[i] * scale));
                        writer.WriteHalf((Half)(data[++i] * scale));
                        break;

                    case DataTypes.Vector3:
                        writer.Write(data[i] * scale);
                        writer.Write(data[++i] * scale);
                        writer.Write(data[++i] * scale);
                        break;

                    case DataTypes.Vector3_360:
                        // TODO: Write this properly
                        throw new NotImplementedException(
                            "Cannot yet write Vector3_360 values");

                    case DataTypes.Vector3_Forces:
                        // TODO: Write this properly
                        throw new NotImplementedException(
                            "Cannot yet write Vector3_Forces values");

                    case DataTypes.Vector4:
                        writer.Write(data[i] * scale);
                        writer.Write(data[++i] * scale);
                        writer.Write(data[++i] * scale);
                        writer.Write(data[++i] * scale);
                        break;

                    case DataTypes.Vector4_Byte:
                    case DataTypes.Indices:
                        // TODO: Write this properly
                        throw new NotImplementedException(
                            "Cannot yet write Vector4_Byte/Indices values");

                    case DataTypes.Indices_Byte:
                        // TODO: Write this properly
                        throw new NotImplementedException(
                            "Cannot yet write Indices_Byte values");
                }
            }
        }
    }
}