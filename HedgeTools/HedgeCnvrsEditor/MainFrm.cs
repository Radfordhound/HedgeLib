using System;
using System.Linq;
using System.Windows.Forms;
using HedgeLib.Misc;
using System.IO;
using System.Drawing;

namespace HedgeCnvrsEditor
{
    public partial class MainFrm : Form
    {
        // Variables/Constants
        public static ForcesText ForcesText;
        private string openFile;
        private Random random = new Random();

        // Constructors
        public MainFrm()
        {
            InitializeComponent();
            treeView.AfterSelect += TreeView_Select;
            treeView.MouseClick += TreeView_Click;
            treeView.AfterLabelEdit += TreeView_LabelEdit;
            listView.DoubleClick += ListView_DoubleClick;
            filterTxtBx.TextChanged += FilterTxtBx_TextChanged;
            filterTxtBx.LostFocus += FilterTxtBx_LostFocus;
            filterTxtBx.GotFocus += FilterTxtBx_GotFocus;
        }

        // Methods
        void AddLVI<T>(string name, T value) where T : struct
        {
            var item = listView.Items.Add(name);
            item.SubItems.Add(value.GetType().Name);
            item.SubItems.Add(value.ToString());
        }

        void AddLVI<T>(string name, T? value) where T : struct
        {
            var item = listView.Items.Add(name);
            item.SubItems.Add(typeof(T).Name);
            item.SubItems.Add(value.HasValue ? value.ToString() : "null");
        }

        void AddLVI(string name, string value)
        {
            var item = listView.Items.Add(name);
            item.SubItems.Add(value.GetType().Name);
            item.SubItems.Add(value.ToString());
        }

        public void UpdateTreeView()
        {
            if (ForcesText == null)
                return;

            treeView.BeginUpdate();

            // Update Sheets
            treeView.Nodes.Clear();
            var rootNode = treeView.Nodes.Add("sheets", "Sheets");
            foreach (var sheet in ForcesText.Sheets)
            {
                var sheetNode = rootNode.Nodes.Add(sheet.Name);
                sheetNode.Tag = sheet;

                foreach (var cell in sheet.Cells)
                {
                    sheetNode.Nodes.Add(new TreeNode()
                    {
                        Text = cell.Name,
                        Tag = cell,
                        Name = cell.Name
                    });
                }
            }

            var layoutsNode = treeView.Nodes.Add("layouts", "Layouts");
            foreach (var category in ForcesText.Layouts)
            {
                layoutsNode.Nodes.Add(new TreeNode()
                {
                    Text = category.Name,
                    Tag = category,
                    Name = category.Name
                });
            }

            var typesNode = treeView.Nodes.Add("types", "Cell Types");
            foreach (var type in ForcesText.Types)
            {
                typesNode.Nodes.Add(new TreeNode()
                {
                    Name = type.Key,
                    Text = type.Key,
                    Tag = type.Value
                });
            }

            treeView.EndUpdate();
        }

        public void UpdateListView()
        {
            listView.BeginUpdate();
            listView.Items.Clear();

            if (treeView.SelectedNode == null)
            {
                listView.EndUpdate();
                return;
            }

            if (treeView.SelectedNode.Tag is ForcesText.Cell cell)
            {
                var category = listView.Items.Add("Layout");
                category.SubItems.Add("String");
                category.SubItems.Add(ForcesText.Layouts[cell.LayoutIndex].Name);
                category.Tag = ForcesText.Layouts[cell.LayoutIndex].Name;


                var dataItem = listView.Items.Add("Data");
                dataItem.SubItems.Add(new ListViewItem.ListViewSubItem() { Text = cell.TypeName, Tag = cell.TypeName });
                dataItem.SubItems.Add(cell.Data.Replace("\n", "\\n"));
                dataItem.Tag = cell.Data;


                var id = listView.Items.Add("UUID");
                id.SubItems.Add("ULong");
                id.SubItems.Add(cell.UUID.ToString());
                id.Tag = cell.UUID;
            }
            else if (treeView.SelectedNode.Tag is ForcesText.Layout Layout)
            {
                AddLVI("Unknown Data 1", Layout.UnknownData1);
                AddLVI("Unknown Data 2", Layout.UnknownData2);
                AddLVI("Unknown Data 3", Layout.UnknownData3);
                AddLVI("Unknown Data 4", Layout.UnknownData4);
                AddLVI("Unknown Data 5", Layout.UnknownData5);
                AddLVI("Unknown Data 6", Layout.UnknownData6);
                AddLVI("Unknown Data 7", Layout.UnknownData7);
                AddLVI("Unknown Data 8", Layout.UnknownData8);
            }
            else if (treeView.SelectedNode.Tag is ForcesText.CellType type)
            {
                AddLVI("Namespace", type.Namespace);
                AddLVI("Unknown Float 1", type.UnknownFloat1);
                AddLVI("Unknown Float 2", type.UnknownFloat2);
                AddLVI("Unknown Float 3", type.UnknownFloat3);
                AddLVI("Unknown Int 1", type.UnknownInt1);
                AddLVI("Unknown Int 1", type.UnknownInt2);
                AddLVI("Unknown ULong 1", type.UnknownULong1);
                AddLVI("Unknown ULong 2", type.UnknownULong2);
            }
            listView.EndUpdate();
        }

