using System;
using System.IO;
using System.Text;
using HedgeLib.Exceptions;
using HedgeLib.Headers;
using HedgeLib.IO;

namespace HedgeLib.Misc
{
    public class FXCol : FileBase
    {
        // Variables/Constants
        public BINAHeader Header = new BINAv2Header(210);
        public const string Extension = ".bin";
        public const uint Signature = 0x4658434F; // FXCO in ASCII

        // Methods
        public override void Load(Stream fileStream)
        {
            // Header
            var reader = new BINAReader(fileStream);
            Header = reader.ReadHeader();

            uint sig = reader.ReadUInt32();
            if (sig != Signature)
            {
                throw new InvalidSignatureException("FXCO",
                    Encoding.ASCII.GetString(BitConverter.GetBytes(sig)));
            }

            uint unknown1 = reader.ReadUInt32(); // Version number?
            ulong instanceCount = reader.ReadUInt64();
            ulong instancesOffset = reader.ReadUInt64();

            ulong shapesCount = reader.ReadUInt64();
            ulong shapesOffset = reader.ReadUInt64();

            ulong unknownCount = reader.ReadUInt64();
            ulong unknownOffset = reader.ReadUInt64();

            // Instances
            reader.JumpTo((long)instancesOffset, false);
            for (uint i = 0; i < instanceCount; ++i)
            {
                reader.FixPadding(8);
                ulong nameOffset = reader.ReadUInt64();
                byte unknownFlag1 = reader.ReadByte();
                byte unknownFlag2 = reader.ReadByte();
                byte unknownFlag3 = reader.ReadByte();
                byte unknownFlag4 = reader.ReadByte();

                var vect1 = reader.ReadVector3(); // Position or size?
                ulong instPadding1 = reader.ReadUInt64();
                uint instPadding2 = reader.ReadUInt32();
                uint instUnknown1 = reader.ReadUInt32();

                ulong instPadding3 = reader.ReadUInt64();
                ulong instPadding4 = reader.ReadUInt64();
                ulong instPadding5 = reader.ReadUInt64();
                ulong instPadding6 = reader.ReadUInt64();

                ulong unknownString1 = reader.ReadUInt64(); // Always "none"?
                var vect2 = reader.ReadVector3(); // Position or size?
                uint instUnknown2 = reader.ReadUInt32(); // Always 0x00000080?
                uint instPadding7 = reader.ReadUInt32();
                uint instUnknown3 = reader.ReadUInt32(); // Always 0x00000080?

                float instUnknown4 = reader.ReadSingle(); // Always 1?
            }

            // Unknown
            reader.JumpTo((long)unknownOffset, false);
            for (uint i = 0; i < unknownCount; ++i)
            {
                ulong uk1 = reader.ReadUInt64(); // Maybe 8 indices?
            }

            // Shapes
            reader.JumpTo((long)shapesOffset, false);
            for (uint i = 0; i < shapesCount; ++i)
            {
                uint shapeUnknown1 = reader.ReadUInt32();
                uint shapeUnknown2 = reader.ReadUInt32();
                var shapeUnknown3 = reader.ReadVector3();
                var shapeUnknown4 = reader.ReadVector3();
            }
        }
    }
}