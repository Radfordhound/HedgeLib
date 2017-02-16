using HedgeLib.Misc;
using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace HedgeGISMEditor
{
    public partial class MainFrm : Form
    {
        //Variables/Constants
        public LWGism Gism = null;
        public List<LWGismo> Gismos = new List<LWGismo>();
        public string FileName = null;

        //Constructors
        public MainFrm()
        {
            InitializeComponent();
        }

        //Methods
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

            Gism.Gismos = Gismos.ToArray();
            Gism.UnknownBoolean1 = (checkBox1.Checked) ? 1u : 0u;
            Gism.Save(FileName);
        }

        //GUI Events
        private void createNewGISMToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Gism = new LWGism();
            Gismos.Clear();
            UpdateGUI();
        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var ofd = new OpenFileDialog()
            {
                Title = "Open GISM File...",
                Filter = "Lost World GISMo lists (*.gism)|*.gism"
            };

            if (ofd.ShowDialog() == DialogResult.OK)
                OpenGISM(ofd.FileName);
        }

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveGISM();
        }

        private void saveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveGISM(true);
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void listBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            rmGismoBtn.Enabled = editGismoBtn.Enabled = (listBox1.SelectedItem != null);
        }

        private void rmGismoBtn_Click(object sender, EventArgs e)
        {
            if (listBox1.SelectedItem == null) return;
            Gismos.RemoveAt(listBox1.SelectedIndex);
            UpdateGUI();
        }

        private void editGismoBtn_Click(object sender, EventArgs e)
        {
            if (listBox1.SelectedItem == null) return;
            EditGismo(listBox1.SelectedIndex);
        }

        private void listBox1_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            var index = listBox1.IndexFromPoint(e.Location);
            if (index != ListBox.NoMatches)
                EditGismo(index);
        }

        private void addGismoBtn_Click(object sender, EventArgs e)
        {
            var gismo = new LWGismo();
            gismo.HavokName = gismo.FileName = ("gismo" + Gismos.Count);
            Gismos.Add(gismo);

            UpdateGUI();
            EditGismo(Gismos.Count - 1);
        }
    }
}