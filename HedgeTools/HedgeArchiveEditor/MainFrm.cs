using HedgeLib.Archives;
using System;
using System.Linq;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Windows.Forms;

namespace HedgeArchiveEditor
{
    public partial class MainFrm : Form
    {
        // Variables/Constants
        public static string tempPath = Path.Combine(Path.GetTempPath(), "HedgeArchiveEditor\\");

        public Dictionary<Archive, string> ArchiveFilePaths = new Dictionary<Archive, string>();
        public Dictionary<Archive, ArchiveDirectory> ArchiveCurrentDir = new Dictionary<Archive, ArchiveDirectory>();

        public List<Archive> Archives = new List<Archive>();

        public Archive CurrentArchive
        {
            get
            {
                return (tabControl.SelectedIndex >= 0 &&
                    tabControl.SelectedIndex < Archives.Count) ?
                    Archives[tabControl.SelectedIndex] : null;
            }

            set
            {
                if (tabControl.SelectedIndex >= 0 && tabControl.SelectedIndex < Archives.Count)
                    Archives[tabControl.SelectedIndex] = value;
            }
        }

        private bool extracting, extracted = false;

        // Constructors
        public MainFrm()
        {
            InitializeComponent();
            UpdateTitle();
            Directory.CreateDirectory(tempPath);
        }

        // Methods
        public void UpdateTitle()
        {
            Text = ((tabControl.TabPages.Count > 0) ?
                $"{tabControl.SelectedTab.Text} - " : "") + Program.ProgramName;
        }

        public string GetFilters(bool includeAllArchives)
        {
            string filters = "";

            if (includeAllArchives)
            {
                filters += "All Supported Archives (*.ar, *.arl, *.pfd, *.pac, *.one";

                // Addons
                foreach (var addon in Addon.Addons)
                    foreach (var archive in addon.Archives)
                        foreach (string ext in archive.FileExtensions)
                            filters += $", *{ext}";
                filters += ")|*.ar;*.arl;*.pfd;*.pac;*.one";

                // Addons
                foreach (var addon in Addon.Addons)
                    foreach (var archive in addon.Archives)
                        foreach (string ext in archive.FileExtensions)
                            filters += $";*{ext}";
            }

            // Generations/Unleashed
            filters += "|Generations/Unleashed Archives (*.ar, *.arl, *.pfd)|*.ar;*.arl;*.pfd";
            // Lost World
            filters += "|Sonic Forces Archives (*.pac)|*.pac";
            // StoryBooks
            filters += "|StoryBook Series Archives (*.one)|*.one";
            // Heroes/Shadow
            filters += "|Heroes Archives (*.one)|*.one";
            // Addons
            filters += AddFiltersFromAddons();
            // All Files
            filters += "|All Files (*.*)|*.*";

            if (!includeAllArchives)
                filters = filters.Substring(1);
            return filters;
        }

