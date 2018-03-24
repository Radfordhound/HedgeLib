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
using HedgeLib.Misc;
using System.IO;

namespace HedgeCnvrsEditor
{
    public partial class MainFrm : Form
    {
        //Variables
        public static ForcesText ForcesText;
        private string openFile;
        private Random random = new Random();
        //Constructor
        public MainFrm()
        {
            InitializeComponent();
            treeView.AfterSelect += TreeView_Select;
            treeView.MouseClick += TreeView_Click;
            treeView.AfterLabelEdit += TreeView_LabelEdit;
            listView.DoubleClick += ListView_DoubleClick;
            filterTxtBx.TextChanged += FilterTxtBx_TextChanged;
        }

        //Methods

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
            //Update Nodes
            treeView.Nodes.Clear();
            var rootNode = treeView.Nodes.Add("nodes","Nodes");
            foreach (var node in ForcesText.Nodes)
            {
                var entryNode = rootNode.Nodes.Add(node.Name);
                entryNode.Tag = node;
                foreach (var entry in node.Entries)
                {
                    entryNode.Nodes.Add(new TreeNode()
                    {
                        Text = entry.Name,
                        Tag = entry,
                        Name = entry.Name
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
            var typesNode = treeView.Nodes.Add("types", "Value Types");
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

            if (treeView.SelectedNode.Tag is ForcesText.Entry entry)
            {
                var category = listView.Items.Add("Layout");
                category.SubItems.Add("String");
                category.SubItems.Add(ForcesText.Layouts[entry.LayoutIndex].Name);
                category.Tag = ForcesText.Layouts[entry.LayoutIndex].Name;


                var dataItem = listView.Items.Add("Data");
                dataItem.SubItems.Add(new ListViewItem.ListViewSubItem() { Text = entry.TypeName, Tag = entry.TypeName });
                dataItem.SubItems.Add(entry.Data.Replace("\n", "\\n"));
                dataItem.Tag = entry.Data;


                var id = listView.Items.Add("UUID");
                id.SubItems.Add("ULong");
                id.SubItems.Add(entry.UUID.ToString());
                id.Tag = entry.UUID;
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
            else if (treeView.SelectedNode.Tag is ForcesText.EntryType type)
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

            if (treeView.SelectedNode.Tag is ForcesText.Node node)
                ForcesText.Nodes.Remove(node);
            else if (treeView.SelectedNode.Tag is ForcesText.Layout layout)
                ForcesText.Layouts.Remove(layout);
            else if (treeView.SelectedNode.Tag is ForcesText.EntryType type)
                ForcesText.Types.Remove(treeView.SelectedNode.Name);
            else if (treeView.SelectedNode.Tag is ForcesText.Entry entry)
            {
                var parent = (ForcesText.Node)treeView.SelectedNode.Parent.Tag;
                parent.Entries.Remove(entry);
            }
            UpdateGUI();
        }

        void UpdateGUI()
        {
            FilterTreeView(filterTxtBx.Text);
            UpdateListView();
        }

        void FilterTreeView(string filter)
        {
            if (ForcesText == null)
                return;

            if(string.IsNullOrEmpty(filter))
            {
                UpdateTreeView();
                return;
            }

            treeView.BeginUpdate();
            //Update Nodes
            treeView.Nodes.Clear();
            var rootNode = treeView.Nodes.Add("nodes", "Nodes");
            foreach (var node in ForcesText.Nodes)
            {
                var entryNode = rootNode.Nodes.Add(node.Name);
                entryNode.Tag = node;
                foreach (var entry in node.Entries)
                {
                    if(entry.Name.Contains(filter))
                    {
                        entryNode.Nodes.Add(new TreeNode()
                        {
                            Text = entry.Name,
                            Tag = entry,
                            Name = entry.Name
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

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
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

        private void addEntry_Click(object sender, EventArgs e)
        {
            var selectedNode = treeView.SelectedNode;
            if (selectedNode.Tag is ForcesText.Node node)
            {
                node.Entries.Add(new ForcesText.Entry()
                {
                    UUID = (ulong)random.Next(),
                    Name = $"newEntry{node.Entries.Count}",
                    LayoutIndex = 0,
                    Data = "*Insert data here*",
                    TypeName = ForcesText.Types.First().Key
                });
            }
            else if(selectedNode.Tag is ForcesText.Entry)
            {
                var parentNode = (ForcesText.Node)selectedNode.Parent.Tag;
                parentNode.Entries.Add(new ForcesText.Entry()
                {
                    UUID = (ulong)random.Next(),
                    Name = $"newEntry{parentNode.Entries.Count}",
                    LayoutIndex = 0,
                    Data = "*Insert data here*",
                    TypeName = ForcesText.Types.First().Key
                });
            }
            else if(selectedNode.Name == "types")
            {
                ForcesText.Types.Add($"newValueType{ForcesText.Types.Count}", new ForcesText.EntryType()
                {
                    Namespace = "mixStd",
                });
            }
            else if(selectedNode.Name == "layouts")
            {
                ForcesText.Layouts.Add(new ForcesText.Layout()
                {
                    Name = $"newLayout{ForcesText.Layouts.Count}"
                });
            }
            else if(selectedNode.Name == "nodes")
            {
                ForcesText.Nodes.Add(new ForcesText.Node()
                {
                    Name = $"newNode{ForcesText.Nodes.Count}"
                });
            }
            FilterTreeView(filterTxtBx.Text);
        }

        private void TreeView_LabelEdit(object sender, NodeLabelEditEventArgs e)
        {
            var selectedNode = treeView.SelectedNode;
            if (selectedNode.Tag != null && !string.IsNullOrEmpty(e.Label))
            {
                if (selectedNode.Tag is ForcesText.Node node)
                    node.Name = e.Label;
                else if (selectedNode.Tag is ForcesText.Entry entry)
                    entry.Name = e.Label;
                else if (selectedNode.Tag is ForcesText.Layout layout)
                    layout.Name = e.Label;
                else if (selectedNode.Tag is ForcesText.EntryType type)
                {
                    var value = ForcesText.Types[selectedNode.Name];
                    ForcesText.Types.Remove(selectedNode.Text);
                    ForcesText.Types.Add(e.Label, value);
                }
            }
        }

        private void exportXMLToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var sfd = new SaveFileDialog()
            {
                Filter = "XML Files|*.xml",
                FileName = Path.ChangeExtension(Path.GetFileName(openFile),".xml")
            };
            if (sfd.ShowDialog() == DialogResult.OK)
            {
                ForcesText.ExportXML(sfd.FileName);
            }
        }

        private void importXMLToolStripMenuItem_Click(object sender, EventArgs e)
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
                        filterTxtBx.Visible = filterTxtBx.Enabled = !filterTxtBx.Enabled;
                        filterTxtBx.Text = string.Empty;
                        filterTxtBx.Select();
                        return true;
                    }
                default:
                    return base.ProcessCmdKey(ref msg, keyData);
            }
        }

        private void renameTsm_Click(object sender, EventArgs e)
        {
            if (treeView.SelectedNode != null)
                treeView.SelectedNode.BeginEdit();
        }

        private void deleteTsm_Click(object sender, EventArgs e)
        {
            DeleteItem();
        }

        private void FilterTxtBx_TextChanged(object sender, EventArgs e)
        {
            FilterTreeView(filterTxtBx.Text);
        }
    }
    #endregion
}
