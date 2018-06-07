using HedgeLib.IO;
using HedgeLib.Headers;
using System.IO;

namespace HedgeLib.Misc
{
    public class ForcesGISM : FileBase
    {
        // Variables/Constants
        public BINAHeader Header = new BINAv2Header(210);
        public ReactionData[] _ReactionData = new ReactionData[ReactionDataCount];
        public PhysicsParam _PhysicsParam;

        public string ModelName, SkeletonName, MeshName;
        public Vector3 BoundingSize, ShapeOffset;
        public float RangeIn, RangeDistance, ShapeSizeOffset;
        public bool RideOnDamage, AerialBounce;

        public BoundingShapes BoundingShape;
        public RigidBodyTypes RigidBodyType;
        public RigidBodyMaterials RigidBodyMaterial;
        public ContactDamageTypes ContactDamageType;

        public enum BoundingShapes
        {
            Box, Sphere, Cylinder, Mesh, None
        }

        public enum RigidBodyTypes
        {
            None, Static, Dynamic
        }

        public enum RigidBodyMaterials
        {
            None, Wood, Iron
        }

        public enum ContactDamageTypes
        {
            None, LowSpeed, MiddleSpeed, HighSpeed
        }

        protected bool IsBigEndian;

        public const string Extension = ".gism";
        public const uint ReactionDataCount = 6;

        // Methods
        public override void Load(Stream fileStream)
        {
            var reader = new BINAReader(fileStream);
            Header = reader.ReadHeader();
            IsBigEndian = reader.IsBigEndian;

            System.Console.WriteLine(reader.BaseStream.Position);
            RangeIn = reader.ReadSingle(); // Unused
            RangeDistance = reader.ReadSingle(); // Unused

            ModelName = reader.GetString();
            reader.JumpAhead(12);

            SkeletonName = reader.GetString();
            reader.JumpAhead(12);

            BoundingShape = (BoundingShapes)reader.ReadUInt32();

            BoundingSize = reader.ReadVector3();

            MeshName = reader.GetString();
            reader.JumpAhead(20);

            ShapeOffset = reader.ReadVector3();
            reader.JumpAhead(4);
            ShapeSizeOffset = reader.ReadSingle();

            RigidBodyType = (RigidBodyTypes)reader.ReadByte();
            RigidBodyMaterial = (RigidBodyMaterials)reader.ReadByte();
            reader.JumpAhead(2);

            _PhysicsParam = new PhysicsParam(reader);

            ContactDamageType = (ContactDamageTypes)reader.ReadByte();
            RideOnDamage = reader.ReadBoolean();
            AerialBounce = reader.ReadBoolean();

            reader.JumpAhead(9);

            // ReactionData
            for (uint i = 0; i < ReactionDataCount; i++)
            {
                _ReactionData[i] = new ReactionData(reader);
            }
        }

        public override void Save(Stream fileStream)
        {
            var writer = new BINAWriter(fileStream, Header);
            writer.Write(RangeIn);
            writer.Write(RangeDistance);

            writer.AddString("modelName", ModelName, 8);
            writer.WriteNulls(8);

            writer.AddString("skeletonName", SkeletonName, 8);
            writer.WriteNulls(8);

            writer.Write((uint)BoundingShape);
            writer.Write(BoundingSize);

            writer.AddString("meshName", MeshName, 8);
            writer.WriteNulls(16);

            writer.Write(ShapeOffset);
            writer.WriteNulls(4);
            writer.Write(ShapeSizeOffset);

            writer.Write((byte)RigidBodyType);
            writer.Write((byte)RigidBodyMaterial);
            writer.WriteNulls(2);

            writer.Write(_PhysicsParam.Mass);
            writer.Write(_PhysicsParam.Friction);
            writer.Write(_PhysicsParam.GravityFactor);
            writer.Write(_PhysicsParam.Restitution);
            writer.Write(_PhysicsParam.LinearDamping);
            writer.Write(_PhysicsParam.AngularDamping);
            writer.Write(_PhysicsParam.MaxLinearVelocity);

            writer.Write((byte)ContactDamageType);
            writer.Write(RideOnDamage);
            writer.Write(AerialBounce);
            writer.WriteNulls(9);

            // ReactionData
            for (int i = 0; i < ReactionDataCount; i++)
            {
                _ReactionData[i].Write(writer, i);
            }

            writer.FinishWrite(Header);
        }

