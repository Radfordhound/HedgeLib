using HedgeLib.Archives;
using System;
using System.IO;

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

            // Figure out the type of archive to repack if none was specified
            if (Type == Types.ArcType.Unknown)
            {
                // If an output was specified, simply figure out the type from that
                if (!string.IsNullOrWhiteSpace(Output))
                {
                    Type = Types.AutoDetectType(Output);
                }

                // Otherwise, try to guess it. If we fail, simply
                // ask the user what type to use.
                else
                {
                    Type = GuessRepackType();
                }
            }

            // Get Output if none was specified
            if (string.IsNullOrWhiteSpace(Output))
            {
                var dirInfo = new DirectoryInfo(Input);
                string parent = dirInfo.Parent?.FullName;

                // Try to auto-determine the Output path if none was specified
                if (string.IsNullOrEmpty(parent))
                {
                    Print("Output path could not be auto-determined.");
                    Print("Please manually specify an output path and press enter");

                    Output = Console.ReadLine();
                }
                else
                {
                    Output = Path.Combine(parent,
                        $"{dirInfo.Name}{Types.GetExtension(Type)}");
                }
            }

            // DEBUG
            PrintDebug("Archive Type: " + Type);

            if (Type == Types.ArcType.Unknown)
            {
                // TODO: Remove this check? This shouldn't be possible anymore.
                PrintError("Archive Type is Unknown, Please specify a type");
                return false;
            }

            // Creates the Archive
            var archive = Types.GetArchive(Type);
            if (archive == null)
                return false;

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

            uint? splitCount = (bool.Parse(Options["split"]) ?
                (uint?)uint.Parse(Options["splitsize"]) : null);

            if (archive is GensArchive gensArchive)
            {
                gensArchive.Save(Output, bool.Parse(
                    Options["createarl"]), splitCount);
            }
            else if (archive is ForcesArchive forcesArchive)
            {
                forcesArchive.Save(Output, splitCount);
            }
            else
            {
                archive.Save(Output, true);
            }

            Print("\nDone!");
            //Print("\nDone!      ");

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
                    var dir = Path.GetDirectoryName(Input);
                    var name = Path.GetFileName(Input);
                    string ext = Archive.GetSplitParentExtension(Input);

                    name = name.Substring(0, name.IndexOf(ext));
                    Output = Path.Combine(dir, name);
                    Console.WriteLine(Output);
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

                // Load the archive
                Print("Loading...");
                archive.Load(Input);

                // Extract the archive
                Print("Extracting...\r", false);

                // Extracts all the files with percent output
                for (int i = 0; i < archive.Data.Count; ++i)
                {
                    var data = archive.Data[i];
                    data.Extract(Path.Combine(Output, data.Name));
                    int percent = (int)(((float)i / archive.Data.Count) * 100f);
                    Print($"Extracting...\t{percent}%\r", false);
                }

                Print("\nDone!");
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
