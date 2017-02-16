using HedgeLib.Misc;
using System;
using System.Windows.Forms;

namespace HedgeGISMEditor
{
    public partial class EditGismoFrm : Form
    {
        //Variables/Constants
        private LWGismo gismo;

        //Constructors
        public EditGismoFrm(LWGismo gismo)
        {
            InitializeComponent();
            this.gismo = gismo;
            UpdateGUI();
        }

        //Methods
        public void UpdateGUI()
        {
            //Container 1
            fileNameTxtBx.Text = gismo.FileName;
            unknown1Nud.Value = gismo.Unknown1;
            unknown2Nud.Value = (decimal)gismo.Unknown2;
            unknown3Nud.Value = (decimal)gismo.Unknown3;

            doesAnimateChkbx.Checked = gismo.DoesAnimate;
            unknownBoolean1Chkbx.Checked = gismo.UnknownBoolean1;

            //Havok Container
            havokNameTxtBx.Text = gismo.HavokName;

            //Container 2
            unknownBoolean2Chkbx.Checked = gismo.UnknownBoolean2;
            unknownBoolean3Chkbx.Checked = gismo.UnknownBoolean3;

            unknown5Nud.Value = (decimal)gismo.Unknown5;
            unknown6Nud.Value = (decimal)gismo.Unknown6;
            unknown7Nud.Value = (decimal)gismo.Unknown7;
            unknown8Nud.Value = (decimal)gismo.Unknown8;
            rotationAmountNud.Value = (decimal)gismo.RotationAmount;
            unknown9Nud.Value = (decimal)gismo.Unknown9;
        }

        public void UpdateGismo()
        {
            //Container 1
            gismo.FileName = fileNameTxtBx.Text;
            gismo.Unknown1 = (uint)unknown1Nud.Value;
            gismo.Unknown2 = (float)unknown2Nud.Value;
            gismo.Unknown3 = (float)unknown3Nud.Value;

            gismo.DoesAnimate = doesAnimateChkbx.Checked;
            gismo.UnknownBoolean1 = unknownBoolean1Chkbx.Checked;

            //Havok Container
            gismo.HavokName = havokNameTxtBx.Text;

            //Container 2
            gismo.UnknownBoolean2 = unknownBoolean2Chkbx.Checked;
            gismo.UnknownBoolean3 = unknownBoolean3Chkbx.Checked;

            gismo.Unknown5 = (float)unknown5Nud.Value;
            gismo.Unknown6 = (float)unknown6Nud.Value;
            gismo.Unknown7 = (float)unknown7Nud.Value;
            gismo.Unknown8 = (float)unknown8Nud.Value;
            gismo.RotationAmount = (float)rotationAmountNud.Value;
            gismo.Unknown9 = (float)unknown9Nud.Value;
        }

        //GUI Events
        private void okBtn_Click(object sender, EventArgs e)
        {
            UpdateGismo();
            DialogResult = DialogResult.OK;
            Close();
        }

        private void FileNameTxtBx_LostFocus(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(havokNameTxtBx.Text))
                havokNameTxtBx.Text = fileNameTxtBx.Text;
        }
    }
}