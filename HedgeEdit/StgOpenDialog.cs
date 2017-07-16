using System;
using System.IO;
using System.Windows.Forms;

namespace HedgeEdit
{
    public partial class StgOpenDialog : Form
    {
        //Variables/Constants
        public string DataDir
        {
            get
            {
                return dataDirTxtbx.Text;
            }
        }

        public string StageID
        {
            get
            {
                return stageIDTxtbx.Text;
            }
        }

        public int GameID
        {
            get
            {
                return gameComboBx.SelectedIndex;
            }
        }

        //Constructors
        public StgOpenDialog()
        {
            InitializeComponent();

            // Populate games list
            foreach (var game in GameList.Games)
            {
                gameComboBx.Items.Add(game.Name);
            }

            if (gameComboBx.Items.Count > 0)
                gameComboBx.SelectedIndex = 0;
        }

        //GUI Events
        private void DataDirBtn_Click(object sender, EventArgs e)
        {
            var fbd = new FolderBrowserDialog()
            {
                Description = "", // TODO
                SelectedPath = (Directory.Exists(dataDirTxtbx.Text)) ?
                    dataDirTxtbx.Text : null
            };

            if (fbd.ShowDialog() == DialogResult.OK)
            {
                dataDirTxtbx.Text = fbd.SelectedPath;
            }
        }

        private void DataDirTxtbx_TextChanged(object sender, EventArgs e)
        {
            okBtn.Enabled = (Directory.Exists(dataDirTxtbx.Text));
        }
    }
}