        // ReactionData
        public class ReactionData
        {
            // Variables/Constants
            public MotionData _MotionData;
            public MirageAnimData _MirageAnimData;
            public ProgramMotionData _ProgramMotionData;
            public EffectData _EffectData;
            public SoundData _SoundData;
            public KillData _KillData;

            // Constructors
            public ReactionData() { }
            public ReactionData(BINAReader reader)
            {
                Read(reader);
            }

            // Methods
            public void Read(BINAReader reader)
            {
                _MotionData = new MotionData(reader);
                _MirageAnimData = new MirageAnimData(reader);
                reader.JumpAhead(8);
                _ProgramMotionData = new ProgramMotionData(reader);
                _EffectData = new EffectData(reader);
                _SoundData = new SoundData(reader);
                _KillData = new KillData(reader);

                reader.JumpAhead(8);
            }

            public void Write(BINAWriter writer, int index)
            {
                // MotionData
                writer.AddString($"_MotionData.MotionName{index}",
                    _MotionData.MotionName, 8);
                writer.WriteNulls(8);

                writer.Write(_MotionData.SyncFrame);
                writer.Write(_MotionData.StopEndFrame);
                writer.WriteNulls(6);

                // MirageAnimationData
                writer.AddString($"_MirageAnimData.TextureSrtAnimName0{index}",
                    _MirageAnimData.TextureSrtAnimName0, 8);
                writer.WriteNulls(8);

                writer.AddString($"_MirageAnimData.TextureSrtAnimName1{index}",
                    _MirageAnimData.TextureSrtAnimName1, 8);
                writer.WriteNulls(8);

                writer.AddString($"_MirageAnimData.TextureSrtAnimName2{index}",
                    _MirageAnimData.TextureSrtAnimName2, 8);
                writer.WriteNulls(8);

                writer.AddString($"_MirageAnimData.TexturePatAnimName0{index}",
                    _MirageAnimData.TexturePatAnimName0, 8);
                writer.WriteNulls(8);

                writer.AddString($"_MirageAnimData.TexturePatAnimName1{index}",
                    _MirageAnimData.TexturePatAnimName1, 8);
                writer.WriteNulls(8);

                writer.AddString($"_MirageAnimData.TexturePatAnimName2{index}",
                    _MirageAnimData.TexturePatAnimName2, 8);
                writer.WriteNulls(8);

                writer.AddString($"_MirageAnimData.MaterialAnimName0{index}",
                    _MirageAnimData.MaterialAnimName0, 8);
                writer.WriteNulls(8);

                writer.AddString($"_MirageAnimData.MaterialAnimName1{index}",
                    _MirageAnimData.MaterialAnimName1, 8);
                writer.WriteNulls(8);

                writer.AddString($"_MirageAnimData.MaterialAnimName2{index}",
                    _MirageAnimData.MaterialAnimName2, 8);
                writer.WriteNulls(16);

                // ProgramMotionData
                writer.Write((uint)_ProgramMotionData.MotionType);
                writer.WriteNulls(12);

                writer.Write(_ProgramMotionData.Axis);
                writer.WriteNulls(4);

                writer.Write(_ProgramMotionData.Power);
                writer.Write(_ProgramMotionData.SpeedScale);
                writer.Write(_ProgramMotionData.Time);
                writer.WriteNulls(4);

                // EffectData
                writer.AddString($"_EffectData.EffectName{index}", _EffectData.EffectName, 8);
                writer.WriteNulls(8);

                writer.Write(_EffectData.LinkMotionStop);
                writer.WriteNulls(7);

                // SoundData
                writer.AddString($"_SoundData.CueName{index}", _SoundData.CueName, 8);
                writer.WriteNulls(8);

                // KillData
                writer.Write((uint)_KillData.KillType);
                writer.Write(_KillData.KillTime);

                writer.AddString($"_KillData.BreakMotionName{index}",
                    _KillData.BreakMotionName, 8);
                writer.WriteNulls(8);

                writer.Write(_KillData._DebrisData.Gravity);
                writer.Write(_KillData._DebrisData.LifeTime);
                writer.Write(_KillData._DebrisData.Mass);
                writer.Write(_KillData._DebrisData.ExplosionScale);
                writer.Write(_KillData._DebrisData.ImpulseScale);
                writer.WriteNulls(12);
            }

            // Structs
            public struct MotionData
            {
                // Variables/Constants
                public string MotionName;
                public bool SyncFrame, StopEndFrame;

