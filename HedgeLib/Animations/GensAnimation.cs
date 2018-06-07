using HedgeLib.Headers;
using HedgeLib.IO;
using System;
using System.Collections.Generic;
using System.IO;
using System.Xml.Linq;

namespace HedgeLib.Animations
{
    public abstract class GensAnimation : FileBase
    {
        // Variables/Constants
        public GensHeader Header = new GensHeader();
        public List<Animation> Animations = new List<Animation>();
        protected string name;

        // Methods
        public override void Load(Stream fileStream)
        {
            Read(fileStream);
        }

        protected virtual string Read(Stream fileStream,
            bool readAdditionalString = false)
        {
            // Header
            var reader = new GensReader(fileStream);
            Header.Read(reader);

            uint metaDataOffset = reader.ReadUInt32();
            uint metaAnimsSize = reader.ReadUInt32();
            uint keyframesOffset = reader.ReadUInt32();
            uint keyframesSize = reader.ReadUInt32();
            uint stringTableOffset = reader.ReadUInt32();
            uint stringTableSize = reader.ReadUInt32();

            // MetaData
            reader.JumpTo(metaDataOffset, false);
            uint materialNameOffset = reader.ReadUInt32();
            uint additionalStringOffset = (readAdditionalString) ?
                reader.ReadUInt32() : 0;

            uint animCount = reader.ReadUInt32();
            var animOffsets = new uint[animCount];
            for (uint i = 0; i < animCount; ++i)
            {
                animOffsets[i] = reader.ReadUInt32();
            }

            // Animations
            for (uint i = 0; i < animCount; ++i)
            {
                reader.JumpTo(animOffsets[i], false);
                Animations.Add(new Animation(reader));
            }

            // Keyframes
            foreach (var anim in Animations)
            {
                foreach (var set in anim.KeyframeSets)
                {
                    set.ReadKeyframes(reader, keyframesOffset);
                }
            }

            // String Table
            name = ReadString(materialNameOffset);
            string additionalString = (readAdditionalString) ?
                ReadString(additionalStringOffset) : null;

            foreach (var anim in Animations)
            {
                anim.BlendType = ReadString(anim.BlendTypeOffset);
            }

            return additionalString;

            // Sub-Methods
            string ReadString(uint off)
            {
                reader.JumpTo(stringTableOffset + off, false);
                return reader.ReadNullTerminatedString();
            }
        }

        public override void Save(Stream fileStream)
        {
            Write(fileStream);
        }

        protected virtual void Write(Stream fileStream,
            string additionalString = null)
        {
            // WHY ARE SIZeS IN THE OFfSET TABlE SEGA YOU BUMS?>??!??!
            int stringTableSize = 0;
            var stringTable = new List<string>();
            var writer = new GensWriter(fileStream, Header);

            writer.AddOffset("metaDataOffset");
            writer.AddOffset("metaAnimsSize");
            writer.AddOffset("keyframesOffset");
            writer.AddOffset("keyframesSize");
            writer.AddOffset("stringTableOffset");
            writer.AddOffset("stringTableSize");

            // MetaData
            uint metaDataOffset = (uint)fileStream.Position;
            writer.FillInOffset("metaDataOffset", false, false);
            AddStringToTable(name);

            if (!string.IsNullOrEmpty(additionalString))
                AddStringToTable(additionalString);

            writer.Write(Animations.Count);
            writer.AddOffsetTable("animsOffset", (uint)Animations.Count);

            // Animations
            for (int i = 0; i < Animations.Count; ++i)
            {
                var anim = Animations[i];
                writer.FillInOffset($"animsOffset_{i}", false, false);
                AddStringToTable(anim.BlendType);
                anim.Write(writer, true);
            }

            // Keyframes
            uint keyframesOffset = (uint)fileStream.Position;
            writer.FillInOffset("keyframesOffset", false, false);
            writer.FillInOffset("metaAnimsSize", keyframesOffset -
                metaDataOffset, true, false);

            foreach (var anim in Animations)
            {
                foreach (var set in anim.KeyframeSets)
                {
                    set.WriteKeyframes(writer);
                }
            }

            // String Table
            uint stringTablePos = (uint)fileStream.Position;
            writer.FillInOffset("stringTableOffset", false, false);
            writer.FillInOffset("keyframesSize", stringTablePos -
                keyframesOffset, true, false);

            foreach (string str in stringTable)
            {
                writer.WriteNullTerminatedString(str);
            }

            writer.FixPadding(4);
            writer.FillInOffset("stringTableSize", (uint)fileStream.Position -
                stringTablePos, true, false);

            writer.FinishWrite(Header, writeEOFNull: false);

            // Sub-Methods
            void AddStringToTable(string str)
            {
                writer.Write(stringTableSize);
                stringTableSize += str.Length + 1;
                stringTable.Add(str);
            }
        }

