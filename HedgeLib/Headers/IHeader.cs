using HedgeLib.IO;

namespace HedgeLib.Headers
{
    public interface IHeader
    {
        /// <summary>
        /// Reads the header.
        /// </summary>
        /// <param name="reader">The ExtendedBinaryReader to use for readingdata.</param>
        void Read(ExtendedBinaryReader reader);

        /// <summary>
        /// Should be used before writing anything else to the file.
        /// Writes nulls to be filled in later with header data by FinishWrite, and
        /// sets the offset of the given writer if necessary.
        /// </summary>
        /// <param name="writer">The ExtendedBinaryWriter to use for writing data.</param>
        void PrepareWrite(ExtendedBinaryWriter writer);

        /// <summary>
        /// Should be used after all other data has been written to the file, including the footer.
        /// Writes the actual header data.
        /// NOTE: Does not automatically jump back to the beginning of the stream!
        /// </summary>
        /// <param name="writer">The ExtendedBinaryWriter to use for writing data.</param>
        void FinishWrite(ExtendedBinaryWriter writer);
    }
}