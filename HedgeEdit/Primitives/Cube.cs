using HedgeLib.Models;

namespace HedgeEdit.Primitives
{
    public static class Cube
    {
        // Variables/Constants
        public static Mesh Mesh => new Mesh()
        {
            VertexData = VertexData,
            Triangles = TriangleIndices
        };

        public static float[] VertexData => new float[]
        {
            -1, 1, 1,     0, 0, 0,    1, 1, 1, 1,    0, 0,
            1, 1, 1,      0, 0, -1,   1, 1, 1, 1,    1, 0,
            1, -1, 1,     0, 1, 0,    1, 1, 1, 1,    1, 1,
            -1, -1, 1,    1, 0, 0,    1, 1, 1, 1,    0, 1,
            -1, 1, -1,    0, 0, 0,    1, 1, 1, 1,    0, 0,
            1, 1, -1,     -1, 0, 0,   1, 1, 1, 1,    1, 0,
            1, -1, -1,    0, -1, 0,   1, 1, 1, 1,    1, 1,
            -1, -1, -1,   0, 0, 1,    1, 1, 1, 1,    0, 1,
        };

        public static uint[] TriangleIndices => new uint[]
        {
            6, 3, 7, 4, 1, 5, 5, 2, 6, 1, 3, 2,
            3, 4, 7, 6, 4, 5, 6, 2, 3, 4, 0, 1,
            5, 1, 2, 1, 0, 3, 3, 0, 4, 6, 7, 4
        };

        public static uint[] LineIndices => new uint[]
        {
            0, 1, 1, 2, 2, 3, 3, 0,
            4, 5, 5, 6, 6, 7, 7, 4,
            4, 0, 7, 3, 5, 1, 6, 2
        };
    }
}