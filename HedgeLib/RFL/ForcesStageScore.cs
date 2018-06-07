using HedgeLib.Headers;
using HedgeLib.IO;
using System;
using System.Collections.Generic;
using System.IO;
using System.Xml.Linq;

namespace HedgeLib.RFL
{
    public class ForcesStageScore : ForcesRFL
    {
        // Variables/Constants
        public List<StageEntry> StageEntries = new List<StageEntry>();
        private static int i = 0;

        // Methods
        public override void Read(BINAReader reader, BINAHeader header = null)
        {
            // Stage Entries
            uint stopOffset = ((header as BINAv2Header).StringTableOffset + reader.Offset);
            StageEntries.Clear();

            while (reader.BaseStream.Position < stopOffset)
            {
                StageEntries.Add(new StageEntry(reader));
            }

            // String Table
            foreach (var entry in StageEntries)
            {
                reader.JumpTo((long)entry.StageIDOffset, false);
                entry.StageID = reader.ReadNullTerminatedString();
            }
        }

        public override void Write(BINAWriter writer)
        {
            i = 0;

            // Stage Entries
            foreach (var entry in StageEntries)
            {
                entry.Write(writer);
            }
        }

        public override void ImportXML(XDocument xml)
        {
            foreach (var elem in xml.Root.Elements("StageEntry"))
            {
                StageEntries.Add(new StageEntry(elem));
            }
        }

        public override void ExportXML(Stream fileStream)
        {
            var root = new XElement("StageScore");
            root.Add(new XComment(
                "TimeBonus is calculated like this: TimeBasis - (TimeDown * [TimeInSeconds])"));
            root.Add(new XComment("Credit to ĐeäTh for figuring this out!"));

            foreach (var entry in StageEntries)
            {
                root.Add(entry.GenerateXElement());
            }

            ExportXML(fileStream, root);
        }

        // Other
        public class StageEntry
        {
            // Variables/Constants
            public string StageID;
            public ulong StageIDOffset => stageIDOffset;
            public uint TimeBasis, TimeDown, NoMissBonus, OneMissBonus,
                TwoMissBonus, ThreeMissBonus, FourMissBonus, FivePlusMissBonus,
                SRankScore, ARankScore, BRankScore;

            public byte StageIndex;
            protected ulong stageIDOffset = 0;

            // Constructors
            public StageEntry() { }
            public StageEntry(BINAReader reader)
            {
                Read(reader);
            }

            public StageEntry(XElement elem)
            {
                ImportXElement(elem);
            }

            // Methods
            public void Read(BINAReader reader, bool readStageID = false)
            {
                // Stage Index
                StageIndex = reader.ReadByte();
                byte unknown2 = reader.ReadByte();
                byte unknown3 = reader.ReadByte();
                byte unknown4 = reader.ReadByte();

                // Time Bonus
                TimeBasis = reader.ReadUInt32();
                TimeDown = reader.ReadUInt32();

                // Miss Bonuses
                NoMissBonus = reader.ReadUInt32();
                OneMissBonus = reader.ReadUInt32();
                TwoMissBonus = reader.ReadUInt32();
                ThreeMissBonus = reader.ReadUInt32();
                FourMissBonus = reader.ReadUInt32();
                FivePlusMissBonus = reader.ReadUInt32();

                // Rank Scores
                SRankScore = reader.ReadUInt32();
                ARankScore = reader.ReadUInt32();
                BRankScore = reader.ReadUInt32();

                uint CRankScore = reader.ReadUInt32(); // Not really used
                uint DRankScore = reader.ReadUInt32(); // Unused

                // Stage ID
                stageIDOffset = reader.ReadUInt64();
                ulong unknown5 = reader.ReadUInt64();

                if (readStageID)
                    StageID = reader.GetString((uint)stageIDOffset);

                // Padding/Unknown Value Checks
                if (unknown2 != 0)
                    Console.WriteLine($"WARNING: Unknown2 != 0! ({unknown2})");

                if (unknown3 != 0)
                    Console.WriteLine($"WARNING: Unknown3 != 0! ({unknown3})");

                if (unknown4 != 0)
                    Console.WriteLine($"WARNING: Unknown4 != 0! ({unknown4})");

                if (unknown5 != 0)
                    Console.WriteLine($"WARNING: Unknown5 != 0! ({unknown5})");
            }

