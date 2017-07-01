namespace HedgeLib.Models
{
    public class Mesh
    {
        //Variables/Constants
        public float[] Vertices, Normals; // Not Vector3s for better control/GL performance
        public uint[] Triangles;
    }
}