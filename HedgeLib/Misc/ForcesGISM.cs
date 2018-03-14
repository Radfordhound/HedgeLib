using HedgeLib.IO;
using HedgeLib.Headers;
using System.IO;

namespace HedgeLib.Misc
{
    public class ForcesGISM : FileBase
    {
        // Variables/Constants
        public BINAHeader Header = new BINAHeader();
        public Property[] Properties = new Property[PropertyCount];

        public string AnimationName, SkeletonName;
        public Vector3 BoundingSize;
        public float RangeIn, RangeOut, UnknownFloat1, UnknownFloat2,
            UnknownFloat3, UnknownFloat4, UnknownFloat5, UnknownFloat6,
            UnknownFloat7, UnknownFloat8, UnknownFloat9;

        public ushort SpeedRequiredToBreak, UnknownShort;
        public BoundingShapes BoundingShape;
        public byte StaticCollision, UnknownByte;

        public enum BoundingShapes
        {
            Box, Sphere, Cylinder, None
        }

        public bool IsBigEndian
        {
            get;
            protected set;
        }

        public const string Extension = ".gism";
        public const uint PropertyCount = 6;

        // Methods
        public override void Load(Stream fileStream)
        {
            var reader = new BINAReader(fileStream, BINA.BINATypes.Version2);
            Header = reader.ReadHeader();
            IsBigEndian = reader.IsBigEndian;

            RangeIn = reader.ReadSingle(); // Unused
            RangeOut = reader.ReadSingle(); // Unused

            AnimationName = reader.GetString();
            reader.JumpAhead(12);

            SkeletonName = reader.GetString();
            reader.JumpAhead(12);

            BoundingShape = (BoundingShapes)reader.ReadByte();
            reader.JumpAhead(3);

            BoundingSize = reader.ReadVector3();
            reader.JumpAhead(28);

            UnknownFloat1 = reader.ReadSingle();
            reader.JumpAhead(8);

            UnknownFloat2 = reader.ReadSingle();

            StaticCollision = reader.ReadByte(); // 00 - OFF, 01 - ON. Maybe a boolean?

            UnknownByte = reader.ReadByte();
            reader.JumpAhead(2);

            UnknownFloat3 = reader.ReadSingle();
            UnknownFloat4 = reader.ReadSingle();
            UnknownFloat5 = reader.ReadSingle();
            UnknownFloat6 = reader.ReadSingle();
            UnknownFloat7 = reader.ReadSingle();
            UnknownFloat8 = reader.ReadSingle();
            UnknownFloat9 = reader.ReadSingle();

            SpeedRequiredToBreak = reader.ReadUInt16();

            UnknownShort = reader.ReadUInt16();
            reader.JumpAhead(8);

            // Properties
            for (uint i = 0; i < PropertyCount; i++)
            {
                Properties[i] = new Property(reader);
            }
        }

        public override void Save(Stream fileStream)
        {
            Save(fileStream, IsBigEndian);
        }

        public void Save(Stream fileStream, bool isBigEndian)
        {
            var writer = new BINAWriter(fileStream,
                BINA.BINATypes.Version2, isBigEndian);

            Header.Version = 210;
            writer.Write(RangeIn);
            writer.Write(RangeOut);

            if (string.IsNullOrEmpty(AnimationName))
                writer.Write(0UL);
            else
                writer.AddString("animationName", AnimationName, 8);

            writer.WriteNulls(8);

            if (string.IsNullOrEmpty(SkeletonName))
                writer.Write(0UL);
            else
                writer.AddString("skeletonName", SkeletonName, 8);

            writer.WriteNulls(8);

            writer.Write((uint)BoundingShape); // Is this correct? Or is it just a single byte?
            writer.Write(BoundingSize);
            writer.WriteNulls(28);

            writer.Write(UnknownFloat1);
            writer.WriteNulls(8);

            writer.Write(UnknownFloat2);
            writer.Write(StaticCollision);
            writer.Write(UnknownByte);
            writer.WriteNulls(2);

            writer.Write(UnknownFloat3);
            writer.Write(UnknownFloat4);
            writer.Write(UnknownFloat5);
            writer.Write(UnknownFloat6);
            writer.Write(UnknownFloat7);
            writer.Write(UnknownFloat8);
            writer.Write(UnknownFloat9);

            writer.Write(SpeedRequiredToBreak);
            writer.Write(UnknownShort);
            writer.WriteNulls(8);

            // Properties
            for (int i = 0; i < PropertyCount; i++)
            {
                Properties[i].Write(writer, i);
            }

            writer.FinishWrite(Header);
        }

        // Other
        public class Property
        {
            // Variables/Constants
            public string UnknownOffset, EffectName, CueName, UnusedOffset;
            public byte UnknownByte1, UnknownByte2, IsBreakable; /*6th preset*/

            public float UnknownFloat1, UnknownFloat2, FlowerRotation; /*2nd preset*/

            public uint UnknownInt;

            public float EndFloat1, EndFloat2, EndFloat3, EndFloat4, EndFloat5;

            // Constructors
            public Property() { }
            public Property(BINAReader reader)
            {
                Read(reader);
            }

            // Methods
            public void Read(BINAReader reader)
            {
                UnknownOffset = reader.GetString();
                reader.JumpAhead(12);

                UnknownByte1 = reader.ReadByte();
                UnknownByte2 = reader.ReadByte();

                reader.JumpAhead(190);

                UnknownFloat1 = reader.ReadSingle();
                UnknownFloat2 = reader.ReadSingle();

                FlowerRotation = reader.ReadSingle();
                reader.JumpAhead(4);

                EffectName = reader.GetString();
                reader.JumpAhead(20);

                CueName = reader.GetString();
                reader.JumpAhead(12);

                IsBreakable = reader.ReadByte();
                reader.JumpAhead(7);

                UnusedOffset = reader.GetString();
                reader.JumpAhead(12);

                EndFloat1 = reader.ReadSingle();
                EndFloat2 = reader.ReadSingle();
                EndFloat3 = reader.ReadSingle();
                EndFloat4 = reader.ReadSingle();
                EndFloat5 = reader.ReadSingle();
                reader.JumpAhead(12);
            }

            public void Write(BINAWriter writer, int index)
            {
                if (string.IsNullOrEmpty(UnknownOffset))
                    writer.Write(0UL);
                else
                    writer.AddString($"unknownOffset{index}", UnknownOffset, 8);

                writer.WriteNulls(8);
                writer.Write(UnknownByte1);
                writer.Write(UnknownByte2);

                writer.WriteNulls(190); // wth sonic team
                writer.Write(UnknownFloat1);
                writer.Write(UnknownFloat2);

                writer.Write(FlowerRotation);
                writer.WriteNulls(4);

                if (string.IsNullOrEmpty(EffectName))
                    writer.Write(0UL);
                else
                    writer.AddString($"EffectName{index}", EffectName, 8);
                writer.WriteNulls(16);

                if (string.IsNullOrEmpty(CueName))
                    writer.Write(0UL);
                else
                    writer.AddString($"CueName{index}", CueName, 8);
                writer.WriteNulls(8);

                writer.Write(IsBreakable);
                writer.WriteNulls(7);

                if (string.IsNullOrEmpty(UnusedOffset))
                    writer.Write(0UL);
                else
                    writer.AddString($"UnusedOffset{index}", UnusedOffset, 8);
                writer.WriteNulls(8);

                writer.Write(EndFloat1);
                writer.Write(EndFloat2);
                writer.Write(EndFloat3);
                writer.Write(EndFloat4);
                writer.Write(EndFloat5);
                writer.WriteNulls(12);
            }
        }
    }
}