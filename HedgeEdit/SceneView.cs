using HedgeLib.Sets;
using System.Drawing;
using System.Windows.Forms;

namespace HedgeEdit
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
                        (obj.CustomData["Name"].Data as string) :
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

                treeView.Nodes.Add(layerNode);
            }

            treeView.EndUpdate();
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
            else
            {
                Viewport.SelectedInstances.Clear();
                Viewport.SelectObject(tag);
                mainForm.RefreshGUI();
            }
        }
    }
}