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
        }

        //GUI Events
        private void button_Click(object sender, EventArgs e)
        {
            Close();
        }
    }
}