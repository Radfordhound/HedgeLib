using HedgeLib.Sets;
using MoonSharp.Interpreter;
using System;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

namespace HedgeEdit.UI
{
    public partial class SceneView : Form
    {
        // Variables/Constants
        public static bool CanAddLayer = false;
        private MainFrm mainForm = null;
        private string filter = string.Empty;

        // Constructors
        public SceneView()
        {
            Owner = mainForm = Program.MainForm;

            InitializeComponent();
            RefreshView();

            Height = mainForm.Height - 78;
            Location = new Point(
                mainForm.Location.X + mainForm.Width - Width,
                mainForm.Location.Y + 78);
        }

        // Methods
        public void RefreshView()
        {
            addLayerMenuItem.Enabled = CanAddLayer;
            treeView.BeginUpdate();
            treeView.Nodes.Clear();

            // Layers
            var layersNode = new TreeNode("Set Layers");
            for (int i = 0; i < Data.SetLayers.Count; ++i)
            {
                var layer = Data.SetLayers[i];
                string layerName = (string.IsNullOrEmpty(layer.Name)) ?
                    $"Layer #{i + 1}" : layer.Name;

                var layerNode = new TreeNode(layerName)
                {
                    Tag = layer
                };

                // Objects
                foreach (var obj in layer.Objects)
                {
                    // If the object has a custom name, use it.
                    // Otherwise, use object type + ID.
                    string name = (obj.CustomData.ContainsKey("Name")) ?
                        $"{(obj.CustomData["Name"].Data as string)} ({obj.ObjectID})" :
                        $"{obj.ObjectType} ({obj.ObjectID})";
                    if (name.Contains(filter))
                    {
                        var objNode = new TreeNode(name)
                        {
                            Tag = obj
                        };

                        // Children objects
                        if (obj.Children != null)
                        {
                            for (int i2 = 0; i2 < obj.Children.Length;)
                            {
                                var child = obj.Children[i2];
                                objNode.Nodes.Add(new TreeNode($"Child {++i2}")
                                {
                                    Tag = child
                                });
                            }
                        }

                        layerNode.Nodes.Add(objNode);
                    }
                }

                layersNode.Nodes.Add(layerNode);
            }

            treeView.Nodes.Add(layersNode);

            // Terrain
            var terrainNode = new TreeNode("Terrain Instances");
            foreach (var mdl in Data.DefaultTerrainGroup)
            {
                foreach (var instance in mdl.Value.Instances)
                {
                    AddTerrainNode(terrainNode, instance);
                }
            }

            foreach (var group in Data.TerrainGroups)
            {
                var groupNode = new TreeNode(group.Key);
                foreach (var mdl in group.Value)
                {
                    foreach (var instance in mdl.Value.Instances)
                    {
                        if(((string)instance.CustomData).Contains(filter))
                        AddTerrainNode(groupNode, instance);
                    }
                }

                terrainNode.Nodes.Add(groupNode);
            }

            treeView.Nodes.Add(terrainNode);
            treeView.EndUpdate();

            // Sub-Methods
            void AddTerrainNode(TreeNode node, VPObjectInstance instance)
            {
                string name = (instance.CustomData as string);
                if (string.IsNullOrEmpty(name))
                    return;

                node.Nodes.Add(new TreeNode(name)
                {
                    Tag = instance
                });
            }
        }

        // GUI Events
        private void SceneView_FormClosing(object sender, FormClosingEventArgs e)
        {
            Program.MainForm.SceneViewMenuItem.Checked = false;
        }

        private void TreeView_AfterSelect(object sender, TreeViewEventArgs e)
        {
            var tag = e.Node.Tag;
            if (tag == null)
                return;

            var layer = (tag as SetData);
            if (layer != null)
            {
                Data.CurrentSetLayer = layer;
                Program.MainForm.UpdateTitle(Stage.ID);
                Program.MainForm.PasteMenuItem.Enabled = (layer != null);
            }
            else if (tag is VPObjectInstance instance)
            {
                Viewport.SelectedInstances.Clear();
                Viewport.SelectedInstances.Add(instance);
                mainForm.RefreshGUI();
            }
            else
            {
                Viewport.SelectedInstances.Clear();
                Viewport.SelectObject(tag);
                mainForm.RefreshGUI();
            }
        }

        private void TreeView_AfterLabelEdit(object sender, NodeLabelEditEventArgs e)
        {
            if (string.IsNullOrWhiteSpace(e.Label))
            {
                e.CancelEdit = true;
                return;
            }

            if (e.Node.Tag is SetData layer)
            {
                var result = Stage.Script.Call("RenameLayer", e.Label);
                if (result == null || result.Type != DataType.Boolean || result.Boolean)
                    layer.Name = e.Label;
                else
                    e.CancelEdit = true;

                return;
            }
        }

