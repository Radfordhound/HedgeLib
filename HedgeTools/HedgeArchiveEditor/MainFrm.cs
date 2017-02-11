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
                return (CurrentArchvieIndex >= 0 && CurrentArchvieIndex < Archives.Count) ?
                    Archives[CurrentArchvieIndex] : null;
            }

            set
            {
                if (CurrentArchvieIndex >= 0 && CurrentArchvieIndex < Archives.Count)
                    Archives[CurrentArchvieIndex] = value;
            }
        }

        public int CurrentArchvieIndex = -1;

        //Constructors
        public MainFrm()
        {
            InitializeComponent();
            UpdateTitle();
        }

        //Methods
        public void OpenArchive(string filePath)
        {
            var arc = Program.LoadArchive(filePath);
            Archives.Add(arc);
            AddTabPage(new FileInfo(filePath).Name);
        }

        public void UpdateTitle()
        {
            Text = Program.ProgramName; //TODO
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

            lv.ColumnClick += Lv_ColumnClick;
            lv.Columns.Add("Name");
            lv.Columns.Add("Extension");

            tabPage.Controls.Add(lv);
            RefreshTabPage(tabPageIndex);
            tabControl.SelectedIndex = tabPageIndex;
        }

        public void CloseArchive(int index)
        {
            //TODO: Prompt the user to save the archive first if not yet saved.

            Archives.RemoveAt(index);
            tabControl.TabPages.RemoveAt(index);
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

            int longestNameLength = 0, longestExtensionLength = 0;
            foreach (var file in arc.Files)
            {
                FileInfo fileInfo = new FileInfo(file.Name);
                ListViewItem lvi = new ListViewItem(new string[]
                {
                    fileInfo.Name,
                    fileInfo.Extension
                });

                if (lvi.Text.Length > longestNameLength)
                    longestNameLength = lvi.Text.Length;

                if (lvi.SubItems[1].Text.Length > longestExtensionLength)
                    longestExtensionLength = lvi.SubItems[1].Text.Length;

                lv.Items.Add(lvi);
            }

            //Update the columns in the file list
            lv.AutoResizeColumn(0, (longestNameLength > lv.Columns[0].Text.Length) ?
                ColumnHeaderAutoResizeStyle.ColumnContent :
                ColumnHeaderAutoResizeStyle.HeaderSize);

            lv.AutoResizeColumn(1, (longestExtensionLength > lv.Columns[1].Text.Length) ?
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
            CurrentArchvieIndex = tabControl.SelectedIndex;
            RefreshGUI();
        }
    }
}