                // Constructors
                public MotionData(BINAReader reader)
                {
                    MotionName = reader.GetString();
                    reader.JumpAhead(12);

                    SyncFrame = reader.ReadBoolean();
                    StopEndFrame = reader.ReadBoolean();
                    reader.JumpAhead(6);
                }
            }

            public struct MirageAnimData
            {
                // Variables/Constants
                public string TextureSrtAnimName0, TextureSrtAnimName1, TextureSrtAnimName2,
                    TexturePatAnimName0, TexturePatAnimName1, TexturePatAnimName2,
                    MaterialAnimName0, MaterialAnimName1, MaterialAnimName2;

                // Constructors
                public MirageAnimData(BINAReader reader)
                {
                    TextureSrtAnimName0 = reader.GetString();
                    reader.JumpAhead(12);
                    TextureSrtAnimName1 = reader.GetString();
                    reader.JumpAhead(12);
                    TextureSrtAnimName2 = reader.GetString();
                    reader.JumpAhead(12);

                    TexturePatAnimName0 = reader.GetString();
                    reader.JumpAhead(12);
                    TexturePatAnimName1 = reader.GetString();
                    reader.JumpAhead(12);
                    TexturePatAnimName2 = reader.GetString();
                    reader.JumpAhead(12);

                    MaterialAnimName0 = reader.GetString();
                    reader.JumpAhead(12);
                    MaterialAnimName1 = reader.GetString();
                    reader.JumpAhead(12);
                    MaterialAnimName2 = reader.GetString();
                    reader.JumpAhead(12);
                }
            }

            public struct ProgramMotionData
            {
                // Variables/Constants
                public enum MotionTypes
                {
                    Swing, Rotate, LinearSwing
                }

                public MotionTypes MotionType;
                public Vector3 Axis;

                public float Power, SpeedScale, Time;

                // Constructors
                public ProgramMotionData(BINAReader reader)
                {
                    MotionType = (MotionTypes)reader.ReadUInt32();
                    reader.JumpAhead(12);

                    Axis = reader.ReadVector3();
                    reader.JumpAhead(4);

                    Power = reader.ReadSingle();
                    SpeedScale = reader.ReadSingle();
                    Time = reader.ReadSingle();
                    reader.JumpAhead(4);
                }
            }

            public struct EffectData
            {
                // Variables/Constants
                public string EffectName;
                public bool LinkMotionStop;

                // Constructors
                public EffectData(BINAReader reader)
                {
                    EffectName = reader.GetString();
                    reader.JumpAhead(12);
                    LinkMotionStop = reader.ReadBoolean();
                    reader.JumpAhead(7);
                }
            }

            public struct SoundData
            {
                // Variables/Constants
                public string CueName;

                // Constructors
                public SoundData(BINAReader reader)
                {
                    CueName = reader.GetString();
                    reader.JumpAhead(12);
                }
            }

            public struct KillData
            {
                // Variables/Constants
                public DebrisData _DebrisData;
                public enum KillTypes
                {
                    NotKill, Kill, Break, Motion
                }

                public KillTypes KillType;
                public float KillTime;
                public string BreakMotionName;

                // Constructors
                public KillData(BINAReader reader)
                {
                    KillType = (KillTypes)reader.ReadUInt32();

                    KillTime = reader.ReadSingle();
                    BreakMotionName = reader.GetString();
                    reader.JumpAhead(12);

                    _DebrisData = new DebrisData(reader);
                    reader.JumpAhead(4);
                }

                // Structs
                public struct DebrisData
                {
                    // Variables/Constants
                    public float Gravity, LifeTime, Mass, ExplosionScale, ImpulseScale;

                    // Constructors
                    public DebrisData(BINAReader reader)
                    {
                        Gravity = reader.ReadSingle();
                        LifeTime = reader.ReadSingle();
                        Mass = reader.ReadSingle();
                        ExplosionScale = reader.ReadSingle();
                        ImpulseScale = reader.ReadSingle();
                    }
                }
            }
        }

        // Structs
        public struct PhysicsParam
        {
            // Variables/Constants
            public float Mass, Friction, GravityFactor, Restitution,
               LinearDamping, AngularDamping, MaxLinearVelocity;

            // Constructors
            public PhysicsParam(BINAReader reader)
            {
                Mass = reader.ReadSingle();
                Friction = reader.ReadSingle();
                GravityFactor = reader.ReadSingle();
                Restitution = reader.ReadSingle();
                LinearDamping = reader.ReadSingle();
                AngularDamping = reader.ReadSingle();
                MaxLinearVelocity = reader.ReadSingle();
            }
        }
    }
}