        private void TreeView_NodeMouseClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            treeView.SelectedNode = e.Node;
        }

        private void SceneViewMenu_Opening(object sender, CancelEventArgs e)
        {
            bool layerClicked = (treeView.SelectedNode != null &&
                treeView.SelectedNode.Tag is SetData);

            exportLayerMenuItem.Enabled = layerClicked;
            addLayerMenuItem.Enabled = CanAddLayer;
        }

        private void AddLayerMenuItem_Click(object sender, EventArgs e)
        {
            if (Stage.GameType == null)
                return;

            var layer = Types.SetDataType;
            var result = Stage.Script.Call("AddLayer", layer);
            if (result != null && result.Type == DataType.Boolean)
            {
                if (!result.Boolean)
                    return;
            }
            else
            {
                string name = GUI.ShowTextBox("What would you like to call this layer?");
                if (string.IsNullOrEmpty(name))
                    return;

                layer.Name = name;
            }

            Data.SetLayers.Add(layer);
            RefreshView();
        }

        private void ImportLayerMenuItem_Click(object sender, EventArgs e)
        {
            // TODO
        }

        private void ExportLayersMenuItem_Click(object sender, EventArgs e)
        {
            // TODO
        }

        private void RenameLayerMenuItem_Click(object sender, EventArgs e)
        {
            if (treeView.SelectedNode == null || Stage.GameType == null)
                return;

            if (treeView.SelectedNode.Tag is SetData layer)
                treeView.SelectedNode.BeginEdit();
        }

        private void DeleteLayerMenuItem_Click(object sender, EventArgs e)
        {
            if (treeView.SelectedNode == null || Stage.GameType == null)
                return;

            if (treeView.SelectedNode.Tag is SetData layer && MessageBox.Show(
                $"Are you sure you wish to delete {layer.Name} and all of its objects?",
                Program.Name, MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.Yes)
            {
                for (int i = layer.Objects.Count - 1; i >= 0; --i)
                {
                    var obj = layer.Objects[i];
                    layer.Objects.RemoveAt(i);

                    // Remove all children of this object (if any)
                    if (obj.Children != null)
                    {
                        foreach (var child in obj.Children)
                        {
                            Data.GetObject(obj, out VPModel mdl,
                                out VPObjectInstance inst);

                            if (mdl != null && inst != null)
                            {
                                mdl.Instances.Remove(inst);
                                Viewport.SelectedInstances.Remove(inst);
                            }
                        }
                    }

                    // Remove the actual object itself
                    Data.GetObject(obj, out VPModel model,
                        out VPObjectInstance instance);

                    if (model != null && instance != null)
                    {
                        model.Instances.Remove(instance);
                        Viewport.SelectedInstances.Remove(instance);
                    }
                }

                if (Data.CurrentSetLayer == layer)
                {
                    Data.CurrentSetLayer = null;
                    Program.MainForm.UpdateTitle(Stage.ID);
                }

                Data.SetLayers.Remove(layer);
                RefreshView();
                mainForm.RefreshGUI();
            }
        }

        private void ExportLayerMenuItem_Click(object sender, EventArgs e)
        {
            if (treeView.SelectedNode == null || Stage.GameType == null)
                return;

            if (treeView.SelectedNode.Tag is SetData layer)
            {
                var sfd = new SaveFileDialog()
                {
                    Title = "Export Set Layer...",
                    Filter = "HedgeLib XML Set Layer (*.xml)|*.xml|All Files (*.*)|*.*",
                    FileName = $"{layer.Name}.xml"
                };

                if (sfd.ShowDialog() == DialogResult.OK)
                {
                    layer.ExportXML(sfd.FileName, Stage.GameType.ObjectTemplates);
                }
            }
        }

        protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
        {
            switch (keyData)
            {
                case Keys.Control | Keys.F:
                    {
                        ToggleFilterBx();
                        return true;
                    }

                default:
                    return base.ProcessCmdKey(ref msg, keyData);
            }
        }

        public void ToggleFilterBx()
        {
            filterBx.Visible = filterBx.Enabled = !filterBx.Focused;

            if (filterBx.Enabled)
            {
                filterBx.Select();
            }
            else
            {
                Select();
                filter = string.Empty;
                filterBx.Text = filter;
                RefreshView();
            }
        }

        private void FilterBx_Changed(object sender, EventArgs e)
        {
            filter = filterBx.Text;
            RefreshView();
        }
    }
}