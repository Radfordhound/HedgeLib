using HedgeLib.Archives;
using System;
using System.Collections;
using System.Collections.Generic;
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
            var arc = Program.LoadArchive(filePath);
            Archives.Add(arc);
            ArchiveFileExtraData.Add(arc, new object[] { filePath });
            arc.Saved = true;
            AddTabPage(new FileInfo(filePath).Name);
        }

        public void SaveArchive(int index, bool saveAs)
        {
            string fileLocation = null;
            int ArchiveType = -1;

            if (!ArchiveFileExtraData.ContainsKey(Archives[index]) || saveAs)
            {
                var sfd = new SaveFileDialog()
                {
                    Title = "Save Archive As...",
                    Filter = "Generations/Unleashed Archives (*.ar, *.arl, *.pfd)|*.ar;*.arl;*.pfd"
                     + "|Lost World Archives (*.pac)|*.pac|StoryBook Series Archives (*.one)|*.one"
                };

                if (sfd.ShowDialog() == DialogResult.OK)
                {
                    ArchiveType = sfd.FilterIndex - 1;
                    fileLocation = sfd.FileName;

                    if (!ArchiveFileExtraData.ContainsKey(Archives[index]))
                        ArchiveFileExtraData.Add(Archives[index], new object[] { fileLocation });
                }
                else return;
            }
            else
            {
                fileLocation = (string)ArchiveFileExtraData[Archives[index]][0];

                // These checks may not work.
                var type = Archives[index].GetType();
                if (type == typeof(GensArchive)) ArchiveType = 0;
                else if (type == typeof(LWArchive)) ArchiveType = 1;
                else if (type == typeof(SBArchive)) ArchiveType = 2;

                if (ArchiveType == -1)
                {
                    if (fileLocation.EndsWith(GensArchive.ListExtension) ||
                        fileLocation.EndsWith(GensArchive.Extension) ||
                        fileLocation.EndsWith(GensArchive.SplitExtension))
                    {
                        ArchiveType = 0;
                    }
                    else if (fileLocation.EndsWith(LWArchive.Extension))
                    {
                        ArchiveType = 1;
                    }
                    else if (fileLocation.EndsWith(SBArchive.Extension))
                    {
                        ArchiveType = 2;
                    }
                }
            }

            var saveOptions = new SaveOptions(ArchiveType);
            if (saveOptions.ShowDialog() == DialogResult.OK && saveOptions.ArchiveType != -1)
            {
                //This is a horrible way of checking this, I know.
                int val = saveOptions.ComboBox1.SelectedIndex;
                switch (val)
                {
                    case 0:
                        uint? splitAmount = (saveOptions.CheckBox2.Checked) ?
                            (uint?)saveOptions.NumericUpDown2.Value : null;
                        var genArc = new GensArchive(CurrentArchive)
                        {
                            Padding = (uint)saveOptions.NumericUpDown1.Value,
                        };
                        
                        genArc.Save(fileLocation, saveOptions.CheckBox1.Checked, splitAmount);
                        break;
                    case 1:
                        var lwArc = new LWArchive(CurrentArchive);
                        lwArc.Save(fileLocation);
                        break;
                    case 2:
                        var srArc = new SBArchive(CurrentArchive);
                        srArc.Save(fileLocation);
                        break;
                    default:
                        throw new NotImplementedException("Unknown Archive Type");
                }

                CurrentArchive.Saved = true;
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
            TabPage tabPage = tabControl.TabPages[tabPageIndex];
            tabPage.Tag = fileName;

            ListView lv = new ListView()
            {
                Dock = DockStyle.Fill,
                View = View.Details,
                MultiSelect = true,
                FullRowSelect = true,
                AllowColumnReorder = true,
            };

            lv.ContextMenuStrip = contextMenu;
            lv.ColumnClick += Lv_ColumnClick;
            lv.KeyPress += new KeyPressEventHandler(Lv_KeyPress);

            lv.Columns.Add("Name");
            lv.Columns.Add("Extension");
            lv.Columns.Add("Size");

            tabPage.Controls.Add(lv);
            RefreshTabPage(tabPageIndex);
            tabControl.SelectedIndex = tabPageIndex;
        }

        public void RefreshTabPage(int index, bool refreshFileList = true)
        {
            TabPage tp = tabControl.TabPages[index];
            ListView lv = tp.Controls[0] as ListView;
            Archive arc = Archives[index];

            //Update TabPage Text
            tp.Text = (tp.Tag as string) + ((arc.Saved) ? "" : "*");
            UpdateTitle();

            //Update File List
            if (!refreshFileList || lv == null) return;

            lv.SmallImageList = new ImageList();
            lv.SmallImageList.Images.Add("-", GetIconFromExtension("-"));
            
            lv.Items.Clear();

            lv.MouseMove += Lv_MouseMove;
            lv.MouseUp += Lv_MouseUp;
            lv.MouseDoubleClick += Lv_MouseDoubleClick;

            int longestNameLength = 0, longestExtensionLength = 0, longestSizeLength = 0;
            foreach (var file in arc.Files)
            {
                FileInfo fileInfo = new FileInfo(file.Name);
                ListViewItem lvi = new ListViewItem(new string[]
                {
                    fileInfo.Name,
                    fileInfo.Extension,
                    file.Data != null ? file.Data.Length >= 1024 ? file.Data.Length >= 1048576 ? (file.Data.Length / 1048576.0).ToString("0.00") + " MB": (file.Data.Length / 1024.0).ToString("0.00") + " KB" : file.Data.Length+" Bytes" : null
                });

                try
                {
                    if (!lv.SmallImageList.Images.ContainsKey(fileInfo.Extension))
                        lv.SmallImageList.Images.Add(fileInfo.Extension, GetIconFromExtension(fileInfo.Extension));

                    lvi.ImageKey = fileInfo.Extension;

                    if (fileInfo.Extension.Length == 0)
                        lvi.ImageKey = "-";
                }
                catch//(Exception ex)
                {
                }

                if (lvi.Text.Length > longestNameLength)
                    longestNameLength = lvi.Text.Length;

                if (lvi.SubItems[1].Text.Length > longestExtensionLength)
                    longestExtensionLength = lvi.SubItems[1].Text.Length;

                if (lvi.SubItems[2].Text.Length > longestSizeLength)
                    longestSizeLength = lvi.SubItems[2].Text.Length;

                lv.Items.Add(lvi);
            }

            //Update the columns in the file list
            lv.AutoResizeColumn(0, (longestNameLength > lv.Columns[0].Text.Length) ?
                ColumnHeaderAutoResizeStyle.ColumnContent :
                ColumnHeaderAutoResizeStyle.HeaderSize);

            lv.AutoResizeColumn(1, (longestExtensionLength > lv.Columns[1].Text.Length) ?
                ColumnHeaderAutoResizeStyle.ColumnContent :
                ColumnHeaderAutoResizeStyle.HeaderSize);

            lv.AutoResizeColumn(2, (longestExtensionLength > lv.Columns[2].Text.Length) ?
                ColumnHeaderAutoResizeStyle.ColumnContent :
                ColumnHeaderAutoResizeStyle.HeaderSize);
        }

        public void RefreshGUI()
        {
            saveToolStripMenuItem.Enabled = saveAsToolStripMenuItem.Enabled =
                addFilesToolStripMenuItem.Enabled = extractAllToolStripMenuItem.Enabled =
                closeToolStripMenuItem.Enabled = tabControl.TabPages.Count > 0;

            //TODO: Update status bar label.
            UpdateTitle();
        }

        //GUI Events
        private void CreateNewArchiveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Archive ar = new Archive();
            Archives.Add(ar);
            AddTabPage("Untitled");
            RefreshGUI();
        }

        private void OpenToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var ofd = new OpenFileDialog()
            {
                Title = "Open Archive(s)...",
                Filter = "Generations/Unleashed Archives (*.ar, *.arl, *.pfd)|*.ar;*.arl;*.pfd" +
                "|Lost World Archives (*.pac)|*.pac|StoryBook Series Archives (*.one)|*.one" +
                "|All Files (*.*)|*.*",

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
                CurrentArchive.Saved = false;
                foreach (var file in ofd.FileNames)
                    CurrentArchive.Files.Add(new ArchiveFile(file));

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
                    Archive ar = CurrentArchive;
                    new System.Threading.Thread(() =>
                    {
                        Invoke(new Action(() => Enabled = false));
                        var fileInfo = new FileInfo(sfd.FileName);
                        System.Diagnostics.Process.Start("explorer.exe", fileInfo.Directory.FullName);
                        ToolStripProgressBar pb = new ToolStripProgressBar();
                        statusStrip.Invoke(new Action(() => statusStrip.Items.AddRange(new ToolStripItem[] { pb })));
                        Invoke(new Action(() => pb.Maximum = ar.Files.Count));

                        foreach (ArchiveFile file in ar.Files)
                        {
                            file.Extract(HedgeLib.Helpers.CombinePaths(fileInfo.Directory.FullName, file.Name));
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

        private void TabControl_SelectedIndexChanged(object sender, EventArgs e)
        {
            RefreshGUI();
        }

        private void ContextMenu_Opening(object sender, System.ComponentModel.CancelEventArgs e)
        {
            var lv = tabControl.SelectedTab.Controls[0] as ListView;

            extractSelectedFilesToolStripMenuItem.Enabled =
                removeSelectedFilesToolStripMenuItem.Enabled =
                renameSelectedFileToolStripMenuItem.Enabled = lv.SelectedItems.Count > 0;
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
                    Archive ar = CurrentArchive;
                    new System.Threading.Thread(() =>
                    {
                        Invoke(new Action(() => Enabled = false));
                        System.Diagnostics.Process.Start("explorer.exe", new FileInfo(sfd.FileName).Directory.FullName);
                        ToolStripProgressBar pb = new ToolStripProgressBar();
                        statusStrip.Invoke(new Action(() => statusStrip.Items.AddRange(new ToolStripItem[] { pb })));
                        ListView lv = null;
                        Invoke(new Action(() => lv = (ListView)tabControl.SelectedTab.Controls[0]));
                        Invoke(new Action(() => pb.Maximum = lv.SelectedItems.Count));
                        Invoke(new Action(() =>
                        {
                            for (int i = 0; i < lv.SelectedItems.Count; ++i)
                            {
                                for (int i2 = 0; i2 < ar.Files.Count; ++i2)
                                {
                                    if (ar.Files[i2].Name == lv.SelectedItems[i].SubItems[0].Text)
                                    {
                                        ar.Files[i2].Extract(Path.Combine(new FileInfo(sfd.FileName).Directory.FullName, ar.Files[i2].Name));
                                        ++pb.Value;
                                        break;
                                    }
                                }
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
                CurrentArchive.Saved = false;
                new System.Threading.Thread(() =>
                {
                    Invoke(new Action(() => Enabled = false));
                    ToolStripProgressBar pb = new ToolStripProgressBar();
                    statusStrip.Invoke(new Action(() => statusStrip.Items.AddRange(new ToolStripItem[] { pb })));
                    Archive ar = null;
                    Invoke(new Action(() => ar = CurrentArchive));
                    ListView lv = null;
                    Invoke(new Action(() => lv = (ListView)tabControl.SelectedTab.Controls[0]));
                    Invoke(new Action(() => pb.Maximum = lv.Items.Count));
                    Invoke(new Action(() =>
                    {
                        for (int i2 = 0; i2 < ar.Files.Count; ++i2)
                        {
                            for (int i = 0; i < lv.SelectedItems.Count; ++i)
                            {
                                if (ar.Files[i2].Name == lv.SelectedItems[i].SubItems[0].Text)
                                {
                                    ++pb.Value;
                                    ar.Files.Remove(ar.Files[i2]);
                                    continue;
                                }
                            }
                        }
                    }));
                    Invoke(new Action(() => RefreshGUI()));
                    Invoke(new Action(() => RefreshTabPage(tabControl.SelectedIndex)));
                    Invoke(new Action(() => Enabled = true));
                    statusStrip.Invoke(new Action(() => statusStrip.Items.Remove(pb)));
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
                bool includeSubfolders = (MessageBox.Show("Include Subfolders?", Text,
                    MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes);
                string[] files = Directory.GetFiles(new FileInfo(sfd.FileName).Directory.FullName, "*", includeSubfolders ? SearchOption.AllDirectories : SearchOption.TopDirectoryOnly);
                if(files.Length > 3000)
                {
                    if (MessageBox.Show("Theres over 3000 files.\n\tContinue?", Text,
                    MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.No) return;
                }

                Archive ar = new Archive();
                foreach (string fileLocation in files)
                {
                    if (new FileInfo(fileLocation).Length < int.MaxValue)
                        ar.Files.Add(new ArchiveFile(fileLocation));
                }
                Archives.Add(ar);
                AddTabPage(new FileInfo(sfd.FileName).Directory.Name);
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
        
        private void TabControl_DragDrop(object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(DataFormats.FileDrop) &&
                e.Data.GetData(DataFormats.FileDrop) is string[] files)
            {
                if (files.Length == 1 & (files[0].EndsWith(".ar.00") || files[0].EndsWith(".arl")))
                {
                    OpenArchive(files[0]);
                    RefreshGUI();
                }
                else
                {
                    CurrentArchive.Saved = false;
                    foreach (var file in files)
                    {
                        if (File.GetAttributes(file) == FileAttributes.Normal)
                        {
                            for (int i = 0; i < CurrentArchive.Files.Count; ++i)
                            {
                                ArchiveFile file2 = CurrentArchive.Files[i];
                                var fileInfo = new FileInfo(file);

                                if (fileInfo.Name.ToLower() == file2.Name.ToLower())
                                {
                                    if (MessageBox.Show($"There's already a file called {file2.Name}.\n" +
                                        $"Do you want to replace {fileInfo.Name}?", Text,
                                        MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.No)
                                        return;

                                    CurrentArchive.Files.Remove(file2);
                                }
                            }
                            CurrentArchive.Files.Add(new ArchiveFile(file));
                        }
                        else if (File.GetAttributes(file) == FileAttributes.Directory)
                        {
                            bool includeSubfolders = (MessageBox.Show("Include Subfolders?", Text,
                                MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes);
                            string[] filesInDir = Directory.GetFiles(file, "*", includeSubfolders ?
                                SearchOption.AllDirectories : SearchOption.TopDirectoryOnly);

                            foreach (var fileDir in filesInDir)
                            {
                                for (int i = 0; i < CurrentArchive.Files.Count; ++i)
                                {
                                    var fileDir2 = CurrentArchive.Files[i];
                                    var fileInfo = new FileInfo(fileDir);

                                    if (fileInfo.Name.ToLower() == fileDir2.Name.ToLower())
                                    {
                                        if (MessageBox.Show($"There's already a file called {fileDir2.Name}.\n" +
                                            $"Do you want to replace {fileInfo.Name}?", Text,
                                            MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.No)
                                            continue;

                                        CurrentArchive.Files.Remove(fileDir2);
                                    }
                                }
                                CurrentArchive.Files.Add(new ArchiveFile(fileDir));
                            }
                        }
                    }

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
                if (Archives.Count > 0)
                    e.Effect = DragDropEffects.Copy;

                if (files.Length > 0 && (files[0].EndsWith(".ar.00") ||
                    files[0].EndsWith(".arl")))
                {
                    e.Effect = DragDropEffects.Copy;
                }
            }
        }

        private void RenameSelectedFileToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var lv = tabControl.SelectedTab.Controls[0] as ListView;
            if (lv == null || lv.SelectedItems.Count < 1) return;

            var renameForm = new Form()
            {
                Size = new Size(400, 120),
                FormBorderStyle = FormBorderStyle.Fixed3D,
                MaximizeBox = false,
                Text = $"Rename item \"{lv.SelectedItems[0].Text}\""
            };

            renameForm.Controls.Add(new Label()
            {
                Location = new Point(12, 12),
                Text = "Rename to: "
            });

            var textBox = new TextBox()
            {
                Location = new Point(30, 38),
                Size = new Size(330, 20),
                Text = lv.SelectedItems[0].Text
            };

            textBox.KeyDown += new KeyEventHandler(RenameTextBox_KeyDown);
            renameForm.Controls.Add(textBox);
            renameForm.ShowDialog();
        }

        private void RenameTextBox_KeyDown(object sender, KeyEventArgs e)
        {
            var tbx = sender as TextBox;
            var form = tbx.Parent as Form;
            if (tbx == null) return;

            if (e.KeyCode == Keys.Return && tbx.Text.Length > 1)
            {
                var lv = tabControl.SelectedTab.Controls[0] as ListView;
                foreach (ArchiveFile file in CurrentArchive.Files)
                {
                    if (file.Name == lv.SelectedItems[0].Text)
                    {
                        file.Name = tbx.Text;
                        form.Close();
                        RefreshGUI();
                        RefreshTabPage(tabControl.SelectedIndex);
                        form.DialogResult = DialogResult.OK;
                        form.Close();
                        return;
                    }
                }
                form.Close();
            }

            if (e.KeyCode == Keys.Escape)
            {
                form.DialogResult = DialogResult.Cancel;
                form.Close();
            }
        }

        private void SelectAllToolStripMenuItem_Click(object sender, EventArgs e)
        {
            foreach (ListViewItem item in (tabControl.SelectedTab.Controls[0] as ListView).Items)
            {
                item.Selected = true;
            }
        }

        private void CopyToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ListView lv = tabControl.SelectedTab.Controls[0] as ListView;
            Archive ar = CurrentArchive;
            if (lv == null) return;

            string path = Path.Combine(tempPath, "Extracted_Files\\");
            Directory.CreateDirectory(path);
            List<string> fileList = new List<string>();

            new System.Threading.Thread(() =>
            {
                Invoke(new Action(() => Enabled = false));
                ToolStripProgressBar pb = new ToolStripProgressBar();
                statusStrip.Invoke(new Action(() => statusStrip.Items.AddRange(new ToolStripItem[] { pb })));
                Invoke(new Action(() => pb.Maximum = lv.SelectedItems.Count));
                Invoke(new Action(() =>
                {
                    for (int i = 0; i < lv.SelectedItems.Count; ++i)
                    {
                        for (int i2 = 0; i2 < ar.Files.Count; ++i2)
                        {
                            if (ar.Files[i2].Name == lv.SelectedItems[i].SubItems[0].Text)
                            {
                                string filePath = Path.Combine(path, ar.Files[i2].Name);
                                ar.Files[i2].Extract(filePath);
                                fileList.Add(filePath);
                                ++pb.Value;
                                break;
                            }
                        }
                    }
                }));

                Invoke(new Action(() => Clipboard.SetData(DataFormats.FileDrop, fileList.ToArray())));
                statusStrip.Invoke(new Action(() => statusStrip.Items.Remove(pb)));
                Invoke(new Action(() => Enabled = true));
            })
            { Name = "CopyExtractSelectedItems" }.Start();
        }

        private void PasteToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if(Clipboard.GetData(DataFormats.FileDrop) is string[] files)
            {
                CurrentArchive.Saved = false;

                foreach (var file in files)
                {
                    for (int i = 0; i < CurrentArchive.Files.Count; ++i)
                    {
                        ArchiveFile file2 = CurrentArchive.Files[i];
                        var fileInfo = new FileInfo(file);

                        if (fileInfo.Name.ToLower() == file2.Name.ToLower())
                        {
                            if (MessageBox.Show($"There's already a file called {file2.Name}.\n" +
                                $"Do you want to replace {fileInfo.Name}?", Text,
                                MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.No)
                                return;

                            CurrentArchive.Files.Remove(file2);
                        }
                    }
                    CurrentArchive.Files.Add(new ArchiveFile(file));
                }
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
                Archive archive = Archives[i];
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

        private void Lv_KeyPress(object sender, KeyPressEventArgs e)
        {
            // Enter key
            if (e.KeyChar == (char)13)
            {
                Lv_MouseDoubleClick(sender, null);
            }
        }

        private void Lv_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            var lv = sender as ListView;
            var ar = CurrentArchive;
            var path = Path.Combine(tempPath, "Extracted_Files\\");
            Directory.CreateDirectory(path);
            if (lv.SelectedItems.Count == 1)
            {
                for (int i = 0; i < ar.Files.Count; ++i)
                {
                    if (ar.Files[i].Name == lv.SelectedItems[0].SubItems[0].Text)
                    {
                        var filePath = Path.Combine(path, ar.Files[i].Name);
                        ar.Files[i].Extract(filePath);
                        System.Diagnostics.Process.Start(filePath);
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
            ListView lv = sender as ListView;
            if (e.Button == MouseButtons.Left && lv.SelectedItems.Count > 0 &&
                !lv.FocusedItem.Bounds.Contains(lv.PointToClient(MousePosition)))
            {
                List<string> fileList = new List<string>();
                try
                {
                    if (!extracting && !extracted)
                    {
                        if (!extracted) extracting = true;
                        Invoke(new Action(() =>
                        {
                            string path = Path.Combine(tempPath, "Extracted_Files\\");
                            Directory.CreateDirectory(path);
                            Enabled = false;
                            Archive ar = CurrentArchive;
                            ToolStripProgressBar pb = new ToolStripProgressBar();
                            statusStrip.Items.AddRange(new ToolStripItem[] { pb });
                            pb.Maximum = lv.SelectedItems.Count;
                            for (int i = 0; i < lv.SelectedItems.Count; ++i)
                            {
                                for (int i2 = 0; i2 < ar.Files.Count; ++i2)
                                {
                                    if (ar.Files[i2].Name == lv.SelectedItems[i].SubItems[0].Text)
                                    {
                                        string filePath = Path.Combine(path, ar.Files[i2].Name);
                                        ar.Files[i2].Extract(filePath);
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
                    DataObject d = new DataObject(DataFormats.FileDrop, fileList.ToArray());
                    DoDragDrop(d, DragDropEffects.Copy);
                }

                if (fileList.Count == 0 && extracted)
                {
                    extracted = false;
                }

            }
        }

        private void Lv_ColumnClick(object sender, ColumnClickEventArgs e)
        {
            ListView lv = sender as ListView;
            lv.Sorting = (lv.Sorting == SortOrder.Ascending) ?
                SortOrder.Descending : SortOrder.Ascending;

            lv.ListViewItemSorter = new ListViewItemSorter(e.Column, lv.Sorting);
            lv.Sort();
        }

        //Other
        private class ListViewItemSorter : IComparer
        {
            private int col = 0;
            private SortOrder order = SortOrder.Ascending;

            public ListViewItemSorter(int column, SortOrder order)
            {
                col = column;
                this.order = order;
            }

            public int Compare(object x, object y)
            {
                int returnVal = -1;
                returnVal = string.Compare(((ListViewItem)x).SubItems[col].Text,
                                        ((ListViewItem)y).SubItems[col].Text);

                // Sorting by file size
                if (col == 2)
                {
                    try
                    {
                        string xx = ((ListViewItem)x).SubItems[col].Text;
                        string yy = ((ListViewItem)y).SubItems[col].Text;

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

                if (order == SortOrder.Descending) returnVal *= -1;
                return returnVal;
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
                uint flags = 0x110; // SHGFI_ICON | SHGFI_USEFILEATTRIBUTES

                SHGetFileInfo(name, fileAttributes, ref shfi, (uint)System.Runtime.InteropServices.Marshal.SizeOf(shfi), flags);

                var icon = (Icon)Icon.FromHandle(shfi.iconHandle).Clone();
                DestroyIcon(shfi.iconHandle);
                return icon;
            }
            catch { return SystemIcons.Error; }
        }
    }
}