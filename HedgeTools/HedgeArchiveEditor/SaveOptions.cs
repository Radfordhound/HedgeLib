using System;
using System.Windows.Forms;

namespace HedgeArchiveEditor
{
    public partial class SaveOptions : Form
    {
        // Variables/Constants
        public int ArchiveType = -1;

        // Constructors
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

            // Addon
            foreach (var addon in Addon.Addons)
                foreach (var archive in addon.Archives)
                    ComboBox1.Items.Add(archive.ArchiveName);

            ComboBox1.Text = ComboBox1.Items[archiveType] as string;
        }

        public void UpdateOption(Label label, Control value, int index)
        {
            value.TabIndex = index + 1;
            
            // Positions
            int x = Width - (14 + value.Size.Width);
            int y = 30 + index * 22;
            
            // Sets the positions
            label.Location = new System.Drawing.Point(8, y + 2);
            value.Location = new System.Drawing.Point(x - 8, y + (22 - value.Size.Height) / 2);
            
            // Makes the option visible
            label.Visible = true;
            value.Visible = true;
        }
        
        public void HideAllOptions()
        {
            NumericUpDown1.Visible = NumericUpDown2.Visible = CheckBox1.Visible
                = CheckBox2.Visible = CheckBox3.Visible = label2.Visible = label3.Visible
                = label4.Visible = label5.Visible = label6.Visible = false;
        }


        // GUI Events
        private void Btn_Click(object sender, EventArgs e)
        {
            Close();
        }

        // Split Into Multiple Files
        private void CheckBox2_CheckedChanged(object sender, EventArgs e)
        {
            label5.Enabled = NumericUpDown2.Enabled =
                label6.Enabled = CheckBox3.Enabled = CheckBox2.Checked;
        }

        private void ComboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            ArchiveType = ComboBox1.SelectedIndex;

            // Hides all the options
            HideAllOptions();

            switch (ArchiveType)
            {
                case 0: // Generations/Unleashed
                    // Show and Position the Options
                    UpdateOption(label2, NumericUpDown1, 0);    // Padding
                    UpdateOption(label3, CheckBox1, 1);         // Generatate ARL file
                    UpdateOption(label4, CheckBox2, 2);         // Split Into Multiple Files
                    UpdateOption(label5, NumericUpDown2, 3);    // Split Size
                    UpdateOption(label6, CheckBox3, 4);         // Delete Old Split Files
                    
                    // Sets the Label names
                    label2.Text = "Padding: ";
                    break;
                case 1: // Lost World
                    // Show and Position the Options
                    UpdateOption(label4, CheckBox2, 0);         // Split Into Multiple Files
                    UpdateOption(label6, CheckBox3, 1);         // Delete Old Split Files

                    // Sets the Label names

                    break;
                case 2: // Story Books
                    // Show and Position the Options

                    // Sets the Label names

                    break;
                case 3: // Heroes/Shadow
                    // Show and Position the Options
                    UpdateOption(label2, NumericUpDown1, 0);    // HeroesMagic

                    // Sets the Label names
                    label2.Text = "HeroesMagic: ";

                    break;
                default:
                    break;
            }
        }
    }
}