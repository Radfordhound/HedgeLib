using System;
using System.Windows.Forms;
using HedgeLib.Misc;
using System.Linq;

namespace HedgeCnvrsEditor
{
    public partial class ParameterEditor : Form
    {
        object Param;
        ForcesText forcesText;
        int Index;

        // Cache
        int tempInt;
        float tempFloat;
        ulong tempULong;

        public ParameterEditor(object param,int index, ForcesText text)
        {
            Param = param;
            Index = index;
            forcesText = text;
            InitializeComponent();

            #region Parameter Stuff

            typeBx.Enabled = false;
            if (Param is ForcesText.Cell cell)
            {
                switch (Index)
                {
                    case 0:
                        {
                            typeBx.Text = cell.LayoutIndex.GetType().Name;
                            dataBx.Text = cell.LayoutIndex.ToString();
                            break;
                        }
                    case 1:
                        {
                            typeBx.Enabled = true;
                            typeBx.Text = cell.TypeName;
                            dataBx.Lines = cell.Data.Split('\n');
                            break;
                        }
                    case 2:
                        {
                            typeBx.Text = cell.UUID.GetType().Name;
                            dataBx.Text = cell.UUID.ToString();
                            break;
                        }
                    default:
                        throw new Exception("Invalid Index");
                }
            }
            else if (Param is ForcesText.Layout layout)
            {
                switch (Index)
                {
                    case 0:
                        {
                            typeBx.Text = "Int32";
                            dataBx.Text = layout.UnknownData1.ToString();
                            break;
                        }
                    case 1:
                        {
                            typeBx.Text = "Single";
                            dataBx.Text = layout.UnknownData2.ToString();
                            break;
                        }
                    case 2:
                        {
                            typeBx.Text = "Single";
                            dataBx.Text = layout.UnknownData3.ToString();
                            break;
                        }
                    case 3:
                        {
                            typeBx.Text = "Int32";
                            dataBx.Text = layout.UnknownData4.ToString();
                            break;
                        }
                    case 4:
                        {
                            typeBx.Text = "Int32";
                            dataBx.Text = layout.UnknownData5.ToString();
                            break;
                        }
                    case 5:
                        {
                            typeBx.Text = "Int32";
                            dataBx.Text = layout.UnknownData6.ToString();
                            break;
                        }
                    case 6:
                        {
                            typeBx.Text = "Int32";
                            dataBx.Text = layout.UnknownData7.ToString();
                            break;
                        }
                    case 7:
                        {
                            typeBx.Text = "Int64";
                            dataBx.Text = layout.UnknownData8.ToString();
                            break;
                        }
                    default:
                        throw new Exception("Invalid Index");
                }
            }
            else if (Param is ForcesText.CellType cellType)
            {
                switch (Index)
                {
                    case 0:
                        {
                            typeBx.Text = "String";
                            dataBx.Text = cellType.Namespace.ToString();
                            break;
                        }
                    case 1:
                        {
                            typeBx.Text = "Single";
                            dataBx.Text = cellType.UnknownFloat1.ToString();
                            break;
                        }
                    case 2:
                        {
                            typeBx.Text = "Single";
                            dataBx.Text = cellType.UnknownFloat2.ToString();
                            break;
                        }
                    case 3:
                        {
                            typeBx.Text = "Single";
                            dataBx.Text = cellType.UnknownFloat3.ToString();
                            break;
                        }
                    case 4:
                        {
                            typeBx.Text = "Int32";
                            dataBx.Text = cellType.UnknownInt1.ToString();
                            break;
                        }
                    case 5:
                        {
                            typeBx.Text = "Int32";
                            dataBx.Text = cellType.UnknownInt2.ToString();
                            break;
                        }
                    case 6:
                        {
                            typeBx.Text = "Int64";
                            dataBx.Text = cellType.UnknownULong1.ToString();
                            break;
                        }
                    case 7:
                        {
                            typeBx.Text = "Int64";
                            dataBx.Text = cellType.UnknownULong2.ToString();
                            break;
                        }
                    default:
                        throw new Exception("Invalid Index");
                }
            }
            #endregion
        }

