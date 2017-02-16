using HedgeLib.Archives;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Windows.Forms;

namespace HedgeArchiveEditor
{
    public partial class MainFrm : Form
    {
        //Variables/Constants
        public List<Archive> Archives = new List<Archive>();
        public Archive CurrentArchive
        {
            get
            {
                return (tabControl.SelectedIndex >= 0 && tabControl.SelectedIndex < Archives.Count)
                    ? Archives[tabControl.SelectedIndex] : null;
            }

            set
            {
                if (tabControl.SelectedIndex >= 0 && tabControl.SelectedIndex < Archives.Count)
                    Archives[tabControl.SelectedIndex] = value;
            }
        }

        //Constructors
        public MainFrm()
        {
            InitializeComponent();
            UpdateTitle();
        }

        //Methods
        public void UpdateTitle()
        {
            Text = Program.ProgramName; //TODO
        }

        public void OpenArchive(string filePath)
        {
            var arc = Program.LoadArchive(filePath);
            Archives.Add(arc);
            AddTabPage(new FileInfo(filePath).Name);
        }

        public void SaveArchive(int index)
        {
            var sfd = new SaveFileDialog()
            {
                Title = "Save Archive As...",
                Filter = "Generations/Unleashed Archives (*.ar, *.arl, *.pfd)|*.ar;*.arl;*.pfd" +
                "|Lost World Archives (*.pac)|*.pac",
            };

            if (sfd.ShowDialog() == DialogResult.OK)
            {
                if (sfd.FileName.EndsWith(".ar")) sfd.FileName = Path.ChangeExtension(sfd.FileName, ".arl");
                var saveOptions = new SaveOptions();
                if (saveOptions.ShowDialog() == DialogResult.OK)
                {
                    //This is a horrible way of checking this, I know.
                    int val = saveOptions.comboBox1.SelectedIndex;
                    if (val == 0)
                    {
                        var genArc = new GensArchive(CurrentArchive);
                        genArc.Padding = (uint)saveOptions.numericUpDown1.Value;
                        genArc.Save(sfd.FileName);
                    }

                    //TODO: Add other archive types.
                }
            }
        }

        public void CloseArchive(int index)
        {
            //TODO: Prompt the user to save the archive first if not yet saved.

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
            //TODO
            //tp.Text = (tp.Tag as string) + ((arc.Saved) ? "" : "*");

            //Update File List
            if (!refreshFileList || lv == null) return;
            lv.Items.Clear();

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
            Text = ((tabControl.TabPages.Count > 0) ?
                $"{tabControl.SelectedTab.Tag.ToString()} - " : "") + Program.ProgramName;
        }

        private void Lv_ColumnClick(object sender, ColumnClickEventArgs e)
        {
            ListView lv = sender as ListView;
            lv.Sorting = (lv.Sorting == SortOrder.Ascending) ?
                SortOrder.Descending : SortOrder.Ascending;

            lv.ListViewItemSorter = new ListViewItemSorter(e.Column, lv.Sorting);
            lv.Sort();
        }

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

                if (order == SortOrder.Descending) returnVal *= -1;
                return returnVal;
            }
        }

        //GUI Events
        private void createNewArchiveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Archives.Add(new Archive());
            AddTabPage("Untitled");
            RefreshGUI();
        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var ofd = new OpenFileDialog()
            {
                Title = "Open Archive(s)...",
                Filter = "Generations/Unleashed Archives (*.ar, *.arl, *.pfd)|*.ar;*.arl;*.pfd" +
                "|Lost World Archives (*.pac)|*.pac|All Files (*.*)|*.*",

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

        private void addFilesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var ofd = new OpenFileDialog()
            {
                Title = "Add File(s)...",
                Filter = "All Files (*.*)|*.*",
                Multiselect = true
            };

            if (ofd.ShowDialog() == DialogResult.OK)
            {
                foreach (var file in ofd.FileNames)
                    CurrentArchive.Files.Add(new ArchiveFile(file));

                RefreshTabPage(tabControl.SelectedIndex);
            }
        }

        private void extractAllToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog sfd = new SaveFileDialog()
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
                        ToolStripProgressBar pb = new ToolStripProgressBar();
                        statusStrip.Invoke(new Action(() => statusStrip.Items.AddRange(new ToolStripItem[] { pb })));
                        Invoke(new Action(() => pb.Maximum = ar.Files.Count));
                        foreach (ArchiveFile file in ar.Files)
                        {
                            file.Extract(HedgeLib.Helpers.CombinePaths(new FileInfo(sfd.FileName).Directory.FullName, file.Name));
                            Invoke(new Action(() => pb.Value++));
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

        private void closeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            CloseArchive(tabControl.SelectedIndex);
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void tabControl_SelectedIndexChanged(object sender, EventArgs e)
        {
            RefreshGUI();
        }

        private void contextMenu_Opening(object sender, System.ComponentModel.CancelEventArgs e)
        {
            var lv = tabControl.SelectedTab.Controls[0] as ListView;

            extractSelectedFilesToolStripMenuItem.Enabled =
                removeSelectedFilesToolStripMenuItem.Enabled = lv.SelectedItems.Count > 0;
        }

        private void extractSelectedFilesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog sfd = new SaveFileDialog()
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
                        ToolStripProgressBar pb = new ToolStripProgressBar();
                        statusStrip.Invoke(new Action(() => statusStrip.Items.AddRange(new ToolStripItem[] { pb })));
                        ListView lv = null;
                        Invoke(new Action(() => lv = (ListView)tabControl.SelectedTab.Controls[0]));
                        Invoke(new Action(() => pb.Maximum = lv.SelectedItems.Count));
                        Invoke(new Action(() =>
                        {
                            for (int i = 0; i < lv.SelectedItems.Count; i++)
                            {
                                for (int ii = 0; ii < ar.Files.Count; ii++)
                                {
                                    if (ar.Files[ii].Name == lv.SelectedItems[i].SubItems[0].Text)
                                    {
                                        ar.Files[ii].Extract(Path.Combine(new FileInfo(sfd.FileName).Directory.FullName, ar.Files[ii].Name));
                                        Invoke(new Action(() => pb.Value++));
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

        private void removeSelectedFilesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
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
                        for (int ii = 0; ii < ar.Files.Count; ii++)
                        {
                            for (int i = 0; i < lv.SelectedItems.Count; i++)
                            {
                                if (ar.Files[ii].Name == lv.SelectedItems[i].SubItems[0].Text)
                                {
                                    Invoke(new Action(() => pb.Value++));
                                    ar.Files.Remove(ar.Files[ii]);
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

        private void createFromDirectoryToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog sfd = new SaveFileDialog()
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

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveArchive(tabControl.SelectedIndex);
        }
    }
}