        void DeleteItem()
        {
            if (treeView.SelectedNode == null)
                return;

            if (treeView.SelectedNode.Tag is ForcesText.Sheet sheet)
                ForcesText.Sheets.Remove(sheet);
            else if (treeView.SelectedNode.Tag is ForcesText.Layout layout)
                ForcesText.Layouts.Remove(layout);
            else if (treeView.SelectedNode.Tag is ForcesText.CellType type)
                ForcesText.Types.Remove(treeView.SelectedNode.Name);
            else if (treeView.SelectedNode.Tag is ForcesText.Cell cell)
            {
                var parent = (ForcesText.Sheet)treeView.SelectedNode.Parent.Tag;
                parent.Cells.Remove(cell);
            }
            UpdateGUI();
        }

        void UpdateGUI()
        {
            FilterTreeView(filterTxtBx.ForeColor == Color.Gray ? string.Empty : filterTxtBx.Text);
            UpdateListView();
        }

        void FilterTreeView(string filter = null)
        {
            if (ForcesText == null)
                return;

            if (string.IsNullOrEmpty(filter))
            {
                UpdateTreeView();
                return;
            }

            treeView.BeginUpdate();

            // Update Nodes
            treeView.Nodes.Clear();
            var rootNode = treeView.Nodes.Add("sheets", "Sheets");

            foreach (var sheet in ForcesText.Sheets)
            {
                var sheetNode = rootNode.Nodes.Add(sheet.Name);
                sheetNode.Tag = sheet;

                foreach (var cell in sheet.Cells)
                {
                    if (cell.Name.Contains(filter))
                    {
                        sheetNode.Nodes.Add(new TreeNode()
                        {
                            Text = cell.Name,
                            Tag = cell,
                            Name = cell.Name
                        });
                    }
                }
            }

            var layoutsNode = treeView.Nodes.Add("layouts", "Layouts");
            foreach (var layout in ForcesText.Layouts)
            {
                if (layout.Name.Contains(filter))
                {
                    layoutsNode.Nodes.Add(new TreeNode()
                    {
                        Text = layout.Name,
                        Tag = layout,
                        Name = layout.Name
                    });
                }
            }

            var typesNode = treeView.Nodes.Add("types", "Value Types");
            foreach (var type in ForcesText.Types)
            {
                if (type.Key.Contains(filter))
                {
                    typesNode.Nodes.Add(new TreeNode()
                    {
                        Name = type.Key,
                        Text = type.Key,
                        Tag = type.Value
                    });
                }
            }

            treeView.EndUpdate();
        }

        #region GUI Events
        //GUI Events
        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var ofd = new OpenFileDialog()
            {
                Filter = "Sonic Forces Text Files|*.cnvrs-text",
                CheckFileExists = true,
            };
            if(ofd.ShowDialog() == DialogResult.OK)
            {
                ForcesText = new ForcesText();
                openFile = ofd.FileName;
                ForcesText.Load(ofd.FileName);
                UpdateGUI();
                saveToolStripMenuItem.Enabled = true;
                saveAsToolStripMenuItem.Enabled = true;
                exportXMLToolStripMenuItem.Enabled = true;
            }
        }

