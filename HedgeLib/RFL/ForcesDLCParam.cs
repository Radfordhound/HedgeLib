using HedgeLib.Headers;
using HedgeLib.IO;
using System.IO;
using System.Xml.Linq;

namespace HedgeLib.RFL
{
    public class ForcesDLCParam : ForcesRFL
    {
        // Variables/Constants
        public string MiscPAC, UnknownPath1, StgMissionLUA, UnknownPath2;
        public Slot Unknown1, CustomizeBody, CustomizeHead, CustomizeGlass,
            Unknown2, CustomizeGlove, CustomizeShoes, CustomizePattern, EventMovie,
            GameScore, StageScore, Unknown3;

        // Methods
        public override void Read(BINAReader reader, BINAHeader header = null)
        {
            // Slots
            Unknown1 = new Slot(reader); // gadget_database?
            CustomizeBody = new Slot(reader);
            CustomizeHead = new Slot(reader);
            CustomizeGlass = new Slot(reader);
            Unknown2 = new Slot(reader); // customize_face?
            CustomizeGlove = new Slot(reader);
            CustomizeShoes = new Slot(reader);
            CustomizePattern = new Slot(reader);
            EventMovie = new Slot(reader);
            GameScore = new Slot(reader);
            StageScore = new Slot(reader);
            Unknown3 = new Slot(reader); // idk (padding?)

            // Paths
            ulong miscPACOffset = reader.ReadUInt64();
            ulong unknownPath1Offset = reader.ReadUInt64();
            ulong stgMissionLUAOffset = reader.ReadUInt64();
            ulong unknownPath2Offset = reader.ReadUInt64();

            MiscPAC = ReadPath(miscPACOffset);
            UnknownPath1 = ReadPath(unknownPath1Offset);
            StgMissionLUA = ReadPath(stgMissionLUAOffset);
            UnknownPath2 = ReadPath(unknownPath2Offset);

            // Sub-Methods
            string ReadPath(ulong offset)
            {
                if (offset == 0)
                    return null;

                reader.JumpTo((long)offset, false);
                return reader.ReadNullTerminatedString();
            }
        }

        public override void Write(BINAWriter writer)
        {
            // Slots
            WriteSlot(Unknown1);
            WriteSlot(CustomizeBody);
            WriteSlot(CustomizeHead);
            WriteSlot(CustomizeGlass);
            WriteSlot(Unknown2);
            WriteSlot(CustomizeGlove);
            WriteSlot(CustomizeShoes);
            WriteSlot(CustomizePattern);
            WriteSlot(EventMovie);
            WriteSlot(GameScore);
            WriteSlot(StageScore);
            WriteSlot(Unknown3);

            // Paths
            uint i = 0;
            WritePath(MiscPAC);
            WritePath(UnknownPath1);
            WritePath(StgMissionLUA);
            WritePath(UnknownPath2);

            writer.FinishWrite(Header);

            // Sub-Methods
            void WriteSlot(Slot slot)
            {
                if (slot == null)
                    writer.Write((ushort)0);
                else
                    slot.Write(writer);
            }

            void WritePath(string path)
            {
                if (string.IsNullOrEmpty(path))
                {
                    writer.Write(0UL);
                    return;
                }

                writer.AddString($"path{++i}", path, 8);
            }
        }

        // TODO: Add ImportXML function

        public override void ExportXML(Stream fileStream)
        {
            var root = new XElement("DLCParam");

            // Slots
            root.Add(new XComment("Slots"));
            WriteSlot("Unknown1", Unknown1);
            WriteSlot("customize_body", CustomizeBody);
            WriteSlot("customize_head", CustomizeHead);
            WriteSlot("customize_glass", CustomizeGlass);
            WriteSlot("Unknown2", Unknown2);
            WriteSlot("customize_glove", CustomizeGlove);
            WriteSlot("customize_shoes", CustomizeShoes);
            WriteSlot("customize_pattern", CustomizePattern);
            WriteSlot("event_movie", EventMovie);
            WriteSlot("game_score", GameScore);
            WriteSlot("stage_score", StageScore);
            WriteSlot("Unknown3", Unknown3);

            // Paths
            root.Add(new XComment("Paths"));
            WritePath("MiscPAC", MiscPAC);
            WritePath("UnknownPath1", UnknownPath1);
            WritePath("StgMissionLua", StgMissionLUA);
            WritePath("UnknownPath2", UnknownPath2);

            ExportXML(fileStream, root);

            // Sub-Methods
            void WriteSlot(string name, Slot slot)
            {
                root.Add(new XElement(name,
                    new XAttribute("startIndex", slot.StartIndex),
                    new XAttribute("length", slot.Length)));
            }

            void WritePath(string name, string path)
            {
                root.Add(new XElement(name, path));
            }
        }

        // Other
        public class Slot
        {
            // Variables/Constants
            public byte StartIndex, Length;

            // Constructors
            public Slot() { }
            public Slot(BinaryReader reader)
            {
                Read(reader);
            }

            // Methods
            public void Read(BinaryReader reader)
            {
                ushort slot = reader.ReadUInt16();
                StartIndex = (byte)(slot & 0xFF);
                Length = (byte)((slot & 0xFF00) >> 8);
            }

            public void Write(BinaryWriter writer)
            {
                writer.Write((ushort)((Length << 8) | StartIndex));
            }
        }
    }
}