using HedgeLib.Sets;
using System;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

namespace HedgeEdit.UI
{
    public partial class SceneView : Form
    {
        // Variables/Constants
        private MainFrm mainForm = null;

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
            treeView.BeginUpdate();
            treeView.Nodes.Clear();

            // Layers
            var layersNode = new TreeNode("Set Layers");
            for (int i = 0; i < Stage.Sets.Count; ++i)
            {
                var layer = Stage.Sets[i];
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
        private void TreeView_AfterSelect(object sender, TreeViewEventArgs e)
        {
            var tag = e.Node.Tag;
            if (tag == null)
                return;

            var layer = (tag as SetData);
            if (layer != null)
            {
                Stage.CurrentSetLayer = layer;
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

        private void SceneViewMenu_Opening(object sender, CancelEventArgs e)
        {
            bool layerClicked = (treeView.SelectedNode != null &&
                treeView.SelectedNode.Tag is SetData);

            exportLayerMenuItem.Enabled = layerClicked;
        }

        private void AddLayerMenuItem_Click(object sender, EventArgs e)
        {
            // TODO
        }

        private void ImportLayerMenuItem_Click(object sender, EventArgs e)
        {
            // TODO
        }

        private void ExportLayersMenuItem_Click(object sender, EventArgs e)
        {
            // TODO
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

                RefreshView();
                mainForm.RefreshGUI();
            }
        }
    }
}