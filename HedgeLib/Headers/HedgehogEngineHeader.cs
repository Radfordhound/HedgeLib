using HedgeLib.IO;

namespace HedgeLib.Headers
{
    /// <summary>
    /// Exists as a way to distinquish Hedgehog Engine
    /// Headers from non-Hedgehog Engine headers.
    /// </summary>
    public abstract class HedgehogEngineHeader : IHeader
    {
        // Variables/Constants
        public uint RootNodeType, FooterOffset;

        // Methods
        public abstract void Read(ExtendedBinaryReader reader);
        public abstract void PrepareWrite(ExtendedBinaryWriter writer);
        public abstract void FinishWrite(ExtendedBinaryWriter writer);
    }
}