using System;
using System.Windows.Forms;

namespace HedgeEdit.UI
{
    public partial class TxtBxDialog : Form
    {
        // Variables/Constants
        public string Result;
        protected bool allowEmpty = false;

        // Constructors
        public TxtBxDialog(string lbl, string title = null,
            string startText = null, bool allowEmpty = false)
        {
            InitializeComponent();
            label.Text = lbl;
            this.allowEmpty = allowEmpty;

            if (!string.IsNullOrEmpty(title))
                Text = title;

            if (!string.IsNullOrEmpty(startText))
                Result = textBox.Text = startText;

            comboBox.Visible = false;
            UpdateOKEnabled();
        }

        public TxtBxDialog(string lbl, string[] choices,
            string title = null)
        {
            InitializeComponent();
            label.Text = lbl;

            if (!string.IsNullOrEmpty(title))
                Text = title;

            textBox.Visible = false;
            comboBox.Items.AddRange(choices);
            UpdateOKEnabled();
        }

        // Methods
        protected void UpdateOKEnabled()
        {
            okBtn.Enabled = (textBox.Visible && (allowEmpty ||
                !string.IsNullOrWhiteSpace(textBox.Text))) || (comboBox.Visible &&
                comboBox.SelectedIndex >= 0);
        }

        // GUI Events
        protected void OkBtn_Click(object sender, EventArgs e)
        {
            Result = (comboBox.Visible) ? (string)comboBox.SelectedItem : textBox.Text;
        }

        protected void ValueChanged(object sender, EventArgs e)
        {
            UpdateOKEnabled();
        }
    }
}