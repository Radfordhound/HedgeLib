namespace HedgeLib.Models
{
    public class Mesh
    {
        // Variables/Constants
        public float[] Vertices, Normals,
            Colors, UVs; // Not Vectors for better control/GL performance

        public uint[] Triangles;
    }
}