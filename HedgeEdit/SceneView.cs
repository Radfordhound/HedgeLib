using HedgeLib.Sets;
using System.Drawing;
using System.Windows.Forms;

namespace HedgeEdit
{
    public partial class SceneView : Form
    {
        //Variables/Constants
        private MainFrm mainForm = null;

        //Constructors
        public SceneView(MainFrm mainFrm)
        {
            Owner = mainForm = mainFrm;

            InitializeComponent();
            RefreshView();

            Height = mainFrm.Height - 78;
            Location = new Point(
                mainFrm.Width - Width,
                mainFrm.Location.Y + 78);
        }

        //Methods
        public void RefreshView()
        {
            treeView.BeginUpdate();
            treeView.Nodes.Clear();

            // Layers
            for (int i = 0; i < Stage.Sets.Count; ++i)
            {
                var layer = Stage.Sets[i];
                var layerNode = new TreeNode((string.IsNullOrEmpty(layer.Name)) ?
                    $"Layer #{i}" : layer.Name);

                // Objects
                foreach (var obj in layer.Objects)
                {
                    var objNode = new TreeNode($"{obj.ObjectType} ({obj.ObjectID})")
                    {
                        Tag = obj
                    };

                    // Children objects
                    if (obj.Children != null)
                    {
                        for (int i2 = 0; i2 < obj.Children.Length;)
                        {
                            objNode.Nodes.Add(new TreeNode($"Child {++i2}")
                            {
                                Tag = obj
                            });
                        }
                    }

                    layerNode.Nodes.Add(objNode);
                }

                treeView.Nodes.Add(layerNode);
            }

            treeView.EndUpdate();
        }

        //GUI Events
        private void TreeView_AfterSelect(object sender, TreeViewEventArgs e)
        {
            if (e.Node.Tag != null)
            {
                mainForm.SelectedObject = (e.Node.Tag as SetObject);
            }
        }
    }
}