using System;
using System.Configuration;
using System.IO;
using System.Windows.Forms;

namespace HedgeEdit.UI
{
    public partial class StgOpenDialog : Form
    {
        // Variables/Constants
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

        // Constructors
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

            // Load from config file
            var config = ConfigurationManager.OpenExeConfiguration(ConfigurationUserLevel.None);
            if (config.AppSettings.Settings["LastDataPath"] != null)
                dataDirTxtbx.Text = config.AppSettings.Settings["LastDataPath"].Value;
            if (config.AppSettings.Settings["LastStageID"] != null)
                stageIDTxtbx.Text = config.AppSettings.Settings["LastStageID"].Value;
            if (config.AppSettings.Settings["LastGameName"] != null)
                gameComboBx.Text = config.AppSettings.Settings["LastGameName"].Value;
        }

        // GUI Events
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

        private void OkBtn_Click(object sender, EventArgs e)
        {
            // Save the config
            var config = ConfigurationManager.OpenExeConfiguration(ConfigurationUserLevel.None);
            config.AppSettings.Settings.Remove("LastDataPath");
            config.AppSettings.Settings.Remove("LastStageID");
            config.AppSettings.Settings.Remove("LastGameName");
            config.AppSettings.Settings.Add("LastDataPath", dataDirTxtbx.Text);
            config.AppSettings.Settings.Add("LastStageID", stageIDTxtbx.Text);
            config.AppSettings.Settings.Add("LastGameName", gameComboBx.Text);
            config.Save();
        }
    }
}