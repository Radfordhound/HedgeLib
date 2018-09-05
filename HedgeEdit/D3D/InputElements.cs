using HedgeLib.Models;
using SharpDX.Direct3D11;
using SharpDX.DXGI;

namespace HedgeEdit.D3D
{
    public static class InputElements
    {
        // Variables/Constants
        public static readonly InputElement[] Default = new InputElement[]
        {
            new InputElement("POSITION", 0, Format.R32G32B32_Float, Mesh.VertPos, 0),
            new InputElement("NORMAL", 0, Format.R32G32B32_Float,
                Mesh.NormPos * sizeof(float), 0),
            new InputElement("COLOR", 0, Format.R32G32B32A32_Float,
                Mesh.ColorPos * sizeof(float), 0),
            new InputElement("TEXCOORD", 0, Format.R32G32_Float,
                Mesh.UVPos * sizeof(float), 0)
        };

        // TODO: Pass in this data like Forces actually
        // does it using the commented-out array below
        public static readonly InputElement[] HedgehogEngine2 = new InputElement[]
        {
            new InputElement("POSITION", 0, Format.R32G32B32A32_Float, Mesh.VertPos, 0),
            new InputElement("NORMAL", 0, Format.R32G32B32_Float,
                Mesh.NormPos * sizeof(float), 0),
            new InputElement("TANGENT", 0, Format.R32G32B32_Float,
                Mesh.NormPos * sizeof(float), 0),
            new InputElement("BINORMAL", 0, Format.R32G32B32_Float,
                Mesh.NormPos * sizeof(float), 0),
            new InputElement("COLOR", 0, Format.R32G32B32A32_Float,
                Mesh.ColorPos * sizeof(float), 0),
            new InputElement("TEXCOORD", 0, Format.R32G32B32A32_Float,
                Mesh.UVPos * sizeof(float), 0),
            new InputElement("TEXCOORD", 1, Format.R32G32B32A32_Float,
                Mesh.UVPos * sizeof(float), 0),
            new InputElement("TEXCOORD", 2, Format.R32G32B32A32_Float,
                Mesh.UVPos * sizeof(float), 0),
            new InputElement("TEXCOORD", 3, Format.R32G32B32A32_Float,
                Mesh.UVPos * sizeof(float), 0),
            new InputElement("BLENDWEIGHT", 0, Format.R32G32B32A32_Float,
                Mesh.NormPos * sizeof(float), 0),
            new InputElement("BLENDINDICES", 0, Format.R32G32B32A32_SInt,
                Mesh.NormPos * sizeof(float), 0)
        };

        //public static readonly InputElement[] HedgehogEngine2 = new InputElement[]
        //{
        //    new InputElement("POSITION", 0, Format.R32G32B32A32_Float, 0, 0),
        //    new InputElement("NORMAL", 0, Format.R32G32B32_Float, 16, 0),
        //    new InputElement("TANGENT", 0, Format.R32G32B32_Float, 28, 0),
        //    new InputElement("BINORMAL", 0, Format.R32G32B32_Float, 40, 0),
        //    new InputElement("COLOR", 0, Format.R32G32B32A32_Float, 52, 0),
        //    new InputElement("TEXCOORD", 0, Format.R32G32B32A32_Float, 68, 0),
        //    new InputElement("TEXCOORD", 1, Format.R32G32B32A32_Float, 84, 0),
        //    new InputElement("TEXCOORD", 2, Format.R32G32B32A32_Float, 100, 0),
        //    new InputElement("TEXCOORD", 3, Format.R32G32B32A32_Float, 116, 0),
        //    new InputElement("BLENDWEIGHT", 0, Format.R32G32B32A32_Float, 132, 0),
        //    new InputElement("BLENDINDICES", 0, Format.R32G32B32A32_SInt, 148, 0)
        //};

        // Methods
        public static InputElement[] GetElements(Viewport.RenderModes renderMode)
        {
            switch (renderMode)
            {
                case Viewport.RenderModes.HedgehogEngine2:
                    return HedgehogEngine2;

                default:
                    return Default;
            }
        }
    }
}