        public static GensAnimation ImportXML(string filePath)
        {
            using (var fs = File.OpenRead(filePath))
            {
                return ImportXML(fs);
            }
        }

        public static GensAnimation ImportXML(Stream fileStream)
        {
            var doc = XDocument.Load(fileStream);
            var root = doc.Root;

            GensAnimation anim;
            switch (root.Name.LocalName.ToLower())
            {
                case "uvanimation":
                    anim = new UVAnimation();
                    break;

                case "materialanimation":
                    anim = new MaterialAnimation();
                    break;

                case "visibilityanimation":
                    anim = new VisibilityAnimation();
                    break;

                //case "textureanimation":
                //    anim = new TextureAnimation();
                //    break;

                default:
                    throw new NotSupportedException(
                        $"Cannot yet import this type of animation. (\"{root.Name.LocalName}\")");
            }

            var versionAttr = root.Attribute("version");
            uint.TryParse(versionAttr?.Value, out anim.Header.RootNodeType);

            anim.ReadXML(root);
            return anim;
        }

        protected virtual void ReadXML(XElement root)
        {
            foreach (var anim in root.Elements("Animation"))
            {
                Animations.Add(new Animation(anim));
            }
        }

        protected string GetStringAttr(XAttribute attr)
        {
            return (attr == null || attr.Value == null) ?
                string.Empty : attr.Value;
        }

        public virtual void ExportXML(string filePath)
        {
            using (var fs = File.Create(filePath))
            {
                ExportXML(fs);
            }
        }

        public virtual void ExportXML(FileStream fileStream)
        {
            var root = new XElement("GensAnim", new XAttribute(
                "version", Header.RootNodeType));

            WriteXML(root);

            var doc = new XDocument(root);
            doc.Save(fileStream);
        }

        protected virtual void WriteXML(XElement root)
        {
            foreach (var anim in Animations)
            {
                root.Add(anim.GenerateXElement());
            }
        }

        // Other
        public class Animation
        {
            // Variables/Constants
            public List<KeyframeSet> KeyframeSets = new List<KeyframeSet>();
            public string BlendType = "default";
            public float FPS, StartTime, EndTime;
            public uint BlendTypeOffset { get; protected set; }

            // Constructors
            public Animation() { }
            public Animation(BinaryReader reader)
            {
                Read(reader);
            }

            public Animation(XElement elem)
            {
                ImportXElement(elem);
            }

            // Methods
            public void Read(BinaryReader reader)
            {
                BlendTypeOffset = reader.ReadUInt32(); // texsetNameOffset for Forces?
                FPS = reader.ReadSingle();
                StartTime = reader.ReadSingle();
                EndTime = reader.ReadSingle();

                // Keyframe Sets
                uint keyframeSetsCount = reader.ReadUInt32();
                for (uint i2 = 0; i2 < keyframeSetsCount; ++i2)
                {
                    KeyframeSets.Add(new KeyframeSet(reader));
                }
            }

            public void Write(BinaryWriter writer, bool wroteBlendTypeOffset = false)
            {
                if (!wroteBlendTypeOffset)
                    writer.Write(0U);

                writer.Write(FPS);
                writer.Write(StartTime);
                writer.Write(EndTime);

                // Keyframe Sets
                int startIndex = 0;
                writer.Write(KeyframeSets.Count);

                foreach (var set in KeyframeSets)
                {
                    set.Write(writer, startIndex);
                    startIndex += set.Count;
                }
            }

            public void ImportXElement(XElement elem)
            {
                var blendTypeAttr = elem.Attribute("blendType");
                var fpsAttr = elem.Attribute("fps");
                var startTimeAttr = elem.Attribute("startTime");
                var endTimeAttr = elem.Attribute("endTime");

                BlendType = (blendTypeAttr == null ||
                    string.IsNullOrEmpty(blendTypeAttr.Value)) ?
                    "default" : blendTypeAttr.Value;

                float.TryParse(fpsAttr?.Value, out FPS);
                float.TryParse(startTimeAttr?.Value, out StartTime);
                float.TryParse(endTimeAttr?.Value, out EndTime);

                foreach (var set in elem.Elements("KeyframeSet"))
                {
                    KeyframeSets.Add(new KeyframeSet(set));
                }
            }

