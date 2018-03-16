using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Reflection;
using HedgeLib;
using HedgeLib.Misc;

namespace HedgeCnvrsEditor
{
    public partial class ParameterEditor : Form
    {
        object Param;
        ForcesText forcesText;
        int Index;

        //Cache
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
            if (Param is ForcesText.Entry entry)
            {
                switch (Index)
                {
                    case 0:
                        {
                            typeBx.Text = entry.LayoutIndex.GetType().Name;
                            dataBx.Text = entry.LayoutIndex.ToString();
                            break;
                        }
                    case 1:
                        {
                            typeBx.Enabled = true;
                            typeBx.Text = entry.TypeName;
                            dataBx.Lines = entry.Data.Split('\n');
                            break;
                        }
                    case 2:
                        {
                            typeBx.Text = entry.UUID.GetType().Name;
                            dataBx.Text = entry.UUID.ToString();
                            break;
                        }
                    default:
                        throw new Exception("Invalid Index");
                }
            }
            else if(Param is ForcesText.Layout layout)
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
            else if(Param is ForcesText.EntryType entryType)
            {
                switch (Index)
                {
                    case 0:
                        {
                            typeBx.Text = "String";
                            dataBx.Text = entryType.Namespace.ToString();
                            break;
                        }
                    case 1:
                        {
                            typeBx.Text = "Single";
                            dataBx.Text = entryType.UnknownFloat1.ToString();
                            break;
                        }
                    case 2:
                        {
                            typeBx.Text = "Single";
                            dataBx.Text = entryType.UnknownFloat2.ToString();
                            break;
                        }
                    case 3:
                        {
                            typeBx.Text = "Single";
                            dataBx.Text = entryType.UnknownFloat3.ToString();
                            break;
                        }
                    case 4:
                        {
                            typeBx.Text = "Int32";
                            dataBx.Text = entryType.UnknownInt1.ToString();
                            break;
                        }
                    case 5:
                        {
                            typeBx.Text = "Int32";
                            dataBx.Text = entryType.UnknownInt2.ToString();
                            break;
                        }
                    case 6:
                        {
                            typeBx.Text = "Int64";
                            dataBx.Text = entryType.UnknownULong1.ToString();
                            break;
                        }
                    case 7:
                        {
                            typeBx.Text = "Int64";
                            dataBx.Text = entryType.UnknownULong2.ToString();
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
            if (Param is ForcesText.Entry entry)
            {
                switch (Index)
                {
                    case 0:
                        {
                            entry.LayoutIndex = int.TryParse(dataBx.Text, out tempInt) ?
                                tempInt >= forcesText.Layouts.Count ? forcesText.Layouts.Count - 1 : tempInt : 0;
                            break;
                        }
                    case 1:
                        {
                            entry.TypeName = typeBx.Text;
                            entry.Data = dataBx.Text;
                            break;
                        }
                    case 2:
                        {
                            entry.UUID = ulong.TryParse(dataBx.Text, out tempULong) ?
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
                                tempInt : 0;
                            break;
                        }
                    case 1:
                        {
                            layout.UnknownData2 = int.TryParse(dataBx.Text, out tempInt) ?
                                tempInt : 0;
                            break;
                        }
                    case 2:
                        {
                            layout.UnknownData3 = float.TryParse(dataBx.Text, out tempFloat) ?
                                tempFloat : 0;
                            break;
                        }
                    case 3:
                        {
                            layout.UnknownData4 = int.TryParse(dataBx.Text, out tempInt) ?
                                tempInt : 0;
                            break;
                        }
                    case 4:
                        {
                            layout.UnknownData5 = int.TryParse(dataBx.Text, out tempInt) ?
                                tempInt : 0;
                            break;
                        }
                    case 5:
                        {
                            layout.UnknownData6 = int.TryParse(dataBx.Text, out tempInt) ?
                                tempInt : 0;
                            break;
                        }
                    case 6:
                        {
                            layout.UnknownData7 = int.TryParse(dataBx.Text, out tempInt) ?
                                tempInt : 0;
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
            else if (Param is ForcesText.EntryType entryType)
            {
                switch (Index)
                {
                    case 0:
                        {
                            entryType.Namespace = dataBx.Text;
                            break;
                        }
                    case 1:
                        {
                            entryType.UnknownFloat1 = float.TryParse(dataBx.Text, out tempFloat) ?
                                tempFloat : 0;
                            break;
                        }
                    case 2:
                        {
                            entryType.UnknownFloat2 = float.TryParse(dataBx.Text, out tempFloat) ?
                                tempFloat : 0;
                            break;
                        }
                    case 3:
                        {
                            entryType.UnknownFloat3 = float.TryParse(dataBx.Text, out tempFloat) ?
                                tempFloat : 0;
                            break;
                        }
                    case 4:
                        {
                            entryType.UnknownInt1 = int.TryParse(dataBx.Text, out tempInt) ?
                                tempInt : 0;
                            break;
                        }
                    case 5:
                        {
                            entryType.UnknownInt2 = int.TryParse(dataBx.Text, out tempInt) ?
                                tempInt : 0;
                            break;
                        }
                    case 6:
                        {
                            entryType.UnknownULong1 = ulong.TryParse(dataBx.Text, out tempULong) ?
                                tempULong : 0;
                            break;
                        }
                    case 7:
                        {
                            entryType.UnknownULong2 = ulong.TryParse(dataBx.Text, out tempULong) ?
                                tempULong : 0;
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