            public void Write(BINAWriter writer)
            {
                // Stage Index
                writer.Write(StageIndex);
                writer.WriteNulls(3);

                // Time Bonus
                writer.Write(TimeBasis);
                writer.Write(TimeDown);

                // Miss Bonuses
                writer.Write(NoMissBonus);
                writer.Write(OneMissBonus);
                writer.Write(TwoMissBonus);
                writer.Write(ThreeMissBonus);
                writer.Write(FourMissBonus);
                writer.Write(FivePlusMissBonus);

                // Rank Scores
                writer.Write(SRankScore);
                writer.Write(ARankScore);
                writer.Write(BRankScore);
                writer.Write(0); // C Rank (not really used)
                writer.Write(0); // D Rank (unused)

                // Stage ID
                if (!string.IsNullOrEmpty(StageID))
                    writer.AddString($"stageID{++i}", StageID, 8);
                else
                    writer.Write(0UL);

                writer.Write(0UL);
            }

            public void ImportXElement(XElement elem)
            {
                // Stage ID and Index
                var stageIndexAttr = elem.Attribute("StageIndex");
                var stageIDAttr = elem.Attribute("StageID");

                StageID = stageIDAttr?.Value;
                byte.TryParse(stageIndexAttr?.Value, out StageIndex);

                // Time
                var timeBasisElem = elem.Element("TimeBasis");
                var timeDownElem = elem.Element("TimeDown");

                uint.TryParse(timeBasisElem?.Value, out TimeBasis);
                uint.TryParse(timeDownElem?.Value, out TimeDown);

                // Miss Bonuses
                var noMissBonusElem = elem.Element("NoMissBonus");
                var oneMissBonusElem = elem.Element("OneMissBonus");
                var twoMissBonusElem = elem.Element("TwoMissBonus");
                var threeMissBonusElem = elem.Element("ThreeMissBonus");
                var fourMissBonusElem = elem.Element("FourMissBonus");
                var fivePlusMissBonusElem = elem.Element("FivePlusMissBonus");

                uint.TryParse(noMissBonusElem?.Value, out NoMissBonus);
                uint.TryParse(oneMissBonusElem?.Value, out OneMissBonus);
                uint.TryParse(twoMissBonusElem?.Value, out TwoMissBonus);
                uint.TryParse(threeMissBonusElem?.Value, out ThreeMissBonus);
                uint.TryParse(fourMissBonusElem?.Value, out FourMissBonus);
                uint.TryParse(fivePlusMissBonusElem?.Value, out FivePlusMissBonus);

                // Rank Scores
                var SRankScoreElem = elem.Element("SRankScore");
                var ARankScoreElem = elem.Element("ARankScore");
                var BRankScoreElem = elem.Element("BRankScore");

                uint.TryParse(SRankScoreElem?.Value, out SRankScore);
                uint.TryParse(ARankScoreElem?.Value, out ARankScore);
                uint.TryParse(BRankScoreElem?.Value, out BRankScore);
            }

            public XElement GenerateXElement()
            {
                var elem = new XElement("StageEntry",
                    new XAttribute("StageIndex", StageIndex),
                    new XAttribute("StageID", StageID));

                // Time
                elem.Add(new XComment("Time"));
                elem.Add(new XElement("TimeBasis", TimeBasis));
                elem.Add(new XElement("TimeDown", TimeDown));

                // Miss Bonuses
                elem.Add(new XComment("Miss Bonuses"));
                elem.Add(new XElement("NoMissBonus", NoMissBonus));
                elem.Add(new XElement("OneMissBonus", OneMissBonus));
                elem.Add(new XElement("TwoMissBonus", TwoMissBonus));
                elem.Add(new XElement("ThreeMissBonus", ThreeMissBonus));
                elem.Add(new XElement("FourMissBonus", FourMissBonus));
                elem.Add(new XElement("FivePlusMissBonus", FivePlusMissBonus));

                // Rank Scores
                elem.Add(new XComment("Rank Scores"));
                elem.Add(new XElement("SRankScore", SRankScore));
                elem.Add(new XElement("ARankScore", ARankScore));
                elem.Add(new XElement("BRankScore", BRankScore));

                return elem;
            }
        }
    }
}