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
            ComboBox1.Text = ComboBox1.Items[ArchiveType] as string;
        }

        public SaveOptions(int archiveType)
        {
            InitializeComponent();
            ArchiveType = archiveType;
            ComboBox1.Text = ComboBox1.Items[archiveType] as string;
        }

        //GUI Events
        private void OkBtn_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.OK;
            // This may not be the best way of checking
            switch (ArchiveType)
            {
                case 0:
                    break;
                case 1:
                    throw new NotImplementedException("Lost World Archive");
                case 2:
                    break;
                case 3:
                    break;
                default:
                    throw new NotImplementedException("Unknown Archive");
            }
            Close();
        }

        private void CancelBtn_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.Cancel;
            ArchiveType = -1;
            Close();
        }

        private void CheckBox2_CheckedChanged(object sender, EventArgs e)
        {
            label5.Enabled = NumericUpDown2.Enabled = CheckBox2.Checked;
        }

        private void ComboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            ArchiveType = ComboBox1.SelectedIndex;
            NumericUpDown1.Visible = NumericUpDown2.Visible = CheckBox1.Visible
                = CheckBox2.Visible = label2.Visible = label3.Visible
                = label4.Visible = label5.Visible = false;

            switch (ArchiveType)
            {
                case 0:
                    NumericUpDown1.Visible = NumericUpDown2.Visible =
                        CheckBox1.Visible = CheckBox2.Visible =
                        label2.Visible = label3.Visible = label4.Visible = label5.Visible = true;
                    label2.Text = "Padding: ";
                    break;
                case 1:
                    break;
                case 2:
                    break;
                case 3:
                    NumericUpDown1.Visible = label2.Visible = true;
                    label2.Text = "HeroesMagic: ";
                    break;
                default:
                    throw new NotImplementedException("Unknown Archive");
            }
        }
    }
}