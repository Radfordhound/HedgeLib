using HedgeLib.Archives;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HedgeArcPack
{
    public partial class Commands
    {
        // RepackArchive Command
        public static bool RepackArchive()
        {
            // Check if Input is a Directory
            if (!Directory.Exists(Input))
                return false;

            if (Type == Types.ArcType.Unknown)
                Type = GuessArchiveType();

            if (Type == Types.ArcType.Unknown)
                Type = Types.AutoDetectType(Input);

            // DEBUG
            PrintDebug("Archive Type: " + Type);

            if (Type == Types.ArcType.Unknown)
            {
                PrintError("Archive Type is Unknown, Please specify a type");
                return false;
            }

            // Creates the Archive
            var archive = Types.GetArchive(Type);
            if (archive == null)
                return false;

            // Get the output filePath if none was specified
            if (string.IsNullOrEmpty(Output))
            {
                var dirInfo = new DirectoryInfo(Input);
                string parent = dirInfo.Parent?.FullName;

                if (string.IsNullOrEmpty(parent))
                {
                    Print("Output path could not be auto-detected.");
                    Print("Please manually specify an output path and press enter");

                    Output = Console.ReadLine();
                    return RepackArchive(); // Please no Stack Overflows
                }
                else
                {
                    Output = Path.Combine(parent, $"{dirInfo.Name}{Types.GetExtension(Type)}");
                }
            }

            // DEBUG
            PrintDebug("Archive: " + archive);
            PrintDebug("Output: " + Output);

            // Options
            if (Options.Count != 0)
            {
                if (archive is GensArchive gensArchiveOp)
                {
                    gensArchiveOp.Padding = uint.Parse(Options["padding"]);
                }
                else if (archive is ONEArchive oneArchiveOp)
                {
                    oneArchiveOp.Magic = uint.Parse(Options["magic"]);
                }
            }

            // Repack the archive
            Print("Repacking... Adding Files\r", false);

            archive.AddDirectory(Input); // TODO: Add an option for including sub-dirs
            Print("Repacking... Saving      \r", false);

            if (archive is GensArchive genArchive)
                genArchive.Save(Output, bool.Parse(Options["createarl"]),
                    (bool.Parse(Options["split"]) ?
                    (uint?)uint.Parse(Options["splitsize"]) : null));
            else
                archive.Save(Output, true);

            Print("\nDone!      ");

            return true;
        }

        // ExtractArchive Command
        public static bool ExtractArchive()
        {
            try
            {
                // Auto-Detect the Archive Type if Unknown
                if (Type == Types.ArcType.Unknown)
                    Type = Types.AutoDetectType(Input);

                // DEBUG
                PrintDebug("Archive Type: " + Type);

                // Create an output path if not specified
                if (string.IsNullOrEmpty(Output))
                {
                    Output = Path.ChangeExtension(Input, null);
                    if (Path.HasExtension(Output))
                        Output = Path.ChangeExtension(Output, null);
                }

                // DEBUG
                PrintDebug("Output: " + Output);

                // Displays warning prompt if the output directory already exists
                if (Directory.Exists(Output))
                {
                    Print("NOTE: You can hide user inputs prompts with -Q");
                    PrintWarning("The given output directory already exists! " +
                        "Directory contents will be merged.");

                    if (!YesNo("Would you like to extract the given archive anyway?"))
                        return true;
                }

                // Create Output Directory if it doesn't exist
                if (!Directory.Exists(Output))
                    Directory.CreateDirectory(Output);

                var archive = Types.GetArchive(Type);

                // DEBUG
                PrintDebug("Archive: " + archive);

                // Loads the archive
                Print("Loading...\r", false);
                archive.Load(Input);
                // Extract the archive
                Print("Extracting...\r", false);

                // Extracts all the files with percent output
                for (int i = 0; i < archive.Data.Count; ++i)
                {
                    var data = archive.Data[i];
                    data.Extract(Path.Combine(Output, data.Name));
                    int percent = (int)(((float)i / archive.Data.Count) * 100f);
                    Print($"{percent}%\r", false);
                }

                Print("Done!");
                return true;
            }
            catch (Exception ex)
            {
                PrintError(ex.ToString());
                return false;
            }
        }
    }
}
