using HedgeLib.Headers;
using HedgeLib.IO;
using System;
using System.IO;
using System.Text;
using System.Xml.Linq;

namespace HedgeLib.RFL
{
    public class ForcesCaptionParam : ForcesRFL
    {
        // Variables/Constants
        public Caption[] Captions = new Caption[CaptionCount];
        public const uint CaptionCount = 10;

        // Methods
        public override void Read(BINAReader reader, BINAHeader header = null)
        {
            for (uint i = 0; i < CaptionCount; ++i)
            {
                reader.FixPadding(8);
                Captions[i] = new Caption(reader);
            }
        }

        public override void Write(BINAWriter writer)
        {
            for (uint i = 0; i < CaptionCount; ++i)
            {
                writer.FixPadding(8);
                Captions[i].Write(writer, i);
            }
        }

        public override void ImportXML(XDocument xml)
        {
            Captions = new Caption[CaptionCount];

            XElement capElem;
            for (uint i = 0; i < CaptionCount; ++i)
            {
                capElem = xml.Root.Element($"Caption{i+1}");
                if (capElem != null)
                    Captions[i] = new Caption(capElem);
            }
        }

        public override void ExportXML(Stream fileStream)
        {
            // Comments
            var root = new XElement("CaptionParam");
            var sb = new StringBuilder();
            sb.AppendLine();
            sb.AppendLine(
                "You MUST have 10 captions per-rfl and 20 lines per caption. No more, no less.");
            sb.AppendLine(
                "This is limiting, but unfortunately there doesn't seem to be any way around it.");
            sb.AppendLine("#BlameIizuka");
            sb.AppendLine();

            sb.AppendLine(
                "If you want to use < 20 lines, just get rid of the others' Name attributes.");
            sb.AppendLine(
                "Any lines without name attributes will be treated by the game as \"blank\".");

            WriteEnumComment("Line Types:", typeof(Line.LineTypes));
            WriteEnumComment("Character Types:", typeof(Line.CharacterTypes));
            WriteEnumComment("Animation Types:", typeof(Line.AnimationTypes));

            root.Add(new XComment(sb.ToString()));

            // Data
            Caption cap;
            for (uint i = 0; i < CaptionCount;)
            {
                cap = Captions[i];
                root.Add(cap.GenerateXElement(++i));
            }

            ExportXML(fileStream, root);

            // Sub-Methods
            void WriteEnumComment(string txt, Type enumType)
            {
                var names = Enum.GetNames(enumType);
                sb.AppendLine();
                sb.AppendLine(txt);

                foreach (string name in names)
                {
                    sb.AppendLine($"\t{name} = {(int)Enum.Parse(enumType, name)}");
                }
            }
        }

        // Other
        public class Caption
        {
            // Variables/Constants
            public Line[] Lines = new Line[LineCount];
            public uint Unknown1;
            public const uint LineCount = 20;

            // Constructors
            public Caption() { }
            public Caption(BINAReader reader)
            {
                Read(reader);
            }

            public Caption(XElement elem)
            {
                ImportXElement(elem);
            }

            // Methods
            public void Read(BINAReader reader)
            {
                for (uint i = 0; i < LineCount; ++i)
                {
                    Lines[i] = new Line(reader);
                }

                Unknown1 = reader.ReadUInt32();
                if (Unknown1 != 0)
                    Console.WriteLine("WARNING: Caption Unknown1 != 0!! {0:X}", Unknown1);
            }

            public void Write(BINAWriter writer, uint index)
            {
                for (uint i = 0; i < LineCount; ++i)
                {
                    Lines[i].Write(writer, index, i);
                }

                writer.Write(Unknown1);
            }

            public void ImportXElement(XElement elem)
            {
                var unknown1Attr = elem.Attribute("unknown1");
                uint.TryParse(unknown1Attr?.Value, out Unknown1);

                XElement lnElem;
                Lines = new Line[LineCount];

                for (uint i = 0; i < LineCount; ++i)
                {
                    lnElem = elem.Element($"Line{i+1}");
                    if (lnElem != null)
                        Lines[i] = new Line(lnElem);
                }
            }

            public XElement GenerateXElement(uint index)
            {
                var elem = new XElement($"Caption{index}",
                    new XAttribute("unknown1", Unknown1));

                Line ln;
                uint endIndex = LineCount;

                for (uint i = 0; i < LineCount;)
                {
                    ln = Lines[i];
                    if (endIndex == LineCount && string.IsNullOrEmpty(ln.Name))
                        endIndex = i;

                    elem.Add(ln.GenerateXElement(++i));
                }

                if (endIndex == 0)
                {
                    elem.AddFirst(new XComment(
                        "All lines in this caption are blank. Game will play nothing."));
                }
                else
                {
                    elem.AddFirst(new XComment(string.Format(
                        "First line without a name is Line{0}. {1}-{2}.",
                        endIndex + 1, "So, Game will play lines 1", endIndex)));
                }

                return elem;
            }
        }

        public class Line
        {
            // Variables/Constants
            public string Name = null;
            public ulong TalkingCharacter = 0;
            public float UnknownFloat1;

            public LineTypes LineType = LineTypes.Subtitle;
            public CharacterTypes FirstCharacter = CharacterTypes.Tails;
            public CharacterTypes SecondCharacter = CharacterTypes.Tails;
            public AnimationTypes FirstCharAnimation = AnimationTypes.BaseTalk;
            public AnimationTypes SecondCharAnimation = AnimationTypes.BaseTalk;

