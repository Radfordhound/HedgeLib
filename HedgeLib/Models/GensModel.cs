using HedgeLib.IO;
using System;
using System.Collections.Generic;
using System.IO;

namespace HedgeLib.Models
{
    public class GensModel : Model
    {
        // Variables/Constants
        public const string Extension = ".model", TerrainExtension = ".terrain-model";
        public const int SubMeshSlotCount = 4;
        public const uint NextGenSignature = 0x133054A;

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
            uint fileSize = reader.ReadUInt32();
            uint rootNodeType = reader.ReadUInt32();

            // Next-Gen Header
            byte nextGenMarker = (byte)(fileSize >> 24);
            if (nextGenMarker == 0x80 && rootNodeType == NextGenSignature)
            {
                uint finalTableOffset = reader.ReadUInt32();
                uint finalTableLength = reader.ReadUInt32();
                reader.Offset = 0x10;

                // Sections
                // TODO: Do something with these
                fileSize >>= 8;
                while (fileStream.Position < fileSize)
                {
                    uint sectionOffset = reader.ReadUInt32();
                    byte sectionType = (byte)(sectionOffset & 0xFF);
                    sectionOffset >>= 8;

                    uint sectionValue = reader.ReadUInt32();
                    string sectionName = new string(reader.ReadChars(8));

                    if (sectionName == "Contexts")
                        break;
                }
            }

            // Generations Header
            else
            {
                uint finalTableOffset = reader.ReadUInt32();
                uint rootNodeOffset = reader.ReadUInt32();
                uint finalTableAbsOffset = reader.ReadUInt32();
                uint fileEndOffset = reader.ReadUInt32();
                reader.Offset = rootNodeOffset;
            }

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
                    ReadSubMesh(reader, scale);
                }

                reader.JumpTo(curPos + (++i * 8));
            }
        }

        protected void ReadSubMesh(GensReader reader, float scale = 1)
        {
            uint offset; // Generic uint reused for different data

            // Offsets
            uint materialNameOffset = reader.ReadUInt32();
            uint faceCount = reader.ReadUInt32();
            uint faceOffset = reader.ReadUInt32();
            uint vertexCount = reader.ReadUInt32();
            uint vertexSize = reader.ReadUInt32();
            uint vertexOffset = reader.ReadUInt32();
            uint vertexFormatOffset = reader.ReadUInt32();
            uint boneCount = reader.ReadUInt32();
            uint boneOffset = reader.ReadUInt32();
            uint textureUnitCount = reader.ReadUInt32();
            uint textureUnitOffsetsOffset = reader.ReadUInt32();

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

            var newFaces = new List<Vector3>();
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
                            newFaces.Add(((newIndex % 2) == 0) ?
                                new Vector3(face1, face2, face3) :
                                new Vector3(face3, face2, face1));
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
                Triangles = new uint[newFaces.Count * 3],
                MaterialName = materialName
            };

            for (int i = 0; i < newFaces.Count; ++i)
            {
                var vect = newFaces[i];
                mesh.Triangles[i * 3] = (uint)vect.X;
                mesh.Triangles[(i * 3) + 1] = (uint)vect.Y;
                mesh.Triangles[(i * 3) + 2] = (uint)vect.Z;
            }

            Meshes.Add(mesh);
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
                        // TODO: Read this properly
                        reader.ReadUInt32();
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
                        throw new NotImplementedException();
                }
            }
        }
    }
}