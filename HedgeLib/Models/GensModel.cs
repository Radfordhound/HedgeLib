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
        public const string Extension = ".model", TerrainExtension = ".terrain-model";
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

        protected Mesh ReadSubMesh(GensReader reader, float scale = 1)
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

            for (uint i = 0; i < faceCount; ++i)
            {
                faces[i] = reader.ReadUInt16();
                //Console.WriteLine($"Face: {faces[i]}");
            }

            // Convert faces from triangle strips
            // Taken from LibGens (for now) because my gosh
            int newStrip = 3, newIndex = 0;
            ushort face1 = 0, face2 = 0, face3 = 0;
            ushort lastFace = 0;

            var newFaces = new List<uint>();
            for (uint i = 0; i < faceCount; ++i)
            {
                ushort t = faces[i];
                if (t == 0xFFFF)
                {
                    newStrip = 3;
                    newIndex = 0;
                }
                else
                {
                    if (newStrip == 3) lastFace = t;

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
                                newFaces.Add(face1);
                                newFaces.Add(face2);
                                newFaces.Add(face3);
                            }
                            else
                            {
                                newFaces.Add(face3);
                                newFaces.Add(face2);
                                newFaces.Add(face1);
                            }

                            //newFaces.Add(((newIndex % 2) == 0) ?
                            //    new Vector3(face1, face2, face3) :
                            //    new Vector3(face3, face2, face1));

                            //newFaces.Add((doReverse) ? face1 : face3);
                            //newFaces.Add(face2);
                            //newFaces.Add((doReverse) ? face3 : face1);
                            //Console.WriteLine($"Final face: {vect}");
                        }

                        newStrip = 1;
                        ++newIndex;
                    }
                }
            }

            //for (uint i = 0; i < faceCount; ++i)
            //{
            //    ushort t = faces[i];
            //    if (t != 0xFFFF)
            //    {
            //        newFaces.Add(t);
            //    }
            //}

            // Vertex Format
            var vertexFormat = new List<VertexFormatElement>();
            reader.JumpTo(vertexFormatOffset, false);

            for (int i = 0; i < 0xFF; ++i)
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
                reader.JumpAhead(1);
            }

            // Vertices
            var data = new float[vertexCount * Mesh.StructureLength];
            var idc = new float[vertexCount * 4]; // TODO: Care

            for (uint i = 0; i < vertexCount; ++i)
            {
                // Set default vertex coloring
                data[(i * Mesh.StructureLength) + Mesh.ColorPos] = 1;
                data[(i * Mesh.StructureLength) + Mesh.ColorPos + 1] = 1;
                data[(i * Mesh.StructureLength) + Mesh.ColorPos + 2] = 1;
                data[(i * Mesh.StructureLength) + Mesh.ColorPos + 3] = 1;

                // Just to be accurate
                offset = (vertexOffset + (i * vertexSize));
                foreach (var element in vertexFormat)
                {
                    reader.JumpTo(offset + element.Offset, false);
                    switch (element.ID)
                    {
                        // Vertex Positions
                        case 0:
                            element.Read(reader, data,
                                (i * Mesh.StructureLength) + Mesh.VertPos, scale);
                            break;

                        // Bone Weights
                        case 1:
                            // TODO
                            element.Read(reader, idc, i);
                            break;

                        // Bone Indices
                        case 2:
                            // TODO
                            element.Read(reader, idc, i);
                            break;

                        // Normals
                        case 3:
                            // TODO: Do I need to scale normals?
                            element.Read(reader, data,
                                (i * Mesh.StructureLength) + Mesh.NormPos);
                            break;

                        // UV Coordinates
                        case 5:
                            if (element.Index < 1)
                            {
                                element.Read(reader, data,
                                    (i * Mesh.StructureLength) + Mesh.UVPos);
                            }
                            else
                            {
                                // TODO: Read multi-UV Channels
                                element.Read(reader, idc, i);
                            }
                            break;

                        // Tangents
                        case 6:
                            // TODO
                            element.Read(reader, idc, i);
                            break;

                        // Binormals
                        case 7:
                            // TODO
                            element.Read(reader, idc, i);
                            break;

                        // Vertex Colors
                        case 10:
                            element.Read(reader, data,
                                (i * Mesh.StructureLength) + Mesh.ColorPos);
                            break;
                    }
                }
            }

            // Bones
            var bones = new byte[boneCount];
            reader.JumpTo(boneOffset, false);

            for (uint i = 0; i < boneCount; ++i)
            {
                bones[i] = reader.ReadByte();
            }

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
                Triangles = newFaces.ToArray(),
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
            public void Read(GensReader reader, float[] data, uint i, float scale = 1)
            {
                switch ((DataTypes)Type)
                {
                    case DataTypes.Vector2:
                        data[i] = reader.ReadSingle() * scale;
                        data[i + 1] = reader.ReadSingle() * scale;
                        break;

                    case DataTypes.Vector2_Half:
                        data[i] = reader.ReadHalf();
                        data[i + 1] = reader.ReadHalf();
                        break;

                    case DataTypes.Vector3:
                        data[i] = reader.ReadSingle() * scale;
                        data[i + 1] = reader.ReadSingle() * scale;
                        data[i + 2] = reader.ReadSingle() * scale;
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
                        data[i] = reader.ReadSingle() * scale;
                        data[i + 1] = reader.ReadSingle() * scale;
                        data[i + 2] = reader.ReadSingle() * scale;
                        data[i + 3] = reader.ReadSingle() * scale;
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
                        writer.Write(data[i + 1] * scale);
                        break;

                    case DataTypes.Vector2_Half:
                        // TODO: Is this correct?
                        writer.WriteHalf((Half)(data[i] * scale));
                        writer.WriteHalf((Half)(data[i + 1] * scale));
                        break;

                    case DataTypes.Vector3:
                        writer.Write(data[i] * scale);
                        writer.Write(data[i + 1] * scale);
                        writer.Write(data[i + 2] * scale);
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
                        writer.Write(data[i + 1] * scale);
                        writer.Write(data[i + 2] * scale);
                        writer.Write(data[i + 3] * scale);
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