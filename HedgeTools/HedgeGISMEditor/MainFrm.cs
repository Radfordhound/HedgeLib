using HedgeLib.Misc;
using System;
using System.Collections.Generic;
using System.IO;
using System.Windows.Forms;

namespace HedgeGISMEditor
{
    public partial class MainFrm : Form
    {
        // Variables/Constants
        public LWGism Gism = null;
        public List<LWGismo> Gismos = new List<LWGismo>();
        public string FileName = null;

        // Constructors
        public MainFrm()
        {
            InitializeComponent();
        }

        // Methods
        public void UpdateGUI()
        {
            mainPnl.Enabled = (Gismos != null);
            if (Gism == null) return;

            checkBox1.Checked = (Gism.UnknownBoolean1 == 1);
            listBox1.Items.Clear();

            foreach (var gismo in Gismos)
            {
                listBox1.Items.Add(gismo.FileName);
            }
        }

        public void EditGismo(int index)
        {
            var gismo = Gismos[index];
            var editGismoFrm = new EditGismoFrm(gismo);

            if (editGismoFrm.ShowDialog() == DialogResult.OK)
                UpdateGUI();
        }

        public void OpenGISM(string filePath)
        {
            FileName = filePath;
            Gism = new LWGism();

            Gism.Load(FileName);
            Gismos = new List<LWGismo>(Gism.Gismos);

            UpdateGUI();
        }

        public void SaveGISM(bool forceSaveAs = false)
        {
            if (forceSaveAs || string.IsNullOrEmpty(FileName))
            {
                var sfd = new SaveFileDialog()
                {
                    Title = "Save GISM File...",
                    Filter = "Lost World GISMo lists (*.gism)|*.gism"
                };

                if (sfd.ShowDialog() != DialogResult.OK)
                    return;

                FileName = sfd.FileName;
            }

            var dr = MessageBox.Show("Would you like to save the file as Big Endian " +
                "(Wii U)? Choosing \"No\" will save the file as Little Endian (PC).",
                Text, MessageBoxButtons.YesNoCancel, MessageBoxIcon.Information,

                (Gism.IsBigEndian) ?
                    MessageBoxDefaultButton.Button1 : MessageBoxDefaultButton.Button2);

            if (dr == DialogResult.Cancel) return;
            Gism.Gismos = Gismos.ToArray();
            Gism.UnknownBoolean1 = (checkBox1.Checked) ? 1u : 0u;
            Gism.Header.IsBigEndian = (dr == DialogResult.Yes);
            Gism.Save(FileName, true);
        }

        // GUI Events
        private void CreateNewGISMToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Gism = new LWGism();
            Gismos.Clear();
            UpdateGUI();
        }

        private void OpenToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var ofd = new OpenFileDialog()
            {
                Title = "Open GISM File...",
                Filter = "Lost World GISMo lists (*.gism)|*.gism"
            };

            if (ofd.ShowDialog() == DialogResult.OK)
                OpenGISM(ofd.FileName);
        }

        private void SaveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveGISM();
        }

        private void SaveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveGISM(true);
        }

        private void ExitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void ListBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            rmGismoBtn.Enabled = editGismoBtn.Enabled = (listBox1.SelectedItem != null);
        }

        private void RmGismoBtn_Click(object sender, EventArgs e)
        {
            if (listBox1.SelectedItem == null) return;
            Gismos.RemoveAt(listBox1.SelectedIndex);
            UpdateGUI();
        }

        private void EditGismoBtn_Click(object sender, EventArgs e)
        {
            if (listBox1.SelectedItem == null) return;
            EditGismo(listBox1.SelectedIndex);
        }

        private void ListBox1_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            var index = listBox1.IndexFromPoint(e.Location);
            if (index != ListBox.NoMatches)
                EditGismo(index);
        }

        private void AddGismoBtn_Click(object sender, EventArgs e)
        {
            var gismo = new LWGismo();
            gismo.HavokName = gismo.FileName = ("gismo" + Gismos.Count);
            Gismos.Add(gismo);

            UpdateGUI();
            EditGismo(Gismos.Count - 1);
        }
    }
}