        public void OpenArchive(string filePath)
        {
            try
            {
                var archive = Program.LoadArchive(filePath);
                Archives.Add(archive);
                ArchiveFilePaths.Add(archive, filePath);
                ArchiveCurrentDir.Add(archive, null);
                archive.Saved = true;
                AddTabPage(new FileInfo(filePath).Name);
            }catch (Exception ex)
            {
                MessageBox.Show(ex.Message, Program.ProgramName,
                        MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            RefreshGUI();
        }

        public void SaveArchive(int index, bool saveAs)
        {
            string fileLocation = "";
            int ArchiveType = -1;
            var archive = Archives[index];

            if (ArchiveFilePaths[archive] == null || saveAs)
            {
                var sfd = new SaveFileDialog()
                {
                    Title = "Save Archive As...",
                    Filter = GetFilters(false)
                };

                if (sfd.ShowDialog() == DialogResult.OK)
                {
                    ArchiveType = sfd.FilterIndex - 1;
                    fileLocation = sfd.FileName;
                    ArchiveFilePaths[archive] = fileLocation;
                }
                else return;
            }
            else
            {
                fileLocation = ArchiveFilePaths[archive];

                // These checks may not work.
                var type = Archives[index].GetType();
                if (type == typeof(GensArchive)) ArchiveType = 0;
                else if (type == typeof(ForcesArchive)) ArchiveType = 1;
                else if (type == typeof(SBArchive)) ArchiveType = 2;
                else if (type == typeof(ONEArchive)) ArchiveType = 3;
            }

            if (ArchiveType == -1)
            {
                if (fileLocation.EndsWith(GensArchive.ListExtension) ||
                    fileLocation.EndsWith(GensArchive.Extension) ||
                    fileLocation.EndsWith(GensArchive.SplitExtension) ||
                    fileLocation.EndsWith(GensArchive.PFDExtension))
                    ArchiveType = 0; // Generations/Unleashed
                else if (fileLocation.EndsWith(ForcesArchive.Extension))
                    ArchiveType = 1; // Lost World
                else if (fileLocation.EndsWith(SBArchive.Extension))
                    ArchiveType = 2; // Story Books
                else if (fileLocation.EndsWith(ONEArchive.Extension)) // NOTE: This never gets called
                    ArchiveType = 3; // Heroes/Shadow

                // Addons
                if (ArchiveType == -1)
                {
                    int i = 3;
                    foreach (var addon in Addon.Addons)
                        foreach (var addonArchive in addon.Archives)
                        {
                            i++;
                            if (addonArchive.FileExtensions.Contains
                                (Path.GetExtension(fileLocation.ToLower())))
                                ArchiveType = i;
                        }
                }
            }
            
            var saveOptions = new SaveOptions(ArchiveType);

            // Automatically set the Magic value if its an ONEArchive
            if (archive.GetType() == typeof(ONEArchive))
                saveOptions.NumericUpDown1.Value = ((ONEArchive)archive).Magic;

            if (saveOptions.ShowDialog() == DialogResult.OK && saveOptions.ArchiveType != -1)
            {
                // This is a horrible way of checking this, I know
                switch (saveOptions.ComboBox1.SelectedIndex)
                {
                    // Generations/Unleashed
                    case 0:
                        uint? splitAmount = (saveOptions.CheckBox2.Checked) ?
                            (uint?)saveOptions.NumericUpDown2.Value : null;
                        var genArc = new GensArchive(archive)
                        {
                            Padding = (uint)saveOptions.NumericUpDown1.Value
                        };
                        if (saveOptions.CheckBox3.Checked && saveOptions.CheckBox2.Checked)
                            genArc.GetSplitArchivesList(fileLocation)
                                .ForEach(file => File.Delete(file));
                        genArc.Save(fileLocation, saveOptions.CheckBox1.Checked, splitAmount);
                        break;
                    // Forces
                    case 1:
                        var fArc = new ForcesArchive(archive);
                        fArc.Save(fileLocation, true);
                        break;
                    // Story Books
                    case 2:
                        var sbArc = new SBArchive(archive);
                        sbArc.Save(fileLocation, true);
                        break;
                    // Heroes/Shadow
                    case 3:
                        var oneArc = new ONEArchive(archive)
                        {
                            Magic = (uint)saveOptions.NumericUpDown1.Value
                        };
                        oneArc.Save(fileLocation, true);
                        break;
                    default:
                        int archiveIndex = saveOptions.ComboBox1.SelectedIndex - 4;
                        int i = 0;
                        foreach (var addon in Addon.Addons)
                            foreach (var addonArchive in addon.Archives)
                            {
                                if (i++ == archiveIndex)
                                {
                                    var arc = Activator.CreateInstance(
                                        addonArchive.ArchiveType) as Archive;
                                    arc.Data = archive.Data;
                                    arc.Save(fileLocation, true);
                                }
                            }
                        break;
                }

                archive.Saved = true;
            }

            RefreshTabPage(index, false);
        }

        public void CloseArchive(int index)
        {
            if (!Archives[index].Saved)
            {
                if (MessageBox.Show("Save Archive before closing?", Text,
                      MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes)
                {
                    try { SaveArchive(index, false); }
                    catch { return; }
                }
            }

            ArchiveFilePaths.Remove(Archives[index]);
            ArchiveCurrentDir.Remove(Archives[index]);
            Archives.RemoveAt(index);
            tabControl.TabPages.RemoveAt(index);
        }

        public void AddTabPage(string fileName)
        {
            tabControl.TabPages.Add(fileName);

            int tabPageIndex = tabControl.TabPages.Count - 1;
            var tabPage = tabControl.TabPages[tabPageIndex];
            tabPage.Tag = fileName;

            var listView = new ListViewSort()
            {
                Dock = DockStyle.Fill,
                View = View.Details,
                MultiSelect = true,
                FullRowSelect = true,
                AllowColumnReorder = true,
                LabelEdit = true
            };

            listView.ContextMenuStrip = contextMenu;

            // Mouse Events
            listView.MouseMove += Lv_MouseMove;
            listView.MouseUp += Lv_MouseUp;
            listView.MouseDoubleClick += Lv_MouseDoubleClick;

            // Other Events
            listView.KeyPress += new KeyPressEventHandler(Lv_KeyPress);
            listView.BeforeLabelEdit += new LabelEditEventHandler(Lv_BeforeLabelEdit);
            listView.AfterLabelEdit += new LabelEditEventHandler(Lv_AfterLabelEdit);

            // Columns
            listView.Columns.Add("Name");
            listView.Columns.Add("Extension");
            listView.Columns.Add("Size");

            tabPage.Controls.Add(listView);
            RefreshTabPage(tabPageIndex);
            tabControl.SelectedIndex = tabPageIndex;
        }

        public void RefreshTabPage(int index, bool refreshFileList = true)
        {
            if(index < 0)
                return;

            var tabPage = tabControl.TabPages[index];
            var listView = tabPage.Controls[0] as ListView;
            var archive = Archives[index];

            // Update TabPage Text
            tabPage.Text = (tabPage.Tag as string) + (archive.Saved ? "" : "*");
            UpdateTitle();

            // Update File List
            if (!refreshFileList || listView == null) return;

            var files = ArchiveCurrentDir[archive] == null ?
                archive.Data : ArchiveCurrentDir[archive].Data;
            
            UpdateList(files, ArchiveCurrentDir[archive] == null);
        }

        public void UpdateList(List<ArchiveData> dataInDirectory, bool isRoot)
        {
            var tabPage = tabControl.SelectedTab;
            var listView = tabPage.Controls[0] as ListView;
            var archive = CurrentArchive;
            var files = archive.Data;                      // Files to add to list
            var items = new List<ListViewItem>();           // List of Items to be added later

            // Stops the ListView from drawing until we call EndUpdate
            listView.BeginUpdate();

            // If listView is set to Details View
            if (listView.View == View.Details)
            {
                listView.SmallImageList = new ImageList();
                listView.SmallImageList.ColorDepth = ColorDepth.Depth32Bit;
                listView.SmallImageList.Images.Add("-", GetIconFromExtension("-"));
            }
            var imgList = listView.LargeImageList ?? listView.SmallImageList;

            // Folder Icon
            ExtractIconEx("shell32.dll", 4, out IntPtr largePointer, out IntPtr SmallPointer, 1);
            imgList.Images.Add("-Directory",
                Icon.FromHandle(largePointer));


            // Clears/Removes all the items from the ListView.
            listView.Items.Clear();

            // Lengths
            int longestNameLength = 0, longestExtensionLength = 0, longestSizeLength = 0;

            if (!isRoot)
            {
                // Current Directory
                var dir = ArchiveCurrentDir[archive];

                files = dir.Data;                          // Change List to the Current Directory's List
                var lvi = new ListViewItem("..");
                lvi.Tag = dir.Parent;                       // Paent Directory
                lvi.ImageKey = "-Directory";                // Directory Icon

                if (lvi.Text.Length > longestNameLength)
                    longestNameLength = lvi.Text.Length;

                items.Insert(0, lvi);
            }

            for (int i = 0; i < dataInDirectory.Count; ++i)
            {
                var data = dataInDirectory[i];
                var lvi = new ListViewItem();

                if (data is ArchiveFile file)
                {
                    var fileInfo = new FileInfo(file.Name);
                    lvi = new ListViewItem(new string[]
                    {
                        fileInfo.Name,
                        fileInfo.Extension,
                        file.Data != null ? 
                        ConvertSize(file.Data.LongLength) : null
                    });

                    try
                    {
                        // Sets the ImageKey to the current file.
                        if (fileInfo.Extension.Length == 0)
                            lvi.ImageKey = "-";
                        else
                        {
                            if (!imgList.Images.ContainsKey(fileInfo.Extension))
                                imgList.Images.Add(fileInfo.Extension, GetIconFromExtension(fileInfo.Extension));

                            lvi.ImageKey = fileInfo.Extension;
                        }
                    }
                    catch { }

                    if (lvi.Text.Length > longestNameLength)
                        longestNameLength = lvi.Text.Length;

                    if (lvi.SubItems[1].Text.Length > longestExtensionLength)
                        longestExtensionLength = lvi.SubItems[1].Text.Length;

                    if (lvi.SubItems[2].Text.Length > longestSizeLength)
                        longestSizeLength = lvi.SubItems[2].Text.Length;
                }
                else if (data is ArchiveDirectory directory)
                {
                    lvi = new ListViewItem(directory.Name);
                    lvi.Tag = directory;
                    lvi.ImageKey = "-Directory";
                    if (lvi.Text.Length > longestNameLength)
                        longestNameLength = lvi.Text.Length;

                }
                else
                    continue; // Skip this object
            

                lvi.Tag = data;
                items.Add(lvi);
            }
            // Adds all the items into the ListView
            listView.Items.AddRange(items.ToArray());

            // Update the columns in the file list
            listView.AutoResizeColumn(0, (longestNameLength > listView.Columns[0].Text.Length) ?
                ColumnHeaderAutoResizeStyle.ColumnContent :
                ColumnHeaderAutoResizeStyle.HeaderSize);

            listView.AutoResizeColumn(1, (longestExtensionLength > listView.Columns[1].Text.Length) ?
                ColumnHeaderAutoResizeStyle.ColumnContent :
                ColumnHeaderAutoResizeStyle.HeaderSize);

            listView.AutoResizeColumn(2, (longestExtensionLength > listView.Columns[2].Text.Length) ?
                ColumnHeaderAutoResizeStyle.ColumnContent :
                ColumnHeaderAutoResizeStyle.HeaderSize);

            listView.EndUpdate();

        }

        public void RefreshGUI()
        {
            saveToolStripMenuItem.Enabled = saveAsToolStripMenuItem.Enabled =
                addFilesToolStripMenuItem.Enabled = extractAllToolStripMenuItem.Enabled =
                closeToolStripMenuItem.Enabled = tabControl.TabPages.Count > 0;
            RefreshTabPage(tabControl.SelectedIndex);
            if (tabControl.SelectedIndex > -1)
                largeIconViewToolStripMenuItem.Checked = 
                    (tabControl.TabPages[tabControl.SelectedIndex].Controls[0] as ListView).View == View.LargeIcon;

            // TODO: Update status bar label.
            UpdateTitle();
        }

        public void AddFilesToCurrentArchive(params string[] filePaths)
        {
            AddFilesToArchive(CurrentArchive, filePaths);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="archive">The Archive you want to add the files to</param>
        /// <param name="filePaths">An array of file paths</param>
        public void AddFilesToArchive(Archive archive, params string[] filePaths)
        {
            var files = ArchiveCurrentDir[archive] == null ?
                archive.Data : ArchiveCurrentDir[archive].Data;
            foreach (var file in filePaths)
            {
                if (File.GetAttributes(file) != FileAttributes.Directory)
                { // File
                    var fileInfo = new FileInfo(file);

                    var archiveFile = files.Find(
                           t => t.Name.ToLower() == fileInfo.Name.ToLower());

                    if (archiveFile != null)
                    {
                        if (MessageBox.Show($"There's already a file called \"{fileInfo.Name}\".\n" +
                                $"Do you want to replace this file?", Text,
                                MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.No)
                            continue;

                        files.Remove(archiveFile);
                    }

                    files.Add(new ArchiveFile(file));
                    archive.Saved = false;
                }
                else
                { // Directory
                    AddDirectoryToArchiveDirectory(archive, null, file);
                    archive.Saved = false;
                }
            }
        }

        public void AddDirectoryToArchiveDirectory(Archive archive, ArchiveDirectory directory, string directoryPath)
        {
            var files = directory == null ? archive.Data : directory.Data;
            var fileInfo = new FileInfo(directoryPath);
            var archiveFile = files.Find(
                   t => t.Name.ToLower() == fileInfo.Name.ToLower());

            if (archiveFile != null)
            {
                if (MessageBox.Show($"There's already a directory called \"{fileInfo.Name}\".\n" +
                        $"Do you want to merge this directory?", Text,
                        MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.No)
                    return;
            }
            var directoryInfo = new DirectoryInfo(directoryPath);
            var newDirectory = new ArchiveDirectory(directoryInfo.Name);

            newDirectory.Parent = directory;

            foreach (string filePath in Directory.GetFiles(directoryPath))
                newDirectory.Data.Add(new ArchiveFile(filePath));

            foreach (string directoryPath2 in Directory.GetDirectories(directoryPath, "*",
                SearchOption.TopDirectoryOnly))
                AddDirectoryToArchiveDirectory(archive, newDirectory, directoryPath2);

            files.Add(newDirectory);
        }

        public string AddFiltersFromAddons()
        {
            string s = "";
            foreach (var addon in Addon.Addons)
            {
                foreach (var archive in addon.Archives)
                {
                    s += $"|{archive.ArchiveName}|";
                    foreach (string ext in archive.FileExtensions)
                        s += $"*{ext};";
                    s = s.Substring(0, s.Length-1);
                }
            }
            return s;
        }

        public static bool HasSupportedArchiveExtension(string fileName)
        {
            string fileExtension = Path.GetExtension(fileName).ToLower();

            foreach (var addon in Addon.Addons)
                foreach (var archive in addon.Archives)
                    if (archive.FileExtensions.Contains(fileExtension)) return true;


            return (fileExtension == GensArchive.Extension || fileExtension == GensArchive.ListExtension
                || fileExtension == GensArchive.PFDExtension || fileExtension == GensArchive.SplitExtension
                || fileExtension == ForcesArchive.Extension
                || fileExtension == SBArchive.Extension || fileExtension == ONEArchive.Extension);
        }

        // GUI Events
        private void TabControl_SelectedIndexChanged(object sender, EventArgs e)
        {
            RefreshGUI();
        }

        // TODO: Allow drag and drop into directories
        private void TabControl_DragDrop(object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(DataFormats.FileDrop) &&
                e.Data.GetData(DataFormats.FileDrop) is string[] files)
            {
                if (HasSupportedArchiveExtension(files[0]))
                {
                    foreach (string fileName in files)
                        OpenArchive(fileName);
                    RefreshGUI();
                }
                else
                {
                    var archive = CurrentArchive;
                    AddFilesToArchive(archive, files);

                    RefreshGUI();
                }
            }
        }

        private void TabControl_DragEnter(object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(DataFormats.FileDrop) &&
                e.Data.GetData(DataFormats.FileDrop) is string[] files)
            {
                // Gets the Current Process PID.
                string processId = Process.GetCurrentProcess().Id.ToString();
                if (e.Data.GetDataPresent("SourcePID") && (e.Data.GetData("SourcePID") as string == processId))
                    return;
                if (Archives.Count > 0)
                    e.Effect = DragDropEffects.Copy;

                if (files.Length > 0 && HasSupportedArchiveExtension(files[0]))
                    e.Effect = DragDropEffects.Copy;
            }
        }

        private void ContextMenu_Opening(object sender, System.ComponentModel.CancelEventArgs e)
        {
            var listView = tabControl.SelectedTab.Controls[0] as ListView;

            extractSelectedFilesToolStripMenuItem.Enabled =
                removeSelectedFilesToolStripMenuItem.Enabled =
                renameSelectedFileToolStripMenuItem.Enabled = listView.SelectedItems.Count > 0;
        }

        // GUI Events (ToolStripMenuItem)
        private void LargeIconViewToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // Checks if theres a selected tab.
            if (tabControl.SelectedIndex >= 0)
            {
                var listView = tabControl.SelectedTab.Controls[0] as ListView;

                if (listView == null) return;

                if (listView.View == View.Details)
                { // Set to Large Icons.
                    listView.LargeImageList = new ImageList()
                    {
                        ImageSize = new Size(64, 64),
                        ColorDepth = ColorDepth.Depth32Bit
                    };
                    listView.View = View.LargeIcon;
                    largeIconViewToolStripMenuItem.CheckState = CheckState.Checked;
                    listView.LargeImageList.Images.Add("-", GetIconFromExtension("-"));
                }
                else
                { // Set to Details.
                    listView.LargeImageList = null;
                    listView.View = View.Details;
                    largeIconViewToolStripMenuItem.CheckState = CheckState.Unchecked;
                }

                // Refreshes the TabPage and ListView.
                RefreshTabPage(tabControl.SelectedIndex);
            }
        }

        private void CreateNewArchiveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var archive = new Archive();
            Archives.Add(archive);
            ArchiveFilePaths.Add(archive, null);
            ArchiveCurrentDir.Add(archive, null);
            AddTabPage("Untitled");
            RefreshGUI();
        }

        private void OpenToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var ofd = new OpenFileDialog()
            {
                Title = "Open Archive(s)...",
                Filter = GetFilters(true),
                Multiselect = true
            };

            if (ofd.ShowDialog() == DialogResult.OK)
            {
                try
                {
                    foreach (string file in ofd.FileNames)
                        OpenArchive(file);
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, Program.ProgramName,
                        MessageBoxButtons.OK, MessageBoxIcon.Error);
                }

                RefreshGUI();
            }
        }

