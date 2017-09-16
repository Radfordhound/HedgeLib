using HedgeLib.IO;
using System.Collections.Generic;

namespace HedgeLib.Models
{
    public class Model : FileBase
    {
        // Variables/Constants
        public List<Mesh> Meshes = new List<Mesh>();

        // TODO: Add materials and such - probably just as names (a list of strings) so
        // scenes that share the same materials between multiple models are optimized.

        // Constructors
        public Model() { }
        public Model(params Mesh[] meshes)
        {
            Meshes.AddRange(meshes);
        }
    }
}