        private void OkBtn_Click(object sender, EventArgs e)
        {
            #region More Param Stuff
            if (Param is ForcesText.Cell cell)
            {
                switch (Index)
                {
                    case 0:
                        {
                            cell.LayoutIndex = int.TryParse(dataBx.Text, out tempInt) ?
                                tempInt >= forcesText.Layouts.Count ? forcesText.Layouts.Count - 1 : tempInt : 0;
                            break;
                        }
                    case 1:
                        {
                            cell.TypeName = typeBx.Text;
                            cell.Data = dataBx.Text.Replace("\r","");
                            break;
                        }
                    case 2:
                        {
                            cell.UUID = ulong.TryParse(dataBx.Text, out tempULong) ?
                                tempULong : 0;
                            break;
                        }
                    default:
                        throw new Exception("Invalid Index");
                }
            }
            else if (Param is ForcesText.Layout layout)
            {
                switch (Index)
                {
                    case 0:
                        {
                            layout.UnknownData1 = int.TryParse(dataBx.Text, out tempInt) ?
                                tempInt : (dataBx.Text.ToLower() == "null") ? (int?)null : 0;
                            break;
                        }
                    case 1:
                        {
                            layout.UnknownData2 = float.TryParse(dataBx.Text, out tempFloat) ?
                                tempFloat : ((dataBx.Text.ToLower() == "null") ? (float?)null : 0);
                            break;
                        }
                    case 2:
                        {
                            layout.UnknownData3 = float.TryParse(dataBx.Text, out tempFloat) ?
                                tempFloat : (dataBx.Text.ToLower() == "null") ? (float?)null : 0;
                            break;
                        }
                    case 3:
                        {
                            layout.UnknownData4 = int.TryParse(dataBx.Text, out tempInt) ?
                                tempInt : (dataBx.Text.ToLower() == "null") ? (int?)null : 0;
                            break;
                        }
                    case 4:
                        {
                            layout.UnknownData5 = int.TryParse(dataBx.Text, out tempInt) ?
                                tempInt : (dataBx.Text.ToLower() == "null") ? (int?)null : 0;
                            break;
                        }
                    case 5:
                        {
                            layout.UnknownData6 = int.TryParse(dataBx.Text, out tempInt) ?
                                tempInt : (dataBx.Text.ToLower() == "null") ? (int?)null : 0;
                            break;
                        }
                    case 6:
                        {
                            layout.UnknownData7 = int.TryParse(dataBx.Text, out tempInt) ?
                                tempInt : (dataBx.Text.ToLower() == "null") ? (int?)null : 0;
                            break;
                        }
                    case 7:
                        {
                            layout.UnknownData8 = int.TryParse(dataBx.Text, out tempInt) ?
                                tempInt : 0;
                            break;
                        }
                    default:
                        throw new Exception("Invalid Index");
                }
            }
            else if (Param is ForcesText.CellType cellType)
            {
                switch (Index)
                {
                    case 0:
                        {
                            cellType.Namespace = dataBx.Text;
                            break;
                        }
                    case 1:
                        {
                            cellType.UnknownFloat1 = float.TryParse(dataBx.Text, out tempFloat) ?
                                tempFloat : (dataBx.Text.ToLower() == "null") ? (float?)null : 0;
                            break;
                        }
                    case 2:
                        {
                            cellType.UnknownFloat2 = float.TryParse(dataBx.Text, out tempFloat) ?
                                tempFloat : (dataBx.Text.ToLower() == "null") ? (float?)null : 0;
                            break;
                        }
                    case 3:
                        {
                            cellType.UnknownFloat3 = float.TryParse(dataBx.Text, out tempFloat) ?
                                tempFloat : (dataBx.Text.ToLower() == "null") ? (float?)null : 0;
                            break;
                        }
                    case 4:
                        {
                            cellType.UnknownInt1 = int.TryParse(dataBx.Text, out tempInt) ?
                                tempInt : (dataBx.Text.ToLower() == "null") ? (int?)null : 0;
                            break;
                        }
                    case 5:
                        {
                            cellType.UnknownInt2 = int.TryParse(dataBx.Text, out tempInt) ?
                                tempInt : (dataBx.Text.ToLower() == "null") ? (int?)null : 0;
                            break;
                        }
                    case 6:
                        {
                            cellType.UnknownULong1 = ulong.TryParse(dataBx.Text, out tempULong) ?
                                tempULong : (dataBx.Text.ToLower() == "null") ? (ulong?)null : 0;
                            break;
                        }
                    case 7:
                        {
                            cellType.UnknownULong2 = ulong.TryParse(dataBx.Text, out tempULong) ?
                                tempULong : (dataBx.Text.ToLower() == "null") ? (ulong?)null : 0;
                            break;
                        }
                    default:
                        throw new Exception("Invalid Index");
                }
            }
            #endregion

            Program.MainFrm.UpdateListView();
            Close();
        }

        private void CancelBtn_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.Cancel;
            Close();
        }
    }
}