        private void AddFilesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var ofd = new OpenFileDialog()
            {
                Title = "Add File(s)...",
                Filter = "All Files (*.*)|*.*",
                Multiselect = true
            };

            if (ofd.ShowDialog() == DialogResult.OK)
            {
                var archive = CurrentArchive;
                AddFilesToArchive(archive, ofd.FileNames);

                RefreshTabPage(tabControl.SelectedIndex);
            }
        }

        private void NewFolderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var archive = CurrentArchive;
            var files = ArchiveCurrentDir[archive] == null ?
                archive.Data : ArchiveCurrentDir[archive].Data;
            // TODO: Allow user to change the directory name at creation
            var directory = new ArchiveDirectory("New Folder");
            if (files.FindIndex(t => t.Name == "New Folder") != -1)
            {
                int index = 1;
                while (files.FindIndex(t => t.Name == $"New Folder ({index})") != -1)
                    index++;

                directory.Name = $"New Folder ({index})";
            }
            archive.Saved = false;

            if (ArchiveCurrentDir[archive] != null)
                directory.Parent = ArchiveCurrentDir[archive];
            
            files.Add(directory);
            RefreshTabPage(tabControl.SelectedIndex);
        }

        private void ExtractAllToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var sfd = new SaveFileDialog()
            {
                Title = "Extract all files",
                FileName = "Enter into a directory and press Save"
            };

            if (sfd.ShowDialog() == DialogResult.OK)
            {
                try
                {
                    var archive = CurrentArchive;
                    var files = ArchiveCurrentDir[archive] == null ?
                        archive.Data : ArchiveCurrentDir[archive].Data;
                    var fileInfo = new FileInfo(sfd.FileName);
                    var pb = new ToolStripProgressBar();

                    new System.Threading.Thread(() =>
                    {
                        Invoke(new Action(() => Enabled = false));
                        Process.Start("explorer.exe", fileInfo.Directory.FullName);
                        statusStrip.Invoke(new Action(() => statusStrip.Items.AddRange(new ToolStripItem[] { pb })));
                        Invoke(new Action(() => pb.Maximum = files.Count));

                        foreach (var archiveFile in files)
                        {
                            archiveFile.Extract(Path.Combine(fileInfo.Directory.FullName, archiveFile.Name));
                            Invoke(new Action(() => ++pb.Value));
                        }

                        statusStrip.Invoke(new Action(() => statusStrip.Items.Remove(pb)));
                        Invoke(new Action(() => Enabled = true));
                    }).Start();
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, Program.ProgramName,
                        MessageBoxButtons.OK, MessageBoxIcon.Error);
                }

                RefreshGUI();
            }
        }

        private void CloseToolStripMenuItem_Click(object sender, EventArgs e)
        {
            CloseArchive(tabControl.SelectedIndex);
        }

        private void ExitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void ExtractSelectedFilesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var sfd = new SaveFileDialog()
            {
                Title = "Extract selected files",
                FileName = "Enter into a directory and press Save"
            };

            if (sfd.ShowDialog() == DialogResult.OK)
            {
                try
                {
                    var archive = CurrentArchive;
                    var files = ArchiveCurrentDir[archive] == null ?
                        archive.Data : ArchiveCurrentDir[archive].Data;
                    var listView = tabControl.SelectedTab.Controls[0] as ListView;
                    var pb = new ToolStripProgressBar();
                    string directoryPath = new FileInfo(sfd.FileName).Directory.FullName;

                    new System.Threading.Thread(() =>
                    {
                        Invoke(new Action(() => Enabled = false));
                        statusStrip.Invoke(new Action(() => statusStrip.Items.AddRange(new ToolStripItem[] { pb })));
                        Process.Start("explorer.exe", directoryPath);
                        Invoke(new Action(() => pb.Maximum = listView.SelectedItems.Count));
                        Invoke(new Action(() =>
                        {
                            foreach (ListViewItem lvi in listView.SelectedItems)
                                foreach (var archiveFile in files)
                                    if (archiveFile.Name == lvi.SubItems[0].Text)
                                    {
                                        archiveFile.Extract(Path.Combine(directoryPath, archiveFile.Name));
                                        ++pb.Value;
                                        break;
                                    }
                        }));
                        statusStrip.Invoke(new Action(() => statusStrip.Items.Remove(pb)));
                        Invoke(new Action(() => Enabled = true));
                    }).Start();
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, Program.ProgramName,
                        MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }

        private void RemoveSelectedFilesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                var archive = CurrentArchive;
                var files = ArchiveCurrentDir[archive] == null ?
                    archive.Data : ArchiveCurrentDir[archive].Data;
                var listView = tabControl.SelectedTab.Controls[0] as ListView;
                archive.Saved = false;
                new System.Threading.Thread(() =>
                {
                    Invoke(new Action(() => Enabled = false));
                    Invoke(new Action(() =>
                    {
                        foreach (ListViewItem lvi in listView.SelectedItems)
                            files.Remove(files.Find(t => t.Name == lvi.Text));
                    }));
                    Invoke(new Action(() => RefreshGUI()));
                    Invoke(new Action(() => RefreshTabPage(tabControl.SelectedIndex)));
                    Invoke(new Action(() => Enabled = true));
                }).Start();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, Program.ProgramName,
                    MessageBoxButtons.OK, MessageBoxIcon.Error);
                RefreshGUI();
                RefreshTabPage(tabControl.SelectedIndex);
            }
        }

        private void CreateFromDirectoryToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var sfd = new SaveFileDialog()
            {
                Title = "Create Archive from Directory",
                FileName = "Enter into a directory and press Save"
            };

            if (sfd.ShowDialog() == DialogResult.OK)
            {
                var fileInfo = new FileInfo(sfd.FileName);
                var archive = new Archive();

                ArchiveFilePaths.Add(archive, null);
                ArchiveCurrentDir.Add(archive, null);

                foreach (string filePath in Directory.GetFiles(fileInfo.DirectoryName))
                    archive.Data.Add(new ArchiveFile(filePath));

                foreach (string directoryPath2 in Directory.GetDirectories(fileInfo.DirectoryName,
                    "*", SearchOption.TopDirectoryOnly))
                    AddDirectoryToArchiveDirectory(archive, null, directoryPath2);

                Archives.Add(archive);
                AddTabPage(Path.GetFileName(fileInfo.DirectoryName));
                RefreshGUI();
            }
        }

        private void SaveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                SaveArchive(tabControl.SelectedIndex, false);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Failed to save archive!\n{ex}", Program.ProgramName,
                     MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void SaveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                SaveArchive(tabControl.SelectedIndex, true);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Failed to save archive!\n{ex}", Program.ProgramName,
                     MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void RenameSelectedFileToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var listView = tabControl.SelectedTab.Controls[0] as ListView;
            if (listView == null || listView.SelectedItems.Count < 1) return;
            listView.FocusedItem.BeginEdit();
        }
        
        private void SelectAllToolStripMenuItem_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in (tabControl.SelectedTab.Controls[0] as ListView).Items)
                item.Selected = true;
        }

        private void CopyToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var archive = CurrentArchive;
            var files = ArchiveCurrentDir[archive] == null ?
                archive.Data : ArchiveCurrentDir[archive].Data;
            var listView = tabControl.SelectedTab.Controls[0] as ListView;
            var pb = new ToolStripProgressBar();
            var fileList = new List<string>();
            string path = Path.Combine(tempPath, "Extracted_Files\\");

            if (listView == null) return;

            Directory.CreateDirectory(path);

            new System.Threading.Thread(() =>
            {
                Invoke(new Action(() => Enabled = false));
                statusStrip.Invoke(new Action(() => statusStrip.Items.AddRange(new ToolStripItem[] { pb })));
                Invoke(new Action(() => pb.Maximum = listView.SelectedItems.Count));
                Invoke(new Action(() =>
                {
                    foreach (ListViewItem lvi in listView.SelectedItems)
                        foreach (var archiveFile in files)
                        {
                            string filePath = Path.Combine(path, archiveFile.Name);
                            archiveFile.Extract(filePath);
                            fileList.Add(filePath);
                            ++pb.Value;
                        }
                }));
                Invoke(new Action(() => Clipboard.SetData(DataFormats.FileDrop, fileList.ToArray())));
                statusStrip.Invoke(new Action(() => statusStrip.Items.Remove(pb)));
                Invoke(new Action(() => Enabled = true));
            }).Start();
        }

        private void PasteToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (Clipboard.GetData(DataFormats.FileDrop) is string[] files)
            {
                var archive = CurrentArchive;
                AddFilesToArchive(archive, files);

                RefreshGUI();
                RefreshTabPage(tabControl.SelectedIndex);
            }
        }

        private void EditToolStripMenuItem_Opening(object sender, EventArgs e)
        {
            ListView lv = null;
            if (Archives.Count > 0) lv = tabControl.SelectedTab.Controls[0] as ListView;

            pasteToolStripMenuItem.Enabled = (lv != null && Clipboard.ContainsFileDropList());
            renameToolStripMenuItem.Enabled = copyToolStripMenuItem.Enabled = deleteToolStripMenuItem.Enabled =
                selectAllToolStripMenuItem.Enabled =
                ((lv != null) ? lv.SelectedItems.Count > 0 : false);
        }

        private void MainFrm_FormClosing(object sender, FormClosingEventArgs e)
        {
            for (int i = 0; i < Archives.Count; ++i)
            {
                var archive = Archives[i];
                if (!archive.Saved)
                {
                    var ArchiveName = Path.GetFileName(ArchiveFilePaths[archive] ?? "Archive");

                    var dialog = MessageBox.Show($"Save {ArchiveName} before closing?", Text,
                          MessageBoxButtons.YesNoCancel, MessageBoxIcon.Question);
                    switch (dialog)
                    {
                        case DialogResult.Yes:
                            try
                            {
                                int index = Archives.IndexOf(archive);
                                SaveArchive(index, false);
                                ArchiveFilePaths.Remove(archive);
                                ArchiveCurrentDir.Remove(archive);
                                Archives.RemoveAt(index);
                                tabControl.TabPages.RemoveAt(index);
                                --i;
                            }
                            catch (Exception ex)
                            {
                                e.Cancel = true;
                                MessageBox.Show($"Failed to save archive!\n{ex}", Program.ProgramName,
                                    MessageBoxButtons.OK, MessageBoxIcon.Error);
                            }
                            break;
                        case DialogResult.Cancel:
                            e.Cancel = true;
                            break;
                        default:
                            break;
                    }
                }
            }

            // Only delete the Temp Folder if the user is not holding 'Alt' when the form is closing
            if (!ModifierKeys.HasFlag(Keys.Alt))
            {
                try
                {
                    #if !DEBUG
                    Directory.Delete(tempPath, true);
                    #endif
                }
                catch { }
            }
        }

        // GUI Events (ListView)
        private void Lv_KeyPress(object sender, KeyPressEventArgs e)
        {
            // Enter key
            if (e.KeyChar == (char)13)
            {
                Lv_MouseDoubleClick(sender, null);
            }
        }

        private string prevName;

        private void Lv_BeforeLabelEdit(object sender, LabelEditEventArgs e)
        {
            var listView = sender as ListView;
            prevName = listView.FocusedItem.Text;
        }

        private void Lv_AfterLabelEdit(object sender, LabelEditEventArgs e)
        {
            var listView = sender as ListView;
            var archive = CurrentArchive;
            var files = ArchiveCurrentDir[archive] == null ?
                archive.Data : ArchiveCurrentDir[archive].Data;
            // Checks if the user has changed the name, If not then return.
            if (prevName == e.Label || e.Label == null) return;

            // Checks for any invalid characters in the new file name.
            if (e.Label.IndexOfAny(Path.GetInvalidFileNameChars()) != -1)
            {
                e.CancelEdit = true;
                MessageBox.Show("The given name contains invalid characters.\n" +
                    "A file name can't contain any of the following characters:\n \\ / : * ? \" < > |", 
                    Program.ProgramName, MessageBoxButtons.OK, MessageBoxIcon.Information);
                return;
            }

            // Goes though all the files inside the selected archive
            // to see if there is a file with the same name. If so, then cancel and return.
            foreach (var archiveFile in files)
            {
                if (archiveFile.Name.Equals(e.Label, StringComparison.OrdinalIgnoreCase))
                {
                    e.CancelEdit = true;
                    MessageBox.Show("The given name is already being used by another file.",
                        Program.ProgramName, MessageBoxButtons.OK, MessageBoxIcon.Information);
                    return;
                }
            }
            
            var data = files.Find(t => t.Name == prevName);
            data.Name = e.Label;
            if (data.GetType() == typeof(ArchiveFile))
                listView.Items[e.Item].SubItems[1].Text = new FileInfo(e.Label).Extension;
        }

        private void Lv_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            var listView = sender as ListView;
            var archive = CurrentArchive;
            var files = ArchiveCurrentDir[archive] == null ?
                archive.Data : ArchiveCurrentDir[archive].Data;
            var path = Path.Combine(tempPath, "Extracted_Files\\");
            Directory.CreateDirectory(path);
            if (listView.SelectedItems.Count == 1)
            {
                if (listView.SelectedItems[0].Tag is ArchiveDirectory ||
                    listView.SelectedItems[0].Tag == null)
                {
                    ArchiveCurrentDir[archive] = listView.SelectedItems[0].Tag as ArchiveDirectory;
                    RefreshTabPage(tabControl.SelectedIndex);
                    return;
                }
                foreach (var archiveFile in files)
                {
                    if (archiveFile.Name == listView.SelectedItems[0].SubItems[0].Text)
                    {
                        var filePath = Path.Combine(path, archiveFile.Name);
                        archiveFile.Extract(filePath);
                        Process.Start(filePath);
                        break;
                    }
                }                
            }
        }

        // NOTE: Lv_MouseMove needs a lot of work
        private void Lv_MouseUp(object sender, MouseEventArgs e)
        {
            extracted = false;
        }

        private void Lv_MouseMove(object sender, MouseEventArgs e)
        {
            var archive = CurrentArchive;
            var files = ArchiveCurrentDir[archive] == null ?
                archive.Data : ArchiveCurrentDir[archive].Data;
            var listView = sender as ListView;
            var fileList = new List<string>();
            string path = Path.Combine(tempPath, "Extracted_Files\\");

            if (e.Button == MouseButtons.Left && listView.SelectedItems.Count > 0 &&
                !listView.FocusedItem.Bounds.Contains(listView.PointToClient(MousePosition)))
            {
                try
                {
                    if (!extracting && !extracted)
                    {
                        if (!extracted) extracting = true;
                        Invoke(new Action(() =>
                        {
                            Enabled = false;
                            Directory.CreateDirectory(path);
                            var pb = new ToolStripProgressBar();
                            statusStrip.Items.AddRange(new ToolStripItem[] { pb });
                            pb.Maximum = listView.SelectedItems.Count;
                            foreach (ListViewItem lvi in listView.SelectedItems)
                            {
                                foreach (var archiveFile in files)
                                {
                                    if (archiveFile.Name == lvi.SubItems[0].Text)
                                    {
                                        string filePath = Path.Combine(path, archiveFile.Name);
                                        archiveFile.Extract(filePath);
                                        fileList.Add(filePath);
                                        ++pb.Value;
                                        break;
                                    }
                                }
                            }
                            statusStrip.Items.Remove(pb);
                            Enabled = true;
                            extracting = false;
                            extracted = true;
                        }));
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, Program.ProgramName,
                        MessageBoxButtons.OK, MessageBoxIcon.Error);
                }

                if (fileList.Count > 0 && extracted)
                {
                    var dataObject = new DataObject(DataFormats.FileDrop, fileList.ToArray());
                    dataObject.SetData("SourcePID", Process.GetCurrentProcess().Id.ToString());
                    DoDragDrop(dataObject, DragDropEffects.Copy);
                }

                if (fileList.Count == 0 && extracted)
                {
                    extracted = false;
                }
            }
        }

        // Other
        private class ListViewSort : ListView, IComparer
        {
            private int column = 0;
            private SortOrder order = SortOrder.Ascending;

            public ListViewSort()
            {
                DoubleBuffered = true;
                ListViewItemSorter = this;
            }

            public int Compare(object x, object y)
            {
                int returnVal = -1;

                #region Directory Sorting
                if (((ListViewItem)x).ImageKey == "-Directory" &&
                    ((ListViewItem)y).ImageKey == "-Directory")
                    return string.Compare(((ListViewItem)x).SubItems[0].Text,
                                        ((ListViewItem)y).SubItems[0].Text);

                if (((ListViewItem)x).ImageKey == "-Directory")
                    return -1;

                if (((ListViewItem)y).ImageKey == "-Directory")
                    return 1;
                #endregion Directory Sorting

                returnVal = string.Compare(((ListViewItem)x).SubItems[column].Text,
                                        ((ListViewItem)y).SubItems[column].Text);
                
                // Sort by file size
                if (Columns[column].Text == "Size")
                {
                    string xx = ((ListViewItem)x).SubItems[column].Text;
                    string yy = ((ListViewItem)y).SubItems[column].Text;

                    returnVal = ConvertSize(xx) > ConvertSize(yy) ? 1 : -1;
                }

                return order == SortOrder.Descending ? -returnVal : returnVal;
            }

            protected override void OnColumnClick(ColumnClickEventArgs e)
            {
                order = order == SortOrder.Ascending ?
                SortOrder.Descending : SortOrder.Ascending;
                column = e.Column;
                Sort();
            }
        }

        // TODO: Think of better names for these 2 functions
        public static string ConvertSize(long byteCount)
        {
            // Unit Names
            var units = new string[] { "Bytes", "KB", "MB", "GB", "TB" };

            // If the size is 1
            if (byteCount == 1)
                return "1 Byte";

            int index = 0;
            decimal buffer = byteCount;
            while (buffer > 1024)
            {
                ++index;
                buffer /= (decimal)1024.0;
            }

            return (buffer.ToString("0.##") + ' ' + units[index]);
        }

        public static long ConvertSize(string byteString)
        {
            // Unit Names
            var units = new string[] { "Byte", "Bytes", "KB", "MB", "GB", "TB" };
            int unitIndex = units.ToList().FindIndex(t => byteString.EndsWith(t));

            // Didn't work
            if (unitIndex == -1)
                return 0;

            // Removes the unit name
            byteString = 
                byteString.Substring(0, byteString.IndexOf(units[unitIndex]) - 1);

            // If its only 1 byte
            if (unitIndex == 0)
                return 1;

            // If its less then a kilobyte
            if (unitIndex == 1)
                return long.Parse(byteString);

            return (long)(Convert.ToDouble(byteString) * Math.Pow(1024, unitIndex - 1));
        }

        private struct SHFILEINFO
        {
            public IntPtr iconHandle;
            public int iconIndex;
            public uint dwAttributes;
            public string szDisplayName;
            public string szTypeName;
        };

        [System.Runtime.InteropServices.DllImport("Shell32.dll")]
        private static extern int ExtractIconEx(string lpszFile, int nIconIndex, out IntPtr phiconLarge, out IntPtr phiconSmall, int nIcons);

        [System.Runtime.InteropServices.DllImport("Shell32.dll")]
        private static extern uint SHGetFileInfo(string fileName, uint fileAttributes, ref SHFILEINFO psfi,
            uint fileInfoSize, uint flags);

        [System.Runtime.InteropServices.DllImport("User32.dll")]
        private static extern bool DestroyIcon(IntPtr iconHandle);


        private static Icon GetIconFromExtension(string name)
        {
            try
            {
                var shfi = new SHFILEINFO();
                uint fileAttributes = 0x80; // FILE_ATTRIBUTE_NORMAL
                uint flags = 0x112; // SHGFI_ICON | SHGFI_USEFILEATTRIBUTES | SHGFI_EXTRALARGEICON

                SHGetFileInfo(name, fileAttributes, ref shfi, (uint)System.Runtime.InteropServices.Marshal.SizeOf(shfi), flags);

                var icon = (Icon)Icon.FromHandle(shfi.iconHandle).Clone();
                DestroyIcon(shfi.iconHandle);
                return icon;
            }
            catch { return SystemIcons.Error; }
        }
    }
}