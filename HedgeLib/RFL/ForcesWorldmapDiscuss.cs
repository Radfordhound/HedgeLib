using HedgeLib.Headers;
using HedgeLib.IO;
using System;

namespace HedgeLib.RFL
{
    public class ForcesWorldmapDiscuss : ForcesRFL
    {
        // Variables/Constants
        public Entry[] Entries = new Entry[EntryCount];
        public const uint EntryCount = 50;

        // Methods
        public override void Read(BINAReader reader, BINAHeader header = null)
        {
            for (uint i = 0; i < EntryCount; ++i)
            {
                Entries[i] = new Entry(reader);
            }
        }

        // Other
        public class Entry
        {
            // Variables/Constants
            public string Name, StageStartCaptionParam, StageEndCaptionParam;

            // Constructors
            public Entry() { }
            public Entry(BINAReader reader)
            {
                Read(reader);
            }

            // Methods
            public void Read(BINAReader reader)
            {
                ulong nameOffset = reader.ReadUInt64();
                ulong padding1 = reader.ReadUInt64();
                ulong startOffset = reader.ReadUInt64();
                ulong padding2 = reader.ReadUInt64();
                ulong endOffset = reader.ReadUInt64();
                ulong padding3 = reader.ReadUInt64();

                // Read strings
                if (nameOffset != 0)
                    Name = reader.GetString((uint)nameOffset + reader.Offset);

                if (startOffset != 0)
                    StageStartCaptionParam = reader.GetString((uint)startOffset + reader.Offset);

                if (endOffset != 0)
                    StageEndCaptionParam = reader.GetString((uint)endOffset + reader.Offset);

                // Padding Checks
                if (padding1 != 0)
                    Console.WriteLine("WARNING: Padding1 != 0! ({0:X})", padding1);

                if (padding2 != 0)
                    Console.WriteLine("WARNING: Padding2 != 0! ({0:X})", padding2);

                if (padding3 != 0)
                    Console.WriteLine("WARNING: Padding3 != 0! ({0:X})", padding3);
            }
        }
    }
}