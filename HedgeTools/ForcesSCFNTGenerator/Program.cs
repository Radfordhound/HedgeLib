using HedgeLib.IO;
using HedgeLib.Headers;
using System.IO;

namespace ForcesSCFNTGenerator
{
    class Program
    {
        public static BINAHeader Header = new BINAHeader() { Version = 210 };

        static void Main(string[] args)
        {
            FileInfo fileInfo = new FileInfo(args[0]);
            var data = File.ReadAllBytes(fileInfo.FullName);
            using (var stream = File.OpenWrite(Path.ChangeExtension(fileInfo.FullName, "scfnt")))
            {
                BINAWriter writer = new BINAWriter(stream, BINA.BINATypes.Version2, false, true);
                writer.WriteSignature("KFCS1000");
                writer.Write((long)data.Length);
                writer.WriteNulls(0x10);
                writer.Write((long)data.Length);
                writer.Write(data);
                writer.FinishWrite(Header);
            }
        }
    }
}