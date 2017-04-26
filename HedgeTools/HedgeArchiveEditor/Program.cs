using HedgeLib.Archives;
using System;
using System.IO;
using System.Windows.Forms;

namespace HedgeArchiveEditor
{
    public static class Program
    {
        //Variables/Constants
        public static MainFrm MainForm;
        public const string ProgramName = "Hedge Archive Editor";

        //Methods
        [STAThread]
        public static void Main(string[] args)
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            if (args.Length < 1)
                ShowGUI();
            else
            {
                switch (args[0].ToLower())
                {
                    case "/e":
                        {
                            if (args.Length < 2)
                            {
                                ShowHelp();
                                break;
                            }

                            try
                            {
                                //Load the archive
                                FileInfo fileInfo = new FileInfo(args[1]);
                                var arc = LoadArchive(args[1]);

                                //Extract it's contents
                                var dir = (args.Length < 3) ?
                                    Path.Combine(fileInfo.DirectoryName,
                                        fileInfo.Name.Substring(0, fileInfo.Name.Length -
                                        fileInfo.Extension.Length))
                                    : args[2];

                                Directory.CreateDirectory(dir);
                                arc.Extract(dir);
                                // Prints the working directory.
                            }
                            catch (Exception ex)
                            {
                                MessageBox.Show(ex.Message, ProgramName,
                                    MessageBoxButtons.OK, MessageBoxIcon.Error);
                            }

                            break;
                        }

                    case "/temp":
                        System.Diagnostics.Process.Start("explorer", $"\"{MainFrm.tempPath}\"");
                        break;

                    case "/?":
                        ShowHelp();
                        break;

                    default:
                        {
                            if (!File.Exists(args[0]))
                            {
                                ShowHelp();
                                break;
                            }

                            MainForm = new MainFrm();
                            MainForm.OpenArchive(args[0]);
                            MainForm.RefreshGUI();

                            Application.Run(MainForm);
                            break;
                        }
                }
            }
        }

        public static Archive LoadArchive(string filePath)
        {
            FileInfo fileInfo = new FileInfo(filePath);
            Archive arc = null;

            //TODO: Add support for other types of archive.
            if (fileInfo.Extension == GensArchive.Extension ||
                fileInfo.Extension == GensArchive.SplitExtension ||
                fileInfo.Extension == GensArchive.PFDExtension ||
                fileInfo.Extension == GensArchive.ListExtension)
            {
                arc = new GensArchive();
            }
            else if (fileInfo.Extension == LWArchive.Extension)
            {
                arc = new LWArchive();
            }
            else if (fileInfo.Extension == SBArchive.Extension)
            {
                arc = new SBArchive();
            }
            else
                throw new Exception("The given archive has an unknown extension.");

            arc.Load(filePath);
            return arc;
        }

        public static void ShowHelp()
        {
            MessageBox.Show("Command-line Arguments:" + Environment.NewLine + Environment.NewLine +
                    "/E archive [directory]" + Environment.NewLine +
                    "\t (Extracts the given archive to the given directory.)" +
                    Environment.NewLine +

                     "/TEMP " + Environment.NewLine +
                    "\t (Opens HedgeArchiveEditor's Temp folder in Explorer.)" +
                    Environment.NewLine +

                    "/?" + Environment.NewLine +
                    "\t (Shows this help.)",
                    
                    ProgramName, MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        public static void ShowGUI()
        {
            MainForm = new MainFrm();
            Application.Run(MainForm);
        }

    }
}