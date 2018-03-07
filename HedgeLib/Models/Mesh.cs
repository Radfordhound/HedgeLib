using System;

namespace HedgeLib.Models
{
    public class Mesh
    {
        // Variables/Constants
        /// <summary>
        /// Vertex Position (XYZ), followed by a Normal Vector (XYZ),
        /// then a Color (RGBA), and finally a Texture Coordinate (XY).
        /// </summary>
        public float[] VertexData;
        public uint[] Triangles;
        public string MaterialName;
        public Slots Slot = Slots.Default;

        public enum Slots
        {
            Default, Transparent, Punch, Special
        }

        public const int StructureLength = 12, StructureByteLength = 48;
        public const int VertPos = 0, NormPos = 3, ColorPos = 6, UVPos = 10;

        // Constructors
        public Mesh() { }
        public Mesh(float[] vertices, float[] normals = null,
            float[] colors = null, float[] UVs = null)
        {
            if (vertices == null)
                throw new ArgumentNullException("vertices");

            int vertsLen = vertices.Length;
            int vertsCount = (vertsLen / 3);
            int normsLen = (normals != null) ? normals.Length : 0;
            int colorsLen = (colors != null) ? colors.Length : (vertsCount * 4);
            int uvsLen = (UVs != null) ? UVs.Length : 0;

            VertexData = new float[vertsCount * StructureLength];

            // Vertices
            uint i = VertPos;
            for (uint i2 = 0; i2 < vertsLen; i2 += 3)
            {
                VertexData[i] = vertices[i2];
                VertexData[i + 1] = vertices[i2 + 1];
                VertexData[i + 2] = vertices[i2 + 2];
                i += StructureLength;
            }

            // Normals
            i = NormPos;
            for (uint i2 = 0; i2 < normsLen; i2 += 3)
            {
                VertexData[i] = normals[i2];
                VertexData[i + 1] = normals[i2 + 1];
                VertexData[i + 2] = normals[i2 + 2];
                i += StructureLength;
            }

            // Colors
            i = ColorPos;
            for (uint i2 = 0; i2 < colorsLen; i2 += 4)
            {
                VertexData[i] = (colors == null) ? 1 : colors[i2];
                VertexData[i + 1] = (colors == null) ? 1 : colors[i2 + 1];
                VertexData[i + 2] = (colors == null) ? 1 : colors[i2 + 2];
                VertexData[i + 3] = (colors == null) ? 1 : colors[i2 + 3];
                i += StructureLength;
            }

            // UVs
            i = UVPos;
            for (uint i2 = 0; i2 < uvsLen; i2 += 2)
            {
                VertexData[i] = UVs[i2];
                VertexData[i + 1] = UVs[i2 + 1];
                i += StructureLength;
            }
        }
    }
}