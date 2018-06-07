using HedgeLib.IO;
using System;

namespace HedgeLib.Headers
{
    public class BINAHeader : IHeader
    {
        // Variables/Constants
        public uint FileSize, FinalTableLength;
        public ushort Version;
        public bool IsBigEndian;
        public const string Signature = "BINA";
        public const char BigEndianFlag = 'B', LittleEndianFlag = 'L';

        // Methods
        public virtual void Read(ExtendedBinaryReader reader)
        {
            throw new NotImplementedException();
        }

        public virtual void PrepareWrite(ExtendedBinaryWriter writer)
        {
            throw new NotImplementedException();
        }

        public virtual void FinishWrite(ExtendedBinaryWriter writer)
        {
            throw new NotImplementedException();
        }
    }
}