using System;
using System.Windows.Forms;

namespace HedgeArchiveEditor
{
    public partial class SaveOptions : Form
    {
        //Constructors
        public SaveOptions()
        {
            InitializeComponent();
            comboBox1.Text = "Generations/Unleashed";
        }

        //GUI Events
        private void button_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.OK;
            Close();
        }
    }
}