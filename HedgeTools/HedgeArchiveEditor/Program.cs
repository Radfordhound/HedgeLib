using HedgeLib.Archives;
using Microsoft.Win32;
using System;
using System.IO;
using System.Windows.Forms;

namespace HedgeArchiveEditor
{
    public static class Program
    {
        // Variables/Constants
        public static MainFrm MainForm;
        public const string ProgramName = "Hedge Archive Editor";

        // Methods
        [STAThread]
        public static void Main(string[] args)
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            // Load Addons
            if (Directory.Exists("Addons"))
                Addon.LoadAddons(new DirectoryInfo("Addons").FullName);


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
                                // Load the archive
                                FileInfo fileInfo = new FileInfo(args[1]);
                                var arc = LoadArchive(args[1]);

                                // Extract it's contents
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

                    case "/fa":
                        if (args.Length < 2)
                        {
                            ShowHelp();
                            break;
                        }
                             if (args[1].ToLower() == GensArchive.ListExtension)
                            SetFileAssociation(GensArchive.ListExtension, "GensARLArchive", "Sonic Generations Archive");
                        else if (args[1].ToLower() == GensArchive.PFDExtension)
                            SetFileAssociation(GensArchive.PFDExtension, "GensPFDArchive", "Sonic Generations Archive");
                        else if (args[1].ToLower() == LWArchive.Extension)
                            SetFileAssociation(LWArchive.Extension, "LWPACArchive", "Sonic Lost World Archive");
                        else if (args[1].ToLower() == ONEArchive.Extension)
                            SetFileAssociation(ONEArchive.Extension, "ONEArchive", "Sonic Heroes ONE Archive");
                        else if (args[1].ToLower() == SBArchive.Extension)
                            SetFileAssociation(SBArchive.Extension, "SBArchive", "Story Book ONE Archive");
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

            // Checks if the file even exist.
            if (!File.Exists(fileInfo.FullName))
                throw new FileNotFoundException("The given archive does not exist.");
            
            // Checks if the file is not empty.
            if (fileInfo.Length == 0)
                throw new Exception("The given file is empty.");

            // Addons
            foreach (var addon in Addon.Addons)
                foreach (var archive in addon.Archives)
                    if (archive.FileExtensions.Contains(fileInfo.Extension))
                    {
                        arc = Activator.CreateInstance(archive.ArchiveType) as Archive;
                        arc.Load(filePath);
                        return arc;
                    }

            // TODO: Add support for other types of archive.
            if (fileInfo.Extension == GensArchive.Extension ||
                fileInfo.Extension == GensArchive.SplitExtension ||
                fileInfo.Extension == GensArchive.PFDExtension ||
                fileInfo.Extension == GensArchive.ListExtension)
            {
                arc = new GensArchive();
            }
            else if (fileInfo.Extension == ForcesArchive.Extension)
            {
                arc = new ForcesArchive();
            }
            else if (fileInfo.Extension == ONEArchive.Extension)
            {
                // I know This is a horrible way of checking between archives.
                if (File.ReadAllBytes(fileInfo.FullName)[3] == 0x00)
                    arc = new ONEArchive();
                else arc = new SBArchive();
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

                    "/FA .{file extension} " + Environment.NewLine +
                    "\t (Sets the file association for the given file.)" +
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

        public static void SetFileAssociation(string extension, string typeName, string typeDisplayName)
        {
            string typeKey = "HKEY_CURRENT_USER\\Software\\Classes\\" + typeName;
            string extensionKey = "HKEY_CURRENT_USER\\Software\\Classes\\" + extension;

            if (Registry.GetValue(typeKey + "\\shell\\open\\command", "", "") as string
                != (Application.ExecutablePath + " \"%1\""))
            {
                if (MessageBox.Show($"Change file association for {extension} to {Application.ExecutablePath}?",
                    ProgramName, MessageBoxButtons.YesNo) == DialogResult.Yes)
                {
                    // HKEY_CURRENT_USER\Software\Classes\{typeName}\(Default) = typeDisplayName
                    Registry.SetValue(typeKey, "", typeDisplayName);
                    // HKEY_CURRENT_USER\Software\Classes\{typeName}\shell\open\command\(Default) = Application.ExecutablePath + " "%1""
                    Registry.SetValue(typeKey + "\\shell\\open\\command", "", Application.ExecutablePath + " \"%1\"");
                    // HKEY_CURRENT_USER\Software\Classes\{extension}\(Default) = typeName
                    Registry.SetValue(extensionKey, "", typeName);

                    // Notifies the system that an application has changed the file associations.
                    long SHCNE_ASSOCCHANGED = 0x8000000;
                    SHChangeNotify(SHCNE_ASSOCCHANGED, 0, IntPtr.Zero, IntPtr.Zero);
                }
            }
        }

        [System.Runtime.InteropServices.DllImport("Shell32.dll")]
        public static extern void SHChangeNotify(long eventId, uint flags, IntPtr dwItem1, IntPtr dwItem2);

    }
}