        private void saveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var sfd = new SaveFileDialog()
            {
                Filter = "Sonic Forces Text Files|*.cnvrs-text",
            };
            if(sfd.ShowDialog() == DialogResult.OK)
            {
                openFile = sfd.FileName;
                ForcesText.Save(sfd.FileName, File.Exists(sfd.FileName));
            }
        }

        private void SaveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ForcesText.Save(openFile, true);
        }

        private void TreeView_Select(object sender, TreeViewEventArgs e)
        {
            UpdateListView();
        }

        void ListView_DoubleClick(object sender,object e)
        {
            var prop = treeView.SelectedNode.Tag;
            int index = listView.SelectedIndices[0];
            var dialog = new ParameterEditor(prop,index,ForcesText);
            dialog.ShowDialog();
        }

        private void refreshToolStripMenuItem_Click(object sender, EventArgs e)
        {
            UpdateGUI();
        }

        void TreeView_Click(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right && treeView.SelectedNode != null)
            {
                TVMenu.Show(Cursor.Position);
            }
        }

        private void AddEntry_Click(object sender, EventArgs e)
        {
            var selectedNode = treeView.SelectedNode;
            if (selectedNode.Tag is ForcesText.Sheet sheet)
            {
                sheet.Cells.Add(new ForcesText.Cell()
                {
                    UUID = (ulong)random.Next(),
                    Name = $"newCell{sheet.Cells.Count}",
                    LayoutIndex = 0,
                    Data = "*Insert data here*",
                    TypeName = ForcesText.Types.First().Key
                });
            }
            else if (selectedNode.Tag is ForcesText.Cell)
            {
                var parentNode = (ForcesText.Sheet)selectedNode.Parent.Tag;
                parentNode.Cells.Add(new ForcesText.Cell()
                {
                    UUID = (ulong)random.Next(),
                    Name = $"newCell{parentNode.Cells.Count}",
                    LayoutIndex = 0,
                    Data = "*Insert data here*",
                    TypeName = ForcesText.Types.First().Key
                });
            }
            else if (selectedNode.Name == "types")
            {
                ForcesText.Types.Add($"newValueType{ForcesText.Types.Count}",
                    new ForcesText.CellType()
                    {
                        Namespace = "mixStd",
                    });
            }
            else if (selectedNode.Name == "layouts")
            {
                ForcesText.Layouts.Add(new ForcesText.Layout()
                {
                    Name = $"newLayout{ForcesText.Layouts.Count}"
                });
            }
            else if (selectedNode.Name == "sheets")
            {
                ForcesText.Sheets.Add(new ForcesText.Sheet()
                {
                    Name = $"newSheet{ForcesText.Sheets.Count}"
                });
            }

            FilterTreeView();
        }

        private void TreeView_LabelEdit(object sender, NodeLabelEditEventArgs e)
        {
            var selectedNode = treeView.SelectedNode;
            if (selectedNode.Tag != null && !string.IsNullOrEmpty(e.Label))
            {
                if (selectedNode.Tag is ForcesText.Sheet sheet)
                    sheet.Name = e.Label;
                else if (selectedNode.Tag is ForcesText.Cell cell)
                    cell.Name = e.Label;
                else if (selectedNode.Tag is ForcesText.Layout layout)
                    layout.Name = e.Label;
                else if (selectedNode.Tag is ForcesText.CellType type)
                {
                    var value = ForcesText.Types[selectedNode.Name];
                    ForcesText.Types.Remove(selectedNode.Text);
                    ForcesText.Types.Add(e.Label, value);
                }
            }
        }

        private void ExportXMLToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var sfd = new SaveFileDialog()
            {
                Filter = "XML Files|*.xml",
                FileName = Path.ChangeExtension(Path.GetFileName(openFile),".xml")
            };

            if (sfd.ShowDialog() == DialogResult.OK)
                ForcesText.ExportXML(sfd.FileName);
        }

        private void ImportXMLToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var ofd = new OpenFileDialog()
            {
                Filter = "XML Files|*.xml",
                CheckFileExists = true
            };

            if (ofd.ShowDialog() == DialogResult.OK)
            {
                ForcesText = new ForcesText();
                ForcesText.ImportXML(ofd.FileName);
                saveToolStripMenuItem.Enabled = true;
                saveAsToolStripMenuItem.Enabled = true;
                exportXMLToolStripMenuItem.Enabled = true;
                UpdateGUI();
            }
        }

        protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
        {
            switch(keyData)
            {
                case Keys.Delete:
                    {
                        DeleteItem();
                        return true;
                    }
                case Keys.Control | Keys.F:
                    {
                        filterTxtBx.Select();
                        return true;
                    }
                default:
                    return base.ProcessCmdKey(ref msg, keyData);
            }
        }

        private void RenameTsm_Click(object sender, EventArgs e)
        {
            if (treeView.SelectedNode != null)
                treeView.SelectedNode.BeginEdit();
        }

        private void DeleteTsm_Click(object sender, EventArgs e)
        {
            DeleteItem();
        }

        private void FilterTxtBx_TextChanged(object sender, EventArgs e)
        {
            if(filterTxtBx.ForeColor == Color.Black && ForcesText != null)
            FilterTreeView(filterTxtBx.Text);
        }

        private void FilterTxtBx_LostFocus(object sender, EventArgs e)
        {
            if(string.IsNullOrEmpty(filterTxtBx.Text))
            {
                filterTxtBx.ForeColor = Color.Gray;
                filterTxtBx.Text = "Type here to search";
            }
        }

        private void FilterTxtBx_GotFocus(object sender, EventArgs e)
        {
            if(filterTxtBx.ForeColor == Color.Gray)
            {
                filterTxtBx.ForeColor = Color.Black;
                filterTxtBx.Text = string.Empty;
            }
        }
    }
    #endregion
}