            public enum LineTypes
            {
                Subtitle, Radio, SoundEffect
            }

            public enum CharacterTypes
            {
                Amy = 0, Charmy = 1, Eggman = 2,
                Espio = 3, Knuckles = 4, Rouge = 5,
                Shadow = 6, Silver = 7, Sonic = 8,
                Tails = 9, Vector = 10, Soldier = 11,
                BirdSoldier = 12, MustacheSoldier = 13,
                CustomHero = 14, ClassicSonic = 15,
                Infinite = 16, Omega = 17, None = 255
            }

            public enum AnimationTypes
            {
                AngryTalk = 0, BaseTalk = 1,
                SadTalk = 2, HappyTalk = 3,
                SurpriseTalk = 4, AngryIdle = 5,
                BaseIdle = 6, SadIdle = 7,
                HappyIdle = 8, SurpriseIdle = 9
            }

            // Constructors
            public Line() { }
            public Line(BINAReader reader)
            {
                Read(reader);
            }

            public Line(XElement elem)
            {
                ImportXElement(elem);
            }

            // Methods
            public void Read(BINAReader reader)
            {
                // Read the data
                uint lineType = reader.ReadUInt32();
                uint padding1 = reader.ReadUInt32();

                ulong nameOffset = reader.ReadUInt64();
                ulong padding2 = reader.ReadUInt64();
                UnknownFloat1 = reader.ReadSingle();
                byte firstCharacter = reader.ReadByte();
                byte firstAnim = reader.ReadByte();
                byte secondCharacter = reader.ReadByte();
                byte secondAnim = reader.ReadByte();

                TalkingCharacter = reader.ReadUInt64();

                // Parse it
                if (nameOffset != 0)
                    Name = reader.GetString((uint)nameOffset + reader.Offset);

                LineType = (LineTypes)lineType;
                FirstCharacter = (CharacterTypes)firstCharacter;
                FirstCharAnimation = (AnimationTypes)firstAnim;
                SecondCharacter = (CharacterTypes)secondCharacter;
                SecondCharAnimation = (AnimationTypes)secondAnim;
            }

            public void Write(BINAWriter writer, uint captionIndex, uint index)
            {
                writer.Write((uint)LineType);
                writer.Write(0);

                if (string.IsNullOrEmpty(Name))
                    writer.Write(0UL);
                else
                    writer.AddString($"Caption{captionIndex}Line{index}Name", Name, 8);

                writer.Write(0UL);
                writer.Write(UnknownFloat1);
                writer.Write((byte)FirstCharacter);
                writer.Write((byte)FirstCharAnimation);
                writer.Write((byte)SecondCharacter);
                writer.Write((byte)SecondCharAnimation);
                writer.Write(TalkingCharacter);
            }

            public void ImportXElement(XElement elem)
            {
                // Read the data
                var nameAttr = elem.Attribute("name");
                var typeAttr = elem.Attribute("type");
                var talkingCharacterAttr = elem.Attribute("talkingCharacter");
                var unknownFloat1Attr = elem.Attribute("unknownFloat1");

                var firstCharElem = elem.Element("FirstCharacter");
                var firstTypeAttr = firstCharElem?.Attribute("type");
                var firstAnimAttr = firstCharElem?.Attribute("animation");

                var secondCharElem = elem.Element("SecondCharacter");
                var secondTypeAttr = secondCharElem?.Attribute("type");
                var secondAnimAttr = secondCharElem?.Attribute("animation");

                // Parse it
                Name = nameAttr?.Value;
                ulong.TryParse(talkingCharacterAttr?.Value, out TalkingCharacter);
                float.TryParse(unknownFloat1Attr?.Value, out UnknownFloat1);

                Enum.TryParse(typeAttr?.Value, out LineType);
                Enum.TryParse(firstTypeAttr?.Value, out FirstCharacter);
                Enum.TryParse(firstAnimAttr?.Value, out FirstCharAnimation);
                Enum.TryParse(secondTypeAttr?.Value, out SecondCharacter);
                Enum.TryParse(secondAnimAttr?.Value, out SecondCharAnimation);
            }

            public XElement GenerateXElement(uint index)
            {
                // Attributes
                var elem = new XElement($"Line{index}");

                if (!string.IsNullOrEmpty(Name))
                    elem.Add(new XAttribute("name", Name));

                elem.Add(new XAttribute("type", Enum.GetName(typeof(LineTypes), LineType)));
                elem.Add(new XAttribute("talkingCharacter", TalkingCharacter));
                elem.Add(new XAttribute("unknownFloat1", UnknownFloat1));

                // Elements
                elem.Add(new XElement("FirstCharacter",
                    new XAttribute("type", Enum.GetName(
                        typeof(CharacterTypes), FirstCharacter)),
                    new XAttribute("animation", Enum.GetName(
                        typeof(AnimationTypes), FirstCharAnimation))));

                elem.Add(new XElement("SecondCharacter",
                    new XAttribute("type", Enum.GetName(
                        typeof(CharacterTypes), SecondCharacter)),
                    new XAttribute("animation", Enum.GetName(
                        typeof(AnimationTypes), SecondCharAnimation))));

                return elem;
            }
        }
    }
}