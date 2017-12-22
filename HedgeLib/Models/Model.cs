using HedgeLib.IO;
using System.Collections.Generic;

namespace HedgeLib.Models
{
    public class Model : FileBase
    {
        // Variables/Constants
        public List<Mesh> Meshes = new List<Mesh>();
        public string Name;

        // Constructors
        public Model() { }
        public Model(params Mesh[] meshes)
        {
            Meshes.AddRange(meshes);
        }
    }
}