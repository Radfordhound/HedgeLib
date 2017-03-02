using System;
using System.Windows.Forms;

namespace HedgeArchiveEditor
{
    public partial class SaveOptions : Form
    {

        //Variables/Constants
        public int ArchiveType = -1;

        //Constructors
        public SaveOptions()
        {
            InitializeComponent();
            ArchiveType = 0;
            comboBox1.Text = comboBox1.Items[ArchiveType] as string;
        }

        public SaveOptions(int ArchiveType)
        {
            InitializeComponent();
            this.ArchiveType = ArchiveType;
            comboBox1.Text = comboBox1.Items[ArchiveType] as string;
        }

        //GUI Events
        private void okBtn_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.OK;
            // This may not be the best way of checking
            switch (ArchiveType)
            {
                case 0:
                    break;
                case 1:
                    throw new NotImplementedException("Lost World Archive");
                default:
                    throw new NotImplementedException("Unknown Archive");
            }
            Close();
        }

        private void cancelBtn_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.Cancel;
            ArchiveType = -1;
            Close();
        }
    }
}