            public XElement GenerateXElement()
            {
                var elem = new XElement("Animation", 
                    new XAttribute("blendType", BlendType),
                    new XAttribute("fps", FPS),
                    new XAttribute("startTime", StartTime),
                    new XAttribute("endTime", EndTime));

                foreach (var set in KeyframeSets)
                {
                    elem.Add(set.GenerateXElement());
                }

                return elem;
            }
        }

        public class KeyframeSet : List<Keyframe>
        {
            // Variables/Constants
            public byte Flag1 = 1, Flag2 = 1, Flag3, Flag4;
            private uint keyframeCount, startIndex;

            // Constructors
            public KeyframeSet() { }
            public KeyframeSet(BinaryReader reader)
            {
                Read(reader);
            }

            public KeyframeSet(XElement elem)
            {
                ImportXElement(elem);
            }

            // Methods
            public void Read(BinaryReader reader)
            {
                Flag1 = reader.ReadByte();
                Flag2 = reader.ReadByte();
                Flag3 = reader.ReadByte();
                Flag4 = reader.ReadByte();

                keyframeCount = reader.ReadUInt32();
                startIndex = reader.ReadUInt32();
            }

            public void Write(BinaryWriter writer, int startIndex)
            {
                writer.Write(Flag1);
                writer.Write(Flag2);
                writer.Write(Flag3);
                writer.Write(Flag4);

                writer.Write(Count);
                writer.Write(startIndex);
            }

            public void ReadKeyframes(ExtendedBinaryReader reader, uint keyframesOffset)
            {
                ReadKeyframes(reader, keyframesOffset, reader.Offset);
            }

            public void ReadKeyframes(BinaryReader reader,
                uint keyframesOffset, uint offset)
            {
                reader.BaseStream.Position = (keyframesOffset + offset +
                    (startIndex * Keyframe.Size));

                for (uint i = 0; i < keyframeCount; ++i)
                {
                    Add(new Keyframe(reader));
                }
            }

            public void WriteKeyframes(BinaryWriter writer)
            {
                foreach (var keyframe in this)
                {
                    keyframe.Write(writer);
                }
            }

            public void ImportXElement(XElement elem)
            {
                var flag1Attr = elem.Attribute("flag1");
                var flag2Attr = elem.Attribute("flag2");
                var flag3Attr = elem.Attribute("flag3");
                var flag4Attr = elem.Attribute("flag4");

                byte.TryParse(flag1Attr?.Value, out Flag1);
                byte.TryParse(flag2Attr?.Value, out Flag2);
                byte.TryParse(flag3Attr?.Value, out Flag3);
                byte.TryParse(flag4Attr?.Value, out Flag4);

                foreach (var keyframe in elem.Elements("Keyframe"))
                {
                    Add(new Keyframe(keyframe));
                }
            }

            public XElement GenerateXElement()
            {
                var elem = new XElement("KeyframeSet",
                    new XAttribute("flag1", Flag1),
                    new XAttribute("flag2", Flag2),
                    new XAttribute("flag3", Flag3),
                    new XAttribute("flag4", Flag4));

                foreach (var keyframe in this)
                {
                    elem.Add(keyframe.GenerateXElement());
                }

                return elem;
            }
        }

        public class Keyframe
        {
            // Variables/Constants
            public float Index, Value;
            public const uint Size = 8;

            // Constructors
            public Keyframe() { }
            public Keyframe(BinaryReader reader)
            {
                Read(reader);
            }

            public Keyframe(XElement elem)
            {
                ImportXElement(elem);
            }

            // Methods
            public void Read(BinaryReader reader)
            {
                Index = reader.ReadSingle();
                Value = reader.ReadSingle();
            }

            public void Write(BinaryWriter writer)
            {
                writer.Write(Index);
                writer.Write(Value);
            }

            public void ImportXElement(XElement elem)
            {
                var indexAttr = elem.Attribute("index");
                var valueAttr = elem.Attribute("value");

                float.TryParse(indexAttr?.Value, out Index);
                float.TryParse(valueAttr?.Value, out Value);
            }

            public XElement GenerateXElement()
            {
                return new XElement("Keyframe",
                    new XAttribute("index", Index),
                    new XAttribute("value", Value));
            }
        }
    }
}