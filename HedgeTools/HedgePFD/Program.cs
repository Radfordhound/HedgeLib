using HedgeLib.Archives;
using HedgeLib.Misc;
using System;
using System.IO;

namespace HedgePFD
{
    public class Program
    {
        public static void Main(string[] args)
        {
            if (args.Length < 1)
            {
                ShowHelp();
                return;
            }

            // Get input directory
            string input = args[0];
            if (!Directory.Exists(input))
            {
                ShowHelp();
                return;
            }

            // Get output directory
            string output = (args.Length > 1) ?
                args[1] : null;

            // Get padding
            uint padding = GensArchive.PFDPadding;
            if (args.Length > 2)
                uint.TryParse(args[2], out padding);

            // Pack PFD and generate PFI
            GensPFI.PackPFD(input, output);
        }

        public static void ShowHelp()
        {
            Console.WriteLine("HedgePFD input [output] [padding]");
            Console.WriteLine("By: Radfordhound");

            Console.WriteLine();
            Console.WriteLine("Arguments (arguments surrounded by square brackets are optional):");
            Console.WriteLine("- input:\tThe directory to pack into a PFD file");
            Console.WriteLine("- [output]:\tThe directory to save the PFD and PFI files.");
            Console.WriteLine("- [padding]:\tThe amount of padding to use on the generated PFD.");
            Console.WriteLine("  \t\t(Defaults to 2048 if no padding is specified.)");

            Pause();
        }

        public static void Pause()
        {
            Console.WriteLine();
            Console.WriteLine("Press any key to continue...");
            Console.ReadLine();
        }
    }
}