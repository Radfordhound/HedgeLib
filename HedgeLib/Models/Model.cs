using HedgeLib.Exceptions;
using HedgeLib.IO;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace HedgeLib.Models
{
    public class Model : FileBase
    {
        // Variables/Constants
        public List<Mesh> Meshes = new List<Mesh>();
        public string Name;

        public const string MDLExtension = ".mdl";
        public const uint Signature = 0x4C444D; // MDL in little-endian
        public const byte Version = 1;

        // Constructors
        public Model() { }
        public Model(params Mesh[] meshes)
        {
            Meshes.AddRange(meshes);
        }

        // Methods
        public override void Load(Stream fileStream)
        {
            // Header
            var reader = new ExtendedBinaryReader(fileStream);
            uint header = reader.ReadUInt32();

            uint sig = (header & 0xFFFFFF);
            byte version = (byte)((header & 0xFF000000) >> 24);
            if (sig != Signature)
            {
                throw new InvalidSignatureException("MDL",
                    Encoding.ASCII.GetString(BitConverter.GetBytes(sig), 0, 3));
            }

            if (version > Version)
            {
                throw new NotSupportedException(
                    $"Cannot read version {version} of the format.");
            }

            // Meshes
            Mesh mesh;
            uint len, i, i2;
            uint meshCount = reader.ReadUInt32();
            byte indicesType;

            for (i = 0; i < meshCount; ++i)
            {
                // Vertex Data
                mesh = new Mesh();
                len = reader.ReadUInt32();
                mesh.VertexData = new float[len];

                for (i2 = 0; i2 < len; ++i2)
                {
                    mesh.VertexData[i2] = reader.ReadSingle();
                }

                // Triangle Indices
                len = reader.ReadUInt32();
                indicesType = (byte)((len & 0xC0000000) >> 30);
                len &= 0x3FFFFFFF;

                mesh.Triangles = new uint[len];
                for (i2 = 0; i2 < len; ++i2)
                {
                    // 0 = 32-bit, 1 = 16-bit, 2 = 8-bit
                    switch (indicesType)
                    {
                        case 0:
                        default:
                            mesh.Triangles[i2] = reader.ReadUInt32();
                            continue;

                        case 1:
                            mesh.Triangles[i2] = reader.ReadUInt16();
                            continue;

                        case 2:
                            mesh.Triangles[i2] = reader.ReadByte();
                            continue;
                    }
                }

                // Material Name
                mesh.MaterialName = reader.ReadString();
                Meshes.Add(mesh);
            }
        }

        public override void Save(Stream fileStream)
        {
            // Header
            var writer = new ExtendedBinaryWriter(fileStream);
            writer.Write(Signature | (Version << 24));
            writer.Write(Meshes.Count);

            // Meshes
            uint tri;
            uint len, i;
            byte indicesType;

            foreach (var mesh in Meshes)
            {
                // Vertex Data
                len = (uint)mesh.VertexData.Length;
                writer.Write(len);
                for (i = 0; i < len; ++i)
                {
                    writer.Write(mesh.VertexData[i]);
                }

                // Triangle Indices
                indicesType = 2; // 8-bit
                len = (uint)mesh.Triangles.Length;

                for (i = 0; i < len; ++i)
                {
                    tri = mesh.Triangles[i];
                    if (tri > ushort.MaxValue)
                    {
                        indicesType = 0; // 32-bit
                        break;
                    }
                    else if (tri > byte.MaxValue)
                    {
                        indicesType = 1; // 16-bit
                    }
                }

                writer.Write(len | ((uint)indicesType << 30));
                for (i = 0; i < len; ++i)
                {
                    // 0 = 32-bit, 1 = 16-bit, 2 = 8-bit
                    switch (indicesType)
                    {
                        case 0:
                        default:
                            writer.Write(mesh.Triangles[i]);
                            continue;

                        case 1:
                            writer.Write((ushort)mesh.Triangles[i]);
                            continue;

                        case 2:
                            writer.Write((byte)mesh.Triangles[i]);
                            continue;
                    }
                }

                // Material Name
                if (string.IsNullOrEmpty(mesh.MaterialName))
                {
                    writer.Write((byte)0);
                }
                else
                {
                    writer.Write(mesh.MaterialName);
                }
            }
        }
    }
}