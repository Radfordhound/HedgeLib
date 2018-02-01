using HedgeLib;
using HedgeLib.Sets;
using System;
using System.Windows.Forms;

namespace HedgeEdit.UI
{
    public partial class ObjectParamEditor : Form
    {
        // Variables/Constants
        protected SetObjectParam objParam;

        // Constructors
        public ObjectParamEditor(SetObjectParam objParam,
            SetObjectTypeParam typeParam = null)
        {
            this.objParam = objParam;
            InitializeComponent();

            valueTxtBx.Text = objParam.Data.ToString();
            descriptionLbl.Text = (typeParam == null) ?
                "" : typeParam.Description;
        }

        // GUI Events
        private void OkBtn_Click(object sender, EventArgs e)
        {
            objParam.Data = Helpers.ChangeType(valueTxtBx.Text, objParam.DataType);
            DialogResult = DialogResult.OK;
            Close();
        }
    }
}