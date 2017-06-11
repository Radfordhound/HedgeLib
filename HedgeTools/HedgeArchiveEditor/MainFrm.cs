using HedgeLib.Archives;
using System;
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
        //Variables/Constants
        public static string tempPath = Path.Combine(Path.GetTempPath(), "HedgeArchiveEditor\\");

        public Dictionary<Archive, object[]> ArchiveFileExtraData =
            new Dictionary<Archive, object[]>();
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

        //Constructors
        public MainFrm()
        {
            InitializeComponent();
            UpdateTitle();
            Directory.CreateDirectory(tempPath);
        }

        //Methods
        public void UpdateTitle()
        {
            Text = ((tabControl.TabPages.Count > 0) ?
                $"{tabControl.SelectedTab.Text} - " : "") + Program.ProgramName;
        }

        public void OpenArchive(string filePath)
        {
            try
            {
                var arc = Program.LoadArchive(filePath);
                Archives.Add(arc);
                ArchiveFileExtraData.Add(arc, new object[] { filePath });
                arc.Saved = true;
                AddTabPage(new FileInfo(filePath).Name);
            }catch (Exception ex)
            {
                MessageBox.Show(ex.Message, Program.ProgramName,
                        MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        public void SaveArchive(int index, bool saveAs)
        {
            string fileLocation = null;
            int ArchiveType = -1;
            var arc = Archives[index];

            if (!ArchiveFileExtraData.ContainsKey(arc) || saveAs)
            {
                var sfd = new SaveFileDialog()
                {
                    Title = "Save Archive As...",
                    Filter = "Generations/Unleashed Archives (*.ar.00, *.pfd)|*.ar.00;*.pfd"
                     + "|Lost World Archives (*.pac)|*.pac|StoryBook Series Archives (*.one)|*.one"
                     + "|Heroes Archives (*.one)|*.one"
                     + "|All Files (*.*)|*.*"
                };

                if (sfd.ShowDialog() == DialogResult.OK)
                {
                    ArchiveType = sfd.FilterIndex - 2;
                    fileLocation = sfd.FileName;

                    if (!ArchiveFileExtraData.ContainsKey(arc))
                        ArchiveFileExtraData.Add(arc, new object[] { fileLocation });
                }
                else return;
            }
            else
            {
                fileLocation = (string)ArchiveFileExtraData[arc][0];

                // These checks may not work.
                var type = Archives[index].GetType();
                if (type == typeof(GensArchive)) ArchiveType = 0;
                else if (type == typeof(LWArchive)) ArchiveType = 1;
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
                else if (fileLocation.EndsWith(LWArchive.Extension))
                    ArchiveType = 1; // Lost World
                else if (fileLocation.EndsWith(SBArchive.Extension))
                    ArchiveType = 2; // Story Books
                else if (fileLocation.EndsWith(ONEArchive.Extension)) // NOTE: This never gets called
                    ArchiveType = 3; // Heroes/Shadow
            }

            var saveOptions = new SaveOptions(ArchiveType);

            // Automatically set the Magic value if its an ONEArchive
            if (arc.GetType() == typeof(ONEArchive))
                saveOptions.NumericUpDown1.Value = ((ONEArchive)arc).Magic;

            if (saveOptions.ShowDialog() == DialogResult.OK && saveOptions.ArchiveType != -1)
            {
                // This is a horrible way of checking this, I know.
                switch (saveOptions.ComboBox1.SelectedIndex)
                {
                    // Generations/Unleashed
                    case 0:
                        uint? splitAmount = (saveOptions.CheckBox2.Checked) ?
                            (uint?)saveOptions.NumericUpDown2.Value : null;
                        var genArc = new GensArchive(arc)
                        {
                            Padding = (uint)saveOptions.NumericUpDown1.Value
                        };
                        if (saveOptions.CheckBox3.Checked && saveOptions.CheckBox2.Checked)
                            genArc.GetSplitArchivesList(fileLocation)
                                .ForEach(file => File.Delete(file));
                        genArc.Save(fileLocation, saveOptions.CheckBox1.Checked, splitAmount);
                        break;
                    // Lost World
                    case 1:
                        var lwArc = new LWArchive(arc);
                        lwArc.Save(fileLocation, true);
                        break;
                    // Story Books
                    case 2:
                        var sbArc = new SBArchive(arc);
                        sbArc.Save(fileLocation, true);
                        break;
                    // Heroes/Shadow
                    case 3:
                        var oneArc = new ONEArchive(arc)
                        {
                            Magic = (uint)saveOptions.NumericUpDown1.Value
                        };
                        oneArc.Save(fileLocation, true);
                        break;
                    default:
                        throw new NotImplementedException("Unknown Archive Type");
                }

                arc.Saved = true;
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

            ArchiveFileExtraData.Remove(Archives[index]);
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
            listView.KeyPress += new KeyPressEventHandler(Lv_KeyPress);
            listView.BeforeLabelEdit += new LabelEditEventHandler(Lv_BeforeLabelEdit);
            listView.AfterLabelEdit += new LabelEditEventHandler(Lv_AfterLabelEdit);

            listView.Columns.Add("Name");
            listView.Columns.Add("Extension");
            listView.Columns.Add("Size");

            tabPage.Controls.Add(listView);
            RefreshTabPage(tabPageIndex);
            tabControl.SelectedIndex = tabPageIndex;
        }

        public void RefreshTabPage(int index, bool refreshFileList = true)
        {
            var tabPage = tabControl.TabPages[index];
            var listView = tabPage.Controls[0] as ListView;
            var archive = Archives[index];

            // Update TabPage Text
            tabPage.Text = (tabPage.Tag as string) + (archive.Saved ? "" : "*");
            UpdateTitle();

            // Update File List
            if (!refreshFileList || listView == null) return;

            // Stops the ListView from drawing until we call EndUpdate()
            listView.BeginUpdate();

            // If listView is set to Details View
            if (listView.View == View.Details)
            {
                listView.SmallImageList = new ImageList();
                listView.SmallImageList.ColorDepth = ColorDepth.Depth32Bit;
                listView.SmallImageList.Images.Add("-", GetIconFromExtension("-"));
            }
            
            // Clears/Removes all the items from the ListView.
            listView.Items.Clear();

            // ListView Mouse Events
            listView.MouseMove += Lv_MouseMove;
            listView.MouseUp += Lv_MouseUp;
            listView.MouseDoubleClick += Lv_MouseDoubleClick;

            int longestNameLength = 0, longestExtensionLength = 0, longestSizeLength = 0;

            // A list of ListViewItems, which will be added to the ListView later
            var items = new List<ListViewItem>();
            foreach (var file in archive.Files)
            {
                var fileInfo = new FileInfo(file.Name);
                var lvi = new ListViewItem(new string[]
                {
                    fileInfo.Name,
                    fileInfo.Extension,
                    file.Data != null ? file.Data.Length >= 1024 ? file.Data.Length >= 1048576 ?
                        (file.Data.Length / 1048576.0).ToString("0.00") + " MB" :
                        (file.Data.Length / 1024.0).ToString("0.00") + " KB" :
                        file.Data.Length + " Bytes" : null
                });

                try
                {
                    // Sets the ImageKey to the current file.
                    if (fileInfo.Extension.Length == 0)
                        lvi.ImageKey = "-";
                    else
                    {
                        var imgList = listView.LargeImageList ?? listView.SmallImageList;

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
            foreach (var file in filePaths)
            {
                if (File.GetAttributes(file) != FileAttributes.Directory)
                { // File
                    var fileInfo = new FileInfo(file);

                    var archiveFile = archive.Files.Find(
                           t => t.Name.ToLower() == fileInfo.Name.ToLower());

                    if (archiveFile != null)
                    {
                        if (MessageBox.Show($"There's already a file called \"{fileInfo.Name}\".\n" +
                                $"Do you want to replace this file?", Text,
                                MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.No)
                            continue;

                        archive.Files.Remove(archiveFile);
                    }

                    archive.Files.Add(new ArchiveFile(file));
                    archive.Saved = false;
                }
                else
                { // Directory
                    bool includeSubfolders = (MessageBox.Show("Include Subfolders?", Text,
                        MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes);
                    string[] filesInDir = Directory.GetFiles(file, "*", includeSubfolders ?
                        SearchOption.AllDirectories : SearchOption.TopDirectoryOnly);
                    if (filesInDir.Length > 3000)
                    {
                        if (MessageBox.Show("There is over 3000 files.\n\tContinue?", Text,
                        MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.No) return;
                    }
                    AddFilesToArchive(archive, filesInDir);
                    archive.Saved = false;
                }
            }
        }

        public static bool HasSupportedArchiveExtension(string fileName)
        {
            string fileExtension = Path.GetExtension(fileName).ToLower();
            return (fileExtension == GensArchive.Extension || fileExtension == GensArchive.ListExtension
                || fileExtension == GensArchive.PFDExtension || fileExtension == GensArchive.SplitExtension
                || fileExtension == LWArchive.Extension
                || fileExtension == SBArchive.Extension || fileExtension == ONEArchive.Extension);
        }

        // GUI Events
        private void TabControl_SelectedIndexChanged(object sender, EventArgs e)
        {
            RefreshGUI();
        }

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
                    RefreshTabPage(tabControl.SelectedIndex);
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
            var ar = new Archive();
            Archives.Add(ar);
            AddTabPage("Untitled");
            RefreshGUI();
        }

        private void OpenToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var ofd = new OpenFileDialog()
            {
                Title = "Open Archive(s)...",
                Filter = "All Supported Archives (*.ar, *.arl, *.pfd, *.pac, *.one)"
                     + "|*.ar;*.arl;*.pfd;*.pac;*.one"
                     + "|Generations/Unleashed Archives (*.ar, *.arl, *.pfd)|*.ar;*.arl;*.pfd"
                     + "|Lost World Archives (*.pac)|*.pac|StoryBook Series Archives (*.one)|*.one"
                     + "|Heroes Archives (*.one)|*.one"
                     + "|All Files (*.*)|*.*",
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
                    var ar = CurrentArchive;
                    var fileInfo = new FileInfo(sfd.FileName);
                    var pb = new ToolStripProgressBar();

                    new System.Threading.Thread(() =>
                    {
                        Invoke(new Action(() => Enabled = false));
                        Process.Start("explorer.exe", fileInfo.Directory.FullName);
                        statusStrip.Invoke(new Action(() => statusStrip.Items.AddRange(new ToolStripItem[] { pb })));
                        Invoke(new Action(() => pb.Maximum = ar.Files.Count));

                        foreach (var archiveFile in ar.Files)
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
                                foreach (var archiveFile in archive.Files)
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
                var listView = tabControl.SelectedTab.Controls[0] as ListView;
                archive.Saved = false;
                new System.Threading.Thread(() =>
                {
                    Invoke(new Action(() => Enabled = false));
                    Invoke(new Action(() =>
                    {
                        foreach (ListViewItem lvi in listView.SelectedItems)
                            archive.Files.Remove(archive.Files.Find(t => t.Name == lvi.Text));
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
                bool includeSubfolders = (MessageBox.Show("Include Subfolders?", Text,
                    MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes);
                string[] files = Directory.GetFiles(fileInfo.Directory.FullName, "*", includeSubfolders ? SearchOption.AllDirectories : SearchOption.TopDirectoryOnly);
                if (files.Length > 3000)
                {
                    if (MessageBox.Show("Theres over 3000 files.\n\tContinue?", Text,
                    MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.No) return;
                }

                var ar = new Archive();
                foreach (string fileLocation in files)
                {
                    if (new FileInfo(fileLocation).Length < int.MaxValue)
                        ar.Files.Add(new ArchiveFile(fileLocation));
                }
                Archives.Add(ar);
                AddTabPage(fileInfo.Directory.Name);
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
                        foreach (var archiveFile in archive.Files)
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
                    var ArchiveName = Path.GetFileName(ArchiveFileExtraData.ContainsKey(archive)
                        ? (string)ArchiveFileExtraData[archive][0] : "Archive");

                    if (MessageBox.Show($"Save {ArchiveName} before closing?", Text,
                          MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes)
                    {
                        try
                        {
                            int index = Archives.IndexOf(archive);
                            SaveArchive(index, false);
                            ArchiveFileExtraData.Remove(archive);
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
            foreach (var archiveFile in CurrentArchive.Files)
            {
                if (archiveFile.Name.Equals(e.Label, StringComparison.OrdinalIgnoreCase))
                {
                    e.CancelEdit = true;
                    MessageBox.Show("The given name is already being used by another file.",
                        Program.ProgramName, MessageBoxButtons.OK, MessageBoxIcon.Information);
                    return;
                }
            }
            
            CurrentArchive.Files.Find(t => t.Name == prevName).Name = e.Label;
            listView.Items[e.Item].SubItems[1].Text = new FileInfo(e.Label).Extension;
        }

        private void Lv_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            var listView = sender as ListView;
            var archive = CurrentArchive;
            var path = Path.Combine(tempPath, "Extracted_Files\\");
            Directory.CreateDirectory(path);
            if (listView.SelectedItems.Count == 1)
            {
                foreach (var archiveFile in archive.Files)
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
                                foreach (var archiveFile in archive.Files)
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
                returnVal = string.Compare(((ListViewItem)x).SubItems[column].Text,
                                        ((ListViewItem)y).SubItems[column].Text);

                // Sort by file size
                if (Columns[column].Text == "Size")
                {
                    try
                    {
                        string xx = ((ListViewItem)x).SubItems[column].Text;
                        string yy = ((ListViewItem)y).SubItems[column].Text;

                        if (xx.EndsWith("MB"))
                            xx = Convert.ToDouble(xx.Substring(0, xx.Length - 3)) * 1024 + " KB";
                        if (xx.EndsWith("KB"))
                            xx = Convert.ToDouble(xx.Substring(0, xx.Length - 3)) * 1024 + " Bytes";
                        if (xx.EndsWith("Bytes"))
                            xx = xx.Substring(0, xx.Length - 6);

                        if (yy.EndsWith("MB"))
                            yy = Convert.ToDouble(yy.Substring(0, yy.Length - 3)) * 1024 + " KB";
                        if (yy.EndsWith("KB"))
                            yy = Convert.ToDouble(yy.Substring(0, yy.Length - 3)) * 1024 + " Bytes";
                        if (yy.EndsWith("Bytes"))
                            yy = yy.Substring(0, yy.Length - 6);
                        returnVal = Convert.ToDouble(xx) > Convert.ToDouble(yy) ? 1 : -1;
                    }
                    catch { }
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

        private struct SHFILEINFO
        {
            public IntPtr iconHandle;
            public int iconIndex;
            public uint dwAttributes;
            public string szDisplayName;
            public string szTypeName;
        };

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